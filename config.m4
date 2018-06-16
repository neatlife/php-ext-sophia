dnl $Id$
dnl config.m4 for extension sophia

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(sophia, for sophia support,
dnl Make sure that the comment is aligned:
dnl [  --with-sophia             Include sophia support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(sophia, whether to enable sophia support,
dnl Make sure that the comment is aligned:
[  --enable-sophia           Enable sophia support])

if test "$PHP_SOPHIA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-sophia -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/sophia.h"  # you most likely want to change this
  dnl if test -r $PHP_SOPHIA/$SEARCH_FOR; then # path given as parameter
  dnl   SOPHIA_DIR=$PHP_SOPHIA
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for sophia files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SOPHIA_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SOPHIA_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the sophia distribution])
  dnl fi

  dnl # --with-sophia -> add include path
  dnl PHP_ADD_INCLUDE($SOPHIA_DIR/include)
  for include in $(echo sophia/sophia/*/); do
    PHP_ADD_INCLUDE($include)
  done

  dnl # --with-sophia -> check for lib and symbol presence
  dnl LIBNAME=sophia # you may want to change this
  dnl LIBSYMBOL=sophia # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SOPHIA_DIR/$PHP_LIBDIR, SOPHIA_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SOPHIALIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong sophia lib version or lib not found])
  dnl ],[
  dnl   -L$SOPHIA_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SOPHIA_SHARED_LIBADD)

  PHP_NEW_EXTENSION(sophia, $(echo sophia/sophia/*/*.c) sophia.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
