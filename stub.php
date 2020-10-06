<?php

namespace OrzClick;

class ClientOptions {
    /** @return ClientOptions */
    public function setHost(string $host) {}

    /** @return ClientOptions */
    public function setPort(int $number) {}

    /** @return ClientOptions */
    public function setUser(string $user) {}

    /** @return ClientOptions */
    public function setPassword(string $passwor) {}

    /** @return ClientOptions */
    public function setPingBeforeQuery(bool $ping) {}

    /** @return ClientOptions */
    public function SetDefaultDatabase(string $database) {}

    /** @return ClientOptions */
    public function SetRethrowException(bool $rethrow) {}

    /** @return ClientOptions */
    public function SetSendRetries(int $retries) {}

    /** @return ClientOptions */
    public function SetRetryTimeout(int $retry_timeout) {}

    /** @return ClientOptions */
    public function SetCompressionMethod(int $method) {}

    /** @return ClientOptions */
    public function TcpKeepAlive(bool $keppalive) {}

    /** @return ClientOptions */
    public function SetTcpKeepAliveIdle(int $idle) {}

    /** @return ClientOptions */
    public function SetTcpKeepAliveInterval(int $interval) {}

    /** @return ClientOptions */
    public function SetTcpKeepAliveCount($count) {}

    /** @return ClientOptions */
    public function TcpNoDelay(bool $keppalive) {}
}

class Client {
    public function __construct(ClientOptions $options) {}

    /** @return array */
    public function select(string $query) {}

    public function execute(string $query) {}

    public function insert(string $table, array $columns, array &$data) {}

    public function insertIndexed(string $table, array $columns, array &$data) {}

    public function insertColumnar(string $table, array $columns, array &$data) {}

    public function ping() {}
}

interface ColumnType {};

class ColumnNothing implements ColumnType {}

class ColumnInt8 implements ColumnType {}
class ColumnInt16 implements ColumnType {}
class ColumnInt32 implements ColumnType {}
class ColumnInt64 implements ColumnType {}

class ColumnUInt8 implements ColumnType {}
class ColumnUInt16 implements ColumnType {}
class ColumnUInt32 implements ColumnType {}
class ColumnUInt64 implements ColumnType {}

class ColumnFloat32 implements ColumnType {}
class ColumnFloat64 implements ColumnType {}

class ColumnDecimal32 implements ColumnType {
    public function __construct(int $scale) {}
}
class ColumnDecimal64 implements ColumnType {
    public function __construct(int $scale) {}
}
class ColumnDecimal128 implements ColumnType {
    public function __construct(int $scale) {}
}
class ColumnDecimal implements ColumnType {
    public function __construct(int $precision, int $scale) {}
}

class ColumnIPv4 implements ColumnType {}
class ColumnIPv6 implements ColumnType {}
// class ColumnUUID implements ColumnType {}

class ColumnDate     implements ColumnType {}
class ColumnDateTime implements ColumnType {}
// class ColumnDateTime64 implements ColumnType {}

class ColumnString implements ColumnType {}
class ColumnFixedString implements ColumnType {
    public function __construct(int $size) {}
}

class ColumnNullable implements ColumnType {
    public function __construct(ColumnType $nested) {}
}

class ColumnTuple implements ColumnType {
    /** @param ColumnType[] $nested **/
    public function __construct($nested) {}
}

class ColumnArray implements ColumnType {
    public function __construct(ColumnType $nested) {}
}

class ColumnEnum8 implements ColumnType  {
    public function __construct(array $array) {}
}

class ColumnEnum16 implements ColumnType  {
    public function __construct(array $array) {}
}
