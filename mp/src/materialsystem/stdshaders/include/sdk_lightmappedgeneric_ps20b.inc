// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $SEAMLESS && $RELIEF_MAPPING
// (! $DETAILTEXTURE) && ( $DETAIL_BLEND_MODE != 0 )
// $SEAMLESS && ( $OUTLINE || $SOFTEDGES)
// $BASETEXTURE2 && ( $OUTLINE || $SOFTEDGES)
// $BUMPMAP2 && ( $OUTLINE || $SOFTEDGES)
// $SELFILLUM && ( $OUTLINE || $SOFTEDGES)
// $MASKEDBLENDING && ( $OUTLINE || $SOFTEDGES)
// $FANCY_BLENDING && ( $OUTLINE || $SOFTEDGES)
// $LIGHTING_PREVIEW && ( $OUTLINE || $SOFTEDGES)
// ($FASTPATH == 0) && ( $OUTLINE || $SOFTEDGES)
// ($DETAILTEXTURE && $BUMPMAP) && ( $OUTLINE || $SOFTEDGES)
// ($WARPLIGHTING) && ( $OUTLINE || $SOFTEDGES)
// ($BUMPMAP) && ( $OUTLINE || $SOFTEDGES)
// ($DETAIL_BLEND_MODE == 2 ) || ($DETAIL_BLEND_MODE == 3 ) || ($DETAIL_BLEND_MODE == 4 )
// ($DETAIL_BLEND_MODE == 5 ) || ($DETAIL_BLEND_MODE == 6 ) || ($DETAIL_BLEND_MODE == 7 )
// ($DETAIL_BLEND_MODE == 8 ) || ($DETAIL_BLEND_MODE == 9 )
// $PARALLAXCORRECT && !$CUBEMAP
// $BUMPMAP2 && $WARPLIGHTING
// $WARPLIGHTING && $DETAILTEXTURE
// $ENVMAPMASK && $BUMPMAP
// $NORMALMAPALPHAENVMAPMASK && $BASEALPHAENVMAPMASK
// $NORMALMAPALPHAENVMAPMASK && $ENVMAPMASK
// $BASEALPHAENVMAPMASK && $ENVMAPMASK
// $BASEALPHAENVMAPMASK && $SELFILLUM
// !$FASTPATH && $FASTPATHENVMAPCONTRAST
// !$FASTPATH && $FASTPATHENVMAPTINT
// !$BUMPMAP && $DIFFUSEBUMPMAP
// !$BUMPMAP && $BUMPMAP2
// $ENVMAPMASK && $BUMPMAP2
// $BASETEXTURENOENVMAP && ( !$BASETEXTURE2 || !$CUBEMAP )
// $BASETEXTURE2NOENVMAP && ( !$BASETEXTURE2 || !$CUBEMAP )
// $BASEALPHAENVMAPMASK && $BUMPMAP
// $PARALLAXMAP && $DETAILTEXTURE
// $SEAMLESS && $RELIEF_MAPPING
// $SEAMLESS && $DETAILTEXTURE
// $SEAMLESS && $MASKEDBLENDING
// $BUMPMASK && ( $SEAMLESS || $DETAILTEXTURE || $SELFILLUM || $BASETEXTURENOENVMAP || $BASETEXTURE2 )
// !$BUMPMAP && ($NORMAL_DECODE_MODE == 1)
// !$BUMPMAP && ($NORMAL_DECODE_MODE == 2)
// !$BUMPMAP && ($NORMALMASK_DECODE_MODE == 1)
// !$BUMPMAP && ($NORMALMASK_DECODE_MODE == 2)
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SDK_LIGHTMAPPEDGENERIC_PS20B_H
#define SDK_LIGHTMAPPEDGENERIC_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_lightmappedgeneric_ps20b_Static_Index
{
	int m_nMASKEDBLENDING : 3;
	int m_nBASETEXTURE2 : 3;
	int m_nDETAILTEXTURE : 3;
	int m_nBUMPMAP : 3;
	int m_nBUMPMAP2 : 3;
	int m_nCUBEMAP : 3;
	int m_nENVMAPMASK : 3;
	int m_nBASEALPHAENVMAPMASK : 3;
	int m_nSELFILLUM : 3;
	int m_nNORMALMAPALPHAENVMAPMASK : 3;
	int m_nDIFFUSEBUMPMAP : 3;
	int m_nBASETEXTURENOENVMAP : 3;
	int m_nBASETEXTURE2NOENVMAP : 3;
	int m_nWARPLIGHTING : 3;
	int m_nFANCY_BLENDING : 3;
	int m_nRELIEF_MAPPING : 2;
	int m_nSEAMLESS : 3;
	int m_nOUTLINE : 3;
	int m_nSOFTEDGES : 3;
	int m_nBUMPMASK : 3;
	int m_nNORMAL_DECODE_MODE : 2;
	int m_nNORMALMASK_DECODE_MODE : 2;
	int m_nDETAIL_BLEND_MODE : 5;
	int m_nPARALLAXCORRECT : 3;
#ifdef _DEBUG
	bool m_bMASKEDBLENDING : 1;
	bool m_bBASETEXTURE2 : 1;
	bool m_bDETAILTEXTURE : 1;
	bool m_bBUMPMAP : 1;
	bool m_bBUMPMAP2 : 1;
	bool m_bCUBEMAP : 1;
	bool m_bENVMAPMASK : 1;
	bool m_bBASEALPHAENVMAPMASK : 1;
	bool m_bSELFILLUM : 1;
	bool m_bNORMALMAPALPHAENVMAPMASK : 1;
	bool m_bDIFFUSEBUMPMAP : 1;
	bool m_bBASETEXTURENOENVMAP : 1;
	bool m_bBASETEXTURE2NOENVMAP : 1;
	bool m_bWARPLIGHTING : 1;
	bool m_bFANCY_BLENDING : 1;
	bool m_bRELIEF_MAPPING : 1;
	bool m_bSEAMLESS : 1;
	bool m_bOUTLINE : 1;
	bool m_bSOFTEDGES : 1;
	bool m_bBUMPMASK : 1;
	bool m_bNORMAL_DECODE_MODE : 1;
	bool m_bNORMALMASK_DECODE_MODE : 1;
	bool m_bDETAIL_BLEND_MODE : 1;
	bool m_bPARALLAXCORRECT : 1;
#endif	// _DEBUG
public:
	void SetMASKEDBLENDING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMASKEDBLENDING = i;
#ifdef _DEBUG
		m_bMASKEDBLENDING = true;
#endif	// _DEBUG
	}

	void SetBASETEXTURE2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE2 = i;
#ifdef _DEBUG
		m_bBASETEXTURE2 = true;
#endif	// _DEBUG
	}

	void SetDETAILTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif	// _DEBUG
	}

	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif	// _DEBUG
	}

	void SetBUMPMAP2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP2 = i;
#ifdef _DEBUG
		m_bBUMPMAP2 = true;
#endif	// _DEBUG
	}

	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif	// _DEBUG
	}

	void SetENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAPMASK = i;
#ifdef _DEBUG
		m_bENVMAPMASK = true;
#endif	// _DEBUG
	}

	void SetBASEALPHAENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASEALPHAENVMAPMASK = i;
#ifdef _DEBUG
		m_bBASEALPHAENVMAPMASK = true;
#endif	// _DEBUG
	}

	void SetSELFILLUM( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSELFILLUM = i;
#ifdef _DEBUG
		m_bSELFILLUM = true;
#endif	// _DEBUG
	}

	void SetNORMALMAPALPHAENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNORMALMAPALPHAENVMAPMASK = i;
#ifdef _DEBUG
		m_bNORMALMAPALPHAENVMAPMASK = true;
#endif	// _DEBUG
	}

	void SetDIFFUSEBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDIFFUSEBUMPMAP = i;
#ifdef _DEBUG
		m_bDIFFUSEBUMPMAP = true;
#endif	// _DEBUG
	}

	void SetBASETEXTURENOENVMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURENOENVMAP = i;
#ifdef _DEBUG
		m_bBASETEXTURENOENVMAP = true;
#endif	// _DEBUG
	}

	void SetBASETEXTURE2NOENVMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE2NOENVMAP = i;
#ifdef _DEBUG
		m_bBASETEXTURE2NOENVMAP = true;
#endif	// _DEBUG
	}

	void SetWARPLIGHTING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWARPLIGHTING = i;
#ifdef _DEBUG
		m_bWARPLIGHTING = true;
#endif	// _DEBUG
	}

	void SetFANCY_BLENDING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFANCY_BLENDING = i;
#ifdef _DEBUG
		m_bFANCY_BLENDING = true;
#endif	// _DEBUG
	}

	void SetRELIEF_MAPPING( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nRELIEF_MAPPING = i;
#ifdef _DEBUG
		m_bRELIEF_MAPPING = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif	// _DEBUG
	}

	void SetOUTLINE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nOUTLINE = i;
#ifdef _DEBUG
		m_bOUTLINE = true;
#endif	// _DEBUG
	}

	void SetSOFTEDGES( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSOFTEDGES = i;
#ifdef _DEBUG
		m_bSOFTEDGES = true;
#endif	// _DEBUG
	}

	void SetBUMPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMASK = i;
#ifdef _DEBUG
		m_bBUMPMASK = true;
#endif	// _DEBUG
	}

	void SetNORMAL_DECODE_MODE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nNORMAL_DECODE_MODE = i;
#ifdef _DEBUG
		m_bNORMAL_DECODE_MODE = true;
#endif	// _DEBUG
	}

	void SetNORMALMASK_DECODE_MODE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nNORMALMASK_DECODE_MODE = i;
#ifdef _DEBUG
		m_bNORMALMASK_DECODE_MODE = true;
#endif	// _DEBUG
	}

	void SetDETAIL_BLEND_MODE( int i )
	{
		Assert( i >= 0 && i <= 11 );
		m_nDETAIL_BLEND_MODE = i;
#ifdef _DEBUG
		m_bDETAIL_BLEND_MODE = true;
#endif	// _DEBUG
	}

	void SetPARALLAXCORRECT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPARALLAXCORRECT = i;
#ifdef _DEBUG
		m_bPARALLAXCORRECT = true;
#endif	// _DEBUG
	}

	sdk_lightmappedgeneric_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nMASKEDBLENDING = 0;
		m_nBASETEXTURE2 = 0;
		m_nDETAILTEXTURE = 0;
		m_nBUMPMAP = 0;
		m_nBUMPMAP2 = 0;
		m_nCUBEMAP = 0;
		m_nENVMAPMASK = 0;
		m_nBASEALPHAENVMAPMASK = 0;
		m_nSELFILLUM = 0;
		m_nNORMALMAPALPHAENVMAPMASK = 0;
		m_nDIFFUSEBUMPMAP = 0;
		m_nBASETEXTURENOENVMAP = 0;
		m_nBASETEXTURE2NOENVMAP = 0;
		m_nWARPLIGHTING = 0;
		m_nFANCY_BLENDING = 0;
		m_nRELIEF_MAPPING = 0;
		m_nSEAMLESS = 0;
		m_nOUTLINE = 0;
		m_nSOFTEDGES = 0;
		m_nBUMPMASK = 0;
		m_nNORMAL_DECODE_MODE = 0;
		m_nNORMALMASK_DECODE_MODE = 0;
		m_nDETAIL_BLEND_MODE = 0;
		m_nPARALLAXCORRECT = 0;
#ifdef _DEBUG
		m_bMASKEDBLENDING = false;
		m_bBASETEXTURE2 = false;
		m_bDETAILTEXTURE = false;
		m_bBUMPMAP = false;
		m_bBUMPMAP2 = false;
		m_bCUBEMAP = false;
		m_bENVMAPMASK = false;
		m_bBASEALPHAENVMAPMASK = false;
		m_bSELFILLUM = false;
		m_bNORMALMAPALPHAENVMAPMASK = false;
		m_bDIFFUSEBUMPMAP = false;
		m_bBASETEXTURENOENVMAP = false;
		m_bBASETEXTURE2NOENVMAP = false;
		m_bWARPLIGHTING = false;
		m_bFANCY_BLENDING = false;
		m_bRELIEF_MAPPING = false;
		m_bSEAMLESS = false;
		m_bOUTLINE = false;
		m_bSOFTEDGES = false;
		m_bBUMPMASK = false;
		m_bNORMAL_DECODE_MODE = false;
		m_bNORMALMASK_DECODE_MODE = false;
		m_bDETAIL_BLEND_MODE = false;
		m_bPARALLAXCORRECT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bMASKEDBLENDING && m_bBASETEXTURE2 && m_bDETAILTEXTURE && m_bBUMPMAP && m_bBUMPMAP2 && m_bCUBEMAP && m_bENVMAPMASK && m_bBASEALPHAENVMAPMASK && m_bSELFILLUM && m_bNORMALMAPALPHAENVMAPMASK && m_bDIFFUSEBUMPMAP && m_bBASETEXTURENOENVMAP && m_bBASETEXTURE2NOENVMAP && m_bWARPLIGHTING && m_bFANCY_BLENDING && m_bRELIEF_MAPPING && m_bSEAMLESS && m_bOUTLINE && m_bSOFTEDGES && m_bBUMPMASK && m_bNORMAL_DECODE_MODE && m_bNORMALMASK_DECODE_MODE && m_bDETAIL_BLEND_MODE && m_bPARALLAXCORRECT );
		return ( 96 * m_nMASKEDBLENDING ) + ( 192 * m_nBASETEXTURE2 ) + ( 384 * m_nDETAILTEXTURE ) + ( 768 * m_nBUMPMAP ) + ( 2304 * m_nBUMPMAP2 ) + ( 4608 * m_nCUBEMAP ) + ( 9216 * m_nENVMAPMASK ) + ( 18432 * m_nBASEALPHAENVMAPMASK ) + ( 36864 * m_nSELFILLUM ) + ( 73728 * m_nNORMALMAPALPHAENVMAPMASK ) + ( 147456 * m_nDIFFUSEBUMPMAP ) + ( 294912 * m_nBASETEXTURENOENVMAP ) + ( 589824 * m_nBASETEXTURE2NOENVMAP ) + ( 1179648 * m_nWARPLIGHTING ) + ( 2359296 * m_nFANCY_BLENDING ) + ( 4718592 * m_nRELIEF_MAPPING ) + ( 4718592 * m_nSEAMLESS ) + ( 9437184 * m_nOUTLINE ) + ( 18874368 * m_nSOFTEDGES ) + ( 37748736 * m_nBUMPMASK ) + ( 75497472 * m_nNORMAL_DECODE_MODE ) + ( 75497472 * m_nNORMALMASK_DECODE_MODE ) + ( 75497472 * m_nDETAIL_BLEND_MODE ) + ( 905969664 * m_nPARALLAXCORRECT ) + 0;
	}
};

#define shaderStaticTest_sdk_lightmappedgeneric_ps20b psh_forgot_to_set_static_MASKEDBLENDING + psh_forgot_to_set_static_BASETEXTURE2 + psh_forgot_to_set_static_DETAILTEXTURE + psh_forgot_to_set_static_BUMPMAP + psh_forgot_to_set_static_BUMPMAP2 + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_ENVMAPMASK + psh_forgot_to_set_static_BASEALPHAENVMAPMASK + psh_forgot_to_set_static_SELFILLUM + psh_forgot_to_set_static_NORMALMAPALPHAENVMAPMASK + psh_forgot_to_set_static_DIFFUSEBUMPMAP + psh_forgot_to_set_static_BASETEXTURENOENVMAP + psh_forgot_to_set_static_BASETEXTURE2NOENVMAP + psh_forgot_to_set_static_WARPLIGHTING + psh_forgot_to_set_static_FANCY_BLENDING + psh_forgot_to_set_static_RELIEF_MAPPING + psh_forgot_to_set_static_SEAMLESS + psh_forgot_to_set_static_OUTLINE + psh_forgot_to_set_static_SOFTEDGES + psh_forgot_to_set_static_BUMPMASK + psh_forgot_to_set_static_NORMAL_DECODE_MODE + psh_forgot_to_set_static_NORMALMASK_DECODE_MODE + psh_forgot_to_set_static_DETAIL_BLEND_MODE + psh_forgot_to_set_static_PARALLAXCORRECT


class sdk_lightmappedgeneric_ps20b_Dynamic_Index
{
	int m_nFASTPATHENVMAPCONTRAST : 3;
	int m_nFASTPATH : 3;
	int m_nWRITEWATERFOGTODESTALPHA : 3;
	int m_nPIXELFOGTYPE : 3;
	int m_nLIGHTING_PREVIEW : 3;
	int m_nWRITE_DEPTH_TO_DESTALPHA : 3;
#ifdef _DEBUG
	bool m_bFASTPATHENVMAPCONTRAST : 1;
	bool m_bFASTPATH : 1;
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bPIXELFOGTYPE : 1;
	bool m_bLIGHTING_PREVIEW : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetFASTPATHENVMAPCONTRAST( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFASTPATHENVMAPCONTRAST = i;
#ifdef _DEBUG
		m_bFASTPATHENVMAPCONTRAST = true;
#endif	// _DEBUG
	}

	void SetFASTPATH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFASTPATH = i;
#ifdef _DEBUG
		m_bFASTPATH = true;
#endif	// _DEBUG
	}

	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif	// _DEBUG
	}

	void SetLIGHTING_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nLIGHTING_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHTING_PREVIEW = true;
#endif	// _DEBUG
	}

	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif	// _DEBUG
	}

	sdk_lightmappedgeneric_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nFASTPATHENVMAPCONTRAST = 0;
		m_nFASTPATH = 0;
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nPIXELFOGTYPE = 0;
		m_nLIGHTING_PREVIEW = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bFASTPATHENVMAPCONTRAST = false;
		m_bFASTPATH = false;
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bPIXELFOGTYPE = false;
		m_bLIGHTING_PREVIEW = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFASTPATHENVMAPCONTRAST && m_bFASTPATH && m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bLIGHTING_PREVIEW && m_bWRITE_DEPTH_TO_DESTALPHA );
		return ( 1 * m_nFASTPATHENVMAPCONTRAST ) + ( 2 * m_nFASTPATH ) + ( 4 * m_nWRITEWATERFOGTODESTALPHA ) + ( 8 * m_nPIXELFOGTYPE ) + ( 16 * m_nLIGHTING_PREVIEW ) + ( 48 * m_nWRITE_DEPTH_TO_DESTALPHA ) + 0;
	}
};

#define shaderDynamicTest_sdk_lightmappedgeneric_ps20b psh_forgot_to_set_dynamic_FASTPATHENVMAPCONTRAST + psh_forgot_to_set_dynamic_FASTPATH + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_LIGHTING_PREVIEW + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA


#endif	// SDK_LIGHTMAPPEDGENERIC_PS20B_H