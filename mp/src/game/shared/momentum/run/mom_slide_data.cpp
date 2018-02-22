#include "cbase.h"
#include "mom_slide_data.h"

CMomPlayerSlideData::CMomPlayerSlideData() { Reset(); }

CMomPlayerSlideData::~CMomPlayerSlideData() { Reset(); }

CMomPlayerSlideData::CMomPlayerSlideData(const CMomPlayerSlideData &src) { *this = src; }

CMomPlayerSlideData &CMomPlayerSlideData::operator=(const CMomPlayerSlideData &src)
{
    memcpy(this, &src, sizeof(CMomPlayerSlideData));
    return *this;
}
