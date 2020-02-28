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

#include <cstdint>
#include <gracht/client.h>
#include <map>
#include <string>

namespace Asgaard {
    class WindowManager {
    public:
        using WmEventData = void*;
        enum WmEvent {
            WM_SYNC,
            WM_ERROR
        };
        
    public:
        WindowManager();
        ~WindowManager();
        
        int  Initialize();
        void Shutdown();
        int  Run();

    public:
        void ExternalEvent(enum WmEvent, WmEventData);
        gracht_client_t* GrachtClient() const { return m_Client; }
        
    public:
        static void *operator new   (size_t)   = delete;
        static void *operator new[] (size_t)   = delete;
        static void  operator delete(void*)    = delete;
        static void  operator delete[] (void*) = delete;
        
    private:
        gracht_client_t* m_Client;
        bool             m_Running;
    };
    
    // Global instance of the window manager
    extern WindowManager WM;
}
