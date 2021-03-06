/* ValiOS
 *
 * Copyright 2018, Philip Meulengracht
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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */

#include "include/application.hpp"
#include "include/memory_pool.hpp"

#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_memory_pool_protocol_client.h"
#include "protocols/wm_memory_protocol_client.h"

namespace Asgaard {
    MemoryPool::MemoryPool(uint32_t id, int size)
        : Object(id)
        , m_size(size)
        , m_attachment({ 0 })
    {
        wm_memory_create_pool(APP.GrachtClient(), nullptr, 0 /* memory_system_id */, id, size);
    }
    
    MemoryPool::~MemoryPool()
    {
        if (m_attachment.buffer) {
            dma_attachment_unmap(&m_attachment);
            dma_detach(&m_attachment);
        }
        wm_memory_pool_destroy(APP.GrachtClient(), nullptr, Id());
    }
    
    void MemoryPool::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                struct wm_core_object_event* event =
                    (struct wm_core_object_event*)data;
                OsStatus_t status;
                
                status = dma_attach(event->system_handle, &m_attachment);
                if (status != OsSuccess) {
                    Notify(static_cast<int>(Notification::ERROR) /* string text todo */);
                    return;
                }
                
                status = dma_attachment_map(&m_attachment, DMA_ACCESS_WRITE);
                if (status != OsSuccess) {
                    Notify(static_cast<int>(Notification::ERROR) /* string text todo */);
                    return;
                }
                
                Notify(static_cast<int>(Notification::CREATED));
            } break;
            
            default:
                break;
        }
    }
    
    void* MemoryPool::CreateBufferPointer(int memoryOffset)
    {
        uint8_t* bufferPointer = static_cast<uint8_t*>(m_attachment.buffer);
        if (bufferPointer == nullptr) {
            return nullptr;
        }
        
        return static_cast<void*>(bufferPointer + memoryOffset);
    }
}
