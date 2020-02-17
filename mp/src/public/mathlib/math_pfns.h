//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef _MATH_PFNS_H_
#define _MATH_PFNS_H_

// These globals are initialized by mathlib and redirected based on available fpu features
extern float (*pfSqrt)(float x);
extern float (*pfRSqrt)(float x);
extern float (*pfRSqrtFast)(float x);
extern void  (*pfFastSinCos)(float x, float *s, float *c);
extern float (*pfFastCos)(float x);

// The following are not declared as macros because they are often used in limiting situations,
// and sometimes the compiler simply refuses to inline them for some reason
#define FastSqrt(x)			(*pfSqrt)(x)
#define	FastRSqrt(x)		(*pfRSqrt)(x)
#define FastRSqrtFast(x)    (*pfRSqrtFast)(x)
#define FastSinCos(x,s,c)   (*pfFastSinCos)(x,s,c)
#define FastCos(x)			(*pfFastCos)(x)

#if defined(__i386__) || defined(_M_IX86)
// On x86, the inline FPU or SSE sqrt instruction is faster than
// the overhead of setting up a function call and saving/restoring
// the FPU or SSE register state and can be scheduled better, too.
#undef FastSqrt
#define FastSqrt(x)			::sqrtf(x)
#endif

#endif // _MATH_PFNS_H_
