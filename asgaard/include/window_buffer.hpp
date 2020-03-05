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

namespace Asgaard {
    class WindowMemory;
    
    class WindowBuffer final : public Object {
    public:
        enum class BufferEvent : int {
            CREATED,
            REFRESHED
        };
        
    public:
        WindowBuffer(uint32_t id, std::shared_ptr<WindowMemory> memory, int memoryOffset, int width, int height, enum PixelFormat format);
        ~WindowBuffer();
        
        void* Buffer() { return m_Buffer; }
        
    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;
        
    private:
        std::shared_ptr<WindowMemory> m_Memory;
        int                           m_Width;
        int                           m_Height;
        enum PixelFormat              m_Format;
        void*                         m_Buffer;
    };
}
