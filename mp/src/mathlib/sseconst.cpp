//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "mathlib/ssemath.h"
#include "mathlib/ssequaternion.h"

const fltx4 Four_PointFives={0.5,0.5,0.5,0.5};
const fltx4 Four_Zeros={0.0,0.0,0.0,0.0};
const fltx4 Four_Ones={1.0,1.0,1.0,1.0};
const fltx4 Four_Twos={2.0,2.0,2.0,2.0};
const fltx4 Four_Threes={3.0,3.0,3.0,3.0};
const fltx4 Four_Fours={4.0,4.0,4.0,4.0};
const fltx4 Four_Origin={0,0,0,1};
const fltx4 Four_NegativeOnes={-1,-1,-1,-1};

const fltx4 Four_2ToThe21s={ (float) (1<<21), (float) (1<<21), (float) (1<<21), (float)(1<<21) };
const fltx4 Four_2ToThe22s={ (float) (1<<22), (float) (1<<22), (float) (1<<22), (float)(1<<22) };
const fltx4 Four_2ToThe23s={ (float) (1<<23), (float) (1<<23), (float) (1<<23), (float)(1<<23) };
const fltx4 Four_2ToThe24s={ (float) (1<<24), (float) (1<<24), (float) (1<<24), (float)(1<<24) };

const fltx4 Four_Point225s={ .225, .225, .225, .225 };
const fltx4 Four_Epsilons={FLT_EPSILON,FLT_EPSILON,FLT_EPSILON,FLT_EPSILON};

const fltx4 Four_FLT_MAX={FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};
const fltx4 Four_Negative_FLT_MAX={-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX};
const fltx4 g_SIMD_0123 = { 0., 1., 2., 3. };

const fltx4 g_QuatMultRowSign[4] =
{
	{  1.0f,  1.0f, -1.0f, 1.0f },
	{ -1.0f,  1.0f,  1.0f, 1.0f },
	{  1.0f, -1.0f,  1.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f }
};

const int32 ALIGN16 g_SIMD_clear_signmask[4] ALIGN16_POST = {0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff};
const int32 ALIGN16 g_SIMD_signmask[4] ALIGN16_POST = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const int32 ALIGN16 g_SIMD_lsbmask[4] ALIGN16_POST = { 0xfffffffe, 0xfffffffe, 0xfffffffe, 0xfffffffe };
const int32 ALIGN16 g_SIMD_clear_wmask[4] ALIGN16_POST = { 0xffffffff, 0xffffffff, 0xffffffff, 0 };
const int32 ALIGN16 g_SIMD_AllOnesMask[4] ALIGN16_POST = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }; // ~0,~0,~0,~0
const int32 ALIGN16 g_SIMD_Low16BitsMask[4] ALIGN16_POST = { 0xffff, 0xffff, 0xffff, 0xffff }; // 0xffff x 4

const int32 ALIGN16 g_SIMD_ComponentMask[4][4] ALIGN16_POST =
{
	{ 0xFFFFFFFF, 0, 0, 0 }, { 0, 0xFFFFFFFF, 0, 0 }, { 0, 0, 0xFFFFFFFF, 0 }, { 0, 0, 0, 0xFFFFFFFF }
};

const int32 ALIGN16 g_SIMD_SkipTailMask[4][4] ALIGN16_POST =
{
	{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
	{ 0xffffffff, 0x00000000, 0x00000000, 0x00000000 },
	{ 0xffffffff, 0xffffffff, 0x00000000, 0x00000000 },
	{ 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 },
};


	// FUNCTIONS
	// NOTE: WHY YOU **DO NOT** WANT TO PUT FUNCTIONS HERE
// Generally speaking, you want to make sure SIMD math functions
// are inlined, because that gives the compiler much more latitude
// in instruction scheduling. It's not that the overhead of calling
// the function is particularly great; rather, many of the SIMD 
// opcodes have long latencies, and if you have a sequence of 
// several dependent ones inside a function call, the latencies 
// stack up to create a big penalty. If the function is inlined,
// the compiler can interleave its operations with ones from the
// caller to better hide those latencies. Finally, on the 360,
// putting parameters or return values on the stack, and then 
// reading them back within the next forty cycles, is a very 
// severe penalty. So, as much as possible, you want to leave your
// data on the registers.

// That said, there are certain occasions where it is appropriate
// to call into functions -- particularly for very large blocks
// of code that will spill most of the registers anyway. Unless your
// function is more than one screen long, yours is probably not one
// of those occasions.



/// You can use this to rotate a long array of FourVectors all by the same
/// matrix. The first parameter is the head of the array. The second is the
/// number of vectors to rotate. The third is the matrix.
void FourVectors::RotateManyBy(FourVectors * RESTRICT pVectors, unsigned int numVectors, const matrix3x4_t& rotationMatrix )
{
	Assert(numVectors > 0);
	if ( numVectors == 0 )
		return;

	// Splat out each of the entries in the matrix to a fltx4. Do this
	// in the order that we will need them, to hide latency. I'm
	// avoiding making an array of them, so that they'll remain in 
	// registers.
	fltx4 matSplat00, matSplat01, matSplat02,
		matSplat10, matSplat11, matSplat12,
		matSplat20, matSplat21, matSplat22;

	{
		// Load the matrix into local vectors. Sadly, matrix3x4_ts are 
		// often unaligned. The w components will be the tranpose row of
		// the matrix, but we don't really care about that.
		fltx4 matCol0 = LoadUnalignedSIMD(rotationMatrix[0]);
		fltx4 matCol1 = LoadUnalignedSIMD(rotationMatrix[1]);
		fltx4 matCol2 = LoadUnalignedSIMD(rotationMatrix[2]);

		matSplat00 = SplatXSIMD(matCol0);
		matSplat01 = SplatYSIMD(matCol0);
		matSplat02 = SplatZSIMD(matCol0);

		matSplat10 = SplatXSIMD(matCol1);
		matSplat11 = SplatYSIMD(matCol1);
		matSplat12 = SplatZSIMD(matCol1);

		matSplat20 = SplatXSIMD(matCol2);
		matSplat21 = SplatYSIMD(matCol2);
		matSplat22 = SplatZSIMD(matCol2);
	}

	fltx4 outX0, outY0, outZ0; // bank one of outputs

	// Because of instruction latencies and scheduling, it's actually faster to use adds and muls
	// rather than madds. (Empirically determined by timing.)
	const FourVectors * stop = pVectors + numVectors;

	// perform an even number of iterations through this loop.
	while (pVectors < stop)
	{
		outX0 = MaddSIMD( pVectors->z, matSplat02, AddSIMD( MulSIMD( pVectors->x, matSplat00 ), MulSIMD( pVectors->y, matSplat01 ) ) );
		outY0 = MaddSIMD( pVectors->z, matSplat12, AddSIMD( MulSIMD( pVectors->x, matSplat10 ), MulSIMD( pVectors->y, matSplat11 ) ) );
		outZ0 = MaddSIMD( pVectors->z, matSplat22, AddSIMD( MulSIMD( pVectors->x, matSplat20 ), MulSIMD( pVectors->y, matSplat21 ) ) );

		pVectors->x = outX0;
		pVectors->y = outY0;
		pVectors->z = outZ0;
		pVectors++;
	}
}
