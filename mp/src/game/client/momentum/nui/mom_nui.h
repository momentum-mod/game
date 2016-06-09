#pragma once

#include "cbase.h"

class IShaderAPI;
class CShaderDeviceMgr;

class CMomNUI
{
public:
	static CMomNUI* GetInstance();
	static void DestroyInstance();

protected:
	static CMomNUI* m_pInstance;

protected:
	CMomNUI();
	~CMomNUI();

public:
	bool Init(bool debug);
	void Shutdown();

protected:
	bool InitWin32(bool debug);
	bool InitLinux(bool debug);
	bool InitOSX(bool debug);

protected:
#ifdef _WIN32
	IShaderAPI* m_pShaderAPI;
	CShaderDeviceMgr* m_pShaderDeviceMgr;
#endif
};