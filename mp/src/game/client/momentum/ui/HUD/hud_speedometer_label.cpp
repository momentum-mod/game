#include "cbase.h"

#include "hud_speedometer_label.h"

#include "iclientmode.h"
#include "momentum/util/mom_util.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

using namespace vgui;

SpeedometerLabel::SpeedometerLabel(Panel* parent, const char* panelName): Label(parent, panelName, "")
{
    Reset();
}

void SpeedometerLabel::SetVisible(bool bVisible)
{
    m_pComparisonLabel->SetVisible(bVisible);
    BaseClass::SetVisible(bVisible);
    // parent's layout depends on the visiblity of this, so invalidate it
    GetParent()->InvalidateLayout();
}

void SpeedometerLabel::Reset()
{
    m_flCurrentValue = 0.0f;
    m_flPastValue = 0.0f;
    m_flDiff = 0.0f;
    BaseClass::SetText("");
}

void SpeedometerLabel::SetText(int value)
{
    char szValue[BUFSIZELOCL];
    Q_snprintf(szValue, sizeof(szValue), "%i", value);
    BaseClass::SetText(szValue);
}

void SpeedometerLabel::Update(float value)
{
    m_flCurrentValue = value;
    SetText(m_flCurrentValue);

    m_flPastValue = m_flCurrentValue;
}
