/* ValiOS
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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */

#include "../include/widgets/icon.hpp"
#include "../include/memory_pool.hpp"
#include "../include/memory_buffer.hpp"
#include <fstream>
#include <vector>

namespace Asgaard {
    Icon::Icon(uint32_t id, std::shared_ptr<Screen> screen, uint32_t parentId, const Rectangle& dimensions)
        : Surface(id, screen, parentId, dimensions)
    {

    }

    Icon::~Icon() {

    }

    void Icon::LoadIcon(IconState state, std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            // publish error
        }
        file.close();

        // use libpng
    }

    void Icon::SetState(IconState state)
    {
        SetBuffer(m_buffers[static_cast<int>(state)]);
        ApplyChanges();
    }

    void Icon::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                auto poolSize = (m_Dimensions.Height() * m_Dimensions.Width() * 4) *
                    static_cast<int>(IconState::COUNT);
                m_memory = MemoryPool::Create(this, poolSize);
            } break;

            default:
                break;
        }
        
        // Run the base class events as well
        Surface::ExternalEvent(event, data);
    }

    void Icon::Notification(Publisher* source, int event, void* data)
    {
        auto memoryObject = dynamic_cast<MemoryPool*>(source);
        if (memoryObject != nullptr)
        {
            switch (static_cast<MemoryPool::MemoryEvent>(event))
            {
                case MemoryPool::MemoryEvent::CREATED: {
                    auto bufferSize = m_Dimensions.Height() * m_Dimensions.Width() * 4;
                    for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) { 
                        m_buffers[i] = MemoryBuffer::Create(this, m_memory, i * bufferSize,
                            m_Dimensions.Width(), m_Dimensions.Height(), PixelFormat::A8R8G8B8);
                    }
                } break;
                
                case MemoryPool::MemoryEvent::ERROR: {
                    
                } break;
            }
        }
        
        auto bufferObject = dynamic_cast<MemoryBuffer*>(source);
        if (bufferObject != nullptr)
        {
            switch (static_cast<MemoryBuffer::BufferEvent>(event))
            {
                case MemoryBuffer::BufferEvent::CREATED: {
                    // when the last buffer is created the icon is "ready"
                } break;
            }
        }
    }
}
