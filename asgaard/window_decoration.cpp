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

#include "include/object_manager.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"
#include "include/rectangle.hpp"
#include "include/window_decoration.hpp"
#include "include/drawing/painter.hpp"

#include "include/widgets/icon.hpp"
#include "include/widgets/label.hpp"

namespace Asgaard {
    WindowDecoration::WindowDecoration(uint32_t id, const std::shared_ptr<Screen>& screen, uint32_t parentId, const Rectangle& dimensions)
        : Surface(id, screen, parentId, dimensions)
        , m_memory(nullptr)
        , m_buffer(nullptr)
        , m_appFont(nullptr)
        , m_appTitle(nullptr)
        , m_appIcon(nullptr)
        , m_minIcon(nullptr)
        , m_maxIcon(nullptr)
        , m_closeIcon(nullptr)
    {
        
    }
    
    WindowDecoration::~WindowDecoration()
    {
        
    }
    
    void WindowDecoration::SetTitle(const std::string& title)
    {
        if (m_appTitle == nullptr || !m_appTitle->Valid()) {
            return;
        }
        
        m_appTitle->SetText(title);
    }
    
    void WindowDecoration::SetIcon(const std::string& iconPath)
    {
        
    }
    
    void WindowDecoration::Redraw()
    {
        Drawing::Painter paint(m_buffer);
        
        //paint.SetColor(0xCF, 0xC9, 0xCB);
        paint.SetColor(0xF0, 0xF0, 0xF0);
        paint.RenderFill();
        
        MarkDamaged(Dimensions());
        ApplyChanges();
    }
    
    void WindowDecoration::CheckCreation()
    {
        if (m_closeIcon != nullptr && m_closeIcon->Valid() && 
            m_minIcon   != nullptr && m_minIcon->Valid()   && 
            m_maxIcon   != nullptr && m_maxIcon->Valid()   && 
            m_appIcon   != nullptr && m_appIcon->Valid()   &&
            m_appTitle  != nullptr && m_appTitle->Valid())
        {
            SetValid(true);
            Notify(static_cast<int>(Event::CREATED));
        }
    }
    
    void WindowDecoration::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                float halfHeight = (float)Dimensions().Height() / 2.0f;
                int   iconY      = (int)(halfHeight - (16.0f));

                // create resources
                auto poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
                m_memory = MemoryPool::Create(this, poolSize);
                
                // middle
                m_appTitle = OM.CreateClientObject<Asgaard::Widgets::Label>(m_screen, Id(),
                    Rectangle(8 + 8 + 32, iconY, 128, 32));
                m_appTitle->Subscribe(this);
                
                // left corner
                m_appIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(8, iconY, 32, 32));
                m_appIcon->Subscribe(this);
                
                // right corner
                m_minIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(Dimensions().Width() - (3 * (8 + 32)), iconY, 32, 32));
                m_minIcon->Subscribe(this);

                // right corner
                m_maxIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(Dimensions().Width() - (2 * (8 + 32)), iconY, 32, 32));
                m_maxIcon->Subscribe(this);

                // right corner
                m_closeIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(Dimensions().Width() - (8 + 32), iconY, 32, 32));
                m_closeIcon->Subscribe(this);
            } break;
            
            case ObjectEvent::ERROR: {
                Notify(static_cast<int>(Event::ERROR));
            } break;
            
            default:
                break;
        }
        
        // Run the base class events as well
        Surface::ExternalEvent(event, data);
    }
    
    void WindowDecoration::Notification(Publisher* source, int event, void* data)
    {
        auto object = dynamic_cast<Object*>(source);
        if (object == nullptr) {
            return;
        }
        
        if (m_memory != nullptr && object->Id() == m_memory->Id()) {
            switch (static_cast<MemoryPool::MemoryEvent>(event)) {
                case MemoryPool::MemoryEvent::CREATED: {
                    m_buffer = MemoryBuffer::Create(this, m_memory, 0,
                        Dimensions().Width(), Dimensions().Height(), PixelFormat::A8B8G8R8);
                } break;
                
                case MemoryPool::MemoryEvent::ERROR: {
                    Notify(static_cast<int>(Event::ERROR));
                } break;
            }
            
            return;
        }
        
        if (m_buffer != nullptr && object->Id() == m_buffer->Id()) {
            switch (static_cast<MemoryBuffer::BufferEvent>(event)) {
                case MemoryBuffer::BufferEvent::CREATED: {
                    SetBuffer(m_buffer);
                    SetDropShadow(Rectangle(0, 0, 0, 10));
                    Redraw();
                } break;
                
                default:
                    break;
            }
            
            return;
        }
        
        if (m_appIcon != nullptr && object->Id() == m_appIcon->Id()) {
            // load app icon from current package './appIcon.png'
            m_appIcon->LoadIcon("$sys/themes/default/app.png");
            CheckCreation();
            return;
        }
        
        if (m_minIcon != nullptr && object->Id() == m_minIcon->Id()) {
            m_minIcon->LoadIcon("$sys/themes/default/minimize.png");
            CheckCreation();
            return;
        }
        
        if (m_maxIcon != nullptr && object->Id() == m_maxIcon->Id()) {
            m_maxIcon->LoadIcon("$sys/themes/default/maximize.png");
            CheckCreation();
            return;
        }
        
        if (m_closeIcon != nullptr && object->Id() == m_closeIcon->Id()) {
            m_closeIcon->LoadIcon("$sys/themes/default/close.png");
            CheckCreation();
            return;
        }
    }
}
