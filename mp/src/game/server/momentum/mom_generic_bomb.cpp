//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Exploding bomb
//
//=============================================================================//

#include "cbase.h"
#include "mom_generic_bomb.h"
#include "baseanimating.h"
#include "mom_gamerules.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CMomGenericBomb)
    DEFINE_KEYFIELD(m_flDamage, FIELD_FLOAT, "damage"), 
    DEFINE_KEYFIELD(m_flRadius, FIELD_FLOAT, "radius"),
    DEFINE_KEYFIELD(m_iHealth, FIELD_INTEGER, "health"),
    DEFINE_KEYFIELD(m_strExplodeSoundName, FIELD_SOUNDNAME, "sound"),

    DEFINE_OUTPUT(m_Detonate, "OnDetonate"),

    DEFINE_INPUTFUNC(FIELD_VOID, "Detonate", InputDetonate), 
END_DATADESC();

PRECACHE_REGISTER(momentum_generic_bomb);

LINK_ENTITY_TO_CLASS(momentum_generic_bomb, CMomGenericBomb);

void CMomGenericBomb::Precache()
{
    PrecacheScriptSound(STRING(m_strExplodeSoundName));
    int iModelIndex = PrecacheModel(STRING(GetModelName()));
    PrecacheGibsForModel(iModelIndex);
    BaseClass::Precache();
}

CMomGenericBomb::CMomGenericBomb()
{
    SetMaxHealth(1);
    SetHealth(1);
    m_flDamage = 50.0f;
    m_flRadius = 100.0f;
    m_takedamage = DAMAGE_YES;
}

void CMomGenericBomb::Spawn()
{
    m_takedamage = DAMAGE_YES;

    const char *pModelName = STRING(GetModelName());

    if (!pModelName || !*pModelName)
    {
        Vector vecOrigin = GetAbsOrigin();
        Warning("momentum_generic_bomb at %.0f %.0f %0.f missing modelname\n", vecOrigin.x, vecOrigin.y, vecOrigin.z);
        // Fallback to pumpkin bomb model
        SetModelName(AllocPooledString("models/props_halloween/pumpkin_explode.mdl"));
    }

    Precache();
    SetModel(STRING(GetModelName()));
    SetMoveType(MOVETYPE_NONE);
    SetSolid(SOLID_VPHYSICS);

    BaseClass::Spawn();
}

void CMomGenericBomb::Event_Killed(const CTakeDamageInfo &info)
{
    SetSolid(SOLID_NONE);

    trace_t tr;
    Vector vecForward = GetAbsVelocity();
    VectorNormalize(vecForward);
    UTIL_TraceLine(WorldSpaceCenter(), WorldSpaceCenter() + 60 * vecForward, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

    if (STRING(m_strExplodeSoundName))
        EmitSound(STRING(m_strExplodeSoundName));

    CBaseEntity *pAttacker = this;

    if (info.GetAttacker())
        pAttacker = info.GetAttacker();

    // Using DMG_BLAST causes assertion error in baseentity.cpp (1577)
    CTakeDamageInfo info2(this, pAttacker, m_flDamage, DMG_GENERIC);

    if (tr.m_pEnt && !tr.m_pEnt->IsPlayer())
        UTIL_DecalTrace(&tr, "Scorch");

    m_Detonate.FireOutput(this, this, 0.0f);

    BaseClass::Event_Killed(info2);

    m_takedamage = DAMAGE_NO;

    GameRulesMomentum()->RadiusDamage(info2, GetAbsOrigin(), m_flRadius, CLASS_NONE, this);
}

// same as above but traces the activator as the damage owner
void CMomGenericBomb::InputDetonate(inputdata_t &inputdata)
{
    if (inputdata.pActivator)
    {
        CTakeDamageInfo info(this, inputdata.pActivator, m_flDamage, DMG_GENERIC);
        Event_Killed(info);
    }
}