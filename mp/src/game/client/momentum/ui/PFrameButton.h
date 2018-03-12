#pragma once

#include <vgui_controls/Button.h>

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

        PFrameButton(Panel *parent, const char *name, const char *text) : Button(parent, name, text){}

        virtual void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;

        virtual void OnClick();

        Color GetButtonFgColor() OVERRIDE;

        // Don't request focus.
        // This will keep items in the listpanel selected.
        virtual void OnMousePressed(MouseCode code) OVERRIDE;
    };

} // namespace vgui