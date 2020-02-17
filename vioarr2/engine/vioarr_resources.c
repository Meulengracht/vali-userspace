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
#include "vioarr_resources.h"
#include "stddef.h"

static int vioarr_resources_load_fonts(NVGcontext*);
static int vioarr_resources_load_images(NVGcontext*);

static vioarr_resource_t* resources[VIOARR_RESOURCE_COUNT] = {
    
};

int vioarr_resources_initialize(NVGcontext* context)
{
    int status;
    
    status = vioarr_resources_load_fonts(context);
    if (status) {
        return status;
    }
    
    status = vioarr_resources_load_images(context);
    if (status) {
        return status;
    }
    
    return 0;
}

vioarr_resource_t* vioarr_resources_get(int id)
{
    if (id < 0 || id >= VIOARR_RESOURCE_COUNT) {
        return NULL;
    }
    return resources[id];
}

static int vioarr_resources_load_fonts(NVGcontext* context)
{
    nvgCreateFont(context, "sans-normal", "$sys/fonts/DejaVuSans.ttf");
    nvgCreateFont(context, "sans-light", "$sys/fonts/DejaVuSans-ExtraLight.ttf");
    return 0;
}

static int vioarr_resources_load_images(NVGcontext* context)
{
    return 0;
}
