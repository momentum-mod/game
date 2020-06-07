// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SPRITECARD_PS20B_H
#define SPRITECARD_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class spritecard_ps20b_Static_Index
{
	unsigned int m_nCONVERT_TO_SRGB : 2;
	unsigned int m_nDUALSEQUENCE : 2;
	unsigned int m_nSEQUENCE_BLEND_MODE : 2;
	unsigned int m_nADDBASETEXTURE2 : 2;
	unsigned int m_nMAXLUMFRAMEBLEND1 : 2;
	unsigned int m_nMAXLUMFRAMEBLEND2 : 2;
	unsigned int m_nEXTRACTGREENALPHA : 2;
	unsigned int m_nCOLORRAMP : 2;
	unsigned int m_nANIMBLEND : 2;
	unsigned int m_nADDSELF : 2;
	unsigned int m_nDEPTHBLEND : 2;
#ifdef _DEBUG
	bool m_bDUALSEQUENCE : 1;
	bool m_bSEQUENCE_BLEND_MODE : 1;
	bool m_bADDBASETEXTURE2 : 1;
	bool m_bMAXLUMFRAMEBLEND1 : 1;
	bool m_bMAXLUMFRAMEBLEND2 : 1;
	bool m_bEXTRACTGREENALPHA : 1;
	bool m_bCOLORRAMP : 1;
	bool m_bANIMBLEND : 1;
	bool m_bADDSELF : 1;
	bool m_bDEPTHBLEND : 1;
#endif	// _DEBUG
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_SRGB = i;
	}

	void SetDUALSEQUENCE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDUALSEQUENCE = i;
#ifdef _DEBUG
		m_bDUALSEQUENCE = true;
#endif	// _DEBUG
	}

	void SetSEQUENCE_BLEND_MODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nSEQUENCE_BLEND_MODE = i;
#ifdef _DEBUG
		m_bSEQUENCE_BLEND_MODE = true;
#endif	// _DEBUG
	}

	void SetADDBASETEXTURE2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nADDBASETEXTURE2 = i;
#ifdef _DEBUG
		m_bADDBASETEXTURE2 = true;
#endif	// _DEBUG
	}

	void SetMAXLUMFRAMEBLEND1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMAXLUMFRAMEBLEND1 = i;
#ifdef _DEBUG
		m_bMAXLUMFRAMEBLEND1 = true;
#endif	// _DEBUG
	}

	void SetMAXLUMFRAMEBLEND2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMAXLUMFRAMEBLEND2 = i;
#ifdef _DEBUG
		m_bMAXLUMFRAMEBLEND2 = true;
#endif	// _DEBUG
	}

	void SetEXTRACTGREENALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEXTRACTGREENALPHA = i;
#ifdef _DEBUG
		m_bEXTRACTGREENALPHA = true;
#endif	// _DEBUG
	}

	void SetCOLORRAMP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLORRAMP = i;
#ifdef _DEBUG
		m_bCOLORRAMP = true;
#endif	// _DEBUG
	}

	void SetANIMBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nANIMBLEND = i;
#ifdef _DEBUG
		m_bANIMBLEND = true;
#endif	// _DEBUG
	}

	void SetADDSELF( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nADDSELF = i;
#ifdef _DEBUG
		m_bADDSELF = true;
#endif	// _DEBUG
	}

	void SetDEPTHBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEPTHBLEND = i;
#ifdef _DEBUG
		m_bDEPTHBLEND = true;
#endif	// _DEBUG
	}

	spritecard_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = g_pHardwareConfig->NeedsShaderSRGBConversion();
		m_nDUALSEQUENCE = 0;
		m_nSEQUENCE_BLEND_MODE = 0;
		m_nADDBASETEXTURE2 = 0;
		m_nMAXLUMFRAMEBLEND1 = 0;
		m_nMAXLUMFRAMEBLEND2 = 0;
		m_nEXTRACTGREENALPHA = 0;
		m_nCOLORRAMP = 0;
		m_nANIMBLEND = 0;
		m_nADDSELF = 0;
		m_nDEPTHBLEND = 0;
#ifdef _DEBUG
		m_bDUALSEQUENCE = false;
		m_bSEQUENCE_BLEND_MODE = false;
		m_bADDBASETEXTURE2 = false;
		m_bMAXLUMFRAMEBLEND1 = false;
		m_bMAXLUMFRAMEBLEND2 = false;
		m_bEXTRACTGREENALPHA = false;
		m_bCOLORRAMP = false;
		m_bANIMBLEND = false;
		m_bADDSELF = false;
		m_bDEPTHBLEND = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDUALSEQUENCE && m_bSEQUENCE_BLEND_MODE && m_bADDBASETEXTURE2 && m_bMAXLUMFRAMEBLEND1 && m_bMAXLUMFRAMEBLEND2 && m_bEXTRACTGREENALPHA && m_bCOLORRAMP && m_bANIMBLEND && m_bADDSELF && m_bDEPTHBLEND );
		return ( 1 * m_nCONVERT_TO_SRGB ) + ( 2 * m_nDUALSEQUENCE ) + ( 4 * m_nSEQUENCE_BLEND_MODE ) + ( 12 * m_nADDBASETEXTURE2 ) + ( 24 * m_nMAXLUMFRAMEBLEND1 ) + ( 48 * m_nMAXLUMFRAMEBLEND2 ) + ( 96 * m_nEXTRACTGREENALPHA ) + ( 192 * m_nCOLORRAMP ) + ( 384 * m_nANIMBLEND ) + ( 768 * m_nADDSELF ) + ( 1536 * m_nDEPTHBLEND ) + 0;
	}
};

#define shaderStaticTest_spritecard_ps20b psh_forgot_to_set_static_DUALSEQUENCE + psh_forgot_to_set_static_SEQUENCE_BLEND_MODE + psh_forgot_to_set_static_ADDBASETEXTURE2 + psh_forgot_to_set_static_MAXLUMFRAMEBLEND1 + psh_forgot_to_set_static_MAXLUMFRAMEBLEND2 + psh_forgot_to_set_static_EXTRACTGREENALPHA + psh_forgot_to_set_static_COLORRAMP + psh_forgot_to_set_static_ANIMBLEND + psh_forgot_to_set_static_ADDSELF + psh_forgot_to_set_static_DEPTHBLEND


class spritecard_ps20b_Dynamic_Index
{
public:
	spritecard_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_spritecard_ps20b 1


#endif	// SPRITECARD_PS20B_H