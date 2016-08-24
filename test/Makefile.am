TESTS = 			\
	tbm-server-test	\
	tbm-client-test \
	tbm-subcomp-test \
	tbm-server-queue-test	\
	tbm-client-queue-test	\
	tbm-leak-test  \
	tbm-server-remote-test \
	tbm-client-remote-provider-test \
	tbm-client-remote-consumer-test


#bin_PROGRAMS = $(TESTS)
#check_PROGRAMS = $(TESTS)
noinst_PROGRAMS = $(TESTS)

SERVER_TEST_CFLAGS = -I$(top_srcdir)/src/ $(AM_CFLAGS) $(WL_TBM_COMMON_CFLAGS) $(WL_TBM_SERVER_CFLAGS)
SERVER_TEST_LDADD = ../src/libwayland-tbm-server.la $(WL_TBM_COMMON_LIBS) $(WL_TBM_SERVER_LIBS)
CLIENT_TEST_CFLAGS = -I$(top_srcdir)/src/ $(AM_CFLAGS) $(WL_TBM_COMMON_CFLAGS) $(WL_TBM_CLIENT_CFLAGS)
CLIENT_TEST_LDADD = ../src/libwayland-tbm-client.la $(WL_TBM_COMMON_LIBS) $(WL_TBM_CLIENT_LIBS)

PROTOCOL_SOURCES = 	\
	wayland-tbm-test-server-protocol.h	\
	wayland-tbm-test-client-protocol.h	\
	wayland-tbm-test-protocol.c

tbm_server_test_LDADD = $(SERVER_TEST_LDADD)
tbm_server_test_CFLAGS = $(SERVER_TEST_CFLAGS)
tbm_server_test_SOURCES = $(PROTOCOL_SOURCES) tbm-server-test.c

tbm_client_test_LDADD = $(CLIENT_TEST_LDADD)
tbm_client_test_CFLAGS = $(CLIENT_TEST_CFLAGS)
tbm_client_test_SOURCES = $(PROTOCOL_SOURCES) tbm-client-test.c

tbm_subcomp_test_LDADD = $(CLIENT_TEST_LDADD) $(SERVER_TEST_LDADD)
tbm_subcomp_test_CFLAGS = $(CLIENT_TEST_CFLAGS) $(SERVER_TEST_CFLAGS)
tbm_subcomp_test_SOURCES = $(PROTOCOL_SOURCES) tbm-subcomp-test.c

tbm_server_queue_test_LDADD = $(SERVER_TEST_LDADD)
tbm_server_queue_test_CFLAGS = $(SERVER_TEST_CFLAGS)
tbm_server_queue_test_SOURCES = $(PROTOCOL_SOURCES) tbm-server-queue-test.c

tbm_client_queue_test_LDADD = $(CLIENT_TEST_LDADD)
tbm_client_queue_test_CFLAGS = $(CLIENT_TEST_CFLAGS)
tbm_client_queue_test_SOURCES = $(PROTOCOL_SOURCES) tbm-client-queue-test.c

tbm_leak_test_LDADD = $(CLIENT_TEST_LDADD)
tbm_leak_test_CFLAGS = $(CLIENT_TEST_CFLAGS)
tbm_leak_test_SOURCES = $(PROTOCOL_SOURCES) tbm-leak-test.c

tbm_server_remote_test_LDADD = $(SERVER_TEST_LDADD)
tbm_server_remote_test_CFLAGS = $(SERVER_TEST_CFLAGS)
tbm_server_remote_test_SOURCES = $(PROTOCOL_SOURCES) tbm-server-remote-test.c

tbm_client_remote_provider_test_LDADD = $(CLIENT_TEST_LDADD)
tbm_client_remote_provider_test_CFLAGS = $(CLIENT_TEST_CFLAGS)
tbm_client_remote_provider_test_SOURCES = $(PROTOCOL_SOURCES) tbm-client-remote-provider-test.c

tbm_client_remote_consumer_test_LDADD = $(CLIENT_TEST_LDADD)
tbm_client_remote_consumer_test_CFLAGS = $(CLIENT_TEST_CFLAGS)
tbm_client_remote_consumer_test_SOURCES = $(PROTOCOL_SOURCES) tbm-client-remote-consumer-test.c