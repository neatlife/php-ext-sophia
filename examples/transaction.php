<?php

var_dump($link = sp_open('/tmp/storage', 'test'));

var_dump($transaction = sp_begin());

var_dump(sp_set('name', 'xiaolin', $link));
var_dump(sp_get('name', $link));
var_dump(sp_delete('name', $link));

var_dump(sp_commit($transaction));
