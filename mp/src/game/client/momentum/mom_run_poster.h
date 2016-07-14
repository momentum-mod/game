#ifndef MOMRUNPOSTER_H
#define MOMRUNPOSTER_H
#ifdef WIN32
#pragma once
#endif

#include "cbase.h"

#include "filesystem.h"
#include "mom_event_listener.h"
#include "momentum/mom_shareddefs.h"

class CRunPoster
{
public:
    DECLARE_CLASS_NOBASE(CRunPoster);
    CRunPoster();
    ~CRunPoster();
};

#endif //MOMRUNPOSTER_H