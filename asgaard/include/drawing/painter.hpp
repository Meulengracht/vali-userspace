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

namespace Asgaard {
    class MemoryBuffer;
    class Rectangle;
    
    namespace Drawing {
        class Font;
        
        class Painter {
            public:
                Painter(std::shared_ptr<MemoryBuffer> canvas);
                ~Painter();
                
                void SetColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
                void SetColor(unsigned char r, unsigned char g, unsigned char b);
                
                void RenderFill(const Rectangle& dimensions);
                void RenderFill();
                
                void SetFont(std::shared_ptr<Font>& font);
                
                void RenderCharacter(int x, int y, char character);
                void RenderText(const Rectangle& dimensions, const std::string& text);
                void RenderText(int x, int y, const std::string& text);
                
            private:
                std::shared_ptr<MemoryBuffer> m_canvas;
                std::shared_ptr<Font>         m_font;
                unsigned int                  m_color;
        };
    }
}
