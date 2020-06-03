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
#pragma once

#include "pixel_format.hpp"
#include "object_manager.hpp"
#include "object.hpp"

namespace Asgaard {
    class MemoryPool;
    
    class MemoryBuffer final : public Object {
    public:
        enum class BufferEvent : int {
            CREATED,
            REFRESHED
        };
        
    public:
        MemoryBuffer(uint32_t id, std::shared_ptr<MemoryPool> memory, int memoryOffset, int width, int height, enum PixelFormat format);
        ~MemoryBuffer();
        
        void* Buffer() const { return m_buffer; }
        void* Buffer(int x, int y);
        
        int   Width()  const { return m_width; }
        int   Height() const { return m_height; }
        int   Stride() const { return m_width * GetBytesPerPixel(m_format); }
        
    public:
        static std::shared_ptr<MemoryBuffer> Create(Object* owner, std::shared_ptr<MemoryPool> memory,
            int memoryOffset, int width, int height, enum PixelFormat format)
        {
            auto buffer = OM.CreateClientObject<
                MemoryBuffer, std::shared_ptr<MemoryPool>, int, int, int, enum PixelFormat>(
                    memory, memoryOffset, width, height, format);
            buffer->Subscribe(owner);
            return buffer;
        }
    
    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;
        
    private:
        std::shared_ptr<MemoryPool> m_memory;
        int                         m_width;
        int                         m_height;
        enum PixelFormat            m_format;
        void*                       m_buffer;
    };
}
