<?php

include_once __DIR__ . '/config.php';

$client = orzclickClient();

$client->execute('DROP TABLE IF EXISTS `select_test`');

$client->execute('
  CREATE TABLE IF NOT EXISTS `select_test` (
   `u8`  UInt8,
   `u16` UInt16,
   `u32` UInt32,
   `u64` UInt64,

   `i8`  Int8,
   `i16` Int16,
   `i32` Int32,
   `i64` Int64,

   `f32` Float32,
   `f64` Float64
  ) ENGINE = MergeTree ORDER BY u8;
');

$client->execute('
  CREATE TABLE IF NOT EXISTS `insert_test` (
   `u8`  UInt8,
   `u16` UInt16,
   `u32` UInt32,
   `u64` UInt64,

   `i8`  Int8,
   `i16` Int16,
   `i32` Int32,
   `i64` Int64,

   `f32` Float32,
   `f64` Float64
  ) ENGINE = Null;
');

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


for ($j = 1; $j <= 10; $j++) {
    $data = [];
    for ($i = 1; $i <= 10000; $i++) {
        $data['u8'][]   = $j * $i;
        $data['u16'][]  = $j * $i * 2;
        $data['u32'][]  = $j * $i * 3;
        $data['u64'][]  = $j * $i * 4;

        $data['i8'][]   = $j * $i;
        $data['i16'][]  = $j * $i * -2;
        $data['i32'][]  = $j * $i * -3;
        $data['i64'][]  = $j * $i * -4;

        $data['f32'][]  = $j * $i * 5;
        $data['f64'][]  = $j * $i * 6;
    }
    $client->insertColumnar('default.select_test', $columns, $data);
}
