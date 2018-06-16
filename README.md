# sophia Extension for PHP

Modern transactional key-value/row storage library binding write by c extension for php.

## Build from sources

``` bash
% git clone --recursive git@github.com:neatlife/php-ext-sophia.git
% cd php-ext-sophia
% phpize
% ./configure
% make
% make install
```

To use the system library

``` bash
% ./configure --with-sophia
```

## Configration

sophia.ini:

```
extension=sophia.so
```

## Function

* sp_open  — open an database for later use
* sp_set — add a key value pair to previous opened database
* sp_get — get value for specific key
* sp_delete — delete specific key with it's value
* sp_begin — begin a transaction
* sp_commit — commit a transaction

### sp_open

#### Description

resource $dblink **sp\_open** ( \$dbpath, \$dbname )

```php
$link = sp_open('/tmp/storage', 'test');
var_dump($link);
```

### sp_set

#### Description

bool  **sp\_set** ( \$key, \$value )

```php
var_dump(sp_set('name', 'suxiaolin', $link));
```

return true represent success or failure

### sp_get

#### Description

string $value **sp\_get** ( \$key )

```php
var_dump(sp_get('name', $link));
```

value for specific key, if key not exists will got null

### sp_delete

#### Description

bool **sp\_delete** ( \$key )

```php
var_dump(sp_delete('name', $link));
```

return true if delete successfully or false on error

### sp_begin

#### Description

resource **sp\_begin** (  )

```php
var_dump($transaction = sp_begin());
```

start a transaction

### sp_commit

#### Description

resource **sp\_commit** ( resource $transaction )

```php
var_dump(sp_commit($transaction));
```

commit a transaction

On success, sp_commit() returns 0. On error, it returns -1. On rollback 1 is returned, 2 on lock.

## Examples

```php
// set
var_dump($link = sp_open('/tmp/storage', 'test'));

var_dump(sp_set('name', 'suxiaolin', $link));

var_dump(sp_get('name', $link));
```

```php
// delete
var_dump($link = sp_open('/tmp/storage', 'test'));

var_dump(sp_set('name', 'suxiaolin', $link));

var_dump(sp_delete('name', $link));

var_dump(sp_get('name', $link));
```

```php
// transaction
var_dump($link = sp_open('/tmp/storage', 'test'));

var_dump($transaction = sp_begin());

var_dump(sp_set('name', 'xiaolin', $link));
var_dump(sp_get('name', $link));
var_dump(sp_delete('name', $link));

var_dump(sp_commit($transaction));
```