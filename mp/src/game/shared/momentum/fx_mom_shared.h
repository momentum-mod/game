#pragma once

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets(int iEntIndex, const Vector &vOrigin, const QAngle &vAngles, int iWeaponID, int iMode, int iSeed,
                    float flSpread);
