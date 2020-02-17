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

#include "protocols/wm_screen_protocol.h"
#include "protocols/wm_core_protocol.h"
#include "engine/vioarr_screen.h"
#include "engine/vioarr_utils.h"
#include <libwm_server.h>

void wm_screen_get_properties_callback(int client, struct wm_screen_get_properties_args* input, struct wm_screen_get_properties_ret* output)
{
    vioarr_screen_t* screen = vioarr_utils_get_object(input->object_id);
    if (!screen) {
        struct wm_core_error_event error_event = {
            .object_id         = input->object_id,
            .error_id          = ENOENT,
            .error_description = { 0 }
        };
        
        wm_server_send_event(client, PROTOCOL_WM_CORE_ID, PROTOCOL_WM_CORE_EVENT_ERROR_ID,
            &error_event, sizeof(struct wm_core_error_event));
    }
    
    
}

void wm_screen_get_modes_callback(int client)
{
    
}

void wm_screen_set_scale_callback(int client, struct wm_screen_set_scale_args* input)
{
    
}

void wm_screen_set_transform_callback(int client, struct wm_screen_set_transform_args* input)
{
    
}

void wm_screen_create_surface_callback(int client, struct wm_screen_create_surface_args* input, struct wm_screen_create_surface_ret* output)
{
    
}
