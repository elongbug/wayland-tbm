/*
 * Copyright 2015 Samsung Electronics co., Ltd. All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that\n the above copyright notice appear in
 * all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * the copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef TBM_TEST_SERVER_PROTOCOL_H
#define TBM_TEST_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

struct wl_client;
struct wl_resource;

struct wl_buffer;
struct wl_callback;
struct wl_tbm;
struct wl_tbm_test;
struct wl_test_remote;
struct wl_test_surface;

extern const struct wl_interface wl_tbm_test_interface;
extern const struct wl_interface wl_test_surface_interface;
extern const struct wl_interface wl_test_remote_interface;

struct wl_tbm_test_interface {
	/**
	 * create_surface - (none)
	 * @surface: (none)
	 */
	void (*create_surface)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t surface);
	/**
	 * set_active_queue - (none)
	 * @surface: (none)
	 */
	void (*set_active_queue)(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_resource *surface);
	/**
	 * set_provider - (none)
	 * @surface: (none)
	 * @name: (none)
	 */
	void (*set_provider)(struct wl_client *client,
			     struct wl_resource *resource,
			     struct wl_resource *surface,
			     const char *name);
	/**
	 * create_remote_surface - (none)
	 * @surface: (none)
	 * @name: (none)
	 */
	void (*create_remote_surface)(struct wl_client *client,
				      struct wl_resource *resource,
				      uint32_t surface,
				      const char *name);
};


struct wl_test_surface_interface {
	/**
	 * destroy - (none)
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * attach - (none)
	 * @buffer: (none)
	 */
	void (*attach)(struct wl_client *client,
		       struct wl_resource *resource,
		       struct wl_resource *buffer);
	/**
	 * frame - (none)
	 * @callback: (none)
	 */
	void (*frame)(struct wl_client *client,
		      struct wl_resource *resource,
		      uint32_t callback);
};


struct wl_test_remote_interface {
	/**
	 * destroy - (none)
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * release - (none)
	 * @buffer: (none)
	 */
	void (*release)(struct wl_client *client,
			struct wl_resource *resource,
			struct wl_resource *buffer);
	/**
	 * redirect - (none)
	 * @wl_tbm: (none)
	 */
	void (*redirect)(struct wl_client *client,
			 struct wl_resource *resource,
			 struct wl_resource *wl_tbm);
	/**
	 * unredirect - (none)
	 */
	void (*unredirect)(struct wl_client *client,
			   struct wl_resource *resource);
	/**
	 * bind - (none)
	 * @surface: (none)
	 */
	void (*bind)(struct wl_client *client,
		     struct wl_resource *resource,
		     struct wl_resource *surface);
};

#define WL_TEST_REMOTE_UPDATE	0

#define WL_TEST_REMOTE_UPDATE_SINCE_VERSION	1

static inline void
wl_test_remote_send_update(struct wl_resource *resource_, struct wl_resource *buffer)
{
	wl_resource_post_event(resource_, WL_TEST_REMOTE_UPDATE, buffer);
}

#ifdef  __cplusplus
}
#endif

#endif
