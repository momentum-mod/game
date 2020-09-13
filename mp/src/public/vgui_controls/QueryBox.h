#pragma once

#include <vgui_controls/MessageBox.h>

namespace vgui
{

    //-----------------------------------------------------------------------------
    // Purpose: Creates A Message box with a question in it and yes/no buttons
    //-----------------------------------------------------------------------------
    class QueryBox : public MessageBox
    {
    public:
        DECLARE_CLASS_SIMPLE(QueryBox, MessageBox);

        QueryBox(const char *title, const char *queryText, Panel *parent = nullptr);
        QueryBox(const wchar_t *wszTitle, const wchar_t *wszQueryText, Panel *parent = nullptr);

        void PerformLayout() override;

        // Set a value of the ok command
        void SetOKCommandValue(const char *keyName, int value);
    };
}