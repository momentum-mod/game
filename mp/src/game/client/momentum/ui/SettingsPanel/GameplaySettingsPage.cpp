#include "cbase.h"

#include "GameplaySettingsPage.h"

GameplaySettingsPage::GameplaySettingsPage(Panel *pParent) : BaseClass(pParent, "GameplaySettings")
{
    m_pYawSpeedSlider = FindControl<CCvarSlider>("YawSpeed");
    m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");

    m_pPlayBlockSound = FindControl<CvarToggleCheckButton<ConVarRef>>("PlayBlockSound");
    m_pSaveCheckpoints = FindControl<CvarToggleCheckButton<ConVarRef>>("SaveCheckpoints");
    m_pEnableTrail = FindControl<CvarToggleCheckButton<ConVarRef>>("EnableTrail");
    m_pTrailColorRSlider = FindControl<CCvarSlider>("TrailColorRed");
    m_pTrailColorREntry = FindControl<TextEntry>("TrailColorRedEntry");
    m_pTrailColorGSlider = FindControl<CCvarSlider>("TrailColorGreen");
    m_pTrailColorGEntry = FindControl<TextEntry>("TrailColorGreenEntry");
    m_pTrailColorBSlider = FindControl<CCvarSlider>("TrailColorBlue");
    m_pTrailColorBEntry = FindControl<TextEntry>("TrailColorBlueEntry");
    m_pTrailColorASlider = FindControl<CCvarSlider>("TrailColorAlpha");
    m_pTrailColorAEntry = FindControl<TextEntry>("TrailColorAlphaEntry");
    m_pSampleColor = FindControl<Panel>("SampleColor");
    if (!m_pYawSpeedSlider || !m_pYawSpeedEntry || !m_pPlayBlockSound || !m_pSaveCheckpoints || !m_pEnableTrail || 
        !m_pTrailColorRSlider || !m_pTrailColorREntry || !m_pSampleColor || !m_pTrailColorGSlider || !m_pTrailColorGEntry ||
        !m_pTrailColorBSlider || !m_pTrailColorBEntry || !m_pTrailColorASlider || !m_pTrailColorAEntry)
    {
        Assert("Null pointers on settings gameplay page. Expect a crash after this");
    }

    m_pYawSpeedSlider->AddActionSignalTarget(this);
    m_pYawSpeedEntry->AddActionSignalTarget(this);
    m_pPlayBlockSound->AddActionSignalTarget(this);
    m_pSaveCheckpoints->AddActionSignalTarget(this);
    m_pEnableTrail->AddActionSignalTarget(this);
    m_pTrailColorRSlider->AddActionSignalTarget(this);
    m_pTrailColorGSlider->AddActionSignalTarget(this);
    m_pTrailColorBSlider->AddActionSignalTarget(this);
    m_pTrailColorASlider->AddActionSignalTarget(this);
}

void GameplaySettingsPage::LoadSettings()
{
    UpdateSlideEntries();
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

        int alphaValue = atoi(buf);
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

    if (p == m_pYawSpeedSlider && m_pYawSpeedSlider->HasBeenModified() || 
        p == m_pTrailColorRSlider && m_pTrailColorRSlider->HasBeenModified() ||
        p == m_pTrailColorGSlider && m_pTrailColorGSlider->HasBeenModified() ||
        p == m_pTrailColorBSlider && m_pTrailColorBSlider->HasBeenModified() ||
        p == m_pTrailColorASlider && m_pTrailColorASlider->HasBeenModified())
    {
        UpdateSlideEntries();
    }
}

void GameplaySettingsPage::UpdateSlideEntries() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", m_pYawSpeedSlider->GetSliderValue());
    m_pYawSpeedEntry->SetText(buf);
    int redColor = static_cast<int>(m_pTrailColorRSlider->GetSliderValue());
    int greenColor = static_cast<int>(m_pTrailColorGSlider->GetSliderValue());
    int blueColor = static_cast<int>(m_pTrailColorBSlider->GetSliderValue());
    int alphaColor = static_cast<int>(m_pTrailColorASlider->GetSliderValue());
    Q_snprintf(buf, sizeof(buf), "%d", redColor);
    m_pTrailColorREntry->SetText(buf);
    Q_snprintf(buf, sizeof(buf), "%d", greenColor);
    m_pTrailColorGEntry->SetText(buf);
    Q_snprintf(buf, sizeof(buf), "%d", blueColor);
    m_pTrailColorBEntry->SetText(buf);
    Q_snprintf(buf, sizeof(buf), "%d", alphaColor);
    m_pTrailColorAEntry->SetText(buf);
    m_pSampleColor->SetBgColor(Color(redColor, greenColor, blueColor, alphaColor));
}