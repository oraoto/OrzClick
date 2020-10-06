#pragma once
#include "clickhouse/columns/column.h"
#include "phpcpp.h"
#include "zend_types.h"

// TODO: merge with FetchColumn ?
class ColumnType : public Php::Base {
public:
    /// @params data input data
    /// @params index if data is in columar format, index is the index of a column, otherwise it the index of a data item inside a column
    /// @params columar is data in columar format
    virtual void append(const Php::Value &data, const Php::Value &index, bool columnar) = 0;

    /// append a single column item to current building column
    virtual void append(zval *val) = 0;

    /// clear current building column
    virtual void clear() = 0;

    /// return current building column
    virtual clickhouse::ColumnRef currentColumn() = 0;

    /// return an empty column (for using in nested types)
    virtual clickhouse::ColumnRef emptyColumn() = 0;
};

class FetchColumn {
public:
    /// fetch column into array
    virtual void fetch(clickhouse::ColumnRef column, size_t nrows, std::string columnName, std::vector<Php::Value>& result) = 0;

    /// fetch column item into associative array, or array when key is null
    virtual void fetch(clickhouse::ColumnRef column, size_t index, const char *key, size_t key_len, zval *val) = 0;

    virtual ~FetchColumn() = default;
};

std::unique_ptr<FetchColumn> createFetcher(clickhouse::ColumnRef columnRef);

void registerColumns(Php::Namespace &ns);
