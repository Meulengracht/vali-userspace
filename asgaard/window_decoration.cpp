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
#include "include/drawing/font_manager.hpp"
#include "include/drawing/font.hpp"

#include "include/widgets/icon.hpp"
#include "include/widgets/label.hpp"

#define ICON_SIZE 16

namespace Asgaard {
    WindowDecoration::WindowDecoration(uint32_t id, const std::shared_ptr<Screen>& screen, uint32_t parentId, const Rectangle& dimensions)
        : WindowDecoration(id, screen, parentId, dimensions, Drawing::FM.CreateFont("$sys/fonts/DejaVuSansMono.ttf", DECORATION_TEXT_SIZE)) { }

    WindowDecoration::WindowDecoration(uint32_t id, 
        const std::shared_ptr<Screen>& screen,
        uint32_t parentId,
        const Rectangle& dimensions,
        const std::shared_ptr<Drawing::Font>& font) 
        : Surface(id, screen, parentId, dimensions)
        , m_memory(nullptr)
        , m_buffer(nullptr)
        , m_appFont(font)
        , m_appTitle(nullptr)
        , m_appIcon(nullptr)
        , m_minIcon(nullptr)
        , m_maxIcon(nullptr)
        , m_closeIcon(nullptr)
        , m_redraw(false)
        , m_redrawReady(false)
    {
        
    }
    
    WindowDecoration::~WindowDecoration()
    {
        
    }
    
    void WindowDecoration::SetTitle(const std::string& title)
    {
        if (m_appTitle && m_appTitle->Valid()) {
            m_appTitle->SetText(title);
            m_appTitle->RequestRedraw();
        }
    }
    
    void WindowDecoration::SetIcon(const std::string& iconPath)
    {
        if (m_appIcon && m_appIcon->Valid()) {
            m_appIcon->LoadIcon(iconPath);
        }
    }
    
    void WindowDecoration::Redraw()
    {
        Drawing::Painter paint(m_buffer);
        paint.SetFillColor(DECORATION_FILL_COLOR);
        paint.RenderFill();
        
        MarkDamaged(Dimensions());
        ApplyChanges();
    }

    void WindowDecoration::RequestRedraw()
    {
        bool shouldRedraw = m_redrawReady.exchange(false);
        if (shouldRedraw) {
            Redraw();
        }
        else {
            m_redraw = true;
        }
    }

    void WindowDecoration::RedrawReady()
    {
        // Request redraw
        if (m_redraw) {
            Redraw();
            m_redraw = false;
        }
        else {
            m_redrawReady.store(true);
        }
    }
    
    void WindowDecoration::CheckCreation()
    {
        if (m_closeIcon && m_closeIcon->Valid() && 
            m_minIcon   && m_minIcon->Valid()   && 
            m_maxIcon   && m_maxIcon->Valid()   && 
            m_appIcon   && m_appIcon->Valid()   &&
            m_appTitle  && m_appTitle->Valid())
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
                int   iconY      = (int)(halfHeight - (ICON_SIZE / 2.0f));

                // create memory resources
                auto poolSize = (Dimensions().Width() * Dimensions().Height() * 4);
                m_memory = MemoryPool::Create(this, poolSize);
                
                // left corner
                m_appIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(8, (int)(halfHeight - (12.0f)), 24, 24));
                m_appIcon->Subscribe(this);

                // middle
                m_appTitle = OM.CreateClientObject<Asgaard::Widgets::Label>(m_screen, Id(),
                    Rectangle(
                        8 + 8 + 24, // start text next to app icon
                        (int)(halfHeight - (m_appFont->GetFontHeight() / 2.0f)), 
                        Dimensions().Width() - ((3 * (8 + ICON_SIZE)) + 8), // let text be as wide as till the 3 buttons
                        m_appFont->GetFontHeight())); // allow text up to 18
                m_appTitle->Subscribe(this);
                
                // right corner
                m_minIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(Dimensions().Width() - (3 * (8 + ICON_SIZE)), 8.0f, ICON_SIZE, ICON_SIZE));
                m_minIcon->Subscribe(this);

                // right corner
                m_maxIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(Dimensions().Width() - (2 * (8 + ICON_SIZE)), 8.0f, ICON_SIZE, ICON_SIZE));
                m_maxIcon->Subscribe(this);

                // right corner
                m_closeIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_screen, Id(),
                    Rectangle(Dimensions().Width() - (8 + ICON_SIZE), 8.0f, ICON_SIZE, ICON_SIZE));
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
                    RedrawReady();
                } break;
                
                default:
                    break;
            }
            
            return;
        }
        
        if (m_appTitle && object->Id() == m_appTitle->Id()) {
            m_appTitle->SetFont(m_appFont);
            m_appTitle->SetAnchors(Widgets::Label::Anchors::CENTER);
            m_appTitle->SetBackgroundColor(DECORATION_FILL_COLOR);
            m_appTitle->SetTextColor(DECORATION_TEXT_COLOR);
            CheckCreation();
            return;
        }
        
        if (m_appIcon && object->Id() == m_appIcon->Id()) {
            // load app icon from current package './appIcon.png'
            m_appIcon->LoadIcon("$sys/themes/default/app.png");
            CheckCreation();
            return;
        }
        
        if (m_minIcon && object->Id() == m_minIcon->Id()) {
            m_minIcon->LoadIcon("$sys/themes/default/minimize.png");
            CheckCreation();
            return;
        }
        
        if (m_maxIcon && object->Id() == m_maxIcon->Id()) {
            m_maxIcon->LoadIcon("$sys/themes/default/maximize.png");
            CheckCreation();
            return;
        }
        
        if (m_closeIcon && object->Id() == m_closeIcon->Id()) {
            m_closeIcon->LoadIcon("$sys/themes/default/close.png");
            CheckCreation();
            return;
        }
    }
}
