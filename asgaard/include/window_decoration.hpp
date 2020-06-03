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

#include <memory>
#include "surface.hpp"

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    
    namespace Widgets {
        class Icon;
        class Label;
    }
    
    class WindowDecoration : public Surface {
    public:
        WindowDecoration(uint32_t id, std::shared_ptr<Screen> screen, uint32_t parentId, const Rectangle&);
        ~WindowDecoration();

    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) final;

    private:
        // consists of multiple resources;
        // a buffer with the header color
        // a buffer with the application title
        // a buffer with the application icon
        // a buffer with the close icon
        std::shared_ptr<Asgaard::MemoryPool>     m_memory;
        std::shared_ptr<Asgaard::MemoryBuffer>   m_buffer;
        std::shared_ptr<Asgaard::Widgets::Label> m_appTitle;
        std::shared_ptr<Asgaard::Widgets::Icon>  m_appIcon;
        std::shared_ptr<Asgaard::Widgets::Icon>  m_closeIcon;
    };
}
