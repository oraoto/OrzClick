<?php

include_once __DIR__ . '/config.php';

$insert = $argv[1];
$loop   = $argv[2];

$client = seasclickClient();

$columns = [
    'u8',
    'u16',
    'u32',
    'u64',

    'i8',
    'i16',
    'i32',
    'i64',

    'f32',
    'f64',
];

for ($i = 1; $i <= $insert; $i++) {
    $data[] = [
       $i,
       $i * 2,
       $i * 3,
       $i * 4,

       $i,
       $i * -2,
       $i * -3,
       $i * -4,

       $i * 5,
       $i * 6,
    ];
}

for ($j = 0; $j < $loop; $j++) {
    $client->insert('default.insert_test', $columns, $data);
}
