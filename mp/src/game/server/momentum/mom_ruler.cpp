#include "cbase.h"

#include "mom_ruler.h"
#include <vgui/ILocalize.h>
#include "vgui_controls/InputDialog.h"  // ReSharper, for some reason, found that including this was necessary. Ty!

#include "tier0/memdbgon.h"

// used to ignore solids that are further away than this 
// from the player when setting the ruler's points
static MAKE_CONVAR(mom_ruler_maxlength, "2000", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the maximum length of the measuring tool.\n", 0, MAX_TRACE_LENGTH);
static MAKE_CONVAR(mom_ruler_width, "1.0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the width of the beam that connects the two endpoints.\n", 0, 100);
static MAKE_CONVAR(mom_ruler_duration, "5.5", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the duration of the measurement (in seconds).\n", 0.1, FLT_MAX);

// This will stay a #define
#define RULER_MARKER_MODEL "models/editor/axis_helper.mdl"

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
    Teleport(&dest, nullptr, &vec3_origin);
}

CMOMRulerTool::CMOMRulerTool(const char* pName) : CAutoGameSystem(pName)
{
    // When we are created, is not any different form just getting ourselves Reset()
    Reset();
}

CMOMRulerTool::~CMOMRulerTool()
{
    m_pFirstMark = nullptr;
    m_pSecondMark = nullptr;
    m_pBeamConnector = nullptr;
}

void CMOMRulerTool::PostInit()
{
    LOCALIZE_TOKEN(distanceFormat, "#MOM_Ruler_Distance", m_szDistanceFormat);
}

// Create a laser that will signal that both points are connected
void CMOMRulerTool::ConnectMarks()
{
    if (!m_pFirstMark || !m_pSecondMark) return; // If we can't attach to anything, simply return.
    
    // If the beam doesn't exist, create it
    if (!m_pBeamConnector)
    {
        m_pBeamConnector = CMOMRulerToolBeam::CreateBeam("sprites/laserbeam.vmt", mom_ruler_width.GetFloat());
        m_pBeamConnector->EntsInit(m_pFirstMark, m_pSecondMark);
        m_pBeamConnector->SetColor(115, 80, 255); // MOM_TODO: Potentially make these all customizable?
        m_pBeamConnector->SetBrightness(128);
        m_pBeamConnector->SetFrameRate(1.0f);
        m_pBeamConnector->SetFrame(random->RandomInt(0, 2));
    }

    // Set it to use the custom variables (duration, width, etc)
    m_pBeamConnector->RelinkBeam();
    m_pBeamConnector->SetEndWidth(mom_ruler_width.GetFloat());
    m_pBeamConnector->SetWidth(mom_ruler_width.GetFloat());
    m_pBeamConnector->OnForDuration(mom_ruler_duration.GetFloat());
        
    // Turn it on if it was turned off
    m_pBeamConnector->TurnOn();
}

void CMOMRulerTool::Reset()
{
    // We reset to our default state
    UTIL_RemoveImmediate(m_pFirstMark);
    UTIL_RemoveImmediate(m_pSecondMark);
    UTIL_RemoveImmediate(m_pBeamConnector);
    
    m_pBeamConnector = nullptr;
    m_pFirstMark = nullptr;
    m_pSecondMark = nullptr;

    m_vFirstPoint = vec3_invalid;
    m_vSecondPoint = vec3_invalid;
}

void CMOMRulerTool::DoTrace(const bool bFirst)
{
    CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
    if (!pPlayer)
        return;

    trace_t tr;
    Vector vecFwd;
    AngleVectors(pPlayer->EyeAngles(), &vecFwd);
    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + vecFwd * mom_ruler_maxlength.GetFloat(), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    if (!tr.DidHit())
        return;

    const Color renderColor = bFirst ? Color(255, 255, 255, 255) : Color(0, 0, 0, 255);
    // We have checked if the player is looking at something within the max length units of the ruler itself.
    // The below allows us to set the point location using pointer magic
    Vector *pVec = bFirst ? &m_vFirstPoint : &m_vSecondPoint;
    *pVec = tr.endpos;
    // Yay no more copy/paste boolean conditionals
    CMOMRulerToolMarker **pMark = bFirst ? &m_pFirstMark : &m_pSecondMark;
    // If we're null, we gotta stop being lazy and make something of ourselves
    if (!*pMark)
    {
        *pMark = static_cast<CMOMRulerToolMarker *>(CreateEntityByName("mom_ruler_mark"));
        if (*pMark)
        {
            (*pMark)->Spawn();
            // To distinguish between each mark, the first one is "whiter", second is "blacker"
            (*pMark)->SetRenderColor(renderColor.r(), renderColor.g(), renderColor.b(), renderColor.a());
        }
    }
    // Now we're either created, or were never null in the first place
    if (*pMark)
    {
        if (m_pBeamConnector && m_pBeamConnector->IsOn())
            m_pBeamConnector->TurnOff();
        
        (*pMark)->MoveTo(*pVec);
        DevMsg("%s point set in (%.4f, %.4f, %.4f)\n", bFirst ? "First" : "Second", pVec->x, pVec->y, pVec->z);
    }
}

void CMOMRulerTool::Measure()
{
    if (m_vFirstPoint.IsValid() && m_vSecondPoint.IsValid() && m_pFirstMark && m_pSecondMark)
    {
        // Create the beam that connects the marks
        ConnectMarks();

        if (m_pBeamConnector)
        {
            char distString[BUFSIZ];
            Q_snprintf(distString, BUFSIZ, m_szDistanceFormat, m_vFirstPoint.DistTo(m_vSecondPoint));
            m_pBeamConnector->EntityText(0, distString, mom_ruler_duration.GetFloat());
            Msg(distString);
        }
    }
    else
    {
        // We don't need to output this in chat, but may it be worth it?
        DevWarning("Can't measure distance! Both points need to be set.\n");
    }
}

CON_COMMAND_F(mom_ruler_first, "Creates the first measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    g_MOMRulerTool->DoTrace(true);
}

CON_COMMAND_F(mom_ruler_second, "Creates the second measure point where the player is looking at.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    g_MOMRulerTool->DoTrace(false);
}

CON_COMMAND_F(mom_ruler_measure, "Measures the distance between the first and second point.\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    g_MOMRulerTool->Measure();
}

CON_COMMAND_F(mom_ruler_close, "Closes the menu (ONLY HANDLES LOGIC ABOUT WHAT TO DO WHEN ONCLOSE!)\n", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_HIDDEN)
{
    g_MOMRulerTool->Reset();  // Simple reset to our default values (And deleting any possible mark we've left on the world)
}

//Expose this to the DLL
static CMOMRulerTool s_MOMRulerTool("MOMRulerTool");
CMOMRulerTool *g_MOMRulerTool = &s_MOMRulerTool;