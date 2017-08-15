#include "cbase.h"

#include "GhostEntityPanel.h"
#include "clientmode.h"

#include "tier0/memdbgon.h"


CGhostEntityPanel::CGhostEntityPanel() : CEntityPanel(g_pClientMode->GetViewport(), "GhostEntityPanel"), 
m_pEntity(nullptr), m_pAvatarImage(nullptr)
{
    m_pAvatarImage = new CAvatarImagePanel(this, "GhostEntityPanelAvatar");
    m_pAvatarImage->SetShouldDrawFriendIcon(true);
    m_pAvatarImage->SetShouldScaleImage(true);
    m_pAvatarImage->SetMinimumSize(10, 10);
    m_pAvatarImage->SetSize(92, 92);
    m_pAvatarImage->SetPos(10, 10);
}

void CGhostEntityPanel::Init(C_MomentumOnlineGhostEntity* pEnt)
{
    m_pEntity = pEnt;
    KeyValues* pKv = new KeyValues("blah");
    pKv->SetString("offset", "0 -30");
    pKv->SetString("size", "30 30");
    BaseClass::Init(pKv, pEnt);
    pKv->deleteThis();

    Panel::SetVisible(true);
    SetPaintBackgroundEnabled(true);

    m_pAvatarImage->ClearAvatar();
}

void CGhostEntityPanel::OnTick()
{
    if (m_pEntity)
    {
        if (!m_pAvatarImage->IsValid() && m_pEntity->m_SteamID.IsValid())
        {
            m_pAvatarImage->SetPlayer(m_pEntity->m_SteamID, k_EAvatarSize184x184);
        }

        // MOM_TODO: Blink the panel if they're typing? Maybe an icon or something? Idk

        // MOM_TODO: Show the name if we're looking at this ghost entity
    }

    BaseClass::OnTick();
}


CGhostEntityPanel::~CGhostEntityPanel()
{
    if (m_pAvatarImage)
    {
        m_pAvatarImage->ClearAvatar();
        m_pAvatarImage->DeletePanel();
    }
        
    m_pAvatarImage = nullptr;
}
