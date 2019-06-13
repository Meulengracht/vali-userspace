/* MollenOS
 *
 * Copyright 2019, Philip Meulengracht
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
 * Wm Server Type Definitions & Structures
 * - This header describes the base server-structure, prototypes
 *   and functionality, refer to the individual things for descriptions
 */

#include "libwm_server.h"
#include <assert.h>
#include <threads.h>
#include <io.h>

static wm_server_message_handler_t wm_server_handler;
static int                         wm_message_fd;
static int                         wm_input_fd;
static thrd_t                      wm_message_thread;
static thrd_t                      wm_input_thread;
static int                         wm_initialized = 0;

// message thread
static int wm_message_handler(void* param)
{
    wm_message_fd = pipe();
    assert(wm_message_fd != -1);


}

// input thread
static int wm_input_handler(void* param)
{
    wm_input_fd = pipe();
    assert(wm_input_fd != -1);

#if defined(MOLLENOS)


#else
#error "Unsupported OS for libwm"
#endif
}

int wm_server_initialize(wm_server_message_handler_t handler)
{
    // store handler
    assert(wm_initialized == 0);
    wm_initialized    = 1;
    wm_server_handler = handler;



    // create threads
    thrd_create(&wm_message_thread, wm_message_handler, NULL);
    thrd_create(&wm_input_thread, wm_message_handler, NULL);
}

int wm_server_shutdown(void)
{

}
