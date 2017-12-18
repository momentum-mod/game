#pragma once
#include <include/internal/cef_ptr.h>

class CMomNUIApp;
class CMomNUIFrame;

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
    bool Init(int width, int height, bool debug, bool host);
    void Shutdown();

public:
    //inline CMomNUIFrame* GetFrame() const { return m_pFrame; }

protected:
    bool InitWin32(int width, int height, bool debug, bool host);
    bool InitLinux(int width, int height, bool debug, bool host);
    bool InitOSX(int width, int height, bool debug, bool host);
    bool InitCEF(int width, int height, bool debug, bool host);

protected:
    CefRefPtr<CMomNUIApp> m_pApp;
    CMomNUIFrame* m_pFrame;
    bool m_bShutdown;
    bool m_bInitialized;
};