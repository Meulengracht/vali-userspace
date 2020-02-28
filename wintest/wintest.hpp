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
    TestWindow(uint32_t id, const Rectangle& dimensions)
        : WindowBase(id, dimensions)
    {
        
    }
    
    ~TestWindow()
    {
        
    }
    
protected:
    void OnCreated(Object* createdObject) override
    {
        if (createdObject == this)
        {
            // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
            auto screenSize = m_Screen->Dimensions()->Width() * m_Screen->Dimensions()->Height() * 4;
            m_Memory = CreateMemory(screenSize);
        }
        else if (createdObject == m_Memory.get())
        {
            // Create initial buffer the size of this surface
            m_Buffer = CreateBuffer(m_Memory, 0, m_Dimensions.Width(),
                m_Dimensions.Height(), PixelFormat::A8R8G8B8);
        }
        else if (createdObject == m_Buffer.get())
        {
            // Now all resources are created
            SetBuffer(m_Buffer);
            SetTitle("Window Test Application");
            
            // Draw initial scene
            Redraw();
            
            // Then update all the changes we've made
            ApplyChanges();
        }
    }
    
    void OnRefreshed(WindowBuffer* buffer) override
    {
        // The window manager has released the buffer
        Redraw();
    }
    
    void OnResized() override
    {
        
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
    
private:
    std::shared_ptr<WindowMemory> m_Memory;
    std::shared_ptr<WindowBuffer> m_Buffer;
};
