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

#include "object_manager.hpp"
#include "rectangle.hpp"
#include "object.hpp"

namespace Asgaard {
    class Screen;
    class MemoryBuffer;
    class KeyEvent;
    
    class Surface : public Object {
    public:
        enum class SurfaceEdges : unsigned int {
            NONE   = 0x0,
            TOP    = 0x1,
            BOTTOM = 0x2,
            LEFT   = 0x4,
            RIGHT  = 0x8
        };
        
        enum class SurfaceEvent : int {
            CREATED
        };
    public:
        Surface(uint32_t, const std::shared_ptr<Screen>&, uint32_t, const Rectangle&);
        Surface(uint32_t, const std::shared_ptr<Screen>&, const Rectangle&);
        Surface(uint32_t, const Rectangle&);
        ~Surface();
        
        void BindToScreen(const std::shared_ptr<Screen>&);
        
        void SetBuffer(const std::shared_ptr<MemoryBuffer>&);
        void MarkDamaged(const Rectangle&);
        void MarkInputRegion(const Rectangle&);
        void ApplyChanges();
        
        void RequestFrame();
        
        const Rectangle& Dimensions() const { return m_dimensions; }
        
    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;
        
    protected:
        virtual void OnResized(enum SurfaceEdges, int width, int height);
        virtual void OnFocus(bool);
        virtual void OnFrame();
        virtual void OnMouseMove();
        virtual void OnMousePressed();
        virtual void OnMouseReleased();
        virtual void OnKeyEvent(const KeyEvent&);
        
    protected:
        Rectangle               m_dimensions;
        std::shared_ptr<Object> m_parent;
        std::shared_ptr<Screen> m_screen;
    };
}
