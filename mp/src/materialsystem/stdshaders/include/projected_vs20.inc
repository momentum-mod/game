// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $DOWATERFOG == 0 ) && ( $HARDWAREFOGBLEND == 0 ) && ( $DOPIXELFOG == 0 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef PROJECTED_VS20_H
#define PROJECTED_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class projected_vs20_Static_Index
{
	unsigned int m_nMODEL : 2;
	unsigned int m_nFOW : 2;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
#ifdef _DEBUG
	bool m_bMODEL : 1;
	bool m_bFOW : 1;
#endif	// _DEBUG
public:
	void SetMODEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMODEL = i;
#ifdef _DEBUG
		m_bMODEL = true;
#endif	// _DEBUG
	}

	void SetFOW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFOW = i;
#ifdef _DEBUG
		m_bFOW = true;
#endif	// _DEBUG
	}

	void SetDOPIXELFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOPIXELFOG = i;
	}

	void SetHARDWAREFOGBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHARDWAREFOGBLEND = i;
	}

	projected_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nMODEL = 0;
		m_nFOW = 0;
		m_nDOPIXELFOG = g_pHardwareConfig->SupportsPixelShaders_2_b() ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
#ifdef _DEBUG
		m_bMODEL = false;
		m_bFOW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bMODEL && m_bFOW );
		return ( 8 * m_nMODEL ) + ( 16 * m_nFOW ) + ( 32 * m_nDOPIXELFOG ) + ( 64 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_projected_vs20 vsh_forgot_to_set_static_MODEL + vsh_forgot_to_set_static_FOW


class projected_vs20_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nDOWATERFOG : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
#endif	// _DEBUG
public:
	void SetCOMPRESSED_VERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMPRESSED_VERTS = i;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif	// _DEBUG
	}

	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
	}

	projected_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_projected_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING


#endif	// PROJECTED_VS20_H