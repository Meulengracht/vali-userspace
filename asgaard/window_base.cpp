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
            
            case SURFACE_RESIZE: {
                
            } break;
            
            case SYNC: {
                OnCreated(this);
            } break;
        }
    }
    
    void WindowBase::Notification(Publisher* source, int event, void* data)
    {
        auto memoryObject = dynamic_cast<WindowMemory*>(source);
        if (memoryObject != nullptr)
        {
            switch (event)
            {
                case WindowMemory::Event::POOL_CREATED: {
                    OnCreated(source);
                } break;
            }
        }
        
        auto bufferObject = dynamic_cast<WindowBuffer*>(source);
        if (bufferObject != nullptr)
        {
            switch (event)
            {
                case WindowBuffer::Event::BUFFER_CREATED: {
                    OnCreated(source);
                } break;
                case WindowBuffer::Event::BUFFER_REFRESHED: {
                    OnRefreshed(bufferObject);
                } break;
            }
        }
    }
    
    std::shared_ptr<WindowMemory> WindowBase::CreateMemory(std::size_t size)
    {
        // Create the window memory pool we're going to use
        auto memory = OM.CreateClientObject<WindowMemory, const Rectangle&>(m_Screen->Dimensions());
        memory->Subscribe(this);
        return memory;
    }
    
    std::shared_ptr<WindowBuffer> CreateBuffer(std::shared_ptr<WindowMemory> memory,
        int memoryOffset, int width, int height, enum PixelFormat format)
    {
        auto buffer = OM.CreateClientObject<WindowBuffer, int, int, int, enum PixelFormat>(
            memory->Id(), memoryOffset, width, height, format);
        buffer->Subscribe(this);
        return buffer;
    }

    void WindowBase::SetBuffer(std::shared_ptr<WindowBuffer> buffer)
    {
        
    }
    
    void WindowBase::SetTitle(const std::string& title)
    {
        
    }
    
    void WindowBase::ApplyChanges()
    {
        
    }
    
    void WindowBase::Shutdown()
    {
        
    }
}
