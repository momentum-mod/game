#ifndef SHADOWMODEL_PS20B_H
#define SHADOWMODEL_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class shadowmodel_ps20b_Static_Index
{
public:
	shadowmodel_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_shadowmodel_ps20b 1


class shadowmodel_ps20b_Dynamic_Index
{
public:
	shadowmodel_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_shadowmodel_ps20b 1


#endif	// SHADOWMODEL_PS20B_H