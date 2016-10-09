#pragma once

#include "cbase.h"

#include "vgui/IInput.h"
#include "vgui_controls/ProgressBar.h"
#include <vgui_controls/Panel.h>

/**
 * This is a scroll bar that the user can click (and drag) on the progress to fire
 * events related to potentially changing the playback to that position.
 */
class ScrubbableProgressBar : public vgui::ContinuousProgressBar
{
    DECLARE_CLASS_SIMPLE(ScrubbableProgressBar, ContinuousProgressBar);

    ScrubbableProgressBar(Panel *pParent, const char *pName) : ContinuousProgressBar(pParent, pName)
    {
        SetMouseInputEnabled(true); // Needed for this panel
        m_bIsHeld = false;
    }

    void DoScrubbing()
    {
        int x, dummy;
        input()->GetCursorPosition(x, dummy);
        ScreenToLocal(x, dummy);
        float scale = static_cast<float>(x) / static_cast<float>(GetWide());
        KeyValues *pKv = new KeyValues("ScrubbedProgress");
        pKv->SetFloat("scale", scale);
        PostActionSignal(pKv);
    }

  protected:
    void OnMousePressed(MouseCode e) OVERRIDE
    {
        if (e == MOUSE_LEFT)
        {
            m_bIsHeld = true;
            DoScrubbing();
        }
    }

    void OnCursorExited() OVERRIDE
    {
        BaseClass::OnCursorExited();
        m_bIsHeld = false;
    }

    void OnCursorMoved(int x, int y) OVERRIDE
    {
        if (m_bIsHeld)
            DoScrubbing();
    }

    void OnMouseReleased(MouseCode e) OVERRIDE
    {
        if (e == MOUSE_LEFT)
            m_bIsHeld = false;
    }

  private:
    bool m_bIsHeld;
};

DECLARE_BUILD_FACTORY(ScrubbableProgressBar);