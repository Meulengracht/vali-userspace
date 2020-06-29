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
        {
            SetColor(0, 0, 0);
        }
        
        void Painter::SetColor(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
        {
            switch (m_canvas->Format()) {
                case PixelFormat::A8R8G8B8:
                    m_color = ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
                    break;
                case PixelFormat::A8B8G8R8:
                    m_color = ((unsigned int)a << 24) | ((unsigned int)b << 16) | ((unsigned int)g << 8) | r;
                    break;
                case PixelFormat::X8R8G8B8:
                    m_color = 0xFF000000 | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
                    break;
                case PixelFormat::R8G8B8A8:
                    m_color = ((unsigned int)r << 24) | ((unsigned int)g << 16) | ((unsigned int)b << 8) | a;
                    break;
                case PixelFormat::B8G8R8A8:
                    m_color = ((unsigned int)b << 24) | ((unsigned int)g << 16) | ((unsigned int)r << 8) | a;
                    break;
            }
        }
        
        void Painter::SetColor(unsigned char r, unsigned char g, unsigned char b)
        {
            SetColor(0xFF, r, g, b);
        }
        
        void Painter::SetFont(const std::shared_ptr<Font>& font)
        {
            m_font = font;
        }

        void Painter::RenderLine(int x1, int y1, int x2, int y2)
        {
            uint32_t* pointer = static_cast<uint32_t*>(m_canvas->Buffer());
            int       dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
            int       dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
            int       err = (dx > dy ? dx : -dy) / 2, e2;

            // Draw the line by brute force
            while (x1 < m_canvas->Width() && y1 < m_canvas->Height()) {
                pointer[(y1 * m_canvas->Width()) + x1] = m_color;

                if (x1 == x2 && y1 == y2) {
                    break;
                }
                
                e2 = err;
                if (e2 > -dx) { err -= dy; x1 += sx; }
                if (e2 <  dy) { err += dx; y1 += sy; }
            }
        }

        void Painter::RenderFillGradientV(const Rectangle& dimensions,
                    unsigned char r1, unsigned char g1, unsigned char b1,
                    unsigned char r2, unsigned char g2, unsigned char b2)
        {
            unsigned int originalColor = m_color;
            for (int y = 0; y < dimensions.Height(); y++) {
                float p = y / (float)(dimensions.Height() - 1);
                unsigned char r = (unsigned char)((1.0f - p) * r1 + p * r2 + 0.5);
                unsigned char g = (unsigned char)((1.0f - p) * g1 + p * g2 + 0.5);
                unsigned char b = (unsigned char)((1.0f - p) * b1 + p * b2 + 0.5);
                SetColor(r, g, b);
                RenderLine(dimensions.X(), y, dimensions.Width(), y);
            }
            m_color = originalColor;
        }
        
        void Painter::RenderFillGradientV(
                    unsigned char r1, unsigned char g1, unsigned char b1,
                    unsigned char r2, unsigned char g2, unsigned char b2)
        {
            unsigned int originalColor = m_color;
            for (int y = 0; y < m_canvas->Height(); y++) {
                float p = y / (float)(m_canvas->Height() - 1);
                unsigned char r = (unsigned char)((1.0f - p) * r1 + p * r2 + 0.5);
                unsigned char g = (unsigned char)((1.0f - p) * g1 + p * g2 + 0.5);
                unsigned char b = (unsigned char)((1.0f - p) * b1 + p * b2 + 0.5);
                SetColor(r, g, b);
                RenderLine(0, y, m_canvas->Width(), y);
            }
            m_color = originalColor;
        }

        void Painter::RenderFill(const Rectangle& dimensions)
        {
            for (int y = dimensions.Y(); y < dimensions.Height(); y++) {
                RenderLine(dimensions.X(), y, dimensions.Width(), y);
            }
        }
        
        void Painter::RenderFill()
        {
            for (int y = 0; y < m_canvas->Height(); y++) {
                RenderLine(0, y, m_canvas->Width(), y);
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
