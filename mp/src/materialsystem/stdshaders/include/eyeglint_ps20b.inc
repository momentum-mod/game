#ifndef EYEGLINT_PS20B_H
#define EYEGLINT_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class eyeglint_ps20b_Static_Index
{
public:
	eyeglint_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_eyeglint_ps20b 1


class eyeglint_ps20b_Dynamic_Index
{
public:
	eyeglint_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_eyeglint_ps20b 1


#endif	// EYEGLINT_PS20B_H