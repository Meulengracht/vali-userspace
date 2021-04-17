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
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/key_event.hpp>
#include <asgaard/drawing/painter.hpp>
#include <os/keycodes.h>

class TestWindow : public Asgaard::WindowBase {
public:
    TestWindow(uint32_t id, const std::shared_ptr<Asgaard::Screen>& screen, const Asgaard::Rectangle& dimensions)
        : WindowBase(id, screen, dimensions) { }
    
    ~TestWindow() { }
    
protected:
    void OnCreated() override
    {
        // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
        auto screenSize = m_screen->GetCurrentWidth() * m_screen->GetCurrentHeight() * 4;
        m_memory = Asgaard::MemoryPool::Create(this, screenSize);

        // Create initial buffer the size of this surface
        m_buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), Asgaard::PixelFormat::A8B8G8R8, Asgaard::MemoryBuffer::Flags::NONE);
        
        // Create the window decoration
        EnableDecoration(true);

        // Now all resources are created
        SetDropShadow(Asgaard::Rectangle(-10, -10, 20, 30));
        SetBuffer(m_buffer);
        Redraw();
    }
    
    void OnRefreshed(Asgaard::MemoryBuffer* buffer) override
    {
        //if (buffer->Id() == m_buffer->Id()) {
        //    Redraw();
        //}
    }
    
    void OnFrame() override
    {
        
    }
    
    void OnResized(enum Asgaard::Surface::SurfaceEdges edges, int width, int height) override
    {
        
    }

    void OnKeyEvent(const Asgaard::KeyEvent& keyEvent) override
    {
        if (keyEvent.KeyCode() == VK_R && !keyEvent.Pressed()) {
            Redraw();
        }
    }

private:
    void Redraw()
    {
        Asgaard::Drawing::Painter paint(m_buffer);
        
        //paint.SetColor(0xFA, 0xEF, 0xDD);
        paint.SetFillColor(0xF0, 0xF0, 0xF0);
        paint.RenderFill();
        
        MarkDamaged(Dimensions());
        ApplyChanges();
    }
    
private:
    std::shared_ptr<Asgaard::MemoryPool>   m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_buffer;
};
