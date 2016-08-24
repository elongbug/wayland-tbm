#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <wayland-server.h>
#include <wayland-tbm-server.h>
#include <tbm_surface.h>

#include "wayland-tbm-test-server-protocol.h"

#define SERVER_LOG(fmt, ...)   fprintf (stderr, "[SERVER(%d):%s] " fmt, getpid(), __func__, ##__VA_ARGS__)

typedef struct {
	struct wl_display *dpy;
	struct wayland_tbm_server *tbm_server;
	struct wl_global *wl_tbm_test;

	struct wl_resource *provider;
}Server;

typedef struct {
	struct wl_resource *wl_surface;
	struct wl_resource *wl_surface_bind;
	struct wl_resource *front;

	struct wl_list remotes;
}Surface;

typedef struct {
	struct wl_resource *wl_remote_surface;
	struct wl_resource *wl_tbm;
	struct wl_resource *front;
	Surface *provider;
	
	struct wl_list link;
}RemoteSurface;

Server gServer;

static void
_wl_test_remote_destroy(struct wl_resource *resource)
{
	RemoteSurface *remote = (RemoteSurface*)wl_resource_get_user_data(resource);

	if (remote->provider) {
		wl_list_remove(&remote->link);
		SERVER_LOG("%p(%p)\n", resource, remote);
	}
	
	free(remote);
}

static void
_wl_test_remote_destroy_cb(struct wl_client *client,
			struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
_wl_test_remote_release_cb(struct wl_client *client,
			struct wl_resource *resource,
			struct wl_resource *buffer)
{
}

static void
_wl_test_remote_redirect_cb(struct wl_client *client,
			 struct wl_resource *resource,
			 struct wl_resource *wl_tbm)
{
	RemoteSurface *remote = wl_resource_get_user_data(resource);
	remote->wl_tbm = wl_tbm;
	SERVER_LOG("%p, wl_tbm: %p\n", resource, wl_tbm);
}

static void
_wl_test_remote_unredirect_cb(struct wl_client *client,
			   struct wl_resource *resource)
{
	RemoteSurface *remote = wl_resource_get_user_data(resource);
	remote->wl_tbm = NULL;
	SERVER_LOG("%p\n", resource);
}

static void
_wl_test_remote_bind_cb(struct wl_client *client,
		     struct wl_resource *resource,
		     struct wl_resource *surface)
{
	RemoteSurface *remote = wl_resource_get_user_data(resource);
	SERVER_LOG("%p(%p) to surface:%p\n", resource, remote, surface);
}

static const struct wl_test_remote_interface wl_test_remote_impl = {
	_wl_test_remote_destroy_cb,
	_wl_test_remote_release_cb,
	_wl_test_remote_redirect_cb,
	_wl_test_remote_unredirect_cb,
	_wl_test_remote_bind_cb
};

static void
_wl_test_surface_destroy(struct wl_resource *resource)
{
	Surface *surface = (Surface *)wl_resource_get_user_data(resource);

	SERVER_LOG("%p provider:%p\n", resource, gServer.provider);
	
	if (!wl_list_empty(&surface->remotes)) {
		RemoteSurface *pos, *tmp;
		
		wl_list_for_each_safe(pos, tmp,&surface->remotes, link) {
			pos->provider = NULL;
			wl_list_remove(&pos->link);
		}
	}

	if (gServer.provider == resource)
		gServer.provider = NULL;

	free(surface);
}


static void
_wl_test_surface_destroy_cb(struct wl_client *client,
			    struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
_wl_test_surface_attach_cb(struct wl_client *client,
			   struct wl_resource *resource,
			   struct wl_resource *wl_buffer)
{
	Surface *surface = (Surface*)wl_resource_get_user_data(resource);

	if (surface->front && (surface->front != wl_buffer))
		wl_buffer_send_release(surface->front);
	
	surface->front = wl_buffer;
	if (!wl_list_empty(&surface->remotes)) {
		RemoteSurface *pos;
		struct wl_resource *wl_remote_buffer = NULL;
		
		wl_list_for_each(pos, &surface->remotes, link) {
			if (pos->wl_tbm) {
				wl_remote_buffer = wayland_tbm_server_get_remote_buffer(wl_buffer, pos->wl_tbm);
				if (!wl_remote_buffer) {
					tbm_surface_h tbm_surface;
					
					tbm_surface = wayland_tbm_server_get_surface(NULL, wl_buffer);
					wl_remote_buffer = wayland_tbm_server_export_buffer(pos->wl_tbm, tbm_surface);
					SERVER_LOG("export: wl_tbm:%p, tbm_surf:%p, wl_buf:%p\n", pos->wl_tbm, tbm_surface, wl_remote_buffer);
				}

				if (wl_remote_buffer) {
					wl_test_remote_send_update(pos->wl_remote_surface, wl_remote_buffer);
					pos->front = wl_remote_buffer;
				}
			}
		}
	}
}

static void
_wl_test_surface_frame_cb(struct wl_client *client,
			  struct wl_resource *resource,
			  uint32_t callback)
{
}

static const struct wl_test_surface_interface wl_test_surface_impl = {
	_wl_test_surface_destroy_cb,
	_wl_test_surface_attach_cb,
	_wl_test_surface_frame_cb
};

static void 
_wl_tbm_test_create_surface(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t id)
{
	Surface *surface;

	surface = calloc(1, sizeof(Surface));
	wl_list_init(&surface->remotes);
	surface->wl_surface = wl_resource_create(client,
						&wl_test_surface_interface, 1, id);
	wl_resource_set_implementation(surface->wl_surface, 
						&wl_test_surface_impl, surface,
						_wl_test_surface_destroy);
}

static void
_wl_tbm_test_set_active_queue(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_resource *surface)
{
}

static void
_wl_tbm_test_set_provider(struct wl_client *client,
			     struct wl_resource *resource,
			     struct wl_resource *surface,
			     const char *name)
{
	Server *srv = (Server *)wl_resource_get_user_data(resource);
	srv->provider = surface;

	SERVER_LOG("Provider:%p\n", resource);
}

static void
_wl_tbm_create_remote_surface(struct wl_client *client,
				      struct wl_resource *resource,
				      uint32_t id,
				      const char *name)
{
	Server *srv = (Server *)wl_resource_get_user_data(resource);
	Surface *provider;
	RemoteSurface *remote;

	if (!srv->provider) return;
	provider = wl_resource_get_user_data(srv->provider);

	remote = calloc(1, sizeof(RemoteSurface));
	remote->provider = provider;
	remote->wl_remote_surface = wl_resource_create(client,
									&wl_test_remote_interface,
									1,
									id);
	wl_resource_set_implementation(remote->wl_remote_surface,
				&wl_test_remote_impl,
				remote, 
				_wl_test_remote_destroy);
	wl_list_init(&remote->link);
	wl_list_insert(&provider->remotes, &remote->link);
	SERVER_LOG("Add remote:%p(%p) to provider:%p(%p)\n",
		resource, remote,
		srv->provider, provider);
}

static const struct wl_tbm_test_interface wl_tbm_test_impl = {
	_wl_tbm_test_create_surface,
	_wl_tbm_test_set_active_queue,
	_wl_tbm_test_set_provider,
	_wl_tbm_create_remote_surface
};

static void
wl_tbm_test_bind_cb(struct wl_client *client, void *data,
		    uint32_t version, uint32_t id)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, &wl_tbm_test_interface, version, id);
	wl_resource_set_implementation(resource, &wl_tbm_test_impl, data, NULL);

	SERVER_LOG("client:%p, wl_tbm_test:%p\n", client, resource);
}

int
main(int argc, char *argv[])
{
	struct wl_display *dpy;
	struct wayland_tbm_server *tbm_server;

	const char *dpy_name = "tbm_remote";

	dpy = wl_display_create();
	if (!dpy) {
		printf("[SRV] failed to create display\n");
		return -1;
	}

	wl_display_add_socket(dpy, dpy_name);
	printf("[SRV] wl_display : %s\n", dpy_name);

	tbm_server = wayland_tbm_server_init(dpy, NULL, -1, 0);
	if (!tbm_server) {
		printf("[SRV] failed to tbm_server init\n");
		wl_display_destroy(dpy);
		return -1;
	}

	gServer.dpy = dpy;
	gServer.tbm_server = tbm_server;
	gServer.wl_tbm_test = wl_global_create(dpy, &wl_tbm_test_interface, 1, &gServer,
					    wl_tbm_test_bind_cb);
	
	wl_display_run(dpy);

	return 0;
}
