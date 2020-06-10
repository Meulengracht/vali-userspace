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
 
#include "../include/drawing/painter.hpp"
#include "../include/drawing/font.hpp"
#include "../include/memory_buffer.hpp"
#include "../include/rectangle.hpp"
#include <string> 

namespace {
    unsigned int AlphaBlend(unsigned int colorA, unsigned int colorB, unsigned int alpha)
    {
        unsigned int rb1 = ((0x100 - alpha) * (colorA & 0xFF00FF)) >> 8;
        unsigned int rb2 = (alpha * (colorB & 0xFF00FF)) >> 8;
        unsigned int g1  = ((0x100 - alpha) * (colorA & 0x00FF00)) >> 8;
        unsigned int g2  = (alpha * (colorB & 0x00FF00)) >> 8;
        return ((rb1 | rb2) & 0xFF00FF) + ((g1 | g2) & 0x00FF00) | 0xFF000000;
    }
}

namespace Asgaard {
    namespace Drawing {
        Painter::Painter(const std::shared_ptr<MemoryBuffer>& canvas)
            : m_canvas(canvas)
            , m_font(nullptr)
            , m_color(0xFF000000)
        {
            
        }
        
        Painter::~Painter()
        {
            
        }
        
        void Painter::SetColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
        {
            m_color = (a << 24) | (r << 16) | (g << 8) | b;
        }
        
        void Painter::SetColor(unsigned char r, unsigned char g, unsigned char b)
        {
            m_color = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
        
        void Painter::SetFont(const std::shared_ptr<Font>& font)
        {
            m_font = font;
        }
        
        void Painter::RenderFill(const Rectangle& dimensions)
        {
            // @todo
            std::size_t size = m_canvas->Width() * m_canvas->Height();
            uint32_t*   pointer = static_cast<uint32_t*>(m_canvas->Buffer());
            for (std::size_t i = 0; i < size; i++) {
                pointer[i] = m_color;
            }
        }
        
        void Painter::RenderFill()
        {
            std::size_t size = m_canvas->Width() * m_canvas->Height();
            uint32_t*   pointer = static_cast<uint32_t*>(m_canvas->Buffer());
            for (std::size_t i = 0; i < size; i++) {
                pointer[i] = m_color;
            }
        }
        
        void Painter::RenderCharacter(int x, int y, char character)
        {
            struct Font::CharInfo bitmap = { 0 };
            
            if (m_font == nullptr) {
                return;
            }
        
            if (m_font->GetCharacterBitmap(character, bitmap)) {
                uint32_t* pointer = static_cast<uint32_t*>(m_canvas->Buffer(
                    x + bitmap.indentX, y + bitmap.indentY));
                uint8_t*  source  = bitmap.bitmap;
                for (int row = 0; row < bitmap.height; row++) {
                    for (int column = 0; column < bitmap.width; column++) { // @todo might need to be reverse
                        uint8_t alpha = source[column];
                        if (alpha == 255) {
                            pointer[column] = m_color;
                        }
                        else {
                            // pointer[Column] = m_FgColor; if CACHED_BITMAP
                            pointer[column] = AlphaBlend(pointer[column], m_color, alpha);
                        }
                    }
                    
                    pointer = reinterpret_cast<uint32_t*>(reinterpret_cast<std::uintptr_t>(pointer) + m_canvas->Stride());
                    source  += bitmap.pitch;
                }
            }
        }
        
        void Painter::RenderText(const Rectangle& dims, const std::string& text)
        {
            struct Font::CharInfo bitmap   = { 0 };
            int                   currentX = dims.X();
            int                   currentY = dims.Y();
            
            if (m_font == nullptr) {
                return;
            }
            
            for (int i = 0; i < text.length(); i++) {
                unsigned int character = text[i];
                
                if (m_font->GetCharacterBitmap(character, bitmap)) {
                    if ((currentX + bitmap.indentX + bitmap.advance) >= dims.Width()) {
                        currentY += 0;
                        currentX = dims.X();
                    }
                    
                    uint32_t* pointer = static_cast<uint32_t*>(m_canvas->Buffer(
                        currentX + bitmap.indentX, currentY + bitmap.indentY));
                    uint8_t*  source  = bitmap.bitmap;
                    
                    for (int row = 0; row < bitmap.height; row++) {
                        for (int column = 0; column < bitmap.width; column++) { // @todo might need to be reverse
                            uint8_t alpha = source[column];
                            if (alpha == 255) {
                                pointer[column] = m_color;
                            }
                            else {
                                // pointer[Column] = m_FgColor; if CACHED_BITMAP
                                pointer[column] = AlphaBlend(pointer[column], m_color, alpha);
                            }
                        }
                        
                        pointer = reinterpret_cast<uint32_t*>(reinterpret_cast<std::uintptr_t>(pointer) + m_canvas->Stride());
                        source  += bitmap.pitch;
                    }
                }
                
                currentX += bitmap.indentX + bitmap.advance;
            }
        }
        
        void Painter::RenderText(int x, int y, const std::string& text)
        {
            Rectangle dimensions(x, y, m_canvas->Width(), m_canvas->Height());
            RenderText(dimensions, text);
        }
    }
}
