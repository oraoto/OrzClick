<?php

include_once __DIR__ . '/config.php';

$client = orzclickClient();

$client->execute('truncate table insert_test');
