#pragma once

#include "vgui/IVGui.h"

class IMessageboxPanel
{
  public:
    virtual void Create(vgui::VPANEL parent) = 0;
    virtual void Destroy(void) = 0;
    virtual void Activate(void) = 0;
    virtual void Close() = 0;
    virtual vgui::HPanel CreateMessagebox(const char *pTitle, const char *pMessage, const char *pAccept = nullptr) = 0;
    virtual void FlushMessageboxes() = 0;
    virtual void FlushMessageboxes(vgui::HPanel pHp) = 0;
};

extern IMessageboxPanel *messageboxpanel;