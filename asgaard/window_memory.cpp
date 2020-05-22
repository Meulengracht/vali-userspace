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
#include "include/window_memory.hpp"

#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_memory_protocol_client.h"

namespace Asgaard {
    WindowMemory::WindowMemory(uint32_t id, int size)
        : Object(id), m_Size(size)
    {
        wm_memory_create_pool(APP.GrachtClient(), nullptr, 0 /* memory_system_id */, id, size);
    }
    
    WindowMemory::~WindowMemory()
    {
        
    }
    
    void WindowMemory::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                struct wm_core_object_event* event =
                    (struct wm_core_object_event*)data;
                int status;
                
                status = dma_attach(event->system_handle, &m_Attachment);
                if (status) {
                    Notification(this, static_cast<int>(MemoryEvent::ERROR) /* string text todo */);
                    return;
                }
                
                status = dma_attachment_map(&m_Attachment);
                if (status) {
                    Notification(this, static_cast<int>(MemoryEvent::ERROR) /* string text todo */);
                    return;
                }
                
                Notification(this, static_cast<int>(MemoryEvent::CREATED));
            } break;
            
            default:
                break;
        }
    }
    
    void* WindowMemory::CreateBufferPointer(int memoryOffset)
    {
        uint8_t* bufferPointer = static_cast<uint8_t*>(m_Attachment.buffer);
        if (bufferPointer == nullptr) {
            return nullptr;
        }
        
        return static_cast<void*>(bufferPointer + memoryOffset);
    }
}
