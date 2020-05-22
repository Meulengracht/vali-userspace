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

#include "include/application.hpp"
#include "include/window_base.hpp"
#include "include/object_manager.hpp"
#include "include/window_memory.hpp"
#include "include/window_buffer.hpp"

#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_screen_protocol_client.h"
#include "protocols/wm_memory_protocol_client.h"
#include "protocols/wm_surface_protocol_client.h"

static enum Asgaard::Surface::SurfaceEdges GetWindowEdges(enum wm_surface_edge edges)
{
    return Asgaard::Surface::SurfaceEdges::NONE;
}

static enum wm_surface_edge ToWindowEdges(enum Asgaard::Surface::SurfaceEdges edges)
{
    return no_edges;
}

namespace Asgaard {
    WindowBase::WindowBase(uint32_t id, const Rectangle& dimensions)
        : Surface(id, dimensions), m_Invalidated(false)
    {
        
    }

    WindowBase::~WindowBase()
    {
        
    }
    
    void WindowBase::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                // retrieve a list of supported window content formats
                wm_surface_get_formats(APP.GrachtClient(), nullptr, Id());
                wm_core_sync(APP.GrachtClient(), nullptr, Id());
            } break;
            
            case ObjectEvent::SURFACE_FORMAT: {
                struct wm_surface_format_event* event = 
                    (struct wm_surface_format_event*)data;
                m_SupportedFormats.push_back((enum PixelFormat)event->format);
            } break;
            
            case ObjectEvent::SYNC: {
                OnCreated(this);
            } break;
            
            default:
                break;
        }
        
        // Run the base class events as well
        Surface::ExternalEvent(event, data);
    }
    
    void WindowBase::Notification(Publisher* source, int event, void* data)
    {
        auto memoryObject = dynamic_cast<WindowMemory*>(source);
        if (memoryObject != nullptr)
        {
            switch (static_cast<WindowMemory::MemoryEvent>(event))
            {
                case WindowMemory::MemoryEvent::CREATED: {
                    OnCreated(memoryObject);
                } break;
                
                case WindowMemory::MemoryEvent::ERROR: {
                    
                } break;
            }
        }
        
        auto bufferObject = dynamic_cast<WindowBuffer*>(source);
        if (bufferObject != nullptr)
        {
            switch (static_cast<WindowBuffer::BufferEvent>(event))
            {
                case WindowBuffer::BufferEvent::CREATED: {
                    OnCreated(bufferObject);
                } break;
                case WindowBuffer::BufferEvent::REFRESHED: {
                    OnRefreshed(bufferObject);
                } break;
            }
        }
    }
    
    std::shared_ptr<WindowMemory> WindowBase::CreateMemory(std::size_t size)
    {
        // Create the window memory pool we're going to use
        auto memory = OM.CreateClientObject<WindowMemory, std::size_t>(size);
        memory->Subscribe(this);
        return memory;
    }
    
    std::shared_ptr<WindowBuffer> WindowBase::CreateBuffer(std::shared_ptr<WindowMemory> memory,
        int memoryOffset, int width, int height, enum PixelFormat format)
    {
        auto buffer = OM.CreateClientObject<
            WindowBuffer, std::shared_ptr<WindowMemory>, int, int, int, enum PixelFormat>(
                memory, memoryOffset, width, height, format);
        buffer->Subscribe(this);
        return buffer;
    }
    
    void WindowBase::InititateResize(enum SurfaceEdges edges)
    {
        wm_surface_resize(APP.GrachtClient(), nullptr, Id(), ToWindowEdges(edges));
    }
    
    void WindowBase::InitiateMove()
    {
        wm_surface_move(APP.GrachtClient(), nullptr, Id());
    }
}
