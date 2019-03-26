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
 * MollenOS - Vioarr Window Compositor System (Display Interface Implementation)
 *  - The window compositor system and general window manager for
 *    MollenOS. This display implementation is of the default display where
 *    we use osmesa as the backend combined with the native framebuffer
 */
#pragma once

#include <ddk/contracts/video.h>
#include <GL/osmesa.h>
#include <GL/gl.h>

#include "utils/log_manager.hpp"
#include "../../display.hpp"
#include <cstdlib>

class CDisplayOsMesa : public CDisplay {
public:
    
    // Constructor
    // Initializes the os-mesa context and prepares a backbuffer for the
    // display (vbe/vesa) framebuffer
    CDisplayOsMesa() {
        int OSMesaAttributes[100], n = 0;

        sLog.Info("Creating the opengl context");
        OSMesaAttributes[n++] = OSMESA_FORMAT;
        OSMesaAttributes[n++] = OSMESA_BGRA;
        OSMesaAttributes[n++] = OSMESA_DEPTH_BITS;
        OSMesaAttributes[n++] = 24;
        OSMesaAttributes[n++] = OSMESA_STENCIL_BITS;
        OSMesaAttributes[n++] = 8;
        OSMesaAttributes[n++] = OSMESA_ACCUM_BITS;
        OSMesaAttributes[n++] = 0;
        OSMesaAttributes[n++] = OSMESA_PROFILE;
        OSMesaAttributes[n++] = OSMESA_CORE_PROFILE;
        OSMesaAttributes[n++] = OSMESA_CONTEXT_MAJOR_VERSION;
        OSMesaAttributes[n++] = 3;
        OSMesaAttributes[n++] = OSMESA_CONTEXT_MINOR_VERSION;
        OSMesaAttributes[n++] = 3;
        OSMesaAttributes[n++] = 0;
        _Context              = OSMesaCreateContextAttribs(&OSMesaAttributes[0], NULL);
    }

    // Destructor
    // Cleans up the opengl context and frees the resources allocated.
    ~CDisplayOsMesa() {
        if (_Context != nullptr) {
            OSMesaDestroyContext(_Context);
        }
    }

    // Initialize
    // Initializes the display to the given parameters, use -1 for maximum size
    bool Initialize() {
        if (!IsValid()) {
            return false;
        }
        
        sLog.Info("Creating access to the headless display framebuffer");
        int Width       = 640;
        int Height      = 480;
        _BackbufferSize = Width * Height * 4 * sizeof(GLubyte);
        _Backbuffer     = std::aligned_alloc(32, _BackbufferSize);
        if (_Backbuffer == nullptr) {
            return false;
        }
        SetDimensions(0, 0, Width, Height);
        return OSMesaMakeCurrent(_Context, _Backbuffer, GL_UNSIGNED_BYTE, Width, Height);
    }
    
    bool IsValid() {
        return (_Context != nullptr);
    }
    
    bool Present() {
        return true;
    }
    
private:
    OSMesaContext _Context;
    void*         _Backbuffer;
    size_t        _BackbufferSize;
};
