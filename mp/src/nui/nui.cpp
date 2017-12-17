#include "nui_predef.h"

#include "filesystem.h"
#include "nui.h"
#include "nui_app.h"
#include "nui_frame.h"
#include "platform.h"

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
    if (!InitCEF(width, height, debug, host))
        return false;

    if (!host)
    {
        m_pFrame = new CMomNUIFrame(width, height);
        if (
            m_pFrame->Init("file://C:\\Users\\Nick\\Documents\\GitHub\\game\\mp\\game\\momentum\\resource\\html\\menu.html")
            //m_pFrame->Init("http://www.google.com")
            )
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
    settings.background_color = 0x00;

    if (debug)
        settings.remote_debugging_port = 8884;

    if (!CefInitialize(args, settings, m_pApp.get(), nullptr))
        return false;

    /*CefRegisterSchemeHandlerFactory("mom", "", m_SchemeHandlerFactory);
    CefAddCrossOriginWhitelistEntry("mom://game", "http", "", true);*/

    m_bInitialized = true;

    return true;
}