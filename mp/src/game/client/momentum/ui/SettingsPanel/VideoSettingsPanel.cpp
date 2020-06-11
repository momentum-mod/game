#include "cbase.h"

#include "VideoSettingsPanel.h"

#include "vgui_controls/ComboBox.h"
#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/CvarTextEntry.h"

#include "vgui/ILocalize.h"
#include <materialsystem/materialsystem_config.h>
#include "IGameUIFuncs.h"

#include "tier0/memdbgon.h"

using namespace vgui;

struct RatioToAspectMode_t
{
    int anamorphic;
    float aspectRatio;
};
RatioToAspectMode_t g_RatioToAspectModes[] =
{
    {	0,		4.0f / 3.0f },
    {	1,		16.0f / 9.0f },
    {	2,		16.0f / 10.0f },
    {	2,		1.0f },
};

//-----------------------------------------------------------------------------
// Purpose: returns the aspect ratio mode number for the given resolution
//-----------------------------------------------------------------------------
int GetScreenAspectMode(int width, int height)
{
    float aspectRatio = (float)width / (float)height;

    // just find the closest ratio
    float closestAspectRatioDist = 99999.0f;
    int closestAnamorphic = 0;
    for (int i = 0; i < ARRAYSIZE(g_RatioToAspectModes); i++)
    {
        float dist = fabsf(g_RatioToAspectModes[i].aspectRatio - aspectRatio);
        if (dist < closestAspectRatioDist)
        {
            closestAspectRatioDist = dist;
            closestAnamorphic = g_RatioToAspectModes[i].anamorphic;
        }
    }

    return closestAnamorphic;
}

//-----------------------------------------------------------------------------
// Purpose: returns the string name of the specified resolution mode
//-----------------------------------------------------------------------------
void GetResolutionName(vmode_t *mode, char *sz, int sizeofsz)
{
    if (mode->width == 1280 && mode->height == 1024)
    {
        // LCD native monitor resolution gets special case
        Q_snprintf(sz, sizeofsz, "%i x %i (LCD)", mode->width, mode->height);
    }
    else
    {
        Q_snprintf(sz, sizeofsz, "%i x %i", mode->width, mode->height);
    }
}

VideoSettingsPanel::VideoSettingsPanel(Panel *pParent, Button *pAssociate) : BaseClass(pParent, "VideoPage", pAssociate), m_cvarPicmip("mat_picmip"), m_cvarForceaniso("mat_forceaniso"),
    m_cvarTrilinear("mat_trilinear"), m_cvarAntialias("mat_antialias"), m_cvarAAQuality("mat_aaquality"),
    m_cvarShadowRenderToTexture("r_shadowrendertotexture"), m_cvarFlashlightDepthTexture("r_flashlightdepthtexture"),
    m_cvarWaterForceExpensive("r_waterforceexpensive"), m_cvarWaterForceReflectEntities("r_waterforcereflectentities"),
    m_cvarVSync("mat_vsync"), m_cvarRootlod("r_rootlod"), m_cvarReduceFillrate("mat_reducefillrate"),
    m_cvarColorCorreciton("mat_colorcorrection"), m_cvarMotionBlur("mat_motion_blur_enabled"), m_cvarQueueMode("mat_queue_mode"),
    m_cvarDisableBloom("mat_disable_bloom"), m_cvarDynamicTonemapping("mat_dynamic_tonemapping"), m_cvarGamma("mat_monitorgamma")
{
    SetSize(20, 20);

    m_pMode = new ComboBox(this, "Resolution", 8, false);

    m_pAspectRatio = new ComboBox(this, "AspectRatio", 6, false);
    m_pAspectRatio->AddItem("#GameUI_AspectNormal", nullptr);
    m_pAspectRatio->AddItem("#GameUI_AspectWide16x9", nullptr);
    m_pAspectRatio->AddItem("#GameUI_AspectWide16x10", nullptr);

    m_pWindowed = new ComboBox(this, "DisplayModeCombo", 6, false);
    m_pWindowed->AddItem("#GameUI_Fullscreen", nullptr);
    m_pWindowed->AddItem("#GameUI_Windowed", nullptr);

    m_pFOVSlider = new CvarSlider(this, "FOVSlider", "fov_desired", 90.0f, 179.0f, 0, true);
    m_pFOVEntry = new CvarTextEntry(this, "FOVEntry", "fov_desired", 0);
    m_pFOVEntry->SetAllowNumericInputOnly(true);

    // =============== Advanced Settings
    m_pModelDetail = new ComboBox(this, "ModelDetail", 6, false);
    m_pModelDetail->AddItem("#gameui_low", nullptr);
    m_pModelDetail->AddItem("#gameui_medium", nullptr);
    m_pModelDetail->AddItem("#gameui_high", nullptr);

    m_pTextureDetail = new ComboBox(this, "TextureDetail", 6, false);
    m_pTextureDetail->AddItem("#gameui_low", nullptr);
    m_pTextureDetail->AddItem("#gameui_medium", nullptr);
    m_pTextureDetail->AddItem("#gameui_high", nullptr);
    m_pTextureDetail->AddItem("#gameui_ultra", nullptr);

    // Build list of MSAA and CSAA modes, based upon those which are supported by the device
    //
    // The modes that we've seen in the wild to date are as follows (in perf order, fastest to slowest)
    //
    //								2x	4x	6x	8x	16x	8x	16xQ
    //		Texture/Shader Samples	1	1	1	1	1	1	1
    //		Stored Color/Z Samples	2	4	6	4	4	8	8
    //		Coverage Samples		2	4	6	8	16	8	16
    //		MSAA or CSAA			M	M	M	C	C	M	C
    //
    //	The CSAA modes are nVidia only (added in the G80 generation of GPUs)
    //
    m_nNumAAModes = 0;
    m_pAntialiasingMode = new ComboBox(this, "AntialiasingMode", 10, false);
    m_pAntialiasingMode->AddItem("#GameUI_None", nullptr);
    m_nAAModes[m_nNumAAModes].m_nNumSamples = 1;
    m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
    m_nNumAAModes++;

    if (materials->SupportsMSAAMode(2))
    {
        m_pAntialiasingMode->AddItem("#GameUI_2X", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 2;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
        m_nNumAAModes++;
    }

    if (materials->SupportsMSAAMode(4))
    {
        m_pAntialiasingMode->AddItem("#GameUI_4X", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
        m_nNumAAModes++;
    }

    if (materials->SupportsMSAAMode(6))
    {
        m_pAntialiasingMode->AddItem("#GameUI_6X", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 6;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
        m_nNumAAModes++;
    }

    if (materials->SupportsCSAAMode(4, 2))							// nVidia CSAA			"8x"
    {
        m_pAntialiasingMode->AddItem("#GameUI_8X_CSAA", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
        m_nNumAAModes++;
    }

    if (materials->SupportsCSAAMode(4, 4))							// nVidia CSAA			"16x"
    {
        m_pAntialiasingMode->AddItem("#GameUI_16X_CSAA", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 4;
        m_nNumAAModes++;
    }

    if (materials->SupportsMSAAMode(8))
    {
        m_pAntialiasingMode->AddItem("#GameUI_8X", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
        m_nNumAAModes++;
    }

    if (materials->SupportsCSAAMode(8, 2))							// nVidia CSAA			"16xQ"
    {
        m_pAntialiasingMode->AddItem("#GameUI_16XQ_CSAA", nullptr);
        m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
        m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
        m_nNumAAModes++;
    }

    m_pFilteringMode = new ComboBox(this, "FilteringMode", 6, false);
    m_pFilteringMode->AddItem("#GameUI_Bilinear", nullptr);
    m_pFilteringMode->AddItem("#GameUI_Trilinear", nullptr);
    m_pFilteringMode->AddItem("#GameUI_Anisotropic2X", nullptr);
    m_pFilteringMode->AddItem("#GameUI_Anisotropic4X", nullptr);
    m_pFilteringMode->AddItem("#GameUI_Anisotropic8X", nullptr);
    m_pFilteringMode->AddItem("#GameUI_Anisotropic16X", nullptr);

    m_pShadowDetail = new ComboBox(this, "ShadowDetail", 6, false);
    m_pShadowDetail->AddItem("#gameui_low", nullptr);
    m_pShadowDetail->AddItem("#gameui_medium", nullptr);
    if (materials->SupportsShadowDepthTextures())
    {
        m_pShadowDetail->AddItem("#gameui_high", nullptr);
    }

    m_pBloom = new ComboBox(this, "Bloom", 2, false);
    m_pBloom->AddItem("#gameui_disabled", nullptr);
    m_pBloom->AddItem("#gameui_enabled", nullptr);

    m_pTonemap = new ComboBox(this, "Tonemap", 2, false);
    m_pTonemap->AddItem("#gameui_disabled", nullptr);
    m_pTonemap->AddItem("#gameui_enabled", nullptr);

    m_pWaterDetail = new ComboBox(this, "WaterDetail", 6, false);
    m_pWaterDetail->AddItem("#gameui_noreflections", nullptr);
    m_pWaterDetail->AddItem("#gameui_reflectonlyworld", nullptr);
    m_pWaterDetail->AddItem("#gameui_reflectall", nullptr);

    m_pVSync = new ComboBox(this, "VSync", 2, false);
    m_pVSync->AddItem("#gameui_disabled", nullptr);
    m_pVSync->AddItem("#gameui_enabled", nullptr);

    m_pShaderDetail = new ComboBox(this, "ShaderDetail", 6, false);
    m_pShaderDetail->AddItem("#gameui_low", nullptr);
    m_pShaderDetail->AddItem("#gameui_high", nullptr);

    m_pColorCorrection = new ComboBox(this, "ColorCorrection", 2, false);
    m_pColorCorrection->AddItem("#gameui_disabled", nullptr);
    m_pColorCorrection->AddItem("#gameui_enabled", nullptr);

    m_pMotionBlur = new ComboBox(this, "MotionBlur", 2, false);
    m_pMotionBlur->AddItem("#gameui_disabled", nullptr);
    m_pMotionBlur->AddItem("#gameui_enabled", nullptr);

    m_pMulticore = new ComboBox(this, "Multicore", 2, false);
    m_pMulticore->AddItem("#gameui_disabled", nullptr);
    m_pMulticore->AddItem("#gameui_enabled", nullptr);

    MarkDefaultSettingsAsRecommended();

    // =============== Gamma settings
    m_pGammaSlider = new CvarSlider(this, "GammaSlider", "mat_monitorgamma", 1, true);
    m_pGammaEntry = new CvarTextEntry(this, "GammaEntry", "mat_monitorgamma", 1);
    m_pGammaEntry->SetAllowNumericInputOnly(true);

    m_flOriginalGamma = m_cvarGamma.GetFloat();

    LoadControlSettings("resource/ui/settings/Settings_Video.res");

    // Moved down here so we can set the Drop down's 
    // menu state after the default (disabled) value is loaded
    PrepareResolutionList();
}

void VideoSettingsPanel::OnPageShow()
{
    const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

    // reset UI elements
    m_pWindowed->SilentActivateItem(config.Windowed() ? 1 : 0);


    int iAspectMode = GetScreenAspectMode(config.m_VideoMode.m_Width, config.m_VideoMode.m_Height);
    m_pAspectRatio->ActivateItem(iAspectMode);

    // reset gamma control
    m_pGammaEntry->SetEnabled(!config.Windowed());
    m_pGammaSlider->SetEnabled(!config.Windowed());

    SetCurrentResolutionComboItem();

    // Advanced settings
    m_pModelDetail->SilentActivateItem(2 - clamp(m_cvarRootlod.GetInt(), 0, 2));
    m_pTextureDetail->SilentActivateItem(2 - clamp(m_cvarPicmip.GetInt(), -1, 2));

    if (m_cvarFlashlightDepthTexture.GetBool()) // If we're doing flashlight shadow depth texturing...
    {
        m_cvarShadowRenderToTexture.SetValue(1); // ...be sure render to texture shadows are also on
        m_pShadowDetail->SilentActivateItem(2);
    }
    else if (m_cvarShadowRenderToTexture.GetBool()) // RTT shadows, but not shadow depth texturing
    {
        m_pShadowDetail->SilentActivateItem(1);
    }
    else	// Lowest shadow quality
    {
        m_pShadowDetail->SilentActivateItem(0);
    }

    m_pShaderDetail->SilentActivateItem(m_cvarReduceFillrate.GetBool() ? 0 : 1);

    switch (m_cvarForceaniso.GetInt())
    {
    case 2:
        m_pFilteringMode->SilentActivateItem(2);
        break;
    case 4:
        m_pFilteringMode->SilentActivateItem(3);
        break;
    case 8:
        m_pFilteringMode->SilentActivateItem(4);
        break;
    case 16:
        m_pFilteringMode->SilentActivateItem(5);
        break;
    case 0:
    default:
        if (m_cvarTrilinear.GetBool())
        {
            m_pFilteringMode->SilentActivateItem(1);
        }
        else
        {
            m_pFilteringMode->SilentActivateItem(0);
        }
        break;
    }

    // Map convar to item on AA drop-down
    int nAASamples = m_cvarAntialias.GetInt();
    int nAAQuality = m_cvarAAQuality.GetInt();
    int nMSAAMode = FindMSAAMode(nAASamples, nAAQuality);
    m_pAntialiasingMode->SilentActivateItem(nMSAAMode);

    if (m_cvarWaterForceExpensive.GetBool())
    {
        if (m_cvarWaterForceReflectEntities.GetBool())
        {
            m_pWaterDetail->SilentActivateItem(2);
        }
        else
        {
            m_pWaterDetail->SilentActivateItem(1);
        }
    }
    else
    {
        m_pWaterDetail->SilentActivateItem(0);
    }

    m_pVSync->SilentActivateItem(m_cvarVSync.GetInt());

    m_pColorCorrection->SilentActivateItem(m_cvarColorCorreciton.GetInt());

    m_pMotionBlur->SilentActivateItem(m_cvarMotionBlur.GetInt());

    m_pMulticore->SilentActivateItem(-m_cvarQueueMode.GetInt());

    // The cvar disables bloom so invert the item
    m_pBloom->SilentActivateItem(!m_cvarDisableBloom.GetInt());

    m_pTonemap->SilentActivateItem(m_cvarDynamicTonemapping.GetInt());
}

void VideoSettingsPanel::SetCurrentResolutionComboItem()
{
    vmode_t *plist = nullptr;
    int count = 0;
    gameuifuncs->GetVideoModes(&plist, &count);

    const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

    int resolution = -1;
    for (int i = 0; i < count; i++, plist++)
    {
        if (plist->width == config.m_VideoMode.m_Width &&
            plist->height == config.m_VideoMode.m_Height)
        {
            resolution = i;
            break;
        }
    }

    if (resolution != -1)
    {
        char sz[256];
        GetResolutionName(plist, sz, sizeof(sz));
        m_pMode->SetText(sz);
    }
}

void VideoSettingsPanel::PrepareResolutionList()
{
    // get the currently selected resolution
    char sz[256];
    m_pMode->GetText(sz, 256);
    int currentWidth = 0, currentHeight = 0;
    sscanf(sz, "%i x %i", &currentWidth, &currentHeight);

    // Clean up before filling the info again.
    m_pMode->DeleteAllItems();
    m_pAspectRatio->SetItemEnabled(1, false);
    m_pAspectRatio->SetItemEnabled(2, false);

    // get full video mode list
    vmode_t *plist = nullptr;
    int count = 0;
    gameuifuncs->GetVideoModes(&plist, &count);

    const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

    bool bWindowed = (m_pWindowed->GetCurrentItem() > 0);
    int desktopWidth, desktopHeight;
    gameuifuncs->GetDesktopResolution(desktopWidth, desktopHeight);

    // iterate all the video modes adding them to the dropdown
    bool bFoundWidescreen = false;
    int selectedItemID = -1;
    for (int i = 0; i < count; i++, plist++)
    {
        char resolution[256];
        GetResolutionName(plist, resolution, sizeof(resolution));

        // don't show modes bigger than the desktop for windowed mode
        if (bWindowed && (plist->width > desktopWidth || plist->height > desktopHeight))
            continue;

        int itemID = -1;
        int iAspectMode = GetScreenAspectMode(plist->width, plist->height);
        if (iAspectMode > 0)
        {
            m_pAspectRatio->SetItemEnabled(iAspectMode, true);
            bFoundWidescreen = true;
        }

        // filter the list for those matching the current aspect
        if (iAspectMode == m_pAspectRatio->GetCurrentItem())
        {
            itemID = m_pMode->AddItem(resolution, nullptr);
        }

        // try and find the best match for the resolution to be selected
        if (plist->width == currentWidth && plist->height == currentHeight)
        {
            selectedItemID = itemID;
        }
        else if (selectedItemID == -1 && plist->width == config.m_VideoMode.m_Width && plist->height == config.m_VideoMode.m_Height)
        {
            selectedItemID = itemID;
        }
    }

    // disable ratio selection if we can't display widescreen.
    m_pAspectRatio->SetEnabled(bFoundWidescreen);

    m_nSelectedMode = selectedItemID;

    if (selectedItemID != -1)
    {
        m_pMode->ActivateItem(selectedItemID);
    }
    else
    {
        char buf[256];
        sprintf(buf, "%d x %d", config.m_VideoMode.m_Width, config.m_VideoMode.m_Height);
        m_pMode->SetText(buf);
    }
}

void VideoSettingsPanel::OnTextChanged(Panel *pPanel, const char *pszText)
{
    if (pPanel == m_pMode)
    {
        const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

        m_nSelectedMode = m_pMode->GetCurrentItem();

        int w = 0, h = 0;
        sscanf(pszText, "%i x %i", &w, &h);
        if (config.m_VideoMode.m_Width != w || config.m_VideoMode.m_Height != h)
        {
            OnDataChanged();
        }
    }
    else if (pPanel == m_pAspectRatio)
    {
        PrepareResolutionList();
    }
    else if (pPanel == m_pWindowed)
    {
        PrepareResolutionList();
        OnDataChanged();
    }
    else
    {
        OnDataChanged();
    }
}

void VideoSettingsPanel::OnDataChanged()
{
    // apply advanced options
    ApplyAdvancedChanges();

    // resolution
    char sz[256];
    if (m_nSelectedMode == -1)
    {
        m_pMode->GetText(sz, 256);
    }
    else
    {
        m_pMode->GetItemText(m_nSelectedMode, sz, 256);
    }

    int width = 0, height = 0;
    sscanf(sz, "%i x %i", &width, &height);

    // windowed
    bool windowed = (m_pWindowed->GetCurrentItem() > 0) ? true : false;

    // make sure there is a change
    const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
    if (config.m_VideoMode.m_Width != width
        || config.m_VideoMode.m_Height != height
        || config.Windowed() != windowed)
    {
        // set mode
        char szCmd[256];
        Q_snprintf(szCmd, sizeof(szCmd), "mat_setvideomode %i %i %i\n", width, height, windowed ? 1 : 0);
        engine->ClientCmd_Unrestricted(szCmd);
    }

    // apply changes
    engine->ClientCmd_Unrestricted("mat_savechanges\n");
}

void VideoSettingsPanel::ApplyAdvancedChanges()
{
    m_cvarRootlod.SetValue(2 - m_pModelDetail->GetCurrentItem());
    m_cvarPicmip.SetValue(2 - m_pTextureDetail->GetCurrentItem());

    // reset everything tied to the filtering mode, then the switch sets the appropriate one
    m_cvarTrilinear.SetValue(false);
    m_cvarForceaniso.SetValue(1);
    switch (m_pFilteringMode->GetCurrentItem())
    {
    case 0:
    default:
        break;
    case 1:
        m_cvarTrilinear.SetValue(true);
        break;
    case 2:
        m_cvarForceaniso.SetValue(2);
        break;
    case 3:
        m_cvarForceaniso.SetValue(4);
        break;
    case 4:
        m_cvarForceaniso.SetValue(8);
        break;
    case 5:
        m_cvarForceaniso.SetValue(16);
        break;
    }

    // Set the AA convars according to the menu item chosen
    int nActiveAAItem = m_pAntialiasingMode->GetCurrentItem();
    m_cvarAntialias.SetValue(m_nAAModes[nActiveAAItem].m_nNumSamples);
    m_cvarAAQuality.SetValue(m_nAAModes[nActiveAAItem].m_nQualityLevel);

    if (m_pShadowDetail->GetCurrentItem() == 0) // Blobby shadows
    {
        m_cvarShadowRenderToTexture.SetValue(0);  // Turn off RTT shadows
        m_cvarFlashlightDepthTexture.SetValue(0); // Turn off shadow depth textures
    }
    else if (m_pShadowDetail->GetCurrentItem() == 1) // RTT shadows only
    {
        m_cvarShadowRenderToTexture.SetValue(1);  // Turn on RTT shadows
        m_cvarFlashlightDepthTexture.SetValue(0); // Turn off shadow depth textures
    }
    else if (m_pShadowDetail->GetCurrentItem() == 2) // Shadow depth textures
    {
        m_cvarShadowRenderToTexture.SetValue(1);  // Turn on RTT shadows
        m_cvarFlashlightDepthTexture.SetValue(1); // Turn on shadow depth textures
    }

    m_cvarReduceFillrate.SetValue((m_pShaderDetail->GetCurrentItem() > 0) ? 0 : 1);

    switch (m_pWaterDetail->GetCurrentItem())
    {
    default:
    case 0:
        m_cvarWaterForceExpensive.SetValue(false);
        m_cvarWaterForceReflectEntities.SetValue(false);
        break;
    case 1:
        m_cvarWaterForceExpensive.SetValue(true);
        m_cvarWaterForceReflectEntities.SetValue(false);
        break;
    case 2:
        m_cvarWaterForceExpensive.SetValue(true);
        m_cvarWaterForceReflectEntities.SetValue(true);
        break;
    }

    m_cvarVSync.SetValue(m_pVSync->GetCurrentItem());

    m_cvarColorCorreciton.SetValue(m_pColorCorrection->GetCurrentItem());

    m_cvarMotionBlur.SetValue(m_pMotionBlur->GetCurrentItem());

    m_cvarQueueMode.SetValue(-m_pMulticore->GetCurrentItem());

    // The cvar disables bloom so invert the item
    m_cvarDisableBloom.SetValue(!m_pBloom->GetCurrentItem());

    m_cvarDynamicTonemapping.SetValue(m_pTonemap->GetCurrentItem());
}

void VideoSettingsPanel::SetComboItemAsRecommended(ComboBox *combo, int iItem)
{
    // get the item text
    wchar_t text[512];
    combo->GetItemText(iItem, text, sizeof(text));

    // append the recommended flag
    wchar_t newText[512];
    V_snwprintf(newText, sizeof(newText) / sizeof(wchar_t), L"%ls *", text);

    // reset
    combo->UpdateItem(iItem, newText, nullptr);
}

int VideoSettingsPanel::FindMSAAMode(int nAASamples, int nAAQuality)
{
    // Run through the AA Modes supported by the device
    for (int nAAMode = 0; nAAMode < m_nNumAAModes; nAAMode++)
    {
        // If we found the mode that matches what we're looking for, return the index
        if ((m_nAAModes[nAAMode].m_nNumSamples == nAASamples) && (m_nAAModes[nAAMode].m_nQualityLevel == nAAQuality))
        {
            return nAAMode;
        }
    }

    return 0;	// Didn't find what we're looking for, so no AA
}

void VideoSettingsPanel::MarkDefaultSettingsAsRecommended()
{
    // Pull in data from dxsupport.cfg database (includes fine-grained per-vendor/per-device config data)
    KeyValuesAD pKeyValues("config");
    materials->GetRecommendedConfigurationInfo(0, pKeyValues);

    // Read individual values from keyvalues which came from dxsupport.cfg database
    int nSkipLevels = pKeyValues->GetInt("ConVar.mat_picmip", 0);
    int nAnisotropicLevel = pKeyValues->GetInt("ConVar.mat_forceaniso", 1);
    int nForceTrilinear = pKeyValues->GetInt("ConVar.mat_trilinear", 0);
    int nAASamples = pKeyValues->GetInt("ConVar.mat_antialias", 0);
    int nAAQuality = pKeyValues->GetInt("ConVar.mat_aaquality", 0);
    int nRenderToTextureShadows = pKeyValues->GetInt("ConVar.r_shadowrendertotexture", 0);
    int nShadowDepthTextureShadows = pKeyValues->GetInt("ConVar.r_flashlightdepthtexture", 0);
    int nWaterUseRealtimeReflection = pKeyValues->GetInt("ConVar.r_waterforceexpensive", 0);
    int nWaterUseEntityReflection = pKeyValues->GetInt("ConVar.r_waterforcereflectentities", 0);
    int nMatVSync = pKeyValues->GetInt("ConVar.mat_vsync", 1);
    int nRootLOD = pKeyValues->GetInt("ConVar.r_rootlod", 0);
    int nReduceFillRate = pKeyValues->GetInt("ConVar.mat_reducefillrate", 0);
    int nColorCorrection = pKeyValues->GetInt("ConVar.mat_colorcorrection", 0);
    int nMotionBlur = pKeyValues->GetInt("ConVar.mat_motion_blur_enabled", 0);
    int nMulticore = pKeyValues->GetInt("ConVar.mat_queue_mode", -1);

    SetComboItemAsRecommended(m_pModelDetail, 2 - nRootLOD);
    SetComboItemAsRecommended(m_pTextureDetail, 2 - nSkipLevels);

    switch (nAnisotropicLevel)
    {
    case 2:
        SetComboItemAsRecommended(m_pFilteringMode, 2);
        break;
    case 4:
        SetComboItemAsRecommended(m_pFilteringMode, 3);
        break;
    case 8:
        SetComboItemAsRecommended(m_pFilteringMode, 4);
        break;
    case 16:
        SetComboItemAsRecommended(m_pFilteringMode, 5);
        break;
    case 0:
    default:
        if (nForceTrilinear != 0)
        {
            SetComboItemAsRecommended(m_pFilteringMode, 1);
        }
        else
        {
            SetComboItemAsRecommended(m_pFilteringMode, 0);
        }
        break;
    }

    // Map desired mode to list item number
    int nMSAAMode = FindMSAAMode(nAASamples, nAAQuality);
    SetComboItemAsRecommended(m_pAntialiasingMode, nMSAAMode);

    if (nShadowDepthTextureShadows)
        SetComboItemAsRecommended(m_pShadowDetail, 2);	// Shadow depth mapping (in addition to RTT shadows)
    else if (nRenderToTextureShadows)
        SetComboItemAsRecommended(m_pShadowDetail, 1);	// RTT shadows
    else
        SetComboItemAsRecommended(m_pShadowDetail, 0);	// Blobbies

    SetComboItemAsRecommended(m_pShaderDetail, nReduceFillRate ? 0 : 1);

    if (nWaterUseRealtimeReflection)
    {
        if (nWaterUseEntityReflection)
        {
            SetComboItemAsRecommended(m_pWaterDetail, 2);
        }
        else
        {
            SetComboItemAsRecommended(m_pWaterDetail, 1);
        }
    }
    else
    {
        SetComboItemAsRecommended(m_pWaterDetail, 0);
    }

    SetComboItemAsRecommended(m_pVSync, nMatVSync != 0);

    SetComboItemAsRecommended(m_pColorCorrection, nColorCorrection);

    SetComboItemAsRecommended(m_pMotionBlur, nMotionBlur);

    SetComboItemAsRecommended(m_pMulticore, -nMulticore);
}