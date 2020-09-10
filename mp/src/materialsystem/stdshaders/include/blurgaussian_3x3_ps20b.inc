#ifndef BLURGAUSSIAN_3X3_PS20B_H
#define BLURGAUSSIAN_3X3_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class blurgaussian_3x3_ps20b_Static_Index
{
public:
	blurgaussian_3x3_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_blurgaussian_3x3_ps20b 1


class blurgaussian_3x3_ps20b_Dynamic_Index
{
public:
	blurgaussian_3x3_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_blurgaussian_3x3_ps20b 1


#endif	// BLURGAUSSIAN_3X3_PS20B_H