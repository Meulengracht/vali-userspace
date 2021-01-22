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
    class Application final : public Object {
    public:
        Application();
        ~Application();
        
        /**
         * Initialize
         * Initializes the asgaard application environment, and must be invoked before any other
         * call made in this class. 
         * @throw ApplicationException
         */
        void Initialize();

        /**
         * AddEventDescriptor
         * Adds a c-type io descriptor like a socket, pipe or file to listen for events on. 
         * DescriptorEvent callback is then invoked in the window-class given when an event occurs.
         * @param iod The C-io descriptor that should be listening for events on
         * @param events The events to listen for, defined in <ioset.h>
         * @throw ApplicationException
         */
        void AddEventDescriptor(int iod, unsigned int events);

        /**
         * Execute
         * Starts the applications main loop. The application will run untill shutdown has been requested
         * or any fault happens (caught exception).
         * @return Status code of execution
         */
        int Execute();
        
    public:
        template<class WC, typename... Params>
        void CreateWindow(Params... parameters) {
            if (!std::is_base_of<WindowBase, WC>::value) {
                return;
            }
            m_window = OM.CreateClientObject<WC, Params...>(parameters...);
        }
        
        gracht_client_t* GrachtClient() { return m_client; }

    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;
        void Notification(Publisher*, int = 0, void* = 0) override;

    private:
        gracht_client_t*            m_client;
        std::shared_ptr<WindowBase> m_window;
        int                         m_ioset;
        bool                        m_initialized;
    };
    
    extern Application APP;
}
