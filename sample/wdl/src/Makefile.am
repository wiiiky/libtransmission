## Process this file with automake to produce Makefile.in



AM_CPPFLAGS = \
			  -DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
			  -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
			  -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
			  $(WDL_CFLAGS)

AM_CFLAGS =\
		   -Wall\
		   -g

bin_PROGRAMS = wdl

wdl_SOURCES =  \
			   main.c \
			   wldownloader.c \
			   wldownloader.h \
			   wldownloadwindow.c \
			   wldownloadwindow.h \
			   wlhttper.c \
			   wlhttper.h \
			   wlhttpermenu.c \
			   wlhttpermenu.h \
			   wlhttperproperties.c \
			   wlhttperproperties.h \
			   wlurldialog.c \
			   wlurldialog.h \
			   wlbter.c \
			   wlbter.h \
			   wlbtfilechooser.c \
			   wlbtfilechooser.h \
			   icons.c \
			   icons.h \
			   wlbtermenu.c \
			   wlbtermenu.h \
			   wlbterproperties.c \
			   wlbterproperties.h

wdl_LDFLAGS = \
			  -Wl,--export-dynamic 

wdl_CFLAGS =-pthread

wdl_LDADD = $(WDL_LIBS) 
