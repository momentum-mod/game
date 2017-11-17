#include "cbase.h"

#include "ReplaysSettingsPage.h"
#include "ModelPanel.h"
#include "ienginevgui.h"
#include "util/mom_util.h"

ReplaysSettingsPage::ReplaysSettingsPage(Panel *pParent) : BaseClass(pParent, "ReplaysSettings"), ghost_color("mom_ghost_color"),
ghost_bodygroup("mom_ghost_bodygroup")
{
    // Outer frame for the model preview
    m_pModelPreviewFrame = new Frame(nullptr, "ModelPreviewFrame");
    m_pModelPreviewFrame->SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pModelPreviewFrame->SetSize(350, scheme()->GetProportionalScaledValue(275));
    m_pModelPreviewFrame->SetMoveable(false);
    m_pModelPreviewFrame->MoveToFront();
    m_pModelPreviewFrame->SetSizeable(false);
    m_pModelPreviewFrame->SetTitle("Preview", false); // MOM_TODO: Localize me
    m_pModelPreviewFrame->SetTitleBarVisible(true);
    m_pModelPreviewFrame->SetMenuButtonResponsive(false);
    m_pModelPreviewFrame->SetCloseButtonVisible(true);
    m_pModelPreviewFrame->SetMinimizeButtonVisible(false);
    m_pModelPreviewFrame->SetMaximizeButtonVisible(false);
    m_pModelPreviewFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);

    // Actual model preview
    m_pModelPreview = new CRenderPanel(m_pModelPreviewFrame, "ModelPreview");
    m_pModelPreview->AddActionSignalTarget(this);
    m_pModelPreview->SetPaintBackgroundEnabled(true);
    m_pModelPreview->SetPaintBackgroundType(2);
    m_pModelPreview->SetSize(200, 150);
    m_pModelPreview->SetPos(14, 30);
    const bool result = m_pModelPreview->LoadModel("models/player/player_shape_base.mdl");
    if (result)
        UpdateModelSettings();

    m_pModelPreview->SetVisible(true);
    m_pModelPreview->MakeReadyForUse();
}

ReplaysSettingsPage::~ReplaysSettingsPage()
{
}

void ReplaysSettingsPage::LoadSettings()
{

}

void ReplaysSettingsPage::OnPageShow()
{
    if (!m_pModelPreviewFrame->IsVisible())
        m_pModelPreviewFrame->Activate();
}

void ReplaysSettingsPage::OnPageHide()
{
    if (m_pModelPreviewFrame)
        m_pModelPreviewFrame->Close();
}

void ReplaysSettingsPage::OnMainDialogClosed()
{
    OnPageHide();
}

void ReplaysSettingsPage::OnMainDialogShow()
{
    OnPageShow();
}

void ReplaysSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);

    
}

void ReplaysSettingsPage::OnControlModified(Panel *p)
{
    BaseClass::OnControlModified(p);

    
}

void ReplaysSettingsPage::UpdateModelSettings()
{
    C_BaseFlex *pModel = m_pModelPreview->GetModel();
    if (!pModel)
        return;

    Color ghostRenderColor;
    if (g_pMomentumUtil->GetColorFromHex(ghost_color.GetString(), ghostRenderColor))
    {
        pModel->SetRenderColor(ghostRenderColor.a(), ghostRenderColor.g(), ghostRenderColor.b(), ghostRenderColor.a());
    }

    pModel->SetBodygroup(1, ghost_bodygroup.GetInt());
}
