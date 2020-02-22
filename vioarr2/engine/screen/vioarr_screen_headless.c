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

#include <ddk/contracts/video.h>
#include <glad/glad.h>
#include <GL/osmesa.h>
#include "../vioarr_renderer.h"
#include "../vioarr_screen.h"
#include "../../protocols/wm_screen_protocol_server.h"
#include <stdlib.h>

typedef struct vioarr_screen {
    OSMesaContext      context;
    void*              backbuffer;
    size_t             backbuffer_size;
    int                depth_bits;
    vioarr_region_t*   dimensions;
    vioarr_renderer_t* renderer;
} vioarr_screen_t;

vioarr_screen_t* vioarr_screen_create(NVGcontext* context, VideoDescriptor_t* video)
{
    vioarr_screen_t* screen;
    int attributes[100], n = 0;

    attributes[n++] = OSMESA_FORMAT;
    attributes[n++] = OSMESA_BGRA;
    attributes[n++] = OSMESA_DEPTH_BITS;
    attributes[n++] = 24;
    attributes[n++] = OSMESA_STENCIL_BITS;
    attributes[n++] = 8;
    attributes[n++] = OSMESA_ACCUM_BITS;
    attributes[n++] = 0;
    attributes[n++] = OSMESA_PROFILE;
    attributes[n++] = OSMESA_CORE_PROFILE;
    attributes[n++] = OSMESA_CONTEXT_MAJOR_VERSION;
    attributes[n++] = 3;
    attributes[n++] = OSMESA_CONTEXT_MINOR_VERSION;
    attributes[n++] = 3;
    attributes[n++] = 0;
    
    screen->context = OSMesaCreateContextAttribs(&attributes[0], NULL);
    screen->dimensions = vioarr_region_create();
    screen->depth_bits = video->Depth;
    vioarr_region_add(screen->dimensions, 0, 0, video->Width, video->Height);
    
    screen->backbuffer_size = video->Width * video->Height * 4 * sizeof(GLubyte);
    screen->backbuffer      = aligned_alloc(32, screen->backbuffer_size);
    if (!screen->backbuffer) {
        free(screen);
        return NULL;
    }
    
    screen->renderer = vioarr_renderer_create(context, screen);
    if (!screen->renderer) {
        free(screen->backbuffer);
        free(screen);
        return NULL;
    }
    return screen;
}

void vioarr_screen_set_scale(vioarr_screen_t* screen, int scale)
{
    // do nothing
}

void vioarr_screen_set_transform(vioarr_screen_t* screen, enum wm_screen_transform transform)
{
    // do nothing
}

vioarr_region_t* vioarr_screen_region(vioarr_screen_t* screen)
{
    if (!screen) {
        return NULL;
    }
    return screen->dimensions;
}

int vioarr_screen_scale(vioarr_screen_t* screen)
{
    if (!screen) {
        return 1;
    }
    return 1;
}

enum wm_screen_transform vioarr_screen_transform(vioarr_screen_t* screen)
{
    return no_transform;
}

int vioarr_screen_publish_modes(vioarr_screen_t* screen, int client)
{
    if (!screen) {
        return -1;
    }
    
    // One hardcoded format
    return wm_screen_event_mode_single(client, mode_current | mode_preferred,
        vioarr_region_width(screen->dimensions), vioarr_region_height(screen->dimensions), 60);
}

void vioarr_screen_register_surface(vioarr_screen_t* screen, vioarr_surface_t* surface)
{
    if (!screen) {
        return;
    }
    vioarr_renderer_register_surface(screen->renderer, surface);
}

void vioarr_screen_unregister_surface(vioarr_screen_t* screen, vioarr_surface_t* surface)
{
    if (!screen) {
        return;
    }
    vioarr_renderer_unregister_surface(screen->renderer, surface);
}

void vioarr_screen_frame(vioarr_screen_t* screen)
{
    OSMesaMakeCurrent(screen->context, screen->backbuffer,
        GL_UNSIGNED_BYTE, vioarr_region_width(screen->dimensions), vioarr_region_height(screen->dimensions));
    vioarr_renderer_render(screen->renderer);
    // no present logic in headless
}
