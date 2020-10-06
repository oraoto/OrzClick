<?php

include __DIR__ . "/../bench/config.php";

$client = orzclickClient();


$client->execute('drop table if exists default.insert_string');

$client->execute('create table default.insert_string (
    str String,
    fstr FixedString(2)
 ) Engine = Memory');


$columns = [
    'str' => new OrzClick\ColumnString(),
    'fstr' => new OrzClick\ColumnFixedString(2),
];

$data = [
    [
        'str' => md5(1),
        'fstr' => 'ab',
    ],
    [
        'str' => md5(2),
        'fstr' => 'cd',
    ],
    [
        'str' => md5(3),
        'fstr' => 'ef',
    ]
];

$client->insert('default.insert_string', $columns, $data);
