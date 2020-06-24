#ifndef CLEARALPHA_PS20B_H
#define CLEARALPHA_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class clearalpha_ps20b_Static_Index
{
public:
	clearalpha_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_clearalpha_ps20b 1


class clearalpha_ps20b_Dynamic_Index
{
public:
	clearalpha_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_clearalpha_ps20b 1


#endif	// CLEARALPHA_PS20B_H