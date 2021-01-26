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
#include "../include/drawing/font.hpp"
#include "../include/memory_pool.hpp"
#include "../include/memory_buffer.hpp"
#include "../include/rectangle.hpp"

namespace Asgaard {
    namespace Widgets {
        Label::Label(uint32_t id, const std::shared_ptr<Screen>& screen, uint32_t parentId, const Rectangle& dimensions)
            : Surface(id, screen, parentId, dimensions)
            , m_memory(nullptr)
            , m_buffer(nullptr)
            , m_font(nullptr)
            , m_text("")
            , m_anchors(Anchors::TOP | Anchors::LEFT)
            , m_textColor(0xFF, 0xFF, 0xFF, 0xFF)
            , m_fillColor(0x0, 0xF0, 0xF0, 0xF0)
            , m_redraw(false)
            , m_redrawReady(false)
        {
            
        }
        
        Label::~Label()
        {
            
        }
            
        void Label::SetBackgroundColor(const Drawing::Color& color)
        {
            m_fillColor = color;
        }

        void Label::SetTextColor(const Drawing::Color& color)
        {
            m_textColor = color;
        }
        
        void Label::SetFont(const std::shared_ptr<Drawing::Font>& font)
        {
            m_font = font;
        }
        
        void Label::SetText(const std::string& text)
        {
            m_text = text;
        }

        void Label::SetAnchors(Anchors anchors)
        {
            m_anchors = anchors;
        }

        void Label::RequestRedraw()
        {
            bool shouldRedraw = m_redrawReady.exchange(false);
            if (shouldRedraw) {
                Redraw();
            }
            else {
                m_redraw = true;
            }
        }

        void Label::RedrawReady()
        {
            if (m_redraw) {
                Redraw();
                m_redraw = false;
            }
            else {
                m_redrawReady.store(true);
            }
        }
        
        void Label::Redraw()
        {
            Drawing::Painter paint(m_buffer);
            Rectangle textDimensions(m_font->GetTextMetrics(m_text));
            int x = CalculateXCoordinate(textDimensions);
            int y = CalculateYCoordinate(textDimensions);

            paint.SetFillColor(m_fillColor);
            paint.RenderFill();
            
            paint.SetFont(m_font);
            paint.SetOutlineColor(m_textColor);
            paint.RenderText(x, y, m_text);

            MarkDamaged(Dimensions());
            ApplyChanges();
        }

        int Label::CalculateXCoordinate(const Rectangle& textDimensions)
        {
            if ((m_anchors & Anchors::RIGHT) == Anchors::RIGHT) {
                return std::max(0, Dimensions().Width() - textDimensions.Width());
            }
            else if ((m_anchors & Anchors::CENTER) == Anchors::CENTER) {
                int centerDims = Dimensions().Width() >> 1;
                int centerText = textDimensions.Width() >> 1;
                return std::max(0, centerDims - centerText);
            }

            return 0;
        }

        int Label::CalculateYCoordinate(const Rectangle& textDimensions)
        {
            if ((m_anchors & Anchors::BOTTOM) == Anchors::BOTTOM) {
                return std::max(0, Dimensions().Height() - textDimensions.Height());
            }
            else if ((m_anchors & Anchors::CENTER) == Anchors::CENTER) {
                int centerDims = Dimensions().Height() >> 1;
                int centerText = textDimensions.Height() >> 1;
                return std::max(0, centerDims - centerText);
            }

            return 0;
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
                        m_buffer = MemoryBuffer::Create(this, m_memory, 0,
                            Dimensions().Width(), Dimensions().Height(), PixelFormat::A8B8G8R8);
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
                        RedrawReady();
                        SetValid(true);
                        Notify(static_cast<int>(LabelEvent::CREATED));
                    } break;

                    case MemoryBuffer::BufferEvent::REFRESHED: {
                        RedrawReady();
                    } break;
                    
                    default:
                        break;
                }
            }
        }
    }
}