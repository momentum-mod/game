//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
// An interface that should not ever be accessed directly from shaders
// but instead is visible only to shaderlib.
//===========================================================================//

#ifndef ISHADERSYSTEM_H
#define ISHADERSYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include <materialsystem/IShader.h>

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
enum Sampler_t;
class ITexture;
class IShader;


//-----------------------------------------------------------------------------
// The Shader system interface version
//-----------------------------------------------------------------------------
#define SHADERSYSTEM_INTERFACE_VERSION		"ShaderSystem002"


//-----------------------------------------------------------------------------
// Modulation flags
//-----------------------------------------------------------------------------
enum
{
	SHADER_USING_ALPHA_MODULATION				= 0x1,
	SHADER_USING_FLASHLIGHT						= 0x2,
	SHADER_USING_FIXED_FUNCTION_BAKED_LIGHTING	= 0x4,
	SHADER_USING_EDITOR							= 0x8,

	// the BUFFER0 and GBUFFER1 bits provide 3 g-buffermodes plus the normal modes.
	// the modes are:
	// Normal rendering = ( gbuffer1 = 0, gbuffer0 = 0 )
	// Output pos, normal, albedo via mrts = (0,1)
	// output fixed lighted single image = (1,0)
	// output the normal = (1,1)
	SHADER_USING_GBUFFER0                       = 0x10,
	SHADER_USING_GBUFFER1                       = 0x20,
};


//-----------------------------------------------------------------------------
// The shader system (a singleton)
//-----------------------------------------------------------------------------
abstract_class IShaderSystem
{
public:
	virtual ShaderAPITextureHandle_t GetShaderAPITextureBindHandle( ITexture *pTexture, int nFrameVar, int nTextureChannel = 0 ) =0;

	// Binds a texture
	virtual void BindTexture( Sampler_t sampler1, ITexture *pTexture, int nFrameVar = 0 ) = 0;
	virtual void BindTexture( Sampler_t sampler1, Sampler_t sampler2, ITexture *pTexture, int nFrameVar = 0 ) = 0;

	// Takes a snapshot
	virtual void TakeSnapshot( ) = 0;

	// Draws a snapshot
	virtual void DrawSnapshot( const unsigned char *pInstanceCommandBuffer, bool bMakeActualDrawCall = true ) = 0;

	// Are we using graphics?
	virtual bool IsUsingGraphics() const = 0;

	// Are editor materials enabled?
	virtual bool CanUseEditorMaterials() const = 0;

	// Bind vertex texture
	virtual void BindVertexTexture( VertexTextureSampler_t vtSampler, ITexture *pTexture, int nFrameVar = 0 ) = 0;
};


//-----------------------------------------------------------------------------
// The Shader plug-in DLL interface version
//-----------------------------------------------------------------------------
#define SHADER_DLL_INTERFACE_VERSION	"ShaderDLL004"


//-----------------------------------------------------------------------------
// The Shader interface versions
//-----------------------------------------------------------------------------
abstract_class IShaderDLLInternal
{
public:
	// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory, bool bIsMaterialSystem ) = 0;
	virtual void Disconnect( bool bIsMaterialSystem ) = 0;

	// Returns the number of shaders defined in this DLL
	virtual int ShaderCount() const = 0;

	// Returns information about each shader defined in this DLL
	virtual IShader *GetShader( int nShader ) = 0;
};


//-----------------------------------------------------------------------------
// Singleton interface
//-----------------------------------------------------------------------------
IShaderDLLInternal *GetShaderDLLInternal();


#endif // ISHADERSYSTEM_H
