AC_INIT([libelperiodic],[m4_esyscmd_s([git rev-parse --abbrev-ref HEAD])],[sobomax@sippysoft.com])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([disable])
AM_PATH_PYTHON([2.7],, [:])

LT_INIT([disable-shared])

AC_PROG_CC
AC_PROG_RANLIB

mydef_version=`git describe --always --dirty || date "+%Y%m%d%H%M%S"`
AC_DEFINE_UNQUOTED([VERSION_GIT], ["$mydef_version"])

AC_ARG_ENABLE(coverage,
  AS_HELP_STRING([--enable-coverage],[enable test coverage support]),
  [AC_DEFINE([ENABLE_COVERAGE])
  CFLAGS="${CFLAGS} --coverage"; LDFLAGS="${LDFLAGS} --coverage"; break], [])

AC_ARG_WITH([python],
   [AS_HELP_STRING([--without-python],[disable building/installing of a python module])],
   [AS_IF([test "x$PYTHON" != "x:" -a "$withval" = "no"], [PYTHON=":"])]
   [])

AC_CHECK_FUNC(clock_gettime, [], [
  AC_CHECK_LIB(rt, clock_gettime, RT_LIB="-lrt",
    AC_MSG_ERROR(Unable to find clock_gettime function; required by ocount))])
AC_SUBST(RT_LIB)

AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
