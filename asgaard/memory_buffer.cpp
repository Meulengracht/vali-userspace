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
#include "include/memory_buffer.hpp"

#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_memory_pool_protocol_client.h"


static int CalculateStride(int width, enum Asgaard::PixelFormat format)
{
    return width * GetBytesPerPixel(format);
}

static enum wm_pixel_format GetWmPixelFormat(enum Asgaard::PixelFormat format)
{
    switch (format) {
        case Asgaard::PixelFormat::A8R8G8B8: return format_a8r8g8b8;
        case Asgaard::PixelFormat::X8R8G8B8: return format_x8r8g8b8;
    }
}

namespace Asgaard {
    MemoryBuffer::MemoryBuffer(uint32_t id, std::shared_ptr<MemoryPool> memory, int memoryOffset, int width, int height, enum PixelFormat format)
        : Object(id)
        , m_memory(memory)
        , m_width(width)
        , m_height(height)
        , m_format(format)
        , m_buffer(memory->CreateBufferPointer(memoryOffset))
    {
        enum wm_pixel_format wmFormat = GetWmPixelFormat(format);
        int                  stride   = CalculateStride(width, format);
        
        wm_memory_pool_create_buffer(APP.GrachtClient(), nullptr, memory->Id(),
            id, memoryOffset, width, height, stride, wmFormat);
    }
    
    MemoryBuffer::~MemoryBuffer()
    {
        
    }
    
    void* MemoryBuffer::Buffer(int x, int y) {
        uint8_t* pointer       = static_cast<uint8_t*>(m_buffer);
        int      bytesPerPixel = GetBytesPerPixel(m_format);
        
        pointer += ((y * (m_width * bytesPerPixel)) + (x * bytesPerPixel));
        return pointer;
    }
    
    void MemoryBuffer::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                Notification(this, static_cast<int>(BufferEvent::CREATED));
            } break;
            
            case ObjectEvent::BUFFER_RELEASE: {
                Notification(this, static_cast<int>(BufferEvent::REFRESHED));
            } break;
            
            default:
                break;
        }
    }
}
