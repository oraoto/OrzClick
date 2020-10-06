#include "ClientOptions.h"
#include "clickhouse/client.h"
#include <chrono>

void ClientOptions::__construct(Php::Parameters &params) {
    this->clientOptions = clickhouse::ClientOptions();
}

Php::Value ClientOptions::SetHost(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetHost(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetPort(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetPort((int)params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetDefaultDatabase(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetDefaultDatabase(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetUser(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetUser(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetPassword(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetPassword(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetRethrowException(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetRethrowException(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetPingBeforeQuery(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetPingBeforeQuery(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetSendRetries(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetSendRetries((int)params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetRetryTimeout(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetRetryTimeout(std::chrono::seconds(params[0]));
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetCompressionMethod(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetCompressionMethod(clickhouse::CompressionMethod((int)params[0]));
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::TcpKeepAlive(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.TcpKeepAlive(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetTcpKeepAliveIdle(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetTcpKeepAliveIdle(std::chrono::seconds(params[0]));
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetTcpKeepAliveInterval(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetTcpKeepAliveInterval(std::chrono::seconds(params[0]));
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::SetTcpKeepAliveCount(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.SetTcpKeepAliveCount((int)params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

Php::Value ClientOptions::TcpNoDelay(Php::Parameters &params) {
    this->clientOptions = this->clientOptions.TcpNoDelay(params[0]);
    return Php::Object(CLIENT_OPTIONS_CLASS_NAME, this);
}

void registerClientOptions(Php::Namespace &ns)
{
    Php::Class<ClientOptions> cls(CLIENT_OPTIONS_CLASS_NAME);

    REGISTER_METHOD(cls, ClientOptions, __construct);
    REGISTER_METHOD(cls, ClientOptions, SetHost);
    REGISTER_METHOD(cls, ClientOptions, SetPort);
    REGISTER_METHOD(cls, ClientOptions, SetDefaultDatabase);
    REGISTER_METHOD(cls, ClientOptions, SetUser);
    REGISTER_METHOD(cls, ClientOptions, SetPassword);
    REGISTER_METHOD(cls, ClientOptions, SetRethrowException);
    REGISTER_METHOD(cls, ClientOptions, SetPingBeforeQuery);
    REGISTER_METHOD(cls, ClientOptions, SetSendRetries);
    REGISTER_METHOD(cls, ClientOptions, SetRetryTimeout);
    REGISTER_METHOD(cls, ClientOptions, SetCompressionMethod);
    REGISTER_METHOD(cls, ClientOptions, TcpKeepAlive);
    REGISTER_METHOD(cls, ClientOptions, SetTcpKeepAliveIdle);
    REGISTER_METHOD(cls, ClientOptions, SetTcpKeepAliveInterval);
    REGISTER_METHOD(cls, ClientOptions, SetTcpKeepAliveCount);
    REGISTER_METHOD(cls, ClientOptions, TcpNoDelay);

    ns.add(cls);
}
