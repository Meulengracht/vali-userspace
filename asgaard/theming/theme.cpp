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


using namespace Asgaard;
using namespace Asgaard::Theming;

// declare the loader class which we want to keep private
class ThemeLoader {
public:
    ThemeLoader(const std::string& path) {
        m_zipHandle = ZipFile::Open(outfile);
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
#endif

        auto dataStream = entry->GetDecompressionStream();
        if (!dataStream) {
            return nullptr;
        }

        return entry;
    }

private:
    ZipArchive::Ptr m_zipHandle;
};

Theme::Theme()
    : m_loader(std::make_unique<ThemeLoader>("$themes/default.pak"))
{

}

Theme::~Theme()
{

}

Drawing::Image Theme::GetImage(enum Elements element)
{

}

Drawing::Color Theme::GetColor(enum Colors color)
{

}
