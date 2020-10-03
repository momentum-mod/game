#pragma once

#include <vgui_controls/Panel.h>

enum DragRotateMode_t
{
    RDRAG_NONE = 0,
    RDRAG_ROTATE = 1 << 0,
    RDRAG_POS = 1 << 1,
    RDRAG_LIGHT = 1 << 2,

    RDRAG_ALL = ~RDRAG_NONE,
};

enum RotateMode_t
{
    ROTATE_NONE = 0,
    ROTATE_PITCH = 1 << 0,
    ROTATE_YAW   = 1 << 1,

    ROTATE_ALL   = ~ROTATE_NONE,
};

class C_ModelPanelModel : public C_BaseFlex
{
public:
    DECLARE_CLASS(C_ModelPanelModel, C_BaseFlex);
    DECLARE_CLIENTCLASS();

    C_ModelPanelModel() {}

    bool IsMenuModel() const OVERRIDE { return true; }
    bool ShouldInterpolate() override { return false; }
};

class CRenderPanel : public vgui::Panel, public CGameEventListener
{
public:
    DECLARE_CLASS_SIMPLE(CRenderPanel, vgui::Panel);

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

    int GetFOV() const { return m_nFOV; }
    void SetFOV(int iNewFOV) { m_nFOV = iNewFOV; }

    float GetPitch() const { return m_flPitch; }
    void SetPitch(float fPitch) { m_flPitch = fPitch; }

    float GetYaw() const { return m_flYaw; }
    void SetYaw(float fYaw) { m_flYaw = fYaw; }

    float GetDist() const { return m_flDist; }
    void SetDist(float fDist) { m_flDist = fDist; }

    void SetCameraDefaults(int nFOV = 60, float fPitch = 45.0f, float fYaw = 180.0f, float fDist = 100.0f);
    void SetDefaultLightAngle(const QAngle &angle) { m_angLightDefault = angle; }
    
    // Model stuff
    bool LoadModel(const char *path);
    void DestroyModel();
    bool IsModelReady();
    C_ModelPanelModel *GetModel() const { return m_pModelInstance; }
    void ReloadModel();
    void ResetModel();
    void GetModelCenter(Vector &vecInto);
    void DrawModel();
    void SetRenderColors(C_BaseEntity *pEnt);

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    int GetDragMode() const { return m_iDragMode; }
    float GetLastDragTime() const { return m_flLastDragTime; }
    void SetAllowedDragModes(int iAllowedDrags) { m_nAllowedDragModes = iAllowedDrags; }
    void SetAllowedRotateModes(int iAllowedRotate) { m_nAllowedRotateModes = iAllowedRotate; }

    bool ShouldAutoRotateModel() const { return m_bShouldAutoRotateModel; }
    void SetShouldAutoRotateModel(bool bRotate);
    QAngle &GetAutoModelRotationSpeed() { return m_angAutoRotation; }
    void SetAutoModelRotationSpeed(const QAngle &angle);

private:
    float m_flPitch, m_flDefaultPitch;
    float m_flYaw, m_flDefaultYaw;
    float m_flDist, m_flDefaultDist;
    int m_nFOV, m_nDefaultFOV;
    QAngle m_angAutoRotation;

    QAngle render_ang;
    Vector render_pos;
    Vector render_offset;
    Vector render_offset_modelBase;
    QAngle lightAng, m_angLightDefault;

    Frustum frustum;
    int m_nAllowedRotateModes;
    int m_nAllowedDragModes;
    int m_iDragMode;
    int m_iCachedMpos_x;
    int m_iCachedMpos_y;
    float m_flLastDragTime;

    VMatrix __view;
    VMatrix __proj;
    VMatrix __ViewProj;
    VMatrix __ViewProjNDC;

    CTextureReference m_hDefaultCubemap;
    C_ModelPanelModel *m_pModelInstance;
    char m_szModelPath[MAX_PATH];

    bool m_bSizeToParent;
    bool m_bShouldAutoRotateModel;
};