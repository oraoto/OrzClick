#include "clickhouse/client.h"
#include "phpcpp.h"
#include "common.h"

#define CLIENT_OPTIONS_CLASS_NAME "ClientOptions"

class ClientOptions : public Php::Base {

public:
    clickhouse::ClientOptions clientOptions;

    ~ClientOptions() {}

    void __construct(Php::Parameters &params);

    DECLARE_METHOD(SetHost);

    DECLARE_METHOD(SetPort);

    DECLARE_METHOD(SetDefaultDatabase);

    DECLARE_METHOD(SetUser);

    DECLARE_METHOD(SetPassword);

    DECLARE_METHOD(SetRethrowException);

    DECLARE_METHOD(SetPingBeforeQuery);

    DECLARE_METHOD(SetSendRetries);

    DECLARE_METHOD(SetRetryTimeout);

    DECLARE_METHOD(SetCompressionMethod);

    DECLARE_METHOD(TcpKeepAlive);

    DECLARE_METHOD(SetTcpKeepAliveIdle);

    DECLARE_METHOD(SetTcpKeepAliveInterval);

    DECLARE_METHOD(SetTcpKeepAliveCount);

    DECLARE_METHOD(TcpNoDelay);
};

void registerClientOptions(Php::Namespace &ns);
