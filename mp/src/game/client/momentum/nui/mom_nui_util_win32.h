#pragma once

#ifdef _WIN32
#include "cbase.h"
#include "shaderapi/IShaderDevice.h"
#include <d3d9.h>

#undef CreateDevice

class CShaderDeviceMgr :
	public IShaderDevice
{
private:
	char pad01[0x2C];

public:
	IDirect3D9* m_pDevice;
};

#endif