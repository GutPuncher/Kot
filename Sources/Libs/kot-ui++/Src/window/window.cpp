#include <kot-ui++/window.h>

#include <kot-graphics/orb.h>

#include <kot/uisd/srvs/system.h>

#include <kot-ui++/component.h>

namespace UiWindow {

    void EventHandler(enum Window_Event EventType, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3, uint64_t GP4){
        Window* Wid = (Window*)Sys_GetExternalDataThread();
        Wid->Handler(EventType, GP0, GP1, GP2, GP3, GP4);
        Sys_Event_Close();
    }

    Window::Window(char* title, uint32_t Width, uint32_t Height, uint32_t XPosition, uint32_t YPosition){
        // Setup event
        Sys_Event_Create(&WindowEvent);
        Sys_Createthread(Sys_GetProcess(), (uintptr_t)&EventHandler, PriviledgeApp, (uint64_t)this, &WindowHandlerThread);
        Sys_Event_Bind(WindowEvent, WindowHandlerThread, false);
        this->EventBuffer = CreateEventBuffer(Width, Height);

        // Setup window
        this->Wid = CreateWindow(WindowEvent, Window_Type_Default);
        ResizeWindow(this->Wid, Width, Height);
        WindowChangePosition(this->Wid, XPosition, YPosition);

        Borders = true;
        if(Borders){
            BordersCtx = CreateGraphicContext(&Wid->Framebuffer);
            framebuffer_t FramebufferWithoutBorder;
            FramebufferWithoutBorder.Bpp = Wid->Framebuffer.Bpp;
            FramebufferWithoutBorder.Btpp = Wid->Framebuffer.Btpp;
            FramebufferWithoutBorder.Width = Wid->Framebuffer.Width - 2;
            FramebufferWithoutBorder.Height = Wid->Framebuffer.Height - 2;
            FramebufferWithoutBorder.Pitch = Wid->Framebuffer.Pitch;
            FramebufferWithoutBorder.Buffer = (uintptr_t)((uint64_t)Wid->Framebuffer.Buffer + Wid->Framebuffer.Btpp + Wid->Framebuffer.Pitch);
            DrawBorders(0x101010);

            GraphicCtx = CreateGraphicContext(&FramebufferWithoutBorder);
            UiCtx = new Ui::UiContext(&FramebufferWithoutBorder);
        }else{
            GraphicCtx = CreateGraphicContext(&Wid->Framebuffer);
            UiCtx = new Ui::UiContext(&Wid->Framebuffer);
        }

        // Test

        /* auto imgtest = Ui::Picturebox("kotlogo.tga", Ui::ImageType::_TGA, { .Width = 256, .Height = 256 });
        this->SetContent(imgtest); */

        auto titlebar = Ui::titlebar(title, { .backgroundColor = WIN_BGCOLOR_ONFOCUS, .foregroundColor = 0xDDDDDD });
        this->SetContent(titlebar);
 
/*         auto wrapper = Ui::box({ .Width = this->UiCtx->fb->Width, .Height = this->UiCtx->fb->Height - titlebar->GetStyle()->Height, .color = WIN_BGCOLOR_ONFOCUS });

        auto flexbox = UiLayout::Flexbox({}, Ui::Layout::HORIZONTAL);

        auto box = Ui::box({ .Width = 20, .Height = 20, .color = 0xFFFF00 });
        flexbox->addChild(box);

        wrapper->addChild(flexbox);

        this->SetContent(wrapper); */

        ChangeVisibilityWindow(this->Wid, true);
    }

    void Window::DrawBorders(uint32_t Color){
        if(Borders){
            ctxDrawRect(BordersCtx, 0, 0, BordersCtx->Width - 1, BordersCtx->Height - 1, Color);
        }
    }

    void Window::SetContent(Ui::Component* content) {
        Ui::Component* windowCpnt = this->UiCtx->cpnt;

        windowCpnt->addChild(content);
        windowCpnt->update();
    }

    void Window::Handler(enum Window_Event EventType, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3, uint64_t GP4){
        switch (EventType){
            case Window_Event_Focus:
                HandlerFocus(GP0);
                break;
            case Window_Event_Mouse:
                HandlerMouse(GP0, GP1, GP2, GP3);
                break;
            case Window_Event_Keyboard:
                // TODO
                break;
            default:
                break;
        }
    }

    void Window::HandlerFocus(bool IsFocus){
        
    }

    void Window::HandlerMouse(uint64_t PositionX, uint64_t PositionY, uint64_t ZValue, uint64_t Status){
        //GetEventData(EventBuffer, PositionX - Wid->Position.x, PositionY - Wid->Position.y);
    }

}