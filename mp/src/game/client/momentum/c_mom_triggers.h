#pragma once

class CTriggerOutlineRenderer : public IBrushRenderer
{
public:
    CTriggerOutlineRenderer();
    virtual ~CTriggerOutlineRenderer();
    bool RenderBrushModelSurface(IClientEntity* pBaseEntity, IBrushSurface* pBrushSurface) OVERRIDE;
    Color outlineColor;
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

    virtual bool ShouldDrawOutline() { return false; }
    virtual bool GetOutlineColor() { return false; }

    void DrawOutlineModel();
    bool ShouldDraw() OVERRIDE;
    int DrawModel(int flags) OVERRIDE;

    void DrawSideFacesModelAsOverlay();

    bool IsTwoPass() override { return true; }
    bool IsTransparent() override { return true; }

    int m_iTrackNumber;

    CUtlVector<Vector> m_vecZonePoints;
    float m_flZoneHeight;

protected:
    CTriggerOutlineRenderer m_OutlineRenderer;
};

class C_TriggerTimerStart : public C_BaseMomZoneTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();
    bool ShouldDrawOutline() OVERRIDE;
    bool GetOutlineColor() OVERRIDE;
};

class C_TriggerTimerStop : public C_BaseMomZoneTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDrawOutline() OVERRIDE;
    bool GetOutlineColor() OVERRIDE;
};

class C_TriggerStage : public C_BaseMomZoneTrigger
{
public:
    DECLARE_CLASS(C_TriggerStage, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDrawOutline() OVERRIDE;
    bool GetOutlineColor() OVERRIDE;
};

class C_TriggerCheckpoint : public C_BaseMomZoneTrigger
{
public:
    DECLARE_CLASS(C_TriggerCheckpoint, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDrawOutline() OVERRIDE;
    bool GetOutlineColor() OVERRIDE;
};

class C_TriggerTrickZone : public C_BaseMomZoneTrigger
{
public:
    DECLARE_CLASS(C_TriggerTrickZone, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();

    C_TriggerTrickZone();

    bool ShouldDrawOutline() override;
    bool GetOutlineColor() override;

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