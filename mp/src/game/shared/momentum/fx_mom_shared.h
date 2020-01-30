#pragma once

#ifndef CLIENT_DLL
#include "particle_parse.h"
#include "ammodef.h"
#endif

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets(int iEntIndex, const Vector &vOrigin, const QAngle &vAngles, int iAmmoType, bool bSecondaryMode, int iSeed,
                    float flSpread);

#ifndef CLIENT_DLL
void TE_TFExplosion(IRecipientFilter &filter, const Vector &vecOrigin, const Vector &vecNormal, WeaponID_t iWeaponID);

void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName,
                         ParticleAttachment_t iAttachType, CBaseEntity *pEntity, const char *pszAttachmentName,
                         bool bResetAllParticlesOnEntity = false);
void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName,
                         ParticleAttachment_t iAttachType, CBaseEntity *pEntity = NULL, int iAttachmentPoint = -1,
                         bool bResetAllParticlesOnEntity = false);
void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName, Vector vecOrigin,
                         QAngle vecAngles, CBaseEntity *pEntity = NULL, int iAttachType = PATTACH_CUSTOMORIGIN);
void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName, Vector vecOrigin,
                         Vector vecStart, QAngle vecAngles, CBaseEntity *pEntity = NULL);
void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, int iEffectIndex, Vector vecOrigin, Vector vecStart,
                         QAngle vecAngles, CBaseEntity *pEntity = NULL);
#endif