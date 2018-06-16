<?php

var_dump(sp_open('/tmp/storage', 'test'));

var_dump(sp_begin());

var_dump(sp_set('name', 'xiaolin'));
var_dump(sp_get('name'));
var_dump(sp_delete('name'));

var_dump(sp_commit());
