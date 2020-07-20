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

#include <cstring>
#include <gracht/client.h>
#include <gracht/link/socket.h>
#include "include/application.hpp"
#include "include/screen.hpp"
#include "include/object_manager.hpp"
#include "include/window_base.hpp"
#include <inet/socket.h>
#include <inet/local.h>
#include <type_traits>

#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_screen_protocol_client.h"
#include "protocols/wm_surface_protocol_client.h"
#include "protocols/wm_buffer_protocol_client.h"
#include "protocols/wm_pointer_protocol_client.h"
#include "protocols/wm_keyboard_protocol_client.h"

extern "C" {
    static void wm_core_event_error_callback(struct wm_core_error_event*);
    static void wm_core_event_sync_callback(struct wm_core_sync_event*);
    static void wm_core_event_object_callback(struct wm_core_object_event*);
 
    static gracht_protocol_function_t wm_core_callbacks[3] = {
        { PROTOCOL_WM_CORE_EVENT_ERROR_ID ,  (void*)wm_core_event_error_callback },
        { PROTOCOL_WM_CORE_EVENT_SYNC_ID ,   (void*)wm_core_event_sync_callback },
        { PROTOCOL_WM_CORE_EVENT_OBJECT_ID , (void*)wm_core_event_object_callback },
    };
    DEFINE_WM_CORE_CLIENT_PROTOCOL(wm_core_callbacks, 3);

    static void wm_screen_event_screen_properties_callback(struct wm_screen_screen_properties_event*);
    static void wm_screen_event_mode_callback(struct wm_screen_mode_event*);
 
    static gracht_protocol_function_t wm_screen_callbacks[2] = {
        { PROTOCOL_WM_SCREEN_EVENT_SCREEN_PROPERTIES_ID , (void*)wm_screen_event_screen_properties_callback },
        { PROTOCOL_WM_SCREEN_EVENT_MODE_ID , (void*)wm_screen_event_mode_callback },
    };
    DEFINE_WM_SCREEN_CLIENT_PROTOCOL(wm_screen_callbacks, 2);

    static void wm_surface_event_format_callback(struct wm_surface_format_event*);
    static void wm_surface_event_frame_callback(struct wm_surface_frame_event*);
    static void wm_surface_event_resize_callback(struct wm_surface_resize_event*);
    static void wm_surface_event_focus_callback(struct wm_surface_focus_event*);

    static gracht_protocol_function_t wm_surface_callbacks[4] = {
        { PROTOCOL_WM_SURFACE_EVENT_FORMAT_ID , (void*)wm_surface_event_format_callback },
        { PROTOCOL_WM_SURFACE_EVENT_FRAME_ID ,  (void*)wm_surface_event_frame_callback },
        { PROTOCOL_WM_SURFACE_EVENT_RESIZE_ID , (void*)wm_surface_event_resize_callback },
        { PROTOCOL_WM_SURFACE_EVENT_FOCUS_ID , (void*)wm_surface_event_focus_callback },
    };
    DEFINE_WM_SURFACE_CLIENT_PROTOCOL(wm_surface_callbacks, 4);

    static void wm_buffer_event_release_callback(struct wm_buffer_release_event*);

    static gracht_protocol_function_t wm_buffer_callbacks[1] = {
        { PROTOCOL_WM_BUFFER_EVENT_RELEASE_ID , (void*)wm_buffer_event_release_callback },
    };
    DEFINE_WM_BUFFER_CLIENT_PROTOCOL(wm_buffer_callbacks, 1);

    static void wm_pointer_event_enter_callback(struct wm_pointer_enter_event*);
    static void wm_pointer_event_leave_callback(struct wm_pointer_leave_event*);
    static void wm_pointer_event_move_callback(struct wm_pointer_move_event*);
    static void wm_pointer_event_click_callback(struct wm_pointer_click_event*);
 
    static gracht_protocol_function_t wm_pointer_callbacks[4] = {
        { PROTOCOL_WM_POINTER_EVENT_ENTER_ID , (void*)wm_pointer_event_enter_callback },
        { PROTOCOL_WM_POINTER_EVENT_LEAVE_ID , (void*)wm_pointer_event_leave_callback },
        { PROTOCOL_WM_POINTER_EVENT_MOVE_ID ,  (void*)wm_pointer_event_move_callback },
        { PROTOCOL_WM_POINTER_EVENT_CLICK_ID , (void*)wm_pointer_event_click_callback },
    };
    DEFINE_WM_POINTER_CLIENT_PROTOCOL(wm_pointer_callbacks, 4);
 
    static void wm_keyboard_event_key_callback(struct wm_keyboard_key_event*);
 
    static gracht_protocol_function_t wm_keyboard_callbacks[1] = {
        { PROTOCOL_WM_KEYBOARD_EVENT_KEY_ID , (void*)wm_keyboard_event_key_callback },
    };
    DEFINE_WM_KEYBOARD_CLIENT_PROTOCOL(wm_keyboard_callbacks, 1);
}

int gracht_os_get_server_client_address(struct sockaddr_storage* address, int* address_length_out)
{
    struct sockaddr_lc* local_address = sstolc(address);
    *address_length_out               = sizeof(struct sockaddr_lc);

    // Prepare the server address.
    memset(local_address, 0, sizeof(struct sockaddr_lc));
    memcpy(&local_address->slc_addr[0], LCADDR_WM0, strlen(LCADDR_WM0));
    local_address->slc_len    = sizeof(struct sockaddr_lc);
    local_address->slc_family = AF_LOCAL;
    return 0;
}

namespace Asgaard {
    Application APP;
    
    Application::Application() 
        : Object(0)
        , m_client(nullptr)
        , m_window(nullptr) { }
    
    Application::~Application() { }

    int Application::Initialize()
    {
        struct socket_client_configuration linkConfiguration;
        struct gracht_client_configuration clientConfiguration;
        int                                status;
        
        linkConfiguration.type = gracht_link_stream_based;
        gracht_os_get_server_client_address(&linkConfiguration.address, &linkConfiguration.address_length);
        
        gracht_link_socket_client_create(&clientConfiguration.link, &linkConfiguration);
        status = gracht_client_create(&clientConfiguration, &m_client);
        if (status) {
            // log
            return -1;
        }
        
        gracht_client_register_protocol(m_client, &wm_core_client_protocol);
        gracht_client_register_protocol(m_client, &wm_screen_client_protocol);
        gracht_client_register_protocol(m_client, &wm_surface_client_protocol);
        gracht_client_register_protocol(m_client, &wm_buffer_client_protocol);
        gracht_client_register_protocol(m_client, &wm_pointer_client_protocol);
        gracht_client_register_protocol(m_client, &wm_keyboard_client_protocol);
        
        // kick off a chain reaction by asking for all objects
        return wm_core_get_objects(m_client, nullptr);
    }
    
    int Application::Execute()
    {
        char* messageBuffer = new char[GRACHT_MAX_MESSAGE_SIZE];
        
        while (true) {
            if (gracht_client_wait_message(m_client, nullptr, messageBuffer, GRACHT_WAIT_BLOCK)) {
                // todo log it
            }
        }
        
        delete[] messageBuffer;
        return 0;
    }

    int Application::Shutdown()
    {
        if (m_client != nullptr) {
            gracht_client_shutdown(m_client);
        }
        return 0;
    }
    
    void Application::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event) {
            case ObjectEvent::CREATION: {
                struct wm_core_object_event* event =
                    (struct wm_core_object_event*)data;
                
                // Handle new server objects
                switch (event->type) {
                    case object_type_screen: {
                        auto screen = Asgaard::OM.CreateServerObject<Asgaard::Screen>(event->object_id);
                        screen->Subscribe(this);
                    } break;
                    
                    default:
                        break;
                }
                
            }
            case ObjectEvent::ERROR: {
                // todo app error for those who listen
            } break;
            
            default:
                break;
        }
    }
    
    void Application::Notification(Publisher* source, int event, void* data)
    {
        auto screenObject = dynamic_cast<Screen*>(source);
        if (screenObject) {
            switch (static_cast<Screen::ScreenEvent>(event)) {
                case Screen::ScreenEvent::CREATED: {
                    if (m_window != nullptr) {
                        auto screenPtr = std::static_pointer_cast<Screen>(OM[screenObject->Id()]);
                        m_window->BindToScreen(screenPtr);
                    }
                } break;
                
                case Screen::ScreenEvent::ERROR: {
                    
                } break;
            }
        }
    }
}

// Protocol callbacks
extern "C"
{
    // CORE PROTOCOL EVENTS
    void wm_core_event_sync_callback(struct wm_core_sync_event* input)
    {
        auto object = Asgaard::OM[input->serial];
        if (!object) {
            return;
        }
        
        // publish to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SYNC);
    }

    void wm_core_event_error_callback(struct wm_core_error_event* input)
    {
        auto object = Asgaard::OM[input->object_id];
        if (!object) {
            // Global error, this must be handled on application level
            Asgaard::APP.ExternalEvent(Asgaard::Object::ObjectEvent::ERROR, input);
            return;
        }
        
        // publish error to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::ERROR, input);
    }
    
    void wm_core_event_object_callback(struct wm_core_object_event* input)
    {
        switch (input->type) {
            // Handle new server objects
            case object_type_screen: {
                Asgaard::APP.ExternalEvent(Asgaard::Object::ObjectEvent::CREATION, input);
            } break;
            
            // Handle client completion objects
            default: {
                auto object = Asgaard::OM[input->object_id];
                if (object == nullptr) {
                    // log
                    return;
                }
                object->ExternalEvent(Asgaard::Object::ObjectEvent::CREATION, input);
            } break;
        }
    }
    
    // SCREEN PROTOCOL EVENTS
    void wm_screen_event_screen_properties_callback(struct wm_screen_screen_properties_event* input)
    {
        auto object = Asgaard::OM[input->screen_id];
        if (!object) {
            // log
            return;
        }
        
        // publish to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SCREEN_PROPERTIES, input);
    }
    
    void wm_screen_event_mode_callback(struct wm_screen_mode_event* input)
    {
        auto object = Asgaard::OM[input->screen_id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SCREEN_MODE, input);
    }
    
    // SURFACE PROTOCOL EVENTS
    void wm_surface_event_format_callback(struct wm_surface_format_event* input)
    {
        auto object = Asgaard::OM[input->surface_id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SURFACE_FORMAT, input);
    }
    
    void wm_surface_event_frame_callback(struct wm_surface_frame_event* input)
    {
        auto object = Asgaard::OM[input->surface_id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SURFACE_FRAME, input);
    }
    
    void wm_surface_event_resize_callback(struct wm_surface_resize_event* input)
    {
        auto object = Asgaard::OM[input->surface_id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SURFACE_RESIZE, input);
    }

    void wm_surface_event_focus_callback(struct wm_surface_focus_event*)
    {

    }
    
    // BUFFER PROTOCOL EVENTS
    void wm_buffer_event_release_callback(struct wm_buffer_release_event* input)
    {
        auto object = Asgaard::OM[input->buffer_id];
        if (!object) {
            // log
            return;
        }
        
        object->ExternalEvent(Asgaard::Object::ObjectEvent::BUFFER_RELEASE, input);
    }

    // POINTER PROTOCOL EVENTS
    void wm_pointer_event_enter_callback(struct wm_pointer_enter_event* event)
    {

    }

    void wm_pointer_event_leave_callback(struct wm_pointer_leave_event* event)
    {

    }

    void wm_pointer_event_move_callback(struct wm_pointer_move_event* event)
    {

    }

    void wm_pointer_event_click_callback(struct wm_pointer_click_event* event)
    {

    }

    // KEYBOARD PROTOCOL EVENTS
    void wm_keyboard_event_key_callback(struct wm_keyboard_key_event* event)
    {

    }
}
