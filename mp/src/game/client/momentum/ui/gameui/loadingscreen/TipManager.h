#pragma once

#include "mom_shareddefs.h"

struct GameModeTips;

class CTipManager
{
public:
    CTipManager();

    const char *GetTipForGamemode(GameMode_t gameMode);

private:
    CUtlVector<GameModeTips *> m_vecTips;
};