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
#include "../vioarr_utils.h"
#include "../vioarr_objects.h"
#include "../../protocols/wm_screen_protocol_server.h"
#include <stdlib.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#define CPUID_FEAT_ECX_SSSE3    1 << 9
#define CPUID_FEAT_ECX_SSE4_1   1 << 19
#define CPUID_FEAT_ECX_SSE4_2   1 << 20
#define CPUID_FEAT_ECX_AVX      1 << 28

#define CPUID_FEAT_EDX_SSE		1 << 25
#define CPUID_FEAT_EDX_SSE2     1 << 26

extern "C" void present_basic(void *Framebuffer, void *Backbuffer, int Rows, int RowLoops, int RowRemaining, int BytesPerScanline);
extern "C" void present_sse(void *Framebuffer, void *Backbuffer, int Rows, int RowLoops, int RowRemaining, int BytesPerScanline);
extern "C" void present_sse2(void *Framebuffer, void *Backbuffer, int Rows, int RowLoops, int RowRemaining, int BytesPerScanline);

typedef struct vioarr_screen {
    uint32_t           id;
    OSMesaContext      context;
    void*              backbuffer;
    size_t             backbuffer_size;
    void*              framebuffer;
    void*              framebuffer_end;
    vioarr_region_t*   dimensions;
    int                depth_bits;
    int                stride;
    vioarr_renderer_t* renderer;
    
    int                row_loops;
    int                bytes_remaining;
    void              (*present)(void*, void*, int, int, int, int);
} vioarr_screen_t;

vioarr_screen_t* vioarr_screen_create(NVGcontext* context, VideoDescriptor_t* video)
{
    vioarr_screen_t* screen;
    int registers[4] = { 0 };
    int attributes[100], n = 0;
    
    int status;
    int bytes_to_copy;
    int bytes_step;

    screen = malloc(sizeof(vioarr_screen_t));
    if (!screen) {
        return NULL;
    }

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
   
    vioarr_utils_trace("[vioarr] [screen] [create] creating os_mesa context, version 3.3");
    screen->context = OSMesaCreateContextAttribs(&attributes[0], NULL);
    if (!screen->context) {
        free(screen);
        return NULL;
    }
    
    vioarr_utils_trace("[vioarr] [screen] [create] allocating screen resources");
    screen->dimensions = vioarr_region_create();
    if (!screen->dimensions) {
        OSMesaDestroyContext(screen->context);
        free(screen);
        return NULL;
    }
    
    screen->depth_bits = video->Depth;
    screen->stride     = video->BytesPerScanline;
    vioarr_region_add(screen->dimensions, 0, 0, video->Width, video->Height);
    
    screen->backbuffer_size = video->Width * video->Height * 4 * sizeof(GLubyte);
    screen->backbuffer      = aligned_alloc(32, screen->backbuffer_size);
    if (!screen->backbuffer) {
        OSMesaDestroyContext(screen->context);
        free(screen->dimensions);
        free(screen);
        return NULL;
    }
    
    screen->framebuffer     = CreateDisplayFramebuffer();
    screen->framebuffer_end = ((char*)_Framebuffer + (video->BytesPerScanline * (video->Height - 1)));
    
    // Set the newly created context as current for now. We must have one pretty quickly
    status = OSMesaMakeCurrent(screen->context, screen->backbuffer, GL_UNSIGNED_BYTE,
        video->Width, video->Height);
    if (status == GL_FALSE) {
        vioarr_utils_error("[vioarr] [initialize] failed to set the os_mesa context");
        OSMesaDestroyContext(screen->context);
        free(screen->dimensions);
        free(screen);
        return NULL;
    }
    
    vioarr_utils_trace("[vioarr] [screen] [create] loading gl extensions");
    status = gladLoadGLLoader((GLADloadproc)OSMesaGetProcAddress, 3, 3);
    if (!status) {
        OSMesaDestroyContext(screen->context);
        free(screen->dimensions);
        free(screen);
        vioarr_utils_error("[vioarr] [initialize] failed to load gl extensions, code %i", status);
        return NULL;
    }
    
    // Select a present-method (basic/sse/sse2)
#if defined(_MSC_VER) && !defined(__clang__)
    __cpuid(registers, 1);
#else
    __cpuid(1, registers[0], registers[1], registers[2], registers[3]);
#endif

    if (registers[3] & CPUID_FEAT_EDX_SSE2) {
        bytes_step       = 128;
        screen->present  = present_sse2;
    }
    else if (registers[3] & CPUID_FEAT_EDX_SSE) {
        bytes_step       = 128;
        screen->present  = present_sse;
    }
    else {
        bytes_step       = 1;
        screen->present  = present_basic;
    }
    
    bytes_to_copy           = video->Width * 4 * sizeof(GLubyte);
    screen->row_loops       = bytes_to_copy / bytes_step;
    screen->bytes_remaining = bytes_to_copy % bytes_step;
    
    vioarr_utils_trace("[vioarr] [screen] [create] initializing renderer");
    screen->renderer = vioarr_renderer_create(context, screen);
    if (!screen->renderer) {
        OSMesaDestroyContext(screen->context);
        free(screen->dimensions);
        free(screen->backbuffer);
        free(screen)
        return NULL;
    }
    
    screen->id = vioarr_objects_create_object(screen, object_type_screen);
    return screen;
}

void vioarr_screen_set_scale(vioarr_screen_t* screen, int scale)
{
    if (!screen) {
        return;
    }
    vioarr_renderer_set_scale(screen->renderer, scale);
}

void vioarr_screen_set_transform(vioarr_screen_t* screen, enum wm_screen_transform transform)
{
    if (!screen) {
        return;
    }
    // TODO
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
    return vioarr_renderer_scale(screen->renderer);
}

enum wm_screen_transform vioarr_screen_transform(vioarr_screen_t* screen)
{
    int rotation;
    
    if (!screen) {
        return no_transform;
    }
    
    rotation = vioarr_renderer_rotation(screen->renderer);
    return no_transform; // TODO
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
    vioarr_renderer_render(screen->renderer);
    screen->present(screen->framebuffer, screen->backbuffer, vioarr_region_height(screen->dimensions), 
        screen->row_loops, screen->bytes_remaining, screen->stride);
}
