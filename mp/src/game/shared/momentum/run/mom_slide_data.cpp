#include "cbase.h"
#include "mom_slide_data.h"

CMomPlayerSlideData::CMomPlayerSlideData() : m_flCurrentTriggerMinZ()
{
    Reset();
    /*
    for (int i = 0; i != MAX_EDICTS + 1; i++)
        m_bTouchingTrigger[i] = false;
    */
}

CMomPlayerSlideData::~CMomPlayerSlideData()
{
    Reset();
    /*
    for (int i = 0; i != MAX_EDICTS + 1; i++)
        m_bTouchingTrigger[i] = false;
    */
}
