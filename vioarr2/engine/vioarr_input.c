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
#include <stdlib.h>
#include <string.h>

typedef struct vioarr_input_source {
    element_t header;
    uint32_t  id;
    UUId_t    deviceId;
    int       type;
    union {
        struct {
            int      x;
            int      y;
            int      z;
            uint32_t buttons;
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

    source->id       = sourceId;
    source->deviceId = deviceId;
    source->type     = type;
}

void vioarr_input_unregister(UUId_t deviceId)
{

}

void vioarr_input_set_surface(vioarr_input_source_t* input)
{
    // unregister the surface from the renderer
    // vioarr_screen_unregister_surface(vioarr_surface_screen(child_surface), child_surface);

    // register the surface as the cursor
}

void vioarr_input_axis_event(UUId_t deviceId, int x, int y, int z)
{
    vioarr_input_source_t* source = list_find_value(&inputDevices, (void*)(uintptr_t)deviceId);
    vioarr_surface_t*      currentSurface;
    vioarr_surface_t*      surfaceAfterMove;
    vioarr_surface_t*      eventSurface;
    if (!source) {
        return;
    }

    // ... we currently do not use z
    currentSurface   = vioarr_manager_surface_at(source->state.pointer.x, source->state.pointer.y);
    surfaceAfterMove = vioarr_manager_surface_at(source->state.pointer.x + x, source->state.pointer.y + y);
    eventSurface     = vioarr_surface_input_at(surfaceAfterMove, x, y);

    // update values before proceeding
    source->state.pointer.x += x;
    source->state.pointer.y += y;
    source->state.pointer.z += z;

    // send events
    if (currentSurface != surfaceAfterMove) {
        if (currentSurface) {
            // send leave
        }
        if (surfaceAfterMove) {
            // send enter
        }
    }

    // send move event
    if (eventSurface) {
        // send move event with surface local coordinates
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

    // get the current surface, check against current active one
    clickedSurface = vioarr_manager_surface_at(source->state.pointer.x, source->state.pointer.y);
    currentSurface = vioarr_manager_front_surface();
    if (clickedSurface != currentSurface) {
        // this means we clicked on a non-active surface, we want to active it
        vioarr_manager_push_to_front(clickedSurface);
        if (currentSurface) {
            // send focus event
        }
        if (clickedSurface) {
            // send focus event
        }
    }
    source->state.pointer.buttons = buttons;
}

void vioarr_input_keyboard_click(UUId_t deviceId, uint8_t keycode, uint32_t flags)
{
    vioarr_surface_t* currentSurface = vioarr_manager_front_surface();
    if (currentSurface) {
        // send event
    }

    
}
