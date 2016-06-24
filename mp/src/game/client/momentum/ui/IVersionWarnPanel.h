#pragma once

#include "vgui/IVGui.h"

class VersionWarnPanel
{
  public:
    virtual ~VersionWarnPanel() {}
    virtual void Create(vgui::VPANEL parent) = 0;
    virtual void Destroy(void) = 0;
    virtual void Activate(void) = 0;
    virtual void Close() = 0;
    virtual void SetVersion(const char *pVersion) const = 0;
    virtual void SetChangelog(char *pChangelog) const = 0;
};

extern VersionWarnPanel *versionwarnpanel;