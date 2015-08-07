dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(libtransmission, 0.1)

dnl AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/transmission.h)

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CXX

LT_INIT

CURL_MINIMUM=7.15.4
AC_SUBST(CURL_MINIMUM)
LIBEVENT_MINIMUM=2.0.10
AC_SUBST(LIBEVENT_MINIMUM)
OPENSSL_MINIMUM=0.9.4
AC_SUBST(OPENSSL_MINIMUM)


DHT_CFLAGS="-I\$(top_srcdir)/third-party/dht"
DHT_LIBS="\$(top_builddir)/third-party/dht/libdht.a"
AC_SUBST(DHT_CFLAGS)
AC_SUBST(DHT_LIBS)

AC_CHECK_LIB([rt],
             [clock_gettime],
             [libutp_extra_libs="-lrt"],
             [libutp_extra_libs=""])

AC_MSG_CHECKING([µTP])
build_utp="no"
if test "x$CXX" != "x" ; then
    have_utp="yes"
else
    have_utp="no"
fi
AC_ARG_ENABLE([utp],
              AS_HELP_STRING([--enable-utp],[build µTP support]),
              [want_utp=${enableval}],
              [want_utp=${have_utp}])
if test "x$want_utp" = "xyes" ; then
    if test "x$have_utp" = "xyes"; then
        LIBUTP_CFLAGS="-I\$(top_srcdir)/third-party/"
        LIBUTP_LIBS="\$(top_builddir)/third-party/libutp/libutp.a"
        if test "x$libutp_extra_libs" != "x" ; then
            LIBUTP_LIBS="$LIBUTP_LIBS $libutp_extra_libs"
        fi
        AC_DEFINE([WITH_UTP],[1])
        build_utp="yes"
    else
      AC_MSG_ERROR("Unable to build uTP support -- C++ compiler not found")
    fi
fi
AC_SUBST(LIBUTP_CFLAGS)
AC_SUBST(LIBUTP_LIBS)
AM_CONDITIONAL([BUILD_UTP],[test "x$build_utp" = "xyes"])
AC_MSG_RESULT([$build_utp])


ac_save_LIBS="$LIBS"
LIBS="-lminiupnpc"
AC_MSG_CHECKING([system miniupnpc library])

dnl See if ANY version of miniupnpc is installed
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <miniupnpc/miniupnpc.h>],
                   [struct UPNPDev dev;])],
  [upnp_version="unknown"],
  [upnp_version="none"]
)

dnl Let's hope it's 1.7 or higher, since it provides
dnl MINIUPNPC_API_VERSION and we won't have to figure
dnl it out on our own
if test "x$upnp_version" = "xunknown" ; then
  AC_RUN_IFELSE(
    [AC_LANG_PROGRAM(
      [#include <stdlib.h>
       #include <miniupnpc/miniupnpc.h>],
      [#ifdef MINIUPNPC_API_VERSION
       return EXIT_SUCCESS;
       #else
       return EXIT_FAILURE;
       #endif]
    )],
    [upnp_version=">= 1.7"]
  )
fi

dnl Or maybe it's miniupnp 1.6
if test "x$upnp_version" = "xunknown" ; then
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
    #include <stdlib.h>
    #include <errno.h>
    #include <miniupnpc/miniupnpc.h>
    #include <miniupnpc/upnpcommands.h>
  ]], [[
    struct UPNPDev * devlist;
    struct UPNPUrls urls;
    struct IGDdatas data;
    char lanaddr[16];
    char portStr[8];
    char intPort[8];
    char intClient[16];
    upnpDiscover( 2000, NULL, NULL, 0, 0, &errno );
    UPNP_GetValidIGD( devlist, &urls, &data, lanaddr, sizeof( lanaddr ) );
    UPNP_GetSpecificPortMappingEntry( urls.controlURL, data.first.servicetype,
                        portStr, "TCP", intClient, intPort, NULL, NULL, NULL );
  ]])],[
  AC_DEFINE(MINIUPNPC_API_VERSION, 8, [miniupnpc 1.6 has API version 8])
  upnp_version="1.6"])
fi

dnl Or maybe it's miniupnp 1.5
if test "x$upnp_version" = "xunknown" ; then
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
    #include <stdlib.h>
    #include <miniupnpc/miniupnpc.h>
    #include <miniupnpc/upnpcommands.h>
  ]], [[
    struct UPNPDev * devlist;
    struct UPNPUrls urls;
    struct IGDdatas data;
    char lanaddr[16];
    char portStr[8];
    char intPort[8];
    char intClient[16];
    upnpDiscover( 2000, NULL, NULL, 0 );
    UPNP_GetValidIGD( devlist, &urls, &data, lanaddr, sizeof( lanaddr ) );
    UPNP_GetSpecificPortMappingEntry( urls.controlURL, data.first.servicetype,
                        portStr, "TCP", intClient, intPort );
  ]])],[
  AC_DEFINE(MINIUPNPC_API_VERSION, 5, [miniupnpc 1.5 has API version 5])
  upnp_version="1.5"])
fi

# ... and the results of our tests
LIBS="$ac_save_LIBS"
AC_MSG_RESULT([$upnp_version])
AM_CONDITIONAL([BUILD_MINIUPNP],[test "x$upnp_version" = "xnone"])
if test "x$upnp_version" = "xnone" ; then
    LIBUPNP_CFLAGS="-I\$(top_srcdir)/third-party/"
    LIBUPNP_LIBS="\$(top_builddir)/third-party/miniupnp/libminiupnp.a"
else
    AC_DEFINE([SYSTEM_MINIUPNP])
    LIBUPNP_CFLAGS=""
    LIBUPNP_LIBS="-lminiupnpc"
fi
AC_SUBST(LIBUPNP_CFLAGS)
AC_SUBST(LIBUPNP_LIBS)

LIBNATPMP_CFLAGS="-I\$(top_srcdir)/third-party/libnatpmp/"
LIBNATPMP_LIBS="\$(top_builddir)/third-party/libnatpmp/libnatpmp.a"
build_bundled_natpmp="yes"
AC_ARG_ENABLE([external-natpmp],
              AS_HELP_STRING([--enable-external-natpmp],[Use system external-natpmp]),
              [want_external_natpmp=${enableval}],
              [want_external_natpmp=no])
if test "x$want_external_natpmp" != "xno" ; then
  dnl Would be lovely if it had pkgconfig
  LIBNATPMP_CFLAGS=""
  LIBNATPMP_LIBS="-lnatpmp"
  build_bundled_natpmp="no"
fi
AM_CONDITIONAL([BUILD_NATPMP],[test "x$build_bundled_natpmp" = "xyes"])
AC_SUBST(LIBNATPMP_CFLAGS)
AC_SUBST(LIBNATPMP_LIBS)


AC_SEARCH_LIBS(cos, [m])
AC_SEARCH_LIBS([socket], [socket net])
AC_SEARCH_LIBS([gethostbyname], [nsl bind])
AC_SEARCH_LIBS([quotacursor_skipidtype], [quota])
PKG_CHECK_MODULES(OPENSSL, [openssl >= $OPENSSL_MINIMUM],,[CHECK_SSL()])
PKG_CHECK_MODULES(LIBCURL, [libcurl >= $CURL_MINIMUM])
PKG_CHECK_MODULES(LIBEVENT, [libevent >= $LIBEVENT_MINIMUM])
AC_PATH_ZLIB

AC_SYS_LARGEFILE
AC_CHECK_FUNCS([lseek64])

AC_FUNC_GETMNTENT


AC_OUTPUT([
Makefile
src/libtransmission-0.1.pc
src/Makefile
third-party/Makefile
third-party/dht/Makefile
third-party/libutp/Makefile
third-party/libnatpmp/Makefile
third-party/miniupnp/Makefile
])
