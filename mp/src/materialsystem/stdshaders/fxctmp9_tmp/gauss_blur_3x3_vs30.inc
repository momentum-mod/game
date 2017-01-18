#include "shaderlib/cshader.h"
class gauss_blur_3x3_vs30_Static_Index
{
public:
	gauss_blur_3x3_vs30_Static_Index( )
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderStaticTest_gauss_blur_3x3_vs30 0
class gauss_blur_3x3_vs30_Dynamic_Index
{
public:
	gauss_blur_3x3_vs30_Dynamic_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderDynamicTest_gauss_blur_3x3_vs30 0
