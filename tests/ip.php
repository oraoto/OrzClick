<?php

use OrzClick\ColumnIPv4;
use OrzClick\ColumnIPv6;

include __DIR__ . "/../bench/config.php";

$client = orzclickClient();


$client->execute('drop table if exists default.insert_ip');

$client->execute('create table default.insert_ip (
    v4 IPv4,
    v6 IPv6
 ) Engine = Memory');


$columns = [
    'v4' => new ColumnIPv4(),
    'v6' => new ColumnIPv6()
];

$data = [
    [
        'v4' => "127.0.0.1",
        'v6' => "::"
    ],
    [
        'v4' => ip2long("192.168.0.1"),
        'v6' => "::"
    ]
];

$client->insert('default.insert_ip', $columns, $data);

$result = $client->select('select * from default.insert_ip');
var_dump($result);
