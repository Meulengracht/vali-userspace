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

#include "protocols/wm_surface_protocol_server.h"
#include "protocols/wm_core_protocol_server.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_objects.h"
#include <errno.h>

void wm_surface_get_formats_callback(int client, struct wm_surface_get_formats_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    
}

void wm_surface_set_buffer_callback(int client, struct wm_surface_set_buffer_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    vioarr_buffer_t*  buffer  = vioarr_objects_get_object(input->buffer_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    if (!buffer) {
        wm_core_event_error_single(client, input->buffer_id, ENOENT, "wm_buffer: object does not exist");
        return;
    }
    
    vioarr_surface_set_buffer(surface, buffer);
}

void wm_surface_invalidate_callback(int client, struct wm_surface_invalidate_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_invalidate(surface, input->x, input->y, input->width, input->height);
}

void wm_surface_add_subsurface_callback(int client, struct wm_surface_add_subsurface_args* input)
{
    vioarr_surface_t* parent_surface = vioarr_objects_get_object(input->parent_id);
    vioarr_surface_t* child_surface  = vioarr_objects_get_object(input->child_id);
    int               status;
    if (!parent_surface) {
        wm_core_event_error_single(client, input->parent_id, ENOENT, "wm_surface: parent object does not exist");
        return;
    }
    
    if (!child_surface) {
        wm_core_event_error_single(client, input->child_id, ENOENT, "wm_surface: child object does not exist");
        return;
    }
    
    status = vioarr_surface_add_child(parent_surface, child_surface, input->x, input->y);
    if (status) {
        wm_core_event_error_single(client, input->parent_id, status, "wm_surface: failed to add surface as a child");
        return;
    }
}

void wm_surface_commit_callback(int client, struct wm_surface_commit_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_commit(surface);
}

void wm_surface_resize_callback(int client, struct wm_surface_resize_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    // todo
}

void wm_surface_move_callback(int client, struct wm_surface_move_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    
    // todo
}

void wm_surface_destroy_callback(int client, struct wm_surface_destroy_args* input)
{
    vioarr_surface_t* surface = vioarr_objects_get_object(input->surface_id);
    if (!surface) {
        wm_core_event_error_single(client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_destroy(surface);
}
