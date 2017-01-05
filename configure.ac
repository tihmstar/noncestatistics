AC_PREREQ([2.69])
AC_INIT([noncestatistics], [1.0.0], [tihmstar@gmail.com])
# prepare for automake
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([noncestatistics/all_noncestatistics.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
CXXFLAGS+=" -std=c++11"
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.

PKG_CHECK_MODULES(libplist, libplist >= 1.0)
PKG_CHECK_MODULES(libimobiledevice, libimobiledevice-1.0 >= 1.2.1)
PKG_CHECK_MODULES(libirecovery, libirecovery >= 0.2.0)


# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset mkdir pow strchr strdup strstr])


AC_CONFIG_FILES([Makefile noncestatistics/Makefile])
AC_OUTPUT

