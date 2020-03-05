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
#pragma once

#include <gracht/client.h>
#include "object_manager.hpp"
#include "window_base.hpp"

namespace Asgaard {
    class Application final {
    public:
        enum ApplicationEvent {
            ERROR,
            SCREEN_REGISTERED,
            SCREEN_REGISTERED_COMPLETE
        };
        
    public:
        Application();
        ~Application();
        
        int Initialize();
        int Execute();
        int Shutdown();
        
    public:
        template<class WC, typename... Params>
        void CreateWindow(Params... parameters) {
            if (!std::is_base_of<WindowBase, WC>::value) {
                return;
            }
            m_Window = OM.CreateClientObject<WC, Params...>(parameters...);
        }
        
        void ExternalEvent(enum ApplicationEvent, void* = 0);
        gracht_client_t* GrachtClient() { return m_Client; }

    private:
        gracht_client_t*            m_Client;
        std::shared_ptr<WindowBase> m_Window;
    };
    
    extern Application APP;
}
