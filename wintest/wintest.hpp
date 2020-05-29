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

#include <asgaard/application.hpp>
#include <asgaard/window_base.hpp>
#include <asgaard/window_memory.hpp>
#include <asgaard/window_buffer.hpp>

class TestWindow : public Asgaard::WindowBase {
public:
    TestWindow(uint32_t id, const Asgaard::Rectangle& dimensions)
        : WindowBase(id, dimensions)
    {
        
    }
    
    ~TestWindow()
    {
        
    }
    
protected:
    void OnCreated(Asgaard::Object* createdObject) override
    {
        if (createdObject->Id() == Id()) {
            // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
            auto screenSize = m_Screen->GetCurrentWidth() * m_Screen->GetCurrentHeight() * 4;
            m_Memory = CreateMemory(screenSize);
        }
        else if (createdObject->Id() == m_Memory->Id()) {
            // Create initial buffer the size of this surface
            m_Buffer = CreateBuffer(m_Memory, 0, m_Dimensions.Width(),
                m_Dimensions.Height(), Asgaard::PixelFormat::A8R8G8B8);
        }
        else if (createdObject->Id() == m_Buffer->Id()) {
            // Now all resources are created
            SetBuffer(m_Buffer);
            Redraw();
            MarkDamaged(m_Dimensions);
            ApplyChanges();
        }
    }
    
    void OnRefreshed(Asgaard::WindowBuffer* buffer) override
    {
        if (buffer->Id() == m_Buffer->Id()) {
            Redraw();
            MarkDamaged(m_Dimensions);
            ApplyChanges();
        }
    }
    
    void OnFrame() override
    {
        
    }
    
    void OnResized(enum Asgaard::Surface::SurfaceEdges edges, int width, int height) override
    {
        
    }

    void Teardown() override
    {
        
    }

private:
    void Redraw()
    {
        // Update window content data
    }
    
private:
    std::shared_ptr<Asgaard::WindowMemory> m_Memory;
    std::shared_ptr<Asgaard::WindowBuffer> m_Buffer;
};
