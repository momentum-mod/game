#pragma once

#include "vgui/IVGui.h"

class IChangelogPanel
{
  public:
    virtual void Create(vgui::VPANEL parent) = 0;
    virtual void Destroy(void) = 0;
    virtual void Activate(void) = 0;
    virtual void Close() = 0;
    virtual void SetChangelog(const char *pChangelog) const = 0;
};

extern IChangelogPanel *changelogpanel;