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

#include <os/mollenos.h>
#include <os/process.h>
#include <os/keycodes.h>
#include <io.h>
#include "../terminal_interpreter.hpp"
#include "../terminal.hpp"
#include "resolver_vali.hpp"

namespace {
    static bool EndsWith(const std::string& String, const std::string& Suffix)
    {
        return String.size() >= Suffix.size() && 0 == String.compare(String.size() - Suffix.size(), Suffix.size(), Suffix);
    }
}

ResolverVali::ResolverVali()
    : ResolverBase()
    , m_profile("Philip")
    , m_currentDirectory("n/a")
    , m_stdout(-1)
    , m_stderr(-1)
    , m_application(UUID_INVALID)
    , m_stdoutThread(std::bind(&ResolverVali::StdoutListener, this)), 
      m_stderrThread(std::bind(&ResolverVali::StderrListener, this))
{
    UpdateWorkingDirectory();
}

ResolverVali::~ResolverVali()
{
    
}

void ResolverVali::UpdateWorkingDirectory()
{
    char* CurrentPath = (char*)std::malloc(_MAXPATH);
    std::memset(CurrentPath, 0, _MAXPATH);
    if (GetWorkingDirectory(CurrentPath, _MAXPATH) == OsSuccess) {
        m_currentDirectory = std::string(CurrentPath);
    }
    else {
        m_currentDirectory = "n/a";
    }
    std::free(CurrentPath);
}

bool ResolverVali::HandleKeyCode(const Asgaard::KeyEvent&)
{
    return true;
}

void ResolverVali::PrintCommandHeader()
{
    // Dont print the command header if an application is running
    if (m_application == UUID_INVALID) {
        m_terminal->Print("[ %s | %s ]\n", m_profile.c_str(), m_currentDirectory.c_str());
        m_terminal->Print("$ ");
    }
    m_terminal->Invalidate();
}

void ResolverVali::WaitForProcess()
{
    int ExitCode = 0;
    ProcessJoin(m_application, 0, &ExitCode);
    m_terminal->Print("process exitted with code %i\n", ExitCode);
    m_application = UUID_INVALID;
}

bool ResolverVali::ExecuteProgram(const std::string& Program, const std::vector<std::string>& Arguments)
{
    ProcessConfiguration_t configuration;
    std::string            line = "";

    // Set arguments
    if (Arguments.size() != 0) {
        for (int i = 0; i < Arguments.size(); i++) {
            if (i != 0) {
                line += " ";
            }
            line += Arguments[i];
        }
    }
    
    ProcessConfigurationInitialize(&configuration);

    // Set inheritation
    configuration.InheritFlags = PROCESS_INHERIT_STDOUT | PROCESS_INHERIT_STDIN | PROCESS_INHERIT_STDERR;
    configuration.StdOutHandle = m_stdout;
    configuration.StdInHandle  = STDIN_FILENO;
    configuration.StdErrHandle = m_stderr;
    
    ProcessSpawnEx(Program.c_str(), line.c_str(), &configuration, &m_application);
    if (m_application != UUID_INVALID) {
        WaitForProcess();
        return true;
    }
    return false;
}

std::vector<std::string> ResolverVali::GetDirectoryContents(const std::string& Path)
{
    struct DIR*              dir;
    struct DIRENT            dp;
    std::vector<std::string> Entries;

    if (opendir(Path.c_str(), 0, &dir) != -1) {
        while (readdir(dir, &dp) != -1) {
            Entries.push_back(std::string(&dp.d_name[0]));
        }
        closedir(dir);
    }
    return Entries;
}

bool ResolverVali::IsProgramPathValid(const std::string& Path)
{
    OsFileDescriptor_t Stats = { 0 };

    if (GetFileInformationFromPath(Path.c_str(), &Stats) == OsSuccess) {
        if (!(Stats.Flags & FILE_FLAG_DIRECTORY) && (Stats.Permissions & FILE_PERMISSION_EXECUTE)) {
            return true;
        }
        m_terminal->Print("%s: not an executable file 0x%x\n", Path.c_str(), Stats.Permissions);
    }
    return false;
}

bool ResolverVali::CommandResolver(const std::string& Command, const std::vector<std::string>& Arguments)
{
    std::string ProgramName = Command;
    std::string ProgramPath = "";

    if (!EndsWith(ProgramName, ".app")) {
        ProgramName += ".app";
    }

    // Guess the path of requested application, right now only working
    // directory and $bin is supported. Should we support apps that don't have .app? ...
    ProgramPath = "$bin/" + ProgramName;
    if (!IsProgramPathValid(ProgramPath)) {
        char TempBuffer[_MAXPATH] = { 0 };
        if (GetWorkingDirectory(&TempBuffer[0], _MAXPATH) == OsSuccess) {
            std::string CwdPath(&TempBuffer[0]);
            ProgramPath = CwdPath + "/" + ProgramName;
            if (!IsProgramPathValid(ProgramPath)) {
                m_terminal->Print("%s: not a valid command\n", Command.c_str());
                return false;
            }
        }
        else {
            return false;
        }
    }
    return ExecuteProgram(ProgramPath, Arguments);
}

bool ResolverVali::ListDirectory(const std::vector<std::string>& Arguments)
{
    std::string Path = m_currentDirectory;
    std::string Line = "";
    if (Arguments.size() != 0) {
        Path = Arguments[0];
    }

    auto DirectoryEntries = GetDirectoryContents(m_currentDirectory);
    for (auto Entry : DirectoryEntries) {
        Line += Entry + " ";
    }

    m_terminal->Print("%s\n", Line.c_str());
    return true;
}

bool ResolverVali::ChangeDirectory(const std::vector<std::string>& Arguments)
{
    if (Arguments.size() != 0) {
        std::string Path = Arguments[0];
        if (SetWorkingDirectory(Path.c_str()) == OsSuccess) {
            UpdateWorkingDirectory();
            return true;
        }
        m_terminal->Print("cd: invalid argument %s\n", Path.c_str());
        return false;
    }
    m_terminal->Print("cd: no argument given\n");
    return false;
}

void ResolverVali::StdoutListener()
{
    char ReadBuffer[256];

    m_stdout = pipe(0x1000, 0);
    if (m_stdout == -1) {
        m_terminal->Print("FAILED TO CREATE STDOUT\n");
        m_terminal->Invalidate();
        return;
    }

    while (Alive()) {
        std::memset(&ReadBuffer[0], 0, sizeof(ReadBuffer));
        read(m_stdout, &ReadBuffer[0], sizeof(ReadBuffer));
        m_terminal->Print(&ReadBuffer[0]);
        m_terminal->Invalidate();
    }
}

void ResolverVali::StderrListener()
{
    char ReadBuffer[256];

    m_stderr = pipe(0x1000, 0);
    if (m_stderr == -1) {
        m_terminal->Print("FAILED TO CREATE STDERR\n");
        m_terminal->Invalidate();
        return;
    }

    while (Alive()) {
        std::memset(&ReadBuffer[0], 0, sizeof(ReadBuffer));
        read(m_stderr, &ReadBuffer[0], sizeof(ReadBuffer));
        m_terminal->Print(&ReadBuffer[0]);
        m_terminal->Invalidate();
    }
}
