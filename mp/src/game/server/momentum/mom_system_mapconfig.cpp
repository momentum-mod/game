#include "cbase.h"
#include "fmtstr.h"
#include "filesystem.h"

#include <tier0/memdbgon.h>

#define MAP_CONFIG_FILE "cfg/mapconfigs.cfg"

class CMapConfigSystem : public CAutoGameSystem
{
public:
    CMapConfigSystem();
    virtual ~CMapConfigSystem();

    void AddMapCmd(const CCommand &args);
    void ClearMapCmds();
    void PrintCurrentMapCmds();

protected:
    void PostInit() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

private:
    void SaveMapCmds(); // Saves the map config to file

    uint32 m_iMapCmdCount;
    KeyValues *m_pMapConfigKVs;
    KeyValues *m_pCurrentMapConfig;
};

//Expose this to the DLL
static CMapConfigSystem s_config;
CMapConfigSystem *g_pMapConfigSystem = &s_config;

CMapConfigSystem::CMapConfigSystem(): CAutoGameSystem("CMapConfigSystem"), m_iMapCmdCount(0), m_pMapConfigKVs(nullptr),
                                      m_pCurrentMapConfig(nullptr)
{
}

CMapConfigSystem::~CMapConfigSystem()
{
    if (m_pMapConfigKVs)
        m_pMapConfigKVs->deleteThis();
    m_pMapConfigKVs = nullptr;
}

void CMapConfigSystem::AddMapCmd(const CCommand& args)
{
    if (Q_stristr(args.ArgS(), "mom_mapcfg_add"))
    {
        Warning("No recursion, please!\n");
        return;
    }

    if (m_pCurrentMapConfig)
    {
        m_pCurrentMapConfig->SetString(CFmtStr("%d", ++m_iMapCmdCount).Get(), args.ArgS());
        SaveMapCmds();
        ConColorMsg(COLOR_GREEN, "Added map config command \"%s\"!\n", args.ArgS());
    }
    else if (gpGlobals->mapname.ToCStr())
    {
        m_pCurrentMapConfig = new KeyValues(gpGlobals->mapname.ToCStr());
        m_pMapConfigKVs->AddSubKey(m_pCurrentMapConfig);
        AddMapCmd(args);
    }
}

void CMapConfigSystem::ClearMapCmds()
{
    if (m_pCurrentMapConfig)
    {
        m_pMapConfigKVs->RemoveSubKey(m_pCurrentMapConfig);
        m_pCurrentMapConfig->deleteThis();
        m_pCurrentMapConfig = nullptr;
        m_iMapCmdCount = 0;
        SaveMapCmds();
        ConColorMsg(COLOR_GREEN, "Successfully deleted the map config for the current map!\n");
    }
    else
    {
        Warning("There are no currently saved map config commands!\n");
    }
}

void CMapConfigSystem::PrintCurrentMapCmds()
{
    if (m_pCurrentMapConfig)
    {
        KeyValuesDumpAsDevMsg(m_pCurrentMapConfig, 0, 0);
    }
}

void CMapConfigSystem::PostInit()
{
    m_pMapConfigKVs = new KeyValues(MAP_CONFIG_FILE);

    if (m_pMapConfigKVs->LoadFromFile(g_pFullFileSystem, MAP_CONFIG_FILE, "GAME"))
    {
        DevMsg("Loaded map configs from file %s!\n", MAP_CONFIG_FILE);
    }
}

void CMapConfigSystem::LevelInitPostEntity()
{
    // Execute all of the commands for this map, if there are any
    if (m_pMapConfigKVs)
    {
        m_iMapCmdCount = 0;
        m_pCurrentMapConfig = m_pMapConfigKVs->FindKey(gpGlobals->mapname.ToCStr());
        if (m_pCurrentMapConfig)
        {
            FOR_EACH_VALUE(m_pCurrentMapConfig, cmd)
            {
                engine->ServerCommand(CFmtStr("%s;", cmd->GetString()).Get());
                m_iMapCmdCount++; // Increase our initial count
            }
        }
    }
}

void CMapConfigSystem::LevelShutdownPostEntity()
{
    m_pCurrentMapConfig = nullptr;
}

void CMapConfigSystem::SaveMapCmds()
{
    if (m_pMapConfigKVs && m_pMapConfigKVs->SaveToFile(g_pFullFileSystem, MAP_CONFIG_FILE, "GAME"))
    {
        DevLog(2, "Successfully saved MapConfigs to file!\n");
    }
}

CON_COMMAND(mom_mapcfg_add, "Adds a command or convar to be executed on start of the map currently being played.\n")
{
    g_pMapConfigSystem->AddMapCmd(args);
}

CON_COMMAND(mom_mapcfg_clear, "Clears the commands currently saved for the current map.\n")
{
    g_pMapConfigSystem->ClearMapCmds();
}

CON_COMMAND(mom_mapcfg_print, "Prints out the values currently stored for the current map.\n")
{
    g_pMapConfigSystem->PrintCurrentMapCmds();
}