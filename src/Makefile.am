## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-D__TRANSMISSION__

AM_CFLAGS = \
  $(DHT_CFLAGS) \
  $(LIBUTP_CFLAGS) \
  $(LIBUPNP_CFLAGS) \
  $(LIBNATPMP_CFLAGS) \
  $(LIBEVENT_CFLAGS) \
  $(LIBCURL_CFLAGS) \
  $(OPENSSL_CFLAGS) \
  $(PTHREAD_CFLAGS) \
  $(ZLIB_CFLAGS)

noinst_LTLIBRARIES = libtransmission.la


libtransmission_la_SOURCES = \
	announcer.c \
	announcer-http.c \
	announcer-udp.c \
	bandwidth.c \
	bitfield.c \
	blocklist.c \
	cache.c \
	clients.c \
	completion.c \
	ConvertUTF.c \
	crypto.c \
	fdlimit.c \
	handshake.c \
	history.c \
	inout.c \
	list.c \
	log.c \
	magnet.c \
	makemeta.c \
	metainfo.c \
	natpmp.c \
	net.c \
	peer-io.c \
	peer-mgr.c \
	peer-msgs.c \
	platform.c \
	platform-quota.c \
	port-forwarding.c \
	ptrarray.c \
	quark.c \
	resume.c \
	rpcimpl.c \
	rpc-server.c \
	session.c \
	stats.c \
	torrent.c \
	torrent-ctor.c \
	torrent-magnet.c \
	tr-dht.c \
	tr-lpd.c \
	tr-udp.c \
	tr-utp.c \
	tr-getopt.c \
	trevent.c \
	upnp.c \
	utils.c \
	variant.c \
	variant-benc.c \
	variant-json.c \
	verify.c \
	web.c \
	webseed.c \
	wildmat.c

#libtransmission_a_LDFLAGS =

libtransmission_la_LIBADD =\
	$(LIBUPNP_LIBS) \
	$(LIBNATPMP_LIBS) \
	$(INTLLIBS) \
	$(DHT_LIBS) \
	$(LIBUTP_LIBS) \
	$(LIBCURL_LIBS) \
	$(LIBEVENT_LIBS) \
	$(OPENSSL_LIBS) \
	$(PTHREAD_LIBS) \
	$(ZLIB_LIBS)


libtransmissionsubincludedir = $(includedir)/libtransmission
libtransmissionsubinclude_HEADERS= \
	announcer.h \
	announcer-common.h \
	bandwidth.h \
	bitfield.h \
	blocklist.h \
	cache.h \
	clients.h \
	ConvertUTF.h \
	crypto.h \
	completion.h \
	fdlimit.h \
	handshake.h \
	history.h \
	inout.h \
	jsonsl.c \
	jsonsl.h \
	libtransmission-test.h \
	list.h \
	log.h \
	magnet.h \
	makemeta.h \
	metainfo.h \
	natpmp_local.h \
	net.h \
	peer-common.h \
	peer-io.h \
	peer-mgr.h \
	peer-msgs.h \
	platform.h \
	platform-quota.h \
	port-forwarding.h \
	ptrarray.h \
	quark.h \
	resume.h \
	rpcimpl.h \
	rpc-server.h \
	session.h \
	stats.h \
	torrent.h \
	torrent-magnet.h \
	tr-getopt.h \
	transmission.h \
	tr-dht.h \
	tr-udp.h \
	tr-utp.h \
	tr-lpd.h \
	trevent.h \
	upnp.h \
	utils.h \
	variant.h \
	variant-common.h \
	verify.h \
	version.h \
	web.h \
	webseed.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtransmission-0.1.pc

EXTRA_DIST = \
	libtransmission-0.1.pc.in
