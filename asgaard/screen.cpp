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

#include "include/application.hpp"
#include "include/screen.hpp"
#include "include/window_manager.hpp"
#include "protocols/wm_core_protocol_client.h"
#include "protocols/wm_screen_protocol_client.h"

static Asgaard::Screen::ScreenTransform ConvertProtocolTransform(enum wm_screen_transform transform)
{
    switch (transform)
    {
        case no_transform:
            return Asgaard::Screen::NONE;
        case rotate_90:
            return Asgaard::Screen::ROTATE_90;
        case rotate_180:
            return Asgaard::Screen::ROTATE_180;
        case rotate_270:
            return Asgaard::Screen::ROTATE_270;
    }
}

static Asgaard::Screen::ScreenMode::ModeFlags ConvertProtocolMode(enum wm_screen_mode_attributes attributes)
{
    unsigned int screenMode = (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::NONE;
    if (attributes & mode_current) {
        screenMode |= (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_CURRENT;
    }
    if (attributes & mode_preferred) {
        screenMode |= (unsigned int)Asgaard::Screen::ScreenMode::ModeFlags::MODE_PREFERRED;
    }
    return (Asgaard::Screen::ScreenMode::ModeFlags)screenMode;
}

namespace Asgaard {
    Screen::Screen(uint32_t id) : Object(id)
    {
        // Register screen that is in setup phase
        APP.ExternalEvent(Application::SCREEN_REGISTERED);
                
        // Get properties of the screen
        wm_screen_get_properties(WM.GrachtClient(), id);
    }

    Screen::~Screen()
    {
        
    }
    
    int Screen::GetCurrentWidth() const
    {
        auto it = std::find_if(m_Modes.begin(), m_Modes.end(), 
            [](const std::unique_ptr<ScreenMode>& mode) { return mode->IsCurrent(); });
        if (it != m_Modes.end()) {
            return (*it)->ResolutionX();
        }
        return -1;
    }
    
    int Screen::GetCurrentHeight() const
    {
        auto it = std::find_if(m_Modes.begin(), m_Modes.end(), 
            [](const std::unique_ptr<ScreenMode>& mode) { return mode->IsCurrent(); });
        if (it != m_Modes.end()) {
            return (*it)->ResolutionY();
        }
        return -1;
    }
    
    int Screen::GetCurrentRefreshRate() const
    {
        auto it = std::find_if(m_Modes.begin(), m_Modes.end(), 
            [](const std::unique_ptr<ScreenMode>& mode) { return mode->IsCurrent(); });
        if (it != m_Modes.end()) {
            return (*it)->RefreshRate();
        }
        return -1;
    }
        
    void Screen::ExternalEvent(enum ObjectEvent event, void* data)
    {
        switch (event)
        {
            case Object::ERROR: {
                struct wm_core_error_event* error = 
                    (struct wm_core_error_event*)data;
                
                // In case of fatal events:
                APP.ExternalEvent(Application::ERROR);
            } break;
                
            case Object::SYNC: {
                // Now this chain of events are done and we are ready
                APP.ExternalEvent(Application::SCREEN_REGISTERED_COMPLETE);
            } break;
            
            case Object::SCREEN_PROPERTIES: {
                struct wm_screen_screen_properties_event* properties = 
                    (struct wm_screen_screen_properties_event*)data;
                
                // update stored information
                m_PositionX   = properties->x;
                m_PositionY   = properties->y;
                m_Scale       = properties->scale;
                m_Transform   = ConvertProtocolTransform(properties->transform);
                
                // continue this charade and ask for modes, end with a sync
                wm_screen_get_modes(WM.GrachtClient(), Id());
                wm_core_sync(WM.GrachtClient(), Id());
            } break;
            
            case Object::SCREEN_MODE: {
                struct wm_screen_mode_event* mode = 
                    (struct wm_screen_mode_event*)data;
                
                std::unique_ptr<ScreenMode> screenMode( 
                    new ScreenMode(ConvertProtocolMode(mode->flags), mode->resolution_x, 
                        mode->resolution_y, mode->refresh_rate));
                
                m_Modes.push_back(std::move(screenMode));
            } break;
            
            default:
                break;
        }
    }
}
