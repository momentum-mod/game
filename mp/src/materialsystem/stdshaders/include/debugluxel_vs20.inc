// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEBUGLUXEL_VS20_H
#define DEBUGLUXEL_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class debugluxel_vs20_Static_Index
{
public:
	debugluxel_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_debugluxel_vs20 1


class debugluxel_vs20_Dynamic_Index
{
public:
	debugluxel_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_debugluxel_vs20 1


#endif	// DEBUGLUXEL_VS20_H