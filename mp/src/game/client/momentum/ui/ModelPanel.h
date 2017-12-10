#pragma once

#include "cbase.h"
using namespace vgui;

#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Controls.h>

enum
{
    RDRAG_NONE = 0,
    RDRAG_ROTATE,
    RDRAG_LIGHT,
    RDRAG_POS,
};

class CRenderPanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CRenderPanel, vgui::Panel);

public:
    CRenderPanel(Panel *parent, const char *pElementName);
    ~CRenderPanel();

    // VGUI Overrides
    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;
    void OnMousePressed(MouseCode code) OVERRIDE;
    void OnMouseReleased(MouseCode code) OVERRIDE;
    void OnCursorMoved(int x, int y) OVERRIDE;
    void OnMouseWheeled(int delta) OVERRIDE;

    // View stuff
    void SetupView(CViewSetup &setup);
    void UpdateRenderPosition();
    void ResetView();
    void SizePanelToParent();
    void SetShouldSizeToParent(bool b) { m_bSizeToParent = b; SizePanelToParent(); }
    
    // Model stuff
    bool LoadModel(const char *path);
    void DestroyModel();
    C_BaseFlex *GetModel() const { return m_pModelInstance; }
    bool IsModelReady();
    void ResetModel();
    void GetModelCenter(Vector &vecInto);
    void DrawModel();
    void SetRenderColors(C_BaseEntity *pEnt);

private:
    QAngle render_ang;
    Vector render_pos;
    Vector render_offset;
    Vector render_offset_modelBase;
    QAngle lightAng;

    float m_flDist;
    float m_flPitch;
    float m_flYaw;
    int m_nFOV;

    Frustum frustum;
    int m_iDragMode;
    int m_iCachedMpos_x;
    int m_iCachedMpos_y;

    VMatrix __view;
    VMatrix __proj;
    VMatrix __ViewProj;
    VMatrix __ViewProjNDC;

    C_BaseFlex *m_pModelInstance;
    char m_szModelPath[MAX_PATH];
    int m_iNumPoseParams;

    bool m_bSizeToParent;
};