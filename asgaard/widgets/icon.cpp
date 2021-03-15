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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::string iconStateExtensions[static_cast<int>(
    Asgaard::Widgets::Icon::IconState::COUNT)] = {
    "",
    "_hover",
    "_active",
    "_disabled"
};

static std::string ExtendFilename(std::string& path, std::string& extension)
{
    if (extension == "") {
        return path;
    }

    auto lastTokenPos = path.find_last_of("/\\");
    auto lastDotPos = path.find_last_of(".");
    if (lastDotPos == std::string::npos) {
        // filepath with no extension 
        return path + extension;
    }
    else {
        auto fileExtension = path.substr(lastDotPos);

        if (lastTokenPos != std::string::npos) {
            if (lastDotPos > lastTokenPos) {
                // seperator comes before the last '.', which means the file has an extension
                return path.substr(0, lastDotPos) + extension + fileExtension;
            }
            else {
                // the dot is in a previous path token, which means no file extension
                return path + extension;
            }
        }

        // there is a dot but no seperator
        return path.substr(0, lastDotPos) + extension + fileExtension;
    }
}

namespace Asgaard {
    namespace Widgets {
        Icon::Icon(uint32_t id, const std::shared_ptr<Screen>& screen, uint32_t parentId, const Rectangle& dimensions)
            : SubSurface(id, screen, parentId, dimensions)
            , m_memory(nullptr)
            , m_originalPath("")
            , m_originalWidth(0)
            , m_originalHeight(0)
        {
            for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) {
                m_stateAvailabilityMap[i] = false;
            }
        }
    
        Icon::~Icon()
        {
            Destroy();
        }

        void Icon::Destroy()
        {
            for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) {
                if (m_stateAvailabilityMap[i]) {
                    m_buffers[i]->Unsubscribe(this);
                }
            }

            if (m_memory) { m_memory->Unsubscribe(this); }

            // invoke base destroy as well
            SubSurface::Destroy();
        }
    
        bool Icon::LoadIcon(const std::string& path)
        {
            int numComponents;
    
            if (m_memory) {
                // todo support for replacing icon
                return false;
            }
    
            int status = stbi_info(path.c_str(), &m_originalWidth, &m_originalHeight, &numComponents);
            if (!status) {
                return false;
            }
    
            m_originalPath = path;
    
            auto poolSize = (m_originalWidth * m_originalHeight * 4) * static_cast<int>(IconState::COUNT);
            m_memory = MemoryPool::Create(this, poolSize);
            return true;
        }
    
        void Icon::SetState(IconState state)
        {
            if (!m_buffers[static_cast<int>(state)] || 
                !m_stateAvailabilityMap[static_cast<int>(state)]) {
                return;
            }
    
            SetBuffer(m_buffers[static_cast<int>(state)]);
            MarkDamaged(Dimensions());
            ApplyChanges();
        }
    
        void Icon::ExternalEvent(enum ObjectEvent event, void* data)
        {
            switch (event)
            {
                case ObjectEvent::CREATION: {
                    SetValid(true);
                    Notify(static_cast<int>(Notification::CREATED));
                } break;
                
                case ObjectEvent::ERROR: {
                    Notify(static_cast<int>(Notification::ERROR));
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
                switch (static_cast<MemoryPool::Notification>(event))
                {
                    case MemoryPool::Notification::CREATED: {
                        auto bufferSize = m_originalWidth * m_originalHeight * 4;
                        for (int i = 0; i < static_cast<int>(IconState::COUNT); i++) { 
                            m_buffers[i] = MemoryBuffer::Create(this, m_memory, i * bufferSize,
                                m_originalWidth, m_originalHeight, PixelFormat::A8B8G8R8);
                        }
                    } break;
                    
                    case MemoryPool::Notification::ERROR: {
                        Notify(static_cast<int>(Notification::ERROR));
                    } break;
                }
                return;
            }
            
            auto bufferObject = dynamic_cast<MemoryBuffer*>(source);
            if (bufferObject != nullptr)
            {
                switch (static_cast<MemoryBuffer::Notification>(event))
                {
                    case MemoryBuffer::Notification::CREATED: {
                        auto bufferSize = m_originalWidth * m_originalHeight * 4;
                        int loadedWidth, loadedHeight, loadedComponents;
                        auto state = IconState::NORMAL;

                        if (bufferObject->Id() == m_buffers[static_cast<int>(IconState::HOVERING)]->Id()) {
                            state = IconState::HOVERING;
                        }
                        else if (bufferObject->Id() == m_buffers[static_cast<int>(IconState::ACTIVE)]->Id()) {
                            state = IconState::ACTIVE;
                        }
                        else if (bufferObject->Id() == m_buffers[static_cast<int>(IconState::DISABLED)]->Id()) {
                            state = IconState::DISABLED;
                        }

                        auto extension = iconStateExtensions[static_cast<int>(state)];
                        auto path      = ExtendFilename(m_originalPath, extension);
                        auto buffer    = stbi_load(path.c_str(), &loadedWidth, &loadedHeight, &loadedComponents, STBI_rgb_alpha);
                        if (buffer != nullptr) {
                            if (loadedWidth != m_originalWidth || loadedHeight != m_originalHeight) {
                                Notify(static_cast<int>(Notification::ERROR) /*, error text*/);
                                break;
                            }
                            
                            m_stateAvailabilityMap[static_cast<int>(state)] = true;
                            memcpy(bufferObject->Buffer(), buffer, bufferSize);
                            stbi_image_free(buffer);

                            if (state == IconState::NORMAL) {
                                MarkInputRegion(Rectangle(0, 0, loadedWidth, loadedHeight));
                                SetTransparency(true);
                                SetState(IconState::NORMAL);
                            }
                        }
                    } break;
                    
                    default:
                        break;
                }
            }
        }

        void Icon::OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY)
        {
            SetState(IconState::HOVERING);
        }

        void Icon::OnMouseLeave(const std::shared_ptr<Pointer>&)
        {
            SetState(IconState::NORMAL);
        }

        void Icon::OnMouseClick(const std::shared_ptr<Pointer>&, unsigned int buttons)
        {
            // LMB
            if (buttons & 0x1) {
                SetState(IconState::ACTIVE);
                Notify(static_cast<int>(Notification::CLICKED));
            }
            else {
                SetState(IconState::NORMAL);
            }
        }
    }
}
