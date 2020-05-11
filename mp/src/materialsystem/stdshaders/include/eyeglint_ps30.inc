#ifndef EYEGLINT_PS30_H
#define EYEGLINT_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class eyeglint_ps30_Static_Index
{
public:
	eyeglint_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_eyeglint_ps30 1


class eyeglint_ps30_Dynamic_Index
{
public:
	eyeglint_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_eyeglint_ps30 1


#endif	// EYEGLINT_PS30_H