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

#include "../vioarr_memory.h"
#include "../vioarr_objects.h"
#include <os/dmabuf.h>
#include <os/mollenos.h>
#include <stdlib.h>

typedef struct vioarr_memory_pool {
    uint32_t              id;
    _Atomic(int)          references;
    struct dma_attachment attachment;
} vioarr_memory_pool_t;

int vioarr_memory_create_pool(uint32_t id, size_t size, vioarr_memory_pool_t** pool_out)
{
    vioarr_memory_pool_t*  pool;
    struct dma_buffer_info dma_info;
    OsStatus_t             os_status;
    
    if (!size) {
        return -1;
    }
    
    pool = malloc(sizeof(vioarr_memory_pool_t));
    if (!pool) {
        return -1;
    }
    
    dma_info.name     = "wm_buffer";
    dma_info.length   = size;
    dma_info.capacity = size;
    dma_info.flags    = DMA_CLEAN;
    
    os_status = dma_create(&dma_info, &pool->attachment);
    if (os_status != OsSuccess) {
        OsStatusToErrno(os_status);
        free(pool);
        return -1;
    }
    
    pool->id         = id;
    pool->references = ATOMIC_VAR_INIT(1);
    
    vioarr_objects_create_client_object(id, pool, object_type_memory_pool);
    *pool_out = pool;
    return 0;
}

int vioarr_memory_pool_acquire(vioarr_memory_pool_t* pool)
{
    int references;
    
    if (!pool) {
        return -1;
    }
    
    references = atomic_fetch_add(&pool->references, 1);
    if (!references) {
        return -1;
    }
    return 0;
}

int vioarr_memory_destroy_pool(vioarr_memory_pool_t* pool)
{
    int references;
    
    if (!pool) {
        return -1;
    }
    
    references = atomic_fetch_sub(&pool->references, 1);
    if (references == 1) {
        vioarr_objects_remove_object(pool->id);
        dma_attachment_unmap(&pool->attachment);
        dma_detach(&pool->attachment);
        free(pool);
    }
    return 0;
}

uint32_t vioarr_memory_pool_id(vioarr_memory_pool_t* pool)
{
    if (!pool) {
        return 0;
    }
    return pool->id;
}

UUId_t vioarr_memory_pool_handle(vioarr_memory_pool_t* pool)
{
    if (!pool) {
        return UUID_INVALID;
    }
    return pool->attachment.handle;
}

void* vioarr_memory_pool_data(vioarr_memory_pool_t* pool, int index, size_t size)
{
    uint8_t* pointer;
    
    if (!pool) {
        return NULL;
    }
    
    if (index < 0 || ((size_t)index + size) > pool->attachment.length) {
        return NULL;
    }
    
    pointer = (uint8_t*)pool->attachment.buffer;
    return (void*)&pointer[index];
}
