#pragma once

#include "vgui/IImage.h"

namespace vgui
{
    // A class to load (almost) any type of image from disk to be used on panels and ImageLists.
    // Uses STB image to parse JPG/PNG/GIF(non-animated)/TGA/BMP, functions very similarly to BitmapImage
    // but allows for more types of images.
    // Use LoadFromFile to load an image.
    class FileImage : public IImage
    {
    public:
        FileImage();
        ~FileImage();

        /// Loads an image from file given the file name and pathID. Returns true if loaded, else false
        bool LoadFromFile(const char *pFileName, const char *pPathID = "GAME");
        bool LoadFromUtlBuffer(CUtlBuffer &buf);
        void LoadFromRGBA(const uint8 *pData, int wide, int tall);

    protected:
        // Call to Paint the image
        // Image will draw within the current panel context at the specified position
        void Paint() OVERRIDE;

        // Set the position of the image
        void SetPos(int x, int y) OVERRIDE { m_iX = x; m_iY = y; }

        // Gets the size of the content
        void GetContentSize(int &wide, int &tall) OVERRIDE { wide = m_iImageWide; tall = m_iImageTall; }

        // Get the size the image will actually draw in (usually defaults to the content size)
        void GetSize(int& wide, int& tall) OVERRIDE;

        // Sets the size of the image
        void SetSize(int wide, int tall) OVERRIDE { m_iDesiredWide = wide; m_iDesiredTall = tall; }

        // Set the draw color 
        void SetColor(Color col) OVERRIDE { m_DrawColor = col; }

        // Set the rotation of the image in degrees
        void SetRotation(int iRotation) OVERRIDE { m_iRotation = iRotation; }

        // Unused
        bool Evict() OVERRIDE { return false; }
        int GetNumFrames() OVERRIDE { return 0; }
        void SetFrame(int nFrame) OVERRIDE {}
        HTexture GetID() OVERRIDE { return (HTexture)0; }

        Color m_DrawColor;
        int m_iX, m_iY, m_iImageWide, m_iDesiredWide, m_iImageTall, m_iDesiredTall, m_iRotation, m_iTextureID;
    private:
        void DestroyTexture();
    };

    // Like FileImage but streamed from the web (meaning not requiring to be locally downloaded & stored)
    // Use LoadFromURL to load an image.
    class URLImage : public FileImage
    {
    public:
        URLImage();

        /// Begins loading a file from the given URL. Returns true if loading, else false.
        /// You may pass in a default image to use while loading, otherwise nothing will be drawn (while loading).
        bool LoadFromURL(const char *pURL, IImage *pDefaultImage = nullptr);

    protected:
        void Paint() OVERRIDE;

        void OnFileStreamStart(KeyValues *pKv);
        void OnFileStreamProgress(KeyValues *pKv);
        void OnFileStreamEnd(KeyValues *pKv);

    private:
        IImage *m_pDefaultImage;
        uint64 m_hRequest;
        bool m_bValid;
    };
}