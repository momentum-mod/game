#include "cbase.h"
#include "mom_nui.h"
#include "util/mom_util.h"
#include "shaderapi/ishaderapi.h"

#ifdef _WIN32
#include "winlite.h"
#include "mom_nui_util_win32.h"
#endif

CMomNUI* CMomNUI::m_pInstance = nullptr;

CMomNUI* CMomNUI::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new CMomNUI();

	return m_pInstance;
}

void CMomNUI::DestroyInstance()
{
	if (!m_pInstance)
		return;

	delete m_pInstance;
	m_pInstance = nullptr;
}

//////////////////////////////////////////////////////////////////////////

CMomNUI::CMomNUI()
{
#ifdef _WIN32
	m_pShaderAPI = nullptr;
	m_pShaderDeviceMgr = nullptr;
#endif
}

CMomNUI::~CMomNUI()
{

}

bool CMomNUI::Init(bool debug)
{
#ifdef _WIN32
	return InitWin32(debug);
#elif defined (__linux__)
	return InitLinux(debug);
#elif defined (__APPLE__)
	return InitOSX(debug);
#endif
}

void CMomNUI::Shutdown()
{

}

bool CMomNUI::InitWin32(bool debug)
{
#if _WIN32
	auto module = GetModuleHandleA("materialsystem.dll");

	if (!module)
		return false;

	uint8_t* moduleBase = reinterpret_cast<uint8_t*>(module);
	size_t sizeOfCode = mom_UTIL->GetSizeOfCode(module);

	// Find the ShaderAPI pointer.
	auto shaderAPIAddr = mom_UTIL->SearchPattern(moduleBase, sizeOfCode,
		(uint8_t*) "\x74\xDD\x6A\x00\x68\xDD\xDD\xDD\xDD\xFF\xD6", 11);

	// Find the ShaderDeviceMgr pointer.
	auto shaderDeviceMgrAddr = mom_UTIL->SearchPattern(moduleBase, sizeOfCode,
		(uint8_t*) "\x8B\x0D\xDD\xDD\xDD\xDD\x52\x89\x45\xD4", 10);

	if (!shaderAPIAddr || !shaderDeviceMgrAddr)
		return false;

	// This should be the g_pShaderAPI pointer.
	IShaderAPI** shaderAPI = (IShaderAPI**) (*(size_t*) ((char*) shaderAPIAddr + 0x13));

	if (!shaderAPI || !*shaderAPI)
		return false;

	// This should be the g_pShaderDeviceMgr pointer.
	IShaderDeviceMgr** shaderDeviceMgr = (IShaderDeviceMgr**) (*(size_t*) ((char*) shaderDeviceMgrAddr + 0x02));

	if (!shaderDeviceMgr || !*shaderDeviceMgr)
		return false;

	m_pShaderAPI = *shaderAPI;
	m_pShaderDeviceMgr = (CShaderDeviceMgr*) (*shaderDeviceMgr);

	return true;
#else
	return false;
#endif
}

bool CMomNUI::InitLinux(bool debug)
{
#if defined (__linux__)
	// TODO (OrfeasZ): Support for linux.
	return false;
#else
	return false;
#endif
}

bool CMomNUI::InitOSX(bool debug)
{
#if defined (__APPLE__)
	// TODO (OrfeasZ): Support for OSX.
	return false;
#else
	return false;
#endif
}