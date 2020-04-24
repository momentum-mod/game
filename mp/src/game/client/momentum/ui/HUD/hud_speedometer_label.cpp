#include "cbase.h"

#include "hud_speedometer_label.h"
#include "momentum/util/mom_util.h"
#include "iclientmode.h"

#include "tier0/memdbgon.h"

using namespace vgui;

SpeedometerLabel::SpeedometerLabel(Panel *parent, const char *panelName, ConVar *CvarLabelEnabled,
                                   SpeedometerLabelUpdate_t updateType, bool (*funcCalcVel)(C_MomentumPlayer *pPlayer, float *velocity, float *pPrevVelocityInContext),
                                   void (*funcColorize)(Color &currentColor, float currentVel, float lastVel, 
                                                        Color normalColor, Color increaseColor, Color decreaseColor))
    : Label(parent, panelName, ""), m_cvarIsEnabled(CvarLabelEnabled), m_cvarMaxVel("sv_maxvelocity"), 
      m_cvarSpeedoUnits("mom_hud_speedometer_units"), m_cvarSpeedoColorize("mom_hud_speedometer_colorize"), m_eUpdateType(updateType),
      GetVelocity(funcCalcVel), ColorizeOverride(funcColorize)
{
    m_flLastVelocity = 0.0f;
    m_flCurrentVelocity = 0.0f;
    m_flNextColorizeCheck = 0.0f;
    m_pPrevVelocityInContext = 0.0f;
    m_bFadedOut = false;
    m_strAnimationName = nullptr;
    m_flAlpha = nullptr;
}

void SpeedometerLabel::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    m_NormalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
    m_IncreaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
    m_DecreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    m_MaxVelColorLevel1 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level1", pScheme);
    m_MaxVelColorLevel2 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level2", pScheme);
    m_MaxVelColorLevel3 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level3", pScheme);
    m_MaxVelColorLevel4 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level4", pScheme);
    m_MaxVelColorLevel5 = GetSchemeColor("MOM.Speedometer.MaxVelColoring.Level5", pScheme);

    SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));

    m_DefaultHeight = GetTall();
}

void SpeedometerLabel::SetLabelText()
{
    char speedoValue[BUFSIZELOCL];
    Q_snprintf(speedoValue, BUFSIZELOCL, "%d", RoundFloatToInt(m_flCurrentVelocity));
    SetText(speedoValue);
}

void SpeedometerLabel::SetEnabled(bool bEnabled)
{
    SetAutoTall(bEnabled);
    if (!bEnabled)
    {
        SetTall(0);
        SetText("");
    }
    BaseClass::SetEnabled(bEnabled);
}

void SpeedometerLabel::OnThink()
{
    bool isCvarEnabled = m_cvarIsEnabled->GetBool();
    if ( (IsEnabled() && !isCvarEnabled) || (!IsEnabled() && isCvarEnabled) )
    {
        SetEnabled(isCvarEnabled);
        GetParent()->InvalidateLayout();
    }
    if (!IsEnabled())
        return;

    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (!pPlayer)
        return;

    if (m_eUpdateType != SPEEDOMETER_LABEL_UPDATE_ONLYFADE && GetVelocity)
    {
        float velocity;
        float *pVelocity = &velocity;
        bool bShouldUpdate = GetVelocity(pPlayer, pVelocity, &m_pPrevVelocityInContext);
        if (bShouldUpdate && pVelocity)
        {
            if (m_bFadedOut) // reset values when faded out
            {
                m_flLastVelocity = 0.0f;
                m_pPrevVelocityInContext = 0.0f;
                m_bFadedOut = false;
            }
            else
            {
                m_flLastVelocity = m_flCurrentVelocity;
            }
            m_flCurrentVelocity = *pVelocity;
            SetLabelText();
            if (m_cvarSpeedoColorize.GetInt() && m_flNextColorizeCheck <= gpGlobals->curtime)
            {
                if (ColorizeOverride) // custom colorize function
                {
                    ColorizeOverride(m_CurrentColor, m_flCurrentVelocity, m_flLastVelocity, m_NormalColor, m_IncreaseColor, m_DecreaseColor);
                }
                else
                {
                    Colorize();
                }
                SetFgColor(m_CurrentColor);
                m_flNextColorizeCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
            }
            StartFade();
        }
        float currentAlpha;
        m_bFadedOut = GetAlpha(&currentAlpha) && CloseEnough(currentAlpha, 0.0f);
    }
    // always apply fade if there is one
    ApplyFade();
}

void SpeedometerLabel::Reset()
{
    m_flLastVelocity = 0.0f;
    m_flCurrentVelocity = 0.0f;
    m_flNextColorizeCheck = 0.0f;
    m_pPrevVelocityInContext = 0.0f;
    SetAlpha(0.0f); // if label has a fadeout, this sets the alpha to 0
}

bool SpeedometerLabel::StartFade()
{
    if (m_strAnimationName && m_strAnimationName[0])
    {
        *m_flAlpha = 255.0f;
        return g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(m_strAnimationName);
    }
    return false;
}

bool SpeedometerLabel::StopFade()
{
    if (m_strAnimationName && m_strAnimationName[0])
    {
        *m_flAlpha = 255.0f;
        return g_pClientMode->GetViewportAnimationController()->StopAnimationSequence(GetParent(), m_strAnimationName);
    }
    return false;
}

void SpeedometerLabel::ApplyFade()
{
    if (m_flAlpha)
    {
        m_CurrentColor = GetFgColor();
        m_CurrentColor = Color(m_CurrentColor.r(), m_CurrentColor.g(), m_CurrentColor.b(), RoundFloatToInt(*m_flAlpha));
        SetFgColor(m_CurrentColor);
    }
}

bool SpeedometerLabel::GetAlpha(float *alpha)
{
    if (m_flAlpha)
    {
        *alpha = *m_flAlpha;
        return true;
    }
    return false;
}

bool SpeedometerLabel::SetAlpha(float alpha)
{
    if (m_flAlpha)
    {
        *m_flAlpha = alpha;
        return true;
    }
    return false;
}

void SpeedometerLabel::Colorize()
{
    switch (m_cvarSpeedoColorize.GetInt())
    {
    case 1: 
    {
        if (!CloseEnough(m_flLastVelocity, m_flCurrentVelocity))
        {
            float variation = 0.0f;
            const float deadzone = 2.0f;
            if (m_cvarSpeedoUnits.GetInt() == SPEEDOMETER_UNITS_ENERGY)
            {
                // For energy, if current value is larger than previous value then we've got an increase
                variation = m_flCurrentVelocity - m_flLastVelocity;
            }
            else
            {
                // Otherwise, if magnitude of value (ie. with abs) is larger then we've got an increase
                // Example: vel = -500, lastvel = -300 counts as an increase in value since magnitude of vel
                // > magnitude of lastvel
                variation = fabs(m_flCurrentVelocity) - fabs(m_flLastVelocity);
            }
            // Get colour from the variation in value
            // If variation > deadzone then color shows as increase
            // Otherwise if variation < deadzone then color shows as decrease
            m_CurrentColor = MomUtil::GetColorFromVariation(variation, deadzone, m_NormalColor, m_IncreaseColor, m_DecreaseColor);
        }
        else
        {
            m_CurrentColor = m_NormalColor;
        }
        break;
    }
    case 2:
    {
        const float maxvel = m_cvarMaxVel.GetFloat();
        // MOM_TODO allow player to define custom ranges
        switch (static_cast<int>(m_flCurrentVelocity / maxvel * 5.0f))
        {
        case 0:
        default:
            m_CurrentColor = m_MaxVelColorLevel1;
            break;
        case 1:
            m_CurrentColor = m_MaxVelColorLevel2;
            break;
        case 2:
            m_CurrentColor = m_MaxVelColorLevel3;
            break;
        case 3:
            m_CurrentColor = m_MaxVelColorLevel4;
            break;
        case 4:
            m_CurrentColor = m_MaxVelColorLevel5;
            break;
        }
        break;
    }
    case 0:
    default:
        break;
    } 
}
