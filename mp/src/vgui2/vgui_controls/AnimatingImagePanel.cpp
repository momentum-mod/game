//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>
#define PROTECTED_THINGS_DISABLE

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IImage.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

#include <vgui_controls/AnimatingImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( AnimatingImagePanel );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
AnimatingImagePanel::AnimatingImagePanel(Panel *parent, const char *name) : Panel(parent, name)
{
    InitSettings();
	m_iCurrentImage = 0;
	m_iFrameTimeMillis = 100;	// 10Hz frame rate
	m_iNextFrameTime = 0;
	m_bFiltered = false;
	m_bScaleImage = false;
	m_bAnimating = false;
	ivgui()->AddTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: Layout the panel for drawing.
//-----------------------------------------------------------------------------
void AnimatingImagePanel::PerformLayout()
{
	Panel::PerformLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Add an image to the end of the list of animations
//-----------------------------------------------------------------------------
void AnimatingImagePanel::AddImage(IImage *image)
{
	m_Frames.AddToTail(image);

	if ( !m_bScaleImage && image != NULL )
	{
		int wide,tall;
		image->GetSize(wide,tall);
		SetSize(wide,tall);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load a set of animations by name.
// Input: 
//		baseName: is the name of the animations without their frame number or 
//			file extension, (e.g. c1.tga becomes just c.)
//		framecount: number of frames in the animation
//-----------------------------------------------------------------------------
void AnimatingImagePanel::LoadAnimation(const char *baseName, int frameCount)
{
	m_Frames.RemoveAll();
	for (int i = 1; i <= frameCount; i++)
	{
		char imageName[512];
		Q_snprintf(imageName, sizeof( imageName ), "%s%d", baseName, i);
		AddImage(scheme()->GetImage(imageName, m_bFiltered));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw the current image
//-----------------------------------------------------------------------------
void AnimatingImagePanel::PaintBackground()
{
	if ( m_Frames.IsValidIndex( m_iCurrentImage ) && m_Frames[m_iCurrentImage] != NULL )
	{
		IImage *pImage = m_Frames[m_iCurrentImage];

		surface()->DrawSetColor( 255, 255, 255, 255 );
		pImage->SetPos(0, 0);
		
		if ( m_bScaleImage )
		{
			// Image size is stored in the bitmap, so temporarily set its size
			// to our panel size and then restore after we draw it.

			int imageWide, imageTall;
			pImage->GetSize( imageWide, imageTall );

			int wide, tall;
			GetSize( wide, tall );
			pImage->SetSize( wide, tall );

			pImage->SetColor( Color( 255,255,255,255 ) );
			pImage->Paint();

			pImage->SetSize( imageWide, imageTall );
		}
		else
		{
			pImage->Paint();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame the panel is visible
//-----------------------------------------------------------------------------
void AnimatingImagePanel::OnTick()
{
	if (m_bAnimating && system()->GetTimeMillis() >= m_iNextFrameTime)
	{
		m_iNextFrameTime = system()->GetTimeMillis() + m_iFrameTimeMillis;
		m_iCurrentImage++;
		if (!m_Frames.IsValidIndex(m_iCurrentImage))
		{
			m_iCurrentImage = 0;
		}
		Repaint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
// Output: outResourceData- a set of keyvalues of imagenames.
//-----------------------------------------------------------------------------
void AnimatingImagePanel::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);
	outResourceData->SetString("image", m_pImageName);
    outResourceData->SetBool("scaleImage", m_bScaleImage);
    outResourceData->SetInt("frames", m_Frames.Count());
    outResourceData->SetInt("anim_framerate", m_iFrameTimeMillis);
}

//-----------------------------------------------------------------------------
// Purpose: Applies resource settings
//-----------------------------------------------------------------------------
void AnimatingImagePanel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

    m_bScaleImage = inResourceData->GetBool("scaleImage", false);
	const char *imageName = inResourceData->GetString("image");
	if (*imageName)
	{
        m_pImageName.Purge();
        m_pImageName = imageName;

		// add in the command
		LoadAnimation(m_pImageName, inResourceData->GetInt("frames"));
	}

	m_iFrameTimeMillis = inResourceData->GetInt( "anim_framerate", 100 );
}

void AnimatingImagePanel::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"image", TYPE_STRING},
    {"scaleImage", TYPE_BOOL},
    {"frames", TYPE_INTEGER},
    {"anim_framerate", TYPE_INTEGER}
    END_PANEL_SETTINGS();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the image doing its animation
//-----------------------------------------------------------------------------
void AnimatingImagePanel::StartAnimation()
{
	m_bAnimating = true;
//	ivgui()->AddTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: Stops the images animation
//-----------------------------------------------------------------------------
void AnimatingImagePanel::StopAnimation()
{
	m_bAnimating = false;
//	ivgui()->RemoveTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: Resets the animation to the start of the sequence.
//-----------------------------------------------------------------------------
void AnimatingImagePanel::ResetAnimation(int frame)
{
	if(m_Frames.IsValidIndex(frame))
	{
		m_iCurrentImage = frame;
	}
	else
	{
		m_iCurrentImage = 0;
	}
	Repaint();
}
