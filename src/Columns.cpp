#include "clickhouse/block.h"
#include "clickhouse/columns/array.h"
#include "clickhouse/columns/column.h"
#include "clickhouse/columns/date.h"
#include "clickhouse/columns/decimal.h"
#include "clickhouse/columns/enum.h"
#include "clickhouse/columns/ip4.h"
#include "clickhouse/columns/ip6.h"
#include "clickhouse/columns/nullable.h"
#include "clickhouse/columns/numeric.h"
#include "clickhouse/columns/string.h"
#include "clickhouse/columns/tuple.h"
#include "clickhouse/types/types.h"
#include "phpcpp.h"
#include "common.h"
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <Zend/zend_operators.h>
#include <Zend/zend_API.h>
#include <Zend/zend_hash.h>
#include <string>
#include <tuple>
#include "Columns.h"
#include "zend_types.h"

using ZvalCallback = std::function<void(zval *item)>;

int nestedLength(const Php::Value &data, const Php::Value &index, bool columnar);

void nestedForeach(const Php::Value &data, const Php::Value &index, bool columnar, ZvalCallback callback);

template<typename T>
class VectorLong : public ColumnType
{
private:
    std::vector<T> vec;

public:
    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	vec.reserve(nestedLength(data, index, columnar));
	nestedForeach(data, index, columnar, [&](zval *val) {
	    vec.push_back(zval_get_long(val));
	});
    }

    void append(zval *val) override {
	vec.push_back(zval_get_long(val));
    }

    void clear() override {
	vec.clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return std::make_shared<clickhouse::ColumnVector<T>>(vec);
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnVector<T>>();
    }
};

template<typename T>
class VectorDouble : public ColumnType
{
private:
    std::vector<T> vec;

public:
    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	vec.reserve(nestedLength(data, index, columnar));
	nestedForeach(data, index, columnar, [&](zval *val) {
	    vec.push_back(zval_get_double(val));
	});
    }

    void append(zval *val) override {
	vec.push_back(zval_get_double(val));
    }

    void clear() override {
	vec.clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return std::make_shared<clickhouse::ColumnVector<T>>(vec);
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnVector<T>>();
    }
};

class ColumnNullable : public ColumnType
{
private:
    std::vector<std::uint8_t> nullsVec;
    ColumnType * nested;
    Php::Value v; // holder of nested

public:
    void __construct(Php::Parameters &params)
    {
	v = params[0]; // keep the reference
	nested = dynamic_cast<ColumnType *>(v.implementation());
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nested->append(data, index, columnar);

	nullsVec.reserve(nestedLength(data, index, columnar));
	nestedForeach(data, index, columnar, [&](zval *val) {
	    nullsVec.push_back(ZVAL_IS_NULL(val));
	});
    }

    void append(zval *val) override
    {
	nested->append(val);
	nullsVec.push_back(ZVAL_IS_NULL(val));
    }

    void clear() override {
	nested->clear();
	nullsVec.clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return std::make_shared<clickhouse::ColumnNullable>(nested->currentColumn(), std::make_shared<clickhouse::ColumnUInt8>(nullsVec));
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnNullable>(nested->emptyColumn(), std::make_shared<clickhouse::ColumnUInt8>(nullsVec));
    }
};

class ColumnTuple : public ColumnType
{
public:
    std::vector<ColumnType *> nested;
    std::vector<Php::Value> vs; // holder of nested

    void __construct(Php::Parameters &params)
    {
	for (auto p : params) {
	    vs.push_back(p);
	    nested.push_back(dynamic_cast<ColumnType *>(p.implementation()));
	}
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) { // val is a tuple (array)
	    append(val);
	});
    }

    void append(zval *val) override
    {
	int i = 0;
	for (auto col: nested) {
	    col->append(zend_hash_index_find(Z_ARRVAL_P(val), i));
	    i += 1;
	}
    }

    void clear() override
    {
	for (auto col: nested) {
	    col->clear();
	}
    }

    clickhouse::ColumnRef currentColumn() override {
	std::vector<clickhouse::ColumnRef> columns;
	for (auto col: nested) {
	    columns.push_back(col->currentColumn());
	}
	return std::make_shared<clickhouse::ColumnTuple>(columns);
    }

    clickhouse::ColumnRef emptyColumn() override {
	std::vector<clickhouse::ColumnRef> columns;
	for (auto col: nested) {
	    columns.push_back(col->emptyColumn());
	}
	return std::make_shared<clickhouse::ColumnTuple>(columns);
    }
};

class ColumnArray : public ColumnType
{
private:
    ColumnType * nested;
    Php::Value v; // holder of nested
    std::shared_ptr<clickhouse::ColumnArray> array = 0;

public:
    void __construct(Php::Parameters &params)
    {
	v = params[0]; // keep the reference
	nested = dynamic_cast<ColumnType *>(v.implementation());
	array = std::make_shared<clickhouse::ColumnArray>(nested->emptyColumn());
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) { // val is an array
	    append(val);
	});
    }

    void append(zval *val) override
    {
	zval *item;
	auto ht = Z_ARRVAL_P(val);
	nested->clear();
	ZEND_HASH_FOREACH_VAL(ht, item) {
	    nested->append(item);
	}
	ZEND_HASH_FOREACH_END();
	array->AppendAsColumn(nested->currentColumn());
    }

    void clear() override {
	nested->clear();
	array->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return array;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnArray>(nested->emptyColumn());
    }
};

class ColumnDecimal : public ColumnType
{
protected:
    std::shared_ptr<clickhouse::ColumnDecimal> column;
    int precision, scale;
public:
    void __construct(Php::Parameters &params)
    {
	precision = params[0];
	scale = params[1];
	column = std::make_shared<clickhouse::ColumnDecimal>(precision, scale);
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	column->Append(zval_get_string(val)->val);
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnDecimal>(precision, scale);
    }
};

class ColumnDecimal32 : public ColumnDecimal
{
public:
    void __construct(Php::Parameters &params)
    {
	precision = 9;
	scale = params[0];
	column = std::make_shared<clickhouse::ColumnDecimal>(9, scale);
    }
};

class ColumnDecimal64 : public ColumnDecimal
{
public:
    void __construct(Php::Parameters &params)
    {
	precision = 18;
	scale = params[0];
	column = std::make_shared<clickhouse::ColumnDecimal>(18, scale);
    }
};

class ColumnDecimal128 : public ColumnDecimal
{
public:
    void __construct(Php::Parameters &params)
    {
	precision = 38;
	scale = params[0];
	column = std::make_shared<clickhouse::ColumnDecimal>(38, scale);
    }
};


template<typename T>
class ColumnAsString : public ColumnType
{
private:
    std::shared_ptr<T> column;

public:
    void __construct(Php::Parameters &params)
    {
	column = std::make_shared<T>();
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	column->Append(zval_get_string(val)->val);
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<T>();
    }
};

class ColumnFixedString : public ColumnType
{
private:
    std::shared_ptr<clickhouse::ColumnFixedString> column;
    size_t len;
public:
    void __construct(Php::Parameters &params)
    {
	len = params[0].numericValue();
	column = std::make_shared<clickhouse::ColumnFixedString>(len);
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	column->Append(zval_get_string(val)->val);
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnFixedString>(len);
    }
};

class ColumnIPv4 : public ColumnType
{
private:
    std::shared_ptr<clickhouse::ColumnIPv4> column;

public:
    void __construct(Php::Parameters &params)
    {
	column = std::make_shared<clickhouse::ColumnIPv4>();
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	if (Z_TYPE_P(val) == IS_STRING) {
	    column->Append(zval_get_string(val)->val);
	} else {
	    column->Append(ntohl(zval_get_long(val)));
	}
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnIPv4>();
    }
};

template<typename T>
class ColumnAsLong : public ColumnType
{
private:
    std::shared_ptr<T> column;

public:
    void __construct(Php::Parameters &params)
    {
	column = std::make_shared<T>();
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	column->Append(zval_get_long(val));
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<T>();
    }
};

class ColumnEnum8 : public ColumnType
{
private:
    std::shared_ptr<clickhouse::ColumnEnum8> column;
    std::vector<clickhouse::Type::EnumItem> enumItems;
public:
    void __construct(Php::Parameters &params)
    {
	for (std::pair<Php::Value, Php::Value> e: params[0]) {
	    enumItems.push_back(std::pair(e.first.stringValue(), e.second.numericValue()));
	}
	column = std::make_shared<clickhouse::ColumnEnum8>(clickhouse::Type::CreateEnum8(enumItems));
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	if (Z_TYPE_P(val) == IS_STRING) {
	    column->Append(zval_get_string(val)->val);
	} else {
	    column->Append(zval_get_long(val), false);
	}
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnEnum8>(clickhouse::Type::CreateEnum8(enumItems));
    }
};

class ColumnEnum16 : public ColumnType
{
private:
    std::shared_ptr<clickhouse::ColumnEnum16> column;
    std::vector<clickhouse::Type::EnumItem> enumItems;
public:
    void __construct(Php::Parameters &params)
    {
	for (std::pair<Php::Value, Php::Value> e: params[0]) {
	    enumItems.push_back(std::pair(e.first.stringValue(), e.second.numericValue()));
	}
	column = std::make_shared<clickhouse::ColumnEnum16>(clickhouse::Type::CreateEnum16(enumItems));
    }

    void append(const Php::Value &data, const Php::Value &index, bool columnar) override
    {
	nestedForeach(data, index, columnar, [&](zval *val) {
	    append(val);
	});
    }

    void append(zval *val) override
    {
	if (Z_TYPE_P(val) == IS_STRING) {
	    column->Append(zval_get_string(val)->val);
	} else {
	    column->Append(zval_get_long(val), false);
	}
    }

    void clear() override {
	column->Clear();
    }

    clickhouse::ColumnRef currentColumn() override
    {
	return column;
    }

    clickhouse::ColumnRef emptyColumn() override
    {
	return std::make_shared<clickhouse::ColumnEnum8>(clickhouse::Type::CreateEnum16(enumItems));
    }
};

using ColumnInt8 = VectorLong<std::int8_t>;
using ColumnInt16 = VectorLong<std::int16_t>;
using ColumnInt32 = VectorLong<std::int32_t>;
using ColumnInt64 = VectorLong<std::int64_t>;

using ColumnUInt8 = VectorLong<std::uint8_t>;
using ColumnUInt16 = VectorLong<std::uint16_t>;
using ColumnUInt32 = VectorLong<std::uint32_t>;
using ColumnUInt64 = VectorLong<std::uint64_t>;

using ColumnFloat32 = VectorDouble<float>;
using ColumnFloat64 = VectorDouble<double>;

using ColumnIPv6 = ColumnAsString<clickhouse::ColumnIPv6>;
using ColumnString = ColumnAsString<clickhouse::ColumnString>;

using ColumnDate = ColumnAsLong<clickhouse::ColumnDate>;
using ColumnDateTime = ColumnAsLong<clickhouse::ColumnDateTime>;

int nestedLength(const Php::Value &data, const Php::Value &index, bool columnar)
{
    if (columnar) {
	zval *item;
	auto name = zend_string_init(index.stringValue().data(), index.stringValue().size(), 0);
	auto column = zend_hash_find(Z_ARRVAL_P(data._val), name);
        auto ht = Z_ARRVAL_P(column);
	zend_string_release(name);
	return zend_hash_num_elements(ht);
    } else {
	return data.length();
    }
}

void nestedForeach(const Php::Value &data, const Php::Value &index, bool columnar, ZvalCallback callback)
{
    if (columnar) {
	zval *item;
	auto name = zend_string_init(index.stringValue().data(), index.stringValue().size(), 0);
	auto column = zend_hash_find(Z_ARRVAL_P(data._val), name);
	auto ht = Z_ARRVAL_P(column);

	ZEND_HASH_FOREACH_VAL(ht, item) {
	    callback(item);
	}
	ZEND_HASH_FOREACH_END();
	zend_string_release(name);
    } else if (index.isString()) {
	zval *item, *row, *rv;
	auto ht = Z_ARRVAL_P(data._val);
	auto name = zend_string_init(index.stringValue().data(), index.stringValue().size(), 0);
	zend_string_hash_func(name);
	ZEND_HASH_FOREACH_VAL(ht, row) {
	    item = _zend_hash_find_known_hash(Z_ARRVAL_P(row), name);
	    callback(item);
	}
	ZEND_HASH_FOREACH_END();
	zend_string_release(name);
    } else if (index.isScalar()) {
	zval *item, *row, *rv;
	auto ht = Z_ARRVAL_P(data._val);
	auto idx = index.numericValue();

	ZEND_HASH_FOREACH_VAL(ht, row) {
	    item = zend_hash_index_find(Z_ARRVAL_P(row), idx);
	    callback(item);
	}
	ZEND_HASH_FOREACH_END();
    }
}

void registerColumns(Php::Namespace &ns)
{
    Php::Class<ColumnInt8> columnInt8("ColumnInt8");
    ns.add(columnInt8);
    Php::Class<ColumnInt16> columnInt16("ColumnInt16");
    ns.add(columnInt16);
    Php::Class<ColumnInt32> columnInt32("ColumnInt32");
    ns.add(columnInt32);
    Php::Class<ColumnInt64> columnInt64("ColumnInt64");
    ns.add(columnInt64);

    Php::Class<ColumnUInt8> columnUInt8("ColumnUInt8");
    ns.add(columnUInt8);
    Php::Class<ColumnUInt16> columnUInt16("ColumnUInt16");
    ns.add(columnUInt16);
    Php::Class<ColumnUInt32> columnUInt32("ColumnUInt32");
    ns.add(columnUInt32);
    Php::Class<ColumnUInt64> columnUInt64("ColumnUInt64");
    ns.add(columnUInt64);

    Php::Class<ColumnFloat32> columnFloat32("ColumnFloat32");
    ns.add(columnFloat32);
    Php::Class<ColumnFloat64> columnFloat64("ColumnFloat64");
    ns.add(columnFloat64);

    Php::Class<ColumnString> columnString("ColumnString");
    REGISTER_METHOD(columnString, ColumnString, __construct);
    ns.add(columnString);

    Php::Class<ColumnFixedString> columnFixedString("ColumnFixedString");
    REGISTER_METHOD(columnFixedString, ColumnFixedString, __construct);
    ns.add(columnFixedString);

    Php::Class<ColumnIPv4> columnIPv4("ColumnIPv4");
    REGISTER_METHOD(columnIPv4, ColumnIPv4, __construct);
    ns.add(columnIPv4);
    Php::Class<ColumnIPv6> columnIPv6("ColumnIPv6");
    REGISTER_METHOD(columnIPv6, ColumnIPv6, __construct);
    ns.add(columnIPv6);

    Php::Class<ColumnDecimal> columnDecimal("ColumnDecimal");
    REGISTER_METHOD(columnDecimal, ColumnDecimal, __construct);
    ns.add(columnDecimal);

    Php::Class<ColumnDecimal32> columnDecimal32("ColumnDecimal32");
    REGISTER_METHOD(columnDecimal32, ColumnDecimal32, __construct);
    ns.add(columnDecimal);

    Php::Class<ColumnDecimal64> columnDecimal64("ColumnDecimal64");
    REGISTER_METHOD(columnDecimal64, ColumnDecimal64, __construct);
    ns.add(columnDecimal);

    Php::Class<ColumnDecimal128> columnDecimal128("ColumnDecimal128");
    REGISTER_METHOD(columnDecimal128, ColumnDecimal128, __construct);
    ns.add(columnDecimal);

    Php::Class<ColumnDate> columnDate("ColumnDate");
    REGISTER_METHOD(columnDate, ColumnDate, __construct);
    ns.add(columnDate);

    Php::Class<ColumnDateTime> columnDateTime("ColumnDateTime");
    REGISTER_METHOD(columnDateTime, ColumnDateTime, __construct);
    ns.add(columnDateTime);

    Php::Class<ColumnEnum8> columnEnum8("ColumnEnum8");
    REGISTER_METHOD(columnEnum8, ColumnEnum8, __construct);
    ns.add(columnEnum8);

    Php::Class<ColumnEnum16> columnEnum16("ColumnEnum16");
    REGISTER_METHOD(columnEnum16, ColumnEnum16, __construct);
    ns.add(columnEnum16);

    // Php::Class<ColumnDateTime64> columnDateTime64("ColumnDateTime64");
    // REGISTER_METHOD(columnDateTime64, ColumnDateTime64, __construct);
    // ns.add(columnDateTime64);

    Php::Class<ColumnNullable> nullable("ColumnNullable");
    REGISTER_METHOD(nullable, ColumnNullable, __construct);
    ns.add(nullable);

    Php::Class<ColumnTuple> tuple("ColumnTuple");
    REGISTER_METHOD(tuple, ColumnTuple, __construct);
    ns.add(tuple);

    Php::Class<ColumnArray> array("ColumnArray");
    REGISTER_METHOD(array, ColumnArray, __construct);
    ns.add(array);

    ns.add(nullable);
}
