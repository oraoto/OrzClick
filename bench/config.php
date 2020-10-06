<?php

// common clickhouse config
$config = [
    'host'        => '127.0.0.1',
    'port'        => 9000,
    'mysql_port'  => 9004,
    'user'        => 'default',
    'password'    => '',
    'database'    => 'default',
    'compression' => true,
    'nodelay'     => true,
];

function mysqlClient() {
    global $config;
    return new PDO("mysql:host=$config[host];port=$config[mysql_port];dbname=$config[database]", $config['user'], $config['password']);
}

function orzclickClient() {
    global $config;
    $options = (new OrzClick\ClientOptions())
             ->setHost($config['host'])
             ->setPort($config['port'])
             ->setUser($config['user'])
             ->setPassword($config['password'])
             ->setCompressionMethod($config['compression'] ? 1 : -1)
             ->TcpNoDelay($config['nodelay']);

    return new OrzClick\Client($options);
}

function seasclickClient() {
    global $config;
    return new SeasClick([
        'host'        => $config['host'],
        'port'        => $config['port'],
        'passwd'      => $config['password'],
        'user'        => $config['user'],
        'compression' => $config['compression'],
    ]);
}
