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

#include <glad/glad.h>
#include "backend/nanovg.h"
#include "backend/nanovg_gl.h"
#include <ds/list.h>
#include "vioarr_renderer.h"
#include "vioarr_screen.h"
#include "vioarr_surface.h"
#include "vioarr_manager.h"
#include "vioarr_utils.h"
#include <stdlib.h>
#include <threads.h>

typedef struct vioarr_renderer {
    NVGcontext*      context;
    vioarr_screen_t* screen;
    int              width;
    int              height;
    int              scale;
    int              rotation;
    float            pixel_ratio;
} vioarr_renderer_t;

static void opengl_initialize(int width, int height)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}

vioarr_renderer_t* vioarr_renderer_create(vioarr_screen_t* screen)
{
    vioarr_renderer_t* renderer;
    int                width  = vioarr_region_width(vioarr_screen_region(screen));
    int                height = vioarr_region_height(vioarr_screen_region(screen));
    
    renderer = (vioarr_renderer_t*)malloc(sizeof(vioarr_renderer_t));
    if (!renderer) {
        return NULL;
    }

    TRACE("[vioarr_renderer_create] initializing openGL");
    opengl_initialize(width, height);

    TRACE("[vioarr_renderer_create] creating nvg context");
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

void vioarr_renderer_render(vioarr_renderer_t* renderer)
{
    element_t*       i;
    list_t*          surfaces;
    list_t*          cursors;
    vioarr_region_t* drawRegion = vioarr_screen_region(renderer->screen);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    nvgBeginFrame(renderer->context, renderer->width, renderer->height, renderer->pixel_ratio);
    
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
    
    nvgEndFrame(renderer->context);
    glFinish();
}
