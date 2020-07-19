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

#ifndef __VIOARR_SURFACE_H__
#define __VIOARR_SURFACE_H__

#include "backend/nanovg.h"
#include <stdint.h>

typedef struct vioarr_surface vioarr_surface_t;
typedef struct vioarr_screen  vioarr_screen_t;
typedef struct vioarr_buffer  vioarr_buffer_t;
typedef struct vioarr_region  vioarr_region_t;

int              vioarr_surface_create(int, uint32_t, vioarr_screen_t*, int, int, int, int, vioarr_surface_t**);
void             vioarr_surface_destroy(vioarr_surface_t*);
void             vioarr_surface_set_buffer(vioarr_surface_t*, vioarr_buffer_t*);
void             vioarr_surface_set_drop_shadow(vioarr_surface_t*, int, int, int, int);
void             vioarr_surface_set_input_region(vioarr_surface_t*, int, int, int, int);
void             vioarr_surface_invalidate(vioarr_surface_t*, int, int, int, int);
void             vioarr_surface_move(vioarr_surface_t*, int, int);
void             vioarr_surface_commit(vioarr_surface_t*);
uint32_t         vioarr_surface_id(vioarr_surface_t*);
vioarr_screen_t* vioarr_surface_screen(vioarr_surface_t*);
vioarr_region_t* vioarr_dimensions(vioarr_surface_t*);

int  vioarr_surface_add_child(vioarr_surface_t*, vioarr_surface_t*, int, int);
void vioarr_surface_set_position(vioarr_surface_t*, int, int);

void vioarr_surface_render(NVGcontext*, vioarr_surface_t*);

#endif //!__VIOARR_SURFACE_H__
