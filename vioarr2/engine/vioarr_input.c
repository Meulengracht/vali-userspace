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

#include <ds/list.h>
#include "vioarr_input.h"
#include "vioarr_surface.h"
#include "vioarr_manager.h"
#include "vioarr_objects.h"
#include "../protocols/wm_core_protocol.h"
#include "../protocols/wm_pointer_protocol.h"
#include "../protocols/wm_keyboard_protocol.h"
#include <stdlib.h>
#include <string.h>

typedef struct vioarr_input_source {
    element_t header;
    uint32_t  id;
    UUId_t    deviceId;
    int       type;
    union {
        struct {
            int               x;
            int               y;
            int               z;
            uint32_t          buttons;
            vioarr_surface_t* surface;
        } pointer;
    } state;
} vioarr_input_source_t;

static list_t inputDevices = LIST_INIT;

void vioarr_input_register(UUId_t deviceId, int type)
{
    vioarr_input_source_t* source;
    uint32_t               sourceId;

    source = malloc(sizeof(vioarr_input_source_t));
    if (!source) {
        return;
    }

    memset(source, 0, sizeof(vioarr_input_source_t));

    sourceId = vioarr_objects_create_server_object(source, object_type_pointer);

    ELEMENT_INIT(&source->header, (uintptr_t)deviceId, source);
    source->id       = sourceId;
    source->deviceId = deviceId;
    source->type     = type;

    list_append(&inputDevices, &source->header);
}

void vioarr_input_unregister(UUId_t deviceId)
{
    vioarr_input_source_t* source = list_find_value(&inputDevices, (void*)(uintptr_t)deviceId);
    if (!source) {
        return;
    }

    list_remove(&inputDevices, &source->header);
    if (source->state.pointer.surface) {
        vioarr_manager_demote_cursor(source->state.pointer.surface);
    }

    // destroy server object
    vioarr_objects_remove_object(-1, source->id);
    free(source);
}

void vioarr_input_set_surface(vioarr_input_source_t* input, vioarr_surface_t* surface)
{
    if (!input) {
        return;
    }

    if (input->state.pointer.surface) {
        vioarr_manager_demote_cursor(input->state.pointer.surface);
    }

    if (surface) {
        vioarr_manager_promote_cursor(surface);
    }

    // update the stored surface pointer
    input->state.pointer.surface = surface;
}

void vioarr_input_axis_event(UUId_t deviceId, int x, int y, int z)
{
    vioarr_input_source_t* source = list_find_value(&inputDevices, (void*)(uintptr_t)deviceId);
    vioarr_surface_t*      currentSurface;
    vioarr_surface_t*      surfaceAfterMove;
    int                    sendUpdates;
    if (!source) {
        return;
    }

    // ... we currently do not use z
    currentSurface   = vioarr_manager_surface_at(source->state.pointer.x, source->state.pointer.y);
    surfaceAfterMove = vioarr_manager_surface_at(source->state.pointer.x + x, source->state.pointer.y + y);
    sendUpdates      = vioarr_surface_supports_input(surfaceAfterMove, source->state.pointer.x + x, source->state.pointer.y + y);

    // send events
    if (currentSurface != surfaceAfterMove) {
        if (vioarr_surface_supports_input(currentSurface, source->state.pointer.x, source->state.pointer.y)) {
            wm_pointer_event_leave_single(
                vioarr_surface_client(currentSurface),
                source->id,
                vioarr_surface_id(currentSurface));
        }
        if (sendUpdates) {
            wm_pointer_event_enter_single(
                vioarr_surface_client(surfaceAfterMove),
                source->id,
                vioarr_surface_id(surfaceAfterMove),
                source->state.pointer.x + x,
                source->state.pointer.y + y);
            
            // skip move event
            sendUpdates = 0;
        }
    }

    // update values before proceeding
    source->state.pointer.x += x;
    source->state.pointer.y += y;
    source->state.pointer.z += z;

    // send move event
    if (sendUpdates) {
        // send move event with surface local coordinates
        vioarr_surface_t* region = vioarr_surface_region(surfaceAfterMove);
        wm_pointer_event_move_single(
            vioarr_surface_client(surfaceAfterMove),
            source->id,
            source->state.pointer.x - vioarr_region_x(region),
            source->state.pointer.y - vioarr_region_y(region));
    }
}

void vioarr_input_pointer_click(UUId_t deviceId, uint32_t buttons)
{
    vioarr_input_source_t* source = list_find_value(&inputDevices, (void*)(uintptr_t)deviceId);
    vioarr_surface_t*      clickedSurface;
    vioarr_surface_t*      currentSurface;
    if (!source) {
        return;
    }

    if (source->state.pointer.buttons == buttons) {
        return;
    }
    source->state.pointer.buttons = buttons;

    // get the current surface, check against current active one
    currentSurface = vioarr_manager_front_surface();
    clickedSurface = vioarr_manager_surface_at(source->state.pointer.x, source->state.pointer.y);
    if (clickedSurface != currentSurface) {
        // this means we clicked on a non-active surface, we want to active it
        vioarr_manager_push_to_front(clickedSurface);
        if (currentSurface) {
            vioarr_surface_focus(currentSurface, 0);
        }
        if (clickedSurface) {
            vioarr_surface_focus(currentSurface, 1);
        }
    }

    if (vioarr_surface_supports_input(clickedSurface, source->state.pointer.x, source->state.pointer.y)) {
        wm_pointer_event_click_single(vioarr_surface_client(surfaceAfterMove), source->id, buttons);
    }
}

void vioarr_input_keyboard_click(UUId_t deviceId, uint32_t keycode, uint32_t modifiers)
{
    vioarr_surface_t* currentSurface = vioarr_manager_front_surface();
    if (currentSurface) {
        wm_keyboard_event_key_single(vioarr_surface_client(currentSurface), keycode, modifiers);
    }
}
