/* MollenOS
 *
 * Copyright 2019, Philip Meulengracht
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
 * Wm Type Definitions & Structures
 * - This header describes the base wm-structure, prototypes
 *   and functionality, refer to the individual things for descriptions
 */

#ifndef __LIBWM_TYPES_H__
#define __LIBWM_TYPES_H__

typedef enum {
    wm_request_create_window
} wm_event_type_t;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} wm_rect_t;

typedef enum {
    surface_8r8g8b8a,
} wm_surface_format_t;

typedef struct {
    wm_rect_t           dimensions;
    wm_surface_format_t format;
} wm_surface_descriptor_t;

typedef struct {
    unsigned flags;
    int      input_pipe_fd;
    int      message_pipe_fd;
} wm_window_create_request_t;

typedef struct {
    char title[64];
} wm_window_set_title_request_t;

typedef struct {
    wm_surface_descriptor_t surface_descriptor;
} wm_surface_request_t;

typedef struct {
    wm_rect_t dirty_area;
} wm_redraw_surface_request_t;

typedef struct {
    int handle;
} wm_set_active_surface_request_t;

typedef struct {
    int       handle;
    wm_rect_t new_dimensions;
} wm_resize_surface_request_t;

#endif // !__LIBWM_TYPES_H__
