#pragma once

class CTriggerOutlineRenderer : public IBrushRenderer
{
public:
    CTriggerOutlineRenderer();
    virtual ~CTriggerOutlineRenderer();
    bool RenderBrushModelSurface(IClientEntity* pBaseEntity, IBrushSurface* pBrushSurface) override;
    Color m_Color;
    int m_iRenderMode;
private:
    BrushVertex_t *m_pVertices;
    int m_vertexCount;
};

class C_BaseMomZoneTrigger : public C_BaseEntity
{
    DECLARE_CLASS(C_BaseMomZoneTrigger, C_BaseEntity);
    DECLARE_CLIENTCLASS();

public:
    C_BaseMomZoneTrigger();

    virtual bool ShouldDrawModel() { return false; }
    virtual bool GetDrawColor() { return false; }
    virtual int GetDrawMode() { return 0; }

    void DrawOutlineModel();

    void DrawSideFacesModelAsBrush();
    void DrawSideFacesModelAsOverlay();

    bool ShouldDraw() override { return true; }
    int DrawModel(int flags) override;
    bool IsTwoPass() override { return true; }
    bool IsTransparent() override { return true; }

    int m_iTrackNumber;

    CUtlVector<Vector> m_vecZonePoints;
    float m_flZoneHeight;

protected:
    CTriggerOutlineRenderer m_ZoneModelRenderer;
};

class C_TriggerTimerStart : public C_BaseMomZoneTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool GetDrawColor() override;
    int GetDrawMode() override;
};

class C_TriggerTimerStop : public C_BaseMomZoneTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool GetDrawColor() override;
    int GetDrawMode() override;
};

class C_TriggerStage : public C_BaseMomZoneTrigger
{
public:
    DECLARE_CLASS(C_TriggerStage, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool GetDrawColor() override;
    int GetDrawMode() override;
};

class C_TriggerCheckpoint : public C_BaseMomZoneTrigger
{
public:
    DECLARE_CLASS(C_TriggerCheckpoint, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool GetDrawColor() override;
    int GetDrawMode() override;
};

class C_TriggerTrickZone : public C_BaseMomZoneTrigger
{
public:
    DECLARE_CLASS(C_TriggerTrickZone, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    C_TriggerTrickZone();

    bool GetDrawColor() override;
    int GetDrawMode() override;

    void OnDataChanged(DataUpdateType_t type) override;

    CNetworkVar(int, m_iID);
    CNetworkString(m_szZoneName, 32);

    CNetworkVar(int, m_iDrawState);
};

class C_TriggerSlide : public C_BaseMomZoneTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();
    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
};