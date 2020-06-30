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
 * MollenOS Terminal Implementation (Alumnious)
 * - The terminal emulator implementation for Vali. Built on manual rendering and
 *   using freetype as the font renderer.
 */

#include <cassert>
#include <cctype>
#include <cmath>

#include <os/keycodes.h>

#include <asgaard/object_manager.hpp>
#include <asgaard/key_event.hpp>
#include "terminal.hpp"
#include "terminal_interpreter.hpp"
#include "targets/resolver_base.hpp"

const int PRINTBUFFER_SIZE = 4096;

Terminal::TerminalLine::TerminalLine(const std::shared_ptr<Asgaard::Drawing::Font>& font, int row, int capacity)
    : m_font(font)
    , m_dimensions(0, (row * font->GetFontHeight()) + 2, capacity, font->GetFontHeight())
    , m_row(row)
    , m_capacity(capacity - 6)
{
    Reset();
}

void Terminal::TerminalLine::Reset()
{
    m_showCursor  = false;
    m_textLength  = 0;
    m_inputOffset = 0;
    m_cursor      = 0;
    m_dirty       = true;
    m_text.clear();
}

bool Terminal::TerminalLine::AddCharacter(int character)
{
    struct Asgaard::Drawing::Font::CharInfo bitmap = { 0 };
    
    char buf = (char)character & 0xFF;
    
    if (!m_font->GetCharacterBitmap(buf, bitmap)) {
        // ignore character
        return true;
    }
    
    if (AddInput(character)) {
        m_inputOffset++;
        return true;
    }
    return false;
}

bool Terminal::TerminalLine::AddInput(int character)
{
    struct Asgaard::Drawing::Font::CharInfo bitmap = { 0 };
    
    char buf = (char)character & 0xFF;
    
    if (!m_font->GetCharacterBitmap(buf, bitmap)) {
        // ignore character
        return true;
    }

    // Handle \r \t \n?
    if ((m_textLength + (bitmap.width + bitmap.indentX)) < m_capacity) {
        if (m_cursor == m_text.length()) {
            m_text.push_back(character);
        }
        else {
            m_text.insert(m_cursor, &buf, 1);
        }
        m_dirty      = true;
        m_textLength += bitmap.width + bitmap.indentX;
        m_cursor++;
        return true;
    }
    return false;
}

bool Terminal::TerminalLine::RemoveInput()
{
    int modifiedCursor = m_cursor - m_inputOffset;
    if (modifiedCursor != 0) {
        m_text.erase(m_cursor - 1, 1);
        m_dirty      = true;
        m_textLength = m_font->GetTextMetrics(m_text).Width();
        m_cursor--;
        return true;
    }
    return false;
}

void Terminal::TerminalLine::Update(std::shared_ptr<Asgaard::MemoryBuffer>& buffer)
{
    if (m_dirty) {
        Asgaard::Drawing::Painter paint(buffer);
        
        paint.SetFillColor(0, 0, 0);
        paint.RenderFill(m_dimensions);
        
        paint.SetOutlineColor(0xFF, 0xFF, 0xFF);
        paint.SetFont(m_font);
        paint.RenderText(m_dimensions.X(), m_dimensions.Y(), m_text);
        if (m_showCursor) {
            //paint.SetOutlineColor(255, 255, 255);
            paint.RenderCharacter(m_textLength, m_dimensions.Y(), '_');
        }
        m_dirty = false;
    }
}

void Terminal::TerminalLine::SetText(const std::string& text)
{
    Reset();
    m_text   = text;
    m_cursor = text.length();
}

std::string Terminal::TerminalLine::GetInput()
{
    if ((int)m_text.length() > m_inputOffset) {
        return m_text.substr(m_inputOffset);
    }
    return "";
}

void Terminal::TerminalLine::HideCursor()
{
    m_showCursor = false;
    m_dirty      = true;
}

void Terminal::TerminalLine::ShowCursor()
{
    m_showCursor = true;
    m_dirty      = true;
}

Terminal::Terminal(uint32_t id, const Asgaard::Rectangle& dimensions, const std::shared_ptr<Asgaard::Drawing::Font>& font,
    const std::shared_ptr<ResolverBase>& resolver)
    : WindowBase(id, dimensions)
    , m_font(font)
    , m_resolver(resolver)
    , m_rows((dimensions.Height() / font->GetFontHeight()) - 1)
    , m_historyIndex(0)
    , m_lineIndex(0)
{
    m_printBuffer = (char*)std::malloc(PRINTBUFFER_SIZE);
    for (int i = 0; i < m_rows; i++) {
        m_lines.push_back(std::make_unique<TerminalLine>(font, i, dimensions.Width()));
    }
}

Terminal::~Terminal()
{
    std::free(m_printBuffer);
    m_history.clear();
    m_lines.clear();
}

void Terminal::AddInput(int character)
{
    if (!m_lines[m_lineIndex]->AddInput(character)) {
        // uh todo, we should skip to next line
        return;
    }
    m_lines[m_lineIndex]->Update(m_buffer);
}

void Terminal::RemoveInput()
{
    if (!m_lines[m_lineIndex]->RemoveInput()) {
        // uh todo, we should skip to prev line
        return;
    }
    m_lines[m_lineIndex]->Update(m_buffer);
}

std::string Terminal::ClearInput(bool newline)
{
    std::string input = m_lines[m_lineIndex]->GetInput();
    if (newline) {
        FinishCurrentLine();
    }
    else {
        m_lines[m_lineIndex]->Reset();
        m_lines[m_lineIndex]->Update(m_buffer);
    }
    return input;
}

void Terminal::FinishCurrentLine()
{
    // Only add to history if not an empty line
    if (m_lines[m_lineIndex]->GetText().length() > 0) {
        m_history.push_back(m_lines[m_lineIndex]->GetText());
        m_historyIndex = m_history.size();
    }

    // Are we at the end?
    if (m_lineIndex == m_rows - 1) {
        ScrollToLine(true);
    }
    else {
        m_lines[m_lineIndex]->HideCursor();
        m_lines[m_lineIndex]->Update(m_buffer);
        m_lineIndex++;
        m_lines[m_lineIndex]->ShowCursor();
        m_lines[m_lineIndex]->Update(m_buffer);
    }
}

void Terminal::ScrollToLine(bool clearInput)
{
    int historyStart = m_historyIndex - m_lineIndex;
    for (int i = 0; i < m_rows; i++) {
        if (i == m_lineIndex && !clearInput) {
            break;
        }

        m_lines[i]->Reset();
        if (historyStart < m_historyIndex) {
            m_lines[i]->SetText(m_history[historyStart++]);
        }
        m_lines[i]->Update(m_buffer);
    }
}

void Terminal::HistoryNext()
{
    // History must be longer than the number of rows - 1
    if (m_history.size()        >= (size_t)m_rows &&
        (size_t)m_historyIndex  < m_history.size()) {
        m_historyIndex++;
        ScrollToLine(false);
    }
}

void Terminal::HistoryPrevious()
{
    // History must be longer than the number of rows - 1
    if (m_history.size()    >= (size_t)m_rows &&
        m_historyIndex      >= m_rows) {
        m_historyIndex--;
        ScrollToLine(false);
    }
}

void Terminal::MoveCursorLeft()
{

}

void Terminal::MoveCursorRight()
{

}

void Terminal::Print(const char* format, ...)
{
    std::unique_lock<std::mutex> lockedSection(m_printLock);
    va_list arguments;

    va_start(arguments, format);
    vsnprintf(m_printBuffer, PRINTBUFFER_SIZE, format, arguments);
    va_end(arguments);

    for (size_t i = 0; i < PRINTBUFFER_SIZE && m_printBuffer[i]; i++) {
        if (m_printBuffer[i] == '\n') {
            FinishCurrentLine();
        }
        else {
            if (!m_lines[m_lineIndex]->AddCharacter(m_printBuffer[i])) {
                FinishCurrentLine();
                i--;
            }
        }
    }
    m_lines[m_lineIndex]->Update(m_buffer);
    Invalidate();
}

void Terminal::Invalidate()
{
    MarkDamaged(Dimensions());
    ApplyChanges();
}

void Terminal::OnCreated(Asgaard::Object* createdObject)
{
    if (createdObject->Id() == Id()) {
        // Don't hardcode 4 bytes per pixel, this is only because we assume a format of ARGB32
        auto screenSize = m_screen->GetCurrentWidth() * m_screen->GetCurrentHeight() * 4;
        m_memory = Asgaard::MemoryPool::Create(this, screenSize);
        
        // we couldn't do this in constructor as the OM had not registered us
        auto terminal = std::dynamic_pointer_cast<Terminal>(Asgaard::OM[Id()]);
        m_resolver->SetTerminal(terminal);
    }
    else if (createdObject->Id() == m_memory->Id()) {
        // Create initial buffer the size of this surface
        m_buffer = Asgaard::MemoryBuffer::Create(this, m_memory, 0, Dimensions().Width(),
            Dimensions().Height(), Asgaard::PixelFormat::A8R8G8B8);
    }
    else if (createdObject->Id() == m_buffer->Id()) {
        // Now all resources are created
        SetBuffer(m_buffer);
        m_resolver->PrintCommandHeader();
        Invalidate();
    }
}

void Terminal::OnRefreshed(Asgaard::MemoryBuffer*)
{
    
}

void Terminal::Teardown()
{
    
}

void Terminal::OnKeyEvent(const Asgaard::KeyEvent& key)
{
    // Don't respond to released events
    if (!key.Pressed()) {
        return;
    }

    if (key.KeyCode() == VK_BACK) {
        RemoveInput();
        Invalidate();
    }
    else if (key.KeyCode() == VK_ENTER) {
        std::string input = ClearInput(true);
        if (!m_resolver->Interpret(input)) {
            if (m_resolver->GetClosestMatch().length() != 0) {
                Print("Command did not exist, did you mean %s?\n", m_resolver->GetClosestMatch().c_str());
            }
        }
        m_resolver->PrintCommandHeader();
    }
    else if (key.KeyCode() == VK_UP) {
        HistoryPrevious();
        Invalidate();
    }
    else if (key.KeyCode() == VK_DOWN) {
        HistoryNext();
        Invalidate();
    }
    else if (key.KeyCode() == VK_LEFT) {
        MoveCursorLeft();
        Invalidate();
    }
    else if (key.KeyCode() == VK_RIGHT) {
        MoveCursorRight();
        Invalidate();
    }
    else if (key.KeyAscii() != 0) {
        AddInput(key.KeyAscii());
        Invalidate();
    }
}
