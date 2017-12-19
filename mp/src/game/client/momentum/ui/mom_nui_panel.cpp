#include "cbase.h"
#include "mom_nui_panel.h"
#include "clientmode_shared.h"
#include "nui/INuiInterface.h"
#include "vgui_controls/ScrollBar.h"

#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include <src/vgui_key_translation.h>
#include "util/jsontokv.h"

CMomNUIPanel* g_pMomNUIPanel = nullptr;

CMomNUIPanel::CMomNUIPanel() :
    m_iTextureID(0),
    m_iLastWidth(0),
    m_iLastHeight(0)
{
    MakePopup(false);

    int width, height;
    GetClientModeNormal()->GetViewport()->GetSize(width, height);
    SetBounds(0, 0, width, height);

    if (nui->IsInitialized())
        nui->CreateBrowser(this, "file://C:\\Users\\Nick\\Documents\\GitHub\\game\\mp\\game\\momentum\\resource\\html\\menu.html");
    m_bNeedsFullTextureUpload = false;

    _hbar = new ScrollBar(this, "HorizScrollBar", false);
    _hbar->SetVisible(false);
    _hbar->AddActionSignalTarget(this);

    _vbar = new ScrollBar(this, "VertScrollBar", true);
    _vbar->SetVisible(false);
    _vbar->AddActionSignalTarget(this);

    SetBgColor(Color(0, 0, 0, 0));
    SetPaintBackgroundEnabled(false);
    
    SetPaintEnabled(true);
    SetProportional(false);
    SetVisible(false);
    SetBuildModeEditable(false);
    SetBuildModeDeletable(false);

    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(true);
}

CMomNUIPanel::~CMomNUIPanel()
{
    surface()->DestroyTextureID(m_iTextureID);
    nui->ShutdownBrowser(m_hBrowser);
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
        surface()->DrawSetTexture(m_iTextureID);
        int tw = 0, tt = 0;
        surface()->DrawSetColor(Color(255, 255, 255, 255));
        GetSize(tw, tt);
        surface()->DrawTexturedRect(0, 0, tw, tt);
    }
}

void CMomNUIPanel::OnCursorEntered()
{
    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseMove(m_hBrowser, x, y, false);
}

void CMomNUIPanel::OnCursorExited()
{
    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseMove(m_hBrowser, x, y, true);
}

void CMomNUIPanel::OnCursorMoved(int x, int y)
{
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
    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseDown(m_hBrowser, x, y, ConvertMouseCodeToCEFCode(code));
}

void CMomNUIPanel::OnMouseDoublePressed(MouseCode code)
{
    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseDoubleClick(m_hBrowser, x, y, ConvertMouseCodeToCEFCode(code));
}

void CMomNUIPanel::OnMouseReleased(MouseCode code)
{
    int x, y;
    input()->GetCursorPos(x, y);
    ScreenToLocal(x, y);

    nui->MouseUp(m_hBrowser, x, y, ConvertMouseCodeToCEFCode(code));
}

void CMomNUIPanel::OnMouseWheeled(int delta)
{
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
    nui->KeyDown(m_hBrowser, KeyCode_VGUIToVirtualKey(code), GetKeyModifiers());
}

void CMomNUIPanel::OnKeyCodeTyped(KeyCode code)
{
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
    nui->KeyChar(m_hBrowser, unichar, GetKeyModifiers());
}

void CMomNUIPanel::OnKeyCodeReleased(KeyCode code)
{
    nui->KeyUp(m_hBrowser, KeyCode_VGUIToVirtualKey(code), GetKeyModifiers());
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

void CMomNUIPanel::OnBrowserPaint(uint8* pBGRA, uint32 texWide, uint32 texTall, uint32 unUpdateX,
    uint32 unUpdateY, uint32 unUpdateWide, uint32 unUpdateTall, uint32 unScrollX, uint32 unScrollY)
{
    int tw = 0, tt = 0;
    if (m_iTextureID != 0)
    {
        tw = m_allocedTextureWidth;
        tt = m_allocedTextureHeight;
    }

    //if (m_iTextureID != 0 && ((_vbar->IsVisible() && unScrollY > 0 && abs((int) unScrollY - m_scrollVertical.m_nScroll) > 5) || (_hbar->IsVisible() && unScrollX > 0 && abs((int) unScrollX - m_scrollHorizontal.m_nScroll) > 5)))
    {
    //    m_bNeedsFullTextureUpload = true;
    //    return;
    }

    // update the vgui texture
    if (m_bNeedsFullTextureUpload || m_iTextureID == 0 || tw != (int) texWide || tt != (int) texTall)
    {
        m_bNeedsFullTextureUpload = false;
        if (m_iTextureID != 0)
            surface()->DeleteTextureByID(m_iTextureID);
        
        // if the dimensions changed we also need to re-create the texture ID to support the overlay properly (it won't resize a texture on the fly, this is the only control that needs
        //   to so lets have a tiny bit more code here to support that)
        m_iTextureID = surface()->CreateNewTextureID(true);
        surface()->DrawSetTextureRGBAEx(m_iTextureID, pBGRA, texWide, texTall, IMAGE_FORMAT_BGRA8888);// BR FIXME - this call seems to shift by some number of pixels?
        m_allocedTextureWidth = texWide;
        m_allocedTextureHeight = texTall;
    }
    else if ((int) unUpdateWide > 0 && (int) unUpdateTall > 0)
    {
        // same size texture, just bits changing in it, lets twiddle
        surface()->DrawUpdateRegionTextureRGBA(m_iTextureID, unUpdateX, unUpdateY, pBGRA, unUpdateWide, unUpdateTall, IMAGE_FORMAT_BGRA8888);
    }
    else
    {
        surface()->DrawSetTextureRGBAEx(m_iTextureID, pBGRA, texWide, texTall, IMAGE_FORMAT_BGRA8888);
    }

    // need a paint next time
    Repaint();

}

CON_COMMAND(test_menu_change, "Bllah")
{
    g_pMomNUIPanel->LoadURL(args.Arg(1));
}

void CMomNUIPanel::OnBrowserPageLoaded(const char* pURL)
{
    DevLog("Loaded the page %s\n", pURL);

    nui->ExecuteJavascript(m_hBrowser, "setLocalization('english')");
    nui->ExecuteJavascript(m_hBrowser, "setVolume(0.5)");
}

void CMomNUIPanel::OnBrowserJSAlertDialog(const char* pString)
{
    KeyValues* pKv = CJsonToKeyValues::ConvertJsonToKeyValues(pString);
    KeyValuesAD autodelete(pKv);

    if (pKv)
    {
        if (!Q_strcmp(pKv->GetString("id"), "menu"))
        {
            if (pKv->GetBool("special"))
            {
                //GetBasePanel()->RunMenuCommand(pKv->GetString("com"));
            }
            else
            {
                
                //GetBasePanel()->RunEngineCommand(pKv->GetString("com"));
            }
        }
        else if (!Q_strcmp(pKv->GetString("id"), "echo"))
        {
            DevLog("%s\n", pKv->GetString("com"));
        }
        else if (!Q_strcmp(pKv->GetString("id"), "lobby"))
        {
            // Separated here because these commands require a connection to actually work through console,
            // so we manually dispatch the commands here.
            ConCommand* pCommand = g_pCVar->FindCommand(pKv->GetString("com"));
            if (pCommand)
            {
                CCommand blah;
                blah.Tokenize(pKv->GetString("com"));
                pCommand->Dispatch(blah);
            }
        }
    }
}

void CMomNUIPanel::Refresh()
{
    if (nui && m_hBrowser != INVALID_NUIBROWSER)
    {
        nui->Reload(m_hBrowser);
    }
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
