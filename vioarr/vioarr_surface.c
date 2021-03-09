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
#include "engine/vioarr_input.h"
#include "engine/vioarr_surface.h"
#include "engine/vioarr_screen.h"
#include "engine/vioarr_objects.h"
#include "engine/vioarr_manager.h"
#include "engine/vioarr_utils.h"
#include <errno.h>

void wm_surface_get_formats_callback(struct gracht_recv_message* message, struct wm_surface_get_formats_args* input)
{
    TRACE("[wm_surface_get_formats_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    ERROR("[wm_surface_get_formats_callback] FIXME: STUB FUNCTION");
}

void wm_surface_set_buffer_callback(struct gracht_recv_message* message, struct wm_surface_set_buffer_args* input)
{
    TRACE("[wm_surface_set_buffer_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    vioarr_buffer_t*  buffer  = vioarr_objects_get_object(message->client, input->buffer_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    if (!buffer) {
        wm_core_event_error_single(message->client, input->buffer_id, ENOENT, "wm_buffer: object does not exist");
        return;
    }
    
    vioarr_surface_set_buffer(surface, buffer);
}

void wm_surface_set_input_region_callback(struct gracht_recv_message* message, struct wm_surface_set_input_region_args* input)
{
    TRACE("[wm_surface_set_input_region_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_set_input_region(surface, input->x, input->y, input->width, input->height);
}

void wm_surface_request_fullscreen_mode(struct gracht_recv_message* message, struct wm_surface_request_fullscreen_mode_args* input)
{
    TRACE("[wm_surface_request_fullscreen_mode] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    switch (input->mode) {
        case fs_mode_exit: {
            vioarr_manager_change_level(surface, 1);
            vioarr_surface_restore_size(surface);
        } break;

        case fs_mode_normal: {
            vioarr_surface_maximize(surface);
        } break;

        case fs_mode_full: {
            vioarr_manager_change_level(surface, 2);
            vioarr_surface_maximize(surface);
        } break;

        default: break;
    }
}

void wm_surface_request_level(struct gracht_recv_message* message, struct wm_surface_request_level_args* input)
{
    TRACE("[wm_surface_request_level] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_manager_change_level(surface, input->level);
}

void wm_surface_request_frame_callback(struct gracht_recv_message* message, struct wm_surface_request_frame_args* input)
{
    TRACE("[wm_surface_request_frame_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_request_frame(surface);
}

void wm_surface_invalidate_callback(struct gracht_recv_message* message, struct wm_surface_invalidate_args* input)
{
    TRACE("[wm_surface_invalidate_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_invalidate(surface, input->x, input->y, input->width, input->height);
}

void wm_surface_set_transparency_callback(struct gracht_recv_message* message, struct wm_surface_set_transparency_args* input)
{
    TRACE("[wm_surface_set_transparency_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_set_transparency(surface, input->enable);
}

void wm_surface_set_drop_shadow_callback(struct gracht_recv_message* message, struct wm_surface_set_drop_shadow_args* input)
{
    TRACE("[wm_surface_set_drop_shadow_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_set_drop_shadow(surface, input->x, input->y, input->width, input->height);
}

void wm_surface_add_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_add_subsurface_args* input)
{
    TRACE("[wm_surface_add_subsurface_callback] client %i, surface %u", message->client, input->parent_id);
    vioarr_surface_t* parent_surface = vioarr_objects_get_object(message->client, input->parent_id);
    vioarr_surface_t* child_surface  = vioarr_objects_get_object(message->client, input->child_id);
    int               status;
    if (!parent_surface) {
        wm_core_event_error_single(message->client, input->parent_id, ENOENT, "wm_surface: parent object does not exist");
        return;
    }
    
    if (!child_surface) {
        wm_core_event_error_single(message->client, input->child_id, ENOENT, "wm_surface: child object does not exist");
        return;
    }

    // unregister the surface
    vioarr_manager_unregister_surface(child_surface);
    
    status = vioarr_surface_add_child(parent_surface, child_surface, input->x, input->y);
    if (status) {
        wm_core_event_error_single(message->client, input->parent_id, status, "wm_surface: failed to add surface as a child");
        return;
    }
}

void wm_surface_resize_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_resize_subsurface_args* input)
{
    TRACE("[wm_surface_resize_subsurface_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_resize(surface, input->width, input->height, no_edges);
}

void wm_surface_move_subsurface_callback(struct gracht_recv_message* message, struct wm_surface_move_subsurface_args* input)
{
    TRACE("[wm_surface_move_subsurface_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_move_absolute(surface, input->x_in_parent, input->y_in_parent);
}

void wm_surface_commit_callback(struct gracht_recv_message* message, struct wm_surface_commit_args* input)
{
    TRACE("[wm_surface_commit_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    
    vioarr_surface_commit(surface);
}

void wm_surface_resize_callback(struct gracht_recv_message* message, struct wm_surface_resize_args* input)
{
    TRACE("[wm_surface_resize_callback] client %i, pointer %u, surface %u", message->client, input->pointer_id, input->surface_id);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, input->surface_id);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, input->pointer_id);
    
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    if (!pointer) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_pointer: object does not exist");
        return;
    }
    
    vioarr_input_request_resize(pointer, surface, input->edges);
}

void wm_surface_move_callback(struct gracht_recv_message* message, struct wm_surface_move_args* input)
{
    TRACE("[wm_surface_move_callback] client %i, pointer %u, surface %u", message->client, input->pointer_id, input->surface_id);
    vioarr_surface_t*      surface = vioarr_objects_get_object(message->client, input->surface_id);
    vioarr_input_source_t* pointer = vioarr_objects_get_object(message->client, input->pointer_id);
    
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }
    if (!pointer) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_pointer: object does not exist");
        return;
    }
    
    vioarr_input_request_move(pointer, surface);
}

void wm_surface_destroy_callback(struct gracht_recv_message* message, struct wm_surface_destroy_args* input)
{
    TRACE("[wm_surface_destroy_callback] client %i, surface %u", message->client, input->surface_id);
    vioarr_surface_t* surface = vioarr_objects_get_object(message->client, input->surface_id);
    if (!surface) {
        wm_core_event_error_single(message->client, input->surface_id, ENOENT, "wm_surface: object does not exist");
        return;
    }

    vioarr_input_on_surface_destroy(surface);
    vioarr_manager_unregister_surface(surface);
    vioarr_surface_destroy(surface);
}
