<?php

use OrzClick\ColumnUInt8;

include __DIR__ . "/../bench/config.php";

$client = orzclickClient();


$client->execute('drop table if exists default.insert_tuple');

$client->execute('create table default.insert_tuple (
    a Tuple(Int16, Nullable(UInt8)),
    b Tuple(Int16, Array(UInt8)),
    c Tuple(Tuple(UInt8, UInt8), UInt8),
    d Tuple(Int16, UInt8),
    e Tuple(Tuple(UInt8, UInt8), Tuple(UInt8, UInt8)),
    f Tuple(Tuple(Tuple(UInt8, UInt8), UInt8), UInt8)
 ) Engine = Memory');


$columns = [
    'a' => new OrzClick\ColumnTuple(
        new OrzClick\ColumnInt16(),
        new OrzClick\ColumnNullable(new OrzClick\ColumnUInt8()),

    ),
    'b' => new OrzClick\ColumnTuple(
        new OrzClick\ColumnInt16(),
        new OrzClick\ColumnArray(new OrzClick\ColumnUInt8()),
    ),
    'c' => new OrzClick\ColumnTuple(
        new OrzClick\ColumnTuple(new OrzClick\ColumnUInt8(), new OrzClick\ColumnUInt8()),
        new OrzClick\ColumnUInt8(),
    ),
    'd' => new OrzClick\ColumnTuple(
        new OrzClick\ColumnInt16(),
        new OrzClick\ColumnUInt8(),
    ),
    'e' => new OrzClick\ColumnTuple(
        new OrzClick\ColumnTuple(
            new OrzClick\ColumnUInt8(),
            new OrzClick\ColumnUInt8()),
        new OrzClick\ColumnTuple(
            new OrzClick\ColumnUInt8(),
            new OrzClick\ColumnUInt8())
    ),
    'f' => new OrzClick\ColumnTuple(
        new OrzClick\ColumnTuple(
            new OrzClick\ColumnTuple(
                new OrzClick\ColumnUInt8(),
                new OrzClick\ColumnUInt8()),
            new ColumnUInt8),
        new ColumnUInt8),
];

$data = [
    [
        'a' => [1, null],
        'b' => [2, [2,3,4]],
        'c' => [[4,5], 3],
        'd' => [10, 11],
        'e' => [[15, 16], [17, 18]],
        'f' => [[[1,2], 3], 4]
    ],
    [
        'a' => [3, 4],
        'b' => [4, []],
        'c' => [[6, 7], 5],
        'd' => [12, 13],
        'e' => [[25, 26], [27, 28]],
        'f' => [[[2,3], 4], 5]
    ],
];

$client->insert('default.insert_tuple', $columns, $data);

$result = $client->select('select * from default.insert_tuple');

echo json_encode($result) . PHP_EOL;
