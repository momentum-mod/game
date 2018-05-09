//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/RotatingProgressBar.h>

#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

#include "mathlib/mathlib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( RotatingProgressBar );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
RotatingProgressBar::RotatingProgressBar(Panel *parent, const char *panelName) : ProgressBar(parent, panelName)
{
    InitSettings();
	m_flStartRadians = 0.0f;
	m_flEndRadians = 0.0f;
	m_flLastAngle = 0.0f;

	m_nTextureId = -1;
	m_pszImageName = nullptr;

	m_flTickDelay = 30.0f;

	ivgui()->AddTickSignal(GetVPanel(), m_flTickDelay );

	SetPaintBorderEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
RotatingProgressBar::~RotatingProgressBar()
{
	if ( surface() && m_nTextureId != -1 )
	{
		surface()->DestroyTextureID( m_nTextureId );
		m_nTextureId = -1;
	}

	m_pszImageName.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RotatingProgressBar::ApplySettings(KeyValues *inResourceData)
{
	const char *imageName = inResourceData->GetString("image");
	if (*imageName)
	{
		SetImage( imageName );
	}

	// Find min and max rotations in radians
	m_flStartRadians = DEG2RAD(inResourceData->GetFloat( "start_degrees" ) );
	m_flEndRadians = DEG2RAD( inResourceData->GetFloat( "end_degrees") );

	// Start at 0 progress
	m_flLastAngle = m_flStartRadians;

	// approach speed is specified in degrees per second.
	// convert to radians per 1/30th of a second
	float flDegressPerSecond = DEG2RAD( inResourceData->GetFloat( "approach_speed", 360.0 ) );	// default is super fast
	m_flApproachSpeed = flDegressPerSecond * ( m_flTickDelay / 1000.0f );	// divide by number of frames in a second

	m_flRotOriginX = inResourceData->GetFloat( "rot_origin_x_percent", 0.5f );
	m_flRotOriginY = inResourceData->GetFloat( "rot_origin_y_percent", 0.5f );

	m_flRotatingX = inResourceData->GetFloat( "rotating_x" );
	m_flRotatingY = inResourceData->GetFloat( "rotating_y");
	m_flRotatingWide = inResourceData->GetFloat( "rotating_wide" );
	m_flRotatingTall = inResourceData->GetFloat( "rotating_tall" );
	
	BaseClass::ApplySettings( inResourceData );
}

void RotatingProgressBar::GetSettings(KeyValues* outResourceData)
{
    BaseClass::GetSettings(outResourceData);

    outResourceData->SetString("image", m_pszImageName);
    outResourceData->SetFloat("start_degrees", RAD2DEG(m_flStartRadians));
    outResourceData->SetFloat("end_degrees", RAD2DEG(m_flEndRadians));

    outResourceData->SetFloat("approach_speed", RAD2DEG((m_flApproachSpeed * 1000.0f) / m_flTickDelay));

    outResourceData->SetFloat("rotating_x", m_flRotatingX);
    outResourceData->SetFloat("rotating_y", m_flRotatingY);
    outResourceData->SetFloat("rotating_wide", m_flRotatingWide);
    outResourceData->SetFloat("rotating_tall", m_flRotatingTall);
}

void RotatingProgressBar::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"image", TYPE_STRING},
    {"start_degrees", TYPE_FLOAT},
    {"end_degrees", TYPE_FLOAT},
    {"approach_speed", TYPE_FLOAT},
    {"rotating_x", TYPE_FLOAT},
    {"rotating_y", TYPE_FLOAT},
    {"rotating_wide", TYPE_FLOAT},
    {"rotating_tall", TYPE_FLOAT},
    END_PANEL_SETTINGS();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RotatingProgressBar::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if ( !m_pszImageName.IsEmpty() )
	{
		if ( m_nTextureId == -1 )
		{
			m_nTextureId = surface()->CreateNewTextureID();
		}

		surface()->DrawSetTextureFile( m_nTextureId, m_pszImageName, true, false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets an image by file name
//-----------------------------------------------------------------------------
void RotatingProgressBar::SetImage(const char *imageName)
{
	m_pszImageName.Purge();
    m_pszImageName.Format("vgui/%s", imageName);
	InvalidateLayout(false, true); // force applyschemesettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RotatingProgressBar::PaintBackground()
{
	// No background
}

//-----------------------------------------------------------------------------
// Purpose: Update event when we aren't drawing so we don't get huge sweeps
// when we start drawing it
//-----------------------------------------------------------------------------
void RotatingProgressBar::OnTick( void )
{
	float flDesiredAngle = RemapVal( GetProgress(), 0.0, 1.0, m_flStartRadians, m_flEndRadians );
	m_flLastAngle = Approach( flDesiredAngle, m_flLastAngle, m_flApproachSpeed );

	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RotatingProgressBar::Paint()
{
	// we have an image that we rotate based on the progress,
	// where '0' is not rotated,'90' is rotated 90 degrees to the right.
	// Image is rotated around its center.

	// desired rotation is GetProgress() ( 0.0 -> 1.0 ) mapped into
	// ( m_flStartDegrees -> m_flEndDegrees )

	surface()->DrawSetTexture( m_nTextureId );
	surface()->DrawSetColor( Color(255,255,255,255) );

	int wide, tall;
	GetSize( wide, tall );

	float mid_x = m_flRotatingX + m_flRotOriginX * m_flRotatingWide;
	float mid_y = m_flRotatingY + m_flRotOriginY * m_flRotatingTall;

	Vertex_t vert[4];	

	vert[0].Init( Vector2D( m_flRotatingX, m_flRotatingY ), Vector2D(0,0) );
	vert[1].Init( Vector2D( m_flRotatingX+m_flRotatingWide, m_flRotatingY ), Vector2D(1,0) );
	vert[2].Init( Vector2D( m_flRotatingX+m_flRotatingWide, m_flRotatingY+m_flRotatingTall ), Vector2D(1,1) );
	vert[3].Init( Vector2D( m_flRotatingX, m_flRotatingY+m_flRotatingTall ), Vector2D(0,1) );

	float flCosA = cos(m_flLastAngle);
	float flSinA = sin(m_flLastAngle);

	// rotate each point around (mid_x, mid_y) by flAngle radians
	for ( int i=0;i<4;i++ )
	{
		Vector2D result;

		// subtract the (x,y) we're rotating around, we'll add it on at the end.
		vert[i].m_Position.x -= mid_x;
		vert[i].m_Position.y -= mid_y;

		result.x = ( vert[i].m_Position.x * flCosA - vert[i].m_Position.y * flSinA ) + mid_x;
		result.y = ( vert[i].m_Position.x * flSinA + vert[i].m_Position.y * flCosA ) + mid_y;

		vert[i].m_Position = result;
	}

	surface()->DrawTexturedPolygon( 4, vert );
}