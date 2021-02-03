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

#ifdef VIOARR_BACKEND_NANOVG
#include <glad/glad.h>
#include "backend/backend.h"
#include "backend/nanovg/nanovg_gl.h"
#endif

#ifdef VIOARR_BACKEND_BLEND2D
#include <blend2d.h>
#endif

#include <ds/list.h>
#include "vioarr_buffer.h"
#include "vioarr_renderer.h"
#include "vioarr_screen.h"
#include "vioarr_surface.h"
#include "vioarr_manager.h"
#include "vioarr_utils.h"
#include <stdlib.h>
#include <threads.h>

typedef struct vioarr_renderer {
#ifdef VIOARR_BACKEND_NANOVG
    vcontext_t*      context;
    mtx_t            context_sync;
#endif
    vioarr_screen_t* screen;
    int              width;
    int              height;
    int              scale;
    int              rotation;
    float            pixel_ratio;
} vioarr_renderer_t;

#ifdef VIOARR_BACKEND_NANOVG
static void opengl_initialize(int width, int height)
{
    // 0x28575A
    glClearColor(0.15f, 0.34f, 0.35f, 1.0f);
}
#endif

vioarr_renderer_t* vioarr_renderer_create(vioarr_screen_t* screen)
{
    vioarr_renderer_t* renderer;
    int                width  = vioarr_region_width(vioarr_screen_region(screen));
    int                height = vioarr_region_height(vioarr_screen_region(screen));
    
    renderer = (vioarr_renderer_t*)malloc(sizeof(vioarr_renderer_t));
    if (!renderer) {
        return NULL;
    }

#ifdef VIOARR_BACKEND_NANOVG
    TRACE("[vioarr_renderer_create] initializing openGL");
    opengl_initialize(width, height);

    TRACE("[vioarr_renderer_create] creating nvg context");
    mtx_init(&renderer->context_sync, mtx_plain);
#ifdef __VIOARR_CONFIG_RENDERER_MSAA
	renderer->context = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#else
	renderer->context = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
    if (!renderer->context) {
        ERROR("[vioarr_renderer_create] failed to create the nvg context");
        free(renderer);
        return NULL;
    }
#endif

    renderer->screen      = screen;
    renderer->width       = width;
    renderer->height      = height;
    renderer->pixel_ratio = (float)width / (float)height;
    renderer->scale       = 1;
    renderer->rotation    = 0;
    
    return renderer;
}

void vioarr_renderer_set_scale(vioarr_renderer_t* renderer, int scale)
{
    if (!renderer) {
        return;
    }
    
    renderer->scale = scale;
}

void vioarr_renderer_set_rotation(vioarr_renderer_t* renderer, int rotation)
{
    if (!renderer) {
        return;
    }
    
    renderer->rotation = rotation;
}

int vioarr_renderer_scale(vioarr_renderer_t* renderer)
{
    if (!renderer) {
        return 1;
    }
    return renderer->scale;
}

int vioarr_renderer_rotation(vioarr_renderer_t* renderer)
{
    if (!renderer) {
        return 0;
    }
    return renderer->rotation;
}

int vioarr_renderer_create_image(vioarr_renderer_t* renderer, vioarr_buffer_t* buffer)
{
    int resourceId = -1;

    if (!renderer) {
        return -1;
    }

    if (!vioarr_buffer_data(buffer)) {
        return -1;
    }

#ifdef VIOARR_BACKEND_NANOVG
    mtx_lock(&renderer->context_sync);
    resourceId = nvgCreateImageRGBA(renderer->context,
            vioarr_buffer_width(buffer), vioarr_buffer_height(buffer),
            /* NVG_IMAGE_FLIPY */ 0,
            (const uint8_t*)vioarr_buffer_data(buffer));
    mtx_unlock(&renderer->context_sync);
#endif
    return resourceId;
}

void vioarr_renderer_destroy_image(vioarr_renderer_t* renderer, int resourceId)
{
    if (!renderer) {
        return;
    }

#ifdef VIOARR_BACKEND_NANOVG
    mtx_lock(&renderer->context_sync);
    nvgDeleteImage(renderer->context, resourceId);
    mtx_unlock(&renderer->context_sync);
#endif
}

void vioarr_renderer_render(vioarr_renderer_t* renderer)
{
    element_t*       i;
    list_t*          surfaces;
    list_t*          cursors;
    vioarr_region_t* drawRegion = vioarr_screen_region(renderer->screen);
    
#ifdef VIOARR_BACKEND_NANOVG
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    mtx_lock(&renderer->context_sync);
    nvgBeginFrame(renderer->context, renderer->width, renderer->height, renderer->pixel_ratio);
    nvgGlobalCompositeBlendFunc(renderer->context, NVG_SRC_ALPHA, NVG_ONE_MINUS_SRC_ALPHA);
#endif

#ifdef VIOARR_BACKEND_BLEND2D
    BLContextCore context;
    blContextInitAs(&context, img, NULL);
#endif

    vioarr_manager_render_start(&surfaces, &cursors);
    _foreach(i, surfaces) {
        vioarr_surface_t* surface = i->value;
        if (vioarr_region_intersects(drawRegion, vioarr_surface_region(surface))) {
            vioarr_surface_render(renderer->context, surface);
        }
    }
    
    _foreach(i, cursors) {
        vioarr_surface_t* surface = i->value;
        if (vioarr_region_intersects(drawRegion, vioarr_surface_region(surface))) {
            vioarr_surface_render(renderer->context, surface);
        }
    }
    vioarr_manager_render_end();
    
#ifdef VIOARR_BACKEND_NANOVG
    nvgEndFrame(renderer->context);
    mtx_unlock(&renderer->context_sync);

    glFinish();
#endif
}
