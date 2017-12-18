#include <include/internal/cef_win.h>
#include <include/internal/cef_ptr.h>
#include "nui_app.h"

#ifdef _WIN32
int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR lpCmdLine,
    int nCmdShow
)
#else
int main(int argc, char** argv)
#endif
{
    CefMainArgs args(
#ifdef _WIN32
        hInstance
#else
        argc, argv
#endif
        );

    CefRefPtr<CMomNUIApp> pApp(new CMomNUIApp);

    int error = CefExecuteProcess(args, pApp, nullptr);

    if (error >= 0)
        return error;

    char hostPath[MAX_PATH];
    strcpy(hostPath, "nui_host.exe"); // MOM_TODO: Linux and mac use different endings here
    
    CefSettings settings;
    settings.multi_threaded_message_loop = true;
    CefString(&settings.product_version) = "Momentum";
    CefString(&settings.browser_subprocess_path).FromASCII(hostPath);
    settings.no_sandbox = true;
    settings.pack_loading_disabled = false;
    settings.windowless_rendering_enabled = true;
    settings.ignore_certificate_errors = true;
    settings.log_severity = LOGSEVERITY_DISABLE;
    settings.single_process = false;
    settings.background_color = 0x00;

    //if (debug)
    //    settings.remote_debugging_port = 8884;

    CefInitialize(args, settings, pApp, nullptr);

    return 0;
}