#include "cbase.h"

#include "ImageGallery.h"

#include "vgui_controls/ImageList.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/ImagePanel.h"

#include "vgui/IInput.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_BUILD_FACTORY(ImageGallery);

class GalleryNavButton : public Button
{
    DECLARE_CLASS_SIMPLE(GalleryNavButton, Button);
    GalleryNavButton(Panel *pParent, bool bNext, bool bDrawTall = false);
    void FadeIn();
    void FadeOut();
    void Update(ImageGallery *pGallery);
    void SetDrawTall(bool bTall) { m_bDrawTall = bTall; }
protected:
    void OnThink() OVERRIDE;
    void PerformLayout() OVERRIDE;
    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;
private:
    bool m_bNextButton, m_bDrawTall;
};

GalleryNavButton::GalleryNavButton(Panel* pParent, bool bNext, bool bTall) : m_bNextButton(bNext), m_bDrawTall(bTall),
Button(pParent, bNext ? "NextButton" : "PrevButton", bNext ? ">" : "<", pParent, bNext ? "NextImage" : "PreviousImage")
{
    SetZPos(1);
    SetSize(64, 64);
    SetContentAlignment(a_center);
    SetVisible(false);
    SetShouldPaint(false);
}

void GalleryNavButton::FadeIn()
{
    if (_paint && (!IsVisible() || GetAlpha() != 255))
    {
        SetVisible(true);
        SetAlpha(1);
        GetAnimationController()->RunAnimationCommand(this, "alpha", 255.0f,
                                                      0, 0.15f, AnimationController::INTERPOLATOR_SIMPLESPLINE);
    }
}

void GalleryNavButton::FadeOut()
{
    if (_paint)
        GetAnimationController()->RunAnimationCommand(this, "alpha", 0.0f,
                                                      0, 0.15f, AnimationController::INTERPOLATOR_SIMPLESPLINE);
}

void GalleryNavButton::Update(ImageGallery *pGallery)
{
    const auto iCurIndx = pGallery->GetCurrentIndex();
    const auto imgCount = pGallery->GetImageCount();
    SetEnabled(iCurIndx != (m_bNextButton ? imgCount - 1 : 0) && imgCount > 1);
    SetShouldPaint(imgCount > 1);
    InvalidateLayout(true);
}

void GalleryNavButton::OnThink()
{
    BaseClass::OnThink();
    if (IsVisible() && GetAlpha() == 0)
        SetVisible(false);
}

void GalleryNavButton::PerformLayout()
{
    BaseClass::PerformLayout();

    int wide, tall;
    GetParent()->GetSize(wide, tall);

    if (m_bDrawTall)
    {
        SetBounds(m_bNextButton ? (wide - GetWide()) : 0, 0, GetWide(), tall);
    }
    else
    {
        SetPos(m_bNextButton ? wide - GetWide() : 0, (tall - GetTall()) / 2);
    }
}

void GalleryNavButton::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetDefaultBorder(pScheme->GetBorder("GalleryNavButton.DefaultBorder"));
    SetKeyFocusBorder(pScheme->GetBorder("GalleryNavButton.KeyFocusBorder"));
    SetDepressedBorder(pScheme->GetBorder("GalleryNavButton.DepressedBorder"));

    const Color defaultFgColor = GetSchemeColor("GalleryNavButton.DefaultFgColor", COLOR_WHITE, pScheme);
    const Color defaultBgColor = GetSchemeColor("GalleryNavButton.DefaultBgColor", Color(0, 0, 0, 128), pScheme);
    SetDefaultColor(defaultFgColor, defaultBgColor);
    const Color armedFgColor = GetSchemeColor("GalleryNavButton.ArmedFgColor", pScheme->GetColor("MomentumBlue", COLOR_BLUE), pScheme);
    const Color armedBgColor = GetSchemeColor("GalleryNavButton.ArmedBgColor", Color(0, 0, 0, 128), pScheme);
    SetArmedColor(armedFgColor, armedBgColor);
    const Color pressedFgColor = GetSchemeColor("GalleryNavButton.PressedFgColor", pScheme->GetColor("MomentumBlue", COLOR_BLUE), pScheme);
    const Color pressedBgColor = GetSchemeColor("GalleryNavButton.PressedBgColor", Color(0, 0, 0, 128), pScheme);
    SetDepressedColor(pressedFgColor, pressedBgColor);
    SetSelectedColor(pressedFgColor, pressedBgColor);
}

class GalleryModalViewPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(GalleryModalViewPanel, Frame);
    GalleryModalViewPanel(ImageGallery *pGallery) : Frame(pGallery, "GalleryModalViewPanel"), m_pGallery(pGallery)
    {
        m_pImagePanel = new ImagePanel(this, "GalleryModelImage");
        m_pImagePanel->SetShouldScaleImage(true);
        m_pImagePanel->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_DOWNANDRIGHT, 5, 5, -5, -5);
        m_pImagePanel->SetZPos(-5);
        m_pImagePanel->SetImage(m_pGallery->GetCurrentImage());
        m_pImagePanel->AddActionSignalTarget(this); // For OnCursorEnter/Exit

        m_pNextButton = new GalleryNavButton(m_pImagePanel, true, false);
        m_pNextButton->AddActionSignalTarget(this); // For OnCursorEnter/Exit
        m_pNextButton->AddActionSignalTarget(m_pGallery); // For clicking them
        m_pNextButton->Update(m_pGallery);

        m_pPrevButton = new GalleryNavButton(m_pImagePanel, false, false);
        m_pPrevButton->AddActionSignalTarget(this); 
        m_pPrevButton->AddActionSignalTarget(m_pGallery);
        m_pPrevButton->Update(m_pGallery);

        m_pGallery->AddActionSignalTarget(this); // For IndexChange

        SetTitleBarVisible(false);
        SetCloseButtonVisible(true);
        SetDeleteSelfOnClose(true);
        SetSizeable(false);
        SetMoveable(false);
    }

    ~GalleryModalViewPanel()
    {
        m_pGallery->RemoveActionSignalTarget(this);
        m_pGallery->OnGalleryModalClosed();
    }

protected:

    MESSAGE_FUNC_INT(OnImageIndxChange, "IndexChange", newIndx)
    {
        m_pImagePanel->SetImage(m_pGallery->GetCurrentImage());
        InvalidateLayout(true);
        m_pNextButton->Update(m_pGallery);
        m_pPrevButton->Update(m_pGallery);
    }

    MESSAGE_FUNC_PARAMS(CursorEntered, "OnCursorEntered", pKv)
    {
        Panel *pPanel = (Panel*)pKv->GetPtr("panel");
        if (pPanel == m_pGallery)
            return;

        m_pNextButton->FadeIn();
        m_pPrevButton->FadeIn();
    }

    MESSAGE_FUNC_PARAMS(CursorExited, "OnCursorExited", pKv)
    {
        Panel *pPanel = (Panel*) pKv->GetPtr("panel");
        if (pPanel == m_pGallery)
            return;

        const VPANEL over = input()->GetMouseOver();
        if (over != m_pImagePanel->GetVPanel() && over != m_pNextButton->GetVPanel() && over != m_pPrevButton->GetVPanel())
        {
            // We genuinely exited, play our animations
            m_pNextButton->FadeOut();
            m_pPrevButton->FadeOut();
        }
    }

    void OnKeyCodeReleased(KeyCode code) OVERRIDE
    {
        if (code == KEY_LEFT)
            m_pGallery->PreviousImage();
        else if (code == KEY_RIGHT)
            m_pGallery->NextImage();
    }


    void PerformLayout() OVERRIDE 
    {
        BaseClass::PerformLayout();

        int screenWide, screenTall;
        surface()->GetScreenSize(screenWide, screenTall);

        int imgContWide, imgContTall;
        m_pGallery->GetCurrentImage()->GetContentSize(imgContWide, imgContTall);

        int frameWide = int(0.95f * float(screenWide));
        int frameTall = int(0.95f * float(screenTall));

        if (screenWide > imgContWide)
            frameWide = imgContWide;

        if (screenTall > imgContTall)
            frameTall = imgContTall;

        SetSize(frameWide, frameTall);
        MoveToCenterOfScreen();
    }

private:
    ImageGallery *m_pGallery;
    ImagePanel *m_pImagePanel;
    GalleryNavButton *m_pNextButton, *m_pPrevButton;
};

ImageGallery::ImageGallery(Panel *pParent, const char *pName, bool bDeleteImagesWhenDone /* = false*/) : EditablePanel(pParent, pName), 
    m_bDeleteOnDone(bDeleteImagesWhenDone), m_pGalleryModal(nullptr)
{
    m_bUseTallButtons = true;
    m_iCurrentIndex = 0;
    m_pImages = new ImageList(bDeleteImagesWhenDone, false);
    m_pCurrentImage = nullptr;

    m_pPrevButton = new GalleryNavButton(this, false, true);
    m_pNextButton = new GalleryNavButton(this, true, true);

    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(true);
}

ImageGallery::~ImageGallery()
{
    RemoveAllImages();
}

int ImageGallery::AddImage(IImage* pImage, bool bShowAdded /* = false*/)
{
    if (!m_pImages)
        m_pImages = new ImageList(m_bDeleteOnDone, false);

    const int index = m_pImages->AddImage(pImage);
    if (bShowAdded || index == 0)
        SetCurrentIndex(index);
    else
        UpdateButtonStates();
    return index;
}

int ImageGallery::GetImageCount()
{
    return m_pImages->GetImageCount();
}

bool ImageGallery::RemoveAllImages()
{
    m_iCurrentIndex = 0;
    m_pCurrentImage = nullptr;
    m_pNextButton->SetShouldPaint(false);
    m_pPrevButton->SetShouldPaint(false);

    if (m_pImages)
    {
        delete m_pImages;
        m_pImages = nullptr;
        return true;
    }
    return false;
}

void ImageGallery::SetCurrentIndex(int indx)
{
    if (indx < 0 || indx >= m_pImages->GetImageCount())
        return;

    m_iCurrentIndex = indx;
    m_pCurrentImage = m_pImages->GetImage(m_iCurrentIndex);

    // Update our buttons
    UpdateButtonStates();

    PostActionSignal(new KeyValues("IndexChange", "newIndx", m_iCurrentIndex));

    Repaint();
}

IImage* ImageGallery::GetCurrentImage()
{
    return m_pCurrentImage;
}

void ImageGallery::OnCursorEntered()
{
    BaseClass::OnCursorEntered();

    m_pNextButton->FadeIn();
    m_pPrevButton->FadeIn();
}

void ImageGallery::OnCursorExited()
{
    BaseClass::OnCursorExited();

    const VPANEL over = input()->GetMouseOver();
    if (over != GetVPanel() && over != m_pNextButton->GetVPanel() && over != m_pPrevButton->GetVPanel())
    {
        // We genuinely exited, play our animations
        m_pNextButton->FadeOut();
        m_pPrevButton->FadeOut();
    }
}

void ImageGallery::PreviousImage()
{
    SetCurrentIndex(m_iCurrentIndex - 1);
}
void ImageGallery::NextImage()
{
    SetCurrentIndex(m_iCurrentIndex + 1);
}

void ImageGallery::ShowGalleryModal()
{
    if (m_pGalleryModal)
        return;

    m_pGalleryModal = new GalleryModalViewPanel(this);
    m_pGalleryModal->DoModal();
}

void ImageGallery::Paint()
{
    BaseClass::Paint();

    if (m_pGalleryModal)
        return;

    if (!m_pCurrentImage)
        return;

    int wide, tall;
    GetSize(wide, tall);
    m_pCurrentImage->SetSize(wide, tall);
    m_pCurrentImage->SetColor(Color(255, 255, 255, 255));
    m_pCurrentImage->Paint();
}

void ImageGallery::PerformLayout() 
{ 
    BaseClass::PerformLayout();
    UpdateButtonStates(); 
}

void ImageGallery::OnMousePressed(MouseCode code)
{
    if (code != MOUSE_LEFT)
        return;

    if (m_pImages->GetImageCount())
    {
        ShowGalleryModal();
    }
}

void ImageGallery::OnKeyCodeReleased(KeyCode code)
{
    if (code == KEY_RIGHT)
        NextImage();
    else if (code == KEY_LEFT)
        PreviousImage();
}

void ImageGallery::OnCommand(const char* command)
{
    if (FStrEq(command, "NextImage"))
        NextImage();
    else if (FStrEq(command, "PreviousImage"))
        PreviousImage();
    else
        BaseClass::OnCommand(command);
}

void ImageGallery::UpdateButtonStates()
{
    m_pPrevButton->Update(this);
    m_pNextButton->Update(this);
}