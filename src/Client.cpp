#include "Client.h"
#include "ClientOptions.h"
#include "Columns.h"
#include "clickhouse/block.h"
#include "clickhouse/client.h"
#include "clickhouse/columns/array.h"
#include "clickhouse/columns/column.h"
#include "clickhouse/columns/date.h"
#include "clickhouse/columns/decimal.h"
#include "clickhouse/columns/enum.h"
#include "clickhouse/columns/ip4.h"
#include "clickhouse/columns/nullable.h"
#include "clickhouse/columns/numeric.h"
#include "clickhouse/columns/string.h"
#include "clickhouse/columns/uuid.h"
#include "clickhouse/exceptions.h"
#include "clickhouse/types/types.h"
#include "zval.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <Zend/zend_API.h>
#include <Zend/zend_operators.h>
#include <Zend/zend_hash.h>
#include <arpa/inet.h>

void Client::__construct(Php::Parameters &params) {
    try {
	this->client = new clickhouse::Client(((ClientOptions *)(params[0].implementation()))->clientOptions);
    } catch (const std::exception &ex) {
	throw Php::Exception(ex.what());
    }
}

void Client::Execute(Php::Parameters &params) {
    std::string query = params[0];
    try {
	this->client->Execute(query);
    } catch (const clickhouse::ServerException &ex) {
	throw Php::Exception(ex.what());
    }
}

Php::Value Client::InsertColumnar(Php::Parameters &params) {
    std::string table = params[0];
    Php::Value data   = params[2];

    clickhouse::Block block;
    for (std::pair<Php::Value, Php::Value> col: params[1]) {
	auto column = dynamic_cast<ColumnType *>(col.second.implementation());
	column->clear();
	column->append(data, col.first, true);
	block.AppendColumn(col.first, column->currentColumn());
    }

    this->client->Insert(table, block);
    return true;
}

Php::Value Client::InsertIndexed(Php::Parameters &params) {
    std::string table	= params[0];
    Php::Value data = params[2];

    clickhouse::Block block;
    int i = 0;
    for (std::pair<Php::Value, Php::Value> col: params[1]) {
	auto column = dynamic_cast<ColumnType *>(col.second.implementation());
	column->clear();
        column->append(data, i, false);
	block.AppendColumn(col.first, column->currentColumn());
	i++;
    }
    this->client->Insert(table, block);
    return true;
}

Php::Value Client::Insert(Php::Parameters &params) {
    std::string table	= params[0];
    Php::Value data = params[2];

    clickhouse::Block block;
    for (std::pair<Php::Value, Php::Value> col: params[1]) {
	auto column = dynamic_cast<ColumnType *>(col.second.implementation());
	column->clear();
	column->append(data, col.first, false);
	block.AppendColumn(col.first, column->currentColumn());
    }
    this->client->Insert(table, block);
    return true;
}

Php::Value Client::Select(Php::Parameters &params) {
    std::string query = params[0];
    auto result = Php::Array();

    try {
	this->client->Select(query, [&result](const clickhouse::Block& block) {
	    auto nrows = block.GetRowCount();
	    if (!nrows) {
		return;
	    }

	    auto ncolums = block.GetColumnCount();

	    std::vector<Php::Value> blockResult(nrows);
	    for (size_t j = 0; j < nrows; ++j) {
		blockResult[j] = Php::Array();
	    }

	    for (size_t i = 0; i < ncolums; ++i) {
		auto columnName = block.GetColumnName(i);
		auto fetcher = createFetcher(block[i]);
                fetcher->fetch(block[i], nrows, columnName, blockResult);
	    }
	    for (size_t j = 0; j < nrows; ++j) {
		add_next_index_zval(result._val, blockResult[j]._val);
		Z_TRY_ADDREF_P(blockResult[j]._val);
	    }
	});
    } catch (const std::exception &ex) {
	throw Php::Exception(ex.what());
    }
    return result;
}

void Client::Ping(Php::Parameters &params)
{
    try {
	client->Ping();
    } catch (const std::exception &ex) {
	throw Php::Exception(ex.what());
    }
}

void registerClient(Php::Namespace &ns)
{
    Php::Class<Client> cls(CLIENT_CLASS_NAME);

    REGISTER_METHOD(cls, Client, __construct);
    REGISTER_METHOD(cls, Client, Select);
    REGISTER_METHOD(cls, Client, Execute);
    REGISTER_METHOD(cls, Client, Ping);

    cls.method<&Client::Insert>("Insert", {
	    Php::ByVal("table", Php::Type::String),
	    Php::ByVal("columns", Php::Type::Array),
	    Php::ByRef("data", Php::Type::Array),
    });
    cls.method<&Client::InsertIndexed>("InsertIndexed", {
	    Php::ByVal("table", Php::Type::String),
	    Php::ByVal("columns", Php::Type::Array),
	    Php::ByRef("data", Php::Type::Array),
    });
    cls.method<&Client::InsertColumnar>("InsertColumnar", {
	    Php::ByVal("table", Php::Type::String),
	    Php::ByVal("columns", Php::Type::Array),
	    Php::ByRef("data", Php::Type::Array),
    });
    ns.add(cls);
}
