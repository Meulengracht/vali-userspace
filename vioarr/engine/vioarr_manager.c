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

#include <ds/list.h>
#include "vioarr_manager.h"
#include "vioarr_surface.h"
#include "vioarr_utils.h"
#include <stdlib.h>
#include <threads.h>

typedef struct vioarr_rwlock {
    mtx_t sync_object;
    int   readers;
    cnd_t signal;
} vioarr_rwlock_t;
#define RWLOCK_INIT { MUTEX_INIT(mtx_plain), 0, COND_INIT }

void vioarr_rwlock_r_lock(vioarr_rwlock_t* lock)
{
    mtx_lock(&lock->sync_object);
    lock->readers++;
    mtx_unlock(&lock->sync_object);
}

void vioarr_rwlock_r_unlock(vioarr_rwlock_t* lock)
{
    mtx_lock(&lock->sync_object);
    lock->readers--;
    if (!lock->readers) {
        cnd_signal(&lock->signal);
    }
    mtx_unlock(&lock->sync_object);
}

void vioarr_rwlock_w_lock(vioarr_rwlock_t* lock)
{
    mtx_lock(&lock->sync_object);
    if (lock->readers) {
        cnd_wait(&lock->signal, &lock->sync_object);
    }
}

void vioarr_rwlock_w_unlock(vioarr_rwlock_t* lock)
{
    mtx_unlock(&lock->sync_object);
    cnd_signal(&lock->signal);
}

typedef struct vioarr_manager {
    list_t          surfaces;
    list_t          cursors;
    vioarr_rwlock_t lock;
} vioarr_manager_t;

static vioarr_manager_t globalManager = { LIST_INIT, LIST_INIT, RWLOCK_INIT };

void vioarr_manager_register_surface(vioarr_surface_t* surface)
{
    if (!surface) {
        ERROR("[vioarr_manager_register_surface] null parameters");
        return;
    }

    element_t* element = malloc(sizeof(element_t));
    if (!element) {
        ERROR("[vioarr_manager_register_surface] out of memory");
        return;
    }

    ELEMENT_INIT(element, (uintptr_t)vioarr_surface_id(surface), surface);
    
    vioarr_rwlock_w_lock(&globalManager.lock);
    list_append(&globalManager.surfaces, element);
    vioarr_rwlock_w_unlock(&globalManager.lock);
}

void vioarr_manager_unregister_surface(vioarr_surface_t* surface)
{
    if (!surface) {
        ERROR("[vioarr_renderer_register_surface] null parameters");
        return;
    }

    vioarr_rwlock_w_lock(&globalManager.lock);
    element_t* element = list_find(&globalManager.surfaces, (void*)(uintptr_t)vioarr_surface_id(surface));
    if (element) {
        list_remove(&globalManager.surfaces, element);
    }
    vioarr_rwlock_w_unlock(&globalManager.lock);
}

void vioarr_manager_promote_cursor(vioarr_surface_t* surface)
{
    vioarr_rwlock_w_lock(&globalManager.lock);
    element_t* element = list_find(&globalManager.surfaces, (void*)(uintptr_t)vioarr_surface_id(surface));
    if (element) {
        list_remove(&globalManager.surfaces, element);
        list_append(&globalManager.cursors, element);
    }
    vioarr_rwlock_w_unlock(&globalManager.lock);
}

void vioarr_manager_demote_cursor(vioarr_surface_t* surface)
{
    vioarr_rwlock_w_lock(&globalManager.lock);
    element_t* element = list_find(&globalManager.cursors, (void*)(uintptr_t)vioarr_surface_id(surface));
    if (element) {
        list_remove(&globalManager.cursors, element);
        list_append(&globalManager.surfaces, element);
    }
    vioarr_rwlock_w_unlock(&globalManager.lock);
}

void vioarr_manager_render_start(list_t** surfaces, list_t** cursors)
{
    vioarr_rwlock_r_lock(&globalManager.lock);
    if (surfaces) *surfaces = &globalManager.surfaces;
    if (cursors)  *cursors  = &globalManager.cursors;
}

void vioarr_manager_render_end(void)
{
    vioarr_rwlock_r_unlock(&globalManager.lock);
}

vioarr_surface_t* vioarr_manager_front_surface(void)
{
    // back element is front
    element_t* front = list_back(&globalManager.surfaces);
    if (front) {
        return front->value;
    }
    return NULL;
}

vioarr_surface_t* vioarr_manager_surface_at(int x, int y)
{
    vioarr_rwlock_r_lock(&globalManager.lock);
    foreach_reverse(i, &globalManager.surfaces) {
        vioarr_surface_t* surface = i->value;
        if (vioarr_surface_contains(surface, x, y)) {
            return surface;
        }
    }
    vioarr_rwlock_r_unlock(&globalManager.lock);
    return NULL;
}

void vioarr_manager_push_to_front(vioarr_surface_t* surface)
{
    vioarr_rwlock_w_lock(&globalManager.lock);
    element_t* element = list_find(&globalManager.surfaces, (void*)(uintptr_t)vioarr_surface_id(surface));
    if (element) {
        list_remove(&globalManager.surfaces, element);
        list_append(&globalManager.surfaces, element);
    }
    vioarr_rwlock_w_unlock(&globalManager.lock);
}
