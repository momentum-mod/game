#include "cbase.h"
#include "ultralight_overlay.h"

#include <Ultralight/Buffer.h>
#include <Ultralight/platform/Config.h>
#include <Ultralight/platform/Platform.h>
#include <clientmode.h>
#include <vgui/IInput.h>
#include <vgui/ISurface.h>

#include "tier0/memdbgon.h"

using namespace ultralight;
using namespace vgui;

static ImageFormat Ultralight2SourceImageFormat(BitmapFormat format);
static vgui::HCursor Ultralight2SourceCursor(ultralight::Cursor cursor);
static ultralight::MouseEvent::Button Source2UltralightMouseCode(MouseCode code);
static int Source2UltralightKeyCode(KeyCode code);
static const char *GetKeyCodeChar(KeyCode code, bool bShiftPressed);
static int GetKeyModifiers();

class ULLoadListener : public ultralight::LoadListener
{
  public:
    ULLoadListener(UltralightOverlay *pOverlay) : m_pOverlay(pOverlay) {}

    // Called when the page begins loading new URL into main frame
    virtual void OnBeginLoading(ultralight::View *caller) OVERRIDE { m_pOverlay->OnBeginLoading(); }

    // Called when the page finishes loading URL into main frame
    virtual void OnFinishLoading(ultralight::View *caller) OVERRIDE { m_pOverlay->OnFinishLoading(); }

    // Called when the history (back/forward state) is modified
    virtual void OnUpdateHistory(ultralight::View *caller) OVERRIDE { m_pOverlay->OnFinishLoading(); }

    // Called when all JavaScript has been parsed and the document is ready.
    // This is the best time to make any initial JavaScript calls to your page.
    virtual void OnDOMReady(ultralight::View *caller) OVERRIDE { m_pOverlay->OnDOMReady(); }

  private:
    UltralightOverlay *m_pOverlay;
};

class ULViewListener : public ultralight::ViewListener
{
  public:
    ULViewListener(UltralightOverlay *pOverlay) : m_pOverlay(pOverlay) {}

    // Called when the page title changes
    virtual void OnChangeTitle(ultralight::View *caller, const String &title) OVERRIDE
    {
        m_pOverlay->OnChangeTitle(title.utf8().data());
    }

    // Called when the page URL changes
    virtual void OnChangeURL(ultralight::View *caller, const String &url) OVERRIDE
    {
        m_pOverlay->OnChangeURL(url.utf8().data());
    }

    // Called when the tooltip changes (usually as result of a mouse hover)
    virtual void OnChangeTooltip(ultralight::View *caller, const String &tooltip) OVERRIDE
    {
        m_pOverlay->OnChangeTooltip(tooltip.utf8().data());
    }

    // Called when the mouse cursor changes
    virtual void OnChangeCursor(ultralight::View *caller, ultralight::Cursor cursor) OVERRIDE
    {
        m_pOverlay->OnChangeCursor(cursor);
    }

    // Called when a message is added to the console (useful for errors / debug)
    virtual void OnAddConsoleMessage(ultralight::View *caller, MessageSource source, MessageLevel level,
                                     const String &message, uint32_t line_number, uint32_t column_number,
                                     const String &source_id) OVERRIDE
    {
        m_pOverlay->OnAddConsoleMessage(source, level, message.utf8().data(), line_number, column_number,
                                        source_id.utf8().data());
    }

  private:
    UltralightOverlay *m_pOverlay;
};

UltralightOverlay::UltralightOverlay(Ref<Renderer> renderer, Panel *pParentPanel, bool bTransparent)
    : m_pView(renderer->CreateView(GetWide(), GetTall(), bTransparent)),
    m_bHasFocus(false),
    m_bHasHover(false),
    m_bLinkToConsole(false),
    BaseClass(pParentPanel, "UltralightUIOverlay")
{
    m_iTextureId = surface()->CreateNewTextureID(true);

    m_pViewListener = new ULViewListener(this);
    view()->set_view_listener(m_pViewListener);
    m_pLoadListener = new ULLoadListener(this);
    view()->set_load_listener(m_pLoadListener);


    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
}

UltralightOverlay::~UltralightOverlay()
{
    surface()->DestroyTextureID(m_iTextureId);
    delete m_pViewListener;
    delete m_pLoadListener;
}

void UltralightOverlay::LoadHTMLFromFile(const char *filename)
{
    char url[MAX_PATH + 8];
    Q_snprintf(url, sizeof(url), "file:///%s", filename);
    view()->LoadURL(url);
}

void UltralightOverlay::LoadHTMLFromURL(const char *url) { view()->LoadURL(url); }

void UltralightOverlay::LoadHTMLFromString(const char *html) { view()->LoadHTML(html); }

JSContextRef UltralightOverlay::GetJSContext() { return view()->js_context(); }

JSValueRef UltralightOverlay::EvaluateScript(const char *script) { return view()->EvaluateScript(script); }

bool UltralightOverlay::CanGoBack() { return view()->CanGoBack(); }

bool UltralightOverlay::CanGoForward() { return view()->CanGoForward(); }

void UltralightOverlay::GoBack() { view()->GoBack(); }

void UltralightOverlay::GoForward() { view()->GoForward(); }

void UltralightOverlay::GoToHistoryOffset(int offset) { view()->GoToHistoryOffset(offset); }

void UltralightOverlay::Reload() { view()->Reload(); }

void UltralightOverlay::Stop() { view()->Stop(); }

void UltralightOverlay::Paint()
{
    BaseClass::Paint();

    bool dirty = view()->is_bitmap_dirty();
    const RefPtr<Bitmap> bitmap = view()->bitmap();
    if (dirty)
    {
        surface()->DrawSetTextureRGBAEx(m_iTextureId, (const unsigned char *)bitmap->raw_pixels(), bitmap->width(),
                                        bitmap->height(), Ultralight2SourceImageFormat(bitmap->format()));
    }

    surface()->DrawSetTexture(m_iTextureId);
    surface()->DrawSetColor(255, 255, 255, 255);

    int x, y, width, height;
    GetBounds(x, y, width, height);
    surface()->DrawTexturedRect(x, y, x + width, y + height);
}

void UltralightOverlay::OnSizeChanged(int newWide, int newTall)
{
    m_pView->Resize((uint32_t)newWide, (uint32_t)newTall);
}

void UltralightOverlay::OnMousePressed(MouseCode code)
{
    MouseEvent evt;
    evt.type = MouseEvent::kType_MouseDown;
    evt.button = Source2UltralightMouseCode(code);
    if (evt.button == MouseEvent::kButton_None)
    {
        return;
    }
    input()->GetCursorPosition(evt.x, evt.y);

    FireMouseEvent(evt);
}

void UltralightOverlay::OnMouseReleased(MouseCode code)
{
    MouseEvent evt;
    evt.type = MouseEvent::kType_MouseUp;
    evt.button = Source2UltralightMouseCode(code);
    if (evt.button == MouseEvent::kButton_None)
    {
        return;
    }
    input()->GetCursorPosition(evt.x, evt.y);

    FireMouseEvent(evt);
}

void UltralightOverlay::OnMouseWheeled(int delta)
{
    ScrollEvent evt;
    evt.type = ScrollEvent::kType_ScrollByPage;
    evt.delta_x = 0;
    evt.delta_y = delta;

    FireScrollEvent(evt);
}

void UltralightOverlay::OnCursorMoved(int x, int y)
{
    MouseEvent evt;
    evt.type = MouseEvent::kType_MouseMoved;
    evt.button = MouseEvent::kButton_None;
    // The position passed into OnCursorMoved is in local space, FireMouseEvent expects absolute position
    input()->GetCursorPosition(evt.x, evt.y);

    FireMouseEvent(evt);
}

void UltralightOverlay::OnCursorExited() { input()->SetCursorOveride(dc_user); }

void UltralightOverlay::OnKeyCodePressed(KeyCode code)
{
    KeyEvent evt;
    evt.type = KeyEvent::kType_KeyDown;
    evt.modifiers = GetKeyModifiers();
    evt.virtual_key_code = Source2UltralightKeyCode(code);
    if (evt.virtual_key_code == KeyCodes::GK_UNKNOWN)
    {
        return;
    }

    evt.native_key_code = 0;
    GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);
    evt.text = GetKeyCodeChar(code, evt.modifiers & KeyEvent::kMod_ShiftKey);
    evt.unmodified_text = GetKeyCodeChar(code, false);
    evt.is_keypad = IsKeypad(code);
    evt.is_auto_repeat = false;

    FireKeyEvent(evt);
}

void UltralightOverlay::OnKeyCodeReleased(KeyCode code)
{
    KeyEvent evt;
    evt.type = KeyEvent::kType_KeyUp;
    evt.modifiers = GetKeyModifiers();
    evt.virtual_key_code = Source2UltralightKeyCode(code);
    if (evt.virtual_key_code == KeyCodes::GK_UNKNOWN)
    {
        return;
    }

    evt.native_key_code = 0;
    GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);
    evt.text = GetKeyCodeChar(code, evt.modifiers & KeyEvent::kMod_ShiftKey);
    evt.unmodified_text = GetKeyCodeChar(code, false);
    evt.is_keypad = IsKeypad(code);
    evt.is_auto_repeat = false;

    FireKeyEvent(evt);
}

void UltralightOverlay::OnKeyCodeTyped(KeyCode code)
{
    KeyEvent evt;
    evt.type = KeyEvent::kType_Char;
    evt.modifiers = GetKeyModifiers();
    evt.virtual_key_code = Source2UltralightKeyCode(code);
    if (evt.virtual_key_code == KeyCodes::GK_UNKNOWN)
    {
        return;
    }

    evt.native_key_code = 0;
    GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);
    evt.text = GetKeyCodeChar(code, evt.modifiers & KeyEvent::kMod_ShiftKey);
    evt.unmodified_text = GetKeyCodeChar(code, false);
    evt.is_keypad = IsKeypad(code);
    evt.is_auto_repeat = false;

    FireKeyEvent(evt);
}

void UltralightOverlay::FireMouseEvent(const MouseEvent &evt)
{
    if (evt.type == MouseEvent::kType_MouseDown && evt.button == MouseEvent::kButton_Left)
        m_bHasFocus = Contains(evt.x, evt.y);
    else if (evt.type == MouseEvent::kType_MouseMoved)
        m_bHasHover = Contains(evt.x, evt.y);

    MouseEvent rel_evt = evt;
    ScreenToLocal(rel_evt.x, rel_evt.y);

    view()->FireMouseEvent(rel_evt);
}

void UltralightOverlay::FireScrollEvent(const ScrollEvent &evt)
{
    // if (m_bHasHover)
    view()->FireScrollEvent(evt);
}

void UltralightOverlay::FireKeyEvent(const KeyEvent &evt) { view()->FireKeyEvent(evt); }

void UltralightOverlay::OnBeginLoading() { PostActionSignal(new KeyValues("OnBeginLoading")); }

void UltralightOverlay::OnFinishLoading() { PostActionSignal(new KeyValues("OnFinishLoading")); }

void UltralightOverlay::OnUpdateHistory() { PostActionSignal(new KeyValues("OnUpdateHistory")); }

void UltralightOverlay::OnDOMReady() { PostActionSignal(new KeyValues("OnDOMReady")); }

void UltralightOverlay::OnChangeTitle(const char *title)
{
    PostActionSignal(new KeyValues("OnChangeTitle", "title", title));
}

void UltralightOverlay::OnChangeURL(const char *url)
{
    PostActionSignal(new KeyValues("OnChangeURL", "url", url));
}

void UltralightOverlay::OnChangeTooltip(const char *tooltip)
{
    PostActionSignal(new KeyValues("OnChangeTooltip", "tooltip", tooltip));
}

void UltralightOverlay::OnChangeCursor(ultralight::Cursor cursor)
{
    input()->SetCursorOveride(Ultralight2SourceCursor(cursor));
    PostActionSignal(new KeyValues("OnChangeCursor", "cursor", (int)cursor));
}

void UltralightOverlay::OnAddConsoleMessage(ultralight::MessageSource source, ultralight::MessageLevel level,
                                            const char *message, uint32_t line_number, uint32_t column_number,
                                            const char *source_id)
{
    if (IsLinkedToConsole())
    {
        switch (level)
        {
        case kMessageLevel_Debug:
            DevLog("UL Debug: %s", message);
            break;
        case kMessageLevel_Info:
            Log("UL Info: %s", message);
            break;
        case kMessageLevel_Log:
            Msg("UL Log: %s", message);
            break;
        case kMessageLevel_Warning:
            Warning("UL Warning: %s", message);
            break;
        case kMessageLevel_Error:
            Warning("UL Error: %s", message);
            break;
        }
    }

    KeyValues *pMessage = new KeyValues("OnAddConsoleMessage");
    pMessage->SetInt("source", (int)source);
    pMessage->SetInt("level", (int)level);
    pMessage->SetString("message", message);
    pMessage->SetInt("line_number", line_number);
    pMessage->SetInt("column_number", column_number);
    pMessage->SetString("source_id", source_id);
    PostActionSignal(pMessage);
}

ImageFormat Ultralight2SourceImageFormat(BitmapFormat format)
{
    switch (format)
    {
    case BitmapFormat::kBitmapFormat_RGBA8:
        return IMAGE_FORMAT_RGBA8888;
    default:
        AssertMsg(false, "Unrecognised Ultralight texture format");
        return IMAGE_FORMAT_ARGB8888;
    }
}

vgui::HCursor Ultralight2SourceCursor(ultralight::Cursor cursor)
{
    switch (cursor)
    {
    case kCursor_Pointer:
        return dc_user;
    case kCursor_Hand:
        return dc_hand;
    case kCursor_IBeam:
        return dc_ibeam;
    case kCursor_EastResize:
    case kCursor_WestResize:
    case kCursor_EastWestResize:
        return dc_sizewe;
    case kCursor_NorthResize:
    case kCursor_SouthResize:
    case kCursor_NorthSouthResize:
        return dc_sizens;
    case kCursor_NorthEastResize:
    case kCursor_SouthWestResize:
    case kCursor_NorthEastSouthWestResize:
        return dc_sizenesw;
    case kCursor_NorthWestResize:
    case kCursor_SouthEastResize:
    case kCursor_NorthWestSouthEastResize:
        return dc_sizenwse;
    default:
        return dc_user;
    }
}

ultralight::MouseEvent::Button Source2UltralightMouseCode(MouseCode code)
{
    switch (code)
    {
    case MOUSE_LEFT:
        return MouseEvent::kButton_Left;
    case MOUSE_RIGHT:
        return MouseEvent::kButton_Right;
    case MOUSE_MIDDLE:
        return MouseEvent::kButton_Middle;
    default:
        return MouseEvent::kButton_None;
    }
}

int Source2UltralightKeyCode(KeyCode code)
{
    switch (code)
    {
    case KEY_0:
        return KeyCodes::GK_0;
    case KEY_1:
        return KeyCodes::GK_1;
    case KEY_2:
        return KeyCodes::GK_2;
    case KEY_3:
        return KeyCodes::GK_3;
    case KEY_4:
        return KeyCodes::GK_4;
    case KEY_5:
        return KeyCodes::GK_5;
    case KEY_6:
        return KeyCodes::GK_6;
    case KEY_7:
        return KeyCodes::GK_7;
    case KEY_8:
        return KeyCodes::GK_8;
    case KEY_9:
        return KeyCodes::GK_9;
    case KEY_A:
        return KeyCodes::GK_A;
    case KEY_B:
        return KeyCodes::GK_B;
    case KEY_C:
        return KeyCodes::GK_C;
    case KEY_D:
        return KeyCodes::GK_D;
    case KEY_E:
        return KeyCodes::GK_E;
    case KEY_F:
        return KeyCodes::GK_F;
    case KEY_G:
        return KeyCodes::GK_G;
    case KEY_H:
        return KeyCodes::GK_H;
    case KEY_I:
        return KeyCodes::GK_I;
    case KEY_J:
        return KeyCodes::GK_J;
    case KEY_K:
        return KeyCodes::GK_K;
    case KEY_L:
        return KeyCodes::GK_L;
    case KEY_M:
        return KeyCodes::GK_M;
    case KEY_N:
        return KeyCodes::GK_N;
    case KEY_O:
        return KeyCodes::GK_O;
    case KEY_P:
        return KeyCodes::GK_P;
    case KEY_Q:
        return KeyCodes::GK_Q;
    case KEY_R:
        return KeyCodes::GK_R;
    case KEY_S:
        return KeyCodes::GK_S;
    case KEY_T:
        return KeyCodes::GK_T;
    case KEY_U:
        return KeyCodes::GK_U;
    case KEY_V:
        return KeyCodes::GK_V;
    case KEY_W:
        return KeyCodes::GK_W;
    case KEY_X:
        return KeyCodes::GK_X;
    case KEY_Y:
        return KeyCodes::GK_Y;
    case KEY_Z:
        return KeyCodes::GK_Z;
    case KEY_PAD_0:
        return KeyCodes::GK_NUMPAD0;
    case KEY_PAD_1:
        return KeyCodes::GK_NUMPAD1;
    case KEY_PAD_2:
        return KeyCodes::GK_NUMPAD2;
    case KEY_PAD_3:
        return KeyCodes::GK_NUMPAD3;
    case KEY_PAD_4:
        return KeyCodes::GK_NUMPAD4;
    case KEY_PAD_5:
        return KeyCodes::GK_NUMPAD5;
    case KEY_PAD_6:
        return KeyCodes::GK_NUMPAD6;
    case KEY_PAD_7:
        return KeyCodes::GK_NUMPAD7;
    case KEY_PAD_8:
        return KeyCodes::GK_NUMPAD8;
    case KEY_PAD_9:
        return KeyCodes::GK_NUMPAD9;
    case KEY_PAD_DIVIDE:
        return KeyCodes::GK_ADD;
    case KEY_PAD_MULTIPLY:
        return KeyCodes::GK_MULTIPLY;
    case KEY_PAD_MINUS:
        return KeyCodes::GK_SUBTRACT;
    case KEY_PAD_PLUS:
        return KeyCodes::GK_ADD;
    case KEY_PAD_ENTER:
        return KeyCodes::GK_RETURN;
    case KEY_PAD_DECIMAL:
        return KeyCodes::GK_DECIMAL;
    case KEY_LBRACKET:
        return KeyCodes::GK_OEM_4;
    case KEY_RBRACKET:
        return KeyCodes::GK_OEM_6;
    case KEY_SEMICOLON:
        return KeyCodes::GK_OEM_1;
    case KEY_APOSTROPHE:
        return KeyCodes::GK_OEM_7;
    case KEY_BACKQUOTE:
        return KeyCodes::GK_OEM_3;
    case KEY_COMMA:
        return KeyCodes::GK_OEM_COMMA;
    case KEY_PERIOD:
        return KeyCodes::GK_OEM_PERIOD;
    case KEY_SLASH:
        return KeyCodes::GK_OEM_2;
    case KEY_BACKSLASH:
        return KeyCodes::GK_OEM_5;
    case KEY_MINUS:
        return KeyCodes::GK_OEM_MINUS;
    case KEY_EQUAL:
        return KeyCodes::GK_OEM_PLUS;
    case KEY_ENTER:
        return KeyCodes::GK_RETURN;
    case KEY_SPACE:
        return KeyCodes::GK_SPACE;
    case KEY_BACKSPACE:
        return KeyCodes::GK_BACK;
    case KEY_TAB:
        return KeyCodes::GK_TAB;
    case KEY_CAPSLOCK:
        return KeyCodes::GK_CAPITAL;
    case KEY_NUMLOCK:
        return KeyCodes::GK_NUMLOCK;
    case KEY_ESCAPE:
        return KeyCodes::GK_ESCAPE;
    case KEY_SCROLLLOCK:
        return KeyCodes::GK_SCROLL;
    case KEY_INSERT:
        return KeyCodes::GK_INSERT;
    case KEY_DELETE:
        return KeyCodes::GK_DELETE;
    case KEY_HOME:
        return KeyCodes::GK_HOME;
    case KEY_END:
        return KeyCodes::GK_END;
    case KEY_PAGEUP:
        return KeyCodes::GK_PRIOR;
    case KEY_PAGEDOWN:
        return KeyCodes::GK_NEXT;
    case KEY_BREAK:
        return KeyCodes::GK_PAUSE;
    case KEY_LSHIFT:
        return KeyCodes::GK_SHIFT;
    case KEY_RSHIFT:
        return KeyCodes::GK_SHIFT;
    case KEY_LALT:
        return KeyCodes::GK_MENU;
    case KEY_RALT:
        return KeyCodes::GK_MENU;
    case KEY_LCONTROL:
        return KeyCodes::GK_CONTROL;
    case KEY_RCONTROL:
        return KeyCodes::GK_CONTROL;
    case KEY_LWIN:
        return KeyCodes::GK_LWIN;
    case KEY_RWIN:
        return KeyCodes::GK_RWIN;
    case KEY_APP:
        return KeyCodes::GK_APPS;
    case KEY_UP:
        return KeyCodes::GK_UP;
    case KEY_LEFT:
        return KeyCodes::GK_LEFT;
    case KEY_DOWN:
        return KeyCodes::GK_DOWN;
    case KEY_RIGHT:
        return KeyCodes::GK_RIGHT;
    case KEY_F1:
        return KeyCodes::GK_F1;
    case KEY_F2:
        return KeyCodes::GK_F2;
    case KEY_F3:
        return KeyCodes::GK_F3;
    case KEY_F4:
        return KeyCodes::GK_F4;
    case KEY_F5:
        return KeyCodes::GK_F5;
    case KEY_F6:
        return KeyCodes::GK_F6;
    case KEY_F7:
        return KeyCodes::GK_F7;
    case KEY_F8:
        return KeyCodes::GK_F8;
    case KEY_F9:
        return KeyCodes::GK_F9;
    case KEY_F10:
        return KeyCodes::GK_F10;
    case KEY_F11:
        return KeyCodes::GK_F11;
    case KEY_F12:
        return KeyCodes::GK_F12;
    case KEY_CAPSLOCKTOGGLE:
        return KeyCodes::GK_UNKNOWN;
    case KEY_NUMLOCKTOGGLE:
        return KeyCodes::GK_UNKNOWN;
    case KEY_SCROLLLOCKTOGGLE:
        return KeyCodes::GK_UNKNOWN;
    default:
        return KeyCodes::GK_UNKNOWN;
    }
}

const char *GetKeyCodeChar(KeyCode code, bool bShiftPressed)
{
    static char *_keyTrans[KEY_LAST];
    static bool initialized = false;
    if (!initialized)
    {
        _keyTrans[KEY_0] = "0\0)\0KEY_0";
        _keyTrans[KEY_1] = "1\0!\0KEY_1";
        _keyTrans[KEY_2] = "2\0@\0KEY_2";
        _keyTrans[KEY_3] = "3\0#\0KEY_3";
        _keyTrans[KEY_4] = "4\0$\0KEY_4";
        _keyTrans[KEY_5] = "5\0%\0KEY_5";
        _keyTrans[KEY_6] = "6\0^\0KEY_6";
        _keyTrans[KEY_7] = "7\0&\0KEY_7";
        _keyTrans[KEY_8] = "8\0*\0KEY_8";
        _keyTrans[KEY_9] = "9\0(\0KEY_9";
        _keyTrans[KEY_A] = "a\0A\0KEY_A";
        _keyTrans[KEY_B] = "b\0B\0KEY_B";
        _keyTrans[KEY_C] = "c\0C\0KEY_C";
        _keyTrans[KEY_D] = "d\0D\0KEY_D";
        _keyTrans[KEY_E] = "e\0E\0KEY_E";
        _keyTrans[KEY_F] = "f\0F\0KEY_F";
        _keyTrans[KEY_G] = "g\0G\0KEY_G";
        _keyTrans[KEY_H] = "h\0H\0KEY_H";
        _keyTrans[KEY_I] = "i\0I\0KEY_I";
        _keyTrans[KEY_J] = "j\0J\0KEY_J";
        _keyTrans[KEY_K] = "k\0K\0KEY_K";
        _keyTrans[KEY_L] = "l\0L\0KEY_L"
                           ", L";
        _keyTrans[KEY_M] = "m\0M\0KEY_M";
        _keyTrans[KEY_N] = "n\0N\0KEY_N";
        _keyTrans[KEY_O] = "o\0O\0KEY_O";
        _keyTrans[KEY_P] = "p\0P\0KEY_P";
        _keyTrans[KEY_Q] = "q\0Q\0KEY_Q";
        _keyTrans[KEY_R] = "r\0R\0KEY_R";
        _keyTrans[KEY_S] = "s\0S\0KEY_S";
        _keyTrans[KEY_T] = "t\0T\0KEY_T";
        _keyTrans[KEY_U] = "u\0U\0KEY_U";
        _keyTrans[KEY_V] = "v\0V\0KEY_V";
        _keyTrans[KEY_W] = "w\0W\0KEY_W";
        _keyTrans[KEY_X] = "x\0X\0KEY_X";
        _keyTrans[KEY_Y] = "y\0Y\0KEY_Y";
        _keyTrans[KEY_Z] = "z\0Z\0KEY_Z";
        _keyTrans[KEY_PAD_0] = "0\0\0\0KEY_PAD_0";
        _keyTrans[KEY_PAD_1] = "1\0\0\0KEY_PAD_1";
        _keyTrans[KEY_PAD_2] = "2\0\0\0KEY_PAD_2";
        _keyTrans[KEY_PAD_3] = "3\0\0\0KEY_PAD_3";
        _keyTrans[KEY_PAD_4] = "4\0\0\0KEY_PAD_4";
        _keyTrans[KEY_PAD_5] = "5\0\0\0KEY_PAD_5";
        _keyTrans[KEY_PAD_6] = "6\0\0\0KEY_PAD_6";
        _keyTrans[KEY_PAD_7] = "7\0\0\0KEY_PAD_7";
        _keyTrans[KEY_PAD_8] = "8\0\0\0KEY_PAD_8";
        _keyTrans[KEY_PAD_9] = "9\0\0\0KEY_PAD_9";
        _keyTrans[KEY_PAD_DIVIDE] = "/\0/\0KEY_PAD_DIVIDE";
        _keyTrans[KEY_PAD_MULTIPLY] = "*\0*\0KEY_PAD_MULTIPLY";
        _keyTrans[KEY_PAD_MINUS] = "-\0-\0KEY_PAD_MINUS";
        _keyTrans[KEY_PAD_PLUS] = "+\0+\0KEY_PAD_PLUS";
        _keyTrans[KEY_PAD_ENTER] = "\0\0\0\0KEY_PAD_ENTER";
        _keyTrans[KEY_PAD_DECIMAL] = ".\0\0\0KEY_PAD_DECIMAL"
                                     ", L";
        _keyTrans[KEY_LBRACKET] = "[\0{\0KEY_LBRACKET";
        _keyTrans[KEY_RBRACKET] = "]\0}\0KEY_RBRACKET";
        _keyTrans[KEY_SEMICOLON] = ";\0:\0KEY_SEMICOLON";
        _keyTrans[KEY_APOSTROPHE] = "'\0\"\0KEY_APOSTROPHE";
        _keyTrans[KEY_BACKQUOTE] = "`\0~\0KEY_BACKQUOTE";
        _keyTrans[KEY_COMMA] = ",\0<\0KEY_COMMA";
        _keyTrans[KEY_PERIOD] = ".\0>\0KEY_PERIOD";
        _keyTrans[KEY_SLASH] = "/\0?\0KEY_SLASH";
        _keyTrans[KEY_BACKSLASH] = "\\\0|\0KEY_BACKSLASH";
        _keyTrans[KEY_MINUS] = "-\0_\0KEY_MINUS";
        _keyTrans[KEY_EQUAL] = "=\0+\0KEY_EQUAL"
                               ", L";
        _keyTrans[KEY_ENTER] = "\0\0\0\0KEY_ENTER";
        _keyTrans[KEY_SPACE] = " \0 \0KEY_SPACE";
        _keyTrans[KEY_BACKSPACE] = "\0\0\0\0KEY_BACKSPACE";
        _keyTrans[KEY_TAB] = "\0\0\0\0KEY_TAB";
        _keyTrans[KEY_CAPSLOCK] = "\0\0\0\0KEY_CAPSLOCK";
        _keyTrans[KEY_NUMLOCK] = "\0\0\0\0KEY_NUMLOCK";
        _keyTrans[KEY_ESCAPE] = "\0\0\0\0KEY_ESCAPE";
        _keyTrans[KEY_SCROLLLOCK] = "\0\0\0\0KEY_SCROLLLOCK";
        _keyTrans[KEY_INSERT] = "\0\0\0\0KEY_INSERT";
        _keyTrans[KEY_DELETE] = "\0\0\0\0KEY_DELETE";
        _keyTrans[KEY_HOME] = "\0\0\0\0KEY_HOME";
        _keyTrans[KEY_END] = "\0\0\0\0KEY_END";
        _keyTrans[KEY_PAGEUP] = "\0\0\0\0KEY_PAGEUP";
        _keyTrans[KEY_PAGEDOWN] = "\0\0\0\0KEY_PAGEDOWN";
        _keyTrans[KEY_BREAK] = "\0\0\0\0KEY_BREAK";
        _keyTrans[KEY_LSHIFT] = "\0\0\0\0KEY_LSHIFT";
        _keyTrans[KEY_RSHIFT] = "\0\0\0\0KEY_RSHIFT";
        _keyTrans[KEY_LALT] = "\0\0\0\0KEY_LALT";
        _keyTrans[KEY_RALT] = "\0\0\0\0KEY_RALT";
        _keyTrans[KEY_LCONTROL] = "\0\0\0\0KEY_LCONTROL"
                                  ", L";
        _keyTrans[KEY_RCONTROL] = "\0\0\0\0KEY_RCONTROL"
                                  ", L";
        _keyTrans[KEY_LWIN] = "\0\0\0\0KEY_LWIN";
        _keyTrans[KEY_RWIN] = "\0\0\0\0KEY_RWIN";
        _keyTrans[KEY_APP] = "\0\0\0\0KEY_APP";
        _keyTrans[KEY_UP] = "\0\0\0\0KEY_UP";
        _keyTrans[KEY_LEFT] = "\0\0\0\0KEY_LEFT";
        _keyTrans[KEY_DOWN] = "\0\0\0\0KEY_DOWN";
        _keyTrans[KEY_RIGHT] = "\0\0\0\0KEY_RIGHT";
        _keyTrans[KEY_F1] = "\0\0\0\0KEY_F1";
        _keyTrans[KEY_F2] = "\0\0\0\0KEY_F2";
        _keyTrans[KEY_F3] = "\0\0\0\0KEY_F3";
        _keyTrans[KEY_F4] = "\0\0\0\0KEY_F4";
        _keyTrans[KEY_F5] = "\0\0\0\0KEY_F5";
        _keyTrans[KEY_F6] = "\0\0\0\0KEY_F6";
        _keyTrans[KEY_F7] = "\0\0\0\0KEY_F7";
        _keyTrans[KEY_F8] = "\0\0\0\0KEY_F8";
        _keyTrans[KEY_F9] = "\0\0\0\0KEY_F9";
        _keyTrans[KEY_F10] = "\0\0\0\0KEY_F10";
        _keyTrans[KEY_F11] = "\0\0\0\0KEY_F11";
        _keyTrans[KEY_F12] = "\0\0\0\0KEY_F12";

        initialized = true;
    }

    return bShiftPressed ? &_keyTrans[code][2] : &_keyTrans[code][0];
}

int GetKeyModifiers()
{
    // Any time a key is pressed reset modifier list as well
    unsigned nModifierCodes = 0;
    if (vgui::input()->IsKeyDown(KEY_LCONTROL) || vgui::input()->IsKeyDown(KEY_RCONTROL))
        nModifierCodes |= KeyEvent::kMod_CtrlKey;

    if (vgui::input()->IsKeyDown(KEY_LALT) || vgui::input()->IsKeyDown(KEY_RALT))
        nModifierCodes |= KeyEvent::kMod_AltKey;

    if (vgui::input()->IsKeyDown(KEY_LSHIFT) || vgui::input()->IsKeyDown(KEY_RSHIFT))
        nModifierCodes |= KeyEvent::kMod_ShiftKey;

    if (vgui::input()->IsKeyDown(KEY_LWIN) || vgui::input()->IsKeyDown(KEY_RWIN))
        nModifierCodes |= KeyEvent::kMod_MetaKey;

    return nModifierCodes;
}