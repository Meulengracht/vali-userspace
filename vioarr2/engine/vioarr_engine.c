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

#define ENGINE_SCREEN_REFRESH_HZ 60
#define ENGINE_SCREEN_REFRESH_MS (1000 / ENGINE_SCREEN_REFRESH_HZ)

#include <ddk/video.h>
#include <os/mollenos.h>
#include "vioarr_engine.h"
#include "vioarr_renderer.h"
#include "vioarr_screen.h"
#include "vioarr_utils.h"
#include <time.h>
#include <threads.h>

#include <glad/glad.h>
#include "backend/nanovg.h"
#include "backend/nanovg_gl.h"

static int vioarr_engine_setup_screens(void);
static int vioarr_engine_update(void*);

static vioarr_screen_t* primary_screen;
static thrd_t           screen_thread;
static NVGcontext*      nvg_root_context;

int vioarr_engine_initialize(void)
{
    int status;
    
    vioarr_utils_trace("[vioarr] [initialize] initializing screens");
    status = vioarr_engine_setup_screens();
    if (status) {
        vioarr_utils_error("[vioarr] [initialize] failed to initialize screens, code %i", status);
        return status;
    }
    
    vioarr_utils_trace("[vioarr] [initialize] creating nvg context");
#ifdef __VIOARR_CONFIG_RENDERER_MSAA
	nvg_root_context = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#else
	nvg_root_context = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
    if (!nvg_root_context) {
        vioarr_utils_error("[vioarr] [initialize] failed to create the nvg context");
        return -1;
    }
    
    // Spawn the renderer thread, this will update the screen at a 60 hz frequency
    // and handle all redrawing
    vioarr_utils_trace("[vioarr] [initialize] creating screen renderer thread");
    return thrd_create(&screen_thread, vioarr_engine_update, primary_screen);
}

static int vioarr_engine_setup_screens(void)
{
    VideoDescriptor_t video;
    OsStatus_t        os_status;
    
    vioarr_utils_trace("[vioarr] [initialize] quering screen information");
    // Get screens available from OS.
    os_status = QueryDisplayInformation(&video);
    if (os_status != OsSuccess) {
        vioarr_utils_error("[vioarr] [initialize] failed to query screens, status %u", os_status);
        OsStatusToErrno(os_status);
        return -1;
    }
    
    vioarr_utils_trace("[vioarr] [initialize] creating primary screen object");
    // Create the primary screen object. In the future we will support
    // multiple displays and also listen for screen hotplugs
    primary_screen = vioarr_screen_create(nvg_root_context, &video);
    if (!primary_screen) {
        vioarr_utils_error("[vioarr] [initialize] failed to create primary screen object");
        return -1;
    }
    return 0;
}

static int vioarr_engine_update(void* context)
{
    vioarr_screen_t* screen = context;
    clock_t start, end, diff_ms;
    
    vioarr_utils_trace("[vioarr] [renderer_thread] started");
    while (1) {
        start = clock();
        
        vioarr_screen_frame(screen);
        
        end = clock();
        diff_ms = (end - start) / CLOCKS_PER_SEC;
        start = end;
        
        thrd_sleepex(ENGINE_SCREEN_REFRESH_MS - (diff_ms % ENGINE_SCREEN_REFRESH_MS));
    }
}
