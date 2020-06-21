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
        
        paint.SetColor(64, 64, 64);
        paint.RenderFill();
        
        MarkDamaged(Dimensions());
        ApplyChanges();
    }
    
    void WindowDecoration::CheckCreation()
    {
        if (m_closeIcon != nullptr && m_closeIcon->Valid() && 
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
                // left corner
                Rectangle appIconSize(8, 8, 32, 32);
                
                // right corner
                Rectangle closeIconSize(Dimensions().Width() - (8 + 32), 8, 32, 32);
                
                // next to app
                Rectangle labelSize(8 + 8 + 32, 8, 128, 32);
                auto      poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
                
                // create resources
                m_memory = MemoryPool::Create(this, poolSize);
                
                m_appTitle = OM.CreateClientObject<Asgaard::Widgets::Label>(m_screen, Id(), labelSize);
                m_appTitle->Subscribe(this);
                
                m_appIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(), appIconSize);
                m_appIcon->Subscribe(this);
                
                m_closeIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(), closeIconSize);
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
                        Dimensions().Width(), Dimensions().Height(), PixelFormat::A8R8G8B8);
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
                    Redraw();
                } break;
                
                default:
                    break;
            }
            
            return;
        }
        
        if (m_appIcon != nullptr && object->Id() == m_appIcon->Id()) {
            // load app icon from current package './appIcon.png'
            
            CheckCreation();
            return;
        }
        
        
        if (m_closeIcon != nullptr && object->Id() == m_closeIcon->Id()) {
            m_closeIcon->LoadIcon("$sys/themes/default/close_inactive.png");
            CheckCreation();
            return;
        }
    }
}
