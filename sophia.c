/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_sophia.h"
#include <libsp.h>

/* If you declare any globals in php_sophia.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(sophia)
*/

/* True global resources - no need for thread safety here */
static int le_sophia;

// latest env
void* latest_env;
void* latest_db;
void* latest_transaction;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("sophia.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_sophia_globals, sophia_globals)
    STD_PHP_INI_ENTRY("sophia.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_sophia_globals, sophia_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

PHP_FUNCTION(sp_open)
{
	char *storage = NULL,  *database = NULL;
	size_t storage_len, database_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &storage, &storage_len, &database, &database_len) == FAILURE) {
		return;
	}

	// create resource object to represent sophia env object
	zend_resource *env_resource, *db_resource;

	void *env = latest_env = sp_env();
	// register as php resource type
	env_resource = zend_register_resource(env, le_sophia_env);
	sp_setstring(env, "sophia.path", storage, 0);
	sp_setstring(env, "db", database, 0);
	sp_setint(env, "db.test.compaction.cache", 4ULL * 1024 * 1024 * 1024);
	sp_open(env);

	// open db for CURD operation
	char full_db_name[256];
	php_sprintf(full_db_name, "db.%s", database);
	void *db = latest_db = sp_getobject(env, full_db_name);
	db_resource = zend_register_resource(db, le_sophia_db);

	void *o = sp_document(db);

	RETURN_RES(db_resource);
}

PHP_FUNCTION(sp_set)
{
	zval *db_resource_zval = NULL;
	char *key = NULL,  *value = NULL;
	size_t key_len, value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|r", &key, &key_len, &value, &value_len, &db_resource_zval) == FAILURE) {
		return;
	}

	void *db = NULL;
	if (db_resource_zval) {
		db = Z_RES_VAL_P(db_resource_zval);
	} else if (latest_db) {
		db = latest_db;
	} else {
		zend_throw_exception(NULL, "no db connection");
		return;
	}

	void *o = sp_document(db);
	sp_setstring(o, "key", key, key_len);
	sp_setstring(o, "value", value, value_len);
	int failure = sp_set(db, o);

	if (failure) { // -1 failure
		RETURN_FALSE;
	} else { // 0 success
		RETURN_TRUE;
	}
}

PHP_FUNCTION(sp_get)
{
	zval *db_resource_zval = NULL;
	char *key = NULL;
	size_t key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|r", &key, &key_len, &db_resource_zval) == FAILURE) {
		return;
	}

	void *db = NULL;
	if (db_resource_zval) {
		db = Z_RES_VAL_P(db_resource_zval);
	} else if (latest_db) {
		db = latest_db;
	} else {
		zend_throw_exception(NULL, "no db connection");
		return;
	}

	void *o = sp_document(db);
	sp_setstring(o, "key", key, key_len);
	void *result = sp_get(db, o);
	if (result) {
		int valuesize;
		char *value = sp_getstring(result, "value", &valuesize);
		sp_destroy(result);

		RETURN_STRING(value);
	} else {
		RETURN_NULL();
	}
}

PHP_FUNCTION(sp_delete)
{
	zval *db_resource_zval = NULL;
	char *key = NULL;
	size_t key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|r", &key, &key_len, &db_resource_zval) == FAILURE) {
		return;
	}

	void *db = NULL;
	if (db_resource_zval) {
		db = Z_RES_VAL_P(db_resource_zval);
	} else if (latest_db) {
		db = latest_db;
	} else {
		zend_throw_exception(NULL, "no db connection");
		return;
	}

	void *o = sp_document(db);
	sp_setstring(o, "key", key, key_len);
	int failure = sp_delete(db, o);

	if (failure) { // -1 failure
		RETURN_FALSE;
	} else { // 0 success
		RETURN_TRUE;
	}
}

PHP_FUNCTION(sp_begin)
{
	// create resource object to represent sophia env object
	zend_resource *transaction_resource;

	void *env = latest_env;
	void *transaction = latest_transaction = sp_begin(env);

	if (!transaction) { // NULL error!
		RETURN_FALSE;
	} else {
		transaction_resource = zend_register_resource(transaction, le_sophia_transaction);
		RETURN_RES(transaction_resource);
	}
}

PHP_FUNCTION(sp_commit)
{
	zval *transaction_resource_zval = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r", &transaction_resource_zval) == FAILURE) {
		return;
	}

	void *transaction = NULL;
	if (transaction_resource_zval) {
		transaction = Z_RES_VAL_P(transaction_resource_zval);
	} else if (latest_transaction) {
		transaction = latest_transaction;
	} else {
		zend_throw_exception(NULL, "no transaction context");
		return;
	}

	int status = sp_commit(transaction);

	RETURN_LONG(status);
}

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_sophia_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_sophia_init_globals(zend_sophia_globals *sophia_globals)
{
	sophia_globals->global_value = 0;
	sophia_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */

// destroy env resource callback
void env_dtor(zend_resource *res)
{
	sp_destroy(res->ptr);
}

PHP_MINIT_FUNCTION(sophia)
{
	le_sophia_env = zend_register_list_destructors_ex(env_dtor, NULL, "sophia_env", module_number);
	le_sophia_db = zend_register_list_destructors_ex(NULL, NULL, "sophia_db", module_number);
	le_sophia_transaction = zend_register_list_destructors_ex(NULL, NULL, "sophia_transaction", module_number);
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(sophia)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(sophia)
{
#if defined(COMPILE_DL_SOPHIA) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(sophia)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(sophia)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "sophia support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ sophia_functions[]
 *
 * Every user visible function must have an entry in sophia_functions[].
 */
const zend_function_entry sophia_functions[] = {
	PHP_FE(sp_open,	NULL)
	PHP_FE(sp_set,	NULL)
	PHP_FE(sp_get,	NULL)
	PHP_FE(sp_delete,	NULL)
	PHP_FE(sp_begin,	NULL)
	PHP_FE(sp_commit,	NULL)
	PHP_FE_END	/* Must be the last line in sophia_functions[] */
};
/* }}} */

/* {{{ sophia_module_entry
 */
zend_module_entry sophia_module_entry = {
	STANDARD_MODULE_HEADER,
	"sophia",
	sophia_functions,
	PHP_MINIT(sophia),
	PHP_MSHUTDOWN(sophia),
	PHP_RINIT(sophia),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(sophia),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(sophia),
	PHP_SOPHIA_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SOPHIA
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(sophia)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
