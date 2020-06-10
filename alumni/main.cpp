/* MollenOS
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
 * Terminal Implementation (Alumnious)
 * - The terminal emulator implementation for Vali. Built on manual rendering and
 *   using freetype as the font renderer.
 */

#include "targets/resolver_vali.hpp"
#include "terminal.hpp"
#include <asgaard/drawing/font_manager.hpp>

int main(int argc, char **argv)
{
    std::string                             fontPath = "$sys/fonts/DejaVuSansMono.ttf";
    std::shared_ptr<Asgaard::Drawing::Font> font     = Asgaard::Drawing::FM.CreateFont(fontPath, 12);
    std::shared_ptr<ResolverVali>           resolver = std::shared_ptr<ResolverVali>(new ResolverVali());
    Asgaard::Rectangle                      initialSize(0, 0, 450, 300);
    
    Asgaard::APP.CreateWindow<Terminal>(initialSize, font, resolver);
    Asgaard::APP.Initialize();
    return Asgaard::APP.Execute();
}
