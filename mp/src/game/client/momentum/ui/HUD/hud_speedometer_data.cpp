#include "cbase.h"

#include "hud_speedometer_data.h"
#include "hud_speedometer.h"
#include "hud_speedometer_label.h"

#include "mom_system_gamemode.h"
#include "filesystem.h"
#include "fmtstr.h"

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
    m_pDefaultSpeedoData = new KeyValues("DefaultData");
    m_pGamemodeSetupData = new KeyValues("LoadedData");
}

void SpeedometerData::Init()
{
    if (!m_pDefaultSpeedoData->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_DEFAULT_FILENAME, "MOD"))
    {
        Error("Failed to load default speedometer data; please verify game cache!");
    }

    if (!m_pGamemodeSetupData->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD"))
    {
        m_pGamemodeSetupData->deleteThis();
        m_pGamemodeSetupData = m_pDefaultSpeedoData->MakeCopy();

        m_pGamemodeSetupData->SaveToFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD");
    }
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
    m_pGamemodeSetupData->Clear();
    m_pGamemodeSetupData->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD");

    const auto pszGamemode = g_szGameModes[gametype];

    auto pGamemodeKV = m_pGamemodeSetupData->FindKey(pszGamemode);

    if (!pGamemodeKV)
    {
        // Is it in the default?
        pGamemodeKV = m_pDefaultSpeedoData->FindKey(pszGamemode);

        if (!pGamemodeKV)
        {
            Warning("!!!!!!!!!!!!!!!!!!!!! The gamemode %s does not have its default Speedometer data set. Tell a developer !!!!!!!!!!!!!!!!!!!!!!!!!!!\n", pszGamemode);
            return;
        }

        pGamemodeKV = pGamemodeKV->MakeCopy();
        m_pGamemodeSetupData->AddSubKey(pGamemodeKV);
    }

    m_CurrentlyLoadedGamemodeSettings = gametype;

    // get autolayout. if no custom layout specified, set autolayout on
    KeyValues *pLayoutKV = pGamemodeKV->FindKey("layout");
    bool bAutoLayout = pGamemodeKV->GetBool("autolayout", true) || !pLayoutKV;
    g_pSpeedometer->SetAutoLayout(bAutoLayout);

    if (bAutoLayout)
    {
        g_pSpeedometer->LoadControlSettings(g_pSpeedometer->GetResFile());
        // get ordering
        KeyValues *pOrderKV = pGamemodeKV->FindKey("order");
        if (pOrderKV)
        {
            auto pOrderList = g_pSpeedometer->GetLabelOrderListPtr();
            pOrderList->RemoveAll();
            for (unsigned int i = 1; i <= SPEEDOMETER_MAX_LABELS; i++)
            {
                const char *szSpeedo = pOrderKV->GetString(CFmtStr("%i", i).Get());
                if (szSpeedo[0])
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
    }
    else
    {
        KeyValuesAD layoutKVTmp(pLayoutKV->MakeCopy());
        // get control settings then merge them into the custom layout (keeping its settings)
        KeyValuesAD pSpeedoSettings("SpeedoSettings");
        pSpeedoSettings->LoadFromFile(g_pFullFileSystem, g_pSpeedometer->GetResFile());
        layoutKVTmp->RecursiveMergeKeyValues(pSpeedoSettings);
        g_pSpeedometer->LoadControlSettings(g_pSpeedometer->GetResFile(), NULL, layoutKVTmp);
    }

    // get speedometer label setups
    // must be last as it overrides some control settings loaded above
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
    const auto pGamemodeKV = m_pGamemodeSetupData->FindKey(g_szGameModes[gametype], true);

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
    for (auto i = 0; i < pOrderList->Count(); i++)
    {
        pOrderKVs->SetString(CFmtStr("%i", i + 1).Get(), (*pOrderList)[i]->GetName());
    }

    // set autolayout
    pGamemodeKV->SetBool("autolayout", g_pSpeedometer->GetAutoLayout());

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
