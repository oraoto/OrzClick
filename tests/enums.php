<?php

include __DIR__ . "/../bench/config.php";

$client = orzclickClient();


$client->execute('drop table if exists default.insert_enums');

$client->execute("create table default.insert_enums (
    a Enum8('A' = 1, 'B' = 2)
 ) Engine = Memory");


$columns = [
    'a' => new OrzClick\ColumnEnum8(['A' => 1, 'B' => 2]),
];

$data = [
    ['a' => 1],
    ['a' => 2],
    ['a' => 'A'],
    ['a' => 'B'],
];

$client->insert('default.insert_enums', $columns, $data);
