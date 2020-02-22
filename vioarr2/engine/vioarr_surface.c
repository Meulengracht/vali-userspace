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

#include "vioarr_surface.h"
#include "vioarr_region.h"
#include "vioarr_buffer.h"
#include "vioarr_objects.h"
#include "vioarr_screen.h"
#include <stdlib.h>
#include <string.h>

typedef struct vioarr_surface_properties {
    int drop_shadow;
    int corner_radius;
    int resource_id;
    
    vioarr_region_t*       dirt;
    vioarr_buffer_t*       content;
    struct vioarr_surface* link;
    struct vioarr_surface* children;
} vioarr_surface_properties_t;

typedef struct vioarr_surface {
    uint32_t         id;
    vioarr_screen_t* screen;
    vioarr_region_t* dimensions;
    int              visible;
    int              swap_properties;
    
    vioarr_surface_properties_t properties;
    vioarr_surface_properties_t pending_properties;
} vioarr_surface_t;

static void vioarr_surface_update(NVGcontext* context, vioarr_surface_t* surface);
static void render_drop_shadow(NVGcontext* context, vioarr_surface_t* surface);

int vioarr_surface_create(vioarr_screen_t* screen, int x, int y, int width, int height, vioarr_surface_t** surface_out)
{
    vioarr_surface_t* surface;
    
    if (!screen) {
        return -1;
    }
    
    surface = malloc(sizeof(vioarr_surface_t));
    if (!surface) {
        return -1;
    }
    
    memset(surface, 0, sizeof(vioarr_surface_t));
    
    surface->dimensions = vioarr_region_create();
    if (!surface->dimensions) {
        free(surface);
        return -1;
    }
    vioarr_region_add(surface->dimensions, x, y, width, height);
    
    surface->properties.dirt = vioarr_region_create();
    if (!surface->properties.dirt) {
        free(surface->dimensions);
        free(surface);
        return -1;
    }
    
    surface->pending_properties.dirt = vioarr_region_create();
    if (!surface->pending_properties.dirt) {
        free(surface->properties.dirt);
        free(surface->dimensions);
        free(surface);
        return -1;
    }
    
    surface->id     = vioarr_objects_create_object(surface, object_type_surface);
    surface->screen = screen;

    vioarr_screen_register_surface(screen, surface);
    *surface_out = surface;
    return 0;
}

void vioarr_surface_destroy(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }
    
    vioarr_screen_unregister_surface(surface->screen, surface);
    free(surface->pending_properties.dirt);
    free(surface->properties.dirt);
    free(surface->dimensions);
    free(surface);
}

int vioarr_surface_add_child(vioarr_surface_t* parent, vioarr_surface_t* child, int x, int y)
{
    
    
    return 0;
}

void vioarr_surface_set_buffer(vioarr_surface_t* surface, vioarr_buffer_t* content)
{
    if (!surface) {
        return;
    }
    
    surface->pending_properties.content = content;
}

void vioarr_surface_invalidate(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    vioarr_region_add(surface->pending_properties.dirt, x, y, width, height);
}

void vioarr_surface_commit(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }
    
    surface->swap_properties = 1;
}

uint32_t vioarr_surface_id(vioarr_surface_t* surface)
{
    if (!surface) {
        return 0;
    }
    return surface->id;
}

void vioarr_surface_render(NVGcontext* context, vioarr_surface_t* surface)
{
    NVGpaint stream_paint;
    float x = (float)vioarr_region_x(surface->dimensions);
    float y = (float)vioarr_region_y(surface->dimensions);
    float width = (float)vioarr_region_width(surface->dimensions);
    float height = (float)vioarr_region_height(surface->dimensions);
    
    if (!surface) {
        return;
    }
    
    vioarr_surface_update(context, surface);
    if (!surface->visible) {
        return;
    }
    
    nvgSave(context);
    nvgTranslate(context, x, y);
    
    if (surface->properties.content) {
        if (surface->properties.drop_shadow) {
            render_drop_shadow(context, surface);
        }
        
        stream_paint = nvgImagePattern(context, x, y, width, height, 0.0f, surface->properties.resource_id, 1.0f);
        nvgBeginPath(context);
        nvgRect(context, x, y, width, height);
        nvgFillPaint(context, stream_paint);
        nvgFill(context);
    }

    if (surface->properties.children) {
        vioarr_surface_t* child = surface->properties.children;
        while (child) {
            vioarr_surface_render(context, child);
            child = child->properties.link;
        }
    }

    nvgRestore(context);
}

static void vioarr_surface_swap_properties(NVGcontext* context, vioarr_surface_t* surface)
{
    // Handle any cleanup if the content is changing
    if (surface->properties.content != surface->pending_properties.content) {
        if (surface->properties.resource_id) {
            nvgDeleteImage(context, surface->properties.resource_id);
            vioarr_buffer_destroy(surface->properties.content);
        }
        
        vioarr_buffer_acquire(surface->pending_properties.content);
        surface->pending_properties.resource_id = nvgCreateImageRGBA(context, 
            vioarr_buffer_width(surface->pending_properties.content), vioarr_buffer_height(surface->pending_properties.content),
            NVG_IMAGE_FLIPY, (const uint8_t*)vioarr_buffer_data(surface->pending_properties.content));
    }
    
    // Perform the properties copy step. Afterwards we need to clean out some of the pending_properties, while
    // we leave the persistant state members
    memcpy(&surface->properties, &surface->pending_properties, sizeof(vioarr_surface_properties_t));
    
    vioarr_region_zero(surface->pending_properties.dirt);
    surface->swap_properties = 0;
}

static void vioarr_surface_update(NVGcontext* context, vioarr_surface_t* surface)
{
    if (surface->swap_properties) {
        vioarr_surface_swap_properties(context, surface);
    }
    
    // Only update the image data if the region is not empty
    if (vioarr_region_is_zero(surface->properties.dirt)) {
        nvgUpdateImage(context, surface->properties.resource_id,
            (const uint8_t*)vioarr_buffer_data(surface->properties.content));
    }
    
    // Determine other attributes about this surface. Is it visible?
    surface->visible = surface->properties.content != NULL;
}

static void render_drop_shadow(NVGcontext* context, vioarr_surface_t* surface)
{
    float    width        = (float)vioarr_region_width(surface->dimensions);
    float    height       = (float)vioarr_region_height(surface->dimensions);
	NVGpaint shadow_paint = nvgBoxGradient(context, 0, 0 + 2.0f, width, height, 
	    surface->properties.corner_radius * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(context);
	nvgRect(context, -10, -10, width + 20, height + 30);
	//nvgRoundedRect(context, 0, 0, surface->width, surface->height, surface->properties.corner_radius);
	nvgPathWinding(context, NVG_HOLE);
	nvgFillPaint(context, shadow_paint);
	nvgFill(context);
}
