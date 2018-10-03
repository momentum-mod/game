#ifndef _MOM_VARIANT_DATA_OPS_H_
#define _MOM_VARIANT_DATA_OPS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "isaverestore.h"
#include "mom_entityoutput.h"
#include "mom_entityinput.h"

class CEventsSaveDataOps : public ISaveRestoreOps
{
public:
	virtual void Save(const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave);
	virtual void Restore(const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore);
	virtual bool IsEmpty(const SaveRestoreFieldInfo_t &fieldInfo);
	virtual void MakeEmpty(const SaveRestoreFieldInfo_t &fieldInfo);
	virtual bool Parse(const SaveRestoreFieldInfo_t &fieldInfo, char const* szValue);
};

class CVariantSaveDataOps : public CDefSaveRestoreOps
{
	// saves the entire array of variables
	virtual void Save(const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave);

	// restores a single instance of the variable
	virtual void Restore(const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore);
	virtual bool IsEmpty(const SaveRestoreFieldInfo_t &fieldInfo);
	virtual void MakeEmpty(const SaveRestoreFieldInfo_t &fieldInfo);
};

extern ISaveRestoreOps *variantFuncs;	// function pointer set for save/restoring variants
extern ISaveRestoreOps *eventFuncs;		// function pointer set for save/restoring events

#endif