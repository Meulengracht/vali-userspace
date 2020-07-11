/* MollenOS
 *
 * Copyright 2020, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Vioarr - Vali Compositor
 * - Implements the default system compositor for Vali. It utilizies the gracht library
 *   for communication between compositor clients and the server. The server renders
 *   using Mesa3D with either the soft-renderer or llvmpipe render for improved performance.
 */

#include <errno.h>
#include <gracht/link/socket.h>
#include <gracht/link/vali.h>
#include <gracht/server.h>
#include <os/process.h>
#include <stdio.h>

#include "protocols/hid_events_protocol_client.h"
#include "protocols/wm_core_protocol_server.h"
#include "protocols/wm_screen_protocol_server.h"
#include "protocols/wm_memory_protocol_server.h"
#include "protocols/wm_memory_pool_protocol_server.h"
#include "protocols/wm_buffer_protocol_server.h"
#include "protocols/wm_surface_protocol_server.h"

#include "engine/vioarr_engine.h"
#include "engine/vioarr_utils.h"


extern void hid_events_event_key_event_callback(struct hid_events_key_event_event*);
extern void hid_events_event_pointer_event_callback(struct hid_events_pointer_event_event*);

static gracht_protocol_function_t hid_events_callbacks[2] = {
    { PROTOCOL_HID_EVENTS_EVENT_KEY_EVENT_ID , hid_events_event_key_event_callback },
    { PROTOCOL_HID_EVENTS_EVENT_POINTER_EVENT_ID , hid_events_event_pointer_event_callback },
};
DEFINE_HID_EVENTS_CLIENT_PROTOCOL(hid_events_callbacks, 2);

extern void wm_core_sync_callback(struct gracht_recv_message* message, struct wm_core_sync_args*);
extern void wm_core_get_objects_callback(struct gracht_recv_message* message);

static gracht_protocol_function_t wm_core_callbacks[2] = {
    { PROTOCOL_WM_CORE_SYNC_ID , wm_core_sync_callback },
    { PROTOCOL_WM_CORE_GET_OBJECTS_ID , wm_core_get_objects_callback },
};
DEFINE_WM_CORE_SERVER_PROTOCOL(wm_core_callbacks, 2);

extern void wm_screen_get_properties_callback(struct gracht_recv_message* message, struct wm_screen_get_properties_args*);
extern void wm_screen_get_modes_callback(struct gracht_recv_message* message, struct wm_screen_get_modes_args*);
extern void wm_screen_set_scale_callback(struct gracht_recv_message* message, struct wm_screen_set_scale_args*);
extern void wm_screen_set_transform_callback(struct gracht_recv_message* message, struct wm_screen_set_transform_args*);
extern void wm_screen_create_surface_callback(struct gracht_recv_message* message, struct wm_screen_create_surface_args*);

static gracht_protocol_function_t wm_screen_callbacks[5] = {
    { PROTOCOL_WM_SCREEN_GET_PROPERTIES_ID , wm_screen_get_properties_callback },
    { PROTOCOL_WM_SCREEN_GET_MODES_ID , wm_screen_get_modes_callback },
    { PROTOCOL_WM_SCREEN_SET_SCALE_ID , wm_screen_set_scale_callback },
    { PROTOCOL_WM_SCREEN_SET_TRANSFORM_ID , wm_screen_set_transform_callback },
    { PROTOCOL_WM_SCREEN_CREATE_SURFACE_ID , wm_screen_create_surface_callback },
};
DEFINE_WM_SCREEN_SERVER_PROTOCOL(wm_screen_callbacks, 5);

extern void wm_memory_create_pool_callback(struct gracht_recv_message* message, struct wm_memory_create_pool_args*);

static gracht_protocol_function_t wm_memory_callbacks[1] = {
    { PROTOCOL_WM_MEMORY_CREATE_POOL_ID , wm_memory_create_pool_callback },
};
DEFINE_WM_MEMORY_SERVER_PROTOCOL(wm_memory_callbacks, 1);

extern void wm_memory_pool_create_buffer_callback(struct gracht_recv_message* message, struct wm_memory_pool_create_buffer_args*);

static gracht_protocol_function_t wm_memory_pool_callbacks[1] = {
    { PROTOCOL_WM_MEMORY_POOL_CREATE_BUFFER_ID , wm_memory_pool_create_buffer_callback },
};
DEFINE_WM_MEMORY_POOL_SERVER_PROTOCOL(wm_memory_pool_callbacks, 1);

extern void wm_buffer_destroy_callback(struct gracht_recv_message* message, struct wm_buffer_destroy_args*);

static gracht_protocol_function_t wm_buffer_callbacks[1] = {
    { PROTOCOL_WM_BUFFER_DESTROY_ID , wm_buffer_destroy_callback },
};
DEFINE_WM_BUFFER_SERVER_PROTOCOL(wm_buffer_callbacks, 1);

extern void wm_surface_get_formats_callback(struct gracht_recv_message* message, struct wm_surface_get_formats_args*);
extern void wm_surface_set_buffer_callback(struct gracht_recv_message* message, struct wm_surface_set_buffer_args*);
extern void wm_surface_invalidate_callback(struct gracht_recv_message* message, struct wm_surface_invalidate_args*);
extern void wm_surface_set_drop_shadow_callback(struct gracht_recv_message* message, struct wm_surface_set_drop_shadow_args*);
extern void wm_surface_set_input_region_callback(struct gracht_recv_message* message, struct wm_surface_set_input_region_args*);
extern void wm_surface_add_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_add_subsurface_args*);
extern void wm_surface_request_frame_callback(struct gracht_recv_message* message, struct wm_surface_request_frame_args*);
extern void wm_surface_commit_callback(struct gracht_recv_message* message, struct wm_surface_commit_args*);
extern void wm_surface_resize_callback(struct gracht_recv_message* message, struct wm_surface_resize_args*);
extern void wm_surface_move_callback(struct gracht_recv_message* message, struct wm_surface_move_args*);
extern void wm_surface_destroy_callback(struct gracht_recv_message* message, struct wm_surface_destroy_args*);

static gracht_protocol_function_t wm_surface_callbacks[11] = {
    { PROTOCOL_WM_SURFACE_GET_FORMATS_ID , wm_surface_get_formats_callback },
    { PROTOCOL_WM_SURFACE_SET_BUFFER_ID , wm_surface_set_buffer_callback },
    { PROTOCOL_WM_SURFACE_INVALIDATE_ID , wm_surface_invalidate_callback },
    { PROTOCOL_WM_SURFACE_SET_DROP_SHADOW_ID , wm_surface_set_drop_shadow_callback },
    { PROTOCOL_WM_SURFACE_SET_INPUT_REGION_ID , wm_surface_set_input_region_callback },
    { PROTOCOL_WM_SURFACE_ADD_SUBSURFACE_ID , wm_surface_add_subsurface_callback },
    { PROTOCOL_WM_SURFACE_REQUEST_FRAME_ID , wm_surface_request_frame_callback },
    { PROTOCOL_WM_SURFACE_COMMIT_ID , wm_surface_commit_callback },
    { PROTOCOL_WM_SURFACE_RESIZE_ID , wm_surface_resize_callback },
    { PROTOCOL_WM_SURFACE_MOVE_ID , wm_surface_move_callback },
    { PROTOCOL_WM_SURFACE_DESTROY_ID , wm_surface_destroy_callback },
};
DEFINE_WM_SURFACE_SERVER_PROTOCOL(wm_surface_callbacks, 11);

int server_initialize(void)
{
    struct socket_server_configuration linkConfiguration;
    struct gracht_server_configuration serverConfiguration;
    int                                status;
    
    gracht_os_get_server_client_address(&linkConfiguration.server_address, &linkConfiguration.server_address_length);
    gracht_os_get_server_packet_address(&linkConfiguration.dgram_address, &linkConfiguration.dgram_address_length);
    gracht_link_socket_server_create(&serverConfiguration.link, &linkConfiguration);
    
    status = gracht_server_initialize(&serverConfiguration);
    if (status) {
        printf("error initializing server library %i", errno);
    }
    return status;
}

int server_run(void)
{
#ifdef VIOARR_WINTEST
    UUId_t pid;
    
    // Spawn the launcher application
    ProcessSpawn("$bin/wintest.app", NULL, &pid);
#endif //VIOARR_WINTEST
    
    // Call the main sever loop
    return gracht_server_main_loop();
}

/*******************************************
 * Entry Point
 *******************************************/
int main(int argc, char **argv)
{
    int status = server_initialize();
    if (status) {
        return status;
    }
    
    status = vioarr_engine_initialize();
    if (status) {
        return status;
    }
    
    //gracht_server_register_protocol(&hid_events_protocol);
    gracht_server_register_protocol(&wm_core_server_protocol);
    gracht_server_register_protocol(&wm_screen_server_protocol);
    gracht_server_register_protocol(&wm_memory_server_protocol);
    gracht_server_register_protocol(&wm_memory_pool_server_protocol);
    gracht_server_register_protocol(&wm_buffer_server_protocol);
    gracht_server_register_protocol(&wm_surface_server_protocol);
    return server_run();
}
