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

#include "object_manager.hpp"
#include <os/dmabuf.h>

namespace Asgaard {
    class MemoryPool : public Object {
    public:
        enum class MemoryEvent : int {
            CREATED,
            ERROR
        };
    public:
        MemoryPool(uint32_t id, int size);
        ~MemoryPool();
        
        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;

    static std::shared_ptr<MemoryPool> Create(Object* owner, std::size_t size)
    {
        // Create the memory pool we're going to use
        auto memory = OM.CreateClientObject<MemoryPool, std::size_t>(size);
        memory->Subscribe(owner);
        return memory;
    }
        
    public:
        void* CreateBufferPointer(int memoryOffset);
        
    private:
        int                   m_Size;
        struct dma_attachment m_Attachment;
    };
}