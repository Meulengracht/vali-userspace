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
#include "vioarr_screen.h"
#include "vioarr_region.h"
#include "vioarr_renderer.h"
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
    int transparent;
    
    vioarr_region_t*       input_region;
    vioarr_region_t*       drop_shadow;
    struct vioarr_surface* children;
} vioarr_surface_properties_t;

typedef struct vioarr_surface_backbuffer {
    int              resource_id;
    vioarr_buffer_t* content;
} vioarr_surface_backbuffer_t;

typedef struct vioarr_surface {
    int              client;
    uint32_t         id;
    vioarr_screen_t* screen;
    int              visible;
    mtx_t            sync_object;
    atomic_int       swap_properties;

    vioarr_region_t*       dimensions;
    struct vioarr_surface* parent;
    struct vioarr_surface* link;
    vioarr_region_t*       dirt;
    
    vioarr_surface_properties_t properties[2];

    atomic_int                  swap_backbuffers;
    atomic_int                  backbuffer_index;
    vioarr_surface_backbuffer_t backbuffers[2];
} vioarr_surface_t;

#define ACTIVE_PROPERTIES(surface)  surface->properties[0]
#define PENDING_PROPERTIES(surface) surface->properties[1]

#define ACTIVE_BACKBUFFER(surface)  surface->backbuffers[atomic_load(&surface->backbuffer_index)]
#define PENDING_BACKBUFFER(surface) surface->backbuffers[atomic_load(&surface->backbuffer_index) ^ 1]

static int  __initialize_surface_properties(vioarr_surface_properties_t* properties);
static void __cleanup_surface_properties(vioarr_surface_properties_t* properties);
static void __cleanup_surface_backbuffer(vioarr_screen_t* screen, vioarr_surface_backbuffer_t* backbuffer);
static void __swap_properties(vioarr_surface_t* surface);
static void __update_surface(vcontext_t* context, vioarr_surface_t* surface);
static void __refresh_content(vcontext_t* context, vioarr_surface_t* surface);
static void __render_drop_shadow(vcontext_t* context, vioarr_surface_t* surface);
static void __render_content(vcontext_t* context, vioarr_surface_t* surface);

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
    mtx_init(&surface->sync_object, mtx_plain);
    surface->client  = client;
    surface->id      = id;
    surface->screen  = screen;

    surface->dimensions = vioarr_region_create();
    if (!surface->dimensions) {
        free(surface);
        return -1;
    }
    vioarr_region_add(surface->dimensions, 0, 0, width, height);
    vioarr_region_set_position(surface->dimensions, x, y);

    surface->dirt = vioarr_region_create();
    if (!surface->dirt) {
        free(surface->dimensions);
        free(surface);
        return -1;
    }
    
    if (__initialize_surface_properties(&surface->properties[0]) ||
        __initialize_surface_properties(&surface->properties[1])) {
        vioarr_surface_destroy(surface);
        return -1;
    }
    
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
        itr = ACTIVE_PROPERTIES(surface->parent).children;
        if (itr == surface) {
            ACTIVE_PROPERTIES(surface->parent).children = surface->link;
        }
        else {
            while (itr->link != surface) {
                itr = itr->link;
            }

            itr->link = surface->link;
        }
    }

    // if we have children, go through them and promote them to regular surfaces
    itr = ACTIVE_PROPERTIES(surface).children;
    while (itr) {
        vioarr_surface_t* next = itr->link;

        // set parent and link NULL
        itr->parent = NULL;
        itr->link   = NULL;

        itr = next;
    }

    __cleanup_surface_properties(&surface->properties[0]);
    __cleanup_surface_properties(&surface->properties[1]);
    __cleanup_surface_backbuffer(surface->screen, &surface->backbuffers[0]);
    __cleanup_surface_backbuffer(surface->screen, &surface->backbuffers[1]);

    free(surface->dirt);
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
    if (PENDING_PROPERTIES(parent).children == NULL) {
        PENDING_PROPERTIES(parent).children = child;
    }
    else {
        vioarr_surface_t* itr = PENDING_PROPERTIES(parent).children;
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
    int resourceId = -1;

    if (!surface) {
        return;
    }

    if (content) {
        vioarr_buffer_acquire(content);
        resourceId = vioarr_renderer_create_image(vioarr_screen_renderer(surface->screen), content);
        TRACE("[vioarr_surface_set_buffer] initialized new content %i 0x%llx", resourceId, content);
    }
    
    mtx_lock(&surface->sync_object);
    if (PENDING_BACKBUFFER(surface).content) {
        TRACE("[vioarr_surface_set_buffer] cleaning up previous %i 0x%llx",
            PENDING_BACKBUFFER(surface).resource_id,
            PENDING_BACKBUFFER(surface).content);
        vioarr_renderer_destroy_image(vioarr_screen_renderer(surface->screen), PENDING_BACKBUFFER(surface).resource_id);
        vioarr_buffer_destroy(PENDING_BACKBUFFER(surface).content);
    }

    PENDING_BACKBUFFER(surface).content = content;
    PENDING_BACKBUFFER(surface).resource_id = resourceId;

    atomic_store(&surface->swap_backbuffers, 1);
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
    vioarr_region_zero(PENDING_PROPERTIES(surface).drop_shadow);
    vioarr_region_add(PENDING_PROPERTIES(surface).drop_shadow, x, y, width, height);
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_set_input_region(vioarr_surface_t* surface, int x, int y, int width, int height)
{
    if (!surface) {
        return;
    }
    
    mtx_lock(&surface->sync_object);
    vioarr_region_zero(PENDING_PROPERTIES(surface).input_region);
    vioarr_region_add(PENDING_PROPERTIES(surface).input_region, x, y, width, height);
    mtx_unlock(&surface->sync_object);
}

void vioarr_surface_set_transparency(vioarr_surface_t* surface, int enable)
{
    if (!surface) {
        return;
    }

    mtx_lock(&surface->sync_object);
    PENDING_PROPERTIES(surface).transparent = enable;
    mtx_unlock(&surface->sync_object);
}

int vioarr_surface_supports_input(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return 0;
    }

    if (vioarr_region_contains(ACTIVE_PROPERTIES(surface).input_region, x, y)) {
        return 1;
    }
    else {
        // check children if they support input
        vioarr_surface_t* itr = ACTIVE_PROPERTIES(surface).children;
        while (itr) {
            if (vioarr_surface_supports_input(itr, x, y)) {
                return 1;
            }
            itr = itr->link;
        }
    }
    return 0;
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
    vioarr_region_add(surface->dirt, x, y, width, height);
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

void vioarr_surface_move_absolute(vioarr_surface_t* surface, int x, int y)
{
    if (!surface) {
        return;
    }

    mtx_lock(&surface->sync_object);
    vioarr_region_set_position(surface->dimensions, x, y);
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

void vioarr_surface_render(vcontext_t* context, vioarr_surface_t* surface)
{
    if (!surface) {
        return;
    }

    //TRACE("[vioarr_surface_render] %u [%i, %i]", 
    //    surface->id, vioarr_region_x(surface->dimensions),
    //    vioarr_region_y(surface->dimensions));
    __update_surface(context, surface);
    if (!surface->visible) {
        return;
    }
    
#ifdef VIOARR_BACKEND_NANOVG
    nvgSave(context);
    nvgTranslate(context, 
        (float)vioarr_region_x(surface->dimensions),
        (float)vioarr_region_y(surface->dimensions)
    );

    // handle transparency of the surface
    if (ACTIVE_PROPERTIES(surface).transparent) {
        nvgGlobalCompositeBlendFunc(context, NVG_SRC_ALPHA, NVG_ONE_MINUS_SRC_ALPHA);
    }
    else {
        nvgGlobalCompositeOperation(context, NVG_COPY);
    }
#endif

    if (ACTIVE_BACKBUFFER(surface).content) {
        //TRACE("[vioarr_surface_render] rendering content");
        if (!vioarr_region_is_zero(ACTIVE_PROPERTIES(surface).drop_shadow)) {
            __render_drop_shadow(context, surface);
        }
        __render_content(context, surface);
    }

    if (ACTIVE_PROPERTIES(surface).children) {
        vioarr_surface_t* child = ACTIVE_PROPERTIES(surface).children;
        while (child) {
            vioarr_surface_render(context, child);
            child = child->link;
        }
    }

#ifdef VIOARR_BACKEND_NANOVG
    nvgRestore(context);
#endif
}

static void __update_surface(NVGcontext* context, vioarr_surface_t* surface)
{
    //TRACE("[__update_surface]");

    if (atomic_load(&surface->swap_properties)) {
        int swap_backbuffers = atomic_load(&surface->swap_backbuffers);
        if (swap_backbuffers) {
            int backbuffer_index = atomic_load(&surface->backbuffer_index);
            atomic_store(&surface->backbuffer_index, backbuffer_index ^ 1);
            atomic_store(&surface->swap_backbuffers, 0);
        }

        __refresh_content(context, surface);
        __swap_properties(surface);
        atomic_store(&surface->swap_properties, 0);
    }
    
    // Determine other attributes about this surface. Is it visible?
    surface->visible = ACTIVE_BACKBUFFER(surface).content != NULL;

    //TRACE("[__update_surface] is visible: %i", surface->visible);
}

static void __refresh_content(NVGcontext* context, vioarr_surface_t* surface)
{
    if (!vioarr_region_is_zero(surface->dirt)) {
        vioarr_buffer_t* buffer     = ACTIVE_BACKBUFFER(surface).content;
        int              resourceId = ACTIVE_BACKBUFFER(surface).resource_id;
        if (buffer) {
#ifdef VIOARR_BACKEND_NANOVG
            nvgUpdateImage(context, resourceId, (const uint8_t*)vioarr_buffer_data(buffer));
#endif
            wm_buffer_event_release_single(surface->client, vioarr_buffer_id(buffer));
        }
        
        vioarr_region_zero(surface->dirt);
    }
}

static void __swap_properties(vioarr_surface_t* surface)
{
    //TRACE("[__swap_properties]");

    // handle basic properties
    ACTIVE_PROPERTIES(surface).border_width  = PENDING_PROPERTIES(surface).border_width;
    ACTIVE_PROPERTIES(surface).border_color  = PENDING_PROPERTIES(surface).border_color;
    ACTIVE_PROPERTIES(surface).corner_radius = PENDING_PROPERTIES(surface).corner_radius;
    ACTIVE_PROPERTIES(surface).transparent   = PENDING_PROPERTIES(surface).transparent;
    vioarr_region_copy(ACTIVE_PROPERTIES(surface).drop_shadow,  PENDING_PROPERTIES(surface).drop_shadow);
    vioarr_region_copy(ACTIVE_PROPERTIES(surface).input_region, PENDING_PROPERTIES(surface).input_region);
    
    if (PENDING_PROPERTIES(surface).children) {
        // append the new children
        if (ACTIVE_PROPERTIES(surface).children) {
            vioarr_surface_t* itr = ACTIVE_PROPERTIES(surface).children;
            while (itr->link) {
                itr = itr->link;
            }
            itr->link = PENDING_PROPERTIES(surface).children;
        }
        else {
            ACTIVE_PROPERTIES(surface).children = PENDING_PROPERTIES(surface).children;
        }
        PENDING_PROPERTIES(surface).children = NULL;
    }
}

static void __render_content(vcontext_t* context, vioarr_surface_t* surface)
{
    float    width        = (float)vioarr_region_width(surface->dimensions);
    float    height       = (float)vioarr_region_height(surface->dimensions);
#ifdef VIOARR_BACKEND_NANOVG
    NVGpaint stream_paint = nvgImagePattern(context, 0.0f, 0.0f, width, height, 0.0f, 
        ACTIVE_BACKBUFFER(surface).resource_id, 1.0f);
    nvgBeginPath(context);
    nvgRect(context, 0.0f, 0.0f, width, height);
    nvgFillPaint(context, stream_paint);
    nvgFill(context);
#endif
}

static void __render_drop_shadow(vcontext_t* context, vioarr_surface_t* surface)
{
    float    width        = (float)vioarr_region_width(surface->dimensions);
    float    height       = (float)vioarr_region_height(surface->dimensions);
#ifdef VIOARR_BACKEND_NANOVG
    if (!ACTIVE_PROPERTIES(surface).transparent) {
        nvgSave(context);
        nvgGlobalCompositeBlendFunc(context, NVG_SRC_ALPHA, NVG_ONE_MINUS_SRC_ALPHA);
    }

	NVGpaint shadow_paint = nvgBoxGradient(context, 0, 0 + 2.0f, width, height, 
	    ACTIVE_PROPERTIES(surface).corner_radius * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(context);
	//nvgRect(context, -10, -10, width + 20, height + 30);
	//nvgRoundedRect(context, 0, 0, surface->width, surface->height, surface->properties.corner_radius);
	nvgRect(context, 
        (float)vioarr_region_x(ACTIVE_PROPERTIES(surface).drop_shadow),
        (float)vioarr_region_y(ACTIVE_PROPERTIES(surface).drop_shadow),
        width + (float)vioarr_region_width(ACTIVE_PROPERTIES(surface).drop_shadow),
        height + (float)vioarr_region_height(ACTIVE_PROPERTIES(surface).drop_shadow));
	nvgPathWinding(context, NVG_HOLE);
	nvgFillPaint(context, shadow_paint);
	nvgFill(context);

    if (!ACTIVE_PROPERTIES(surface).transparent) {
        nvgRestore(context);
    }
#endif
}

static int __initialize_surface_properties(vioarr_surface_properties_t* properties)
{
    properties->drop_shadow  = vioarr_region_create();
    properties->input_region = vioarr_region_create();
    if (!properties->drop_shadow || !properties->input_region)
        return -1;
    return 0;
}

static void __cleanup_surface_properties(vioarr_surface_properties_t* properties)
{
    if (properties->input_region) {
        free(properties->input_region);
    }

    if (properties->drop_shadow) {
        free(properties->drop_shadow);
    }
}

static void __cleanup_surface_backbuffer(vioarr_screen_t* screen, vioarr_surface_backbuffer_t* backbuffer)
{
    if (backbuffer->content) {
        vioarr_renderer_destroy_image(vioarr_screen_renderer(screen), backbuffer->resource_id);
        vioarr_buffer_destroy(backbuffer->content);
    }
}
