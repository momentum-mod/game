#include "cbase.h"
#include "mom_nui_panel.h"
#include "clientmode_shared.h"
#include "nui.h"
#include "nui_frame.h"

#include "vgui/IInput.h"
#include "vgui/IVGui.h"

CMomNUIPanel* g_pMomNUIPanel = nullptr;

CMomNUIPanel::CMomNUIPanel(vgui::VPANEL parent) :
    BaseClass(nullptr, "MomNUIPanel"),
    m_iTextureID(0),
    m_iLastWidth(0),
    m_iLastHeight(0)
{
    SetParent(parent);

    int width, height;
    GetClientModeNormal()->GetViewport()->GetSize(width, height);

    SetBounds(0, 0, width, height);
    SetBgColor(Color(0, 0, 0, 0));
    SetPaintBackgroundEnabled(false);
    
    SetProportional(false);
    SetTitleBarVisible(false);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(false);
    SetSizeable(false);
    SetMoveable(false);
    SetVisible(true);

    vgui::ivgui()->AddTickSignal(GetVPanel());

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    m_iTextureID = surface()->CreateNewTextureID(true);
}

CMomNUIPanel::~CMomNUIPanel()
{

}

void CMomNUIPanel::OnThink()
{
    BaseClass::OnThink();

    int width, height;
    GetSize(width, height);

    if (width != m_iLastWidth || height != m_iLastHeight)
    {
        m_iLastWidth = width;
        m_iLastHeight = height;

        CMomNUI::GetInstance()->GetFrame()->OnResized(width, height);
    }
}

void CMomNUIPanel::Paint()
{
    auto frame = CMomNUI::GetInstance()->GetFrame();

    if (!frame || !frame->TextureBuffer())
        return;

    if (frame->Dirty())
        surface()->DrawSetTextureRGBAEx(m_iTextureID, frame->TextureBuffer(), frame->FrameWidth(), frame->FrameHeight(), IMAGE_FORMAT_BGRA8888);

    surface()->DrawSetTexture(m_iTextureID);
    surface()->DrawSetColor(Color(255, 255, 255, 255));
    surface()->DrawTexturedRect(0, 0, frame->FrameWidth(), frame->FrameHeight());
}

void CMomNUIPanel::OnCursorEntered()
{
    int x, y;
    vgui::input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    CefMouseEvent event;
    event.x = x;
    event.y = y;
    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseMoveEvent(event, false);
}

void CMomNUIPanel::OnCursorExited()
{
    int x, y;
    vgui::input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    CefMouseEvent event;
    event.x = x;
    event.y = y;
    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseMoveEvent(event, false);
}

void CMomNUIPanel::OnCursorMoved(int x, int y)
{
    CefMouseEvent event;
    event.x = x;
    event.y = y;
    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseMoveEvent(event, false);
}

void CMomNUIPanel::OnMousePressed(vgui::MouseCode code)
{
    int x, y;
    vgui::input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    CefMouseEvent me;
    me.x = x;
    me.y = y;

    CefBrowserHost::MouseButtonType eventType = MBT_LEFT;
    uint32 eventFlags = 0;

    switch (code)
    {
    case MOUSE_LEFT:
        eventType = MBT_LEFT;
        eventFlags |= EVENTFLAG_LEFT_MOUSE_BUTTON;
        break;
    case MOUSE_RIGHT:
        eventType = MBT_RIGHT;
        eventFlags |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
        break;
    case MOUSE_MIDDLE:
        eventType = MBT_MIDDLE;
        eventFlags |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
        break;
    }

    me.modifiers = eventFlags;

    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseClickEvent(me, eventType, false, 1);
}

void CMomNUIPanel::OnMouseDoublePressed(vgui::MouseCode code)
{
    int x, y;
    vgui::input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    CefMouseEvent me;
    me.x = x;
    me.y = y;

    CefBrowserHost::MouseButtonType eventType = MBT_LEFT;
    uint32 eventFlags = 0;

    switch (code)
    {
    case MOUSE_LEFT:
        eventType = MBT_LEFT;
        eventFlags |= EVENTFLAG_LEFT_MOUSE_BUTTON;
        break;
    case MOUSE_RIGHT:
        eventType = MBT_RIGHT;
        eventFlags |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
        break;
    case MOUSE_MIDDLE:
        eventType = MBT_MIDDLE;
        eventFlags |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
        break;
    }

    me.modifiers = eventFlags;

    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseClickEvent(me, eventType, false, 2);
}

void CMomNUIPanel::OnMouseReleased(vgui::MouseCode code)
{
    int x, y;
    vgui::input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    CefMouseEvent me;
    me.x = x;
    me.y = y;

    CefBrowserHost::MouseButtonType eventType = MBT_LEFT;
    uint32 eventFlags = 0;

    switch (code)
    {
    case MOUSE_LEFT:
        eventType = MBT_LEFT;
        eventFlags &= ~EVENTFLAG_LEFT_MOUSE_BUTTON;
        break;
    case MOUSE_RIGHT:
        eventType = MBT_RIGHT;
        eventFlags &= ~EVENTFLAG_RIGHT_MOUSE_BUTTON;
        break;
    case MOUSE_MIDDLE:
        eventType = MBT_MIDDLE;
        eventFlags &= ~EVENTFLAG_MIDDLE_MOUSE_BUTTON;
        break;
    }

    me.modifiers = eventFlags;

    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseClickEvent(me, eventType, true, 1);
}

void CMomNUIPanel::OnMouseWheeled(int delta)
{
    int x, y;
    vgui::input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    CefMouseEvent me;
    me.x = x;
    me.y = y;

    CMomNUI::GetInstance()->GetFrame()->Client()->Browser()->GetHost()->SendMouseWheelEvent(me, 0, delta);
}

void CMomNUIPanel::OnKeyCodePressed(vgui::KeyCode code)
{
    // TODO (OrfeasZ)
}

void CMomNUIPanel::OnKeyCodeTyped(vgui::KeyCode code)
{
    // TODO (OrfeasZ)
}

void CMomNUIPanel::OnKeyTyped(wchar_t unichar)
{
    // TODO (OrfeasZ)
}

void CMomNUIPanel::OnKeyCodeReleased(vgui::KeyCode code)
{
    // TODO (OrfeasZ)
}
