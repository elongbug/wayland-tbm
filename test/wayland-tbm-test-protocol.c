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

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_buffer_interface;
extern const struct wl_interface wl_callback_interface;
extern const struct wl_interface wl_tbm_interface;
extern const struct wl_interface wl_test_remote_interface;
extern const struct wl_interface wl_test_surface_interface;

static const struct wl_interface *types[] = {
	&wl_test_surface_interface,
	&wl_test_surface_interface,
	&wl_test_surface_interface,
	NULL,
	&wl_test_remote_interface,
	NULL,
	&wl_buffer_interface,
	&wl_callback_interface,
	&wl_buffer_interface,
	&wl_tbm_interface,
	&wl_test_surface_interface,
	&wl_buffer_interface,
};

static const struct wl_message wl_tbm_test_requests[] = {
	{ "create_surface", "n", types + 0 },
	{ "set_active_queue", "o", types + 1 },
	{ "set_provider", "os", types + 2 },
	{ "create_remote_surface", "ns", types + 4 },
};

WL_EXPORT const struct wl_interface wl_tbm_test_interface = {
	"wl_tbm_test", 1,
	4, wl_tbm_test_requests,
	0, NULL,
};

static const struct wl_message wl_test_surface_requests[] = {
	{ "destroy", "", types + 0 },
	{ "attach", "o", types + 6 },
	{ "frame", "n", types + 7 },
};

WL_EXPORT const struct wl_interface wl_test_surface_interface = {
	"wl_test_surface", 1,
	3, wl_test_surface_requests,
	0, NULL,
};

static const struct wl_message wl_test_remote_requests[] = {
	{ "destroy", "", types + 0 },
	{ "release", "o", types + 8 },
	{ "redirect", "o", types + 9 },
	{ "unredirect", "", types + 0 },
	{ "bind", "o", types + 10 },
};

static const struct wl_message wl_test_remote_events[] = {
	{ "update", "o", types + 11 },
};

WL_EXPORT const struct wl_interface wl_test_remote_interface = {
	"wl_test_remote", 1,
	5, wl_test_remote_requests,
	1, wl_test_remote_events,
};

