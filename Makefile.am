#datadir = @datadir@/xtraceroute

SUBDIRS = share

bin_PROGRAMS = xtraceroute

xtraceroute_SOURCES = readtexture.c db.c countries.c main.c sphere.c dbio.c \
                glstuff.c spinner.c dialogs.c info_window.c doughnut.c \
                util.c resolve.c db_host_gui.c db_net_gui.c db_gen_gui.c \
		xt.h tweaks.h trackball.h trackball.c extprog.c astronomy.c \
		net.c


xtraceroute_CFLAGS = $(GDK_PIXBUF_CFLAGS) $(GTKGL_CFLAGS) $(GTK_CFLAGS) $(GEOIP_CFLAGS)
xtraceroute_LDADD  = $(GDK_PIXBUF_LIBS) $(GTKGL_LIBS) $(GTK_LIBS) $(GEOIP_LIBS)

DEFS = @DEFS@ -DDATADIR=\"${xtraceroutedatadir}\" \
       -DLOCALEDIR=\"${xtraceroutelocaledir}\"

ACLOCAL_AMFLAGS=-I m4

if DEBUG
AM_CFLAGS = -DXT_DEBUG -Wall
endif

man_MANS = xtraceroute.1

EXTRA_DIST = BUGS COPYING AUTHORS \
		xtraceroute.1.in \
		xtraceroute.png \
		xtraceroute.desktop \
		xtraceroute.spec \
		autogen.sh
