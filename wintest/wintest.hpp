/* ValiOS
 *
 * Copyright 2020, Philip Meulengracht
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
 * WinTest - Application Window Test
 *  - Test application used to stress test/conformance test the WM protocol
 */
#pragma once

#include <asgaard/window_base.hpp>

class TestWindow : public WindowBase {
public:
    TestWindow(uint32_t id, const Rectangle& dimensions, const std::string& title)
        : WindowBase(id, dimensions, title)
    {
        
    }
    
    ~TestWindow()
    {
        
    }
    
protected:
    void Setup() override
    {
        
    }
    
    void ExternalEvent(enum ObjectEvent event, void* data = 0) override
    {
        switch (event)
        {
            case BUFFER_RELEASE: {
                Redraw();
            } break;
            
            default:
                break;
        }
        
        // Let the base class also have a shot at handling the event
        WindowBase::ExternalEvent(event, data);
    }
    
    void Teardown() override
    {
        
    }

private:
    void Redraw()
    {
        // Update window content data
        
        
        // invalidate content
        
    }
};
