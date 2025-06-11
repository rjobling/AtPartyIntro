////////////////////////////////////////////////////////////////////////////////
// core.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <proto/exec.h>
#include "support/gcc8_c_support.h"

////////////////////////////////////////////////////////////////////////////////
// Basic types.
////////////////////////////////////////////////////////////////////////////////
typedef char s8;
typedef short s16;
typedef int s32;
typedef unsigned int uint;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef u32 size_t;

////////////////////////////////////////////////////////////////////////////////
// Compiler hints.
////////////////////////////////////////////////////////////////////////////////
#define unused(a) ((void) a)
#define barrier() asm("" ::: "memory")

////////////////////////////////////////////////////////////////////////////////
// Asserts.
////////////////////////////////////////////////////////////////////////////////
#if defined(DEBUG)
#define assert(b) { if (!(b)) { KPrintF("Assert: %s %s:%ld", __FUNCTION__, __FILE__, __LINE__); asm volatile("illegal"); } }
#define assert_message(b, ...) { if (!(b)) { KPrintF("Assert: %s %s:%ld", __FUNCTION__, __FILE__, __LINE__); KPrintF(__VA_ARGS__); asm volatile("illegal"); } }
#define assert_pointer(p) { if ((p) == nullptr) { KPrintF("Assert: %s %s:%ld", __FUNCTION__, __FILE__, __LINE__); asm volatile("illegal"); } }
#define assert_pointer_message(p, ...) { if ((p) == nullptr) { KPrintF("Assert: %s %s:%ld", __FUNCTION__, __FILE__, __LINE__); KPrintF(__VA_ARGS__); asm volatile("illegal"); } }
#define error() { KPrintF("Error: %s %s:%ld", __FUNCTION__, __FILE__, __LINE__); asm volatile("illegal"); }
#define error_message(...) { KPrintF("Error: %s %s:%ld", __FUNCTION__, __FILE__, __LINE__); KPrintF(__VA_ARGS__); asm volatile("illegal"); }
#else
#define assert(b)
#define assert_message(b, ...)
#define assert_pointer(p)
#define assert_pointer_message(p, ...)
#define error()
#define error_message(...)
#endif

////////////////////////////////////////////////////////////////////////////////
// Alignment.
////////////////////////////////////////////////////////////////////////////////
template<typename T> constexpr bool isaligned(T a, T b) { return ((a & (b - 1)) == 0); }
template<typename T> constexpr T alignup(T a, T b) { return ((a + b - 1) & ~(b - 1)); }
template<typename T> constexpr T aligndown(T a, T b) { return (a & ~(b - 1)); }
template<typename T> constexpr T alignnext(T a) { a--; a |= a >> 1; a |= a >> 2; a |= a >> 4; a |= a >> 8; a |= a >> 16; a++; return a; }

////////////////////////////////////////////////////////////////////////////////
// Arrays.
////////////////////////////////////////////////////////////////////////////////
#define countof(a) ((int) (sizeof(a) / sizeof((a)[0])))

////////////////////////////////////////////////////////////////////////////////
// Math.
////////////////////////////////////////////////////////////////////////////////
template<typename T> constexpr T sqr(T a) { return (a * a); }
template<typename T> constexpr T cube(T a) { return (a * a * a); }
template<typename T> constexpr T log2(T a) { return __builtin_ctz(a); }
template<typename T> constexpr T is_log2(T a) { return ((a != 0) && ((a & (a - 1)) == 0)); }
template<typename T> constexpr T abs(T a) { return ((a < 0) ? -a : a); }
template<typename T> constexpr T sign(T a) { return ((a < 0) ? ((T) -1) : ((T) 1)); }
template<typename T> constexpr T min(T a, T b) { return ((a < b) ? a : b); }
template<typename T> constexpr T max(T a, T b) { return ((a > b) ? a : b); }
template<typename T> constexpr T clamp(T a, T amin, T amax) { T t = ((a > amin) ? a : amin); t = ((t < amax) ? t : amax); return t; }
template<typename T> inline void swap(T& a, T& b) { T t = a; a = b; b = t; }
template<typename T, typename TBlend> inline T lerp(T a, T b, TBlend blend) { return (a + (b - a) * blend); }
s16 sin(u16 x);
s16 cos(u16 x);
s32 tan(u16 x);

////////////////////////////////////////////////////////////////////////////////
// Rand.
////////////////////////////////////////////////////////////////////////////////
u16 rand(int& value);
