#include "basepanel.h"
#include "gameui2_interface.h"

//#include "mainmenu.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"

#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static BasePanel *g_pBasePanel;
BasePanel *GetBasePanel() { return g_pBasePanel; }

BasePanel::BasePanel(vgui::VPANEL parent) : BaseClass(nullptr)
{
    SetParent(parent);
    SetZPos(-100);//This guy is going way back
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetKeyBoardInputEnabled(false);//And we're not taking input on it
    SetMouseInputEnabled(false);//Whatsoever, because it could mess with the main menu
    SetProportional(false);
    SetVisible(true);
    SetPostChildPaintEnabled(true);

    m_backgroundMusic = "Interface.Music";
    m_nBackgroundMusicGUID = 0;

    g_pVGuiLocalize->AddFile("resource/momentum_%language%.txt");

    m_pMainMenu = new MainMenu(nullptr);
}

vgui::VPANEL BasePanel::GetVPanel() { return BaseClass::GetVPanel(); }

void BasePanel::Create()
{
    ConColorMsg(Color(0, 148, 255, 255), "Trying to create BasePanel...\n");

    if (g_pBasePanel == nullptr)
    {
        g_pBasePanel = new BasePanel(GameUI2().GetRootPanel());
        ConColorMsg(Color(0, 148, 255, 255), "BasePanel created.\n");
    }
    else
    {
        ConColorMsg(Color(0, 148, 255, 255), "BasePanel already exists.\n");
    }
}

void BasePanel::OnThink()
{
    BaseClass::OnThink();

    if (vgui::ipanel())
        SetBounds(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);
    
    if (!CommandLine()->FindParm("-nostartupsound"))
    {
        if (!IsBackgroundMusicPlaying())
        {
            if (GameUI2().IsInBackgroundLevel() || !GameUI2().IsInLevel())
                StartBackgroundMusic(1.0f);
        }
        else if (IsBackgroundMusicPlaying())
        {
            if (!GameUI2().IsInBackgroundLevel() || GameUI2().IsInLevel())
                ReleaseBackgroundMusic();
        }
    }
}

void BasePanel::PaintBlurMask()
{
    BaseClass::PaintBlurMask();

    if (GameUI2().IsInLevel())
    {
        vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));
        vgui::surface()->DrawFilledRect(0, 0, GameUI2().GetViewport().x, GameUI2().GetViewport().y);
    }
}

bool BasePanel::IsBackgroundMusicPlaying()
{
    if (m_backgroundMusic.IsEmpty())
        return false;

    if (m_nBackgroundMusicGUID == 0)
        return false;

    return GameUI2().GetEngineSound()->IsSoundStillPlaying(m_nBackgroundMusicGUID);
}

#define BACKGROUND_MUSIC_DUCK 0.15f

bool BasePanel::StartBackgroundMusic(float fVol)
{
    if (IsBackgroundMusicPlaying())
        return true;

    if (m_backgroundMusic.IsEmpty())
        return false;

    CSoundParameters params;
    if (!GameUI2().GetSoundEmitterSystemBase()->GetParametersForSound(m_backgroundMusic.Get(), params, GENDER_NONE))
        return false;

    GameUI2().GetEngineSound()->EmitAmbientSound(params.soundname, params.volume * fVol, params.pitch);
    m_nBackgroundMusicGUID = GameUI2().GetEngineSound()->GetGuidForLastSoundEmitted();

    return (m_nBackgroundMusicGUID != 0);
}

void BasePanel::UpdateBackgroundMusicVolume(float fVol)
{
    if (!IsBackgroundMusicPlaying())
        return;

    GameUI2().GetEngineSound()->SetVolumeByGuid(m_nBackgroundMusicGUID, BACKGROUND_MUSIC_DUCK * fVol);
}

void BasePanel::ReleaseBackgroundMusic()
{
    if (m_backgroundMusic.IsEmpty())
        return;

    if (m_nBackgroundMusicGUID == 0)
        return;

    GameUI2().GetEngineSound()->StopSoundByGuid(m_nBackgroundMusicGUID);

    m_nBackgroundMusicGUID = 0;
}
