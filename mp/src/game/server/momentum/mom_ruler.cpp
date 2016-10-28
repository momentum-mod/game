#include "cbase.h"

#include "mom_ruler.h"
#include <vgui/ILocalize.h>
#include "vgui_controls/InputDialog.h"  // ReSharper, for some reason, found that including this was necessary. Ty!



// MOM_TODO: Fine tune this value
// used to ignore solids that are further away than this 
// from the player when setting the ruler's points
#define RULER_MAXLENGTH 2000
#define RULER_MARKER_MODEL "models/editor/axis_helper.mdl"
#define RULER_MARKER_LINK_WIDTH 1.0f

LINK_ENTITY_TO_CLASS(mom_ruler_mark, CMOMRulerToolMarker);

void CMOMRulerToolMarker::Precache()
{
    BaseClass::Precache();
    PrecacheModel(RULER_MARKER_MODEL);
    PrecacheMaterial("sprites/laserbeam.spr");
}

void CMOMRulerToolMarker::Spawn()
{
    Precache();
    BaseClass::Spawn();
    SetModel(RULER_MARKER_MODEL);
    SetCollisionGroup(COLLISION_GROUP_NONE);
}

void CMOMRulerToolMarker::MoveTo(const Vector& dest)
{
    // SetAbsOrigin(dest); moves them visibly (Interpolates a litle bit)
    // teleport does not.
    // MOM_TODO: What behaviour is preferred?
    Teleport(&dest, nullptr, &vec3_origin);
}

CMOMRulerTool::CMOMRulerTool(const char* pName) : CAutoGameSystem(pName)
{
    // When we are created, is not any different form just getting ourselves Reset()
    Reset();
}

CMOMRulerTool::~CMOMRulerTool()
{
    // MOM_TODO: Ensure a good destructor (Crashes on exit with current state)
    /*
    if (firstMark)
    {
        firstMark->Remove();
        firstMark = nullptr;
    }

    if (secondMark)
    {
        secondMark->Remove();
        secondMark = nullptr;
    }*/
}

void CMOMRulerTool::ConnectMarks()
{
    if (!firstMark || !secondMark) return; // If we can't attach to anything, simply return.
    // Create a laser that will signal that both points are connected

    if (beam_connector)
    {
        UTIL_Remove(beam_connector);
    }

    beam_connector = CBeam::BeamCreate("sprites/laserbeam.vmt", RULER_MARKER_LINK_WIDTH);
    beam_connector->PointsInit(firstMark->GetAbsOrigin(), secondMark->GetAbsOrigin());
    beam_connector->SetColor(115, 80, 255);
    beam_connector->SetBrightness(128);
    beam_connector->SetNoise(0.0f);
    beam_connector->SetEndWidth(RULER_MARKER_LINK_WIDTH);
    beam_connector->SetWidth(RULER_MARKER_LINK_WIDTH);
    beam_connector->LiveForTime(5.5f);	// Live for 5 and a half seconds
    beam_connector->SetFrameRate(1.0f);
    beam_connector->SetFrame(random->RandomInt(0, 2));

}

void CMOMRulerTool::Reset()
{
    // We reset to our default state
    if (firstMark)
        firstMark->Remove();
    firstMark = nullptr;

    if (secondMark)
        secondMark->Remove();
    secondMark = nullptr;

    if (beam_connector)
        beam_connector->Remove();
    beam_connector = nullptr;

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

    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + vecFwd * RULER_MAXLENGTH, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
    if (!tr.DidHit())
        return;
    // We have checked if the player is looking at something within RULER_MAXLENGTH units of itself. if so, set the point
    g_MOMRulerTool->m_vFirstPoint = tr.endpos;
    if (!g_MOMRulerTool->firstMark)
    {
        g_MOMRulerTool->firstMark = static_cast<CMOMRulerToolMarker *>(CreateEntityByName("mom_ruler_mark"));
        if (g_MOMRulerTool->firstMark)
        {
            g_MOMRulerTool->firstMark->Spawn();
            // To distinguish between each mark, the first one is "wither"
            g_MOMRulerTool->firstMark->SetRenderColor(255, 255, 255, 255);
        }
    }
    if (g_MOMRulerTool->firstMark)
    {
        if (g_MOMRulerTool->beam_connector)
        {
            UTIL_Remove(g_MOMRulerTool->beam_connector);
        }
        g_MOMRulerTool->firstMark->MoveTo(tr.endpos);
        DevMsg("First point set in (%.4f, %.4f, %.4f)\n", g_MOMRulerTool->m_vFirstPoint.x, g_MOMRulerTool->m_vFirstPoint.y, g_MOMRulerTool->m_vFirstPoint.z);
    }
}

CON_COMMAND_F(mom_ruler_second, "Creates the second measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    trace_t tr;
    Vector vecFwd;

    AngleVectors(pPlayer->EyeAngles(), &vecFwd);

    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + vecFwd * RULER_MAXLENGTH, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
    if (!tr.DidHit())
        return;
    // We have checked if the player is looking at something within RULER_MAXLENGTH units of itself. if so, set the point
    g_MOMRulerTool->m_vSecondPoint = tr.endpos;
    if (!g_MOMRulerTool->secondMark)
    {
        g_MOMRulerTool->secondMark = static_cast<CMOMRulerToolMarker *>(CreateEntityByName("mom_ruler_mark"));
        if (g_MOMRulerTool->secondMark)
        {
            g_MOMRulerTool->secondMark->Spawn();
            // To distinguish between each mark, the second one is "blacker"
            g_MOMRulerTool->secondMark->SetRenderColor(0, 0, 0, 255);
        }
    }

    if (g_MOMRulerTool->secondMark)
    {
        if (g_MOMRulerTool->beam_connector)
        {
            UTIL_Remove(g_MOMRulerTool->beam_connector);
        }
        g_MOMRulerTool->secondMark->MoveTo(tr.endpos);
        DevMsg("Second point set in (%.4f, %.4f, %.4f)\n", g_MOMRulerTool->m_vSecondPoint.x, g_MOMRulerTool->m_vSecondPoint.y, g_MOMRulerTool->m_vSecondPoint.z);
    }
}

CON_COMMAND_F(mom_ruler_measure, "Measures the distance between the first and second point.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer || !g_MOMRulerTool)
        return;
    if (g_MOMRulerTool->m_vFirstPoint != vec3_invalid && g_MOMRulerTool->m_vSecondPoint != vec3_invalid)
    {
        g_MOMRulerTool->ConnectMarks();
        CSingleUserRecipientFilter filter(pPlayer);
        filter.MakeReliable();
        // We print the distance to the chat via usermessage
        UserMessageBegin(filter, "SayText");
        WRITE_BYTE(pPlayer->GetClientIndex());
        char printOutText[BUFSIZ];
        char formatText[BUFSIZ];
        LOCALIZE_TOKEN(distance_ruler, "MOM_Ruler_Distance ", printOutText);
        Q_snprintf(formatText, BUFSIZ, printOutText,
            g_MOMRulerTool->m_vFirstPoint.x, g_MOMRulerTool->m_vFirstPoint.y, g_MOMRulerTool->m_vFirstPoint.z,
            g_MOMRulerTool->m_vSecondPoint.x, g_MOMRulerTool->m_vSecondPoint.y, g_MOMRulerTool->m_vSecondPoint.z,
            g_MOMRulerTool->m_vFirstPoint.DistTo(g_MOMRulerTool->m_vSecondPoint));
        WRITE_STRING(formatText);
        MessageEnd();
    }
    else
    {
        // We don't need to output this in chat, but may it be worth it?
        DevWarning("Can't measure distance because both points need to be set.\n");
    }
}

// MOM_TODO: When opening another panel that hides the ruler, this command is not called
CON_COMMAND_F(mom_ruler_close, "Closes the menu (ONLY HANDLES LOGIC ABOUT WHAT TO DO WHEN ONCLOSE!)\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    g_MOMRulerTool->Reset();  // Simple reset to our default values (And deleting any possible mark we've left on the world)
}

//Expose this to the DLL
static CMOMRulerTool s_MOMRulerTool("MOMRulerTool");
CMOMRulerTool *g_MOMRulerTool = &s_MOMRulerTool;