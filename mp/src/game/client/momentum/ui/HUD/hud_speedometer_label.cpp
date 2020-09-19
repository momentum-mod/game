#include "cbase.h"

#include "hud_speedometer_label.h"

#include <vgui_controls/AnimationController.h>

#include "hud_comparisons.h"
#include "iclientmode.h"
#include "momentum/util/mom_util.h"
#include "mom_player_shared.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

// 1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
#define UPS_TO_KMH_FACTOR 0.06858f
// 1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH
#define UPS_TO_MPH_FACTOR 0.04262f

// velocity has to change this much before it is colored as increase/decrease
#define COLORIZE_DEADZONE 2.0f

using namespace vgui;

extern ConVar sv_gravity;

SpeedometerLabel::SpeedometerLabel(Panel *parent, const char *panelName, SpeedometerColorize_t colorizeType)
    : DoubleLabel(parent, panelName, CFmtStr("%sComparison", panelName).Get()), m_eColorizeType(colorizeType),
      m_pflAlpha(nullptr), m_bSupportsEnergyUnits(false), m_bDoneFading(false),
      m_eUnitType(SPEEDOMETER_UNITS_UPS), m_bDrawComparison(true), m_bSupportsSeparateComparison(true)
{
    Reset();
}

void SpeedometerLabel::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible, bVisible && m_bDrawComparison);
    // parent's layout depends on the visibility of this, so invalidate it
    GetParent()->InvalidateLayout();
}

void SpeedometerLabel::SetCustomDiff(float diff)
{
    m_flDiff = diff;
    m_bCustomDiff = true;
}

void SpeedometerLabel::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    m_NormalColor = GetSchemeColor("MOM.Speedometer.Normal", pScheme);
    m_IncreaseColor = GetSchemeColor("MOM.Speedometer.Increase", pScheme);
    m_DecreaseColor = GetSchemeColor("MOM.Speedometer.Decrease", pScheme);
    Reset();
}

void SpeedometerLabel::OnThink()
{
    BaseClass::OnThink();

    if (!HasFadeOutAnimation() || m_bDoneFading)
        return;

    SetAlpha(*m_pflAlpha);

    if (CloseEnough(*m_pflAlpha, 0.0f, FLT_EPSILON))
    {
        m_bDoneFading = true;
        Reset();
    }
}

void SpeedometerLabel::Reset()
{
    m_flCurrentValue = 0.0f;
    m_flPastValue = 0.0f;
    m_flDiff = 0.0f;
    m_flNextUpdateCheck = 0.0f;

    m_bCustomDiff = false;
    m_bDrawComparison = true;
    m_bDoneFading = true;

    BaseClass::SetText("", "");

    SetAlpha(HasFadeOutAnimation() ? 0 : 255);
}

void SpeedometerLabel::SetText(int value)
{
    SetPrimaryText(CFmtStr("%i", value).Get());
}

void SpeedometerLabel::Update(float value)
{
    m_flCurrentValue = value;

    ConvertUnits();
    SetText(m_flCurrentValue);
    Colorize();
    m_bDoneFading = HasFadeOutAnimation() ? !StartFadeout() : false;
}

void SpeedometerLabel::SetUnitType(int type) 
{ 
    if (type >= SPEEDOMETER_UNITS_FIRST && type <= SPEEDOMETER_UNITS_LAST)
        SetUnitType(SpeedometerUnits_t(type));
}

void SpeedometerLabel::ConvertUnits()
{
    if (m_eUnitType == SPEEDOMETER_UNITS_UPS)
        return;

    switch (m_eUnitType)
    {
    case SPEEDOMETER_UNITS_KMH:
        m_flCurrentValue *= UPS_TO_KMH_FACTOR;
        m_flDiff *= UPS_TO_KMH_FACTOR;
        break;
    case SPEEDOMETER_UNITS_MPH:
        m_flCurrentValue *= UPS_TO_MPH_FACTOR;
        m_flDiff *= UPS_TO_MPH_FACTOR;
        break;
    case SPEEDOMETER_UNITS_ENERGY:
        // outlier as it requires more than just 1 velocity
        if (m_bSupportsEnergyUnits)
        {
            const auto gravity = sv_gravity.GetFloat();
            const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
            Vector velocity = pPlayer->GetAbsVelocity();
            const auto pEntData = pPlayer->GetCurrentUIEntData();
            if (!pEntData)
                break;

            m_flCurrentValue =
                (velocity.LengthSqr() / 2.0f + gravity * (pPlayer->GetLocalOrigin().z - pEntData->m_flLastJumpZPos)) /
                gravity;
        }
        break;
    default:
        break;
    }
}

void SpeedometerLabel::SetColorizeType(int type) 
{
    if (type >= SPEEDOMETER_COLORIZE_FIRST && type <= SPEEDOMETER_COLORIZE_LAST)
        SetColorizeType(SpeedometerColorize_t(type));
}

void SpeedometerLabel::Colorize()
{
    if (m_flNextUpdateCheck > gpGlobals->curtime)
        return;

    switch (m_eColorizeType)
    {
    case SPEEDOMETER_COLORIZE_RANGE:
        ColorizeRange();
        break;
    case SPEEDOMETER_COLORIZE_COMPARISON:
        ColorizeComparison();
        break;
    case SPEEDOMETER_COLORIZE_COMPARISON_SEPARATE:
        ColorizeComparisonSeparate();
        break;
    default:
        SetPrimaryFgColor(m_NormalColor);
        break;
    }

    m_flPastValue = m_flCurrentValue;
    m_flNextUpdateCheck = gpGlobals->curtime + MOM_COLORIZATION_CHECK_FREQUENCY;
}

void SpeedometerLabel::SetFadeOutAnimation(char *animationName, float *animationAlpha)
{
    Q_strncpy(m_pszAnimationName, animationName, sizeof(m_pszAnimationName));
    m_pflAlpha = animationAlpha;
}

bool SpeedometerLabel::StartFadeout()
{
    Assert(HasFadeOutAnimation());
    g_pClientMode->GetViewportAnimationController()->StopAnimationSequence(GetParent(), m_pszAnimationName);
    *m_pflAlpha = 255.0f;
    return g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(m_pszAnimationName);
}

void SpeedometerLabel::ColorizeRange()
{
    int roundedCurrentValue = RoundFloatToInt(m_flCurrentValue);
    Color newColor = m_NormalColor;
    FOR_EACH_VEC(m_vecRangeList, i)
    {
        Range_t range = m_vecRangeList[i];
        if (roundedCurrentValue >= range.min && roundedCurrentValue <= range.max)
        {
            newColor = range.color;
            break;
        }
    }
    SetPrimaryFgColor(newColor);
}

void SpeedometerLabel::ColorizeComparison()
{
    m_flDiff = m_flCurrentValue - m_flPastValue;
    SetPrimaryFgColor(MomUtil::GetColorFromVariation(m_flDiff, COLORIZE_DEADZONE, m_NormalColor, m_IncreaseColor, m_DecreaseColor));
}

void SpeedometerLabel::ColorizeComparisonSeparate()
{
    SetPrimaryFgColor(m_NormalColor);

    if (!m_bDrawComparison)
        return;

    if (!m_bCustomDiff) // calculate diff unless explicitly told not to
    {
        m_flDiff = m_flCurrentValue - m_flPastValue;
    }
    m_bCustomDiff = false;

    Color compareColor;
    g_pMOMRunCompare->GetDiffColor(m_flDiff, &compareColor, true);

    char diffChar = m_flDiff > 0.0f ? '+' : '-';

    SetSecondaryText(CFmtStr(" (%c %i)", diffChar, RoundFloatToInt(fabs(m_flDiff))).Get());
    SetSecondaryFgColor(compareColor);
}

void SpeedometerLabel::SaveToKV(KeyValues* pOut)
{
    pOut->Clear();
    pOut->SetBool("visible", IsVisible());
    pOut->SetInt("colorize", GetColorizeType());
    KeyValues *pRangesKV = pOut->FindKey("ranges", true);

    FOR_EACH_VEC(m_vecRangeList, i)
    {
        Range_t range = m_vecRangeList[i];
        KeyValues *rangeKV = pRangesKV->FindKey(CFmtStr("%i", i + 1).Get(), true);
        rangeKV->SetInt("min", range.min);
        rangeKV->SetInt("max", range.max);
        rangeKV->SetColor("color", range.color);
    }
    pOut->SetInt("units", GetUnitType());
}

void SpeedometerLabel::LoadFromKV(KeyValues* pIn)
{
    if (!pIn)
        return;

    SetVisible(pIn->GetBool("visible", false));
    if (!IsVisible())
        return;

    int colorize = pIn->GetInt("colorize", SPEEDOMETER_COLORIZE_NONE);
    if (colorize == SPEEDOMETER_COLORIZE_RANGE)
    {
        KeyValues *pRangesKV = pIn->FindKey("ranges");
        if (!pRangesKV)
            return;

        m_vecRangeList.RemoveAll();
        FOR_EACH_SUBKEY(pRangesKV, pRangeItem)
        {
            Range_t range;
            range.min = pRangeItem->GetInt("min", 0);
            range.max = pRangeItem->GetInt("max", 0);
            range.color = pRangeItem->GetColor("color");
            m_vecRangeList.AddToTail(range);
        }
    }
    SetColorizeType(colorize);
    SetUnitType(pIn->GetInt("units", SPEEDOMETER_UNITS_UPS));
}
