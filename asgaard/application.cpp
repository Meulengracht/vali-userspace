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
 * ValiOS - Application Framework (Asgard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */

#include "include/application.hpp"
#include "include/eventqueue.hpp"

namespace Asgard {
    Application::Application(const Rectangle& SurfaceArea, const std::string& Name)
        : m_EventQueue(nullptr), m_Invalidated(false), m_WmEventThread(nullptr),
          m_KeyEventThread(nullptr)
    {
    }

    Application::~Application()
    {

    }

    int Application::Execute()
    {
        if (m_Running) {
            return -1;
        }

        m_Running           = true;
        m_EventQueue        = new EventQueue();
        //m_WmEventThread     = new std::thread(std::bind(&Application::WindowEventWorker, this));
        m_KeyEventThread    = new std::thread(std::bind(&Application::KeyEventWorker, this));
        
        int ReturnCode = Entry();
        m_Running = false;
        
        return ReturnCode;
    }

    int Application::Entry()
    {
        return 0;
    }

    void Application::WindowEvent()
    {

    }

    void Application::KeyEvent(SystemKey_t* Key)
    {

    }

    void Application::Invalidate()
    {

    }

    void Application::WindowEventWorker()
    {
        while (m_Running) {
            WindowEvent();
        }
    }
    
    void Application::KeyEventWorker()
    {
        SystemKey_t Key;
        while (m_Running) {
            if (ReadSystemKey(&Key) == OsSuccess) {
                KeyEvent(&Key);
            }
        }
    }
}