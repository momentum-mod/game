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
        /// Creates a new FileImage, using the pDefaultImage to render if the image does not/fails to load
        FileImage(IImage *pDefaultImage = nullptr);
        FileImage(const char *pFileName, const char *pPathID = "GAME", IImage *pDefaultImage = nullptr);
        ~FileImage();

        /// Loads an image from file given the file name and pathID. Returns true if loaded, else false
        bool LoadFromFile(const char *pFileName, const char *pPathID = "GAME");
        bool LoadFromUtlBuffer(CUtlBuffer &buf);
        void LoadFromRGBA(const uint8 *pData, int wide, int tall);

        // Call to Paint the image
        // Image will draw within the current panel context at the specified position
        void Paint() OVERRIDE;

        // Set the position of the image
        void SetPos(int x, int y) OVERRIDE { m_iX = x; m_iY = y; }

        // Gets the size of the content
        void GetContentSize(int &wide, int &tall) OVERRIDE { wide = m_iOriginalImageWide; tall = m_iOriginalImageTall; }

        // Get the size the image will actually draw in (usually defaults to the content size)
        void GetSize(int& wide, int& tall) OVERRIDE;

        // Sets the size of the image
        void SetSize(int wide, int tall) OVERRIDE;

        // Set the draw color 
        void SetColor(Color col) OVERRIDE { m_DrawColor = col; }

        // Set the rotation of the image in degrees
        void SetRotation(int iRotation) OVERRIDE { m_iRotation = iRotation; }

        // Overridden to cause a reload
        bool Evict() OVERRIDE;

        // Unused
        int GetNumFrames() OVERRIDE { return 0; }
        void SetFrame(int nFrame) OVERRIDE {}
        HTexture GetID() OVERRIDE { return (HTexture) 0; }

    protected:
        Color m_DrawColor;
        int m_iX, m_iY, m_iDesiredWide, m_iDesiredTall, m_iRotation, m_iTextureID;

        IImage *m_pDefaultImage;

        int m_iOriginalImageWide, m_iOriginalImageTall; // Original dimensions when loaded
        CUtlBuffer m_bufOriginalImage, m_bufImage;
    private:
        bool LoadFromFileInternal();
        bool LoadFromUtlBufferInternal();
        void DestroyTexture();
        char m_szFileName[MAX_PATH];
        char m_szPathID[16];

        void PaintDefaultImage();

    };

    // Like FileImage but streamed from the web (meaning not requiring to be locally downloaded & stored)
    // Use LoadFromURL to load an image.
    class URLImage : public FileImage
    {
    public:
        /// Constructs a URL image.
        /// If bDrawProgress is true, a progress bar is drawn while the image loads to show stream progress.
        /// If pDefaultImage is non-null, the image is drawn while loading if bDrawProgress is false, 
        /// and regardless if the image fails to load.
        URLImage(IImage *pDefaultImage = nullptr, bool bDrawProgress = false);
        URLImage(const char *pURL, IImage *pDefault = nullptr, bool bDrawProgress = false);
        ~URLImage();
        /// Begins loading a file from the given URL. Returns true if loading, else false.
        /// If you passed in a default image in the constructor, it will draw while loading,
        /// otherwise if bDrawProgress is true, a progress bar will denote progress until loaded.
        bool LoadFromURL(const char *pURL);

        // Overrides from FileImage
        void Paint() OVERRIDE;
        bool Evict() OVERRIDE;
    protected:
        void OnFileStreamSize(KeyValues *pKv);
        void OnFileStreamProgress(KeyValues *pKv);
        void OnFileStreamEnd(KeyValues *pKv);

    private:
        bool LoadFromURLInternal();
        char m_szURL[256];
        uint64 m_hRequest;
        bool m_bDrawProgressBar;
        float m_fProgress;
        uint64 m_uTotalSize;
    };
}