// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $CONVERT_FROM_LINEAR == 0 ) && ( $CONVERT_TO_LINEAR == 1 )
// ( $TOOL_MODE == 0 ) && ( $CONVERT_TO_LINEAR == 1 )
// ( $LOCAL_CONTRAST_ENABLE == 0 ) && ( $BLURRED_VIGNETTE_ENABLE == 1 )
// ( $TOOL_MODE == 0 ) && $TV_GAMMA
// ( $TOOL_MODE == 0 ) && $DESATURATEENABLE
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef ENGINE_POST_PS20B_H
#define ENGINE_POST_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class engine_post_ps20b_Static_Index
{
	unsigned int m_nTOOL_MODE : 2;
	unsigned int m_nDEPTH_BLUR_ENABLE : 2;
#ifdef _DEBUG
	bool m_bTOOL_MODE : 1;
	bool m_bDEPTH_BLUR_ENABLE : 1;
#endif	// _DEBUG
public:
	void SetTOOL_MODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTOOL_MODE = i;
#ifdef _DEBUG
		m_bTOOL_MODE = true;
#endif	// _DEBUG
	}

	void SetDEPTH_BLUR_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEPTH_BLUR_ENABLE = i;
#ifdef _DEBUG
		m_bDEPTH_BLUR_ENABLE = true;
#endif	// _DEBUG
	}

	engine_post_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nTOOL_MODE = 0;
		m_nDEPTH_BLUR_ENABLE = 0;
#ifdef _DEBUG
		m_bTOOL_MODE = false;
		m_bDEPTH_BLUR_ENABLE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bTOOL_MODE && m_bDEPTH_BLUR_ENABLE );
		return ( 2048 * m_nTOOL_MODE ) + ( 4096 * m_nDEPTH_BLUR_ENABLE ) + 0;
	}
};

#define shaderStaticTest_engine_post_ps20b psh_forgot_to_set_static_TOOL_MODE + psh_forgot_to_set_static_DEPTH_BLUR_ENABLE


class engine_post_ps20b_Dynamic_Index
{
	unsigned int m_nAA_ENABLE : 1;
	unsigned int m_nCOL_CORRECT_NUM_LOOKUPS : 3;
	unsigned int m_nCONVERT_FROM_LINEAR : 2;
	unsigned int m_nCONVERT_TO_LINEAR : 2;
	unsigned int m_nNOISE_ENABLE : 2;
	unsigned int m_nVIGNETTE_ENABLE : 2;
	unsigned int m_nLOCAL_CONTRAST_ENABLE : 2;
	unsigned int m_nBLURRED_VIGNETTE_ENABLE : 2;
	unsigned int m_nVOMIT_ENABLE : 2;
	unsigned int m_nTV_GAMMA : 2;
	unsigned int m_nDESATURATEENABLE : 2;
#ifdef _DEBUG
	bool m_bAA_ENABLE : 1;
	bool m_bCOL_CORRECT_NUM_LOOKUPS : 1;
	bool m_bCONVERT_FROM_LINEAR : 1;
	bool m_bCONVERT_TO_LINEAR : 1;
	bool m_bNOISE_ENABLE : 1;
	bool m_bVIGNETTE_ENABLE : 1;
	bool m_bLOCAL_CONTRAST_ENABLE : 1;
	bool m_bBLURRED_VIGNETTE_ENABLE : 1;
	bool m_bVOMIT_ENABLE : 1;
	bool m_bTV_GAMMA : 1;
	bool m_bDESATURATEENABLE : 1;
#endif	// _DEBUG
public:
	void SetAA_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nAA_ENABLE = i;
#ifdef _DEBUG
		m_bAA_ENABLE = true;
#endif	// _DEBUG
	}

	void SetCOL_CORRECT_NUM_LOOKUPS( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nCOL_CORRECT_NUM_LOOKUPS = i;
#ifdef _DEBUG
		m_bCOL_CORRECT_NUM_LOOKUPS = true;
#endif	// _DEBUG
	}

	void SetCONVERT_FROM_LINEAR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_FROM_LINEAR = i;
#ifdef _DEBUG
		m_bCONVERT_FROM_LINEAR = true;
#endif	// _DEBUG
	}

	void SetCONVERT_TO_LINEAR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_LINEAR = i;
#ifdef _DEBUG
		m_bCONVERT_TO_LINEAR = true;
#endif	// _DEBUG
	}

	void SetNOISE_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNOISE_ENABLE = i;
#ifdef _DEBUG
		m_bNOISE_ENABLE = true;
#endif	// _DEBUG
	}

	void SetVIGNETTE_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVIGNETTE_ENABLE = i;
#ifdef _DEBUG
		m_bVIGNETTE_ENABLE = true;
#endif	// _DEBUG
	}

	void SetLOCAL_CONTRAST_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLOCAL_CONTRAST_ENABLE = i;
#ifdef _DEBUG
		m_bLOCAL_CONTRAST_ENABLE = true;
#endif	// _DEBUG
	}

	void SetBLURRED_VIGNETTE_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLURRED_VIGNETTE_ENABLE = i;
#ifdef _DEBUG
		m_bBLURRED_VIGNETTE_ENABLE = true;
#endif	// _DEBUG
	}

	void SetVOMIT_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVOMIT_ENABLE = i;
#ifdef _DEBUG
		m_bVOMIT_ENABLE = true;
#endif	// _DEBUG
	}

	void SetTV_GAMMA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTV_GAMMA = i;
#ifdef _DEBUG
		m_bTV_GAMMA = true;
#endif	// _DEBUG
	}

	void SetDESATURATEENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDESATURATEENABLE = i;
#ifdef _DEBUG
		m_bDESATURATEENABLE = true;
#endif	// _DEBUG
	}

	engine_post_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nAA_ENABLE = 0;
		m_nCOL_CORRECT_NUM_LOOKUPS = 0;
		m_nCONVERT_FROM_LINEAR = 0;
		m_nCONVERT_TO_LINEAR = 0;
		m_nNOISE_ENABLE = 0;
		m_nVIGNETTE_ENABLE = 0;
		m_nLOCAL_CONTRAST_ENABLE = 0;
		m_nBLURRED_VIGNETTE_ENABLE = 0;
		m_nVOMIT_ENABLE = 0;
		m_nTV_GAMMA = 0;
		m_nDESATURATEENABLE = 0;
#ifdef _DEBUG
		m_bAA_ENABLE = false;
		m_bCOL_CORRECT_NUM_LOOKUPS = false;
		m_bCONVERT_FROM_LINEAR = false;
		m_bCONVERT_TO_LINEAR = false;
		m_bNOISE_ENABLE = false;
		m_bVIGNETTE_ENABLE = false;
		m_bLOCAL_CONTRAST_ENABLE = false;
		m_bBLURRED_VIGNETTE_ENABLE = false;
		m_bVOMIT_ENABLE = false;
		m_bTV_GAMMA = false;
		m_bDESATURATEENABLE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bAA_ENABLE && m_bCOL_CORRECT_NUM_LOOKUPS && m_bCONVERT_FROM_LINEAR && m_bCONVERT_TO_LINEAR && m_bNOISE_ENABLE && m_bVIGNETTE_ENABLE && m_bLOCAL_CONTRAST_ENABLE && m_bBLURRED_VIGNETTE_ENABLE && m_bVOMIT_ENABLE && m_bTV_GAMMA && m_bDESATURATEENABLE );
		return ( 1 * m_nAA_ENABLE ) + ( 1 * m_nCOL_CORRECT_NUM_LOOKUPS ) + ( 4 * m_nCONVERT_FROM_LINEAR ) + ( 8 * m_nCONVERT_TO_LINEAR ) + ( 16 * m_nNOISE_ENABLE ) + ( 32 * m_nVIGNETTE_ENABLE ) + ( 64 * m_nLOCAL_CONTRAST_ENABLE ) + ( 128 * m_nBLURRED_VIGNETTE_ENABLE ) + ( 256 * m_nVOMIT_ENABLE ) + ( 512 * m_nTV_GAMMA ) + ( 1024 * m_nDESATURATEENABLE ) + 0;
	}
};

#define shaderDynamicTest_engine_post_ps20b psh_forgot_to_set_dynamic_AA_ENABLE + psh_forgot_to_set_dynamic_COL_CORRECT_NUM_LOOKUPS + psh_forgot_to_set_dynamic_CONVERT_FROM_LINEAR + psh_forgot_to_set_dynamic_CONVERT_TO_LINEAR + psh_forgot_to_set_dynamic_NOISE_ENABLE + psh_forgot_to_set_dynamic_VIGNETTE_ENABLE + psh_forgot_to_set_dynamic_LOCAL_CONTRAST_ENABLE + psh_forgot_to_set_dynamic_BLURRED_VIGNETTE_ENABLE + psh_forgot_to_set_dynamic_VOMIT_ENABLE + psh_forgot_to_set_dynamic_TV_GAMMA + psh_forgot_to_set_dynamic_DESATURATEENABLE


#endif	// ENGINE_POST_PS20B_H