// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEBUGMORPHACCUMULATOR_VS30_H
#define DEBUGMORPHACCUMULATOR_VS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class debugmorphaccumulator_vs30_Static_Index
{
public:
	debugmorphaccumulator_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_debugmorphaccumulator_vs30 1


class debugmorphaccumulator_vs30_Dynamic_Index
{
public:
	debugmorphaccumulator_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_debugmorphaccumulator_vs30 1


#endif	// DEBUGMORPHACCUMULATOR_VS30_H