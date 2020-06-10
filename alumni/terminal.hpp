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
#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include <list>

#include <asgaard/application.hpp>
#include <asgaard/window_base.hpp>
#include <asgaard/memory_pool.hpp>
#include <asgaard/memory_buffer.hpp>
#include <asgaard/drawing/font.hpp>
#include <asgaard/drawing/painter.hpp>

class ResolverBase;

class Terminal : public Asgaard::WindowBase {
private:
    class TerminalLine {
    public:
        TerminalLine(const std::shared_ptr<Asgaard::Drawing::Font>&, int row, int capacity);
        ~TerminalLine() = default;

        void Reset();
        bool AddInput(int character);
        bool RemoveInput();
        bool AddCharacter(int character);
        void Update(std::shared_ptr<Asgaard::MemoryBuffer>&);

        void SetText(const std::string& text);
        void HideCursor();
        void ShowCursor();

        std::string        GetInput();
        const std::string& GetText() const { return m_text; }

    private:
        std::shared_ptr<Asgaard::Drawing::Font> m_font;
        
        Asgaard::Rectangle m_dimensions;
        std::string        m_text;
        int                m_textLength;
        int                m_row;
        int                m_capacity;
        int                m_cursor;
        int                m_inputOffset;
        bool               m_showCursor;
        bool               m_dirty;
    };

public:
    Terminal(uint32_t id, const Asgaard::Rectangle&, const std::shared_ptr<Asgaard::Drawing::Font>&,
        const std::shared_ptr<ResolverBase>&);
    ~Terminal();

    void Print(const char* format, ...);
    void Invalidate();

    // Input manipulation
    std::string ClearInput(bool newline);
    void RemoveInput();
    void AddInput(int character);
    
    // History manipulation
    void HistoryPrevious();
    void HistoryNext();

    // Cursor manipulation
    void MoveCursorLeft();
    void MoveCursorRight();
    
protected:
    void OnCreated(Asgaard::Object*) override;
    void Teardown() override;
    void OnRefreshed(Asgaard::MemoryBuffer*) override;
    void OnKeyEvent(const Asgaard::KeyEvent&) override;
    
private:
    void FinishCurrentLine();
    void ScrollToLine(bool clearInput);

private:
    std::shared_ptr<Asgaard::MemoryPool>    m_memory;
    std::shared_ptr<Asgaard::MemoryBuffer>  m_buffer;
    std::shared_ptr<Asgaard::Drawing::Font> m_font;
    std::shared_ptr<ResolverBase>           m_resolver;
    
    int                                        m_rows;
    std::vector<std::string>                   m_history;
    int                                        m_historyIndex;
    std::vector<std::unique_ptr<TerminalLine>> m_lines;
    int                                        m_lineIndex;
    char*                                      m_printBuffer;
    std::mutex                                 m_printLock;
};
