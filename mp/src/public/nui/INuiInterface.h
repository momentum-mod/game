#pragma once

#include "interface.h"

typedef int HNUIBrowser;
const uint32 INVALID_NUIBROWSER = 0;
class NuiBrowserListener;

abstract_class INuiInterface : public IBaseInterface
{
public:
    // Initialize the CEF process. Returns true if successfully initted, else false.
    // Once successfully initted, a browser can be created with CreateBrowser().
    virtual bool Init(int argc, char** argv) = 0;
    // Shut down the CEF process entirely. After calling this method,
    // any of these interface methods will not work until Init is called again.
    virtual void Shutdown() = 0;

    // Returns true if this interface has been initialized, else false.
    virtual bool IsInitialized() = 0;

    // Creates a browser for a given NuiBrowserListener.
    virtual void CreateBrowser(NuiBrowserListener *pListener, const char *pURL = nullptr) = 0;
    // Shuts down and frees the resources of the given browser.
    virtual void ShutdownBrowser(HNUIBrowser &handle) = 0;

    // Navigate to this URL, results in a HTML_StartRequest_t as the request commences
    // pchPostData can be null if the request is a simple GET request
    virtual void LoadURL(HNUIBrowser unBrowserHandle, const char *pchURL, const char *pchPostData) = 0;

    // Stop the load of the current html page
    virtual void StopLoad(HNUIBrowser unBrowserHandle) = 0;
    // Reload (most likely from local cache) the current page
    virtual void Reload(HNUIBrowser unBrowserHandle) = 0;
    // navigate back in the page history
    virtual void GoBack(HNUIBrowser unBrowserHandle) = 0;
    // navigate forward in the page history
    virtual void GoForward(HNUIBrowser unBrowserHandle) = 0;

    // run this javascript script in the currently loaded page
    virtual void ExecuteJavascript(HNUIBrowser unBrowserHandle, const char *pchScript) = 0;

    // Tells the CEF browser that the panel was resized, and needs re-calculating and repainting
    virtual void WasResized(HNUIBrowser unBrowserHandle) = 0;

    enum EHTMLMouseButton
    {
        eHTMLMouseButton_Left = 0,
        eHTMLMouseButton_Middle = 1,
        eHTMLMouseButton_Right = 2
    };

    // Mouse click and mouse movement commands
    virtual void MouseUp(HNUIBrowser &unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton) = 0;
    virtual void MouseDown(HNUIBrowser &unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton) = 0;
    virtual void MouseDoubleClick(HNUIBrowser &unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton) = 0;
    // x and y are relative to the HTML bounds, mouseLeft is if the cursor exited
    virtual void MouseMove(HNUIBrowser &unBrowserHandle, int x, int y, bool bMouseLeft) = 0;
    // nDelta is pixels of scroll
    virtual void MouseWheel(HNUIBrowser &unBrowserHandle, int32 nDelta) = 0;

    enum EHTMLKeyModifiers
    {
        k_eHTMLKeyModifier_None = 0,
        k_eHTMLKeyModifier_AltDown = 1 << 0,
        k_eHTMLKeyModifier_CtrlDown = 1 << 1,
        k_eHTMLKeyModifier_ShiftDown = 1 << 2,
    };

    // keyboard interactions, native keycode is the virtual key code value from your OS
    virtual void KeyDown(HNUIBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers) = 0;
    virtual void KeyUp(HNUIBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers) = 0;
    // cUnicodeChar is the unicode character point for this keypress (and potentially multiple chars per press)
    virtual void KeyChar(HNUIBrowser unBrowserHandle, wchar_t cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers) = 0;

    // tell the html control if it has key focus currently, controls showing the I-beam cursor in text controls amongst other things
    virtual void SetKeyFocus(HNUIBrowser unBrowserHandle, bool bHasKeyFocus) = 0;

    // programmatically scroll this many pixels on the page
    //virtual void SetHorizontalScroll(HNUIBrowser unBrowserHandle, uint32 nAbsolutePixelScroll) = 0;
    //virtual void SetVerticalScroll(HNUIBrowser unBrowserHandle, uint32 nAbsolutePixelScroll) = 0;

};

#define NUI_INTERFACE_VERSION "NuiInterface001"

extern INuiInterface *nui;
