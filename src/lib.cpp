#include "phpcpp.h"
#include "ClientOptions.h"
#include "Client.h"
#include "Columns.h"

using namespace clickhouse;

extern "C" {
    PHPCPP_EXPORT void *get_module() {

	// create extension
	static Php::Extension extension("orzclick", "1.0");

	// create namespace
	Php::Namespace ns("OrzClick");

	registerClientOptions(ns);
	registerClient(ns);
	registerColumns(ns);

	// add extension and return a module
	extension.add(ns);
	return extension.module();
    }
}
