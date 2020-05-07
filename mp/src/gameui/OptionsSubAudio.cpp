//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "OptionsSubAudio.h"

#include "ModInfo.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/QueryBox.h"
#include "vgui_controls/CvarSlider.h"
#include <vgui_controls/CvarToggleCheckButton.h>
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include "vgui/IInput.h"
#include "tier1/strtools.h"
#include "EngineInterface.h"
#include "GameUI_Interface.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


// This member is static so that the updated audio language can be referenced during shutdown
char* COptionsSubAudio::m_pchUpdatedAudioLanguage = (char*)GetLanguageShortName( k_Lang_English );

enum SoundQuality_e
{
	SOUNDQUALITY_LOW,
	SOUNDQUALITY_MEDIUM,
	SOUNDQUALITY_HIGH,
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
COptionsSubAudio::COptionsSubAudio(vgui::Panel *parent) : PropertyPage(parent, NULL), m_cvarSubtitles("cc_subtitles"), m_cvarCloseCaption("closecaption"),
      m_cvarSndSurroundSpeakers("Snd_Surround_Speakers"), m_cvarSndPitchQuality("Snd_PitchQuality"),
      m_cvarDSPSlowCPU("dsp_slow_cpu"), m_cvarDSPEnhanceStereo("dsp_enhance_stereo")
{
    SetSize(20, 20);

	m_pSFXSlider = new CvarSlider(this, "SFXSlider", "volume", 2, true);
    m_pMusicSlider = new CvarSlider(this, "MusicSlider", "snd_musicvolume", 2, true);
	
	m_pCloseCaptionCombo = new ComboBox( this, "CloseCaptionCheck", 3, false );
	m_pCloseCaptionCombo->AddItem( "#GameUI_NoClosedCaptions", NULL );
	m_pCloseCaptionCombo->AddItem( "#GameUI_SubtitlesAndSoundEffects", NULL );
	m_pCloseCaptionCombo->AddItem( "#GameUI_Subtitles", NULL );

	m_pSoundQualityCombo = new ComboBox( this, "SoundQuality", 3, false );
	m_pSoundQualityCombo->AddItem( "#GameUI_High", new KeyValues("SoundQuality", "quality", SOUNDQUALITY_HIGH) );
	m_pSoundQualityCombo->AddItem( "#GameUI_Medium", new KeyValues("SoundQuality", "quality", SOUNDQUALITY_MEDIUM) );
	m_pSoundQualityCombo->AddItem( "#GameUI_Low", new KeyValues("SoundQuality", "quality", SOUNDQUALITY_LOW) );

	m_pSpeakerSetupCombo = new ComboBox( this, "SpeakerSetup", 6, false );
	m_pSpeakerSetupCombo->AddItem( "#GameUI_Headphones", new KeyValues("SpeakerSetup", "speakers", 0) );
	m_pSpeakerSetupCombo->AddItem( "#GameUI_2Speakers", new KeyValues("SpeakerSetup", "speakers", 2) );
	m_pSpeakerSetupCombo->AddItem( "#GameUI_4Speakers", new KeyValues("SpeakerSetup", "speakers", 4) );
	m_pSpeakerSetupCombo->AddItem( "#GameUI_5Speakers", new KeyValues("SpeakerSetup", "speakers", 5) );
	m_pSpeakerSetupCombo->AddItem( "#GameUI_7Speakers", new KeyValues("SpeakerSetup", "speakers", 7) );

    m_pSpokenLanguageCombo = new ComboBox (this, "AudioSpokenLanguage", 6, false );

    m_pMuteLoseFocus = new CvarToggleCheckButton(this, "snd_mute_losefocus", "#GameUI_SndMuteLoseFocus", "snd_mute_losefocus");

	LoadControlSettings("resource/optionssubaudio.res");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
COptionsSubAudio::~COptionsSubAudio()
{
}

//-----------------------------------------------------------------------------
// Purpose: Reloads data
//-----------------------------------------------------------------------------
void COptionsSubAudio::OnResetData()
{
    ResetCloseCaption();
    ResetSpeakerSetup();
    ResetSoundQuality();
    ResetSpokenLanguage();
}

void COptionsSubAudio::ResetCloseCaption()
{
    if (m_cvarCloseCaption.GetBool())
    {
        if (m_cvarSubtitles.GetBool())
        {
            m_pCloseCaptionCombo->ActivateItem(2);
        }
        else
        {
            m_pCloseCaptionCombo->ActivateItem(1);
        }
    }
    else
    {
        m_pCloseCaptionCombo->ActivateItem(0);
    }
}

void COptionsSubAudio::ResetSpokenLanguage()
{
    char szCurrentLanguage[50] = "";
    char szAvailableLanguages[512] = "";
    szAvailableLanguages[0] = NULL;

    // Fallback to current engine language
    engine->GetUILanguage(szCurrentLanguage, sizeof(szCurrentLanguage));

    // When Steam isn't running we can't get the language info...
    if (SteamApps())
    {
        Q_strncpy(szCurrentLanguage, SteamApps()->GetCurrentGameLanguage(), sizeof(szCurrentLanguage));
        Q_strncpy(szAvailableLanguages, SteamApps()->GetAvailableGameLanguages(), sizeof(szAvailableLanguages));
    }

    // Get the spoken language and store it for comparison purposes
    m_nCurrentAudioLanguage = PchLanguageToELanguage(szCurrentLanguage);

    // Check to see if we have a list of languages from Steam
    if (V_strlen(szAvailableLanguages))
    {
        // Populate the combo box with each available language
        CSplitString languagesList(szAvailableLanguages, ",");

        for (int i = 0; i < languagesList.Count(); i++)
        {
            const ELanguage languageCode = PchLanguageToELanguage(languagesList[i]);
            m_pSpokenLanguageCombo->AddItem(GetLanguageVGUILocalization(languageCode),
                                            new KeyValues("Audio Languages", "language", languageCode));
        }
    }
    else
    {
        // Add the current language to the combo
        m_pSpokenLanguageCombo->AddItem(GetLanguageVGUILocalization(m_nCurrentAudioLanguage),
                                        new KeyValues("Audio Languages", "language", m_nCurrentAudioLanguage));
    }

    // Activate the current language in the combo
    for (int itemID = 0; itemID < m_pSpokenLanguageCombo->GetItemCount(); itemID++)
    {
        KeyValues *kv = m_pSpokenLanguageCombo->GetItemUserData(itemID);
        if (kv && kv->GetInt("language") == m_nCurrentAudioLanguage)
        {
            m_pSpokenLanguageCombo->ActivateItem(itemID);
            break;
        }
    }
}

void COptionsSubAudio::ResetSoundQuality()
{
    int quality = SOUNDQUALITY_LOW;
    if (!m_cvarDSPSlowCPU.GetBool())
    {
        quality = SOUNDQUALITY_MEDIUM;
    }
    if (m_cvarSndPitchQuality.GetBool())
    {
        quality = SOUNDQUALITY_HIGH;
    }
    // find the item in the list and activate it
    for (int itemID = 0; itemID < m_pSoundQualityCombo->GetItemCount(); itemID++)
    {
        KeyValues *kv = m_pSoundQualityCombo->GetItemUserData(itemID);
        if (kv && kv->GetInt("quality") == quality)
        {
            m_pSoundQualityCombo->ActivateItem(itemID);
        }
    }
}

void COptionsSubAudio::ResetSpeakerSetup()
{
    int speakers = m_cvarSndSurroundSpeakers.GetInt();
    for (int itemID = 0; itemID < m_pSpeakerSetupCombo->GetItemCount(); itemID++)
    {
        KeyValues *kv = m_pSpeakerSetupCombo->GetItemUserData(itemID);
        if (kv && kv->GetInt("speakers") == speakers)
        {
            m_pSpeakerSetupCombo->ActivateItem(itemID);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Called on controls changing, enables the Apply button
//-----------------------------------------------------------------------------
void COptionsSubAudio::OnControlModified(Panel *panel)
{
    if (panel == m_pCloseCaptionCombo)
    {
        ApplyCloseCaption();
    }
    else if (panel == m_pSoundQualityCombo)
    {
        ApplySoundQuality();
    }
    else if (panel == m_pSpeakerSetupCombo)
    {
        ApplySpeakerSetup();
    }
    else if (panel == m_pSpokenLanguageCombo)
    {
        ApplySpokenLanguage();
    }
}

void COptionsSubAudio::ApplyCloseCaption()
{
    // Tracker 28933:  Note we can't do this because closecaption is marked
    //  FCVAR_USERINFO and it won't get sent to server is we direct set it, we
    //  need to pass it along to the engine parser!!!
    // ConVar *closecaption = (ConVar *)cvar->FindVar("closecaption");
    int closecaption_value = 0;

    switch (m_pCloseCaptionCombo->GetActiveItem())
    {
    default:
    case 0:
        closecaption_value = 0;
        m_cvarSubtitles.SetValue(0);
        break;
    case 1:
        closecaption_value = 1;
        m_cvarSubtitles.SetValue(0);
        break;
    case 2:
        closecaption_value = 1;
        m_cvarSubtitles.SetValue(1);
        break;
    }

    // Stuff the close caption change to the console so that it can be
    //  sent to the server (FCVAR_USERINFO) so that you don't have to restart
    //  the level for the change to take effect.
    char cmd[64];
    Q_snprintf(cmd, sizeof(cmd), "closecaption %i\n", closecaption_value);
    engine->ClientCmd_Unrestricted(cmd);
}

void COptionsSubAudio::ApplySpokenLanguage()
{
    // Audio spoken language
    KeyValues *kv = m_pSpokenLanguageCombo->GetItemUserData(m_pSpokenLanguageCombo->GetActiveItem());
    const ELanguage nUpdatedAudioLanguage = (ELanguage)(kv ? kv->GetInt("language") : k_Lang_English);

    if (nUpdatedAudioLanguage != m_nCurrentAudioLanguage)
    {
        // Store new language in static member so that it can be accessed during shutdown when this instance is gone
        m_pchUpdatedAudioLanguage = (char *)GetLanguageShortName(nUpdatedAudioLanguage);

        // Inform user that they need to restart in order change language at this time
        QueryBox *qb = new QueryBox("#GameUI_ChangeLanguageRestart_Title", "#GameUI_ChangeLanguageRestart_Info",
                                    GetParent()->GetParent()->GetParent());
        if (qb != NULL)
        {
            qb->SetOKCommand(new KeyValues("Command", "command", "RestartWithNewLanguage"));
            qb->SetOKButtonText("#GameUI_ChangeLanguageRestart_OkButton");
            qb->SetCancelButtonText("#GameUI_ChangeLanguageRestart_CancelButton");
            qb->AddActionSignalTarget(GetParent()->GetParent()->GetParent());
            qb->DoModal();
        }
    }
}

void COptionsSubAudio::ApplySoundQuality()
{
    KeyValues *qualityKv = m_pSoundQualityCombo->GetActiveItemUserData();

    if (!qualityKv)
        return;

    switch (qualityKv->GetInt("quality"))
    {
    case SOUNDQUALITY_LOW:
        m_cvarDSPSlowCPU.SetValue(true);
        m_cvarSndPitchQuality.SetValue(false);
        break;
    case SOUNDQUALITY_MEDIUM:
        m_cvarDSPSlowCPU.SetValue(false);
        m_cvarSndPitchQuality.SetValue(false);
        break;
    case SOUNDQUALITY_HIGH:
        m_cvarDSPSlowCPU.SetValue(false);
        m_cvarSndPitchQuality.SetValue(true);
        break;
    default:
        Assert("Undefined sound quality setting.");
    };

    ApplyEnhanceStereo();
}

void COptionsSubAudio::ApplySpeakerSetup()
{
    KeyValues *speakersKv = m_pSpeakerSetupCombo->GetActiveItemUserData();

    if (speakersKv)
    {
        m_cvarSndSurroundSpeakers.SetValue(speakersKv->GetInt("speakers"));
        ApplyEnhanceStereo();
    }
}

void COptionsSubAudio::ApplyEnhanceStereo()
{
    KeyValues *speakersKv = m_pSpeakerSetupCombo->GetActiveItemUserData();
    KeyValues *qualityKv = m_pSoundQualityCombo->GetActiveItemUserData();

    if (!speakersKv || !qualityKv)
        return;

    // headphones at high quality get enhanced stereo turned on
    if (speakersKv->GetInt("speakers") == 0 && qualityKv->GetInt("quality") == SOUNDQUALITY_HIGH)
    {
        m_cvarDSPEnhanceStereo.SetValue(1);
    }
    else
    {
        m_cvarDSPEnhanceStereo.SetValue(0);
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubAudio::OnCommand( const char *command )
{
	if ( !stricmp( command, "TestSpeakers" ) )
	{
		// ask them if they REALLY want to test the speakers if they're in a game already.
		if (engine->IsConnected())
		{
			QueryBox *qb = new QueryBox("#GameUI_TestSpeakersWarning_Title", "#GameUI_TestSpeakersWarning_Info" );
			if (qb != NULL)
			{
				qb->SetOKCommand(new KeyValues("RunTestSpeakers"));
				qb->SetOKButtonText("#GameUI_TestSpeakersWarning_OkButton");
				qb->SetCancelButtonText("#GameUI_TestSpeakersWarning_CancelButton");
				qb->AddActionSignalTarget( this );
				qb->DoModal();
			}
			else
			{
				// couldn't create the warning dialog for some reason, so just test the speakers.
				RunTestSpeakers();
			}
		}
		else
		{
			// player isn't connected to a game so there's no reason to warn them about being disconnected.
			// create the command to execute
			RunTestSpeakers();
		}
	}
   else if ( !stricmp( command, "ShowThirdPartyAudioCredits" ) )
   {
      OpenThirdPartySoundCreditsDialog();
   }

	BaseClass::OnCommand( command );
}

//-----------------------------------------------------------------------------
// Purpose: Run the test speakers map.
//-----------------------------------------------------------------------------
void COptionsSubAudio::RunTestSpeakers()
{
	engine->ClientCmd_Unrestricted( "disconnect\nwait\nwait\nsv_lan 1\nsetmaster enable\nmaxplayers 1\n\nhostname \"Speaker Test\"\nmap test_speakers\n" );
}

//-----------------------------------------------------------------------------
// Purpose: Open third party audio credits dialog
//-----------------------------------------------------------------------------
void COptionsSubAudio::OpenThirdPartySoundCreditsDialog()
{
   if (!m_OptionsSubAudioThirdPartyCreditsDlg.Get())
   {
      m_OptionsSubAudioThirdPartyCreditsDlg = new COptionsSubAudioThirdPartyCreditsDlg(GetVParent());
   }
   m_OptionsSubAudioThirdPartyCreditsDlg->Activate();
}


COptionsSubAudioThirdPartyCreditsDlg::COptionsSubAudioThirdPartyCreditsDlg( vgui::VPANEL hParent ) : BaseClass( NULL, NULL )
{
    SetTitle("#GameUI_ThirdPartyAudio_Title", true);
    SetSize(500, 200);
    LoadControlSettings("resource/optionssubaudiothirdpartydlg.res");
    MoveToCenterOfScreen();
    SetSizeable(false);
    SetDeleteSelfOnClose(true);
}

void COptionsSubAudioThirdPartyCreditsDlg::Activate()
{
	BaseClass::Activate();

	input()->SetAppModalSurface(GetVPanel());
}

void COptionsSubAudioThirdPartyCreditsDlg::OnKeyCodeTyped(vgui::KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}