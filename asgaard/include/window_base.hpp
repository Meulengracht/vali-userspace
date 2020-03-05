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

#include <string>
#include <vector>
#include "pixel_format.hpp"
#include "rectangle.hpp"
#include "surface.hpp"
#include "screen.hpp"

namespace Asgaard {
    class WindowMemory;
    class WindowBuffer;
    
    class WindowBase : public Surface {
    public:
        WindowBase(uint32_t, const Rectangle&);
        ~WindowBase();
        
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;

    // Window events that can/should be reacted on.
    protected:
        virtual void OnCreated(Object*) = 0;
        virtual void OnRefreshed(WindowBuffer*) = 0;
        virtual void Teardown() = 0;

    // Window functions that can be called to configure this window 
    protected:
        std::shared_ptr<WindowMemory>  CreateMemory(std::size_t size);
        std::shared_ptr<WindowBuffer>  CreateBuffer(std::shared_ptr<WindowMemory>,
            int memoryOffset, int width, int height, enum PixelFormat format);
        
        void InititateResize(enum SurfaceEdges);
        void InitiateMove();

    private:
        void Notification(Publisher*, int = 0, void* = 0) override;

    private:
        std::vector<enum PixelFormat> m_SupportedFormats;
        bool                          m_Invalidated;
    };
}
