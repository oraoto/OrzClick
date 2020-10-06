<?php

include_once __DIR__ . '/config.php';

$limit = $argv[1];
$loop  = $argv[2];

$client = orzclickClient();

$query = "select i8,i16,i32,i64,u8,u16,u32,u64 from select_test limit $limit";

for ($i = 0; $i < $loop; $i++) {
    $client->select($query);
}
