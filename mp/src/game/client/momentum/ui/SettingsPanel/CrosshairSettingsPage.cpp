#include "cbase.h"

#include <ienginevgui.h>
#include "CrosshairSettingsPage.h"
#include "clientmode.h"
#include "CrosshairSettingsPreview.h"
#include "vgui/ISurface.h"

#include <vgui_controls/AnimationController.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Tooltip.h>

#include <tier0/memdbgon.h>

using namespace vgui;

CrosshairSettingsPage::CrosshairSettingsPage(Panel *pParent) : BaseClass(pParent, "CrosshairSettings")
{
    //add settings

    m_pCrosshairPreviewFrame = nullptr;
    m_pCrosshairPreviewPanel = nullptr;

    LoadControlSettings("resource/ui/SettingsPanel_ComparisonsSettings.res");
}

CrosshairSettingsPage::~CrosshairSettingsPage() {}

void CrosshairSettingsPage::DestroyBogusCrosshairPanel()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->DeletePanel();

    m_pCrosshairPreviewFrame = nullptr;
    m_pCrosshairPreviewPanel = nullptr;
}

void CrosshairSettingsPage::InitBogusCrosshairPanel()
{
    // Initialize the frame we're putting this into
    m_pCrosshairPreviewFrame = new Frame(nullptr, "CrosshairPreviewFrame");
    m_pCrosshairPreviewFrame->SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pCrosshairPreviewFrame->SetSize(350, scheme()->GetProportionalScaledValue(275));
    m_pCrosshairPreviewFrame->SetMoveable(false);
    m_pCrosshairPreviewFrame->MoveToFront();
    m_pCrosshairPreviewFrame->SetSizeable(false);
    m_pCrosshairPreviewFrame->SetTitle("#MOM_Settings_Compare_Bogus_Run", false);
    m_pCrosshairPreviewFrame->SetTitleBarVisible(true);
    m_pCrosshairPreviewFrame->SetMenuButtonResponsive(false);
    m_pCrosshairPreviewFrame->SetCloseButtonVisible(false);
    m_pCrosshairPreviewFrame->SetMinimizeButtonVisible(false);
    m_pCrosshairPreviewFrame->SetMaximizeButtonVisible(false);
    m_pCrosshairPreviewFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);
    m_pCrosshairPreviewFrame->InvalidateLayout(true);

    m_pCrosshairPreviewPanel = new C_CrosshairPreview("CrosshairPreviewPanel", m_pCrosshairPreviewFrame);
    m_pCrosshairPreviewPanel->AddActionSignalTarget(this);
    m_pCrosshairPreviewPanel->SetPaintBackgroundEnabled(true);
    m_pCrosshairPreviewPanel->SetPaintBackgroundType(2);
    //m_pCrosshairPreviewPanel->Init();
    int x, y, wid, tal;
    m_pCrosshairPreviewFrame->GetClientArea(x, y, wid, tal);
    m_pCrosshairPreviewPanel->SetBounds(x, y, wid, tal);
    //m_pCrosshairPreviewPanel->LoadBogusComparisons();
    m_pCrosshairPreviewPanel->SetScheme(scheme()->GetScheme("ClientScheme"));
    m_pCrosshairPreviewPanel->SetVisible(true);
    m_pCrosshairPreviewPanel->MakeReadyForUse();

    // Finally, set the frame visible (after the bogus panel is loaded and dandy)
    m_pCrosshairPreviewFrame->SetVisible(true);
}

void CrosshairSettingsPage::OnMainDialogClosed()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->Close();
}

void CrosshairSettingsPage::OnMainDialogShow()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->SetVisible(true);
}

void CrosshairSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();
    //don't know if we need this
}

void CrosshairSettingsPage::OnScreenSizeChanged(int oldwide, int oldtall)
{
    BaseClass::OnScreenSizeChanged(oldwide, oldtall);

    DestroyBogusCrosshairPanel();
}

void CrosshairSettingsPage::LoadSettings()
{
    //don't know if we need this
}

void CrosshairSettingsPage::OnPageShow()
{
    BaseClass::OnPageShow();

    if (!m_pCrosshairPreviewFrame)
        InitBogusCrosshairPanel();
    else if (!m_pCrosshairPreviewFrame->IsVisible())
        m_pCrosshairPreviewFrame->Activate();
}

void CrosshairSettingsPage::OnPageHide()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->Close();
}

void CrosshairSettingsPage::OnCheckboxChecked(Panel *p)
{
    BaseClass::OnCheckboxChecked(p);

    //update bools
}

void CrosshairSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);
    //update crosshair style or file
}

void CrosshairSettingsPage::OnComparisonResize(int wide, int tall)
{
    int scaledPad = scheme()->GetProportionalScaledValue(15);
    m_pCrosshairPreviewFrame->SetSize(wide + scaledPad, tall + float(scaledPad) * 1.5f);
    m_pCrosshairPreviewPanel->SetPos(m_pCrosshairPreviewFrame->GetXPos() + scaledPad / 2,
                                     m_pCrosshairPreviewFrame->GetYPos() + scaledPad);
}