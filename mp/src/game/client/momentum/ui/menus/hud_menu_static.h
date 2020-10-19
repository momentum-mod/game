#pragma once

#include "hudelement.h"
#include <vgui_controls/Panel.h>

class CHudMenuStatic : public CHudElement, public vgui::Panel
{

    DECLARE_CLASS_SIMPLE(CHudMenuStatic, vgui::Panel);

  public:
    CHudMenuStatic(const char *);

    wchar_t g_szMenuString[512];
    char g_szPrelocalisedMenuString[512];

    void Init(void) OVERRIDE;
    void VidInit(void) OVERRIDE;
    void Reset(void) OVERRIDE;
    bool ShouldDraw(void) OVERRIDE;
    virtual bool IsMenuDisplayed();
    void HideMenu(bool bImmediate = false);
    void Paint() OVERRIDE;
    void OnThink() OVERRIDE;
    void LevelShutdown() OVERRIDE;

    // Overrides
    // Called from a CON_COMMAND most likely.
    // kv is the menu items, SelecFunc is the override/custom code for SelectMenuItem
    // ClosFunc is a function to be called when this menu gets closed (could be due to something else)
    virtual void ShowMenu(KeyValues *kv, void (*SelecFunc)(int), void (*ClosFunc)() = nullptr)
    {
        SelectFunc = SelecFunc;
        CloseFunc = ClosFunc;
        m_bLoadedFromFile = false;
        ShowMenu_KeyValueItems(kv);
    }
    virtual void ShowMenu(const char *file, void (*ClosFunc)() = nullptr);
    virtual void SelectMenuItem(int menu_item);
    void SelectMenuItemFromFile(int menu_item);

    void ProcessText(void);
    void ShowMenu_KeyValueItems(KeyValues *pKV);
    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void PaintString(const wchar_t *text, int textlen, vgui::HFont &font, int x, int y);

  private:
    void (*SelectFunc)(int);
    void (*CloseFunc)();
    const char *m_pszCloseCmd;

    struct ProcessedLine
    {
        int menuitem; // -1 for just text
        int startchar;
        int length;
        int pixels;
        int height;
    };

    CUtlVector<ProcessedLine> m_Processed;

    int m_nMaxPixels;
    int m_nHeight;

    bool m_bMenuDisplayed;
    int m_bitsValidSlots;
    float m_flShutoffTime;
    int m_fWaitingForMore;
    int m_nSelectedItem;
    bool m_bMenuTakesInput;
    float m_flSelectionTime;

  private:
    KeyValues *m_kvFromFile;
    bool m_bLoadedFromFile;

    CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");
    CPanelAnimationStringVar(32, m_szTextFont, "TextFont", "Default");
    vgui::HFont m_hTextFont;
    CPanelAnimationVar(float, m_flOpenCloseTime, "OpenCloseTime", "1");

    CPanelAnimationVar(float, m_flBlur, "Blur", "0");
    CPanelAnimationVar(float, m_flTextScan, "TextScan", "1.0");

    CPanelAnimationVar(float, m_flAlphaOverride, "Alpha", "255.0");
    CPanelAnimationVar(float, m_flSelectionAlphaOverride, "SelectionAlpha", "255.0");

    CPanelAnimationStringVar(32, m_szItemFont, "ItemFont", "Default");
    vgui::HFont m_hItemFont;
    CPanelAnimationStringVar(32, m_szItemFontPulsing, "ItemFontPulsing", "Default"); //"MenuItemFontPulsing");
    vgui::HFont m_hItemFontPulsing;

    CPanelAnimationVar(Color, m_MenuColor, "MenuColor", "MenuColor");
    CPanelAnimationVar(Color, m_ItemColor, "MenuItemColor", "ItemColor");
    CPanelAnimationVar(Color, m_BoxColor, "MenuBoxColor", "MenuBoxBg");
};