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

#include "vioarr_objects.h"
#include "../protocols/wm_core_protocol_server.h"
#include <ds/list.h>
#include <stdatomic.h>
#include <stdlib.h>
 
typedef struct vioarr_object {
    uint32_t                 id;
    enum wm_core_object_type type;
    void*                    object;
    UUId_t                   handle;
    
    element_t link;
} vioarr_object_t;
 
static _Atomic(uint32_t) object_id = ATOMIC_VAR_INIT(0x80000000);
static list_t            objects   = LIST_INIT;
 
static uint32_t vioarr_utils_get_object_id(void)
{
    return atomic_fetch_add(&object_id, 1);
}

uint32_t vioarr_objects_create_object(void* object, enum wm_core_object_type type)
{
    vioarr_object_t* resource;
    
    resource = malloc(sizeof(vioarr_object_t));
    if (!resource) {
        return 0;
    }
    
    resource->id     = vioarr_utils_get_object_id();
    resource->object = object;
    resource->type   = type;
    ELEMENT_INIT(&resource->link, (uintptr_t)resource->id, resource);
    
    list_append(&objects, &resource->link);
    return resource->id;
}

void* vioarr_objects_get_object(uint32_t id)
{
    vioarr_object_t* object = list_find_value(&objects, (void*)(uintptr_t)id);
    if (!object){
        return NULL;
    }
    
    return object->object;
}

int vioarr_objects_remove_object(uint32_t id)
{
    vioarr_object_t* object = list_find_value(&objects, (void*)(uintptr_t)id);
    if (!object){
        return -1;
    }
    
    list_remove(&objects, &object->link);
    //wm_core_event_object_all(id); DESTROY EVENT
    free(object);
    return 0;
}

void vioarr_objects_publish(int client)
{
    foreach (i, &objects) {
        vioarr_object_t* object = i->value;
        wm_core_event_object_single(client, object->id, object->handle, object->type);
    }
}
