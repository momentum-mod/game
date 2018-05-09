//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ProgressBar.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Progress Bar in the shape of a pie graph
//-----------------------------------------------------------------------------
class CircularProgressBar : public ProgressBar
{
	DECLARE_CLASS_SIMPLE( CircularProgressBar, ProgressBar );

public:
	CircularProgressBar(Panel *parent, const char *panelName);
	~CircularProgressBar();

	void SetFgImage(const char *imageName) { SetImage( imageName, true ); }
	void SetBgImage(const char *imageName) { SetImage( imageName, false ); }

	enum CircularProgressDir_e
	{
		PROGRESS_CW,
		PROGRESS_CCW
	};
	int GetProgressDirection() const { return m_iProgressDirection; }
	void SetProgressDirection( int val ) { m_iProgressDirection = val; }
	void SetStartSegment( int val ) { m_iStartSegment = val; }

protected:
	void Paint() OVERRIDE;
	void PaintBackground() OVERRIDE;
    void ApplySettings(KeyValues *inResourceData) OVERRIDE;
    void GetSettings(KeyValues* outResourceData) OVERRIDE;
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;
    void InitSettings() OVERRIDE;
	
	void DrawCircleSegment( Color c, float flEndDegrees, bool clockwise /* = true */ );
	void SetImage(const char *imageName, bool isFg);

private:
	int m_iProgressDirection;
	int m_iStartSegment;

	int m_nTextureIdFG, m_nTextureIdBG;
	CUtlString m_ImageFGName;
    CUtlString m_ImageBGName;
};

} // namespace vgui

#endif // CIRCULARPROGRESSBAR_H