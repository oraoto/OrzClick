<?php

include __DIR__ . "/../bench/config.php";

$client = orzclickClient();


$client->execute('drop table if exists default.insert_array');

$client->execute('create table default.insert_array (
    a Array(Array(UInt8))
 ) Engine = Memory');

// $client->execute('create table default.insert_array (
//     a Array(UInt8)
//  ) Engine = Memory');

$columns = [
    'a' => new OrzClick\ColumnArray(new OrzClick\ColumnArray(new OrzClick\ColumnUInt8())),
    // 'a' => new OrzClick\ColumnArray(new OrzClick\ColumnUInt8())
];

$data = [
    ['a' => [[6,2], [3,4], [1], [2]]],
    // ['a' => [1,2,3,4]]
];

$client->insert('default.insert_array', $columns, $data);

$result = $client->select('select * from default.insert_array');
var_dump($result);
echo json_encode($result);
