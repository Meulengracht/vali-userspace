/* MollenOS
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
 * Vioarr - Vali Compositor
 * - Implements the default system compositor for Vali. It utilizies the gracht library
 *   for communication between compositor clients and the server. The server renders
 *   using Mesa3D with either the soft-renderer or llvmpipe render for improved performance.
 */

#include <errno.h>
#include <libwm_os.h>
#include <libwm_server.h>
#include <stdio.h>

#include "protocols/hid_events_protocol_server.h"
#include "protocols/wm_core_protocol_server.h"
#include "protocols/wm_screen_protocol_server.h"
#include "protocols/wm_memory_protocol_server.h"
#include "protocols/wm_memory_pool_protocol_server.h"
#include "protocols/wm_buffer_protocol_server.h"
#include "protocols/wm_surface_protocol_server.h"

#include "engine/vioarr_engine.h"

int server_initialize(void)
{
    wm_server_configuration_t configuration;
    int                       status;
    
    wm_os_get_server_client_address(&configuration.server_address, &configuration.server_address_length);
    wm_os_get_server_packet_address(&configuration.dgram_address, &configuration.dgram_address_length);
    status = wm_server_initialize(&configuration);
    if (status) {
        printf("error initializing server library %i", errno);
    }
    return status;
}

int server_run(void)
{
    return wm_server_main_loop();    
}

/*******************************************
 * Entry Point
 *******************************************/
int main(int argc, char **argv)
{
    int status = server_initialize();
    if (status) {
        return status;
    }
    
    status = vioarr_engine_initialize();
    if (status) {
        return status;
    }
    
    wm_server_register_protocol(&hid_events_protocol);
    wm_server_register_protocol(&wm_core_protocol);
    wm_server_register_protocol(&wm_screen_protocol);
    wm_server_register_protocol(&wm_memory_protocol);
    wm_server_register_protocol(&wm_memory_pool_protocol);
    wm_server_register_protocol(&wm_buffer_protocol);
    wm_server_register_protocol(&wm_surface_protocol);
    return server_run();
}
