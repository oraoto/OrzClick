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

// takes from clickhouse/ut/client_ut.cpp
std::string int128_to_string(clickhouse::Int128 value) {
    std::string result;
    const bool sign = value >= 0;

    if (!sign) {
	value = -value;
    }

    while (value) {
	result += static_cast<char>(value % 10) + '0';
	value /= 10;
    }

    if (result.empty()) {
	result = "0";
    } else if (!sign) {
	result.push_back('-');
    }

    std::reverse(result.begin(), result.end());

    return result;
};

template<typename T>
class FetchVectorLong : public FetchColumn
{
public:
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto columnNameSize = columnName.size();
	auto col = *(column->As<clickhouse::ColumnVector<T>>().get());
	for (size_t j = 0; j < nrows; j++) {
	    add_assoc_long_ex(result[j]._val, columnName.c_str(), columnNameSize, col[j]);
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	long r = column->As<clickhouse::ColumnVector<T>>()->At(index);
	if (key) {
	    add_assoc_long_ex(val, key, key_len, r);
	} else {
	    add_index_long(val, key_len, r);
	}
    }
};

template<typename T>
class FetchVectorDouble : public FetchColumn
{
public:
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto columnNameSize = columnName.size();
	auto col = *(column->As<clickhouse::ColumnVector<T>>().get());
	for (size_t j = 0; j < nrows; j++) {
	    add_assoc_double_ex(result[j]._val, columnName.c_str(), columnNameSize, col[j]);
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto r = column->As<clickhouse::ColumnVector<T>>()->At(index);
	if (key) {
	    add_assoc_double_ex(val, key, key_len, r);
	} else {
	    add_index_double(val, key_len, r);
	}
    }
};

class FetchColumnNullable : public FetchColumn
{
private:
    std::unique_ptr<FetchColumn> nested;

public:
    FetchColumnNullable(clickhouse::ColumnRef column) {
	nested = createFetcher(column->As<clickhouse::ColumnNullable>()->Nested());
    }

    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto columnNameSize = columnName.size();
	auto col = *(column->As<clickhouse::ColumnNullable>().get());
	for (size_t j = 0; j < nrows; j++) {
	    if (col.IsNull(j)) {
		add_assoc_null_ex(result[j]._val, columnName.c_str(), columnNameSize);
	    } else {
		nested->fetch(col.Nested(), j, columnName.c_str(), columnNameSize, result[j]._val);
	    }
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto col = column->As<clickhouse::ColumnNullable>();
	if (col->IsNull(index)) {
	    if (key) {
		add_assoc_null_ex(val, key, key_len);
	    } else {
		add_index_null(val, key_len);
	    }
	} else {
	    nested->fetch(col->Nested(), index, key, key_len, val);
	}
    }

};

class FetchColumnTuple : public FetchColumn
{
public:
    std::vector<std::unique_ptr<FetchColumn>> nested;

    FetchColumnTuple(clickhouse::ColumnRef c) {
	auto tupleColumn = *(c->As<clickhouse::ColumnTuple>().get());
	for (size_t i = 0; i < tupleColumn.TupleSize(); i++) {
	    nested.push_back(createFetcher(tupleColumn[i]));
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	std::vector<Php::Value> tupleResult(nrows);
	for (size_t j = 0; j < nrows; ++j) {
	    tupleResult[j] = Php::Array();
	    add_assoc_zval_ex(result[j]._val, columnName.c_str(), columnName.size(), tupleResult[j]._val);
	    Z_TRY_ADDREF_P(tupleResult[j]._val);
	}

	auto col = *(column->As<clickhouse::ColumnTuple>().get());

	auto i = 0;
	for (const auto &n: nested) {
	    auto c = col[i];
	    for (size_t j = 0; j < nrows; j++) {
		n->fetch(c, j, 0, i, tupleResult[j]._val);
	    }
	    i++;
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto col = *(column->As<clickhouse::ColumnTuple>().get());
	auto arr = Php::Array();
	auto i = 0;
	for (auto &n: nested) {
	    n->fetch(col[i], index, 0, i, arr._val);
	    i++;
	}
	if (key) {
	    add_assoc_zval_ex(val, key, key_len, arr._val);
	} else {
	    add_index_zval(val, key_len, arr._val);
	}
	Z_TRY_ADDREF_P(arr._val);
    }
};

class FetchColumnArray : public FetchColumn
{
private:
    std::unique_ptr<FetchColumn> nested;

public:
    FetchColumnArray(clickhouse::ColumnRef c)
    {
	nested = createFetcher(c->As<clickhouse::ColumnArray>()->GetAsColumn(0));
    }

    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto col = column->As<clickhouse::ColumnArray>();
	for (size_t j = 0; j < nrows; j++) {
	    auto arr = Php::Array();
            auto nestedColumn = col->GetAsColumn(j);
	    size_t size = nestedColumn->Size();
            for (size_t i = 0; i < size; i++) {
		nested->fetch(nestedColumn, i, 0, i, arr._val);
	    }
	    add_assoc_zval_ex(result[j]._val, columnName.c_str(), columnName.size(), arr._val);
	    Z_TRY_ADDREF_P(arr._val);
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto nestedColumn = column->As<clickhouse::ColumnArray>()->GetAsColumn(index);
	auto arr = Php::Array();
	for (size_t i = 0; i < nestedColumn->Size(); i++) {
	    nested->fetch(nestedColumn, i, 0, i, arr._val);
	}
	if (key) {
	    add_assoc_zval_ex(val, key, key_len, arr._val);
	} else {
	    add_index_zval(val, key_len, arr._val);
	}
	Z_TRY_ADDREF_P(arr._val);
    }
};

class FetchColumnDecimal : public FetchColumn
{
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto columnNameSize = columnName.size();
	auto col = *(column->As<clickhouse::ColumnDecimal>().get());
	for (size_t j = 0; j < nrows; j++) {
	    auto str = int128_to_string(col.At(j));
	    add_assoc_string_ex(result[j]._val, columnName.c_str(), columnNameSize, str.c_str());
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto r = column->As<clickhouse::ColumnDecimal>()->At(index);
	auto str = int128_to_string(r);
	if (key) {
	    add_assoc_string_ex(val, key, key_len, str.c_str());
	} else {
	    add_index_string(val, key_len, str.c_str());
	}
    }
};

template<typename T>
class FetchColumnAsString : public FetchColumn
{
public:
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto col = column->As<T>().get();
	for (size_t j = 0; j < nrows; j++) {
	    add_assoc_string_ex(result[j]._val, columnName.c_str(), columnName.size(), col->At(j).data());
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto str = column->As<T>()->At(index);
	if (key) {
	    add_assoc_string_ex(val, key, key_len, str.data());
	} else {
	    add_index_string(val, key_len, str.data());
	}
    }
};

template<typename T>
class FetchColumnIP : public FetchColumn
{
public:
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto col = *(column->As<T>().get());
	for (size_t j = 0; j < nrows; j++) {
	    add_assoc_string_ex(result[j]._val, columnName.c_str(), columnName.size(), col.AsString(j).c_str());
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto addr = column->As<T>()->AsString(index);
	if (key) {
	    add_assoc_string_ex(val, key, key_len, addr.c_str());
	} else {
	    add_index_string(val, key_len, addr.c_str());
	}
    }
};

template<typename T>
class FetchColumnAsLong : public FetchColumn
{
public:
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto columnNameSize = columnName.size();
	auto col = *(column->As<T>().get());
	for (size_t j = 0; j < nrows; j++) {
	    add_assoc_long_ex(result[j]._val, columnName.c_str(), columnNameSize, col.At(j));
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto r = column->As<T>()->At(index);
	if (key) {
	    add_assoc_long_ex(val, key, key_len, r);
	} else {
	    add_index_long(val, key_len, r);
	}
    }
};

template<typename T>
class FetchColumnEnum : public FetchColumn
{
public:
    void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) override
    {
	auto col = *(column->As<T>().get());
	for (size_t j = 0; j < nrows; j++) {
	    add_assoc_long_ex(result[j]._val, columnName.c_str(), columnName.size(), col[j]);
	}
    }

    void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) override
    {
	auto e = column->As<T>()->At(index);
	if (key) {
	    add_assoc_long_ex(val, key, key_len, e);
	} else {
	    add_index_long(val, key_len, e);
	}
    }
};

std::unique_ptr<FetchColumn> createFetcher(clickhouse::ColumnRef columnRef)
{
    switch (columnRef->GetType().GetCode()) {
    case clickhouse::Type::Int8:
	return std::make_unique<FetchVectorLong<std::int8_t>>();
    case clickhouse::Type::Int16:
	return std::make_unique<FetchVectorLong<std::int16_t>>();
    case clickhouse::Type::Int32:
	return std::make_unique<FetchVectorLong<std::int32_t>>();
    case clickhouse::Type::Int64:
	return std::make_unique<FetchVectorLong<std::int64_t>>();
    case clickhouse::Type::UInt8:
	return std::make_unique<FetchVectorLong<std::uint8_t>>();
    case clickhouse::Type::UInt16:
	return std::make_unique<FetchVectorLong<std::uint16_t>>();
    case clickhouse::Type::UInt32:
	return std::make_unique<FetchVectorLong<std::uint32_t>>();
    case clickhouse::Type::UInt64:
	return std::make_unique<FetchVectorLong<std::uint64_t>>();
    case clickhouse::Type::Float32:
	return std::make_unique<FetchVectorDouble<float>>();
    case clickhouse::Type::Float64:
	return std::make_unique<FetchVectorDouble<double>>();
    case clickhouse::Type::IPv4:
	return std::make_unique<FetchColumnIP<clickhouse::ColumnIPv4>>();
    case clickhouse::Type::IPv6:
	return std::make_unique<FetchColumnIP<clickhouse::ColumnIPv6>>();
    case clickhouse::Type::Enum8:
	return std::make_unique<FetchColumnEnum<clickhouse::ColumnEnum8>>();
    case clickhouse::Type::Enum16:
	return std::make_unique<FetchColumnEnum<clickhouse::ColumnEnum16>>();
    case clickhouse::Type::Decimal:
    case clickhouse::Type::Decimal32:
    case clickhouse::Type::Decimal64:
    case clickhouse::Type::Decimal128:
	return std::make_unique<FetchColumnDecimal>();
    case clickhouse::Type::String:
	return std::make_unique<FetchColumnAsString<clickhouse::ColumnString>>();
    case clickhouse::Type::FixedString:
	return std::make_unique<FetchColumnAsString<clickhouse::ColumnFixedString>>();
    case clickhouse::Type::Date:
	return std::make_unique<FetchColumnAsLong<clickhouse::ColumnDate>>();
    case clickhouse::Type::DateTime:
	return std::make_unique<FetchColumnAsLong<clickhouse::ColumnDateTime>>();
    case clickhouse::Type::DateTime64:
	return std::make_unique<FetchColumnAsLong<clickhouse::ColumnDateTime64>>();
    case clickhouse::Type::Nullable:
	return std::make_unique<FetchColumnNullable>(columnRef);
    case clickhouse::Type::Tuple:
	return std::make_unique<FetchColumnTuple>(columnRef);
    case clickhouse::Type::Array:
	return std::make_unique<FetchColumnArray>(columnRef);
    case clickhouse::Type::Void:
    case clickhouse::Type::UUID:
    case clickhouse::Type::Int128:
    case clickhouse::Type::LowCardinality:
	throw new Php::Exception("Data type not supported");
    }
}
