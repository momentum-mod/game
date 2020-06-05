#include "cbase.h"

#include "hud_speedometer.h"
#include "hud_speedometer_label.h"
#include "hud_speedometer_data.h"

#include "baseviewport.h"
#include "hud_comparisons.h"
#include "iclientmode.h"

#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_velocity_type, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Toggles the velocity type used in comparisons and map finished dialog. 0 = ABSOLUTE, 1 = HORIZONTAL\n");

static CHudElement *Create_CHudSpeedMeter(void)
{
    auto pPanel = new CHudSpeedMeter("HudSpeedMeter");
    g_pSpeedometer = pPanel;
    return pPanel;
};
static CHudElementHelper g_CHudSpeedMeter_Helper(Create_CHudSpeedMeter, 50);

CHudSpeedMeter *g_pSpeedometer = nullptr;

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudSpeedMeter"), 
    m_cvarTimeScale("mom_replay_timescale"), m_pRunStats(nullptr), m_pRunEntData(nullptr), m_iLastZone(0)
{
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("player_jumped");
    
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_pAbsSpeedoLabel = new SpeedometerLabel(this, "AbsSpeedometer", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pAbsSpeedoLabel->SetSupportsEnergyUnits(true);
    m_pAbsSpeedoLabel->SetSupportsSeparateComparison(false);

    m_pHorizSpeedoLabel = new SpeedometerLabel(this, "HorizSpeedometer", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pHorizSpeedoLabel->SetSupportsSeparateComparison(false);

    m_pLastJumpVelLabel = new SpeedometerLabel(this, "LastJumpVelocity", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pLastJumpVelLabel->SetFadeOutAnimation("FadeOutLastJumpVel", &m_fLastJumpVelAlpha);

    m_pStageEnterExitVelLabel = new SpeedometerLabel(this, "StageEnterExitVelocity", SPEEDOMETER_COLORIZE_COMPARISON_SEPARATE);
    m_pStageEnterExitVelLabel->SetFadeOutAnimation("FadeOutStageVel", &m_fStageVelAlpha);

    m_Labels[0] = m_pAbsSpeedoLabel;
    m_Labels[1] = m_pHorizSpeedoLabel;
    m_Labels[2] = m_pLastJumpVelLabel;
    m_Labels[3] = m_pStageEnterExitVelLabel;

    ResetLabelOrder();

    LoadControlSettings("resource/ui/Speedometer.res");
}

void CHudSpeedMeter::Init()
{
    m_iLastZone = 0;
    m_pRunStats = nullptr;
    m_pRunEntData = nullptr;
}

void CHudSpeedMeter::Reset()
{
    m_iLastZone = 0;
    m_pRunStats = nullptr;
    m_pRunEntData = nullptr;
    g_pSpeedometerData->LoadGamemodeData();
}

void CHudSpeedMeter::FireGameEvent(IGameEvent *pEvent)
{
    C_MomentumPlayer *pLocal = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pLocal || !m_pRunEntData)
        return; 

    if (FStrEq(pEvent->GetName(), "player_jumped"))
    {
        m_pLastJumpVelLabel->Update(m_pRunEntData->m_flLastJumpVel);
        return;
    }

    if (!m_pRunStats)
        return;

    // zone enter/exit
    const auto ent = pEvent->GetInt("ent");
    if (ent == pLocal->GetCurrentUIEntity()->GetEntIndex())
    {
        const auto bExit = FStrEq(pEvent->GetName(), "zone_exit");
        const auto bLinear = pLocal->m_iLinearTracks.Get(pLocal->m_Data.m_iCurrentTrack);
        int iCurrentZone = m_pRunEntData->m_iCurrentZone;

        // Logical XOR; equivalent to (bLinear && !bExit) || (!bLinear && bExit)
        // if map is linear, only update if player progresses to a new zone further into the map
        if (m_pRunEntData->m_bTimerRunning && (bLinear != bExit) && (iCurrentZone > m_iLastZone || !bLinear))
        {
            m_iLastZone = iCurrentZone;

            int velType = mom_hud_velocity_type.GetInt();
            float act = m_pRunStats->GetZoneEnterSpeed(iCurrentZone, velType);
            bool bComparisonLoaded = g_pMOMRunCompare->LoadedComparison();
            if (bComparisonLoaded)
            {
                // set the label's custom diff
                float diff = act - g_pMOMRunCompare->GetRunComparisons()->runStats.GetZoneEnterSpeed(
                                        m_pRunEntData->m_iCurrentZone, velType);
                m_pStageEnterExitVelLabel->SetCustomDiff(diff);
            }
            m_pStageEnterExitVelLabel->SetDrawComparison(bComparisonLoaded);
            m_pStageEnterExitVelLabel->Update(act);
        }
        else if (m_pRunEntData->m_bIsInZone && iCurrentZone == 1 && FStrEq(pEvent->GetName(), "zone_enter"))
        {   // disappear when entering start zone
            m_fLastJumpVelAlpha = 0.0f;
            m_fStageVelAlpha = 0.0f;

            m_iLastZone = 0;
        }
    }
}

void CHudSpeedMeter::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(m_bgColor);
}

void CHudSpeedMeter::OnReloadControls()
{
    BaseClass::OnReloadControls();
    g_pSpeedometerData->LoadGamemodeData(g_pSpeedometerData->GetCurrentlyLoadedGameMode());
}

void CHudSpeedMeter::PerformLayout()
{
    EditablePanel::PerformLayout();

    int iHeightAcc = 0;
    for (auto i = 0; i < m_LabelOrderList.Count(); i++)
    {
        SpeedometerLabel *pLabel = m_LabelOrderList[i];
        if (pLabel->IsVisible())
        {
            pLabel->SetPos(pLabel->GetXPos(), iHeightAcc);
            iHeightAcc += pLabel->GetTall();
        }
    }
    SetTall(iHeightAcc);
}

void CHudSpeedMeter::OnThink()
{
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return;

    m_pRunEntData = pPlayer->GetCurrentUIEntData();
    m_pRunStats = pPlayer->GetCurrentUIEntStats();
    // Note: Velocity is also set to the player when watching first person
    Vector velocity = pPlayer->GetAbsVelocity();

    if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
    {
        const float fReplayTimeScale = m_cvarTimeScale.GetFloat();
        if (fReplayTimeScale < 1.0f)
        {
            velocity /= fReplayTimeScale;
        }
    }

    float absVel = velocity.Length();
    float horizVel = Vector(velocity.x, velocity.y, 0).Length();

    m_pAbsSpeedoLabel->Update(absVel);
    m_pHorizSpeedoLabel->Update(horizVel);
}

void CHudSpeedMeter::ResetLabelOrder()
{
    m_LabelOrderList.RemoveAll();
    for (int i = 0; i < SPEEDOMETER_MAX_LABELS; i++)
    {
        m_LabelOrderList.AddToTail(m_Labels[i]);
    }
}
