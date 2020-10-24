#pragma once

#include "mom_shareddefs.h"

class SpeedometerLabel;

class SpeedometerData
{
  public:
    SpeedometerData();

    void Init();

    void Load(bool bApply = false);
    void Save(bool bApply = false);
    void Apply();

    void SetVisible(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType, bool bVisible);
    bool GetVisible(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const;

    void SetColorize(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType, SpeedometerColorize_t speedometerColorizeType);
    SpeedometerColorize_t GetColorize(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const;

    void SetUnits(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType, SpeedometerUnits_t speedometerUnitsType);
    SpeedometerUnits_t GetUnits(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const;

  private:
    bool LoadDefaultSubKeyData(KeyValues *pGamemodeKV, KeyValues *pDefaultGamemodeKV, const char *pszKeyName);

    KeyValues *GetSpeedoKVs(GameMode_t gametype, SpeedometerLabel_t speedometerLabelType) const;

    KeyValues *m_pDefaultSpeedoData;
    KeyValues *m_pSpeedoData;
};

extern SpeedometerData *g_pSpeedometerData;
