#include "cbase.h"

#include "SavelocRequestFrame.h"
#include "vgui_controls/CheckButtonList.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "tier1/fmtstr.h"
#include "mom_modulecomms.h"

#include "tier0/memdbgon.h"

using namespace vgui;

SavelocReqFrame::SavelocReqFrame() : BaseClass(nullptr, "SavelocReqFrame"), m_iSteamID(0)
{
    SetProportional(true);
    SetMaximizeButtonVisible(false);
    SetMinimizeButtonVisible(false);
    SetMenuButtonVisible(false);
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
    SetSizeable(false); // MOM_TODO: Make it sizeable?
    SetTitle("#MOM_Saveloc_Frame", true);

    SetScheme("SourceScheme");

    m_pSavelocSelect = new CheckButtonList(this, "SavelocSelect");
    m_pRequestButton = new Button(this, "RequestButton", "#MOM_Saveloc_Frame_Request", this, "Request");
    m_pCancelButton = new Button(this, "CancelButton", "#GameUI_Cancel", this, "Cancel");
    m_pStatusLabel = new Label(this, "StatusLabel", "Obtaining saveloc count...");
    m_pToggleAllButton = new Button(this, "ToggleAll", "#MOM_Saveloc_Frame_Toggle", this, "ToggleSelect");

    LoadControlSettings("resource/ui/SavelocReqFrame.res");

    m_pRequestButton->SetEnabled(false);

    g_pModuleComms->ListenForEvent("req_savelocs", UtlMakeDelegate(this, &SavelocReqFrame::FireEvent));

    SetVisible(false);
}

SavelocReqFrame::~SavelocReqFrame()
{
}

void SavelocReqFrame::Activate(uint64 steamID)
{
    m_iSteamID = steamID;

    BaseClass::Activate();
}

void SavelocReqFrame::Close()
{
    m_iSteamID = 0;
    m_pStatusLabel->SetText("Obtaining saveloc count...");
    BaseClass::Close();
}

void SavelocReqFrame::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_pStatusLabel->SetFont(pScheme->GetFont("DefaultSmall", true));
}

void SavelocReqFrame::FireEvent(KeyValues* pKv)
{
    if (FStrEq(pKv->GetName(), "req_savelocs"))
    {
        int stage = pKv->GetInt("stage");

        if (stage == 2)
        {
            // We got the number of savelocs
            SetSavelocCount(pKv->GetInt("count"));
        }
        else if (stage == -1) // We got the savelocs we needed
        {
            m_pStatusLabel->SetText("Savelocs downloaded!");

            // MOM_TODO: Re-enable these, just in case they want more?
            m_pSavelocSelect->SetEnabled(false);
            m_pRequestButton->SetEnabled(false);
            m_pToggleAllButton->SetEnabled(false);
        }
        else if (stage == -2)
        {
            // They left
            m_pStatusLabel->SetText("The requestee has left.");
            m_pRequestButton->SetEnabled(false);
            m_pSavelocSelect->SetEnabled(false);
            m_pToggleAllButton->SetEnabled(false);
        }
    }
}

void SavelocReqFrame::OnCommand(const char* command)
{
    if (FStrEq(command, "Request"))
    {
        if (m_iSteamID)
        {
            // Build the stage 3 packet
            KeyValues* pKv = new KeyValues("req_savelocs");
            pKv->SetInt("stage", 3);
            pKv->SetUint64("target", m_iSteamID);
            pKv->SetInt("count", m_vecSelected.Count());
            pKv->SetPtr("nums", m_vecSelected.Base());
            g_pModuleComms->FireEvent(pKv);

            // Prevent them from changing this request
            m_pRequestButton->SetEnabled(false);
            m_pSavelocSelect->SetEnabled(false);

            // Let em know
            m_pStatusLabel->SetText("Downloading savelocs...");
        }
        else
        {
            Warning("Cannot download savelocs, invalid steam ID!\n");
        }

        return;
    }

    if (FStrEq(command, "Cancel") || FStrEq(command, "Close"))
    {
        if (m_iSteamID)
        {
            KeyValues* pKv = new KeyValues("req_savelocs");
            pKv->SetInt("stage", -3);
            pKv->SetUint64("target", m_iSteamID);
            g_pModuleComms->FireEvent(pKv);
        }

        Close();
        return;
    }

    if (FStrEq(command, "ToggleSelect"))
    {
        m_pSavelocSelect->ToggleSelectAll(!m_pSavelocSelect->AllItemsChecked());
        return;
    }

    BaseClass::OnCommand(command);
}

void SavelocReqFrame::SetSavelocCount(int count)
{
    m_pSavelocSelect->RemoveAll();

    CUtlVector<int> copy;
    if (!m_vecSelected.IsEmpty())
        copy.AddVectorToTail(m_vecSelected);

    m_vecSelected.RemoveAll();

    for (int i = 0; i < count; i++)
    {
        const bool isSelected = copy.HasElement(i);
        // MOM_TODO: Localize this?
        m_pSavelocSelect->AddItem(CFmtStr("Saveloc %i", i + 1).Get(), isSelected, nullptr);
        if (isSelected)
            m_vecSelected.AddToTail(i);
    }

    m_pStatusLabel->SetText("Select savelocs to request.");
    m_pRequestButton->SetEnabled(!m_vecSelected.IsEmpty());
    m_pToggleAllButton->SetEnabled(true);
    m_pSavelocSelect->SetEnabled(true);
}

void SavelocReqFrame::OnCheckButtonChecked(KeyValues* pKv)
{
    bool state = pKv->GetBool("state");
    int itemID = pKv->GetInt("itemid");
    int idx = m_vecSelected.Find(itemID);
    if (idx != m_vecSelected.InvalidIndex())
    {
        if (!state)
            m_vecSelected.Remove(idx);
    }
    else if (state)
        m_vecSelected.AddToTail(itemID);

    m_pRequestButton->SetEnabled(!m_vecSelected.IsEmpty());
}
