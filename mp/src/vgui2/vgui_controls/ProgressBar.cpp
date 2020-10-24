//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/ProgressBar.h>
#include "vgui_controls/Label.h"
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY(ProgressBar);

ProgressBar::ProgressBar(Panel *parent, const char *panelName) : Panel(parent, panelName)
{
    InitSettings();
    _progress = 0.0f;
    m_pProgressPercent = new Label(this, "ProgressPercent", "");
    m_pProgressPercent->SetContentAlignment(Label::a_center);
    m_pProgressPercent->SetMouseInputEnabled(false);
    m_pProgressPercent->SetKeyBoardInputEnabled(false);
    SetProgressText();
    m_pProgressPercent->SetVisible(false);
    m_pszDialogVar = nullptr;
    SetSegmentInfo(4, 8);
    SetBarInset(4);
    SetMargin(0);
    m_iProgressDirection = PROGRESS_EAST;

    m_bSubdivMarksVisible = false;
}

ProgressBar::~ProgressBar()
{
    m_pszDialogVar.Purge();
}

void ProgressBar::SetSegmentInfo(int gap, int width)
{
    _segmentGap = gap;
    _segmentWide = width;
}

void ProgressBar::SetProgressText()
{
    if (m_pProgressPercent->IsVisible())
    {
        m_pProgressPercent->SetText(CFmtStr("%.1f%%", _progress * 100.0f));
    }
}

//-----------------------------------------------------------------------------
// Purpose: returns the number of segment blocks drawn
//-----------------------------------------------------------------------------
int ProgressBar::GetDrawnSegmentCount()
{
    int wide, tall;
    GetSize(wide, tall);
    int segmentTotal = wide / (_segmentGap + _segmentWide);
    return static_cast<int>(segmentTotal * _progress);
}

void ProgressBar::PaintBackground()
{
    int wide, tall;
    GetSize(wide, tall);

    surface()->DrawSetColor(GetBgColor());
    surface()->DrawFilledRect(0, 0, wide, tall);
}

void ProgressBar::PaintSegment(int &x, int &y, int tall, int wide)
{
    switch(m_iProgressDirection)
    {
    case PROGRESS_EAST:
    default:
        x += _segmentGap;
        surface()->DrawFilledRect(x, y, x + _segmentWide, y + tall - (y * 2));
        x += _segmentWide;
        break;

    case PROGRESS_WEST:
        x -= _segmentGap + _segmentWide;
        surface()->DrawFilledRect(x, y, x + _segmentWide, y + tall - (y * 2));
        break;

    case PROGRESS_NORTH:
        y -= _segmentGap + _segmentWide;
        surface()->DrawFilledRect(x, y, x + wide - (x * 2), y + _segmentWide);
        break;

    case PROGRESS_SOUTH:
        y += _segmentGap;
        surface()->DrawFilledRect(x, y, x + wide - (x * 2), y + _segmentWide);
        y += _segmentWide;
        break;
    }
}

void ProgressBar::PaintSubdivMarks(int x, int y, int tall, int wide)
{
    if (!m_bSubdivMarksVisible || !m_bSubdivMarksEnabled || !m_iSubdivMarksCount || m_iSubdivMarksWidth <= 0 || !m_SubdivMarksColor.a())
        return;

    bool bVertical = m_iProgressDirection == PROGRESS_SOUTH || m_iProgressDirection == PROGRESS_NORTH;
    surface()->DrawSetColor(m_SubdivMarksColor);
    int iSubdivCoord = 0 - (m_iSubdivMarksWidth / 2);
    for (int i = 0; i < m_iSubdivMarksCount; i++)
    {
        if (bVertical)
        {
            iSubdivCoord += tall / (m_iSubdivMarksCount + 1);
            surface()->DrawFilledRect(x, iSubdivCoord, wide - x, iSubdivCoord + m_iSubdivMarksWidth);
        }
        else
        {
            iSubdivCoord += wide / (m_iSubdivMarksCount + 1);
            surface()->DrawFilledRect(iSubdivCoord, y, iSubdivCoord + m_iSubdivMarksWidth, tall - y);
        }
    }
}

void ProgressBar::Paint()
{
    int wide, tall;
    GetSize(wide, tall);

    // gaps
    int segmentTotal = 0, segmentsDrawn = 0;
    int x = 0, y = 0;

    switch(m_iProgressDirection)
    {
    case PROGRESS_WEST:
        wide -= 2 * m_iBarMargin;
        x = wide - m_iBarMargin;
        y = m_iBarInset;
        segmentTotal = wide / (_segmentGap + _segmentWide);
        segmentsDrawn = static_cast<int>(segmentTotal * _progress);
        break;

    case PROGRESS_EAST:
    default:
        wide -= 2 * m_iBarMargin;
        x = m_iBarMargin;
        y = m_iBarInset;
        segmentTotal = wide / (_segmentGap + _segmentWide);
        segmentsDrawn = static_cast<int>(segmentTotal * _progress);
        break;

    case PROGRESS_NORTH:
        tall -= 2 * m_iBarMargin;
        x = m_iBarInset;
        y = tall - m_iBarMargin;
        segmentTotal = tall / (_segmentGap + _segmentWide);
        segmentsDrawn = static_cast<int>(segmentTotal * _progress);
        break;

    case PROGRESS_SOUTH:
        tall -= 2 * m_iBarMargin;
        x = m_iBarInset;
        y = m_iBarMargin;
        segmentTotal = tall / (_segmentGap + _segmentWide);
        segmentsDrawn = static_cast<int>(segmentTotal * _progress);
        break;
    }

    int xTmp = x, yTmp = y;
    surface()->DrawSetColor(GetFgColor());
    for (int i = 0; i < segmentsDrawn; i++)
    {
        PaintSegment(xTmp, yTmp, tall, wide);
    }

    PaintSubdivMarks(x, y, tall, wide);
}

void ProgressBar::PerformLayout()
{
    BaseClass::PerformLayout();

    int wide, tall;
    GetSize(wide, tall);
    m_pProgressPercent->SetSize(wide, tall);
}

void ProgressBar::SetProgress(float progress)
{
    if (progress != _progress)
    {
        // clamp the progress value within the range
        if (progress < 0.0f)
        {
            progress = 0.0f;
        }
        else if (progress > 1.0f)
        {
            progress = 1.0f;
        }

        _progress = progress;

        SetProgressText();

        Repaint();
    }
}

float ProgressBar::GetProgress()
{
    return _progress;
}

void ProgressBar::ApplySchemeSettings(IScheme *pScheme)
{
    Panel::ApplySchemeSettings(pScheme);

    SetFgColor(GetSchemeColor("ProgressBar.FgColor", pScheme));
    SetBgColor(GetSchemeColor("ProgressBar.BgColor", pScheme));
    SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
    HFont hProgressFont = pScheme->GetFont(pScheme->GetResourceString("ProgressBar.ProgressTextFont"), IsProportional());
    if (hProgressFont == INVALID_FONT)
        hProgressFont = pScheme->GetFont("DefaultSmall", IsProportional());

    Color progressColor = pScheme->GetColor("ProgressBar.ProgressTextColor", Color(0, 0, 0, 255));
    m_pProgressPercent->SetFgColor(progressColor);
    m_pProgressPercent->SetFont(hProgressFont);
}

//-----------------------------------------------------------------------------
// Purpose: utility function for calculating a time remaining string
//-----------------------------------------------------------------------------
bool ProgressBar::ConstructTimeRemainingString(wchar_t *output, int outputBufferSizeInBytes, float startTime, float currentTime, float currentProgress, float lastProgressUpdateTime, bool addRemainingSuffix)
{
    Assert(lastProgressUpdateTime <= currentTime);
    output[0] = 0;

    // calculate pre-extrapolation values
    float timeElapsed = lastProgressUpdateTime - startTime;
    float totalTime = timeElapsed / currentProgress;

    // calculate seconds
    int secondsRemaining = static_cast<int>(totalTime - timeElapsed);
    if (lastProgressUpdateTime < currentTime)
    {
        // old update, extrapolate
        float progressRate = currentProgress / timeElapsed;
        float extrapolatedProgress = progressRate * (currentTime - startTime);
        float extrapolatedTotalTime = (currentTime - startTime) / extrapolatedProgress;
        secondsRemaining = static_cast<int>(extrapolatedTotalTime - timeElapsed);
    }
    // if there's some time, make sure it's at least one second left
    if (secondsRemaining == 0 && ((totalTime - timeElapsed) > 0))
    {
        secondsRemaining = 1;
    }

    // calculate minutes
    int minutesRemaining = 0;
    while (secondsRemaining >= 60)
    {
        minutesRemaining++;
        secondsRemaining -= 60;
    }

    char minutesBuf[16];
    Q_snprintf(minutesBuf, sizeof(minutesBuf), "%d", minutesRemaining);
    char secondsBuf[16];
    Q_snprintf(secondsBuf, sizeof(secondsBuf), "%d", secondsRemaining);

    if (minutesRemaining > 0)
    {
        wchar_t unicodeMinutes[16];
        g_pVGuiLocalize->ConvertANSIToUnicode(minutesBuf, unicodeMinutes, sizeof(unicodeMinutes));
        wchar_t unicodeSeconds[16];
        g_pVGuiLocalize->ConvertANSIToUnicode(secondsBuf, unicodeSeconds, sizeof(unicodeSeconds));

        const char *unlocalizedString = "#vgui_TimeLeftMinutesSeconds";
        if (minutesRemaining == 1 && secondsRemaining == 1)
        {
            unlocalizedString = "#vgui_TimeLeftMinuteSecond";
        }
        else if (minutesRemaining == 1)
        {
            unlocalizedString = "#vgui_TimeLeftMinuteSeconds";
        }
        else if (secondsRemaining == 1)
        {
            unlocalizedString = "#vgui_TimeLeftMinutesSecond";
        }

        char unlocString[64];
        Q_strncpy(unlocString, unlocalizedString,sizeof(unlocString));
        if (addRemainingSuffix)
        {
            Q_strncat(unlocString, "Remaining", sizeof(unlocString), COPY_ALL_CHARACTERS);
        }
        g_pVGuiLocalize->ConstructString(output, outputBufferSizeInBytes, g_pVGuiLocalize->Find(unlocString), 2, unicodeMinutes, unicodeSeconds);

    }
    else if (secondsRemaining > 0)
    {
        wchar_t unicodeSeconds[16];
        g_pVGuiLocalize->ConvertANSIToUnicode(secondsBuf, unicodeSeconds, sizeof(unicodeSeconds));

        const char *unlocalizedString = "#vgui_TimeLeftSeconds";
        if (secondsRemaining == 1)
        {
            unlocalizedString = "#vgui_TimeLeftSecond";
        }
        char unlocString[64];
        Q_strncpy(unlocString, unlocalizedString,sizeof(unlocString));
        if (addRemainingSuffix)
        {
            Q_strncat(unlocString, "Remaining",sizeof(unlocString), COPY_ALL_CHARACTERS);
        }
        g_pVGuiLocalize->ConstructString(output, outputBufferSizeInBytes, g_pVGuiLocalize->Find(unlocString), 1, unicodeSeconds);
    }
    else
    {
        return false;
    }
    return true;
}

void ProgressBar::SetBarInset(int pixels)
{ 
    m_iBarInset = pixels;
}

int ProgressBar::GetBarInset()
{
    return m_iBarInset;
}

void ProgressBar::SetMargin(int pixels)
{ 
    m_iBarMargin = pixels;
}

int ProgressBar::GetMargin()
{
    return m_iBarMargin;
}

void ProgressBar::SetShouldDrawPercentString(bool bDraw)
{
    m_pProgressPercent->SetVisible(bDraw);
}

void ProgressBar::ApplySettings(KeyValues *inResourceData)
{
    _progress = inResourceData->GetFloat("progress", 0.0f);

    _segmentGap = inResourceData->GetInt("segment_gap", 4);
    _segmentWide = inResourceData->GetInt("segment_width", 8);

    const char *dialogVar = inResourceData->GetString("variable", "");
    if (dialogVar && *dialogVar)
    {
        m_pszDialogVar.Purge();
        m_pszDialogVar = dialogVar;
    }

    if (inResourceData->FindKey("draw_percent"))
        SetShouldDrawPercentString(inResourceData->GetBool("draw_percent"));

    BaseClass::ApplySettings(inResourceData);
}

void ProgressBar::GetSettings(KeyValues *outResourceData)
{
    BaseClass::GetSettings(outResourceData);
    outResourceData->SetFloat("progress", _progress);
    outResourceData->SetInt("segment_gap", _segmentGap);
    outResourceData->SetInt("segment_width", _segmentWide);
    outResourceData->SetString("variable", m_pszDialogVar);
    outResourceData->SetBool("draw_percent", m_pProgressPercent->IsVisible());
}

void ProgressBar::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"progress", TYPE_STRING},
    {"segment_gap", TYPE_INTEGER},
    {"segment_width", TYPE_INTEGER},
    {"variable", TYPE_STRING},
    {"draw_percent", TYPE_BOOL},
    END_PANEL_SETTINGS();
}

//-----------------------------------------------------------------------------
// Purpose: updates progress bar bases on values
//-----------------------------------------------------------------------------
void ProgressBar::OnDialogVariablesChanged(KeyValues *dialogVariables)
{
    if (!m_pszDialogVar.IsEmpty())
    {
        int val = dialogVariables->GetInt(m_pszDialogVar, -1);
        if (val >= 0.0f)
        {
            SetProgress(val / 100.0f);
        }
    }
}


DECLARE_BUILD_FACTORY(ContinuousProgressBar);

ContinuousProgressBar::ContinuousProgressBar(Panel *parent, const char *panelName) : ProgressBar(parent, panelName)
{
}

void ContinuousProgressBar::Paint()
{
    int x = 0, y = 0;
    int wide, tall;
    GetSize(wide, tall);

    surface()->DrawSetColor(GetFgColor());

    switch(m_iProgressDirection)
    {
    case PROGRESS_EAST:
    default:
        surface()->DrawFilledRect(x, y, x + static_cast<int>(wide * _progress), y + tall);
        break;

    case PROGRESS_WEST:
        surface()->DrawFilledRect(x + static_cast<int>(wide * (1.0f - _progress)), y, x + wide, y + tall);
        break;

    case PROGRESS_NORTH:
        surface()->DrawFilledRect(x, y + static_cast<int>(tall * (1.0f - _progress)), x + wide, y + tall);
        break;

    case PROGRESS_SOUTH:
        surface()->DrawFilledRect(x, y, x + wide, y + static_cast<int>(tall * _progress));
        break;
    }

    PaintSubdivMarks(x, y, tall, wide);
}
