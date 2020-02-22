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
 
#ifndef __VIOARR_SCREEN_H__
#define __VIOARR_SCREEN_H__

#include "backend/nanovg.h"
#include <ddk/contracts/video.h>
#include "../protocols/wm_screen_protocol.h"
#include "vioarr_region.h"
#include "vioarr_surface.h"

typedef struct vioarr_screen vioarr_screen_t;

vioarr_screen_t*         vioarr_screen_create(NVGcontext*, VideoDescriptor_t*);
void                     vioarr_screen_set_scale(vioarr_screen_t*, int);
void                     vioarr_screen_set_transform(vioarr_screen_t*, enum wm_screen_transform);
vioarr_region_t*         vioarr_screen_region(vioarr_screen_t*);
int                      vioarr_screen_scale(vioarr_screen_t*);
enum wm_screen_transform vioarr_screen_transform(vioarr_screen_t*);
int                      vioarr_screen_publish_modes(vioarr_screen_t*, int);
void                     vioarr_screen_register_surface(vioarr_screen_t*, vioarr_surface_t*);
void                     vioarr_screen_unregister_surface(vioarr_screen_t*, vioarr_surface_t*);
void                     vioarr_screen_frame(vioarr_screen_t*);

#endif //!__VIOARR_SCREEN_H__
