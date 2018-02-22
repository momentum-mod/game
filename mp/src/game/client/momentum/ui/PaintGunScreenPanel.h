#pragma once

#include "c_vguiscreen.h"


class PaintGunScreenPanel : public CVGuiScreenPanel
{
    DECLARE_CLASS_SIMPLE(PaintGunScreenPanel, CVGuiScreenPanel);

    PaintGunScreenPanel(Panel *pParent, const char *pName);
    ~PaintGunScreenPanel();

    void Paint() OVERRIDE;

private:
    C_BaseEntity *m_pVguiScreenEntity;
    ConVarRef m_cvarPaintColor, m_cvarDecalScale;

    int m_iDecalTextureID;
};