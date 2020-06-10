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
 
#include "include/key_event.hpp"

namespace Asgaard {
    KeyEvent::KeyEvent(struct hid_events_key_event_args*)
        : m_modifiers(0)
        , m_keyUnicode(0)
        , m_keyAscii(0)
        , m_keyCode(0)
    {
        
    }
    
    char KeyEvent::KeyAscii() const
    {
        return 0;
    }
    
    unsigned int KeyEvent::KeyUnicode() const
    {
        return 0;
    }
    
    unsigned char KeyEvent::KeyCode() const
    {
        return 0;
    }
    
    bool KeyEvent::Pressed() const
    {
        return false;
    }
}