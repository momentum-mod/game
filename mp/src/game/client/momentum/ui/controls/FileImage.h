#pragma once

#include "vgui/IImage.h"

namespace vgui
{
    class FileImage : public IImage
    {
    public:
        FileImage();
        ~FileImage();

        bool LoadFromFile(const char *pFileName, const char *pPathID = "GAME");
        void LoadFromRGBA(const uint8 *pData, int wide, int tall);

    protected:
        // Call to Paint the image
        // Image will draw within the current panel context at the specified position
        void Paint() OVERRIDE;

        // Set the position of the image
        void SetPos(int x, int y) OVERRIDE { m_iX = x; m_iY = y; }

        // Gets the size of the content
        void GetContentSize(int &wide, int &tall) OVERRIDE { wide = m_iWide; tall = m_iTall; }

        // Get the size the image will actually draw in (usually defaults to the content size)
        void GetSize(int &wide, int &tall) OVERRIDE { GetContentSize(wide, tall); }

        // Sets the size of the image
        void SetSize(int wide, int tall) OVERRIDE { m_iWide = wide; m_iTall = tall; }

        // Set the draw color 
        void SetColor(Color col) OVERRIDE { m_DrawColor = col; }

        // Set the rotation of the image in degrees
        void SetRotation(int iRotation) OVERRIDE { m_iRotation = iRotation; }

        // Unused
        bool Evict() OVERRIDE { return false; }
        int GetNumFrames() OVERRIDE { return 0; }
        void SetFrame(int nFrame) OVERRIDE {}
        HTexture GetID() OVERRIDE { return (HTexture)0; }

    private:
        void DestroyTexture();
        Color m_DrawColor;
        int m_iX, m_iY, m_iWide, m_iTall, m_iRotation, m_iTextureID;
    };
}