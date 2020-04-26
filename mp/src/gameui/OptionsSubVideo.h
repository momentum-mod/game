//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OPTIONS_SUB_VIDEO_H
#define OPTIONS_SUB_VIDEO_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/PropertyPage.h>
#include "EngineInterface.h"
#include "IGameUIFuncs.h"
#include "URLButton.h"
#include "vgui_controls/Frame.h"

//-----------------------------------------------------------------------------
// Purpose: Video Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubVideo : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COptionsSubVideo, vgui::PropertyPage );

public:
	COptionsSubVideo(vgui::Panel *parent);
	~COptionsSubVideo();

	virtual void OnResetData();
	virtual void PerformLayout();

	virtual bool RequiresRestart();

	MESSAGE_FUNC( OpenGammaDialog, "OpenGammaDialog" );
	static vgui::DHANDLE<class CGammaDialog> m_hGammaDialog;

private:
    void        SetCurrentResolutionComboItem();

    MESSAGE_FUNC( OnDataChanged, "ControlModified" );
	MESSAGE_FUNC_PTR_CHARPTR( OnTextChanged, "TextChanged", panel, text );
	MESSAGE_FUNC( OpenAdvanced, "OpenAdvanced" );
	MESSAGE_FUNC( LaunchBenchmark, "LaunchBenchmark" );

	void		PrepareResolutionList();

	int m_nSelectedMode; // -1 if we are running in a nonstandard mode

	vgui::ComboBox		*m_pMode;
	vgui::ComboBox		*m_pWindowed;
	vgui::ComboBox		*m_pAspectRatio;
	vgui::Button		*m_pGammaButton;
	vgui::Button		*m_pAdvanced;
	vgui::Button		*m_pBenchmark;

	vgui::DHANDLE<class COptionsSubVideoAdvancedDlg> m_hOptionsSubVideoAdvancedDlg;

	bool m_bRequireRestart;
   MESSAGE_FUNC( OpenThirdPartyVideoCreditsDialog, "OpenThirdPartyVideoCreditsDialog" );
   vgui::URLButton   *m_pThirdPartyCredits;
   vgui::DHANDLE<class COptionsSubVideoThirdPartyCreditsDlg> m_OptionsSubVideoThirdPartyCreditsDlg;
};

//-----------------------------------------------------------------------------
// Purpose: Gamma-adjust dialog
//-----------------------------------------------------------------------------
class CGammaDialog : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CGammaDialog, vgui::Frame);

  public:
    CGammaDialog(vgui::VPANEL hParent);

    MESSAGE_FUNC(OnOK, "OK");

    virtual void Activate();
    virtual void OnClose();
    void OnKeyCodeTyped(vgui::KeyCode code);

  private:
    vgui::CvarSlider *m_pGammaSlider;
    vgui::Label *m_pGammaLabel;
    vgui::CvarTextEntry *m_pGammaEntry;
    float m_flOriginalGamma;

    ConVarRef m_cvarGamma;
};

class COptionsSubVideoThirdPartyCreditsDlg : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( COptionsSubVideoThirdPartyCreditsDlg, vgui::Frame );
public:
	COptionsSubVideoThirdPartyCreditsDlg( vgui::VPANEL hParent );

	virtual void Activate();
	void OnKeyCodeTyped(vgui::KeyCode code);
};

struct AAMode_t
{
    int m_nNumSamples;
    int m_nQualityLevel;
};

//-----------------------------------------------------------------------------
// Purpose: advanced video settings dialog
//-----------------------------------------------------------------------------
class COptionsSubVideoAdvancedDlg : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(COptionsSubVideoAdvancedDlg, vgui::Frame);

  public:
    COptionsSubVideoAdvancedDlg(vgui::Panel *parent);

    void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    virtual void OnCommand(const char *command) OVERRIDE;
    virtual void Activate();
    virtual void ApplyChanges();
    virtual void OnResetData();

    bool RequiresRestart();
    void SetComboItemAsRecommended(vgui::ComboBox *combo, int iItem);
    int FindMSAAMode(int nAASamples, int nAAQuality);
    void MarkDefaultSettingsAsRecommended();
    void ApplyChangesToConVar(const char *pConVarName, int value);

    MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden") // called when the GameUI is hidden
    {
        Close();
    }
	MESSAGE_FUNC(OK_Confirmed, "OK_Confirmed");

  private: 
    bool m_bUseChanges;
    vgui::ComboBox *m_pModelDetail, *m_pTextureDetail, *m_pAntialiasingMode, *m_pFilteringMode;
    vgui::ComboBox *m_pShadowDetail, *m_pWaterDetail, *m_pVSync, *m_pShaderDetail;
    vgui::ComboBox *m_pColorCorrection;
    vgui::ComboBox *m_pMotionBlur;

    vgui::ComboBox *m_pMulticore;

    vgui::CvarSlider *m_pFOVSlider;

    int m_nNumAAModes;
    AAMode_t m_nAAModes[16];

    vgui::ComboBox *m_pTonemap, *m_pBloom;
};


#endif // OPTIONS_SUB_VIDEO_H