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
#include <asgaard/drawing/painter.hpp>

class TestWindow : public Asgaard::WindowBase {
public:
    TestWindow(uint32_t id, const Asgaard::Rectangle& dimensions)
        : WindowBase(id, dimensions)
    {
        r = 32;
        g = 32;
        b = 32;
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
            m_Memory = Asgaard::MemoryPool::Create(this, screenSize);
        }
        else if (createdObject->Id() == m_Memory->Id()) {
            // Create initial buffer the size of this surface
            m_Buffer = Asgaard::MemoryBuffer::Create(this, m_Memory, 0, Dimensions().Width(),
                Dimensions().Height(), Asgaard::PixelFormat::A8R8G8B8);
        }
        else if (createdObject->Id() == m_Buffer->Id()) {
            // Now all resources are created
            SetBuffer(m_Buffer);
            Redraw();
            MarkDamaged(Dimensions());
            ApplyChanges();
        }
    }
    
    void OnRefreshed(Asgaard::MemoryBuffer* buffer) override
    {
        if (buffer->Id() == m_Buffer->Id()) {
            Redraw();
            MarkDamaged(Dimensions());
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
    void PixelFill(unsigned int color) {
        std::size_t size = Dimensions().Width() * Dimensions().Height();
        uint32_t*   pointer = static_cast<uint32_t*>(m_Buffer->Buffer());
        for (std::size_t i = 0; i < size; i++) {
            pointer[i] = color;
        }
    }

    void Redraw()
    {
        Drawing::Painter paint(m_Buffer);
        
        paint.SetColor(r, g, b);
        paint.RenderFill();
        
        r++; g++; b++;
    }
    
private:
    std::shared_ptr<Asgaard::MemoryPool> m_Memory;
    std::shared_ptr<Asgaard::MemoryBuffer> m_Buffer;

    uint8_t r,g,b;
};
