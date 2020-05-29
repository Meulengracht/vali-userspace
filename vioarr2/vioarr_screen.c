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

#include "protocols/wm_screen_protocol_server.h"
#include "protocols/wm_core_protocol_server.h"
#include "engine/vioarr_screen.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_utils.h"
#include <gracht/server.h>
#include <errno.h>

void wm_screen_get_properties_callback(struct gracht_recv_message* message, struct wm_screen_get_properties_args* input)
{
    vioarr_utils_trace("[wm_screen_get_properties_callback] client %i", message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(input->screen_id);
    vioarr_region_t* region;
    
    if (!screen) {
        wm_core_event_error_single(message->client, input->screen_id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    region = vioarr_screen_region(screen);
    wm_screen_event_screen_properties_single(message->client,
        input->screen_id, vioarr_region_x(region), vioarr_region_y(region),
        vioarr_screen_transform(screen), vioarr_screen_scale(screen));
}

void wm_screen_get_modes_callback(struct gracht_recv_message* message, struct wm_screen_get_modes_args* input)
{
    vioarr_utils_trace("[wm_screen_get_modes_callback] client %i", message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(input->screen_id);
    int              status;
    if (!screen) {
        wm_core_event_error_single(message->client, input->screen_id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    status = vioarr_screen_publish_modes(screen, message->client);
    if (status) {
        wm_core_event_error_single(message->client, input->screen_id, status, "wm_screen: failed to publish modes");
    }
}

void wm_screen_set_scale_callback(struct gracht_recv_message* message, struct wm_screen_set_scale_args* input)
{
    vioarr_utils_trace("[wm_screen_set_scale_callback] client %i", message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(input->screen_id);
    if (!screen) {
        wm_core_event_error_single(message->client, input->screen_id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    vioarr_screen_set_scale(screen, input->scale);
}

void wm_screen_set_transform_callback(struct gracht_recv_message* message, struct wm_screen_set_transform_args* input)
{
    vioarr_utils_trace("[wm_screen_set_transform_callback] client %i", message->client);
    vioarr_screen_t* screen = vioarr_objects_get_object(input->screen_id);
    if (!screen) {
        wm_core_event_error_single(message->client, input->screen_id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    vioarr_screen_set_transform(screen, input->transform);
}

void wm_screen_create_surface_callback(struct gracht_recv_message* message, struct wm_screen_create_surface_args* input)
{
    vioarr_utils_trace("[wm_screen_create_surface_callback] client %i", message->client);
    vioarr_screen_t*  screen = vioarr_objects_get_object(input->screen_id);
    vioarr_surface_t* surface;
    int               status;
    if (!screen) {
        wm_core_event_error_single(message->client, input->screen_id, ENOENT, "wm_screen: object does not exist");
        return;
    }
    
    status = vioarr_surface_create(message->client, input->surface_id, screen,
        100, 100, input->width, input->height, &surface);
    if (status) {
        wm_core_event_error_single(message->client, input->screen_id, status, "wm_screen: failed to create surface");
        return;
    }
    
    wm_core_event_object_single(message->client, input->surface_id, UUID_INVALID, object_type_surface);
}
