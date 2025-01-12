//------------------------------------------------------------------------------
// GB_warnings.h: turn off compiler warnings
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#if defined __INTEL_COMPILER

//  10397: remark about where *.optrpt reports are placed
//  15552: loop not vectorized
#pragma warning (disable: 10397 15552 )

// disable icc -w2 warnings
//  191:  type qualifier meangingless
//  193:  zero used for undefined #define
#pragma warning (disable: 191 193 )

// disable icc -w3 warnings
//  144:  initialize with incompatible pointer
//  181:  format
//  869:  unused parameters
//  1572: floating point compares
//  1599: shadow
//  2259: typecasting may lose bits
//  2282: unrecognized pragma
//  2557: sign compare
#pragma warning (disable: 144 181 869 1572 1599 2259 2282 2557 )

// See GB_unused.h, for warnings 177 and 593, which are not globally
// disabled, but selectively by #include'ing GB_unused.h as needed.

#elif defined __GNUC__

// disable warnings for gcc 5.x and higher:
#if (__GNUC__ > 4)
// disable warnings
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
#pragma GCC diagnostic ignored "-Wformat-truncation="
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
// enable these warnings as errors
// #pragma GCC diagnostic error "-Wmisleading-indentation"
#endif

// disable warnings from -Wall -Wextra -Wpendantic
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#if defined ( __cplusplus )
#pragma GCC diagnostic ignored "-Wwrite-strings"
#else
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

// enable these warnings as errors
#pragma GCC diagnostic error "-Wswitch-default"
#if !defined ( __cplusplus )
#pragma GCC diagnostic error "-Wmissing-prototypes"
#endif

#endif

// disable warnings for clang
#ifdef __clang__
#pragma clang diagnostic ignored "-Wpointer-sign"
#endif

#if ( _MSC_VER && !__INTEL_COMPILER )
// disable MS Visual Studio warnings
#pragma warning(disable:4146)
#endif

