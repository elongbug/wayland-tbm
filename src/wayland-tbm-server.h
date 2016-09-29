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

#ifndef WAYLAND_TBM_SERVER_H
#define WAYLAND_TBM_SERVER_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <wayland-server.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

struct wayland_tbm_server;
struct wayland_tbm_client_queue;

typedef void (*wayland_tbm_server_surface_destroy_cb) (tbm_surface_h surface, void *data);

struct wayland_tbm_server *
wayland_tbm_server_init(struct wl_display *display,
			const char *device_name,
			int fd,
			uint32_t flags);

void
wayland_tbm_server_deinit(struct wayland_tbm_server *tbm_srv);

void *
wayland_tbm_server_get_bufmgr(struct wayland_tbm_server	*tbm_srv);

tbm_surface_h
wayland_tbm_server_get_surface(struct wayland_tbm_server *tbm_srv,
			       struct wl_resource *wl_buffer);

uint32_t
wayland_tbm_server_get_buffer_flags(struct wayland_tbm_server *tbm_srv,
					struct wl_resource *wl_buffer);

struct wl_resource *
wayland_tbm_server_export_buffer(struct wayland_tbm_server *tbm_srv,
					struct wl_resource *wl_tbm,
					tbm_surface_h surface);

struct wl_resource *
wayland_tbm_server_get_remote_buffer(struct wayland_tbm_server *tbm_srv,
					struct wl_resource *wl_buffer,
					struct wl_resource *wl_tbm);

struct wayland_tbm_client_queue *
wayland_tbm_server_client_queue_get(struct wayland_tbm_server *tbm_srv,
					struct wl_resource *wl_surface);

void
wayland_tbm_server_client_queue_activate(struct wayland_tbm_client_queue *client_queue,
					uint32_t usage, uint32_t queue_size, uint32_t need_flush);

void
wayland_tbm_server_client_queue_deactivate(struct wayland_tbm_client_queue *client_queue);

struct wl_resource *
wayland_tbm_server_client_queue_export_buffer(struct wayland_tbm_client_queue *client_queue,
			tbm_surface_h surface, uint32_t flags, wayland_tbm_server_surface_destroy_cb destroy_cb,
			void *user_data);

void
wayland_tbm_server_client_queue_flush(struct wayland_tbm_client_queue *cqueue);

void
wayland_tbm_server_increase_buffer_sync_timeline(struct wayland_tbm_server *tbm_srv,
			       struct wl_resource *wl_buffer, unsigned int count);

int
wayland_tbm_server_buffer_has_sync_timeline(struct wayland_tbm_server *tbm_srv,
			       struct wl_resource *wl_buffer);

#ifdef  __cplusplus
}
#endif

#endif
