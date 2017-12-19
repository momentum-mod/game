#pragma once

#include "nui/INuiInterface.h"
#include "utlmap.h"
#include <include/internal/cef_ptr.h>
#include "utlqueue.h"

class NuiBrowserListener;
class CefBrowser;
class CMomNUIApp;
class CMomNUIClient;

class CNuiInterface : public INuiInterface
{
public:
    CNuiInterface();
    ~CNuiInterface();

    // INuiInterface Overrides
    bool Init() OVERRIDE;
    void Shutdown() OVERRIDE;
    bool IsInitialized() OVERRIDE { return m_bInitialized; }

    void CreateBrowser(NuiBrowserListener *pListener, const char *pURL) OVERRIDE;
    void ShutdownBrowser(HNUIBrowser& handle) OVERRIDE;
    void LoadURL(HNUIBrowser unBrowserHandle, const char* pchURL, const char* pchPostData) OVERRIDE;
    void StopLoad(HNUIBrowser unBrowserHandle) OVERRIDE;
    void Reload(HNUIBrowser unBrowserHandle) OVERRIDE;
    void GoBack(HNUIBrowser unBrowserHandle) OVERRIDE;
    void GoForward(HNUIBrowser unBrowserHandle) OVERRIDE;
    void ExecuteJavascript(HNUIBrowser unBrowserHandle, const char* pchScript) OVERRIDE;
    void WasResized(HNUIBrowser unBrowserHandle) OVERRIDE;

    void MouseUp(HNUIBrowser& unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton) OVERRIDE;
    void MouseDown(HNUIBrowser& unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton) OVERRIDE;
    void MouseDoubleClick(HNUIBrowser& unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton) OVERRIDE;
    void MouseMove(HNUIBrowser& unBrowserHandle, int x, int y, bool bMouseLeft) OVERRIDE;
    void MouseWheel(HNUIBrowser& unBrowserHandle, int32 nDelta) OVERRIDE;

    void KeyChar(HNUIBrowser unBrowserHandle, wchar_t cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers) OVERRIDE;
    void KeyDown(HNUIBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers) OVERRIDE;
    void KeyUp(HNUIBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers) OVERRIDE;
    void SetKeyFocus(HNUIBrowser unBrowserHandle, bool bHasKeyFocus) OVERRIDE;

    //void SetHorizontalScroll(HNUIBrowser unBrowserHandle, uint32 nAbsolutePixelScroll) OVERRIDE;
    //void SetVerticalScroll(HNUIBrowser unBrowserHandle, uint32 nAbsolutePixelScroll) OVERRIDE;

    // Methods that the NUI Client needs
    NuiBrowserListener *GetBrowserListener(HNUIBrowser handle);
    void OnBrowserCreated(CefRefPtr<CefBrowser> browser);
private:

    CefRefPtr<CefBrowser> GetBrowser(HNUIBrowser handle);

    CefRefPtr<CMomNUIApp> m_pApp;
    CMomNUIClient *m_pNuiClient;

    bool m_bInitialized; // Whether the CEF was initialized or not
    // This queue is used when awaiting the OnAfterCreated call in the Nui Client. It will empty out.
    CUtlQueue<NuiBrowserListener *> m_queueWaitingListeners; // Listeners waiting for their browser
    // This map is used for the callback functions (browser -> panels)
    CUtlMap<HNUIBrowser, NuiBrowserListener*> m_mapListeners; // Map of listeners given their browser handles
    // This map is used for forwarding INuiInterface methods (panels -> browsers)
    CUtlMap<HNUIBrowser, CefRefPtr<CefBrowser>> m_mapBrowsers; // Map of the browsers this interface has created
};