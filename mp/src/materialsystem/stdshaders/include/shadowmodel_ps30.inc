#ifndef SHADOWMODEL_PS30_H
#define SHADOWMODEL_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class shadowmodel_ps30_Static_Index
{
public:
	shadowmodel_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_shadowmodel_ps30 1


class shadowmodel_ps30_Dynamic_Index
{
public:
	shadowmodel_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_shadowmodel_ps30 1


#endif	// SHADOWMODEL_PS30_H