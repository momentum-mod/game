#include "cbase.h"

#include "PFrameButton.h"

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(PFrameButton, PFrameButton);

void PFrameButton::ApplySchemeSettings(IScheme* pScheme)
{
    Button::ApplySchemeSettings(pScheme);
    _selectedColor = GetSchemeColor("ToggleButton.SelectedTextColor", pScheme);
}

void PFrameButton::OnClick()
{
    // post a button toggled message
    KeyValues *msg = new KeyValues("ButtonToggled");
    msg->SetInt("state", static_cast<int>(IsSelected()));
    PostActionSignal(msg);
    Repaint();
}

Color PFrameButton::GetButtonFgColor()
{
    if (IsSelected())
    {
        // highlight the text when depressed
        return _selectedColor;
    }
    return BaseClass::GetButtonFgColor();
}

void PFrameButton::OnMousePressed(MouseCode code)
{
    if (!IsEnabled() || !IsMouseClickEnabled(code))
        return;

    if (IsUseCaptureMouseEnabled())
    {
        SetSelected(true);
        Repaint();
        // lock mouse input to going to this button
        input()->SetMouseCapture(GetVPanel());
    }
}
