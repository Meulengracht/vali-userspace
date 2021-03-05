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
#include "include/key_event.hpp"
#include "include/pointer.hpp"
#include "include/object_manager.hpp"
#include "include/memory_buffer.hpp"
#include "include/surface.hpp"

#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_screen_protocol_client.h"
#include "protocols/wm_memory_protocol_client.h"
#include "protocols/wm_surface_protocol_client.h"
#include "protocols/wm_pointer_protocol_client.h"

static enum Asgaard::Surface::SurfaceEdges GetSurfaceEdges(enum wm_surface_edge edges)
{
    return Asgaard::Surface::SurfaceEdges::NONE;
}

namespace Asgaard {
    Surface::Surface(uint32_t id, const std::shared_ptr<Screen>& screen, uint32_t parent_id, const Rectangle& dimensions)
        : Object(id)
        , m_screen(nullptr)
        , m_parent(OM[parent_id])
        , m_dimensions(dimensions)
    {
        // Try to perform binding.
        BindToScreen(screen);
    }
    
    Surface::Surface(uint32_t id, const std::shared_ptr<Screen>& screen, const Rectangle& dimensions)
        : Surface(id, screen, 0, dimensions) { }

    Surface::Surface(uint32_t id, const Rectangle& dimensions)
        : Surface(id, std::shared_ptr<Screen>(nullptr), 0, dimensions) { }

    Surface::~Surface()
    {
        wm_surface_destroy(APP.GrachtClient(), nullptr, Id());
    }
    
    void Surface::BindToScreen(const std::shared_ptr<Screen>& screen)
    {
        // If we previously were not attached to a screen and now are attaching
        // then we need to provide an underlying surface
        if (m_screen == nullptr && screen != nullptr) {
            wm_screen_create_surface(APP.GrachtClient(), nullptr, screen->Id(), Id(),    
                m_dimensions.Width(), m_dimensions.Height());
            if (m_parent != nullptr) {
                wm_surface_add_subsurface(APP.GrachtClient(), nullptr, m_parent->Id(),
                    Id(), m_dimensions.X(), m_dimensions.Y());
            }
        }
        m_screen = screen;
    }
    
    void Surface::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event) {
            case ObjectEvent::SURFACE_RESIZE: {
                struct wm_surface_resize_event* event = 
                    (struct wm_surface_resize_event*)data;
                
                // When we get a resize event, the event is sent only to the parent surface
                // which equals this instance. Now we have to invoke the RESIZE event for all
                // registered children
                OnResized(GetSurfaceEdges(event->edges), event->width, event->height);
                m_dimensions.SetWidth(event->width);
                m_dimensions.SetHeight(event->height);
            } break;
            
            case ObjectEvent::SURFACE_FRAME: {
                OnFrame();
            } break;

            case ObjectEvent::KEY_EVENT: {
                OnKeyEvent(KeyEvent((struct wm_keyboard_key_event*)data));
            } break;

            case ObjectEvent::SURFACE_FOCUSED: {
                struct wm_surface_focus_event* event = 
                    (struct wm_surface_focus_event*)data;
                OnFocus((bool)event->focus);
            } break;

            case ObjectEvent::POINTER_ENTER: {
                struct wm_pointer_enter_event* event = 
                    (struct wm_pointer_enter_event*)data;
                auto pointer = Asgaard::OM[event->pointer_id];
                OnMouseEnter(std::dynamic_pointer_cast<Pointer>(pointer), event->surface_x, event->surface_y);
            } break;

            case ObjectEvent::POINTER_LEAVE: {
                struct wm_pointer_leave_event* event = 
                    (struct wm_pointer_leave_event*)data;
                auto pointer = Asgaard::OM[event->pointer_id];
                OnMouseLeave(std::dynamic_pointer_cast<Pointer>(pointer));
            } break;

            case ObjectEvent::POINTER_MOVE: {
                struct wm_pointer_move_event* event = 
                    (struct wm_pointer_move_event*)data;
                auto pointer = Asgaard::OM[event->pointer_id];
                OnMouseMove(std::dynamic_pointer_cast<Pointer>(pointer), event->surface_x, event->surface_y);
            } break;

            case ObjectEvent::POINTER_CLICK: {
                struct wm_pointer_click_event* event = 
                    (struct wm_pointer_click_event*)data;
                auto pointer = Asgaard::OM[event->pointer_id];
                OnMouseClick(std::dynamic_pointer_cast<Pointer>(pointer), event->buttons);
            } break;

            default:
                break;
        }
    }
    
    void Surface::SetBuffer(const std::shared_ptr<MemoryBuffer>& buffer)
    {
        wm_surface_set_buffer(APP.GrachtClient(), nullptr, Id(), buffer->Id());
    }
    
    void Surface::MarkDamaged(const Rectangle& dimensions)
    {
        wm_surface_invalidate(APP.GrachtClient(), nullptr, Id(),
            dimensions.X(), dimensions.Y(),
            dimensions.Width(), dimensions.Height());
    }
    
    void Surface::MarkInputRegion(const Rectangle& dimensions)
    {
        wm_surface_set_input_region(APP.GrachtClient(), nullptr, Id(),
            dimensions.X(), dimensions.Y(),
            dimensions.Width(), dimensions.Height());
    }

    void Surface::SetDropShadow(const Rectangle& dimensions)
    {
        wm_surface_set_drop_shadow(APP.GrachtClient(), nullptr, Id(),
            dimensions.X(), dimensions.Y(), dimensions.Width(), dimensions.Height());
    }

    void Surface::SetTransparency(bool enable)
    {
        wm_surface_set_transparency(APP.GrachtClient(), nullptr, Id(), (int)enable);
    }
    
    void Surface::ApplyChanges()
    {
        wm_surface_commit(APP.GrachtClient(), nullptr, Id());
    }
    
    void Surface::RequestFrame()
    {
        wm_surface_request_frame(APP.GrachtClient(), nullptr, Id());
    }
    
    void Surface::OnResized(enum SurfaceEdges edge, int width, int height) { }
    void Surface::OnFocus(bool focus) { }
    void Surface::OnFrame() { }
    void Surface::OnMouseEnter(const std::shared_ptr<Pointer>&, int localX, int localY) { }
    void Surface::OnMouseLeave(const std::shared_ptr<Pointer>&) { }
    void Surface::OnMouseMove(const std::shared_ptr<Pointer>&, int localX, int localY) { }
    void Surface::OnMouseClick(const std::shared_ptr<Pointer>&, unsigned int buttons) { }
    void Surface::OnKeyEvent(const KeyEvent&) { }
}
