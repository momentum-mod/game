#pragma once

#include "cbase.h"

#include <vgui_controls/Panel.h>

enum
{
    RDRAG_NONE = 0,
    RDRAG_ROTATE,
    RDRAG_LIGHT,
    RDRAG_POS,
};

class CModelPanelModel : public C_BaseFlex
{
public:
    CModelPanelModel() {}
    DECLARE_CLASS(CModelPanelModel, C_BaseFlex);

    bool IsMenuModel() const OVERRIDE { return true; }
    bool ShouldInterpolate() OVERRIDE { return false; }
};

class CRenderPanel : public vgui::Panel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(CRenderPanel, vgui::Panel);

public:
    CRenderPanel(Panel *parent, const char *pElementName);
    ~CRenderPanel();

    // VGUI Overrides
    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
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
    bool IsModelReady();
    CModelPanelModel *GetModel() { return m_pModelInstance;}
    void ResetModel();
    void GetModelCenter(Vector &vecInto);
    void DrawModel();
    void SetRenderColors(C_BaseEntity *pEnt);

    void FireGameEvent(IGameEvent* event) OVERRIDE;

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

    CModelPanelModel *m_pModelInstance;
    char m_szModelPath[MAX_PATH];

    bool m_bSizeToParent;

};