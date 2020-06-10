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

#include <os/osdefs.h>
#include "resolver_base.hpp"
#include <thread>

class ResolverVali : public ResolverBase {
public:
    ResolverVali();
    ~ResolverVali();

public:
    bool HandleKeyCode(const Asgaard::KeyEvent&) override;
    void PrintCommandHeader() override;

protected:
    bool CommandResolver(const std::string&, const std::vector<std::string>&) override;
    bool ListDirectory(const std::vector<std::string>&) override;
    bool ChangeDirectory(const std::vector<std::string>&) override;

private:
    std::vector<std::string> GetDirectoryContents(const std::string& Path);
    bool ExecuteProgram(const std::string&, const std::vector<std::string>&);
    bool IsProgramPathValid(const std::string&);
    void UpdateWorkingDirectory();
    void WaitForProcess();
    
    void StdoutListener();
    void StderrListener();

private:
    std::string m_profile;
    std::string m_currentDirectory;

    int     m_stdout;
    int     m_stderr;
    UUId_t  m_application;

    std::thread m_stdoutThread;
    std::thread m_stderrThread;
};
