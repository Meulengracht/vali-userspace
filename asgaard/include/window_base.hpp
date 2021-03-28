/**
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
    class MemoryPool;
    class MemoryBuffer;
    class WindowDecoration;
    class WindowEdge;

    namespace Drawing {
        class Image;
    }
    
    class WindowBase : public Surface {
    public:
        enum class FullscreenMode : int {
            EXIT,
            NORMAL,
            FULL
        };

        enum class PriorityLevel : int {
            BOTTOM,
            DEFAULT,
            TOP
        };
        
    public:
        WindowBase(uint32_t, const std::shared_ptr<Screen>&, const Rectangle&);
        ~WindowBase();

        void SetTitle(const std::string& title);
        void SetIconImage(const std::shared_ptr<Drawing::Image>& image);

        void SetResizable(bool resizeable);
        void EnableDecoration(bool enable);

        void RequestPriorityLevel(enum PriorityLevel);
        void RequestFullscreenMode(enum FullscreenMode);
        
    // Window events that can/should be reacted on.
    protected:
        virtual void OnMinimize() { }
        virtual void OnMaximize() { }
        virtual void OnCreated(Object*) = 0;
        virtual void OnRefreshed(MemoryBuffer*) = 0;
        virtual void Teardown() = 0;

    // Window functions that can be called to configure this window 
    protected:
        void InitiateResize(const std::shared_ptr<Pointer>&, enum SurfaceEdges);
        void InitiateMove(const std::shared_ptr<Pointer>&);

        // Protected function, allow override
        void Notification(Publisher*, int = 0, void* = 0) override;

    private:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;

    private:
        std::vector<enum PixelFormat>     m_supportedFormats;
        std::shared_ptr<WindowDecoration> m_decoration;
        std::shared_ptr<WindowEdge>       m_edge;
    };
}
