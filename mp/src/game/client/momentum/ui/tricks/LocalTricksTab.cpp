#include "cbase.h"

#include "LocalTricksTab.h"

#include "vgui_controls/ListPanel.h"
#include "controls/ContextMenu.h"

#include "fmtstr.h"
#include "mom_system_tricks.h"

#include "tier0/memdbgon.h"

using namespace vgui;

LocalTricksTab::LocalTricksTab(Panel *pParent): BaseClass(pParent, "LocalTricksTab")
{
    SetProportional(true);

    m_pLocalTricksList = nullptr;

    m_pContextMenu = new ContextMenu(this);
    m_pContextMenu->SetAutoDelete(false);
    m_pContextMenu->AddActionSignalTarget(this);
    m_pContextMenu->SetVisible(false);
}

void LocalTricksTab::OnPageHide()
{
    if (m_pContextMenu && m_pContextMenu->IsVisible())
    {
        // Close the menu
        m_pContextMenu->OnKillFocus();
    }
}

void LocalTricksTab::OnResetData()
{
    m_pLocalTricksList = new ListPanel(this, "LocalTricksList");
    m_pLocalTricksList->SetRowHeightOnFontChange(false);
    m_pLocalTricksList->SetRowHeight(GetScaledVal(20));
    m_pLocalTricksList->SetMultiselectEnabled(false);
    m_pLocalTricksList->SetAutoTallHeaderToFont(true);
    m_pLocalTricksList->ResetScrollBar();

    LoadControlSettings("resource/ui/tricks/LocalTricksTab.res");

    m_pLocalTricksList->AddColumnHeader(0, "steps", "#MOM_Trick_Steps", GetScaledVal(60));
    m_pLocalTricksList->AddColumnHeader(1, "difficulty", "#MOM_MapSelector_Difficulty", GetScaledVal(60));
    m_pLocalTricksList->AddColumnHeader(2, "name", "#MOM_Trick_Name", GetScaledVal(120));

    m_pLocalTricksList->SetSortFunc(2, [](ListPanel *pListPanel, const ListPanelItem &item1, const ListPanelItem &item2) ->
    int
    {
        return Q_strcmp(item1.kv->GetString("name"), item2.kv->GetString("name"));
    });
    m_pLocalTricksList->SetSortColumn(2);

    ParseTrickData();
}

void LocalTricksTab::ParseTrickData()
{
    if (!m_pLocalTricksList)
        return;

    m_pLocalTricksList->RemoveAll();

    const auto iTrickCount = g_pTrickSystem->GetTrickCount();
    for (int i = 0; i < iTrickCount; i++)
    {
        const auto pTrick = g_pTrickSystem->GetTrick(i);

        const auto pKv = new KeyValues("Trick");
        pKv->SetString("name", pTrick->GetName());
        pKv->SetInt("steps", pTrick->StepCount());
        pKv->SetInt("difficulty", pTrick->GetDifficulty());

        m_pLocalTricksList->AddItem(pKv, pTrick->GetID(), false, true);
    }
}

void LocalTricksTab::OnItemSelected()
{
    const auto selectedItemID = m_pLocalTricksList->GetSelectedItem(0);
    const auto iTrickID = m_pLocalTricksList->GetItemUserData(selectedItemID);

    engine->ClientCmd(CFmtStr("mom_tricks_track_trick %i", iTrickID));

    // MOM_TODO keep track of this tracked trick for panel rendering things (outline?)
}

void LocalTricksTab::OnItemContextMenu(vgui::Panel *panel, int itemID)
{
    const auto pKVData = m_pLocalTricksList->GetItem(itemID);
    if (!pKVData)
        return;

    m_pContextMenu->OnKillFocus();
    m_pContextMenu->DeleteAllItems();

    KeyValues *pKv = new KeyValues("ContextTrickTele");
    pKv->SetInt("trickID", m_pLocalTricksList->GetItemUserData(itemID));
    m_pContextMenu->AddMenuItem("TrickTele", "#MOM_Trick_Teleport", pKv, this);

    m_pContextMenu->ShowMenu();
}

void LocalTricksTab::OnContextTrickTele(int trickID)
{
    engine->ClientCmd(CFmtStr("mom_tricks_tele_to_trick %i", trickID));
}