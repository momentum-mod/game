#include "cbase.h"

#include "hud_speedometer_data.h"
#include "hud_speedometer.h"
#include "hud_speedometer_label.h"

#include "mom_system_gamemode.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

#define SPEEDOMETER_FILENAME "cfg/speedometer.vdf"
#define SPEEDOMETER_DEFAULT_FILENAME "cfg/speedometer_default.vdf"

CON_COMMAND_F(mom_hud_speedometer_savecfg,
              "Writes the current speedometer setup for the current gamemode to file.\n"
              "Optionally takes in the gamemode to write to.\n",
              FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    if (args.ArgC() > 1)
    {
        int iGamemodeIndex = Q_atoi(args[1]);
        if (iGamemodeIndex >= 0 && iGamemodeIndex < GAMEMODE_COUNT)
        {
            g_pSpeedometerData->SaveGamemodeData(GameMode_t(iGamemodeIndex));
        }
    }
    else
    {
        g_pSpeedometerData->SaveGamemodeData(g_pGameModeSystem->GetGameMode()->GetType());
    }
}

CON_COMMAND_F(mom_hud_speedometer_loadcfg,
              "Loads the speedometer setup for the current gamemode from file.\n"
              "Optionally takes in the gamemode to load from.\n",
              FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    if (args.ArgC() > 1)
    {
        int iGamemodeIndex = Q_atoi(args[1]);
        if (iGamemodeIndex >= 0 && iGamemodeIndex < GAMEMODE_COUNT)
        {
            g_pSpeedometerData->LoadGamemodeData(GameMode_t(iGamemodeIndex));
        }
    }
    else
    {
        g_pSpeedometerData->LoadGamemodeData(g_pGameModeSystem->GetGameMode()->GetType());
    }
}

SpeedometerData::SpeedometerData()
    : m_pGamemodeSetupData(nullptr), m_CurrentlyLoadedGamemodeSettings(GAMEMODE_UNKNOWN)
{
}

void SpeedometerData::LoadGamemodeData() 
{ 
    LoadGamemodeData(g_pGameModeSystem->GetGameMode()->GetType()); 
}

void SpeedometerData::LoadGamemodeData(int gametype)
{
    if (gametype >= 0 && gametype < GAMEMODE_COUNT)
        LoadGamemodeData(GameMode_t(gametype));
}

void SpeedometerData::LoadGamemodeData(GameMode_t gametype)
{
    if (m_pGamemodeSetupData)
        m_pGamemodeSetupData->deleteThis();

    m_pGamemodeSetupData = new KeyValues("Gamemodes");

    if (!g_pFullFileSystem->FileExists(SPEEDOMETER_FILENAME))
    {
        KeyValues *pKVTemp = new KeyValues(SPEEDOMETER_FILENAME);
        pKVTemp->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_DEFAULT_FILENAME, "MOD");
        pKVTemp->SaveToFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD");
        pKVTemp->deleteThis();
    }

    m_pGamemodeSetupData->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD");

    KeyValues *pGamemodeKV = m_pGamemodeSetupData->FindKey(g_szGameModes[gametype]);

    if (!pGamemodeKV) // unknown gamemode
        return;

    m_CurrentlyLoadedGamemodeSettings = gametype;

    // get ordering
    KeyValues *pOrderKV = pGamemodeKV->FindKey("order");
    if (pOrderKV)
    {
        auto pOrderList = g_pSpeedometer->GetLabelOrderListPtr();
        pOrderList->RemoveAll();
        char szIndex[BUFSIZELOCL];
        for (unsigned int i = 1; i <= SPEEDOMETER_MAX_LABELS; i++)
        {
            Q_snprintf(szIndex, BUFSIZELOCL, "%i", i);
            const char *szSpeedo = pOrderKV->GetString(szIndex);
            if (szSpeedo)
            {
                SpeedometerLabel *label = GetLabelFromName(szSpeedo);
                if (label)
                {
                    pOrderList->AddToTail(label);
                }
            }
        }
    }
    else
    {
        g_pSpeedometer->ResetLabelOrder();
    }

    // get speedometer label setups
    FOR_EACH_SUBKEY(pGamemodeKV, pSpeedoItem)
    {
        const char *speedoname = pSpeedoItem->GetName();
        SpeedometerLabel *pLabel = GetLabelFromName(speedoname);
        KeyValues *pSpeedoKV = pGamemodeKV->FindKey(speedoname);
        if (!pLabel || !pSpeedoKV)
            continue;

        pLabel->LoadFromKV(pSpeedoKV);
    }
}

void SpeedometerData::SaveGamemodeData() 
{ 
    SaveGamemodeData(g_pGameModeSystem->GetGameMode()->GetType());
}

void SpeedometerData::SaveGamemodeData(int gametype)
{
    if (gametype >= 0 && gametype < GAMEMODE_COUNT)
        SaveGamemodeData(GameMode_t(gametype));
}

void SpeedometerData::SaveGamemodeData(GameMode_t gametype)
{
    if (!g_pFullFileSystem)
        return;

    KeyValues *pGamemodeKV = m_pGamemodeSetupData->FindKey(g_szGameModes[gametype]);

    if (!pGamemodeKV) // unknown gamemode
        return;

    // set speedometer label setups
    FOR_EACH_SUBKEY(pGamemodeKV, pSpeedoItem)
    {
        const char *speedoname = pSpeedoItem->GetName();
        SpeedometerLabel *pLabel = GetLabelFromName(speedoname);
        KeyValues *pSpeedoKV = pGamemodeKV->FindKey(speedoname, true);
        if (!pLabel || !pSpeedoKV)
            continue;

        pLabel->SaveToKV(pSpeedoKV);
    }

    // set ordering
    auto pOrderList = g_pSpeedometer->GetLabelOrderListPtr();
    KeyValues *pOrderKVs = pGamemodeKV->FindKey("order", true);
    pOrderKVs->Clear();
    char tmpBuf[BUFSIZELOCL];
    for (auto i = 0; i < pOrderList->Count(); i++)
    {
        Q_snprintf(tmpBuf, BUFSIZELOCL, "%i", i + 1);
        pOrderKVs->SetString(tmpBuf, (*pOrderList)[i]->GetName());
    }

    m_pGamemodeSetupData->SaveToFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD");
}

SpeedometerLabel *SpeedometerData::GetLabelFromName(const char *name)
{
    for (int i = 0; i < SPEEDOMETER_MAX_LABELS; i++)
    {
        const auto pSpeedoLabel = g_pSpeedometer->GetLabel(i);
        if (!Q_strcmp(pSpeedoLabel->GetName(), name))
            return pSpeedoLabel;
    }
    return nullptr;
}

SpeedometerData s_SpeedometerData;
SpeedometerData *g_pSpeedometerData = &s_SpeedometerData;
