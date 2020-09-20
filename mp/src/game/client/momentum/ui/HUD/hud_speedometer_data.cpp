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
#define SPEEDOMETER_KEY_VISIBLE "visible"
#define SPEEDOMETER_KEY_COLORIZE "colorize"
#define SPEEDOMETER_KEY_UNITS "units"
#define SPEEDOMETER_KEY_LAYOUT "layout"
#define SPEEDOMETER_KEY_AUTOLAYOUT "autolayout"
#define SPEEDOMETER_KEY_ORDER "order"

CON_COMMAND_F(mom_hud_speedometer_loadcfg, "Loads the speedometer setup for the current gamemode from file.\n",
              FLAG_HUD_CVAR | FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSpeedometerData->Load(true);
}

SpeedometerData::SpeedometerData()
{
    m_pDefaultSpeedoData = new KeyValues("DefaultData");
    m_pSpeedoData = new KeyValues("LoadedData");
}

void SpeedometerData::Init()
{
    if (!m_pDefaultSpeedoData->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_DEFAULT_FILENAME, "MOD"))
    {
        Error("Failed to load default speedometer data; please verify game cache!");
    }

    Load();
}

void SpeedometerData::Load(bool bApply)
{
    m_pSpeedoData->Clear();
    if (!m_pSpeedoData->LoadFromFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD"))
    {
        m_pSpeedoData->deleteThis();
        m_pSpeedoData = m_pDefaultSpeedoData->MakeCopy();

        Save();
    }

    if (bApply)
        Apply();
}

void SpeedometerData::Save(bool bApply)
{
    m_pSpeedoData->SaveToFile(g_pFullFileSystem, SPEEDOMETER_FILENAME, "MOD");

    if (bApply)
        Apply();
}

void SpeedometerData::Apply() 
{ 
    const auto gametype = g_pGameModeSystem->GetGameMode()->GetType();
    const auto pszGamemode = g_szGameModes[gametype];

    bool bShouldSave = false;
    
    auto pGamemodeKV = m_pSpeedoData->FindKey(pszGamemode);
    auto pDefaultGamemodeKV = m_pDefaultSpeedoData->FindKey(pszGamemode);
    if (!pGamemodeKV)
    {
        // Is it in the default?
        if (!pDefaultGamemodeKV)
        {
            Warning("!!! The gamemode %s does not have its default Speedometer data set. Tell a developer !!!\n", pszGamemode);
            return;
        }

        pGamemodeKV = pDefaultGamemodeKV->MakeCopy();
        m_pSpeedoData->AddSubKey(pGamemodeKV);
        bShouldSave = true;
    }

    // get autolayout. if no custom layout specified, set autolayout on
    auto pLayoutKV = pGamemodeKV->FindKey(SPEEDOMETER_KEY_LAYOUT);
    bool bAutoLayout = pGamemodeKV->GetBool(SPEEDOMETER_KEY_AUTOLAYOUT, true) || !pLayoutKV;
    g_pSpeedometer->SetAutoLayout(bAutoLayout);

    if (bAutoLayout)
    {
        g_pSpeedometer->LoadControlSettings(g_pSpeedometer->GetResFile());

        // get ordering
        auto pOrderKV = pGamemodeKV->FindKey(SPEEDOMETER_KEY_ORDER);
        if (!pOrderKV)
        {
            if (LoadDefaultSubKeyData(pGamemodeKV, pDefaultGamemodeKV, SPEEDOMETER_KEY_ORDER))
            {
                pOrderKV = pGamemodeKV->FindKey(SPEEDOMETER_KEY_ORDER);
                bShouldSave = true;
            }
            else
            {
                Warning("!!! The speedometer ordering for gamemode %s does not exist in the default Speedometer data set. Tell a developer !!!\n", pszGamemode);
                g_pSpeedometer->ResetLabelOrder();
            }
        }

        if (pOrderKV)
        {
            auto pOrderList = g_pSpeedometer->GetLabelOrderListPtr();
            pOrderList->RemoveAll();
            for (unsigned int i = 1; i <= SPEEDOMETER_LABEL_TYPE_COUNT; i++)
            {
                const auto pszIndex = CFmtStr("%i", i).Get();
                bool bFound = false;
                for (unsigned int j = 0; j < SPEEDOMETER_LABEL_TYPE_COUNT; j++)
                {
                    const auto pSpeedoLabel = g_pSpeedometer->GetLabel(j);
                    if (FStrEq(pSpeedoLabel->GetName(), pOrderKV->GetString(pszIndex)))
                    {
                        pOrderList->AddToTail(pSpeedoLabel);
                        bFound = true;
                        break;
                    }
                }

                if (bFound)
                    continue;

                Warning("! The speedometer ordering for gamemode %s has at least one incorrect or missing speedometer type. Loading default !\n", pszGamemode);
                pGamemodeKV->RemoveSubKey(pOrderKV);
                if (LoadDefaultSubKeyData(pGamemodeKV, pDefaultGamemodeKV, SPEEDOMETER_KEY_ORDER))
                {
                    bShouldSave = true;
                }
                else
                {
                    Warning("!!! The speedometer ordering for gamemode %s does not exist in the default Speedometer data set. Tell a developer !!!\n", pszGamemode);
                }
                g_pSpeedometer->ResetLabelOrder();
                break;
            }
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
    for (unsigned int i = 0; i < SPEEDOMETER_LABEL_TYPE_COUNT; i++)
    {
        const auto pSpeedoLabel = g_pSpeedometer->GetLabel(i);
        const auto pszSpeedo = pSpeedoLabel->GetName();
        auto pSpeedoKV = pGamemodeKV->FindKey(pszSpeedo);

        if (!pSpeedoKV)
        {
            if (!LoadDefaultSubKeyData(pGamemodeKV, pDefaultGamemodeKV, pszSpeedo))
            {
                Warning("!!! The speedometer %s for gamemode %s does not exist in the default Speedometer data set. Tell a developer !!!\n", pszSpeedo, pszGamemode);
                continue;
            }
            pSpeedoKV = pGamemodeKV->FindKey(pszSpeedo);
            bShouldSave = true;
        }

        pSpeedoLabel->ApplyKV(pSpeedoKV);
    }

    if (bShouldSave)
        Save();
}

bool SpeedometerData::LoadDefaultSubKeyData(KeyValues *pGamemodeKV, KeyValues *pDefaultGamemodeKV, const char *pszKeyName)
{
    auto pDefaultSubKeyKV = pDefaultGamemodeKV->FindKey(pszKeyName);
    if (!pDefaultSubKeyKV)
        return false;

    pGamemodeKV->AddSubKey(pDefaultSubKeyKV->MakeCopy());
    return true;
}

void SpeedometerData::SetVisible(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType, bool bVisible) 
{
    GetSpeedoKVs(gametype, speedometerLabelType)->SetBool(SPEEDOMETER_KEY_VISIBLE, bVisible);
}

bool SpeedometerData::GetVisible(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const
{
    return GetSpeedoKVs(gametype, speedometerLabelType)->GetBool(SPEEDOMETER_KEY_VISIBLE, true);
}

void SpeedometerData::SetColorize(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType, SpeedometerColorize_t speedometerColorizeType)
{
    GetSpeedoKVs(gametype, speedometerLabelType)->SetInt(SPEEDOMETER_KEY_COLORIZE, speedometerColorizeType);
}

SpeedometerColorize_t SpeedometerData::GetColorize(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const
{
    return SpeedometerColorize_t(GetSpeedoKVs(gametype, speedometerLabelType)->GetInt(SPEEDOMETER_KEY_COLORIZE, SPEEDOMETER_COLORIZE_COMPARISON));
}

void SpeedometerData::SetUnits(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType, SpeedometerUnits_t speedometerUnitsType)
{
    GetSpeedoKVs(gametype, speedometerLabelType)->SetInt(SPEEDOMETER_KEY_UNITS, speedometerUnitsType);
}

SpeedometerUnits_t SpeedometerData::GetUnits(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const
{
    return SpeedometerUnits_t(GetSpeedoKVs(gametype, speedometerLabelType)->GetInt(SPEEDOMETER_KEY_UNITS, SPEEDOMETER_UNITS_UPS));
}

KeyValues *SpeedometerData::GetSpeedoKVs(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const
{
    const auto pGamemodeKVs = m_pSpeedoData->FindKey(g_szGameModes[gametype], true);
    return pGamemodeKVs->FindKey(g_pSpeedometer->GetLabel(speedometerLabelType)->GetName(), true);
}

SpeedometerData s_SpeedometerData;
SpeedometerData *g_pSpeedometerData = &s_SpeedometerData;
