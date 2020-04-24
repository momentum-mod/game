#include "cbase.h"

#include "hud_speedometer.h"

#include "baseviewport.h"
#include "hud_comparisons.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>

#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

#include "c_baseplayer.h"
#include "movevars_shared.h"
#include "run/run_compare.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_CONVAR(mom_hud_speedometer_units, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Changes the units of measurement of the speedometer.\n 1 = Units per second\n 2 = "
                   "Kilometers per hour\n 3 = Miles per hour\n 4 = Energy",
                   1, 4);

static MAKE_CONVAR(mom_hud_speedometer_colorize, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Toggles speedometer colorization. 0 = OFF, 1 = ON (Based on acceleration),"
                   " 2 = ON (Staged by relative velocity to max.)\n", 0, 2);

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_unit_labels, "0", FLAG_HUD_CVAR,
                          "Toggles showing the unit value labels (KM/H, MPH, Energy, UPS). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_showenterspeed, "1", FLAG_HUD_CVAR,
    "Toggles showing the stage/checkpoint enter speed (and comparison, if existent). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_horiz, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles displaying the speedometer. 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_speedometer_lastjumpvel, "1", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles showing player velocity at last jump (XY only). 0 = OFF, 1 = ON\n");

static MAKE_TOGGLE_CONVAR(mom_hud_velocity_type, "0", FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE,
                   "Toggles the velocity type used in comparisons and map finished dialog. 0 = ABSOLUTE, 1 = HORIZONTAL\n");

DECLARE_HUDELEMENT(CHudSpeedMeter);

CHudSpeedMeter::CHudSpeedMeter(const char *pElementName)
    : CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudSpeedMeter")
{
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("zone_enter");
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_pUnitsLabel = new Label(this, "UnitsLabel", "");

    m_pAbsSpeedoLabel = new SpeedometerLabel(this, "AbsSpeedoLabel", &mom_hud_speedometer, SPEEDOMETER_LABEL_UPDATE_ALWAYS, GetAbsVelocity);
    m_pHorizSpeedoLabel = new SpeedometerLabel(this, "HorizSpeedoLabel", &mom_hud_speedometer_horiz, SPEEDOMETER_LABEL_UPDATE_ALWAYS, GetHorizVelocity);

    m_pLastJumpVelLabel = new SpeedometerLabel(this, "LastJumpVelLabel", &mom_hud_speedometer_lastjumpvel, SPEEDOMETER_LABEL_UPDATE_ALWAYS,
                                               GetLastJumpVelocity, LastJumpVelColorizeOverride);
    m_pLastJumpVelLabel->SetFadeOutAnimation("FadeOutJumpSpeed", &m_flLastJumpVelAlpha);

    m_pStageEnterLabel = new SpeedometerLabel(this, "StageEnterExitLabel", &mom_hud_speedometer_showenterspeed, SPEEDOMETER_LABEL_UPDATE_ONLYFADE);
    m_pStageEnterLabel->SetFadeOutAnimation("FadeOutEnterSpeed", &m_flStageStartAlpha);
    m_pStageEnterComparisonLabel = new SpeedometerLabel(this, "StageEnterExitComparisonLabel", &mom_hud_speedometer_showenterspeed, SPEEDOMETER_LABEL_UPDATE_ONLYFADE);
    m_pStageEnterComparisonLabel->SetFadeOutAnimation("FadeOutEnterSpeed", &m_flStageStartAlpha);
    
    LoadControlSettings("resource/ui/Speedometer.res");
}

void CHudSpeedMeter::Init()
{
    m_flStageStartAlpha = 0.0f;
    m_flLastJumpVelAlpha = 0.0f;
}

void CHudSpeedMeter::Reset()
{
    m_flStageStartAlpha = 0.0f;
    m_flLastJumpVelAlpha = 0.0f;
    m_pAbsSpeedoLabel->Reset();
    m_pHorizSpeedoLabel->Reset();
    m_pLastJumpVelLabel->Reset();
    m_pStageEnterLabel->Reset();
    m_pStageEnterComparisonLabel->Reset();
}

void CHudSpeedMeter::FireGameEvent(IGameEvent *pEvent)
{
    C_MomentumPlayer *pLocal = C_MomentumPlayer::GetLocalMomPlayer();

    if (pLocal)
    {
        const auto ent = pEvent->GetInt("ent");
        if (ent == pLocal->GetCurrentUIEntity()->GetEntIndex())
        {
            const auto pRunEntData = pLocal->GetCurrentUIEntData();
            if (FStrEq(pEvent->GetName(), "zone_exit") && pRunEntData->m_iCurrentZone == 1)
            {
                // start fade when exiting start zone as well
                m_pStageEnterLabel->StartFade();
            }
            else if (FStrEq(pEvent->GetName(), "zone_enter"))
            {
                if (pRunEntData->m_iCurrentZone == 1)
                {
                    // reset alphas when run is restarted
                    m_flStageStartAlpha = 0.0f;
                    m_flLastJumpVelAlpha = 0.0f;
                }
                else
                {
                    // stop fade out & make visible
                    m_pStageEnterLabel->StartFade();
                }
            }
        }
    }
}

void CHudSpeedMeter::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
    SetBgColor(GetSchemeColor("Blank", pScheme));

    m_defaultAbsSpeedoLabelHeight = surface()->GetFontTall(m_pAbsSpeedoLabel->GetFont());
    m_defaultHorizSpeedoLabelHeight = surface()->GetFontTall(m_pHorizSpeedoLabel->GetFont());
    m_defaultLastJumpVelLabelHeight = surface()->GetFontTall(m_pLastJumpVelLabel->GetFont());
    m_pStageEnterComparisonLabel->SetFont(m_pStageEnterLabel->GetFont()); // need to have same font
}

void CHudSpeedMeter::PerformLayout()
{
    m_pAbsSpeedoLabel->SetPos(m_pAbsSpeedoLabel->GetXPos(), 0);
    m_pHorizSpeedoLabel->SetPos(m_pHorizSpeedoLabel->GetXPos(), 0);
    m_pLastJumpVelLabel->SetPos(m_pLastJumpVelLabel->GetXPos(), 0);
    m_pStageEnterLabel->SetPos(m_pStageEnterLabel->GetXPos(), 0);
    m_pStageEnterComparisonLabel->SetPos(m_pStageEnterComparisonLabel->GetXPos(), 0);

    int absSpeedoTall = mom_hud_speedometer.GetBool() ? m_defaultAbsSpeedoLabelHeight : 0,
        horizSpeedoTall = mom_hud_speedometer_horiz.GetBool() ? m_defaultHorizSpeedoLabelHeight : 0,
        lastJumpVelTall = mom_hud_speedometer_lastjumpvel.GetBool() ? m_defaultLastJumpVelLabelHeight : 0;

    m_pHorizSpeedoLabel->SetPos(m_pHorizSpeedoLabel->GetXPos(), absSpeedoTall);
    m_pLastJumpVelLabel->SetPos(m_pLastJumpVelLabel->GetXPos(), absSpeedoTall + horizSpeedoTall);
    m_pStageEnterLabel->SetPos(m_pStageEnterLabel->GetXPos(), absSpeedoTall + horizSpeedoTall + lastJumpVelTall);
}

void CHudSpeedMeter::OnThink()
{
    if (mom_hud_speedometer_unit_labels.GetBool() &&
        (mom_hud_speedometer.GetBool() || mom_hud_speedometer_horiz.GetBool() ||
         mom_hud_speedometer_lastjumpvel.GetBool() || mom_hud_speedometer_showenterspeed.GetBool()))
    {
        switch (mom_hud_speedometer_units.GetInt())
        {
        case SPEEDOMETER_UNITS_KMH:
            m_pUnitsLabel->SetText(L"KM/H");
            break;
        case SPEEDOMETER_UNITS_MPH:
            m_pUnitsLabel->SetText(L"MPH");
            break;
        case SPEEDOMETER_UNITS_ENERGY:
            m_pUnitsLabel->SetText(L"Energy");
            break;
        case SPEEDOMETER_UNITS_UPS:
        default:
            m_pUnitsLabel->SetText(L"UPS");
        }
    }
    else
    {
        m_pUnitsLabel->SetTall(0);
        m_pUnitsLabel->SetText(L"");
    }

    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return;
    // Set stage enter vel
    const auto pRunEntData = pPlayer->GetCurrentUIEntData();
    const auto pRunStats = pPlayer->GetCurrentUIEntStats();
    if (pRunEntData && pRunEntData->m_bTimerRunning)
    {
        char enterVelComparisonStr[BUFSIZELOCL], enterVelStr[BUFSIZELOCL];
        Color compareColor = Color(0, 0, 0, 0);

        g_pMOMRunCompare->GetComparisonString(VELOCITY_ENTER, pRunStats, pRunEntData->m_iCurrentZone, enterVelStr,
                                              enterVelComparisonStr, &compareColor, true);

        HFont labelFont = m_pStageEnterLabel->GetFont();
        int combinedLength = UTIL_ComputeStringWidth(labelFont, enterVelStr);
        if (g_pMOMRunCompare->LoadedComparison()) // comparison is available
        {
            m_pStageEnterComparisonLabel->SetFgColor(compareColor);
            m_pStageEnterComparisonLabel->SetText(enterVelComparisonStr);
            combinedLength += UTIL_ComputeStringWidth(labelFont, enterVelComparisonStr);
        }
        else
        {
            combinedLength -= UTIL_ComputeStringWidth(labelFont, " ");
        }
        int xOffset = (GetWide() - combinedLength) / 2;
        m_pStageEnterLabel->SetText(enterVelStr);
        m_pStageEnterLabel->SetPos(0 - xOffset, m_pStageEnterLabel->GetYPos());
    }
}

void CHudSpeedMeter::AdjustToReplayTimeScale(float *vel)
{
    static ConVarRef CvarTimeScale("mom_replay_timescale");
    float timescale = CvarTimeScale.GetFloat();
    if (timescale < 1.0f)
    {
        *vel /= timescale;
    }
}

void CHudSpeedMeter::AdjustToUnits(float *vel, C_MomentumPlayer *pPlayer)
{
    // Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
    switch (mom_hud_speedometer_units.GetInt())
    {
    case SPEEDOMETER_UNITS_KMH:
        *vel *= UPS_TO_KMH_FACTOR;
        break;
    case SPEEDOMETER_UNITS_MPH:
        *vel *= UPS_TO_MPH_FACTOR;
        break;
    case SPEEDOMETER_UNITS_ENERGY:
    {
        // Normalized units of energy
        const auto gravity = sv_gravity.GetFloat();
        *vel = (Square(*vel) / 2.0f + gravity * (pPlayer->GetLocalOrigin().z - pPlayer->GetCurrentUIEntData()->m_flLastJumpZPos)) / gravity;
        break;
    }
    case SPEEDOMETER_UNITS_UPS:
    default:
        break;
    }
}

bool CHudSpeedMeter::GetAbsVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext)
{
    Vector vecVelocity = pPlayer->GetAbsVelocity();
    *pVelocity = static_cast<float>(vecVelocity.Length());
    if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
    {
        AdjustToReplayTimeScale(pVelocity);
    }
    AdjustToUnits(pVelocity, pPlayer);
    return true;
}

bool CHudSpeedMeter::GetHorizVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext)
{
    Vector vecVelocity = pPlayer->GetAbsVelocity();
    vecVelocity.z = 0;
    *pVelocity = static_cast<float>(vecVelocity.Length());
    if (pPlayer->IsObserver() && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
    {
        AdjustToReplayTimeScale(pVelocity);
    }
    AdjustToUnits(pVelocity, pPlayer);
    return true;
}

bool CHudSpeedMeter::GetLastJumpVelocity(C_MomentumPlayer *pPlayer, float *pVelocity, float *pPrevVelocityInContext)
{
    const auto pRunEntData = pPlayer->GetRunEntData();
    float lastJumpVel = pRunEntData->m_flLastJumpVel;
    float lastJumpTime = pRunEntData->m_flLastJumpTime;

    bool shouldUpdate = *pPrevVelocityInContext != lastJumpTime; // new jump since last called
    if (shouldUpdate)
    {
        *pVelocity = lastJumpVel;
        *pPrevVelocityInContext = lastJumpTime;
        AdjustToUnits(pVelocity, pPlayer);
    }
    return shouldUpdate;
}

void CHudSpeedMeter::LastJumpVelColorizeOverride(Color &currentColor, float currentVel, float lastVel,
                                                 Color normalColor, Color increaseColor, Color decreaseColor)
{
    if (CloseEnough(currentVel, lastVel))
    {
        currentColor = normalColor;
    }
    else
    {
        currentColor = MomUtil::GetColorFromVariation(fabs(currentVel) - fabs(lastVel), 0.0f,
                                                      normalColor, increaseColor, decreaseColor);
    }
}
