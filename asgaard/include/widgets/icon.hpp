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
#include "../surface.hpp"
#include <string>

namespace Asgaard {
    class MemoryPool;
    class MemoryBuffer;
    
    namespace Widgets {
        class Icon : public Surface {
        public:
            enum class IconState {
                NORMAL = 0,
                HOVERING,
                ACTIVE,
                DISABLED,
    
                COUNT
            };
            
            enum class Notification : int {
                CREATED = static_cast<int>(Object::Notification::CUSTOM_START),
                ERROR
            };
        public:
            Icon(uint32_t id, const std::shared_ptr<Screen>& screen, uint32_t parentId, const Rectangle&);
            ~Icon();
            
            bool LoadIcon(const std::string& path);
            void SetState(IconState state);
    
        public:
            void ExternalEvent(enum ObjectEvent event, void* data = 0) final;
    
        private:
            void Notification(Publisher*, int = 0, void* = 0) override;
    
        private:
            std::shared_ptr<Asgaard::MemoryPool>   m_memory;
            std::shared_ptr<Asgaard::MemoryBuffer> m_buffers[static_cast<int>(IconState::COUNT)];
    
            int         m_originalWidth;
            int         m_originalHeight;
            std::string m_originalPath;
        };
    }
}
