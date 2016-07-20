/*
Copyright (C) 2015 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
      SooChan Lim <sc1.lim@samsung.com>,

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>

#include "wayland-tbm-int.h"
#include "wayland-tbm-client-protocol.h"

//#define WL_TBM_MONITOR_DEBUG

struct wayland_tbm_monitor {
	struct wl_display *dpy;
	struct wl_tbm *wl_tbm;

	struct {
		WL_TBM_MONITOR_COMMAND command;
		union {
			int  subcommand;
			WL_TBM_MONITOR_TRACE_COMMAND trace_command;
			WL_TBM_MONITOR_DUMP_COMMAND dump_command;
		};
		WL_TBM_MONITOR_TARGET target;
		int pid;
	} options;
};

static const struct wl_tbm_listener wl_tbm_monitor_listener = {
	NULL, /* monitor_client_tbm_bo */
};

static void
_wayland_tbm_monitor_registry_handle_global(void *data,
		struct wl_registry *registry,
		uint32_t name,
		const char *interface,
		uint32_t version)
{
	struct wayland_tbm_monitor *tbm_monitor = (struct wayland_tbm_monitor *)data;

	if (!strcmp(interface, "wl_tbm")) {
		tbm_monitor->wl_tbm = wl_registry_bind(registry, name, &wl_tbm_interface,
						       version);
		WL_TBM_RETURN_IF_FAIL(tbm_monitor->wl_tbm != NULL);

		wl_tbm_add_listener(tbm_monitor->wl_tbm, &wl_tbm_monitor_listener, tbm_monitor);

		/* request the tbm monitor */
		wl_tbm_request_tbm_monitor(tbm_monitor->wl_tbm,
					   tbm_monitor->options.command,
					   tbm_monitor->options.subcommand,
					   tbm_monitor->options.target,
					   tbm_monitor->options.pid);
		wl_display_roundtrip(tbm_monitor->dpy);
	}
}

static const struct wl_registry_listener registry_listener = {
	_wayland_tbm_monitor_registry_handle_global,
	NULL
};

static void
_wl_tbm_trace_usage()
{
	WL_TBM_LOG("  trace : trace the changes or the infomation of the tbm_bo\n");
	WL_TBM_LOG("   usage : wayland-tbm-monitor trace [command]\n");
	WL_TBM_LOG("    <command>\n");
	WL_TBM_LOG("      on                     : turn on the trace.\n");
	WL_TBM_LOG("      off                    : turn off the trace.\n");
	WL_TBM_LOG("      register client [pid]  : register the pid of client to trace.\n");
	WL_TBM_LOG("      register server        : register the server to trace.\n");
	WL_TBM_LOG("      register all           : register all clients and server.\n");
	WL_TBM_LOG("      unregister client [pid]: unregister the pid of client to trace.\n");
	WL_TBM_LOG("      unregister server      : unregister the server to trace.\n");
	WL_TBM_LOG("      unregister all         : unregister all clients and server.\n");
	WL_TBM_LOG("      status                 : show the status of the trace setting values.\n");
	WL_TBM_LOG("    <examples>\n");
	WL_TBM_LOG("      # wayland-tbm-monitor trace register client 1234\n");
	WL_TBM_LOG("      # wayland-tbm-monitor trace on\n");
	WL_TBM_LOG("      # wayland-tbm-monitor trace off\n");
	WL_TBM_LOG("\n");
}

static void
_wl_tbm_dump_usage()
{
	WL_TBM_LOG("  dump : dump the tbm_surfaces\n");
	WL_TBM_LOG("   usage : wayland-tbm-monitor dump [command]\n");
	WL_TBM_LOG("    <command>\n");
	WL_TBM_LOG("      snapshot client [pid] : make dumps of all tbm_surfaces for the client with pid.\n");
	WL_TBM_LOG("      snapshot server       : make dumps of all tbm_surfaces for the server.\n");
	WL_TBM_LOG("      snapshot all          : make dumps of all tbm_surfaces for all clients and server.\n");
	WL_TBM_LOG("      queue client [pid]    : make dumps of all tbm_surfaces for the client with pid at enqueue and aquire.\n");
	WL_TBM_LOG("      queue server          : make dumps of all tbm_surfaces for the server at enqueue and aquire.\n");
	WL_TBM_LOG("      queue all             : make dumps of all tbm_surfaces for all clients and server at enqueue and aquire.\n");
	WL_TBM_LOG("      queue on              : turn on the dumps of the tbm surface queue.\n");
	WL_TBM_LOG("      queue off             : turn off the dumps of the tbm surface queue.\n");
	WL_TBM_LOG("    <examples>\n");
	WL_TBM_LOG("      # wayland-tbm-monitor dump snapshot all\n");
	WL_TBM_LOG("      # wayland-tbm-monitor dump queue all\n");
	WL_TBM_LOG("      # wayland-tbm-monitor dump queue on\n");
	WL_TBM_LOG("      # wayland-tbm-monitor dump queue off\n");
	WL_TBM_LOG("\n");
}

static void
_wl_tbm_show_usage()
{
	WL_TBM_LOG("  show : show the infomation of the tbm_bo\n");
	WL_TBM_LOG("   show usage : wayland-tbm-monitor show [target]\n");
	WL_TBM_LOG("    <target>\n");
	WL_TBM_LOG("      client [pid]: shows all tbm_bo information contained by pid of client.\n");
	WL_TBM_LOG("      server      : shows all tbm_bo information contained by server.\n");
	WL_TBM_LOG("      all         : shows all tbm_bo information contained by all clients and server.\n");
	WL_TBM_LOG("    <examples>\n");
	WL_TBM_LOG("      # wayland-tbm-monitor show client 1234\n");
	WL_TBM_LOG("      # wayland-tbm-monitor show server\n");
	WL_TBM_LOG("      # wayland-tbm-monitor show all\n");
	WL_TBM_LOG("\n");
}

static void
_wl_tbm_list_usage()
{
	WL_TBM_LOG("  list : list the wayland-tbm client\n");
	WL_TBM_LOG("   list usage : wayland-tbm-monitor list\n");
	WL_TBM_LOG("\n");
}

static void
_wl_tbm_usage()
{
	WL_TBM_LOG("wayland-tbm-monitor : show/trace the infomation of tbm_bo in clietns and in server.\n");
	_wl_tbm_list_usage();
	_wl_tbm_show_usage();
	_wl_tbm_trace_usage();
	_wl_tbm_dump_usage();
	WL_TBM_LOG("\n");
}

static int
_wl_tbm_select_target_option(struct wayland_tbm_monitor *tbm_monitor, int argc,
			     char *argv[], int arg_pos)
{
	if (!strncmp(argv[arg_pos], "client", strlen(argv[arg_pos]) + 1)) {
		if (argc < (arg_pos + 2)) {
			WL_TBM_LOG("error: no pid. please type the pid.\n");
			return 0;
		}

		tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_CLIENT;
		tbm_monitor->options.pid = atoi(argv[arg_pos + 1]);
	} else if (!strncmp(argv[arg_pos], "server", 6)) {
		tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_SERVER;
	} else if (!strncmp(argv[arg_pos], "all", 3)) {
		tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_ALL;
	} else {
		return 0;
	}

	return 1;
}


static int
_wl_tbm_monitor_process_options(struct wayland_tbm_monitor *tbm_monitor,
				int argc, char *argv[])
{
	if (argc < 2) {
		_wl_tbm_usage();
		return 0;
	}

	if (!strncmp(argv[1], "list", strlen(argv[1]) + 1)) {
		tbm_monitor->options.command = WL_TBM_MONITOR_COMMAND_LIST;
	} else if (!strncmp(argv[1], "show", strlen(argv[1]) + 1)) {
		if (argc < 3) {
			_wl_tbm_show_usage();
			return 0;
		}

		tbm_monitor->options.command = WL_TBM_MONITOR_COMMAND_SHOW;
		if (!_wl_tbm_select_target_option(tbm_monitor, argc, argv, 2)) {
			_wl_tbm_show_usage();
			return 0;
		}
	} else if (!strncmp(argv[1], "trace", strlen(argv[1]) + 1)) {
		if (argc < 3) {
			_wl_tbm_trace_usage();
			return 0;
		}

		tbm_monitor->options.command = WL_TBM_MONITOR_COMMAND_TRACE;

		if (!strncmp(argv[2], "on", strlen(argv[2]) + 1)) {
			tbm_monitor->options.trace_command = WL_TBM_MONITOR_TRACE_COMMAND_ON;
			tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_ALL;
		} else if (!strncmp(argv[2], "off", strlen(argv[2]) + 1)) {
			tbm_monitor->options.trace_command = WL_TBM_MONITOR_TRACE_COMMAND_OFF;
			tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_ALL;
		} else if (!strncmp(argv[2], "register", strlen(argv[2]) + 1)) {
			if (argc < 4) {
				WL_TBM_LOG("error: no pid. please type the target(client [pid]/server/all).\n");
				_wl_tbm_trace_usage();
				return 0;
			}

			tbm_monitor->options.trace_command = WL_TBM_MONITOR_TRACE_COMMAND_REGISTER;
			if (!_wl_tbm_select_target_option(tbm_monitor, argc, argv, 3)) {
				WL_TBM_LOG("error: no pid. please type the target(client [pid]/server/all).\n");
				_wl_tbm_trace_usage();
				return 0;
			}
		} else if (!strncmp(argv[2], "unregister", strlen(argv[2]) + 1)) {
			if (argc < 4) {
				WL_TBM_LOG("error: no pid. please type the target(client [pid]/server/all).\n");
				_wl_tbm_trace_usage();
				return 0;
			}

			tbm_monitor->options.trace_command = WL_TBM_MONITOR_TRACE_COMMAND_UNREGISTER;
			if (!_wl_tbm_select_target_option(tbm_monitor, argc, argv, 3)) {
				WL_TBM_LOG("error: no pid. please type the target(client [pid]/server/all).\n");
				_wl_tbm_trace_usage();
				return 0;
			}
		} else if (!strncmp(argv[2], "status", strlen(argv[2]) + 1)) {
			tbm_monitor->options.trace_command = WL_TBM_MONITOR_TRACE_COMMAND_STATUS;
			tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_ALL;
		} else {
			_wl_tbm_trace_usage();
			return 0;
		}
	} else if (!strncmp(argv[1], "dump", strlen(argv[1]) + 1)) {

		if (argc < 4) {
			_wl_tbm_dump_usage();
			return 0;
		}

		tbm_monitor->options.command = WL_TBM_MONITOR_COMMAND_DUMP;

		if (!strncmp(argv[2], "snapshot", strlen(argv[2]) + 1)) {
			tbm_monitor->options.dump_command = WL_TBM_MONITOR_DUMP_COMMAND_SNAPSHOT;
			if (!_wl_tbm_select_target_option(tbm_monitor, argc, argv, 3)) {
				WL_TBM_LOG("error: no pid. please type the target(client [pid]/server/all).\n");
				_wl_tbm_dump_usage();
				return 0;
			}
		} else if (!strncmp(argv[2], "queue", strlen(argv[2]) + 1)) {

			if (!strncmp(argv[3], "on", strlen(argv[3]) + 1)) {
				tbm_monitor->options.dump_command = WL_TBM_MONITOR_DUMP_COMMAND_ON;
				tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_ALL;
			} else if (!strncmp(argv[3], "off", strlen(argv[3]) + 1)) {
				tbm_monitor->options.dump_command = WL_TBM_MONITOR_DUMP_COMMAND_OFF;
				tbm_monitor->options.target = WL_TBM_MONITOR_TARGET_ALL;
			} else {
				tbm_monitor->options.dump_command = WL_TBM_MONITOR_DUMP_COMMAND_QUEUE;
				if (!_wl_tbm_select_target_option(tbm_monitor, argc, argv, 3)) {
					WL_TBM_LOG("error: no pid. please type the target(client [pid]/server/all).\n");
					_wl_tbm_dump_usage();
					return 0;
				}
			}
		} else {
			_wl_tbm_dump_usage();
			return 0;
		}
	} else {
		_wl_tbm_usage();
		return 0;
	}

	return 1;
}

int
main(int argc, char *argv[])
{
	struct wayland_tbm_monitor *tbm_monitor = NULL;
	struct wl_registry *wl_registry;
	int ret = 0;

	tbm_monitor = calloc(1, sizeof(struct wayland_tbm_monitor));
	WL_TBM_RETURN_VAL_IF_FAIL(tbm_monitor != NULL, -1);

	ret = _wl_tbm_monitor_process_options(tbm_monitor, argc, argv);
	if (!ret)
		goto finish;

	tbm_monitor->dpy = wl_display_connect(NULL);
	WL_TBM_GOTO_IF_FAIL(tbm_monitor->dpy != NULL, finish);

	wl_registry = wl_display_get_registry(tbm_monitor->dpy);
	WL_TBM_GOTO_IF_FAIL(wl_registry != NULL, finish);

#ifdef WL_TBM_MONITOR_DEBUG
	WL_TBM_LOG("[%s]: tbm_monitor=%p, display=%p\n", __func__, tbm_monitor,
		   tbm_monitor->dpy);
#endif

	wl_registry_add_listener(wl_registry, &registry_listener, tbm_monitor);
	wl_display_dispatch(tbm_monitor->dpy);
	wl_display_roundtrip(tbm_monitor->dpy);

	wl_registry_destroy(wl_registry);

finish:
	if (tbm_monitor) {
		if (tbm_monitor->wl_tbm) {
			wl_tbm_set_user_data(tbm_monitor->wl_tbm, NULL);
			wl_tbm_destroy(tbm_monitor->wl_tbm);
		}

		free(tbm_monitor);
	}

	return 1;
}

