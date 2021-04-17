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

#include "../include/theming/theme.hpp"
#include <libzip/ZipFile.h>
#include <libzip/streams/memstream.h>
#include <libzip/methods/Bzip2Method.h>

namespace Asgaard {
namespace Theming {
    // declare the loader class which we want to keep private
    class ThemeLoader {
    public:
        ThemeLoader(const std::string& path) {
            m_zipHandle = ZipFile::Open(path);
        }
        ~ThemeLoader() {
            
        }

        ZipArchiveEntry::Ptr GetEntry(std::string& path) {
            if (!m_zipHandle) {
                return nullptr;
            }

            auto entry = m_zipHandle->GetEntry(path);
            if (!entry) {
                return nullptr;
            }

    #ifdef ASGAARD_THEME_PASSWORD
            if (entry->IsPasswordProtected()) {
                entry->SetPassword(std::string(ASGAARD_THEME_PASSWORD));
            }
    #else
            if (entry->IsPasswordProtected()) {
                return nullptr;
            }
    #endif

            return entry;
        }

    private:
        ZipArchive::Ptr m_zipHandle;
    };
}
}

using namespace Asgaard;
using namespace Asgaard::Theming;


Theme::Theme(const std::string& themePack)
    : m_loader(nullptr)
{

}

Theme::~Theme()
{

}

void Theme::InitializeTheme()
{
    m_loader = std::make_unique<ThemeLoader>("$themes/default.pak");
    
    // push paths
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_CURSOR), "cursor16.png")); // 20, 24, 32

    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_CLOSE), "close16.png")); // 20, 24, 32
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_MAXIMIZE), "max16.png")); // 20, 24, 32
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_MINIMIZE), "min16.png")); // 20, 24, 32
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_APP_DEFAULT), "app16.png")); // 20, 24, 32

    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_SEARCH), "search-grey24.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_BACKGROUND), "bg.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_TERMINAL), "terminal64.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_EDITOR), "notepad64.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_GAME), "game64.png"));

    // load colors
    m_colors.insert(std::make_pair(static_cast<int>(Colors::DECORATION_FILL), Drawing::Color(0x0, 0x0C, 0x35, 0x33)));
    m_colors.insert(std::make_pair(static_cast<int>(Colors::DECORATION_TEXT), Drawing::Color(0xFF, 0xFF, 0xFF)));
}

Drawing::Image Theme::GetImage(enum Elements element)
{
    if (!m_loader) {
        InitializeTheme();
    }

    auto entry = m_loader->GetEntry(m_paths[static_cast<int>(element)]);
    if (!entry) {
        return Drawing::Image();
    }

    auto dataStream = entry->GetDecompressionStream();
    if (!dataStream) {
        return Drawing::Image();   
    }

    return Drawing::Image(*dataStream, entry->GetSize());
}

Drawing::Color Theme::GetColor(enum Colors color)
{
    if (!m_loader) {
        InitializeTheme();
    }

    return m_colors[static_cast<int>(color)];
}
