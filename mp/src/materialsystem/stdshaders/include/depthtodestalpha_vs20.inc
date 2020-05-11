// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEPTHTODESTALPHA_VS20_H
#define DEPTHTODESTALPHA_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class depthtodestalpha_vs20_Static_Index
{
public:
	depthtodestalpha_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_depthtodestalpha_vs20 1


class depthtodestalpha_vs20_Dynamic_Index
{
public:
	depthtodestalpha_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_depthtodestalpha_vs20 1


#endif	// DEPTHTODESTALPHA_VS20_H