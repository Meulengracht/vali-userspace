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

    int Application::Initialize()
    {
        gracht_client_configuration_t Configuration;
        int                           Status;
        
        Configuration.type = gracht_client_stream_based;
        gracht_os_get_server_client_address(&Configuration.address, &Configuration.address_length);
        
        Status = gracht_client_create(&Configuration, &m_Client);
        if (Status) {
            // log
            return -1;
        }
        
        gracht_client_register_protocol(m_Client, &wm_core_protocol);
        gracht_client_register_protocol(m_Client, &wm_screen_protocol);
        gracht_client_register_protocol(m_Client, &wm_surface_protocol);
        gracht_client_register_protocol(m_Client, &wm_buffer_protocol);
        
        // kick off a chain reaction by asking for all objects
        return wm_core_get_objects(m_Client);
    }
    
    int Application::Execute()
    {
        char* messageBuffer = new char[GRACHT_MAX_MESSAGE_SIZE];
        
        while (m_Running) {
            if (!gracht_client_wait_message(m_Client, messageBuffer)) {
                gracht_client_process_message(m_Client, messageBuffer);
            }
        }
        
        delete[] messageBuffer;
        return 0;
    }

    int Application::Shutdown()
    {
        if (m_Client != nullptr) {
            gracht_client_shutdown(m_Client);
        }
        return 0;
    }
    
    void Application::ExternalEvent(enum ApplicationEvent event, void* data)
    {
        switch (event)
        {
            case ERROR: {
                
            } break;
            case SCREEN_REGISTERED: {
                
            } break;
            case SCREEN_REGISTERED_COMPLETE: {
                // If all screens are setup, then we can create the window
                if (m_Window != nullptr) {
                    m_Window->ExternalEvent(Object::ObjectEvent::CREATION);
                }
            } break;
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
            Asgaard::APP.ExternalEvent(Asgaard::Application::ERROR, input);
            return;
        }
        
        // publish error to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::ERROR, input);
    }
    
    void wm_core_event_object_callback(struct wm_core_object_event* input)
    {
        switch (input->type)
        {
            // Handle new server objects
            case object_type_screen: {
                auto screen = Asgaard::OM.CreateServerObject<Asgaard::Screen>(input->object_id);
            } break;
            
            // Handle client completion objects
            default: {
                auto object = Asgaard::OM[input->object_id];
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
        
        // verify object is a screen object
        auto screen = dynamic_cast<Asgaard::Screen*>(object.get());
        if (!screen) {
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
        
        // verify object is a screen object
        auto screen = dynamic_cast<Asgaard::Screen*>(object.get());
        if (!screen) {
            // log
            return;
        }
        
        // publish to object
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
        
        // verify object is a window object
        auto window = dynamic_cast<Asgaard::WindowBase*>(object.get());
        if (!window) {
            // log
            return;
        }
        
        // publish to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SURFACE_FORMAT, input);
    }
    
    void wm_surface_event_resize_callback(struct wm_surface_resize_event* input)
    {
        auto object = Asgaard::OM[input->surface_id];
        if (!object) {
            // log
            return;
        }
        
        // verify object is a window object
        auto window = dynamic_cast<Asgaard::WindowBase*>(object.get());
        if (!window) {
            // log
            return;
        }
        
        // publish to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::SURFACE_RESIZE, input);
    }
    
    // BUFFER PROTOCOL EVENTS
    void wm_buffer_event_release_callback(struct wm_buffer_release_event* input)
    {
        auto object = Asgaard::OM[input->buffer_id];
        if (!object) {
            // log
            return;
        }
        
        // verify object is a window object
        auto buffer = dynamic_cast<Asgaard::WindowBase*>(object.get());
        if (!buffer) {
            // log
            return;
        }
        
        // publish to object
        object->ExternalEvent(Asgaard::Object::ObjectEvent::BUFFER_RELEASE, input);
    }
}
