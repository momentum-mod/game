#pragma once

#include "cbase.h"
#include "IMenuOverride.h"
#include "vgui_controls/EditablePanel.h"
#include "GameUI/IGameUI.h"

// This class is what is actually used instead of the main menu.
class CMOMMenuOverride : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CMOMMenuOverride, vgui::EditablePanel);
public:
    CMOMMenuOverride(vgui::VPANEL parent);
    virtual ~CMOMMenuOverride();

    IGameUI*		GetGameUI();

protected:
    void	ApplySchemeSettings(vgui::IScheme *pScheme) override;

    void OnCommand(const char *command) override;

private:
    bool			LoadGameUI();

    int				m_ExitingFrameCount;
    bool			m_bCopyFrameBuffer;

    IGameUI*		m_pGameUI;
};


// Derived class of override interface
class COverrideInterface : public IMenuOverride
{
private:
    CMOMMenuOverride *m_pMainMenu;

public:
    COverrideInterface(void)
    {
        m_pMainMenu = nullptr;
    }

    void Create(vgui::VPANEL parent) override
    {
        // Create immediately
        m_pMainMenu = new CMOMMenuOverride(parent);

        //Set this as the main menu override
        if (m_pMainMenu->GetGameUI())
        {
            m_pMainMenu->GetGameUI()->SetMainMenuOverride(m_pMainMenu->GetVPanel());
        }
    }

    vgui::VPANEL GetPanel(void) override
    {
        if (!m_pMainMenu)
            return NULL;
        return m_pMainMenu->GetVPanel();
    }

    void Destroy(void) override
    {
        if (m_pMainMenu)
        {
            m_pMainMenu->SetParent(nullptr);
            delete m_pMainMenu;
        }
    }
};