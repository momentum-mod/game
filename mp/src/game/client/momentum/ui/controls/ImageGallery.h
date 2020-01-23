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
    virtual int GetImageCount();
    virtual bool RemoveAllImages();
    virtual void SetCurrentIndex(int indx);
    virtual vgui::IImage *GetCurrentImage();
    virtual int GetCurrentIndex() { return m_iCurrentIndex; }

    virtual void SetUseTallButtons(bool bTall) { m_bUseTallButtons = bTall; }

    virtual void PreviousImage();
    virtual void NextImage();
protected:
    MESSAGE_FUNC(OnCursorExited, "OnCursorExited");
    MESSAGE_FUNC(OnCursorEntered, "OnCursorEntered");

    void Paint() OVERRIDE;
    void PerformLayout() OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    void OnKeyCodeReleased(vgui::KeyCode code) OVERRIDE;
    void OnCommand(const char* command) OVERRIDE;

private:
    void UpdateButtonStates();

    int m_iCurrentIndex;
    bool m_bDeleteOnDone, m_bUseTallButtons;
    vgui::ImageList *m_pImages;
    vgui::IImage *m_pCurrentImage;
    GalleryNavButton *m_pPrevButton, *m_pNextButton;
};