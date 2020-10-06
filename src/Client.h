#include "clickhouse/client.h"
#include "phpcpp.h"
#include "common.h"

#define CLIENT_CLASS_NAME "Client"

class Client : public Php::Base {
public:
    clickhouse::Client *client;

    ~Client() {}

    void __construct(Php::Parameters &params);

    DECLARE_VOID_METHOD(Execute);

    DECLARE_VOID_METHOD(SelectBlocks);

    DECLARE_METHOD(Select);

    DECLARE_METHOD(Insert);
    DECLARE_METHOD(InsertColumnar);
    DECLARE_METHOD(InsertIndexed);

    DECLARE_VOID_METHOD(Ping);

    // DECLARE_METHOD(ResetConnection);

    // DECLARE_METHOD(GetServerInfo);
};

void registerClient(Php::Namespace &ns);
