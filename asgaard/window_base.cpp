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

#include "include/window_base.hpp"
#include "inclued/object_manager.hpp"
#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_screen_protocol_client.h"
#include "protocols/wm_memory_protocol_client.h"

namespace Asgaard {
    WindowBase::WindowBase(uint32_t id, std::shared_ptr<Screen> screen, const Rectangle& dimensions)
        : Object(id), m_Invalidated(false), m_Screen(screen), m_Memory(nullptr)
    {
        // Create a new surface with the window manager
        wm_screen_create_surface(WM.GetGrachtClient(), screen->Id(), id,    
            dimensions.Width(), dimensions.Height());
        wm_core_sync(WM.GetGrachtClient(), id);
    }

    WindowBase::~WindowBase()
    {
        
    }
    
    void WindowBase::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case CREATION: {
                // retrieve a list of supported window content formats
                wm_surface_get_formats(WM.GetGrachtClient(), Id());
            } break;
            
            case SURFACE_FORMAT: {
                struct wm_surface_format_event* event = 
                    (struct wm_surface_format_event*)data;
                m_SupportedFormats.push_back(event->format);
            } break;
            
            case SYNC: {
                // Create the window memory pool we're going to use
                m_Memory = OM.CreateClientObject<WindowMemory, const Rectangle&>(m_Screen->Dimensions());
                m_Memory.Subscribe(this);
            } break;
        }
    }
    
    void WindowBase::Notification(Publisher* source, int event, void* data)
    {
        if (source == m_Memory.get())
        {
            switch (event)
            {
                case WindowMemory::Event::POOL_CREATED: {
                    // The pool was created, now we request a buffer
                    enum PixelFormat format = GetPrefferedPixelFormat(m_SupportedFormats);
                    m_Buffer = OM.CreateClientObject<WindowBuffer, int, const Rectangle&, enum PixelFormat>(
                        m_Memory->Id(), 0, m_Screen->Dimensions(), format);
                    m_Buffer.Subscribe(this);
                } break;
            }
        }
        
        if (source == m_Buffer.get())
        {
            switch (event)
            {
                case WindowBuffer::Event::BUFFER_REFRESHED: {
                    OnRefreshed(dynamic_cast<WindowBuffer*>(source));
                } break;
            }
        }
    }
    
    enum PixelFormat WindowBase::GetPrefferedPixelFormat(std::vector<enum PixelFormat>& availableFormats)
    {
        // select system preferable
        return PixelFormat::A8R8G8B8;
    }
    
    void WindowBase::Shutdown()
    {
        
    }

    WindowContent* WindowBase::CreateContent(const Rectangle&)
    {
        return nullptr;
    }
    
    void WindowBase::SetContent(WindowContent*)
    {
        
    }
    
    void WindowBase::ApplyChanges()
    {
        
    }
}
