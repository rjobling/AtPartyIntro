////////////////////////////////////////////////////////////////////////////////
// customhelpers.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "core.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct CopCommand
{
	u16 inst;
	u16 data;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
constexpr CopCommand CopInst(u16 inst, u16 data) { return {inst, data}; }
#define CopMove(reg, value) CopInst((u16) offsetof(Custom, reg), (u16) (value))
#define CopMoveH(reg, value) CopInst((u16) offsetof(Custom, reg), (u16) (((u32) (value)) >> 16))
#define CopMoveL(reg, value) CopInst((u16) offsetof(Custom, reg) + 2, (u16) (((u32) (value)) & 0xffff))
constexpr CopCommand CopNull() { return {0x1fe, 0}; }
constexpr CopCommand CopWait(int hp, int vp, int he = 0x7f, int ve = 0x7f, bool bfd = true) { return {(u16) ((vp << 8) | (hp << 1) | 0x1), (u16) ((bfd ? 0x8000 : 0) | (ve << 8) | (he << 1))}; }
constexpr CopCommand CopSkip(int hp, int vp, int he = 0x7f, int ve = 0x7f, bool bfd = true) { return {(u16) ((vp << 8) | (hp << 1) | 0x1), (u16) ((bfd ? 0x8000 : 0) | (ve << 8) | (he << 1) | 1)}; }
constexpr CopCommand CopEnd() { return {0xffff, 0xfffe}; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
constexpr u16 PackBplcon0(int bpls, bool dpf = false, bool ham = false, bool hires = false) { return ((hires ? 0x8000 : 0) | (bpls << 12) | (ham ? 0x800 : 0) | (dpf ? 0x400 : 0) | 0x200); }
constexpr u16 PackBplcon1(int x1, int x2, int x1aga = 0, int x2aga = 0) { return ((x2aga << 12) | (x1aga << 8) | (x2 << 4) | x1); }
constexpr u16 PackBplcon2(bool pf2pri, int pf2p, int pf1p) { return ((pf2pri ? 0x40 : 0) | (pf2p << 3) | pf1p); }
constexpr u16 PackDiwstrt(int sx, int sy) { return (((sy + 0x2c) << 8) | (sx + 0x81)); }
constexpr u16 PackDiwstop(int sx, int sy) { return ((((sy - 256) + 0x2c) << 8) | ((sx - 256) + 0x81)); }
constexpr u16 PackDdfstrt(int sx, bool hires = false) { return (((sx + 0x81 - (hires ? 9 : 17)) >> 1) & 0xfc); }
constexpr u16 PackDdfstop(int sx, bool hires = false, u16 fmode = 0x0000) { return (PackDdfstrt(0, hires) + (hires ? (((sx >> 4) - 2 - ((fmode & 3) << 1)) << 2) : (((sx >> 4) - 1 - (fmode & 3)) << 3))); }
constexpr u16 PackSprpos(int sh, int sv) { return (((sv + 0x2c) << 8) | ((sh + 0x7c) >> 1)); }
constexpr u16 PackSprctl(int sh, int sv, int ev, bool att) { return (((ev + 0x2c) << 8) | (att ? 0x0080 : 0) | ((sv + 0x2c) >> 8) | ((ev + 0x2c) >> 8) | ((sh + 0x7c) & 1)); }
