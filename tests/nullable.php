<?php

use OrzClick\ColumnFloat32;
use OrzClick\ColumnNullable;
use OrzClick\ColumnUInt8;

include __DIR__ . "/../bench/config.php";

$client = orzclickClient();


$client->execute('drop table if exists default.insert_nullable');

$client->execute('create table default.insert_nullable (
    i UInt8,
    a Nullable(UInt8)
 ) Engine = MergeTree Order by i');


$columns = [
    'a' => new ColumnNullable(new OrzClick\ColumnUInt8()),
];

$data = [
    ['a' => 1],
    ['a' => null],
    ['a' => 5],
];

$client->insert('default.insert_nullable', $columns, $data);


$result = $client->select('select a from default.insert_nullable');
echo json_encode($result);
