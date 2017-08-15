#include "cbase.h"

#include "GhostEntityPanel.h"
#include "clientmode.h"

#include "tier0/memdbgon.h"



CGhostEntityPanel::CGhostEntityPanel() : CEntityPanel(g_pClientMode->GetViewport(), "GhostEntityPanel"), 
m_pEntity(nullptr), m_pAvatarImage(nullptr)
{
}

void CGhostEntityPanel::Init(C_MomentumOnlineGhostEntity* pEnt)
{
    m_pEntity = pEnt;
    KeyValues* pKv = new KeyValues("blah");
    pKv->SetString("offset", "0 0");
    pKv->SetString("size", "30 30");
    BaseClass::Init(pKv, pEnt);
    pKv->deleteThis();

    Panel::SetVisible(true);
    SetPaintBackgroundEnabled(true);
}


CGhostEntityPanel::~CGhostEntityPanel()
{

}
