#pragma once

#include "interface.h"
#include "INuiInterface.h"

abstract_class NuiBrowserListener : public IBaseInterface
{
public:

    // Called when a browser was created for this listener.
    // It is recommended that the listener implementing this interface stores the
    // handle passed through here, as it is 
    virtual void OnBrowserCreated(HNUIBrowser handle) = 0;

    // Called if the CreateBrowser method fails to create a browser (returns false)
    virtual void OnBrowserFailedToCreate() = 0;

    // Called when our browser was finally closed, meaning this listener is effectively
    // done for, and will not be called for any more functions.
    virtual void OnBrowserClosed() = 0;

    // Called when the CEF wants to know what the size of our browser is. 
    // Usually this is just panel width and height.
    virtual void OnBrowserSize(int &wide, int &tall) = 0;

    // Called when the browser is done loading a URL.
    virtual void OnBrowserPageLoaded(const char *pURL) = 0;

    // Called when the browser needs paint. Not always called every frame.
    virtual void OnBrowserPaint(
        const void *pBGRA, // a pointer to the B8G8R8A8 data for this surface
        uint32 texWide,             // the total width of the pBGRA texture
        uint32 texTall,             // the total height of the pBGRA texture
        uint32 unUpdateX,           // the offset in X for the damage rect for this update
        uint32 unUpdateY,           // the offset in Y for the damage rect for this update
        uint32 unUpdateWide,        // the width of the damage rect for this update
        uint32 unUpdateTall,        // the height of the damage rect for this update
        uint32 unScrollX,           // the page scroll the browser was at when this texture was rendered
        uint32 unScrollY) = 0;      // the page scroll the browser was at when this texture was rendered

    // Called when the browser has an alert dialog popping up.
    virtual void OnBrowserJSAlertDialog(const char *pString) = 0;
};