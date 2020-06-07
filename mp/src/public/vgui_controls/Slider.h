#pragma once

#include <vgui_controls/Panel.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Labeled horizontal slider
//-----------------------------------------------------------------------------
class Slider : public Panel
{
	DECLARE_CLASS_SIMPLE( Slider, Panel );
public:
	Slider(Panel *parent, const char *panelName);

	// interface
	virtual void SetValue(int value, bool bTriggerChangeMessage = true); 
	virtual int  GetValue();
    virtual void SetRange(int min, int max);	 // set to max and min range of rows to display
	virtual void GetRange(int &min, int &max);
	virtual void GetNobPos(int &min, int &max);	// get current Slider position
	virtual void SetButtonOffset(int buttonOffset);
	virtual void SetTickCaptions(const wchar_t *left, const wchar_t *right);
	virtual void SetTickCaptions(const char *left, const char *right);
	virtual void SetNumTicks(int ticks);
	virtual void SetThumbWidth( float width );
	virtual int	 EstimateValueAtPos( int localMouseX, int localMouseY );
	virtual void SetInverted( bool bInverted );
	
	// If you click on the slider outside of the nob, the nob jumps
	// to the click position, and if this setting is enabled, the nob
	// is then draggable from the new position until the mouse is released
	virtual void SetDragOnRepositionNob( bool state );
	virtual bool IsDragOnRepositionNob() const;

	// Get if the slider nob is being dragged by user, usually the application
	// should refuse from forcefully setting slider value if it is being dragged
	// by user since the next frame the nob will pop back to mouse position
	virtual bool IsDragged( void ) const;

	// This allows the slider to behave like it's larger than what's actually being drawn
	virtual void SetSliderThumbSubRange( bool bEnable, int nMin = 0, int nMax = 100 );

protected:
    void OnSizeChanged(int wide, int tall) override;
    void Paint() override;
    void PaintBackground() override;
    void PerformLayout() override;
    void ApplySchemeSettings(IScheme *pScheme) override;
    void GetSettings(KeyValues *outResourceData) override;
    void ApplySettings(KeyValues *inResourceData) override;
    void InitSettings() override;

    void OnKeyCodeTyped(KeyCode code) override;
	void OnCursorMoved(int x, int y) override;
	void OnMousePressed(MouseCode code) override;
	void OnMouseDoublePressed(MouseCode code) override;
	void OnMouseReleased(MouseCode code) override;

	virtual void DrawNob();
	virtual void DrawTicks();
	virtual void DrawTickLabels();

	virtual void GetTrackRect( int &x, int &y, int &w, int &h );

	virtual void RecomputeNobPosFromValue();
	virtual void RecomputeValueFromNobPos();
	
	virtual void SendSliderMovedMessage();
	virtual void SendSliderDragStartMessage();
	virtual void SendSliderDragEndMessage();

	void SetTickColor(Color color) { m_TickColor = color; }

	void ClampRange();

private:
	bool _dragging;
	int _nobPos[2];
	int _nobDragStartPos[2];
	int _dragStartPos[2];
	int _range[2];
	int _subrange[ 2 ];
	int _value;		// the position of the Slider, in coordinates as specified by SetRange/SetRangeWindow
	int _buttonOffset;
	IBorder *_sliderBorder;
	IBorder *_insetBorder;
	float _nobSize;

	TextImage *_leftCaption;
	TextImage *_rightCaption;

	Color m_TickColor;
	Color m_TrackColor;
	Color m_DisabledTextColor1;
	Color m_DisabledTextColor2;

	int		m_nNumTicks;
	bool	m_bIsDragOnRepositionNob;
	bool	m_bUseSubRange;
	bool	m_bInverted;
};

}