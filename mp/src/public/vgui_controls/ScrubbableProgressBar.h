#pragma once

#include "vgui_controls/ProgressBar.h"
#include <vgui_controls/Panel.h>

/**
 * This is a scroll bar that the user can click (and drag) on the progress to fire
 * events related to potentially changing the playback to that position.
 */
namespace vgui
{
    class ScrubbableProgressBar : public ContinuousProgressBar
    {
        DECLARE_CLASS_SIMPLE(ScrubbableProgressBar, ContinuousProgressBar);

        ScrubbableProgressBar(Panel *pParent, const char *pName);

        void DoScrubbing();

    protected:
        void OnMousePressed(MouseCode e) OVERRIDE;

        void OnCursorExited() OVERRIDE;

        void OnCursorMoved(int x, int y) OVERRIDE;
        void OnMouseWheeled(int delta) OVERRIDE;
        void OnMouseReleased(MouseCode e) OVERRIDE;

    private:
        bool m_bIsHeld;
    };
}