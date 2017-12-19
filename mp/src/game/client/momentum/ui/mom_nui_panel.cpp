#include "cbase.h"
#include "mom_nui_panel.h"
#include "nui/INuiInterface.h"
#include "vgui_controls/ScrollBar.h"
#include <vgui/ISurface.h>
#include "vgui/IInput.h"
#include <src/vgui_key_translation.h>

using namespace vgui;

CMomNUIPanel::CMomNUIPanel() :
    m_pTextureBuffer(nullptr),
    m_bDirtyBuffer(true),
    m_iTextureID(0),
    m_iLastWidth(0),
    m_iLastHeight(0),
    m_allocedTextureWidth(0),
    m_allocedTextureHeight(0),
    m_bNeedsFullTextureUpload(false)
{
    _hbar = new ScrollBar(this, "HorizScrollBar", false);
    _hbar->SetVisible(false);
    _hbar->AddActionSignalTarget(this);

    _vbar = new ScrollBar(this, "VertScrollBar", true);
    _vbar->SetVisible(false);
    _vbar->AddActionSignalTarget(this);
}

CMomNUIPanel::~CMomNUIPanel()
{
    if (m_iTextureID)
        surface()->DestroyTextureID(m_iTextureID);
    if (m_pTextureBuffer)
        delete[] m_pTextureBuffer;
    if (m_hBrowser)
        nui->ShutdownBrowser(m_hBrowser);
}

void CMomNUIPanel::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
}

void CMomNUIPanel::OnThink()
{
    BaseClass::OnThink();

    int width, height;
    GetSize(width, height);

    if ((width != m_iLastWidth || height != m_iLastHeight) && m_hBrowser != INVALID_NUIBROWSER)
    {
        m_iLastWidth = width;
        m_iLastHeight = height;

        nui->WasResized(m_hBrowser);
    }
}

void CMomNUIPanel::Paint()
{
    if (m_iTextureID != 0)
    {
        if (m_bDirtyBuffer)
        {
            surface()->DrawSetTextureRGBAEx(m_iTextureID, m_pTextureBuffer, m_allocedTextureWidth, m_allocedTextureHeight, IMAGE_FORMAT_BGRA8888);
            m_bDirtyBuffer = false;
        }

        surface()->DrawSetTexture(m_iTextureID);
        int tw = 0, tt = 0;
        surface()->DrawSetColor(Color(255, 255, 255, GetAlpha()));
        GetSize(tw, tt);
        surface()->DrawTexturedRect(0, 0, tw, tt);
    }
}

void CMomNUIPanel::OnCursorEntered()
{
    BaseClass::OnCursorEntered();

    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseMove(m_hBrowser, x, y, false);
}

void CMomNUIPanel::OnCursorExited()
{
    BaseClass::OnCursorExited();

    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseMove(m_hBrowser, x, y, true);
}

void CMomNUIPanel::OnCursorMoved(int x, int y)
{
    BaseClass::OnCursorMoved(x, y);

    if (m_hBrowser != INVALID_NUIBROWSER)
        nui->MouseMove(m_hBrowser, x, y, false);
}

INuiInterface::EHTMLMouseButton ConvertMouseCodeToCEFCode(MouseCode code)
{
    switch (code)
    {
    default:
    case MOUSE_LEFT:
        return INuiInterface::eHTMLMouseButton_Left;
    case MOUSE_RIGHT:
        return INuiInterface::eHTMLMouseButton_Right;
    case MOUSE_MIDDLE:
        return INuiInterface::eHTMLMouseButton_Middle;
    }
}

void CMomNUIPanel::OnMousePressed(MouseCode code)
{
    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseDown(m_hBrowser, x, y, ConvertMouseCodeToCEFCode(code));
}

void CMomNUIPanel::OnMouseDoublePressed(MouseCode code)
{
    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseDoubleClick(m_hBrowser, x, y, ConvertMouseCodeToCEFCode(code));
}

void CMomNUIPanel::OnMouseReleased(MouseCode code)
{
    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseUp(m_hBrowser, x, y, ConvertMouseCodeToCEFCode(code));
}

void CMomNUIPanel::OnMouseWheeled(int delta)
{
    BaseClass::OnMouseWheeled(delta);

    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    if (_vbar)
    {
        int val = _vbar->GetValue();
        val -= (delta * 100.0 / 3.0); // 100 for every 3 lines matches chromes code
        _vbar->SetValue(val);
    }

    nui->MouseWheel(m_hBrowser, delta * 100.0 / 3.0);
}


//-----------------------------------------------------------------------------
// Purpose: return the bitmask of any modifier keys that are currently down
//-----------------------------------------------------------------------------
INuiInterface::EHTMLKeyModifiers GetKeyModifiers()
{
    // Any time a key is pressed reset modifier list as well
    int nModifierCodes = 0;
    if (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL))
        nModifierCodes |= INuiInterface::k_eHTMLKeyModifier_CtrlDown;

    if (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT))
        nModifierCodes |= INuiInterface::k_eHTMLKeyModifier_AltDown;

    if (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT))
        nModifierCodes |= INuiInterface::k_eHTMLKeyModifier_ShiftDown;

#ifdef OSX
    // for now pipe through the cmd-key to be like the control key so we get copy/paste
    if (input()->IsKeyDown(KEY_LWIN) || input()->IsKeyDown(KEY_RWIN))
        nModifierCodes |= INuiInterface::k_eHTMLKeyModifier_CtrlDown;
#endif

    return static_cast<INuiInterface::EHTMLKeyModifiers>(nModifierCodes);
}

void CMomNUIPanel::OnKeyCodePressed(KeyCode code)
{
    BaseClass::OnKeyCodePressed(code);

    if (m_hBrowser != INVALID_NUIBROWSER)
        nui->KeyDown(m_hBrowser, KeyCode_VGUIToVirtualKey(code), GetKeyModifiers());
}

void CMomNUIPanel::OnKeyCodeTyped(KeyCode code)
{
    BaseClass::OnKeyCodeTyped(code);

    if (m_hBrowser == INVALID_NUIBROWSER)
        return;

    switch (code)
    {
    case KEY_PAGEDOWN:
    {
        int val = _vbar->GetValue();
        val += 200;
        _vbar->SetValue(val);
        break;
    }
    case KEY_PAGEUP:
    {
        int val = _vbar->GetValue();
        val -= 200;
        _vbar->SetValue(val);
        break;
    }
    case KEY_F5:
    {
        Refresh();
        break;
    }
    case KEY_TAB:
    {
        if (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL))
        {
            // pass control-tab to parent (through baseclass)
            BaseClass::OnKeyTyped(code);
            return;
        }
        break;
    }
    default:
        break;
    }

    nui->KeyDown(m_hBrowser, KeyCode_VGUIToVirtualKey(code), GetKeyModifiers());
}

void CMomNUIPanel::OnKeyTyped(wchar_t unichar)
{
    BaseClass::OnKeyTyped(unichar);

    if (m_hBrowser != INVALID_NUIBROWSER)
        nui->KeyChar(m_hBrowser, unichar, GetKeyModifiers());
}

void CMomNUIPanel::OnKeyCodeReleased(KeyCode code)
{
    BaseClass::OnKeyCodeReleased(code);

    if (m_hBrowser != INVALID_NUIBROWSER)
        nui->KeyUp(m_hBrowser, KeyCode_VGUIToVirtualKey(code), GetKeyModifiers());
}

void CMomNUIPanel::OnSizeChanged(int newWide, int newTall)
{
    BaseClass::OnSizeChanged(newWide, newTall);

    if (m_hBrowser != INVALID_NUIBROWSER)
    {
        m_iLastWidth = newWide;
        m_iLastHeight = newTall;

        nui->WasResized(m_hBrowser);
    }
}

void CMomNUIPanel::OnBrowserCreated(HNUIBrowser handle)
{
    m_hBrowser = handle;
    SetVisible(true);
}

void CMomNUIPanel::OnBrowserClosed()
{
    m_hBrowser = INVALID_NUIBROWSER;
}

void CMomNUIPanel::OnBrowserSize(int& wide, int& tall)
{
    GetSize(wide, tall);
}

void CMomNUIPanel::OnBrowserFailedToCreate()
{
    m_hBrowser = INVALID_NUIBROWSER;
}

void CMomNUIPanel::OnBrowserPaint(const void* pBGRA, uint32 texWide, uint32 texTall, uint32 unUpdateX,
    uint32 unUpdateY, uint32 unUpdateWide, uint32 unUpdateTall, uint32 unScrollX, uint32 unScrollY)
{

    //if (m_iTextureID != 0 && ((_vbar->IsVisible() && unScrollY > 0 && abs((int) unScrollY - m_scrollVertical.m_nScroll) > 5) || (_hbar->IsVisible() && unScrollX > 0 && abs((int) unScrollX - m_scrollHorizontal.m_nScroll) > 5)))
    //{
    //    m_bNeedsFullTextureUpload = true;
    //    return;
    //}

    if (m_iTextureID == 0 || m_allocedTextureWidth != texWide || m_allocedTextureHeight !=  texTall)
    {
        if (m_iTextureID != 0)
            surface()->DeleteTextureByID(m_iTextureID);

        if (m_pTextureBuffer)
            delete[] m_pTextureBuffer;

        m_allocedTextureWidth = texWide;
        m_allocedTextureHeight = texTall;

        m_iTextureID = surface()->CreateNewTextureID(true);
        m_pTextureBuffer = new uint8[texWide * texTall * 4];
        memcpy(m_pTextureBuffer, pBGRA, texWide * texTall * 4);
    }
    else if (unUpdateWide > 0 && unUpdateTall > 0)
    {
        const unsigned char *textureBuffer = static_cast<const unsigned char *>(pBGRA);
        
        for (uint32 y = unUpdateY; y < unUpdateY + unUpdateTall; y++)
        {
            memcpy(m_pTextureBuffer + (y * texWide * 4) + (unUpdateX * 4),
                     textureBuffer + (y * texWide * 4) + (unUpdateX * 4), 
                     unUpdateWide * 4);
        }
    }
    else
    {
        memcpy(m_pTextureBuffer, pBGRA, texWide * texTall * 4);
    }

    m_bDirtyBuffer = true;

    // need a paint next time
    Repaint();
}

void CMomNUIPanel::Refresh()
{
    if (m_hBrowser != INVALID_NUIBROWSER)
        nui->Reload(m_hBrowser);
}

void CMomNUIPanel::LoadURL(const char* pURL)
{
    if (m_hBrowser != INVALID_NUIBROWSER)
        nui->LoadURL(m_hBrowser, pURL, nullptr);
}

void CMomNUIPanel::OnSliderMoved()
{
    if (_hbar->IsVisible())
    {
        //int scrollX = _hbar->GetValue();
        //if (m_SteamAPIContext.SteamHTMLSurface())
        //    m_SteamAPIContext.SteamHTMLSurface()->SetHorizontalScroll(m_unBrowserHandle, scrollX);
    }

    if (_vbar->IsVisible())
    {
        //int scrollY = _vbar->GetValue();
        //if (m_SteamAPIContext.SteamHTMLSurface())
        //    m_SteamAPIContext.SteamHTMLSurface()->SetVerticalScroll(m_unBrowserHandle, scrollY);
    }
}
