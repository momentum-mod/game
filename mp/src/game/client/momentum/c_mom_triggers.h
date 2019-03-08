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

class C_BaseMomentumTrigger : public C_BaseEntity
{
    DECLARE_CLASS(C_BaseMomentumTrigger, C_BaseEntity);
};

class C_TriggerTimerStart : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();
    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
private:
    CTriggerOutlineRenderer m_OutlineRenderer;
};

class C_TriggerTimerStop : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();
    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;

private:
    CTriggerOutlineRenderer m_OutlineRenderer;
};

class C_TriggerSlide : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();
    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
    CNetworkVar(bool, m_bFixUpsideSlope);
};