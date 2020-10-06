<?php

include_once __DIR__ . '/config.php';

$insert = $argv[1];
$loop   = $argv[2];

$client = orzclickClient();

$columns = [
    'u8'  => new OrzClick\ColumnUInt8(),
    'u16' => new OrzClick\ColumnUInt16(),
    'u32' => new OrzClick\ColumnUInt32(),
    'u64' => new OrzClick\ColumnUInt64(),

    'i8'  => new OrzClick\ColumnInt8(),
    'i16' => new OrzClick\ColumnInt16(),
    'i32' => new OrzClick\ColumnInt32(),
    'i64' => new OrzClick\ColumnInt64(),

    'f32' => new OrzClick\ColumnFloat32(),
    'f64' => new OrzClick\ColumnFloat64()
];

$data = [];
for ($i = 1; $i <= $insert; $i++) {
    $data[] = [
        'u8'  => $i,
        'u16' => $i * 2,
        'u32' => $i * 3,
        'u64' => $i * 4,

        'i8'  => $i,
        'i16' => $i * -2,
        'i32' => $i * -3,
        'i64' => $i * -4,

        'f32' => $i * 5,
        'f64' => $i * 6,
    ];
}

for ($j = 0; $j < $loop; $j++) {
    $client->insert('default.insert_test', $columns, $data);
}
