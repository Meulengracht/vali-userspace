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

#include "protocols/wm_memory_protocol.h"
#include "protocols/wm_memory_pool_protocol.h"
#include "protocols/wm_buffer_protocol.h"

void wm_memory_create_pool_callback(int client, struct wm_memory_create_pool_args* input, struct wm_memory_create_pool_ret* output)
{
    
}

void wm_memory_pool_create_buffer_callback(int client, struct wm_memory_pool_create_buffer_args* input, struct wm_memory_pool_create_buffer_ret* output)
{
    
}

void wm_buffer_destroy_callback(int client, struct wm_buffer_destroy_args* input)
{
    
}
