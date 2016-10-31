#include "cbase.h"

#include "GameplaySettingsPage.h"

GameplaySettingsPage::GameplaySettingsPage(Panel *pParent) : BaseClass(pParent, "GameplaySettings")
{
    m_pYawSpeedSlider = FindControl<CCvarSlider>("YawSpeed");
    m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");

    m_pPlayBlockSound = FindControl<CvarToggleCheckButton<ConVarRef>>("PlayBlockSound");
    m_pSaveCheckpoints = FindControl<CvarToggleCheckButton<ConVarRef>>("SaveCheckpoints");
    m_pEnableTrail = FindControl<CvarToggleCheckButton<ConVarRef>>("EnableTrail");

    m_pPickColorButton = FindControl<Button>("PickColorButton");
}

void GameplaySettingsPage::LoadSettings()
{
    UpdateSliderEntries();
}

void GameplaySettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);

    if (p == m_pYawSpeedEntry)
    {
        char buf[64];
        m_pYawSpeedEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 1.0)
        {
            m_pYawSpeedSlider->SetSliderValue(fValue);
        }
    } 
    // Can't reduce this code without further checks (Sliders are different)
    else if (p == m_pTrailColorREntry)
    {
        char buf[64];
        m_pTrailColorREntry->GetText(buf, 64);

        int colorValue = atoi(buf);
        if (colorValue >= 0 && colorValue < 256)
        {
            m_pTrailColorRSlider->SetSliderValue(colorValue);
            Color backColor = m_pSampleColor->GetBgColor();
            backColor = Color(colorValue, backColor.g(), backColor.b(), backColor.a());
            m_pSampleColor->SetBgColor(backColor);
        }
    }
    else if (p == m_pTrailColorGEntry)
    {
        char buf[64];
        m_pTrailColorGEntry->GetText(buf, 64);

        int colorValue = atoi(buf);
        if (colorValue >= 0 && colorValue < 256)
        {
            m_pTrailColorGSlider->SetSliderValue(colorValue);
            Color backColor = m_pSampleColor->GetBgColor();
            backColor = Color(backColor.r(), colorValue, backColor.b(), backColor.a());
            m_pSampleColor->SetBgColor(backColor);
        }
    }
    else if (p == m_pTrailColorBEntry)
    {
        char buf[64];
        m_pTrailColorBEntry->GetText(buf, 64);

        int colorValue = atoi(buf);
        if (colorValue >= 0 && colorValue < 256)
        {
            m_pTrailColorBSlider->SetSliderValue(colorValue);
            Color backColor = m_pSampleColor->GetBgColor();
            backColor = Color(backColor.r(), backColor.g(), colorValue, backColor.a());
            m_pSampleColor->SetBgColor(backColor);
        }
    }
    else if (p == m_pTrailColorAEntry)
    {
        char buf[64];
        m_pTrailColorAEntry->GetText(buf, 64);

        int alphaValue = Q_atoi(buf);
        if (alphaValue >= 0 && alphaValue < 256)
        {
            m_pTrailColorASlider->SetSliderValue(alphaValue);
            Color backColor = m_pSampleColor->GetBgColor();
            backColor = Color(backColor.r(), backColor.g(), backColor.b(), alphaValue);
            m_pSampleColor->SetBgColor(backColor);
        }
    }
}

void GameplaySettingsPage::OnControlModified(Panel *p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pYawSpeedSlider && m_pYawSpeedSlider->HasBeenModified())
    {
        UpdateSliderEntries();
    }
}

void GameplaySettingsPage::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");

    m_pPickColorButton->SetBgColor(selected);
    // MOM_TODO: Finish this

}


void GameplaySettingsPage::OnCommand(const char* pCommand)
{
    if (FStrEq(pCommand, "picker"))
    {
        m_pColorPicker = new ColorPicker(this, this);
    }

    BaseClass::OnCommand(pCommand);
}

void GameplaySettingsPage::UpdateSliderEntries() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", m_pYawSpeedSlider->GetSliderValue());
    m_pYawSpeedEntry->SetText(buf);
}
