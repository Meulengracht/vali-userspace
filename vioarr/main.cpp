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
 * Vioarr Window Compositor System
 *  - The window compositor system and general window manager for Vali.
 */

#include <os/mollenos.h>
#include <os/types/ui.h>
#include <ddk/services/session.h>
#include <ddk/services/window.h>

#include "engine/elements/window.hpp"
#include "engine/scene.hpp"
#include "utils/log_manager.hpp"
#include "vioarr.hpp"

extern void InputHandler();

// Communication handles we have to register to get system events
static UUId_t RcChannelHandle  = UUID_INVALID;
static UUId_t InputEventHandle = UUID_INVALID;
static UUId_t ServiceHandle    = UUID_INVALID;

bool ConvertSurfaceFormatToGLFormat(UISurfaceFormat_t Format, GLenum &FormatResult, 
    GLenum &InternalFormatResult, int &BytesPerPixel)
{
    bool Success = true;
    switch (Format) {
        case SurfaceRGBA: {
            FormatResult         = GL_RGBA;
            InternalFormatResult = GL_RGBA8;
            BytesPerPixel        = 4;
        } break;

        default: {
            Success = false;
        } break;
    }
    return Success;
}

long HandleCreateWindowRequest(MRemoteCallAddress_t *Process, UIWindowParameters_t *Parameters, 
    UUId_t BufferHandle)
{
    DmaBuffer_t* Buffer;
    CWindow*     Window;
    long         ElementId;
    GLenum       Format, InternalFormat;
    int          BytesPerPixel;

    // Does the process already own a window? Then don't create
    // one as we don't want a process to flood us
    ElementId = sEngine.GetTopElementByOwner(Process->Process);
    if (ElementId != -1) {
        sLog.Warning("Tried to create a second process window");
        return ElementId;
    }

    // Now we must validate the parameters of the request, so validate
    // the sizes, surface-type and buffer-size
    if (Parameters->Surface.Dimensions.w < 0 || Parameters->Surface.Dimensions.h < 0
        || !ConvertSurfaceFormatToGLFormat(Parameters->Surface.Format, Format, InternalFormat, BytesPerPixel)) {
        sLog.Warning("Invalid window parameters");
        return -1;
    }

    // Inherit the buffer
    if (BufferHandle == UUID_INVALID) {
        sLog.Warning("Invalid window buffer handle");
        return -1;
    }
    Buffer = CreateBuffer(BufferHandle, 0);

    // Validate the size of the buffer before acquiring it
    if (GetBufferSize(Buffer) < (Parameters->Surface.Dimensions.w * Parameters->Surface.Dimensions.h * BytesPerPixel)) {
        sLog.Warning("Invalid window buffer size");
        return -1;
    }
    ZeroBuffer(Buffer);

    // Everything is ok, create the window, set elements up and queue up for render
    Window = new CWindow(sEngine.GetContext());
    Window->SetOwner(Process->Process);
    Window->SetInputPipe(Parameters->InputPipeHandle);
    Window->SetWmEventPipe(Parameters->WmEventPipeHandle);
    Window->SetWidth(Parameters->Surface.Dimensions.w);
    Window->SetHeight(Parameters->Surface.Dimensions.h);
    Window->SetPosition(250, 200, 0);

    Window->SetStreamingBufferFormat(Format, InternalFormat);
    Window->SetStreamingBufferDimensions(Parameters->Surface.Dimensions.w, Parameters->Surface.Dimensions.h);
    Window->SetStreamingBuffer(Buffer);
    Window->SetStreaming(true);
    Window->Invalidate();

    // Add the window and redraw
    sEngine.AddElementToCurrentScene(Window);
    return Window->GetId();
}

void MessageHandler()
{
    MRemoteCall_t Message;
    char*         ArgumentBuffer;
    bool          IsRunning = true;
    SetCurrentThreadName("vioarr_message");

    // Listen for messages
    ArgumentBuffer = (char*)::malloc(IPC_MAX_MESSAGELENGTH);
    while (IsRunning) {
        // Keep processing messages untill no more
        if (RPCListen(RcChannelHandle, &Message, ArgumentBuffer) == OsSuccess) {
            if (Message.Function == __WINDOWMANAGER_CREATE) {
                UIWindowParameters_t* Parameters;
                UUId_t                BufferHandle;
                long                  ElementId;

                // Get arguments
                Parameters   = (UIWindowParameters_t*)RPCGetPointerArgument(&Message, 0);
                BufferHandle = (UUId_t)Message.Arguments[1].Data.Value;
                
                ElementId = HandleCreateWindowRequest(&Message.From, Parameters, BufferHandle);
                RPCRespond(&Message.From, &ElementId, sizeof(long));
            }
            if (Message.Function == __WINDOWMANAGER_DESTROY) {
                long ElementId = (long)Message.Arguments[0].Data.Value;
                sEngine.RemoveElement(ElementId);
            }
            if (Message.Function == __WINDOWMANAGER_SWAPBUFFER) {
                long ElementId = (long)Message.Arguments[0].Data.Value;
                sEngine.InvalidateElement(ElementId);
            }
            if (Message.Function == __WINDOWMANAGER_QUERY) {
                
            }
            sVioarr.UpdateNotify();
        }
    }
}

// Spawn the message handler for compositor
void VioarrCompositor::SpawnInputHandlers() {
    m_MessageThread = new std::thread(MessageHandler);
    m_InputThread   = new std::thread(InputHandler);
}

int main(int argc, char **argv) {
    OsStatus_t Status;
    int        ExitCode;
    
    // Create all the neccessary pipes
    CreatePipe(PIPE_STRUCTURED, &RcChannelHandle);
    CreatePipe(PIPE_RAW, &InputEventHandle);
    
    Status = ListenForSystemEvents(OS_EVENT_STDIN | OS_EVENT_WM, InputEventHandle);
    if (Status != OsSuccess) {
        return -1;
    }
    
    Status = RegisterServiceObject("vioarr", WindowingService, RcChannelHandle, &ServiceHandle);
    if (Status != OsSuccess) {
        return -1;
    }
    ExitCode = sVioarr.Run();
    
    // Cleanup communication resources
    UnregisterServiceObject(ServiceHandle);
    DestroyPipe(InputEventHandle);
    DestroyPipe(RcChannelHandle);
    return ExitCode;
}
