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

#include "include/object_manager.hpp"
#include "include/memory_pool.hpp"
#include "include/memory_buffer.hpp"
#include "include/rectangle.hpp"
#include "include/window_decoration.hpp"

#include "include/widgets/icon.hpp"
#include "include/widgets/label.hpp"

namespace Asgaard {
    WindowDecoration::WindowDecoration(uint32_t id, std::shared_ptr<Screen> screen, uint32_t parentId, const Rectangle& dimensions)
        : Surface(id, screen, parentId, dimensions)
    {
        
    }
    
    WindowDecoration::~WindowDecoration()
    {
        
    }
    
    void WindowDecoration::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case ObjectEvent::CREATION: {
                // left corner
                Rectangle appIconSize(8, 8, 32, 32);
                
                // right corner
                Rectangle closeIconSize(m_Dimensions.Width() - (8 + 32), 8, 32, 32);
                
                // next to app
                Rectangle labelSize(8 + 8 + 32, 8, 128, 32);
                auto      poolSize = (m_Dimensions.Width() * m_Dimensions.Height() * 4);
                
                // create resources
                m_memory = MemoryPool::Create(this, poolSize);
                
                m_appTitle = OM.CreateClientObject<Asgaard::Widgets::Label>(m_Screen, Id(), labelSize);
                m_appTitle->Subscribe(this);
                
                m_appIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_Screen, Id(), appIconSize);
                m_appIcon->Subscribe(this);
                
                m_closeIcon = OM.CreateClientObject<Asgaard::Widgets::Icon>(m_Screen, Id(), closeIconSize);
                m_closeIcon->Subscribe(this);
                
            } break;
            
            default:
                break;
        }
        
        // Run the base class events as well
        Surface::ExternalEvent(event, data);
    }
}
