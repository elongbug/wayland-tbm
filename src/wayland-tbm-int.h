/*
Copyright (C) 2015 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
      SooChan Lim <sc1.lim@samsung.com>,
      Sangjin Lee <lsj119@samsung.com>,
      Boram Park <boram1288.park@samsung.com>,
      Changyeon Lee <cyeon.lee@samsung.com>

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

#ifndef WAYLAND_TBM_INT_H
#define WAYLAND_TBM_INT_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <tbm_bufmgr.h>
#include "wayland-tbm-client.h"

#define HAVE_DLOG
#ifdef HAVE_DLOG
#include <dlog.h>

extern int bDlog;

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "WL_TBM"

#define WL_TBM_C_LOG(fmt, ...) {\
	if (bDlog) {\
		LOGE("[WL_TBM_C] " fmt, ##__VA_ARGS__);\
	} \
	else {\
		fprintf(stderr, "[WL_TBM_C(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__);\
	} \
}
#define WL_TBM_S_LOG(fmt, ...) {\
	if (bDlog) {\
		LOGE("[WL_TBM_S] " fmt, ##__VA_ARGS__);\
	} \
	else{\
		fprintf(stderr, "[WL_TBM_S(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__);\
	} \
}
#define WL_TBM_LOG(fmt, ...) {\
	if (bDlog) {\
		LOGE("[WL_TBM] " fmt, ##__VA_ARGS__);\
	} \
	else {\
		fprintf(stderr, "[WL_TBM(%d)] " fmt, getpid(), ##__VA_ARGS__);\
	} \
}
#else
#define WL_TBM_C_LOG(fmt, ...)   fprintf(stderr, "[WL_TBM_C(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)
#define WL_TBM_S_LOG(fmt, ...)   fprintf(stderr, "[WL_TBM_S(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)
#define WL_TBM_LOG(fmt, ...)     fprintf(stderr, "[WL_TBM(%d)] " fmt, getpid(), ##__VA_ARGS__)
#endif /* ENABLE_DLOG */

#define WL_TBM_DEBUG(fmt, ...)   fprintf(stderr, "[WL_TBM:DEBUG(%d)] " fmt, getpid(), ##__VA_ARGS__)

/* check condition */
#define WL_TBM_RETURN_IF_FAIL(cond) {\
	if (!(cond)) {\
		WL_TBM_LOG("'%s' failed.\n", #cond);\
		return;\
	} \
}
#define WL_TBM_RETURN_VAL_IF_FAIL(cond, val) {\
	if (!(cond)) {\
		WL_TBM_LOG("'%s' failed.\n", #cond);\
		return val;\
	} \
}
#define WL_TBM_GOTO_IF_FAIL(cond, dst) {\
	if (!(cond)) {\
		WL_TBM_LOG("'%s' failed.\n", #cond);\
		goto dst;\
	} \
}

typedef enum {
	WL_TBM_MONITOR_COMMAND_LIST,
	WL_TBM_MONITOR_COMMAND_SHOW,
	WL_TBM_MONITOR_COMMAND_TRACE,
	WL_TBM_MONITOR_COMMAND_DUMP,
} WL_TBM_MONITOR_COMMAND;

typedef enum {
	WL_TBM_MONITOR_TRACE_COMMAND_ON,
	WL_TBM_MONITOR_TRACE_COMMAND_OFF,
	WL_TBM_MONITOR_TRACE_COMMAND_REGISTER,
	WL_TBM_MONITOR_TRACE_COMMAND_UNREGISTER,
	WL_TBM_MONITOR_TRACE_COMMAND_CHANGE,
	WL_TBM_MONITOR_TRACE_COMMAND_INFO,
	WL_TBM_MONITOR_TRACE_COMMAND_STATUS,
} WL_TBM_MONITOR_TRACE_COMMAND;

typedef enum {
	WL_TBM_MONITOR_DUMP_COMMAND_ON,
	WL_TBM_MONITOR_DUMP_COMMAND_OFF,
	WL_TBM_MONITOR_DUMP_COMMAND_REGISTER,
	WL_TBM_MONITOR_DUMP_COMMAND_UNREGISTER,
	WL_TBM_MONITOR_DUMP_COMMAND_SNAPSHOT,
	WL_TBM_MONITOR_DUMP_COMMAND_STATUS,
} WL_TBM_MONITOR_DUMP_COMMAND;

typedef enum {
	WL_TBM_MONITOR_DUMP_TYPE_IMPORT,
	WL_TBM_MONITOR_DUMP_TYPE_REF,
	WL_TBM_MONITOR_DUMP_TYPE_MAP,
} WL_TBM_MONITOR_DUMP_TYPE;

typedef enum {
	WL_TBM_MONITOR_TRACE_STATUS_UNREGISTERED,
	WL_TBM_MONITOR_TRACE_STATUS_OFF,
	WL_TBM_MONITOR_TRACE_STATUS_ON,
} WL_TBM_MONITOR_TRACE_STATUS;

typedef enum {
	WL_TBM_MONITOR_TARGET_CLIENT,
	WL_TBM_MONITOR_TARGET_SERVER,
	WL_TBM_MONITOR_TARGET_ALL,
} WL_TBM_MONITOR_TARGET;

/* internal functions */
struct wl_tbm *
_wayland_tbm_client_get_wl_tbm(struct wayland_tbm_client *tbm_client);

void
_wayland_tbm_util_get_appname_brief(char *brief);

void
_wayland_tbm_util_get_appname_from_pid(long pid, char *str);

void
_wayland_tbm_check_dlog_enable(void);

void
_change_trace_status(WL_TBM_MONITOR_TRACE_STATUS * curr_status, WL_TBM_MONITOR_TRACE_COMMAND cmd, tbm_bufmgr bufmgr);

char *
_tarce_status_to_str(WL_TBM_MONITOR_TRACE_STATUS status);

char *
_wayland_tbm_dump_directory_make(void);

#ifdef  __cplusplus
}
#endif

#endif
