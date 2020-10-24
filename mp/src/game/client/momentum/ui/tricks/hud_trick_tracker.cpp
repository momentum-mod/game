#include "cbase.h"

#include "hud_trick_tracker.h"

#include "clientmode.h"
#include "c_mom_player.h"
#include "mom_system_gamemode.h"
#include "mom_system_tricks.h"
#include "c_mom_triggers.h"

#include "vgui/ILocalize.h"

#include "tier0/memdbgon.h"

using namespace vgui;

TrickStepLabel::TrickStepLabel(Panel* pParent, const char* pszStepName) : Label(pParent, pszStepName, pszStepName)
{
    m_bCompleted = false;

    SetPaintBackgroundEnabled(true);
    SetContentAlignment(a_center);

    MakeReadyForUse();
}

void TrickStepLabel::PerformLayout()
{
    BaseClass::PerformLayout();

    SetTextInset(GetScaledVal(8), 0);

    int cWide, cTall;
    GetContentSize(cWide, cTall);

    SetSize(cWide + GetScaledVal(8), cTall + GetScaledVal(4));
}

void TrickStepLabel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cCompletedColor = GetSchemeColor("MomentumBlue", pScheme);
    m_cDefaultColor = GetSchemeColor("White", pScheme);

    SetBgColor(GetSchemeColor("BlackHO", pScheme));
}

void TrickStepLabel::OnThink()
{
    BaseClass::OnThink();

    SetFgColor(m_bCompleted ? m_cCompletedColor : m_cDefaultColor);
}


DECLARE_HUDELEMENT(TrickTrackerHUD);

TrickTrackerHUD::TrickTrackerHUD(const char* pElementName) : CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), pElementName)
{
    SetProportional(true);

    m_iTrackedTrick = -1;
    m_pPathNameLabel = new Label(this, "PathNameLabel", "Path for: <trick>");

    LoadControlSettings("resource/ui/tricks/TrickTracker.res");

    ListenForGameEvent("tricks_tracking");
}

void TrickTrackerHUD::Reset()
{
    if (m_iTrackedTrick > -1)
    {
        PopulateTrickStepLabels();
        UpdateTrickStepLabels();
    }
    else
    {
        ClearTrickStepLabels();
        m_iTrackedTrick = -1;
        m_pPathNameLabel->SetText("");
    }
}

void TrickTrackerHUD::FireGameEvent(IGameEvent* pEvent)
{
    const auto iType = pEvent->GetInt("type", -1);
    if (iType == TRICK_TRACK_UPDATE_TRICK)
    {
        m_iTrackedTrick = pEvent->GetInt("num", -1);

        if (m_iTrackedTrick > -1)
        {
            PopulateTrickStepLabels();
        }
        else
        {
            Reset();
        }
    }
    else if (iType == TRICK_TRACK_UPDATE_STEP)
    {
        UpdateTrickStepLabels();
    }
}

void TrickTrackerHUD::PerformLayout()
{
    BaseClass::PerformLayout();

    const auto y = m_pPathNameLabel->GetTall() + GetScaledVal(4);

    int x = 0;
    FOR_EACH_VEC(m_vecTrickSteps, i)
    {
        const auto pLabel = m_vecTrickSteps[i];

        pLabel->SetPos(x, y);

        x += pLabel->GetWide() + GetScaledVal(4);
    }
}

bool TrickTrackerHUD::ShouldDraw()
{
    return m_iTrackedTrick > -1 && g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF) && CHudElement::ShouldDraw();
}

void TrickTrackerHUD::LevelShutdown()
{
    m_iTrackedTrick = -1;
    Reset();
}

void TrickTrackerHUD::PopulateTrickStepLabels()
{
    ClearTrickStepLabels();

    const auto pTrick = g_pTrickSystem->GetTrickByID(m_iTrackedTrick);
    if (!pTrick)
    {
        Warning("TrickTrackerHUD::PopulateTrickStepLabels failed to load trick with ID %i!\n", m_iTrackedTrick);
        m_iTrackedTrick = -1;
        return;
    }

    // Populate the trick name label first
    KeyValuesAD trickName("TrickName");
    trickName->SetString("name", pTrick->GetName());
    m_pPathNameLabel->SetText(CConstructLocalizedString(g_pVGuiLocalize->FindSafe("#MOM_Trick_Path"), static_cast<KeyValues*>(trickName)));

    const auto iStepCount = pTrick->StepCount();
    for (int i = 0; i < iStepCount; i++)
    {
        const auto pStep = pTrick->Step(i);

        const auto pStepLabel = new TrickStepLabel(this, pStep->GetTrigger()->m_szZoneName.Get());
        m_vecTrickSteps.AddToTail(pStepLabel);
    }

    InvalidateLayout(true);
}

void TrickTrackerHUD::UpdateTrickStepLabels()
{
    if (m_vecTrickSteps.IsEmpty())
        return;

    const auto pPlayer = CMomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return;

    const auto iCurrentStep = pPlayer->m_Data.m_iCurrentZone;

    FOR_EACH_VEC(m_vecTrickSteps, i)
    {
        const auto pLabel = m_vecTrickSteps[i];

        pLabel->SetCompleted(i <= iCurrentStep);
    }

    InvalidateLayout(true);
}

void TrickTrackerHUD::ClearTrickStepLabels()
{
    FOR_EACH_VEC(m_vecTrickSteps, i)
    {
        m_vecTrickSteps[i]->DeletePanel();
    }

    m_vecTrickSteps.RemoveAll();
}