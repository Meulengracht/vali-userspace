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

#include "rectangle.hpp"
#include "object.hpp"

namespace Asgaard {
    class Screen;
    class WindowBuffer;
    
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
        Surface(uint32_t, std::shared_ptr<Screen>, uint32_t, const Rectangle&);
        Surface(uint32_t, std::shared_ptr<Screen>, const Rectangle&);
        Surface(uint32_t, const Rectangle&);
        ~Surface();
        
        void BindToScreen(std::shared_ptr<Screen>);
        
        std::shared_ptr<Surface> CreateSubSurface(const Rectangle&);
        void SetBuffer(std::shared_ptr<WindowBuffer>);
        void ApplyChanges();
        
    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;
        
    protected:
        virtual void OnResized(enum SurfaceEdges, int width, int height);
        virtual void OnFocus(bool);
        virtual void OnMouseMove();
        virtual void OnMouseDown();
        virtual void OnMouseUp();
        virtual void OnKeyDown();
        virtual void OnKeyUp();
        
    protected:
        Rectangle               m_Dimensions;
        std::shared_ptr<Object> m_Parent;
        std::shared_ptr<Screen> m_Screen;
    };
}
