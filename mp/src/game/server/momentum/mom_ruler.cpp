#include "cbase.h"
#include "mom_ruler.h"

#include "tier0/memdbgon.h"

CMOMRulerTool::CMOMRulerTool(const char* pName) : CAutoGameSystem(pName)
{
    m_vFirstPoint = vec3_invalid;
    m_vSecondPoint = vec3_invalid;
}

CON_COMMAND_F(mom_ruler_first, "Creates the first measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    // MOM_TODO: Make it get set on player eyes target, not oprigin, and put some graphical effect
    g_MOMRulerTool->m_vFirstPoint = pPlayer->GetAbsOrigin();
    DevMsg("First point set in (%f, %f, %f)\n", g_MOMRulerTool->m_vFirstPoint);
}

CON_COMMAND_F(mom_ruler_second, "Creates the second measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    // MOM_TODO: Make it get set on player eyes target, not oprigin, and put some graphical effect
    g_MOMRulerTool->m_vSecondPoint = pPlayer->GetAbsOrigin();
    DevMsg("Second point set in (%f, %f, %f)\n", g_MOMRulerTool->m_vFirstPoint);
}

CON_COMMAND_F(mom_ruler_measure, "Measures the distance between the first and second point.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    if (g_MOMRulerTool->m_vFirstPoint != vec3_invalid && g_MOMRulerTool->m_vSecondPoint != vec3_invalid)
    {
        // MOM_TODO: Put this somewhere better.. Like, in the "chat"?
        DevMsg("Distance between first point (%f, %f, %f) and second point (%f, %f, %f) is %f units.\n", g_MOMRulerTool->m_vFirstPoint.x, g_MOMRulerTool->m_vFirstPoint.y, g_MOMRulerTool->m_vFirstPoint.z,
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