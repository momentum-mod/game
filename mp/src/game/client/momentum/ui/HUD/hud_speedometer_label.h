#pragma once

#include "hudelement.h"
#include "c_mom_replay_entity.h"
#include "mom_player_shared.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>

// Label update types
enum SpeedometerLabelUpdate_t
{
    // Don't update velocity/color, just apply fadeout. Useful for event based speedos.
    SPEEDOMETER_LABEL_UPDATE_ONLYFADE = 0,   
    // Only update velocity/color when the velocity differs from the previously calculated velocity.
    // Useful for speedos with values that don't update constantly (eg. last jump velocity)
    SPEEDOMETER_LABEL_UPDATE_ON_DIFFVELONLY,
    // Update velocity/color every tick.
    SPEEDOMETER_LABEL_UPDATE_ON_SAMEVEL
};

class SpeedometerLabel : public vgui::Label
{
    DECLARE_CLASS_SIMPLE(SpeedometerLabel, vgui::Label);

  public:
    // constructor for sending in velocity/colorize functions
    SpeedometerLabel(vgui::Panel *parent, const char *panelName, ConVar *CvarLabelEnabled, 
                     SpeedometerLabelUpdate_t updateType, float (*funcCalcVel)(C_MomentumPlayer *pPlayer) = nullptr,
                     void (*funcColorize)(Color &currentColor, Color lastColor, float currentVel, float lastVel,
                                          Color normalColor, Color increaseColor, Color decreaseColor) = nullptr);
    //SpeedometerLabel(Panel *parent, const char *panelName, const wchar_t *wszText);

    void SetEnabled(bool bEnabled) OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void OnThink() OVERRIDE;

    void Reset();
    void SetFadeOutAnimation(const char *animationName, float *animationAlpha)
    {
        m_strAnimationName = animationName;
        m_flAlpha = animationAlpha;
    }
    bool StartFade();
    bool StopFade();
    void ApplyFade();

  private:
    float (*CalcVelocity)(C_MomentumPlayer *pPlayer);
    void (*ColorizeOverride)(Color &currentColor, Color lastColor, float currentVel, float lastVel,
                             Color normalColor, Color increaseColor, Color decreaseColor);

    void Colorize();
    void SetLabelText();

    SpeedometerLabelUpdate_t m_eUpdateType;

    Color m_LastColor, m_CurrentColor, m_NormalColor, m_IncreaseColor, m_DecreaseColor,
        m_MaxVelColorLevel1, m_MaxVelColorLevel2, m_MaxVelColorLevel3, m_MaxVelColorLevel4, m_MaxVelColorLevel5;

    int m_DefaultHeight;

    float m_flNextColorizeCheck, m_flCurrentVelocity, m_flLastVelocity;

    // fadeout related variables
    const char *m_strAnimationName;
    float *m_flAlpha;
    
    ConVar *m_cvarIsEnabled;
    ConVarRef m_cvarMaxVel, m_cvarSpeedoUnits, m_cvarSpeedoColorize;
};
