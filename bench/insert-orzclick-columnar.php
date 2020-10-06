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

for ($i = 1; $i <= $insert; $i++) {
    $data['u8'][]   = $i;
    $data['u16'][]  = $i * 2;
    $data['u32'][]  = $i * 3;
    $data['u64'][]  = $i * 4;

    $data['i8'][]   = $i;
    $data['i16'][]  = $i * -2;
    $data['i32'][]  = $i * -3;
    $data['i64'][]  = $i * -4;

    $data['f32'][]  = $i * 5;
    $data['f64'][]  = $i * 6;
}

// $client->insertColumnar('default.insert_test', $columns, $data);

for ($j = 0; $j < $loop; $j++) {
    $client->insertColumnar('default.insert_test', $columns, $data);
}
