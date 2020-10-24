#ifndef WHITE_PS20B_H
#define WHITE_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class white_ps20b_Static_Index
{
public:
	white_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_white_ps20b 1


class white_ps20b_Dynamic_Index
{
public:
	white_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_white_ps20b 1


#endif	// WHITE_PS20B_H