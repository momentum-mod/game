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
    GalleryNavButton(Panel *pParent, bool bNext);
    void FadeIn();
    void FadeOut();
protected:
    void OnThink() OVERRIDE;
};

GalleryNavButton::GalleryNavButton(Panel* pParent, bool bNext) : 
Button(pParent, bNext ? "NextButton" : "PrevButton", bNext ? ">" : "<", pParent, bNext ? "NextImage" : "PreviousImage")
{
    SetZPos(1);
    SetSize(64, 64);
    SetContentAlignment(a_center);
    SetPaintBackgroundEnabled(false);
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

void GalleryNavButton::OnThink()
{
    BaseClass::OnThink();
    if (IsVisible() && GetAlpha() == 0)
        SetVisible(false);
}

class GalleryModalViewPanel : public Frame
{
    DECLARE_CLASS_SIMPLE(GalleryModalViewPanel, Frame);
    GalleryModalViewPanel(IImage *pImage) : Frame(nullptr, "GalleryModalViewPanel")
    {
        SetProportional(true);

        m_pImage = new ImagePanel(this, "GalleryImage");
        m_pImage->SetShouldScaleImage(true);
        m_pImage->SetImage(pImage);
        m_pImage->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_DOWNANDRIGHT, 5, 5, -5, -5);
        m_pImage->SetZPos(-5);

        SetTitleBarVisible(false);
        SetCloseButtonVisible(true);
        SetDeleteSelfOnClose(true);
        SetSizeable(false);
        SetMoveable(false);
    }

protected:
    void PerformLayout() OVERRIDE 
    {
        BaseClass::PerformLayout();

        int screenWide, screenTall;
        surface()->GetScreenSize(screenWide, screenTall);

        int imgContWide, imgContTall;
        m_pImage->GetImage()->GetContentSize(imgContWide, imgContTall);

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
    ImagePanel *m_pImage;
};

ImageGallery::ImageGallery(Panel *pParent, const char *pName, bool bDeleteImagesWhenDone /* = false*/) : EditablePanel(pParent, pName), 
    m_bDeleteOnDone(bDeleteImagesWhenDone)
{
    m_bUseTallButtons = true;
    m_iCurrentIndex = 0;
    m_pImages = new ImageList(bDeleteImagesWhenDone, false);
    m_pCurrentImage = nullptr;

    m_pPrevButton = new GalleryNavButton(this, false);
    m_pNextButton = new GalleryNavButton(this, true);

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
    if (indx < 0 || indx > m_pImages->GetImageCount())
        return;

    m_iCurrentIndex = indx;
    m_pCurrentImage = m_pImages->GetImage(m_iCurrentIndex);

    // Update our buttons
    UpdateButtonStates();

    Repaint();
}

void ImageGallery::OnNextImage()
{
    SetCurrentIndex(m_iCurrentIndex + 1);
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

void ImageGallery::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_pNextButton->SetFgColor(COLOR_WHITE);
    m_pPrevButton->SetFgColor(COLOR_WHITE);
}

void ImageGallery::OnPreviousImage()
{
    SetCurrentIndex(m_iCurrentIndex - 1);
}

void ImageGallery::PerformLayout()
{
    BaseClass::PerformLayout();

    int wide, tall;
    GetSize(wide, tall);

    if (m_bUseTallButtons)
    {
        m_pPrevButton->SetBounds(0, 0, m_pPrevButton->GetWide(), tall);
        m_pNextButton->SetBounds(wide - m_pNextButton->GetWide(), 0, m_pNextButton->GetWide(), tall);
    }
    else
    {
        m_pPrevButton->SetPos(0, (tall - m_pPrevButton->GetTall()) / 2);
        m_pNextButton->SetPos(wide - m_pNextButton->GetWide(), (tall - m_pNextButton->GetTall()) / 2);
    }
}

void ImageGallery::Paint()
{
    BaseClass::Paint();

    if (m_pCurrentImage)
    {
        int wide, tall;
        GetSize(wide, tall);
        m_pCurrentImage->SetSize(wide, tall);
        m_pCurrentImage->SetColor(Color(255, 255, 255, 255));
        m_pCurrentImage->Paint();
    }
}

void ImageGallery::OnMouseReleased(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        // Pop open a modal frame with just the full res photo
        GalleryModalViewPanel *pPanel = new GalleryModalViewPanel(m_pCurrentImage);
        pPanel->DoModal();
    }
}

void ImageGallery::OnKeyCodeReleased(vgui::KeyCode code)
{
    if (code == KEY_RIGHT)
        OnNextImage();
    else if (code == KEY_LEFT)
        OnPreviousImage();
}

void ImageGallery::UpdateButtonStates()
{
    const int count = m_pImages->GetImageCount();
    m_pPrevButton->SetEnabled(m_iCurrentIndex != 0 && count > 1);
    m_pNextButton->SetEnabled(m_iCurrentIndex != count - 1 && count > 1);
    m_pPrevButton->SetShouldPaint(count > 1);
    m_pNextButton->SetShouldPaint(count > 1);
}
