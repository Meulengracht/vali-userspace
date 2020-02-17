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

#include "backend/nanovg.h"
#include <glad/glad.h>
#include "vioarr_renderer.h"
#include <stdlib.h>

typedef struct vioarr_renderer {
    NVGcontext* context;
    int         width;
    int         height;
    float       pixel_ratio;
} vioarr_renderer_t;

vioarr_renderer_t* vioarr_renderer_create(NVGcontext* context, vioarr_screen_t* screen)
{
    vioarr_renderer_t* renderer;
    
    renderer = (vioarr_renderer_t*)malloc(sizeof(vioarr_renderer_t));
    if (!renderer) {
        return NULL;
    }
    
    renderer->context     = context;
    renderer->width       = vioarr_screen_width(screen);
    renderer->height      = vioarr_screen_height(screen);
    renderer->pixel_ratio = (float)vioarr_screen_width(screen) / (float)vioarr_screen_height(screen);
    return renderer;
}

void vioarr_renderer_set_scale(vioarr_renderer_t* renderer, int scale)
{
    
}

void vioarr_renderer_set_transform(vioarr_renderer_t* renderer, int transform)
{
    
}

void vioarr_renderer_render(vioarr_renderer_t* renderer)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    nvgBeginFrame(renderer->context, renderer->width, renderer->height, renderer->pixel_ratio);
    // render(renderer->context)
    nvgEndFrame(renderer->context);
    
    glFinish();
}
