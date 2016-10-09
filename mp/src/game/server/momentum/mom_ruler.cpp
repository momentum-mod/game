#include "cbase.h"
#include "mom_ruler.h"

#include "tier0/memdbgon.h"

CMOMRulerTool::CMOMRulerTool(const char* pName) : CAutoGameSystem(pName)
{
    m_vFirstPoint = vec3_invalid;
    m_vSecondPoint = vec3_invalid;
}

CON_COMMAND_F(mom_ruler_first, "Creates the first measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    trace_t tr;
    Vector vecFwd;

    AngleVectors(pPlayer->EyeAngles(), &vecFwd);

    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + vecFwd * 1000, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
    if (!tr.DidHit())
        return;
    g_MOMRulerTool->m_vFirstPoint = tr.endpos;
    pPlayer->DecalTrace(&tr, "BirdPoop");
    DevMsg("First point set in (%.4f, %.4f, %.4f)\n", g_MOMRulerTool->m_vFirstPoint.x, g_MOMRulerTool->m_vFirstPoint.y, g_MOMRulerTool->m_vFirstPoint.z);
    
}

CON_COMMAND_F(mom_ruler_second, "Creates the second measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    trace_t tr;
    Vector vecFwd;

    AngleVectors(pPlayer->EyeAngles(), &vecFwd);

    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + vecFwd * 1000, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
    if (!tr.DidHit())
        return;
    g_MOMRulerTool->m_vSecondPoint = tr.endpos;
    pPlayer->DecalTrace(&tr, "BirdPoop");
    DevMsg("Second point set in (%.4f, %.4f, %.4f)\n", g_MOMRulerTool->m_vSecondPoint.x, g_MOMRulerTool->m_vSecondPoint.y, g_MOMRulerTool->m_vSecondPoint.z);
}

CON_COMMAND_F(mom_ruler_measure, "Measures the distance between the first and second point.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    if (g_MOMRulerTool->m_vFirstPoint != vec3_invalid && g_MOMRulerTool->m_vSecondPoint != vec3_invalid)
    {
        // MOM_TODO: Put this somewhere better.. Like, in the "chat"?
        DevMsg("Distance between first point (%.4f, %.4f, %.4f) and second point (%.4f, %.4f, %.4f) is %.4f units.\n", g_MOMRulerTool->m_vFirstPoint.x, g_MOMRulerTool->m_vFirstPoint.y, g_MOMRulerTool->m_vFirstPoint.z,
            g_MOMRulerTool->m_vSecondPoint.x, g_MOMRulerTool->m_vSecondPoint.y, g_MOMRulerTool->m_vSecondPoint.y, g_MOMRulerTool->m_vFirstPoint.DistTo(g_MOMRulerTool->m_vSecondPoint));
    }
    else
    {
        DevWarning("Can't measure distance because both points need to be set.\n");
    }
}

//Expose this to the DLL
static CMOMRulerTool s_MOMRulerTool("MOMRulerTool");
CMOMRulerTool *g_MOMRulerTool = &s_MOMRulerTool;