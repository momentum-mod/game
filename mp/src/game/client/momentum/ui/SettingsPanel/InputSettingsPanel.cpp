#include "cbase.h"

#include "InputSettingsPanel.h"

#include "fmtstr.h"
#include "filesystem.h"
#include "inputsystem/iinputsystem.h"
#include "IGameUIFuncs.h"

#include "vgui_controls/CvarComboBox.h"
#include "vgui_controls/CvarToggleCheckButton.h"
#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/CvarTextEntry.h"
#include "vgui_controls/QueryBox.h"

#include "controls/VControlsListPanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

InputSettingsPanel::InputSettingsPanel(Panel *pParent, Button *pAssociate) : BaseClass(pParent, "InputPage", pAssociate), m_cvarCustomAccel("m_customaccel")
{
    // Mouse

    m_pReverseMouseCheckBox = new CvarToggleCheckButton(
        this,
        "ReverseMouseBox",
        "#GameUI_ReverseMouse",
        "m_pitch_inverse");

    m_pMouseFilterCheckBox = new CvarToggleCheckButton(
        this,
        "MouseFilterBox",
        "#GameUI_MouseFilter",
        "m_filter");

    m_pMouseRawCheckbox = new CvarToggleCheckButton(
        this,
        "MouseRawBox",
        "#GameUI_MouseRaw",
        "m_rawinput");

    m_pMouseSensitivitySlider = new CvarSlider(this, "MouseSensitivitySlider", "sensitivity", 0.0001f, 20.0f, 3, true);

    m_pMouseSensitivityEntry = new CvarTextEntry(this, "MouseSensitivityEntry", "sensitivity", 3);
    m_pMouseSensitivityEntry->AddActionSignalTarget(this);
    m_pMouseSensitivityEntry->SetAllowNumericInputOnly(true);

    m_pMouseAccelEntry = new CvarTextEntry(this, "MouseAccelerationEntry", "m_customaccel_exponent", 3);
    m_pMouseAccelEntry->AddActionSignalTarget(this);
    m_pMouseAccelEntry->SetAllowNumericInputOnly(true);
    m_pMouseAccelToggle = new CheckButton(this, "MouseAccelerationCheckbox", "#GameUI_MouseAcceleration");
    m_pMouseAccelSlider = new CvarSlider(this, "MouseAccelerationSlider", "m_customaccel_exponent", 1.0f, 20.0f, 3, true);
    
    // Keyboard
    m_pKeyBindList = new VControlsListPanel(this, "KeyBindPanel");
    ParseActionDescriptions();

    m_pSetBindingButton = new Button(this, "ChangeKeyButton", "#GameUI_SetNewKey");
    m_pClearBindingButton = new Button(this, "ClearKeyButton", "#GameUI_ClearKey");
    m_pRevertToDefaultBindsButton = new Button(this, "Defaults", "#GameUI_UseDefaults");

    LoadControlSettings("resource/ui/settings/Settings_Input.res");

    m_pSetBindingButton->SetEnabled(false);
    m_pClearBindingButton->SetEnabled(false);
}

void InputSettingsPanel::OnPageShow()
{
    BaseClass::OnPageShow();

    m_pMouseAccelToggle->SetSelected(m_cvarCustomAccel.GetInt() == 3);

    bool bEnabled = m_pMouseAccelToggle->IsSelected();
    m_pMouseAccelSlider->SetEnabled(bEnabled);
    m_pMouseAccelEntry->SetEnabled(bEnabled);

    // Keyboard

    FillInCurrentBindings();
    if (IsVisible())
    {
        m_pKeyBindList->SetSelectedItem(0);
    }
}

void InputSettingsPanel::OnCheckboxChecked(Panel *panel)
{
    if (panel == m_pMouseAccelToggle)
    {
        bool bMouseAccelEnabled = m_pMouseAccelToggle->IsSelected();
        m_cvarCustomAccel.SetValue(bMouseAccelEnabled ? 3 : 0);
        m_pMouseAccelSlider->SetEnabled(bMouseAccelEnabled);
        m_pMouseAccelEntry->SetEnabled(bMouseAccelEnabled);
    }
}

void InputSettingsPanel::OnKeyCodePressed(KeyCode code)
{
    // Enter key pressed and not already trapping next key/button press
    if (!m_pKeyBindList->IsCapturing())
    {
        // Grab which item was set as interesting
        int r = m_pKeyBindList->GetItemOfInterest();

        // Check that it's visible
        int x, y, w, h;
        bool visible = m_pKeyBindList->GetCellBounds(r, 1, x, y, w, h);
        if (visible)
        {
            if (KEY_DELETE == code)
            {
                // find the current binding and remove it
                KeyValues *kv = m_pKeyBindList->GetItemData(r);

                const char *key = kv->GetString("Key", nullptr);
                if (key && *key)
                {
                    RemoveKeyFromBindItems(nullptr, key);
                }

                m_pClearBindingButton->SetEnabled(false);
                m_pKeyBindList->InvalidateItem(r);

                // message handled, don't pass on
                return;
            }
        }
    }

    // Allow base class to process message instead
    BaseClass::OnKeyCodePressed(code);
}

void InputSettingsPanel::OnKeyCodeTyped(KeyCode code)
{
    if (code == KEY_ENTER)
    {
        OnCommand("ChangeKey");
    }
    else
    {
        BaseClass::OnKeyCodeTyped(code);
    }
}

void InputSettingsPanel::OnThink()
{
    BaseClass::OnThink();

    if (m_pKeyBindList->IsCapturing())
    {
        ButtonCode_t code = BUTTON_CODE_INVALID;
        if (engine->CheckDoneKeyTrapping(code))
        {
            Finish(code);
        }
    }
}

void InputSettingsPanel::OnCommand(const char *command)
{
    if (!stricmp(command, "Defaults"))
    {
        // open a box asking if we want to restore defaults
        QueryBox *box = new QueryBox("#GameUI_KeyboardSettings", "#GameUI_KeyboardSettingsText");
        box->AddActionSignalTarget(this);
        box->SetOKCommand(new KeyValues("Command", "command", "DefaultsOK"));
        box->DoModal();
    }
    else if (!stricmp(command, "DefaultsOK"))
    {
        FillInDefaultBindings();
        m_pKeyBindList->RequestFocus();
    }
    else if (!m_pKeyBindList->IsCapturing() && !stricmp(command, "ChangeKey"))
    {
        m_pKeyBindList->StartCaptureMode(dc_blank);
    }
    else if (!m_pKeyBindList->IsCapturing() && !stricmp(command, "ClearKey"))
    {
        OnKeyCodePressed(KEY_DELETE);
        m_pKeyBindList->RequestFocus();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void InputSettingsPanel::ParseActionDescriptions()
{
    char szBinding[256];
    char szDescription[256];

    KeyValues *item;

    // Load the default keys list
    CUtlBuffer buf(0, 0, CUtlBuffer::TEXT_BUFFER);
    if (!g_pFullFileSystem->ReadFile("scripts/kb_act.lst", NULL, buf))
        return;

    const char *data = (const char *)buf.Base();

    int sectionIndex = 0;
    char token[512];
    while (data)
    {
        data = engine->ParseFile(data, token, sizeof(token));
        // Done.
        if (strlen(token) <= 0)
            break;

        Q_strncpy(szBinding, token, sizeof(szBinding));

        data = engine->ParseFile(data, token, sizeof(token));
        if (strlen(token) <= 0)
        {
            break;
        }

        Q_strncpy(szDescription, token, sizeof(szDescription));

        // Skip '======' rows
        if (szDescription[0] != '=')
        {
            // Flag as special header row if binding is "blank"
            if (!stricmp(szBinding, "blank"))
            {
                // add header item
                int nColumn1 = GetScaledVal(286);
                int nColumn2 = GetScaledVal(128);
                m_pKeyBindList->AddSection(++sectionIndex, szDescription);
                m_pKeyBindList->AddColumnToSection(sectionIndex, "Action", szDescription, /*SectionedListPanel::COLUMN_BRIGHT*/ 0, nColumn1);
                m_pKeyBindList->AddColumnToSection(sectionIndex, "Key", "#GameUI_KeyButton", /*SectionedListPanel::COLUMN_BRIGHT*/0, nColumn2);
            }
            else
            {
                // Create a new: blank item
                item = new KeyValues("Item");

                item->SetString("Action", szDescription);
                item->SetString("Binding", szBinding);
                item->SetString("Key", "");

                m_pKeyBindList->AddItem(sectionIndex, item);
                item->deleteThis();
            }
        }
    }
}

void InputSettingsPanel::BindKey(const char *key, const char *binding)
{
    engine->ClientCmd_Unrestricted(CFmtStr("bind \"%s\" \"%s\"\n", key, binding));
}

void InputSettingsPanel::UnbindKey(const char *key)
{
    engine->ClientCmd_Unrestricted(CFmtStr("unbind \"%s\"\n", key));
}

void InputSettingsPanel::FillInCurrentBindings()
{
    // Clear any current settings
    ClearBindItems();

    for (int i = 0; i < BUTTON_CODE_LAST; i++)
    {
        ButtonCode_t bc = static_cast<ButtonCode_t>(i);

        // Look up binding
        const char *binding = gameuifuncs->GetBindingForButtonCode(bc);
        if (!binding)
            continue;

        // See if there is an item for this one?
        KeyValues *item = GetItemForBinding(binding);
        if (item)
        {
            // Bind it by name
            AddBinding(item, g_pInputSystem->ButtonCodeToString(bc));
        }
    }
}

void InputSettingsPanel::ClearBindItems()
{
    for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
    {
        KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));
        if (!item)
            continue;

        item->SetString("Key", "");

        m_pKeyBindList->InvalidateItem(i);
    }

    m_pKeyBindList->InvalidateLayout();
}

void InputSettingsPanel::FillInDefaultBindings()
{
    FileHandle_t fh = g_pFullFileSystem->Open("cfg/config_default.cfg", "rb");
    if (fh == FILESYSTEM_INVALID_HANDLE)
        return;

    engine->ClientCmd_Unrestricted("unbindall\n");

    int size = g_pFullFileSystem->Size(fh) + 1;
    CUtlBuffer buf(0, size, CUtlBuffer::TEXT_BUFFER);
    g_pFullFileSystem->Read(buf.Base(), size, fh);
    g_pFullFileSystem->Close(fh);

    // NULL terminate!
    ((char *)buf.Base())[size - 1] = '\0';

    ClearBindItems();

    const char *data = (const char *)buf.Base();

    // loop through all the bindings
    while (data != nullptr)
    {
        char cmd[64];
        data = engine->ParseFile(data, cmd, sizeof(cmd));
        if (strlen(cmd) <= 0)
            break;

        if (!stricmp(cmd, "bind"))
        {
            char szKeyName[256];
            data = engine->ParseFile(data, szKeyName, sizeof(szKeyName));
            if (strlen(szKeyName) <= 0)
                break;

            char szBinding[256];
            data = engine->ParseFile(data, szBinding, sizeof(szBinding));
            if (strlen(szKeyName) <= 0)
                break;

            KeyValues *item = GetItemForBinding(szBinding);
            if (item)
            {
                AddBinding(item, szKeyName);
            }
        }
    }

    // Make sure console and escape key are always valid
    KeyValues *item = GetItemForBinding("toggleconsole");
    if (item)
    {
        AddBinding(item, "`");
    }
    item = GetItemForBinding("cancelselect");
    if (item)
    {
        AddBinding(item, "ESCAPE");
    }
}

void InputSettingsPanel::AddBinding(KeyValues *item, const char *keyname)
{
    // See if it's already there as a binding
    if (!stricmp(item->GetString("Key", ""), keyname))
        return;

    // Make sure it doesn't live anywhere
    RemoveKeyFromBindItems(item, keyname);

    const char *binding = item->GetString("Binding", "");

    // Loop through all the key bindings and set all entries that have
    // the same binding. This allows us to have multiple entries pointing 
    // to the same binding.
    for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
    {
        KeyValues *curitem = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));
        if (!curitem)
            continue;

        const char *curbinding = curitem->GetString("Binding", "");

        if (!stricmp(curbinding, binding))
        {
            curitem->SetString("Key", keyname);
            BindKey(keyname, binding);
            m_pKeyBindList->InvalidateItem(i);
        }
    }
}

void InputSettingsPanel::RemoveKeyFromBindItems(KeyValues *org_item, const char *key)
{
    Assert(key && key[0]);
    if (!key || !key[0])
        return;

    int len = Q_strlen(key);
    char *pszKey = new char[len + 1];

    if (!pszKey)
        return;

    Q_memcpy(pszKey, key, len + 1);

    for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
    {
        KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));
        if (!item)
            continue;

        // If it's bound to the primary: then remove it
        const char *curKey = item->GetString("Key", "");
        if (!stricmp(pszKey, curKey))
        {
            bool bClearEntry = true;

            if (org_item)
            {
                // Only clear it out if the actual binding isn't the same. This allows
                // us to have the same key bound to multiple entries in the keybinding list
                // if they point to the same command.
                const char *org_binding = org_item->GetString("Binding", "");
                const char *binding = item->GetString("Binding", "");
                if (!stricmp(org_binding, binding))
                {
                    bClearEntry = false;
                }
            }

            if (bClearEntry)
            {
                UnbindKey(key);
                item->SetString("Key", "");
                m_pKeyBindList->InvalidateItem(i);
            }
        }
    }

    delete[] pszKey;

    // Make sure the display is up to date
    m_pKeyBindList->InvalidateLayout();
}

KeyValues *InputSettingsPanel::GetItemForBinding(const char *binding)
{
    static int bindingSymbol = KeyValuesSystem()->GetSymbolForString("Binding");

    // Loop through all items
    for (int i = 0; i < m_pKeyBindList->GetItemCount(); i++)
    {
        KeyValues *item = m_pKeyBindList->GetItemData(m_pKeyBindList->GetItemIDFromRow(i));
        if (!item)
            continue;

        KeyValues *bindingItem = item->FindKey(bindingSymbol);
        const char *bindString = bindingItem->GetString();

        // Check the "Binding" key
        if (!stricmp(bindString, binding))
            return item;
    }
    // Didn't find it
    return nullptr;
}

void InputSettingsPanel::ItemSelected(int itemID)
{
    m_pKeyBindList->SetItemOfInterest(itemID);

    if (m_pKeyBindList->IsItemIDValid(itemID))
    {
        // find the details, see if we should be enabled/clear/whatever
        m_pSetBindingButton->SetEnabled(true);

        KeyValues *kv = m_pKeyBindList->GetItemData(itemID);
        if (kv)
        {
            const char *key = kv->GetString("Key", nullptr);
            if (key && *key)
            {
                m_pClearBindingButton->SetEnabled(true);
            }
            else
            {
                m_pClearBindingButton->SetEnabled(false);
            }

            if (kv->GetInt("Header"))
            {
                m_pSetBindingButton->SetEnabled(false);
            }
        }
    }
    else
    {
        m_pSetBindingButton->SetEnabled(false);
        m_pClearBindingButton->SetEnabled(false);
    }
}

void InputSettingsPanel::Finish(ButtonCode_t code)
{
    int r = m_pKeyBindList->GetItemOfInterest();

    // Retrieve clicked row and column
    m_pKeyBindList->EndCaptureMode(dc_arrow);

    // Find item for this row
    KeyValues *item = m_pKeyBindList->GetItemData(r);
    if (item)
    {
        // Handle keys: but never rebind the escape key
        // Esc just exits bind mode silently
        if (code != BUTTON_CODE_NONE && code != KEY_ESCAPE && code != BUTTON_CODE_INVALID)
        {
            // Bind the named key
            AddBinding(item, g_pInputSystem->ButtonCodeToString(code));
        }

        m_pKeyBindList->InvalidateItem(r);
    }

    m_pSetBindingButton->SetEnabled(true);
    m_pClearBindingButton->SetEnabled(true);
}
