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
#include "include/window_manager.hpp"

namespace Asgaard {
    Application APP;
    
    Application::Application()
        : m_Window(nullptr)
    {
        
    }

    Application::~Application()
    {

    }

    int Application::Execute()
    {
        // Initialize global classes
        WM.Initialize();
        
        // The main loop is in the window manager which might be a bit wierd
        // but the window manager runs the window manager protocol client
        return WM.Run();
    }
    
    void Application::ExternalEvent(enum ApplicationEvent event)
    {
        switch (event)
        {
            case ERROR: {
                
            } break;
            case SCREEN_REGISTERED: {
                
            } break;
            case SCREEN_REGISTERED_COMPLETE: {
                
                
                // If all screens are setup, then we can create the window
            } break;
        }
    }
}
