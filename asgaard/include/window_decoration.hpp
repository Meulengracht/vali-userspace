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
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */
#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "subsurface.hpp"

#define DECORATION_FILL_COLOR Drawing::Color(0x0, 0x0C, 0x35, 0x33)
#define DECORATION_TEXT_COLOR Drawing::Color(0x28, 0x57, 0x5A)
#define DECORATION_TEXT_SIZE  12

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    
    namespace Drawing {
        class Font;
    }
    
    namespace Widgets {
        class Icon;
        class Label;
    }
    
    class WindowDecoration : public SubSurface {
    public:
        enum class Notification : int {
            CREATED = static_cast<int>(Object::Notification::CUSTOM_START),
            ERROR
        };
    public:
        WindowDecoration(uint32_t id, 
            const std::shared_ptr<Screen>& screen, 
            uint32_t parentId, 
            const Rectangle&);

        WindowDecoration(uint32_t id, 
            const std::shared_ptr<Screen>& screen, 
            uint32_t parentId, 
            const Rectangle&,
            const std::shared_ptr<Drawing::Font>& font);

        ~WindowDecoration();
        
        void SetTitle(const std::string& title);
        void SetIcon(const std::string& iconPath);
        void RequestRedraw();

    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;
    
    private:
        void Redraw();
        void RedrawReady();
        void Notification(Publisher*, int = 0, void* = 0) override;
        void CheckCreation();

    private:
        // consists of multiple resources;
        // a buffer with the header color
        // a buffer with the application title
        // a buffer with the application icon
        // a buffer with the close icon
        std::shared_ptr<MemoryPool>     m_memory;
        std::shared_ptr<MemoryBuffer>   m_buffer;
        std::shared_ptr<Drawing::Font>  m_appFont;
        std::shared_ptr<Widgets::Label> m_appTitle;
        std::shared_ptr<Widgets::Icon>  m_appIcon;
        std::shared_ptr<Widgets::Icon>  m_minIcon;
        std::shared_ptr<Widgets::Icon>  m_maxIcon;
        std::shared_ptr<Widgets::Icon>  m_closeIcon;

        bool              m_redraw;
        std::atomic<bool> m_redrawReady;
    };
}
