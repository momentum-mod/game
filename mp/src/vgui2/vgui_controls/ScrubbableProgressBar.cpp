#include "vgui_controls/ScrubbableProgressBar.h"
#include "vgui/IInput.h"
#include "tier1/KeyValues.h"

using namespace vgui;

DECLARE_BUILD_FACTORY(ScrubbableProgressBar);

ScrubbableProgressBar::ScrubbableProgressBar(Panel *pParent, const char *pName) : ContinuousProgressBar(pParent, pName)
{
    SetMouseInputEnabled(true); // Needed for this panel
    m_bIsHeld = false;
}

void ScrubbableProgressBar::DoScrubbing()
{
    int x, dummy;
    input()->GetCursorPosition(x, dummy);
    ScreenToLocal(x, dummy);
    float scale = static_cast<float>(x) / static_cast<float>(GetWide());
    KeyValues *pKv = new KeyValues("ScrubbedProgress");
    pKv->SetFloat("scale", scale);
    PostActionSignal(pKv);
}

void ScrubbableProgressBar::OnMousePressed(MouseCode e)
{
    if (e == MOUSE_LEFT)
    {
        m_bIsHeld = true;
        DoScrubbing();
    }
}

void ScrubbableProgressBar::OnCursorExited()
{
    BaseClass::OnCursorExited();
    m_bIsHeld = false;
}

void ScrubbableProgressBar::OnCursorMoved(int x, int y)
{
    if (m_bIsHeld)
        DoScrubbing();
}

void ScrubbableProgressBar::OnMouseWheeled(int delta)
{
    BaseClass::OnMouseWheeled(delta);
    PostActionSignal(new KeyValues("PBMouseWheeled", "delta", delta));
}

void ScrubbableProgressBar::OnMouseReleased(MouseCode e)
{
    if (e == MOUSE_LEFT)
        m_bIsHeld = false;
}
