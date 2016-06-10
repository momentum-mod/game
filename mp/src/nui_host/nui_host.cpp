#include "nui_host.h"
#include "nui.h"

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    CMomNUI::GetInstance()->Init(1, 1, false, true);
    return 0;
}