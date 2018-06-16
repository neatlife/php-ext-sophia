<?php

var_dump($link = sp_open('/tmp/storage', 'test'));

var_dump(sp_set('name', 'suxiaolin', $link));

var_dump(sp_get('name', $link));
