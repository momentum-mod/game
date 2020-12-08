#include "cbase.h"

#include "LobbySearchPanel.h"

#include "fmtstr.h"

#include "vgui_controls/ImageList.h"
#include "vgui_controls/Label.h"
#include "vgui_avatarimage.h"

#include "mom_shareddefs.h"
#include "util/mom_util.h"

#include <steam/isteammatchmaking.h>

#include "tier0/memdbgon.h"

using namespace vgui;

#define LOBBY_REQUEST_DELAY 15.0f

static CSteamID s_LobbyID = k_steamIDNil;

class LobbyListProvider
{
public:
    LobbyListProvider() : m_flLastRequestTime(-LOBBY_REQUEST_DELAY), m_pPanel(nullptr) { }
    virtual ~LobbyListProvider() = default;

    virtual void SetLobbySearchPanel(LobbySearchPanel *pPanel) { m_pPanel = pPanel; }

    virtual void SearchForLobbies() = 0;
    virtual void CancelSearch() { m_flLastRequestTime = 0.0f; }

    bool ShouldSearchForLobbies() const { return false;}
    bool IsTrackedLobby(const CSteamID &hLobby) const { return m_vecTrackedLobbies.HasElement(hLobby.ConvertToUint64()); }

    virtual void OnLobbyRemoved(const CSteamID &hLobbyID)
    {
        m_vecTrackedLobbies.FindAndRemove(hLobbyID.ConvertToUint64());
    }

    // Lobby data
    virtual bool UpdateLobbyData(const CSteamID &hLobbyID, KeyValues *pLobbyData)
    {
        CHECK_STEAM_API_B(SteamMatchmaking());
        CHECK_STEAM_API_B(SteamFriends());

        if (!pLobbyData)
            return false;

        uint64 hAvatarID;
        if (!GetLobbyListAvatar(hLobbyID, hAvatarID))
            return false;

        if (hAvatarID)
        {
            pLobbyData->SetInt("avatar", m_pPanel->TryAddAvatar(hAvatarID));
        }

        GetLobbyName(hLobbyID, hAvatarID, pLobbyData);

        pLobbyData->SetString("map", GetLobbyMap(hLobbyID));

        const auto iMemLimit = SteamMatchmaking()->GetLobbyMemberLimit(hLobbyID);
        pLobbyData->SetInt("mem_limit", iMemLimit);
        const auto iCurrentMems = SteamMatchmaking()->GetNumLobbyMembers(hLobbyID);
        pLobbyData->SetInt("current_mems", iCurrentMems);
        pLobbyData->SetString("slots", CFmtStr("%i / %i", iCurrentMems, iMemLimit));

        return true;
    }

    virtual bool GetLobbyListAvatar(const CSteamID &hLobbyID, uint64 &uInto)
    {
        uInto = Q_atoui64(SteamMatchmaking()->GetLobbyData(hLobbyID, LOBBY_DATA_OWNER));

        return true;
    }

    virtual void GetLobbyName(const CSteamID &hLobbyID, uint64 hAvatarID, KeyValues *pOut) = 0;

    virtual const char *GetLobbyMap(const CSteamID &hLobbyID)
    {
        return SteamMatchmaking()->GetLobbyData(hLobbyID, LOBBY_DATA_OWNER_MAP);
    }

protected:
    float m_flLastRequestTime;
    CUtlVector<uint64> m_vecTrackedLobbies;
    LobbySearchPanel *m_pPanel;
};

class PublicLobbyListProvider : public LobbyListProvider
{
public:
    void CancelSearch() override
    {
        if (m_cSearchResult.IsActive())
            m_cSearchResult.Cancel();

        LobbyListProvider::CancelSearch();
    }

    void SearchForLobbies() override
    {
        CHECK_STEAM_API(SteamMatchmaking());

        if (m_cSearchResult.IsActive())
        {
            Warning("Already searching for lobbies!\n");
            return;
        }

        SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide); // MOM_TODO consider changing to Far

        // MOM_TODO (0.11.0) filter out roaming lobbies

        const auto sRet = SteamMatchmaking()->RequestLobbyList();
        m_cSearchResult.Set(sRet, this, &PublicLobbyListProvider::OnLobbyMatchList);

        m_flLastRequestTime = gpGlobals->curtime;
    }

    void GetLobbyName(const CSteamID &hLobbyID, uint64 hAvatarID, KeyValues *pOut) override
    {
        const char *pOwnerName = "";

        if (hAvatarID)
            pOwnerName = SteamFriends()->GetFriendPersonaName(hAvatarID);

        pOut->SetString("name", pOwnerName[0] ? CFmtStr("%s's Lobby", pOwnerName).Get() : "#MOM_Drawer_Lobby_Public_Fallback");
    }

protected:
    void OnLobbyMatchList(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure)
    {
        m_pPanel->SetEmptyListText("#MOM_Drawer_Lobby_None");

        if (bIOFailure)
        {
            Warning("Failed to find lobbies due to IO failure!\n");
            return;
        }

        CHECK_STEAM_API(SteamMatchmaking());

        m_vecTrackedLobbies.RemoveAll();

        for (auto i = 0u; i < pLobbyMatchList->m_nLobbiesMatching; i++)
        {
            const auto hLobbyID = SteamMatchmaking()->GetLobbyByIndex(i);

            if (SteamMatchmaking()->RequestLobbyData(hLobbyID))
                m_vecTrackedLobbies.AddToTail(hLobbyID.ConvertToUint64());
        }
    }

private:
    CCallResult<PublicLobbyListProvider, LobbyMatchList_t> m_cSearchResult;
};

class FriendLobbyListProvider : public LobbyListProvider
{
public:
    void SetLobbySearchPanel(LobbySearchPanel *pPanel) override
    {
        SetDefLessFunc(m_mapLobbyIDToFriendsID);

        LobbyListProvider::SetLobbySearchPanel(pPanel);
    }

    void SearchForLobbies() override
    {
        m_pPanel->SetEmptyListText("#MOM_Drawer_Lobby_None");

        CHECK_STEAM_API(SteamFriends());
        CHECK_STEAM_API(SteamMatchmaking());

        CUtlVector<uint64> vecCurrentLobbies;
        vecCurrentLobbies.AddVectorToTail(m_vecTrackedLobbies);

        m_vecTrackedLobbies.RemoveAll();
        m_mapLobbyIDToFriendsID.PurgeAndDeleteElements();

        const auto cFriends = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
        for (auto i = 0; i < cFriends; i++)
        {
            FriendGameInfo_t friendGameInfo;
            const auto steamIDFriend = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
            if (!SteamFriends()->GetFriendGamePlayed(steamIDFriend, &friendGameInfo))
                continue;

            if (friendGameInfo.m_gameID.AppID() != 669270u)
                continue;

            const auto hLobbyID = friendGameInfo.m_steamIDLobby;

            if (!hLobbyID.IsValid())
                continue;

            vecCurrentLobbies.FindAndFastRemove(hLobbyID.ConvertToUint64());

            const auto index = m_mapLobbyIDToFriendsID.Find(hLobbyID.ConvertToUint64());
            if (m_mapLobbyIDToFriendsID.IsValidIndex(index))
            {
                // We already have a friend in this lobby
                m_mapLobbyIDToFriendsID[index]->m_vecFriendIDs.AddToTail(steamIDFriend.ConvertToUint64());
            }
            else if (SteamMatchmaking()->RequestLobbyData(hLobbyID))
            {
                m_vecTrackedLobbies.AddToTail(hLobbyID.ConvertToUint64());

                const auto pEntry = new LobbyFriendsEntry;
                pEntry->m_vecFriendIDs.AddToTail(steamIDFriend.ConvertToUint64());
                m_mapLobbyIDToFriendsID.Insert(hLobbyID.ConvertToUint64(), pEntry);
            }
        }

        FOR_EACH_VEC(vecCurrentLobbies, i)
        {
            // These are lobbies we tracked that friends were in with other (non-friend) people, but friends left the lobby.
            // The lobby could still be valid (non-friends are still in lobby), but we want to manually remove this lobby as it's false advertising otherwise.
            
            // NOTE: if more than 1 friend is in this lobby as well, and only one friend leaves, it will still be tracked as an
            // active lobby, and removed from vecCurrentLobbies.

            // NOTE NOTE: The check for m_bSuccess == false inside LobbySearchPanel::UpdateLobby will remove lobbies that friends were the only
            // members of.

            m_pPanel->RemoveLobby(vecCurrentLobbies[i]);
        }
    }

    bool GetLobbyListAvatar(const CSteamID &hLobbyID, uint64 &uInto) override
    {
        const auto index = m_mapLobbyIDToFriendsID.Find(hLobbyID.ConvertToUint64());
        if (!m_mapLobbyIDToFriendsID.IsValidIndex(index))
        {
            return false;
        }

        uInto = m_mapLobbyIDToFriendsID[index]->m_vecFriendIDs[0];

        return true;
    }

    const char *GetLobbyMap(const CSteamID &hLobbyID) override
    {
        uint64 hAvatarID;
        if (GetLobbyListAvatar(hLobbyID, hAvatarID))
        {
            const auto pMap = SteamFriends()->GetFriendRichPresence(hAvatarID, "map");
            if (pMap[0])
                return pMap;
        }

        return LobbyListProvider::GetLobbyMap(hLobbyID);
    }

    void GetLobbyName(const CSteamID &hLobbyID, uint64 hAvatarID, KeyValues *pOut) override
    {
        const char *pToReturn = "#MOM_Drawer_Lobby_Friend_Fallback";

        const char *pFriendName = "";

        if (hAvatarID)
            pFriendName = SteamFriends()->GetFriendPersonaName(hAvatarID);

        if (pFriendName[0])
        {
            const auto index = m_mapLobbyIDToFriendsID.Find(hLobbyID.ConvertToUint64());
            if (m_mapLobbyIDToFriendsID.IsValidIndex(index))
            {
                const auto iCount = m_mapLobbyIDToFriendsID[index]->m_vecFriendIDs.Count();

                if (iCount > 1)
                {
                    pToReturn = CFmtStr("%s (+ %i)", pFriendName, iCount - 1).Get();
                }
                else
                {
                    pToReturn = pFriendName;
                }
            }
            else
            {
                pToReturn = pFriendName;
            }
        }

        pOut->SetString("name", pToReturn);
    }

    void OnLobbyRemoved(const CSteamID &hLobbyID) override
    {
        LobbyListProvider::OnLobbyRemoved(hLobbyID);

        const auto index = m_mapLobbyIDToFriendsID.Find(hLobbyID.ConvertToUint64());
        if (m_mapLobbyIDToFriendsID.IsValidIndex(index))
        {
            delete m_mapLobbyIDToFriendsID[index];
            m_mapLobbyIDToFriendsID.RemoveAt(index);
        }
    }

private:
    struct LobbyFriendsEntry
    {
        CUtlVector<uint64> m_vecFriendIDs; // Friends in this lobby
    };
    CUtlMap<uint64, LobbyFriendsEntry*> m_mapLobbyIDToFriendsID; // Friends that are in lobbies need to map their lobby to their ID
};

LobbySearchPanel::LobbySearchPanel(Panel *pParent, bool bSearchingFriends) : BaseClass(pParent, "LobbySearchPanel")
{
    m_bSearchingFriends = bSearchingFriends;

    if (m_bSearchingFriends)
    {
        m_pProvider = new FriendLobbyListProvider;
    }
    else
    {
        m_pProvider = new PublicLobbyListProvider;
    }

    m_pProvider->SetLobbySearchPanel(this);

    SetProportional(true);

    AddColumnHeader(0, "avatar", "", GetScaledVal(24), COLUMN_IMAGE | COLUMN_IMAGE_SIZETOFIT | COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO | COLUMN_DISABLED | COLUMN_UNHIDABLE | COLUMN_FIXEDSIZE);
    AddColumnHeader(1, "name", "#MOM_Name", GetScaledVal(180), GetScaledVal(100), GetScaledVal(180), 0);
    AddColumnHeader(2, "map", "#MOM_Map", GetScaledVal(180), GetScaledVal(130), GetScaledVal(180), 0);
    AddColumnHeader(3, "slots", "#MOM_Drawer_Lobby_Players", GetScaledVal(200), GetScaledVal(10), 9000, 0);

    SetColumnSortable(1, true);
    SetSortColumn(1);

    SetColumnTextAlignment(0, Label::Alignment::a_center);

    SetAutoTallHeaderToFont(true);
    SetRowHeightOnFontChange(false);
    SetRowHeight(GetScaledVal(26));

    SetShouldCenterEmptyListText(true);
    SetEmptyListText("#MOM_API_WaitingForResponse");

    m_pAvatarImages = nullptr;
    SetDefLessFunc(m_mapLobbyIDToImageListIndex);
    SetDefLessFunc(m_mapLobbyIDToListPanelIndex);

    InitImageList();
}

LobbySearchPanel::~LobbySearchPanel()
{
    delete m_pProvider;
}

void LobbySearchPanel::SearchForLobbies()
{
    if (m_pProvider->ShouldSearchForLobbies())
    {
        SetEmptyListText("#MOM_API_WaitingForResponse");
        m_pProvider->SearchForLobbies();
    }
}

void LobbySearchPanel::CancelSearch()
{
    m_pProvider->CancelSearch();

    SetEmptyListText("#MOM_Drawer_Lobby_None");
}

void LobbySearchPanel::OnLobbyEnter(LobbyEnter_t *pEnter)
{
    CancelSearch();

    s_LobbyID = pEnter->m_ulSteamIDLobby;
}

void LobbySearchPanel::OnLobbyLeave()
{
    SearchForLobbies();

    s_LobbyID = k_steamIDNil;
}

void LobbySearchPanel::OnLobbyDataUpdate(LobbyDataUpdate_t *pUpdate)
{
    // We do not care about the lobby we are in
    if (pUpdate->m_ulSteamIDLobby == s_LobbyID.ConvertToUint64())
        return;

    // We only care about lobby updates themselves
    if (pUpdate->m_ulSteamIDMember != pUpdate->m_ulSteamIDLobby)
        return;

    const auto hLobbyID = CSteamID(pUpdate->m_ulSteamIDLobby);

    if (!pUpdate->m_bSuccess)
    {
        RemoveLobby(hLobbyID);
        return;
    }

    if (!m_pProvider->IsTrackedLobby(hLobbyID))
        return;

    const auto index = m_mapLobbyIDToListPanelIndex.Find(hLobbyID.ConvertToUint64());
    if (!m_mapLobbyIDToListPanelIndex.IsValidIndex(index))
    {
        AddLobby(hLobbyID);
        return;
    }

    const auto itemID = m_mapLobbyIDToListPanelIndex[index];
    const auto pLobbyData = GetItem(itemID);

    if (UpdateLobby(hLobbyID, pLobbyData))
    {
        ApplyItemChanges(itemID);
    }
    else
    {
        RemoveLobby(hLobbyID);
    }
}

void LobbySearchPanel::AddLobby(const CSteamID &hLobbyID)
{
    if (!LobbyTypeIsValid(hLobbyID))
        return;

    KeyValuesAD data("Data");

    data->SetUint64("lobby", hLobbyID.ConvertToUint64());

    if (UpdateLobby(hLobbyID, data))
    {
        const auto itemID = AddItem(data, 0, false, true);

        m_mapLobbyIDToListPanelIndex.Insert(hLobbyID.ConvertToUint64(), itemID);
    }
}

bool LobbySearchPanel::UpdateLobby(const CSteamID &hLobbyID, KeyValues *pLobbyData)
{
    if (!LobbyTypeIsValid(hLobbyID))
        return false;

    return m_pProvider->UpdateLobbyData(hLobbyID, pLobbyData);
}

void LobbySearchPanel::RemoveLobby(const CSteamID &hLobbyID)
{
    const auto index = m_mapLobbyIDToListPanelIndex.Find(hLobbyID.ConvertToUint64());
    if (!m_mapLobbyIDToListPanelIndex.IsValidIndex(index))
        return;

    const auto itemID = m_mapLobbyIDToListPanelIndex[index];

    RemoveItem(itemID);

    m_mapLobbyIDToListPanelIndex.RemoveAt(index);
    m_mapLobbyIDToImageListIndex.Remove(hLobbyID.ConvertToUint64());

    m_pProvider->OnLobbyRemoved(hLobbyID);

    SetEmptyListText("#MOM_Drawer_Lobby_None");
}

void LobbySearchPanel::OnKeyCodeTyped(KeyCode code)
{
    if (code == KEY_ENTER)
    {
        const auto pUserData = GetItem(GetSelectedItem(0));

        if (pUserData)
        {
            MomUtil::DispatchConCommand(CFmtStr("connect_lobby %llu", pUserData->GetUint64("lobby")));
            return;
        }
    }

    BaseClass::OnKeyCodeTyped(code);
}

void LobbySearchPanel::OnThink()
{
    if (m_pProvider->ShouldSearchForLobbies())
    {
        RefreshLobbyData();
    }
}

bool LobbySearchPanel::LobbyTypeIsValid(const CSteamID &hLobbyID)
{
    CHECK_STEAM_API_B(SteamMatchmaking())

    const auto iType = Q_atoi(SteamMatchmaking()->GetLobbyData(hLobbyID, LOBBY_DATA_TYPE));
    // Public search returns only public
    if (!m_bSearchingFriends && iType != k_ELobbyTypePublic)
        return false;

    // Friends should be either public or friends only to show
    if (m_bSearchingFriends && iType != k_ELobbyTypePublic && iType != k_ELobbyTypeFriendsOnly)
        return false;

    return true;
}

void LobbySearchPanel::RefreshLobbyData()
{
    CHECK_STEAM_API(SteamMatchmaking());

    // Update current
    FOR_EACH_MAP_FAST(m_mapLobbyIDToListPanelIndex, i)
    {
        SteamMatchmaking()->RequestLobbyData(m_mapLobbyIDToListPanelIndex.Key(i));
    }

    // And search for new
    SearchForLobbies();
}

void LobbySearchPanel::InitImageList()
{
    m_mapLobbyIDToImageListIndex.RemoveAll();

    delete m_pAvatarImages;
    m_pAvatarImages = nullptr;

    m_pAvatarImages = new ImageList(true);
    SetImageList(m_pAvatarImages, false);
}

int LobbySearchPanel::TryAddAvatar(uint64 uID)
{
    if (!m_pAvatarImages)
        return -1;

    // See if we already have that avatar in our list
    const unsigned short mapIndex = m_mapLobbyIDToImageListIndex.Find(uID);
    int iImageIndex;
    if (!m_mapLobbyIDToImageListIndex.IsValidIndex(mapIndex))
    {
        const auto pImage = new CAvatarImage;
        // 64 is enough up to full HD resolutions.
        pImage->SetAvatarSteamID(CSteamID(uID), k_EAvatarSize64x64);

        pImage->SetDrawFriend(false);
        pImage->SetAvatarSize(32, 32);
        iImageIndex = m_pAvatarImages->AddImage(pImage);
        m_mapLobbyIDToImageListIndex.Insert(uID, iImageIndex);
    }
    else
    {
        iImageIndex = m_mapLobbyIDToImageListIndex.Element(mapIndex);
    }

    return iImageIndex;
}