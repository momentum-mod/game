#pragma once

#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IInput.h>
#include <KeyValues.h>

namespace vgui
{
    class Button;
    class CheckButton;
    class Label;
    class ProgressBar;
    class FileOpenDialog;
    class Slider;
};

namespace vgui
{
    //-----------------------------------------------------------------------------
    // Purpose: overrides normal button drawing to use different colors & borders
    //-----------------------------------------------------------------------------
    class PFrameButton : public Button
    {
        DECLARE_CLASS_SIMPLE(PFrameButton, Button);
        Color _selectedColor;

        PFrameButton(Panel *parent, const char *name, const char *text) : Button(parent, name, text)
        {
        }

        virtual void ApplySchemeSettings(IScheme *pScheme)
        {
            Button::ApplySchemeSettings(pScheme);
            _selectedColor = GetSchemeColor("ToggleButton.SelectedTextColor", pScheme);
        }

        virtual void OnClick()
        {
            // post a button toggled message
            KeyValues *msg = new KeyValues("ButtonToggled");
            msg->SetInt("state", static_cast<int>(IsSelected()));
            PostActionSignal(msg);
            Repaint();
        }

        virtual Color GetButtonFgColor()
        {
            if (IsSelected())
            {
                // highlight the text when depressed
                return _selectedColor;
            }
            return BaseClass::GetButtonFgColor();
        }

        // Don't request focus.
        // This will keep items in the listpanel selected.
        virtual void OnMousePressed(MouseCode code)
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
    };

    DECLARE_BUILD_FACTORY_DEFAULT_TEXT(PFrameButton, PFrameButton);
} // namespace vgui