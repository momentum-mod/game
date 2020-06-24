// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEBUGMRTTEXTURE_VS20_H
#define DEBUGMRTTEXTURE_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class debugmrttexture_vs20_Static_Index
{
public:
	debugmrttexture_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_debugmrttexture_vs20 1


class debugmrttexture_vs20_Dynamic_Index
{
public:
	debugmrttexture_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_debugmrttexture_vs20 1


#endif	// DEBUGMRTTEXTURE_VS20_H