#pragma once
#include "vgui_controls/Frame.h"
#include "mom_modulecomms.h"

class SavelocReqFrame : public vgui::Frame, public EventListener
{
public:
    DECLARE_CLASS_SIMPLE(SavelocReqFrame, Frame);

    SavelocReqFrame();
    ~SavelocReqFrame();

    void Activate(uint64 steamID);

    void Close() OVERRIDE;

    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

    void FireEvent(KeyValues* pKv) OVERRIDE;

    void OnCommand(const char* command) OVERRIDE;

    void SetSavelocCount(int count);

    MESSAGE_FUNC_PARAMS(OnCheckButtonChecked, "CheckButtonChecked", pKv);

private:
    CUtlVector<int> m_vecSelected;
    vgui::CheckButtonList *m_pSavelocSelect;
    vgui::Button *m_pRequestButton, *m_pToggleAllButton;
    vgui::Button *m_pCancelButton;
    vgui::Label *m_pStatusLabel;

    uint64 m_iSteamID;
};