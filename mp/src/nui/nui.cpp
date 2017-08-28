#include "nui_predef.h"
#include "nui_utils.h"
#include "shaderapi/ishaderapi.h"

#include "nui.h"
#include "nui_app.h"
#include "nui_frame.h"

#ifdef _WIN32
#include "winlite.h"
#include "nui_util_win32.h"
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

CMomNUI::CMomNUI() :
    m_bShutdown(true),
    m_pFrame(nullptr),
    m_bInitialized(false)
{
}

CMomNUI::~CMomNUI()
{
    Shutdown();
}

bool CMomNUI::Init(int width, int height, bool debug, bool host)
{
    if (m_bInitialized)
        return true;

#ifdef _WIN32
    return InitWin32(width, height, debug, host);
#elif defined (__linux__)
    return InitLinux(width, height, debug, host);
#elif defined (__APPLE__)
    return InitOSX(width, height, debug, host);
#endif
}

void CMomNUI::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_pFrame)
    {
        m_pFrame->Client()->Browser()->GetHost()->CloseBrowser(true);
    }

    m_bInitialized = false;

    CefShutdown();
}

bool CMomNUI::InitWin32(int width, int height, bool debug, bool host)
{
#if _WIN32
    if (!host)
    {
        /*auto module = GetModuleHandleA("materialsystem.dll");

        if (!module)
            return false;

        uint8_t* moduleBase = reinterpret_cast<uint8_t*>(module);
        size_t sizeOfCode = CMomNUIUtils::GetSizeOfCode(module);

        // Find the ShaderAPI pointer.
        auto shaderAPIAddr = CMomNUIUtils::SearchPattern(moduleBase, sizeOfCode,
            (uint8_t*) "\x74\xDD\x6A\x00\x68\xDD\xDD\xDD\xDD\xFF\xD6", 11);

        // Find the ShaderDeviceMgr pointer.
        auto shaderDeviceMgrAddr = CMomNUIUtils::SearchPattern(moduleBase, sizeOfCode,
            (uint8_t*) "\x8B\x0D\xDD\xDD\xDD\xDD\x52\x89\x45\xD4", 10);

        if (!shaderAPIAddr || !shaderDeviceMgrAddr)
            return false;

        // This should be the g_pShaderAPI pointer.
        IShaderAPI** shaderAPI = (IShaderAPI**)(*(size_t*)((char*)shaderAPIAddr + 0x13));

        if (!shaderAPI || !*shaderAPI)
            return false;

        // This should be the g_pShaderDeviceMgr pointer.
        IShaderDeviceMgr** shaderDeviceMgr = (IShaderDeviceMgr**)(*(size_t*)((char*)shaderDeviceMgrAddr + 0x02));

        if (!shaderDeviceMgr || !*shaderDeviceMgr)
            return false;

        m_pShaderAPI = *shaderAPI;
        m_pShaderDeviceMgr = (CShaderDeviceMgr*)(*shaderDeviceMgr);*/

        /*auto module = GetModuleHandleA("shaderapidx9.dll");

        if (!module)
            return false;

        uint8_t* moduleBase = reinterpret_cast<uint8_t*>(module);
        size_t sizeOfCode = CMomNUIUtils::GetSizeOfCode(module);

        // Find the D3D device pointer.
        auto deviceAddr = CMomNUIUtils::SearchPattern(moduleBase, sizeOfCode,
            (uint8_t*) "\x68\xE1\x0D\x74\x5E\xA3", 6);

        if (!deviceAddr)
            return false;

        // This should be the g_pShaderAPI pointer.
        IDirect3DDevice9Ex** device = (IDirect3DDevice9Ex**) (*(size_t*) ((char*) deviceAddr + 0x06));

        if (!device || !*device)
            return false;

        m_pDevice = *device;*/
    }

    if (!InitCEF(width, height, debug, host))
        return false;

    if (!host)
    {
        m_pFrame = new CMomNUIFrame(width, height);
        m_pFrame->Init("http://google.com");
        m_pFrame->ShouldRender(true);
    }

    return true;
#else
    return false;
#endif
}

bool CMomNUI::InitLinux(int width, int height, bool debug, bool host)
{
#if defined (__linux__)
    // TODO (OrfeasZ): Support for linux.
    return false;
#else
    return false;
#endif
}

bool CMomNUI::InitOSX(int width, int height, bool debug, bool host)
{
#if defined (__APPLE__)
    // TODO (OrfeasZ): Support for OSX.
    return false;
#else
    return false;
#endif
}

bool CMomNUI::InitCEF(int width, int height, bool debug, bool host)
{
    if (!m_bShutdown)
        return false;

    m_bShutdown = false;

    if (m_pFrame)
        delete m_pFrame;

    if (m_pApp.get())
        delete m_pApp.get();

    m_pApp = new CMomNUIApp();

    CefMainArgs args(GetModuleHandle(NULL));

    int error = CefExecuteProcess(args, m_pApp.get(), nullptr);

    if (error >= 0)
        TerminateProcess(GetCurrentProcess(), 0);

    char hostPath[MAX_PATH];
    
    if (host)
    {
        Q_strcpy(hostPath, "nui_host.exe");
    }
    else
    {
        char relativeHostPath[MAX_PATH];
        V_ComposeFileName("bin", "nui_host.exe", relativeHostPath, MAX_PATH);
        g_pFullFileSystem->RelativePathToFullPath(relativeHostPath, "MOD", hostPath, MAX_PATH);
    }

    CefSettings settings;

    settings.multi_threaded_message_loop = true;
    CefString(&settings.product_version) = "Momentum";
    CefString(&settings.browser_subprocess_path) = hostPath;
    settings.no_sandbox = true;
    settings.pack_loading_disabled = false;
    settings.windowless_rendering_enabled = true;
    settings.ignore_certificate_errors = true;
    settings.log_severity = LOGSEVERITY_DISABLE;
    settings.single_process = false;

    if (debug)
        settings.remote_debugging_port = 8884;

    if (!CefInitialize(args, settings, m_pApp.get(), nullptr))
        return false;

    /*CefRegisterSchemeHandlerFactory("mom", "", m_SchemeHandlerFactory);
    CefAddCrossOriginWhitelistEntry("mom://game", "http", "", true);*/

    m_bInitialized = true;

    return true;
}