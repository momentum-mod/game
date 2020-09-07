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
    m_cvarTimeScale("mom_replay_timescale"), m_pRunStats(nullptr), m_pRunEntData(nullptr), m_iLastZone(0), m_bAutoLayout(true)
{
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("zone_enter");
    ListenForGameEvent("player_jumped");
    ListenForGameEvent("ramp_leave");
    ListenForGameEvent("ramp_board");
    ListenForGameEvent("player_explosive_hit");
    
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_pAbsSpeedoLabel = new SpeedometerLabel(this, "AbsSpeedometer", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pAbsSpeedoLabel->SetSupportsEnergyUnits(true);
    m_pAbsSpeedoLabel->SetSupportsSeparateComparison(false);

    m_pHorizSpeedoLabel = new SpeedometerLabel(this, "HorizSpeedometer", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pHorizSpeedoLabel->SetSupportsSeparateComparison(false);

    m_pVertSpeedoLabel = new SpeedometerLabel(this, "VertSpeedometer", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pVertSpeedoLabel->SetSupportsSeparateComparison(false);

    m_pExplosiveJumpVelLabel = new SpeedometerLabel(this, "ExplosiveJumpVelocity", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pExplosiveJumpVelLabel->SetFadeOutAnimation("FadeOutExplosiveJumpVel", &m_fExplosiveJumpVelAlpha);

    m_pLastJumpVelLabel = new SpeedometerLabel(this, "LastJumpVelocity", SPEEDOMETER_COLORIZE_COMPARISON);
    m_pLastJumpVelLabel->SetFadeOutAnimation("FadeOutLastJumpVel", &m_fLastJumpVelAlpha);

    m_pRampBoardVelLabel = new SpeedometerLabel(this, "RampBoardVelocity", SPEEDOMETER_COLORIZE_NONE);
    m_pRampBoardVelLabel->SetFadeOutAnimation("FadeOutRampBoardVel", &m_fRampBoardVelAlpha);

    m_pRampLeaveVelLabel = new SpeedometerLabel(this, "RampLeaveVelocity", SPEEDOMETER_COLORIZE_NONE);
    m_pRampLeaveVelLabel->SetFadeOutAnimation("FadeOutRampLeaveVel", &m_fRampLeaveVelAlpha);

    m_pStageEnterExitVelLabel = new SpeedometerLabel(this, "StageEnterExitVelocity", SPEEDOMETER_COLORIZE_COMPARISON_SEPARATE);
    m_pStageEnterExitVelLabel->SetFadeOutAnimation("FadeOutStageVel", &m_fStageVelAlpha);

    m_Labels[0] = m_pAbsSpeedoLabel;
    m_Labels[1] = m_pHorizSpeedoLabel;
    m_Labels[2] = m_pVertSpeedoLabel;
    m_Labels[3] = m_pExplosiveJumpVelLabel;
    m_Labels[4] = m_pLastJumpVelLabel;
    m_Labels[5] = m_pRampBoardVelLabel;
    m_Labels[6] = m_pRampLeaveVelLabel;
    m_Labels[7] = m_pStageEnterExitVelLabel;

    ResetLabelOrder();

    LoadControlSettings(GetResFile());
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
        
    if (FStrEq(pEvent->GetName(), "player_explosive_hit"))
    {
        m_pExplosiveJumpVelLabel->Update(pEvent->GetFloat("speed"));
        return;
    }

    if (FStrEq(pEvent->GetName(), "player_jumped"))
    {
        m_pLastJumpVelLabel->Update(m_pRunEntData->m_flLastJumpVel);
        return;
    }

    if (FStrEq(pEvent->GetName(), "ramp_board"))
    {
        m_pRampBoardVelLabel->Update(pEvent->GetFloat("speed"));
        return;
    }

    if (FStrEq(pEvent->GetName(), "ramp_leave"))
    {
        m_pRampLeaveVelLabel->Update(pEvent->GetFloat("speed"));
        return;
    }

    if (!m_pRunStats)
        return;

    // zone enter/exit
    if (pEvent->GetInt("ent") != pLocal->GetCurrentUIEntity()->GetEntIndex())
        return;

    int iCurrentZone = pEvent->GetInt("num");
    const bool bExit = FStrEq(pEvent->GetName(), "zone_exit"), bEnter = FStrEq(pEvent->GetName(), "zone_enter");

    if (bEnter && iCurrentZone == 1)
    {
        // disappear when entering start zone
        m_fExplosiveJumpVelAlpha = 0.0f;
        m_fLastJumpVelAlpha = 0.0f;
        m_fStageVelAlpha = 0.0f;
        m_fRampBoardVelAlpha = 0.0f;
        m_fRampLeaveVelAlpha = 0.0f;
        m_iLastZone = 0;
    }

    const auto bLinear = pLocal->m_iLinearTracks.Get(pLocal->m_Data.m_iCurrentTrack);

    // return on current or previous zone on a linear map
    if (!m_pRunEntData->m_bTimerRunning || (iCurrentZone <= m_iLastZone && bLinear))
        return;

    const auto bLinearStartExit = bLinear && bExit && iCurrentZone == 1;
    // Logical XOR; shown on enter for linear maps, exits for staged maps
    if (bLinear != bExit || bLinearStartExit) 
    {
        m_iLastZone = iCurrentZone;

        int velType = mom_hud_velocity_type.GetInt();
        float act = m_pRunStats->GetZoneEnterSpeed(iCurrentZone, velType);
        bool bComparisonLoaded = g_pMOMRunCompare->LoadedComparison();
        if (bComparisonLoaded)
        {
            // set the label's custom diff
            float diff = act - g_pMOMRunCompare->GetRunComparisons()->runStats.GetZoneEnterSpeed(iCurrentZone, velType);
            m_pStageEnterExitVelLabel->SetCustomDiff(diff);
        }
        m_pStageEnterExitVelLabel->SetDrawComparison(bComparisonLoaded);
        m_pStageEnterExitVelLabel->Update(act);
    }
}

void CHudSpeedMeter::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(m_bgColor);
}

void CHudSpeedMeter::OnReloadControls()
{
    // no need to call baseclass as controls are reloaded when loading speedo data
    g_pSpeedometerData->LoadGamemodeData(g_pSpeedometerData->GetCurrentlyLoadedGameMode());
}

void CHudSpeedMeter::PerformLayout()
{
    EditablePanel::PerformLayout();

    if (!m_bAutoLayout)
        return;

    int iHeightAcc = 0;
    for (auto i = 0; i < m_LabelOrderList.Count(); i++)
    {
        SpeedometerLabel *pLabel = m_LabelOrderList[i];
        if (pLabel->IsVisible())
        {
            pLabel->SetYPos(iHeightAcc);
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
    float vertVel = Vector(0, 0, velocity.z).Length();

    m_pAbsSpeedoLabel->Update(absVel);
    m_pHorizSpeedoLabel->Update(horizVel);
    m_pVertSpeedoLabel->Update(vertVel);
}

void CHudSpeedMeter::ResetLabelOrder()
{
    m_LabelOrderList.RemoveAll();
    for (int i = 0; i < SPEEDOMETER_MAX_LABELS; i++)
    {
        m_LabelOrderList.AddToTail(m_Labels[i]);
    }
}
