#pragma once

#include "mom_shareddefs.h"

class SpeedometerLabel;

class SpeedometerData
{
  public:
    SpeedometerData();

    // gets what game mode pertains to the settings currently loaded
    GameMode_t GetCurrentlyLoadedGameMode() { return m_CurrentlyLoadedGamemodeSettings; }

    void Init();

    void LoadGamemodeData();
    void LoadGamemodeData(int gametype);
    void LoadGamemodeData(GameMode_t gametype);

    void SaveGamemodeData();
    void SaveGamemodeData(int gametype);
    void SaveGamemodeData(GameMode_t gametype);

  private:
    SpeedometerLabel *GetLabelFromName(const char *name);

    KeyValues *m_pDefaultSpeedoData;
    KeyValues *m_pGamemodeSetupData;
    GameMode_t m_CurrentlyLoadedGamemodeSettings;
};

extern SpeedometerData *g_pSpeedometerData;
