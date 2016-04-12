//The following include files are necessary to allow your MyPanel.cpp to compile.
#include "cbase.h"
#include "IVersionWarnPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/URLLabel.h>

#include "momentum/mom_shareddefs.h"

#include "tier0/memdbgon.h"

//CVersionWarnPanel class: Tutorial example class
class CVersionWarnPanel : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CVersionWarnPanel, vgui::Frame);
    //CVersionWarnPanel : This Class / vgui::Frame : BaseClass

    CVersionWarnPanel(vgui::VPANEL parent); 	// Constructor
    ~CVersionWarnPanel(){};				// Destructor

protected:
    //VGUI overrides:
    virtual void OnTick();
    virtual void OnCommand(const char* pcCommand);

private:
    //Other used VGUI control Elements:
    URLLabel *m_pReleaseText;
    const char *m_pVersion;
};

// Constuctor: Initializes the Panel
CVersionWarnPanel::CVersionWarnPanel(vgui::VPANEL parent)
    : BaseClass(nullptr, "VersionWarnPanel")
{
    SetParent(parent);

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    SetProportional(true);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMoveable(true);
    SetVisible(false);


    SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

    LoadControlSettings("resource/ui/versionwarnpanel.res");

    vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

    m_pReleaseText = FindControl<URLLabel>("ReleaseText", true);
    if (!m_pReleaseText)
    {
        Assert("Missing one more gameui controls from ui/versionwarnpanel.res");
    }

}

//Class: CMyPanelInterface Class. Used for construction.
class CVersionWarnPanelInterface : public VersionWarnPanel
{
private:
    CVersionWarnPanel *MyPanel;
public:
    CVersionWarnPanelInterface()
    {
        MyPanel = nullptr;
    }
    void Create(vgui::VPANEL parent)
    {
        MyPanel = new CVersionWarnPanel(parent);
    }
    void Destroy()
    {
        if (MyPanel)
        {
            MyPanel->SetParent(nullptr);
            delete MyPanel;
        }
    }
    void Activate(void)
    {
        if (MyPanel)
        {
            MyPanel->Activate();
        }
    }
    void Close()
    {
        if (MyPanel)
        {
            MyPanel->Close();
        }
    }
};
static CVersionWarnPanelInterface g_VersionWarn;
VersionWarnPanel* versionwarnpanel = (VersionWarnPanel*)&g_VersionWarn;

ConVar cl_showversionwarnpanel("cl_showversionwarnpanel", "0", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_HIDDEN, "Sets the state of versionwarnpanel");

void CVersionWarnPanel::OnTick()
{
    BaseClass::OnTick();
    if (Q_strcmp(cl_showversionwarnpanel.GetString(), cl_showversionwarnpanel.GetDefault()) != 0)
    {
        SetVisible(true);        
        HFont m_hfReleaseFont = m_pReleaseText->GetFont();
        char m_cReleaseText[225];
        m_pReleaseText->GetText(m_cReleaseText, sizeof(m_cReleaseText));
        char m_cReleaseF[225];

        Q_snprintf(m_cReleaseF, 225, m_cReleaseText, MOM_CURRENT_VERSION, cl_showversionwarnpanel.GetString());
        cl_showversionwarnpanel.Revert();
        m_pReleaseText->SetText(m_cReleaseF);
        m_pReleaseText->SetURL("https://github.com/momentum-mod/game/releases");
        SetSize(UTIL_ComputeStringWidth(m_hfReleaseFont, m_cReleaseF) + m_pReleaseText->GetXPos() * 2, GetTall());
        m_pReleaseText->SetPos(m_pReleaseText->GetXPos(), GetTall() / 2);
    }
}

void CVersionWarnPanel::OnCommand(const char* pcCommand)
{
    BaseClass::OnCommand(pcCommand);

    if (!Q_stricmp(pcCommand, "turnoff"))
    {
        SetVisible(false);
        Close();
    }
}

CON_COMMAND(mom_version, "Prints mod current installed version")
{
    Log("Mod currently installed version: %s\n",MOM_CURRENT_VERSION);
}