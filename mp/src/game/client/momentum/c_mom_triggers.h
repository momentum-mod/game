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

    void DrawOutlineModel(const Color &outlineColor);
    bool ShouldDraw() OVERRIDE;
    int DrawModel(int flags) OVERRIDE;

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

class C_TriggerSlide : public C_BaseMomZoneTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, C_BaseMomZoneTrigger);
    DECLARE_CLIENTCLASS();
    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
};