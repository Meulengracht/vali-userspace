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

#include "protocols/wm_core_protocol_server.h"
#include "protocols/wm_memory_protocol_server.h"
#include "protocols/wm_memory_pool_protocol_server.h"
#include "protocols/wm_buffer_protocol_server.h"
#include "engine/vioarr_memory.h"
#include "engine/vioarr_buffer.h"
#include "engine/vioarr_objects.h"
#include <errno.h>

void wm_memory_create_pool_callback(int client, struct wm_memory_create_pool_args* input, struct wm_memory_create_pool_ret* output)
{
    vioarr_memory_pool_t* pool;
    int                   status;
    
    // get memory subsystem
    // TODO
    
    status = vioarr_memory_create_pool(input->size, &pool);
    if (status) {
        wm_core_event_error_single(client, 0 /* input->object_id */, status, "wm_memory: failed to create memory pool");
        return;
    }
    
    output->object_id = vioarr_memory_pool_id(pool);
    output->handle    = vioarr_memory_pool_handle(pool);
}

void wm_memory_pool_create_buffer_callback(int client, struct wm_memory_pool_create_buffer_args* input, struct wm_memory_pool_create_buffer_ret* output)
{
    vioarr_memory_pool_t* pool = vioarr_objects_get_object(input->object_id);
    vioarr_buffer_t*      buffer;
    int                   status;
    if (!pool) {
        wm_core_event_error_single(client, input->object_id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    status = vioarr_buffer_create(pool, input->offset, input->width, input->height, input->stride, input->format, &buffer);
    if (status) {
        wm_core_event_error_single(client, input->object_id, status, "wm_memory: failed to create memory buffer");
        return;
    }
    
    output->object_id = vioarr_buffer_id(buffer);
}

void wm_buffer_destroy_callback(int client, struct wm_buffer_destroy_args* input)
{
    vioarr_buffer_t* buffer = vioarr_objects_get_object(input->object_id);
    int              status;
    if (!buffer) {
        wm_core_event_error_single(client, input->object_id, ENOENT, "wm_memory: object does not exist");
        return;
    }
    
    vioarr_buffer_destroy(buffer);
}
