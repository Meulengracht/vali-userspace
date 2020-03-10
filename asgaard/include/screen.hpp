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

#include <cstdint>
#include <list>
#include <memory>
#include "object.hpp"

namespace Asgaard {
    class Screen : public Object {
    public:
        enum ScreenTransform {
            NONE,
            ROTATE_90,
            ROTATE_180,
            ROTATE_270,
        };
        
        enum class ScreenEvent {
            ERROR,
            CREATED
        };
        
    public:
        class ScreenMode {
        public:
            enum class ModeFlags : unsigned int {
                NONE = 0,
                MODE_CURRENT = 1,
                MODE_PREFERRED = 2
            };
            
        public:
            ScreenMode(enum ModeFlags flags, int resolutionX, int resolutionY, int refreshRate)
                : m_Flags(flags), m_ResolutionX(resolutionX), m_ResolutionY(resolutionY), m_RefreshRate(refreshRate)
                { }
                
            bool IsCurrent() const { return ((unsigned int)m_Flags & (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_CURRENT) != 0; }
            int  ResolutionX() const { return m_ResolutionX; }
            int  ResolutionY() const { return m_ResolutionY; }
            int  RefreshRate() const { return m_RefreshRate; }
            
        private:
            ModeFlags m_Flags;
            int       m_ResolutionX;
            int       m_ResolutionY;
            int       m_RefreshRate;
        };
        
    public:
        Screen(uint32_t id);
        ~Screen();
        
        int GetCurrentWidth() const;
        int GetCurrentHeight() const;
        int GetCurrentRefreshRate() const;
        
    public:
        void ExternalEvent(enum ObjectEvent event, void* data = 0) override;
        
    private:
        std::list<std::unique_ptr<ScreenMode>> m_Modes;
        int             m_PositionX;
        int             m_PositionY;
        int             m_Scale;
        ScreenTransform m_Transform;
    };
}
