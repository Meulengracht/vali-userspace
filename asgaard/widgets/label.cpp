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
 
#include "../include/widgets/label.hpp"
#include "../include/drawing/painter.hpp"
#include "../include/memory_pool.hpp"
#include "../include/memory_buffer.hpp"
#include "../include/rectangle.hpp"

namespace Asgaard {
    namespace Widgets {
        Label::Label(uint32_t id, std::shared_ptr<Screen> screen, uint32_t parentId, const Rectangle& dimensions)
            : Surface(id, screen, parentId, dimensions)
            , m_memory(nullptr)
            , m_buffer(nullptr)
            , m_font(nullptr)
            , m_text("")
        {
            
        }
        
        Label::~Label()
        {
            
        }
        
        void Label::SetFont(std::shared_ptr<Drawing::Font>& font)
        {
            m_font = font;
            
            if (m_text != "") {
                Redraw();
            }
        }
        
        void Label::SetText(const std::string& text)
        {
            m_text = text;
            
            if (m_font != nullptr) {
                Redraw();
            }
        }
        
        void Label::Redraw()
        {
            Drawing::Painter paint(m_buffer);
            
            paint.SetColor(0, 0, 0);
            paint.RenderFill();
            
            paint.SetFont(m_font);
            paint.RenderText(Dimensions(), m_text);
            
            MarkDamaged(Dimensions());
            ApplyChanges();
        }
        
        void Label::ExternalEvent(enum ObjectEvent event, void* data)
        {
            switch (event)
            {
                case ObjectEvent::CREATION: {
                    auto poolSize = Dimensions().Width() * Dimensions().Height() * 4;
                    m_memory = MemoryPool::Create(this, poolSize);
                } break;
                
                case ObjectEvent::ERROR: {
                    Notify(static_cast<int>(LabelEvent::ERROR));
                } break;
    
                default:
                    break;
            }
            
            // Run the base class events as well
            Surface::ExternalEvent(event, data);
        }
        
        void Label::Notification(Publisher* source, int event, void* data)
        {
            auto memoryObject = dynamic_cast<MemoryPool*>(source);
            if (memoryObject != nullptr) {
                // OK notification from the memory pool
                switch (static_cast<MemoryPool::MemoryEvent>(event))
                {
                    case MemoryPool::MemoryEvent::CREATED: {
                        auto bufferSize = Dimensions().Width() * Dimensions().Height() * 4;
                        m_buffer = MemoryBuffer::Create(this, m_memory, bufferSize,
                            Dimensions().Width(), Dimensions().Height(), PixelFormat::A8R8G8B8);
                    } break;
                    
                    case MemoryPool::MemoryEvent::ERROR: {
                        Notify(static_cast<int>(LabelEvent::ERROR));
                    } break;
                }
            }
            
            auto bufferObject = dynamic_cast<MemoryBuffer*>(source);
            if (bufferObject != nullptr)
            {
                switch (static_cast<MemoryBuffer::BufferEvent>(event))
                {
                    case MemoryBuffer::BufferEvent::CREATED: {
                        SetBuffer(m_buffer);
                        Notify(static_cast<int>(LabelEvent::CREATED));
                    } break;
                    
                    default:
                        break;
                }
            }
        }
    }
}