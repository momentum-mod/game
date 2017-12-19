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
    pApp->GetCEFProcessSettings(settings, hostPath);

    CefInitialize(args, settings, pApp, nullptr);

    return 0;
}