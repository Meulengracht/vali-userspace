/* MollenOS
 *
 * Copyright 2011 - 2018, Philip Meulengracht
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
 * MollenOS - Windowing Test Suite for Userspace
 *  - Runs a variety of userspace tests against the window manager to verify
 *    the stability and integrity of the operating system.
 */

#include "wintest.hpp"

int main(int argc, char **argv)
{
    Asgaard::Rectangle initialSize(0, 0, 640, 320);
    Asgaard::APP.Initialize();
    
    Asgaard::APP.GetScreen()->CreateWindow<TestWindow>(initialSize);
    return Asgaard::APP.Execute();
}
