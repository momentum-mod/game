#pragma once

#include "vgui_controls/EditablePanel.h"

class GalleryNavButton;

class ImageGallery : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(ImageGallery, EditablePanel);

    ImageGallery(Panel *pParent, const char *pName, bool bDeleteImagesWhenDone = false);
    ~ImageGallery();

    // Adds an image to the gallery. If bShowAdded is true, the panel changes current index to it.
    virtual int AddImage(vgui::IImage *pImage, bool bShowAdded = false);
    virtual bool RemoveAllImages();
    virtual void SetCurrentIndex(int indx);

    virtual void SetUseTallButtons(bool bTall) { m_bUseTallButtons = bTall; }

protected:
    MESSAGE_FUNC(OnCursorExited, "OnCursorExited");
    MESSAGE_FUNC(OnCursorEntered, "OnCursorEntered");
    MESSAGE_FUNC(OnPreviousImage, "PreviousImage");
    MESSAGE_FUNC(OnNextImage, "NextImage");

    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;
    void PerformLayout() OVERRIDE;
    void Paint() OVERRIDE;
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    void OnKeyCodeReleased(vgui::KeyCode code) OVERRIDE;

private:
    void UpdateButtonStates();

    int m_iCurrentIndex;
    bool m_bDeleteOnDone, m_bUseTallButtons;
    vgui::ImageList *m_pImages;
    vgui::IImage *m_pCurrentImage;
    GalleryNavButton *m_pPrevButton, *m_pNextButton;
};