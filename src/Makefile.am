lib_LTLIBRARIES = libwayland-tbm-server.la libwayland-tbm-client.la

AM_CFLAGS = \
	$(GCC_CFLAGS) \
    -I$(top_srcdir)/src

include_HEADERS = \
	wayland-tbm-server.h \
	wayland-tbm-client.h

nodist_include_HEADERS = \
	wayland-tbm-server-protocol.h \
	wayland-tbm-client-protocol.h

libwayland_tbm_server_la_LDFLAGS = -export-dynamic
libwayland_tbm_server_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(WL_TBM_COMMON_CFLAGS) \
	$(WL_TBM_CLIENT_CFLAGS) \
	$(WL_TBM_SERVER_CFLAGS)
libwayland_tbm_server_la_LIBADD = \
	$(WL_TBM_COMMON_LIBS) \
	$(WL_TBM_CLIENT_LIBS) \
	$(WL_TBM_SERVER_LIBS)
libwayland_tbm_server_la_SOURCES = \
	wayland-tbm-client.c \
	wayland-tbm-server.c \
	wayland-tbm-util.c

nodist_libwayland_tbm_server_la_SOURCES =		\
	wayland-tbm-server-protocol.h	\
	wayland-tbm-client-protocol.h	\
	wayland-tbm-protocol.c

libwayland_tbm_client_la_LDFLAGS = -export-dynamic
libwayland_tbm_client_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(WL_TBM_COMMON_CFLAGS) \
	$(WL_TBM_CLIENT_CFLAGS)
libwayland_tbm_client_la_LIBADD = \
	$(WL_TBM_COMMON_LIBS) \
	$(WL_TBM_CLIENT_LIBS)
libwayland_tbm_client_la_SOURCES = \
	wayland-tbm-client.c \
	wayland-tbm-util.c

nodist_libwayland_tbm_client_la_SOURCES =		\
	wayland-tbm-client-protocol.h	\
	wayland-tbm-protocol.c

%-protocol.c : $(top_srcdir)/protocol/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(wayland_scanner) code < $< > $@

%-server-protocol.h : $(top_srcdir)/protocol/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(wayland_scanner) server-header < $< > $@

%-client-protocol.h : $(top_srcdir)/protocol/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(wayland_scanner) client-header < $< > $@

BUILT_SOURCES = \
	$(nodist_libwayland_tbm_server_la_SOURCES)	\
	$(nodist_libwayland_tbm_client_la_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

