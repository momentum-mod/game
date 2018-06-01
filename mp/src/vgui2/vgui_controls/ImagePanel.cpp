//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include <vgui/IBorder.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IBorder.h>
#include <KeyValues.h>

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Image.h>
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( ImagePanel );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ImagePanel::ImagePanel(Panel *parent, const char *name) : Panel(parent, name)
{
    InitSettings();
	m_pImage = nullptr;
	m_pszImageName = nullptr;
	m_pszFillColorName = nullptr;
	m_pszDrawColorName = nullptr;	// HPE addition
	m_bCenterImage = false;
	m_bScaleImage = false;
	m_bTileImage = false;
	m_bTileHorizontally = false;
	m_bTileVertically = false;
	m_bPositionImage = true;
	m_fScaleAmount = 0.0f;
	m_FillColor = Color(0, 0, 0, 0);
	m_DrawColor = Color(255,255,255,255);
	m_iRotation = ROTATED_UNROTATED;

	SetImage( m_pImage );

	REGISTER_COLOR_AS_OVERRIDABLE( m_FillColor, "fillcolor_override" );
	REGISTER_COLOR_AS_OVERRIDABLE( m_DrawColor, "drawcolor_override" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ImagePanel::~ImagePanel()
{
    m_pszImageName.Purge();
    m_pszFillColorName.Purge();
    m_pszDrawColorName.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: handles size changing
//-----------------------------------------------------------------------------
void ImagePanel::OnSizeChanged(int newWide, int newTall)
{
	BaseClass::OnSizeChanged(newWide, newTall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ImagePanel::SetImage(IImage *image)
{
	m_pImage = image;
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: sets an image by file name
//-----------------------------------------------------------------------------
void ImagePanel::SetImage(const char *imageName)
{
	if ( imageName && !m_pszImageName.IsEmpty() && m_pszImageName.IsEqual_CaseInsensitive(imageName))
		return;

    m_pszImageName.Purge();
    m_pszImageName = imageName;
	InvalidateLayout(false, true); // force applyschemesettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IImage *ImagePanel::GetImage()
{
	return m_pImage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Color ImagePanel::GetDrawColor( void )
{
	return m_DrawColor;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ImagePanel::SetDrawColor( Color drawColor )
{
	m_DrawColor = drawColor;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ImagePanel::PaintBackground()
{
	if (m_FillColor[3] > 0)
	{
		// draw the specified fill color
		int wide, tall;
		GetSize(wide, tall);
		surface()->DrawSetColor(m_FillColor);
		surface()->DrawFilledRect(0, 0, wide, tall);
	}
	if ( m_pImage )
	{
		//=============================================================================
		// HPE_BEGIN:
		// [pfreese] Color should be always set from GetDrawColor(), not just when 
		// scaling is true (see previous code)
		//=============================================================================
		
		// surface()->DrawSetColor( 255, 255, 255, GetAlpha() );
		m_pImage->SetColor( GetDrawColor() );
		
		//=============================================================================
		// HPE_END
		//=============================================================================

		if ( m_bPositionImage )
		{
			if ( m_bCenterImage )
			{
				int wide, tall;
				GetSize(wide, tall);

				int imageWide, imageTall;
				m_pImage->GetSize( imageWide, imageTall );

				if ( m_bScaleImage && m_fScaleAmount > 0.0f )
				{
					imageWide = static_cast<int>( static_cast<float>(imageWide) * m_fScaleAmount );
					imageTall = static_cast<int>( static_cast<float>(imageTall) * m_fScaleAmount );
				}

				m_pImage->SetPos( (wide - imageWide) / 2, (tall - imageTall) / 2 );
			}
			else
			{
				m_pImage->SetPos(0, 0);
			}
		}

		if (m_bScaleImage)
		{
			// Image size is stored in the bitmap, so temporarily set its size
			// to our panel size and then restore after we draw it.

			int imageWide, imageTall;
			m_pImage->GetSize( imageWide, imageTall );

			if ( m_fScaleAmount > 0.0f )
			{
				float wide, tall;
				wide = static_cast<float>(imageWide) * m_fScaleAmount;
				tall = static_cast<float>(imageTall) * m_fScaleAmount;
				m_pImage->SetSize( static_cast<int>(wide), static_cast<int>(tall) );
			}
			else
			{
				int wide, tall;
				GetSize( wide, tall );
				m_pImage->SetSize( wide, tall );
			}

			m_pImage->Paint();

			m_pImage->SetSize( imageWide, imageTall );
		}
		else if ( m_bTileImage || m_bTileHorizontally || m_bTileVertically )
		{
			int wide, tall;
			GetSize(wide, tall);
			int imageWide, imageTall;
			m_pImage->GetSize( imageWide, imageTall );

			int y = 0;
			while ( y < tall )
			{
				int x = 0;
				while (x < wide)
				{
					m_pImage->SetPos(x,y);
					m_pImage->Paint();

					x += imageWide;

					if ( !m_bTileHorizontally )
						break;
				}

				y += imageTall;

				if ( !m_bTileVertically )
					break;
			}

			if ( m_bPositionImage )
			{
				m_pImage->SetPos(0, 0);
			}
		}
		else
		{
			m_pImage->SetColor( GetDrawColor() );
			m_pImage->Paint();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gets control settings for editing
//-----------------------------------------------------------------------------
void ImagePanel::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);
	outResourceData->SetString("image", m_pszImageName);
	outResourceData->SetString("fillcolor", m_pszFillColorName);

	//=============================================================================
	// HPE_BEGIN:
	// [pfreese] Added support for specifying drawcolor
	//=============================================================================
	outResourceData->SetString("drawcolor", m_pszDrawColorName);
	//=============================================================================
	// HPE_END
	//=============================================================================

	outResourceData->GetInt("positionImage", m_bPositionImage );
	outResourceData->SetInt("scaleImage", m_bScaleImage);
	outResourceData->SetFloat("scaleAmount", m_fScaleAmount);
	outResourceData->SetInt("tileImage", m_bTileImage);
	outResourceData->SetInt("tileHorizontally", m_bTileHorizontally);
	outResourceData->SetInt("tileVertically", m_bTileVertically);
}

void ImagePanel::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"image", TYPE_STRING},
    {"fillcolor", TYPE_STRING},
    {"drawcolor", TYPE_STRING},
    {"positionImage", TYPE_BOOL},
    {"scaleImage", TYPE_BOOL},
    {"scaleAmount", TYPE_FLOAT},
    {"tileImage", TYPE_BOOL},
    {"tileHorizontally", TYPE_BOOL},
    {"tileVertically", TYPE_BOOL}
    END_PANEL_SETTINGS();
}

//-----------------------------------------------------------------------------
// Purpose: Applies designer settings from res file
//-----------------------------------------------------------------------------
void ImagePanel::ApplySettings(KeyValues *inResourceData)
{
	m_pszImageName.Purge();
    m_pszFillColorName.Purge();
    m_pszDrawColorName.Purge();

	m_bPositionImage = inResourceData->GetBool("positionImage", true);
	m_bScaleImage = inResourceData->GetBool("scaleImage", false);
	m_fScaleAmount = inResourceData->GetFloat("scaleAmount", 0.0f);
	m_bTileImage = inResourceData->GetBool("tileImage", false);
	m_bTileHorizontally = inResourceData->GetBool("tileHorizontally", m_bTileImage);
	m_bTileVertically = inResourceData->GetBool("tileVertically", m_bTileImage);
	const char *imageName = inResourceData->GetString("image", "");
	if ( *imageName )
	{
		SetImage( imageName );
	}

	const char *pszFillColor = inResourceData->GetString("fillcolor", "");
	if (*pszFillColor)
	{
		int r = 0, g = 0, b = 0, a = 255;
        m_pszFillColorName = pszFillColor;

		if (sscanf(pszFillColor, "%d %d %d %d", &r, &g, &b, &a) >= 3)
		{
			// it's a direct color
			m_FillColor = Color(r, g, b, a);
		}
		else
		{
			IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
			m_FillColor = pScheme->GetColor(pszFillColor, Color(0, 0, 0, 0));
		}
	}

	//=============================================================================
	// HPE_BEGIN:
	// [pfreese] Added support for specifying drawcolor
	//=============================================================================
	const char *pszDrawColor = inResourceData->GetString("drawcolor", "");
	if (*pszDrawColor)
	{
		int r = 255, g = 255, b = 255, a = 255;
        m_pszDrawColorName = pszDrawColor;

		if (sscanf(pszDrawColor, "%d %d %d %d", &r, &g, &b, &a) >= 3)
		{
			// it's a direct color
			m_DrawColor = Color(r, g, b, a);
		}
		else
		{
			IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
			m_DrawColor = pScheme->GetColor(pszDrawColor, Color(255, 255, 255, 255));
		}
	}
	//=============================================================================
	// HPE_END
	//=============================================================================

	BaseClass::ApplySettings(inResourceData);
}

//-----------------------------------------------------------------------------
// Purpose:  load the image, this is done just before this control is displayed
//-----------------------------------------------------------------------------
void ImagePanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
	if ( !m_pszImageName.IsEmpty() )
	{
		SetImage(scheme()->GetImage(m_pszImageName, m_bScaleImage));
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets whether or not the image should scale to fit the size of the ImagePanel (defaults to false)
//-----------------------------------------------------------------------------
void ImagePanel::SetShouldScaleImage( bool state )
{
	m_bScaleImage = state;
}

//-----------------------------------------------------------------------------
// Purpose: gets whether or not the image should be scaled to fit the size of the ImagePanel
//-----------------------------------------------------------------------------
bool ImagePanel::GetShouldScaleImage()
{
	return m_bScaleImage;
}

//-----------------------------------------------------------------------------
// Purpose: used in conjunction with setting that the image should scale and defines an absolute scale amount
//-----------------------------------------------------------------------------
void ImagePanel::SetScaleAmount( float scale )
{
	m_fScaleAmount = scale;
}

float ImagePanel::GetScaleAmount( void )
{
	return m_fScaleAmount;
}

//-----------------------------------------------------------------------------
// Purpose: set the color to fill with, if no Image is specified
//-----------------------------------------------------------------------------
void ImagePanel::SetFillColor( Color col )
{
	m_FillColor = col;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
Color ImagePanel::GetFillColor()
{
	return m_FillColor;
}

const char *ImagePanel::GetImageName()
{
	return m_pszImageName;
}

bool ImagePanel::EvictImage()
{
	if ( !m_pImage )
	{
		// nothing to do
		return false;
	}

	if ( !scheme()->DeleteImage( m_pszImageName ) )
	{
		// no eviction occured, could have an outstanding reference
		return false;
	}

	// clear out our cached concept of it
	// as it may change
	// the next SetImage() will re-establish
	m_pImage = nullptr;
    m_pszImageName.Purge();

	return true;
}

int ImagePanel::GetNumFrames()
{
	if ( !m_pImage )
	{
		return 0;
	}

	return m_pImage->GetNumFrames();
}

void ImagePanel::SetFrame( int nFrame )
{
	if ( !m_pImage )
	{
		return;
	}

	return m_pImage->SetFrame( nFrame );
}
