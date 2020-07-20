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
#include "vioarr_utils.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <threads.h>

#include "../protocols/wm_surface_protocol_server.h"
#include "../protocols/wm_buffer_protocol_server.h"

typedef struct vioarr_surface_properties {
    int corner_radius;
    int border_width;
    int border_color;
    
    vioarr_region_t*       input_region;
    vioarr_region_t*       drop_shadow;
    vioarr_region_t*       dirt;
    vioarr_buffer_t*       content;
    struct vioarr_surface* children;
} vioarr_surface_properties_t;

typedef struct vioarr_surface {
    int              client;
    uint32_t         id;
    vioarr_screen_t* screen;
    int              visible;
    atomic_int       swap_properties;
    mtx_t            sync_object;

    // non-versioned data
    vioarr_region_t*       dimensions;
    struct vioarr_surface* parent;
    struct vioarr_surface* link;
    int                    resource_id;
    
    // versioned data
    vioarr_surface_properties_t properties;
    vioarr_surface_properties_t pending_properties;
} vioarr_surface_t;

static void vioarr_surface_update(NVGcontext* context, vioarr_surface_t* surface);
static void render_drop_shadow(NVGcontext* context, vioarr_surface_t* surface);

int vioarr_surface_create(int client, uint32_t id, vioarr_screen_t* screen, int x, int y,
    int width, int height, vioarr_surface_t** surfaceOut)
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
    vioarr_region_add(surface->dimensions, 0, 0, width, height);
    vioarr_region_set_position(surface->dimensions, x, y);
    
    surface->properties.dirt                 = vioarr_region_create();
    surface->properties.drop_shadow          = vioarr_region_create();
    surface->properties.input_region         = vioarr_region_create();
    surface->pending_properties.dirt         = vioarr_region_create();
    surface->pending_properties.drop_shadow  = vioarr_region_create();
    surface->pending_properties.input_region = vioarr_region_create();
    if (!surface->properties.dirt ||
        !surface->properties.drop_shadow ||
        !surface->properties.input_region ||
        !surface->pending_properties.dirt || 
        !surface->pending_properties.drop_shadow || 
        !surface->pending_properties.input_region) {
        vioarr_surface_destroy(surface);
        return -1;
    }
    
    mtx_init(&surface->sync_object, mtx_plain);
    surface->client          = client;
    surface->id              = id;
    surface->screen          = screen;
    surface->swap_properties = ATOMIC_VAR_INIT(0);

    *surfaceOut = surface;
    return 0;
}

void vioarr_surface_destroy(vioarr_surface_t* surface)
{
    vioarr_surface_t* itr;

    if (!surface) {
        return;
    }

    // if this surface is a child, remove it from the parent
    if (surface->parent) {
        itr = surface->parent->properties.children;
        if (itr == surface) {
            surface->parent->properties.children = surface->link;
        }
        else {
            while (itr->link != surface) {
                itr = itr->link;
            }

            itr->link = surface->link;
        }
    }

    // if we have children, go through them and promote them to regular surfaces
    itr = surface->properties.children;
    while (itr) {
        vioarr_surface_t* next = itr->link;

        // set parent and link NULL
        itr->parent = NULL;
        itr->link   = NULL;

        itr = next;
    }

    if (surface->pending_properties.input_region) {
        free(surface->pending_properties.input_region);
    }
    
    if (surface->pending_properties.drop_shadow) {
        free(surface->pending_properties.drop_shadow);
    }
    
    if (surface->pending_properties.dirt) {
        free(surface->pending_properties.dirt);
    }
    
    if (surface->properties.content) {
        vioarr_buffer_destroy(surface->properties.content);
    }

    if (surface->properties.input_region) {
        free(surface->properties.input_region);
    }

    if (surface->properties.drop_shadow) {
        free(surface->properties.drop_shadow);
    }
    
    if (surface->properties.dirt) {
        free(surface->properties.dirt);
    }

    free(surface->dimensions);
    free(surface);
}

int vioarr_surface_add_child(vioarr_surface_t* parent, vioarr_surface_t* child, int x, int y)
{
    if (!parent || !child) {
        return -1;
    }

    if (child->parent) {
        return -1;
    }
    child->parent = parent;

    TRACE("[vioarr_surface_add_child]");
    mtx_lock(&parent->sync_object);
    // the list that we keep updated is actually the one in pending properties
    // which means all list changes are performed there
    if (parent->pending_properties.children == NULL) {
        parent->pending_properties.children = child;
    }
    else {
        vioarr_surface_t* itr = parent->pending_properties.children;
        while (itr->link) {
            itr = itr->link;
        }
        itr->link = child;
    }
    mtx_unlock(&parent->sync_object);

    // update child position
    vioarr_surface_set_position(child, x, y);
    return 0;
}

void vioarr_surface_set_buffer(vioarr_surface_t* surface, vioarr_buffer_t* content)
{
    if (!surface) {
        return;
    }

    mtx_lock(&surface->sync_object);
    surface->pending_properties.content = content;
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_set_position(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return;
    }

    mtx_lock(&surface->sync_object);
    vioarr_region_set_position(surface->dimensions, x, y);
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_set_drop_shadow(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    mtx_lock(&surface->sync_object);
    vioarr_region_zero(surface->pending_properties.drop_shadow);
    vioarr_region_add(surface->pending_properties.drop_shadow, x, y, width, height);
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_set_input_region(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    mtx_lock(&surface->sync_object);
    vioarr_region_zero(surface->pending_properties.input_region);
    vioarr_region_add(surface->pending_properties.input_region, x, y, width, height);
    mtx_unlock(&surface->sync_object);
}

int vioarr_surface_supports_input(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return 0;
    }

    // check children
    if (vioarr_region_contains(surface->properties.input_region, x, y)) {
        return 1;
    }
}

int vioarr_surface_contains(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return 0;
    }

    return vioarr_region_contains(surface->dimensions, x, y);
}

void vioarr_surface_invalidate(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    mtx_lock(&surface->sync_object);
    vioarr_region_add(surface->pending_properties.dirt, x, y, width, height);
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_commit(vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }
    
    atomic_store(&surface->swap_properties, 1);
}

void vioarr_surface_move(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return;
    }

    mtx_lock(&surface->sync_object);
    vioarr_region_set_position(surface->dimensions,
        vioarr_region_x(surface->dimensions) + x,
        vioarr_region_y(surface->dimensions) + y);
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_focus(vioarr_surface_t* surface, int focus)
{
    if (!surface) {
        return;
    }

    wm_surface_event_focus_single(surface->client, surface->id, focus);
}

uint32_t vioarr_surface_id(vioarr_surface_t* surface)
{
    if (!surface) {
        return 0;
    }
    return surface->id;
}

int vioarr_surface_client(vioarr_surface_t* surface)
{
    if (!surface) {
        return -1;
    }
    return surface->client;
}

vioarr_screen_t* vioarr_surface_screen(vioarr_surface_t* surface)
{
    if (!surface) {
        return NULL;
    }
    return surface->screen;
}

vioarr_region_t* vioarr_surface_region(vioarr_surface_t* surface)
{
    if (!surface) {
        return NULL;
    }
    return surface->dimensions;
}

void vioarr_surface_render(NVGcontext* context, vioarr_surface_t* surface)
{
    NVGpaint stream_paint;
    float width = (float)vioarr_region_width(surface->dimensions);
    float height = (float)vioarr_region_height(surface->dimensions);
    
    if (!surface) {
        return;
    }

    TRACE("[vioarr_surface_render] %u [%i, %i]", 
        surface->id, vioarr_region_x(surface->dimensions),
        vioarr_region_y(surface->dimensions));
    vioarr_surface_update(context, surface);
    if (!surface->visible) {
        return;
    }
    
    nvgSave(context);
    nvgTranslate(context, 
        (float)vioarr_region_x(surface->dimensions),
        (float)vioarr_region_y(surface->dimensions)
    );

    if (surface->properties.content) {
        TRACE("[vioarr_surface_render] rendering content");
        if (!vioarr_region_is_zero(surface->properties.drop_shadow)) {
            render_drop_shadow(context, surface);
        }
        
        stream_paint = nvgImagePattern(context, 0.0f, 0.0f, width, height, 0.0f, surface->resource_id, 1.0f);
        nvgBeginPath(context);
        nvgRect(context, 0.0f, 0.0f, width, height);
        nvgFillPaint(context, stream_paint);
        nvgFill(context);
    }

    if (surface->properties.children) {
        TRACE("[vioarr_surface_render] rendering children");
        vioarr_surface_t* child = surface->properties.children;
        while (child) {
            vioarr_surface_render(context, child);
            child = child->link;
        }
    }

    nvgRestore(context);
}

static void vioarr_surface_swap_properties(NVGcontext* context, vioarr_surface_t* surface)
{
    TRACE("[vioarr_surface_swap_properties]");
    mtx_lock(&surface->sync_object);

    // handle basic properties
    surface->properties.border_width  = surface->pending_properties.border_width;
    surface->properties.border_color  = surface->pending_properties.border_color;
    surface->properties.corner_radius = surface->pending_properties.corner_radius;
    vioarr_region_copy(surface->properties.drop_shadow, surface->pending_properties.drop_shadow);
    vioarr_region_copy(surface->properties.input_region, surface->pending_properties.input_region);
    
    if (surface->pending_properties.children) {
        // append the new children
        if (surface->properties.children) {
            vioarr_surface_t* itr = surface->properties.children;
            while (itr->link) {
                itr = itr->link;
            }
            itr->link = surface->pending_properties.children;
        }
        else {
            surface->properties.children = surface->pending_properties.children;
        }
        surface->pending_properties.children = NULL;
    }

    // Handle any cleanup if the content is changing
    if (surface->pending_properties.content) {
        TRACE("[vioarr_surface_swap_properties] initializing new content");
        if (surface->resource_id) {
            TRACE("[vioarr_surface_swap_properties] cleaning up previous");
            nvgDeleteImage(context, surface->resource_id);
            vioarr_buffer_destroy(surface->properties.content);
        }
        
        vioarr_buffer_acquire(surface->pending_properties.content);
        surface->resource_id = nvgCreateImageRGBA(context, 
            vioarr_buffer_width(surface->pending_properties.content),
            vioarr_buffer_height(surface->pending_properties.content),
            /* NVG_IMAGE_FLIPY */ 0, 
            (const uint8_t*)vioarr_buffer_data(surface->pending_properties.content));

        surface->properties.content         = surface->pending_properties.content;
        surface->pending_properties.content = NULL;
    }
    
    vioarr_region_copy(surface->properties.dirt, surface->pending_properties.dirt);
    vioarr_region_zero(surface->pending_properties.dirt);
    mtx_unlock(&surface->sync_object);

    atomic_store(&surface->swap_properties, 0);
}

static void vioarr_surface_update(NVGcontext* context, vioarr_surface_t* surface)
{
    TRACE("[vioarr_surface_update]");
    if (atomic_load(&surface->swap_properties)) {
        vioarr_surface_swap_properties(context, surface);
    }
    
    // Only update the image data if the region is not empty
    if (!vioarr_region_is_zero(surface->properties.dirt)) {
        TRACE("[vioarr_surface_update] refreshing conent");
        nvgUpdateImage(context, surface->resource_id,
            (const uint8_t*)vioarr_buffer_data(surface->properties.content));
        
        // Now we are done with the user-buffer, send the release event
        wm_buffer_event_release_single(surface->client, vioarr_buffer_id(surface->properties.content));

        // clear dirt
        vioarr_region_zero(surface->properties.dirt);
    }
    
    // Determine other attributes about this surface. Is it visible?
    surface->visible = surface->properties.content != NULL;
    TRACE("[vioarr_surface_update] is visible: %i", surface->visible);
}

static void render_drop_shadow(NVGcontext* context, vioarr_surface_t* surface)
{
    float    width        = (float)vioarr_region_width(surface->dimensions);
    float    height       = (float)vioarr_region_height(surface->dimensions);
	NVGpaint shadow_paint = nvgBoxGradient(context, 0, 0 + 2.0f, width, height, 
	    surface->properties.corner_radius * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(context);
	//nvgRect(context, -10, -10, width + 20, height + 30);
	//nvgRoundedRect(context, 0, 0, surface->width, surface->height, surface->properties.corner_radius);
	nvgRect(context, 
        (float)vioarr_region_x(surface->properties.drop_shadow),
        (float)vioarr_region_y(surface->properties.drop_shadow),
        width + (float)vioarr_region_width(surface->properties.drop_shadow),
        height + (float)vioarr_region_height(surface->properties.drop_shadow));
	nvgPathWinding(context, NVG_HOLE);
	nvgFillPaint(context, shadow_paint);
	nvgFill(context);
}
