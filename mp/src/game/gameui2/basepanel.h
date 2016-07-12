#pragma once

#include "mainmenu.h"
#include "vgui/ISurface.h"
#include "vgui2d/panel2d.h"

class BasePanel : public Panel2D
{
    DECLARE_CLASS_SIMPLE(BasePanel, Panel2D);

  public:
    BasePanel(vgui::VPANEL parent);

    static void Create();
    void OnThink() override;
    void PaintBlurMask() override;

    virtual bool IsBackgroundMusicPlaying();
    virtual bool StartBackgroundMusic(float fVol);
    virtual void UpdateBackgroundMusicVolume(float fVol);
    virtual void ReleaseBackgroundMusic();

    vgui::VPANEL GetVPanel() override;

    MainMenu *GetMainMenu() const { return m_pMainMenu; }

  private:
    MainMenu *m_pMainMenu;
    CUtlString m_backgroundMusic;
    int32 m_nBackgroundMusicGUID;
};

extern BasePanel *GetBasePanel();