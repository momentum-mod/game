#pragma once

class CMOMClientEvents : public CAutoGameSystem
{
public:
    CMOMClientEvents(const char *pName) : CAutoGameSystem(pName)
    {
    }

    //After DLL inits successfully
    bool Init() OVERRIDE;
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;

    // Precaching things we want to here, done every level start
    // Used mostly for materials
    void Precache();

};