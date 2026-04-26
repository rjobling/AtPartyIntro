////////////////////////////////////////////////////////////////////////////////
// intro.cpp
////////////////////////////////////////////////////////////////////////////////

#include "intro.h"
#include <hardware/blit.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include "framework/core.h"
#include "framework/customhelpers.h"
#include "framework/font.h"
#include "framework/lsp.h"
#include "framework/palette.h"
#include "framework/system.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kViewWidth	 = 320;
static constexpr int kViewHeight = 256;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kLogoWidth		= 320;
static constexpr int kLogoHeight	= 48;
static constexpr int kLogoPlanes	= 4;
static constexpr int kLogoPlaneSize = (kLogoWidth / 8) * kLogoHeight;
static constexpr int kLogoPitch		= kLogoWidth / 8;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kImageWidth	 = 320;
static constexpr int kImageHeight	 = 320;
static constexpr int kImagePlanes	 = 2;
static constexpr int kImagePlaneSize = (kImageWidth / 8) * kImageHeight;
static constexpr int kImagePitch	 = kImageWidth / 8;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kPaletteSize  = 1 << kImagePlanes;
static constexpr int kPaletteCount = 128;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kBobsWidth		= 64;
static constexpr int kBobsHeight	= 64;
static constexpr int kBobsPlanes	= 2;
static constexpr int kBobsPlaneSize	= (kBobsWidth / 8) * kBobsHeight;
static constexpr int kBobsPitch		= kBobsWidth / 8;
static constexpr int kBobSize		= 16;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kBufferWidth	  = 320;
static constexpr int kBufferHeight	  = 176;
static constexpr int kBufferPlanes	  = 2;
static constexpr int kBufferPlaneSize = (kBufferWidth / 8) * kBufferHeight;
static constexpr int kBufferPitch	  = kBufferWidth / 8;
static constexpr int kBufferSize	  = kBufferPlaneSize * kBufferPlanes;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN(gFontBpls, "data/font_bpls.bin");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN_CHIP(gLogoBpls, "data/logo_bpls.bin");
INCBIN(gLogoPal, "data/logo_pal.bin");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN_CHIP(gImageBpls, "data/image_bpls.bin");
INCBIN(gPalettes, "data/palettes.bin");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN_CHIP(gBobsBpls, "data/bobs_bpls.bin");
INCBIN(gBobsPal, "data/bobs_pal.bin");
INCBIN_CHIP(gMasksBpls, "data/masks_bpls.bin");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static u16 sBufferBpls[2][kBufferSize / 2] __attribute__((section(".MEMF_CHIP")));

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN(gLSPMusic, "data/cosmic_makeup.lsmusic");
INCBIN_CHIP(gLSPBank, "data/cosmic_makeup.lsbank");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct CopList
{
	struct
	{
		CopCommand bplcon0 = CopMove(bplcon0, PackBplcon0(kLogoPlanes));
		CopCommand bpl1mod = CopMove(bpl1mod, 0);
		CopCommand bpl2mod = CopMove(bpl2mod, 0);

		CopCommand bplpth[kLogoPlanes] = {
			CopMoveH(bplpt[0], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 0),
			CopMoveH(bplpt[1], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 1),
			CopMoveH(bplpt[2], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 2),
			CopMoveH(bplpt[3], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 3),
		};
		CopCommand bplptl[kLogoPlanes] = {
			CopMoveL(bplpt[0], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 0),
			CopMoveL(bplpt[1], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 1),
			CopMoveL(bplpt[2], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 2),
			CopMoveL(bplpt[3], (const u16*) gLogoBpls + kLogoPlaneSize / 2 * 3),
		};

		CopCommand colors[1 << kLogoPlanes] = {
			CopMove(color[ 0], ((const u16*) gLogoPal)[ 0]),
			CopMove(color[ 1], ((const u16*) gLogoPal)[ 1]),
			CopMove(color[ 2], ((const u16*) gLogoPal)[ 2]),
			CopMove(color[ 3], ((const u16*) gLogoPal)[ 3]),
			CopMove(color[ 4], ((const u16*) gLogoPal)[ 4]),
			CopMove(color[ 5], ((const u16*) gLogoPal)[ 5]),
			CopMove(color[ 6], ((const u16*) gLogoPal)[ 6]),
			CopMove(color[ 7], ((const u16*) gLogoPal)[ 7]),
			CopMove(color[ 8], ((const u16*) gLogoPal)[ 8]),
			CopMove(color[ 9], ((const u16*) gLogoPal)[ 9]),
			CopMove(color[10], ((const u16*) gLogoPal)[10]),
			CopMove(color[11], ((const u16*) gLogoPal)[11]),
			CopMove(color[12], ((const u16*) gLogoPal)[12]),
			CopMove(color[13], ((const u16*) gLogoPal)[13]),
			CopMove(color[14], ((const u16*) gLogoPal)[14]),
			CopMove(color[15], ((const u16*) gLogoPal)[15]),
		};
	}
	logo;

	struct
	{
		CopCommand topwait = CopWait(0, 44 + kLogoHeight);

		CopCommand bplcon0 = CopMove(bplcon0, PackBplcon0(kImagePlanes + kBufferPlanes));
		CopCommand bpl1mod = CopMove(bpl1mod, 0);
		CopCommand bpl2mod = CopMove(bpl2mod, 0);

		CopCommand bplpth[kImagePlanes + kBufferPlanes] = {
			CopMoveH(bplpt[0], (const u16*) gImageBpls + kImagePlaneSize / 2 * 0),
			CopMoveH(bplpt[1], (const u16*) gImageBpls + kImagePlaneSize / 2 * 1),
			CopMoveH(bplpt[2], sBufferBpls[0] + kBufferPlaneSize / 2 * 0),
			CopMoveH(bplpt[3], sBufferBpls[0] + kBufferPlaneSize / 2 * 1),
		};
		CopCommand bplptl[kImagePlanes + kBufferPlanes] = {
			CopMoveL(bplpt[0], (const u16*) gImageBpls + kImagePlaneSize / 2 * 0),
			CopMoveL(bplpt[1], (const u16*) gImageBpls + kImagePlaneSize / 2 * 1),
			CopMoveL(bplpt[2], sBufferBpls[0] + kBufferPlaneSize / 2 * 0),
			CopMoveL(bplpt[3], sBufferBpls[0] + kBufferPlaneSize / 2 * 1),
		};

		CopCommand topcolors[1 << (kImagePlanes + kBufferPlanes)] = {
			CopMove(color[ 0], 0x000),
			CopMove(color[ 1], 0x000),
			CopMove(color[ 2], 0x000),
			CopMove(color[ 3], 0x000),
			CopMove(color[ 4], 0x000),
			CopMove(color[ 5], 0x000),
			CopMove(color[ 6], 0x000),
			CopMove(color[ 7], 0x000),
			CopMove(color[ 8], 0x000),
			CopMove(color[ 9], 0x000),
			CopMove(color[10], 0x000),
			CopMove(color[11], 0x000),
			CopMove(color[12], 0x000),
			CopMove(color[13], 0x000),
			CopMove(color[14], 0x000),
			CopMove(color[15], 0x000),
		};

		CopCommand midwait1 = CopWait(4, 255);
		CopCommand midwait2 = CopWait(4, 44 + 224);

		CopCommand midbpl1mod = CopMove(bpl1mod, -kImagePitch * 3);
		CopCommand midbpl2mod = CopMove(bpl2mod, -kImagePitch * 3);

		CopCommand midcolors[1 << kImagePlanes] = {
			CopMove(color[0], 0x000),
			CopMove(color[1], 0x000),
			CopMove(color[2], 0x000),
			CopMove(color[3], 0x000),
		};
	}
	image;

	CopCommand end = CopEnd();
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static CopList sCopList __attribute__((section(".MEMF_CHIP")));

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static u16* sFrontBpls = sBufferBpls[0];
static u16* sBackBpls  = sBufferBpls[1];

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static int sFrame = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool Intro_Init()
{
	Font_Init(sBackBpls, sBackBpls + kBufferPlaneSize / 2, kBufferPitch, (const u16*) gFontBpls);

	Palette_InitBlendTable();

	custom.bplcon0 = PackBplcon0(kImagePlanes);
	custom.bplcon1 = PackBplcon1(0, 0);
	custom.bplcon2 = PackBplcon2(false, 4, 4);
	custom.diwstrt = PackDiwstrt(0, 0);
	custom.diwstop = PackDiwstop(kViewWidth, kViewHeight);
	custom.ddfstrt = PackDdfstrt(0);
	custom.ddfstop = PackDdfstop(kViewWidth);

	custom.bpl1mod = 0;
	custom.bpl2mod = 0;

	debug_register_bitmap(gFontBpls, "FontBpls", Font_kBplWidth, Font_kBplHeight, Font_kBplPlanes, debug_resource_bitmap_interleaved);
	debug_register_bitmap(gLogoBpls, "LogoBpls", kLogoWidth, kLogoHeight, kLogoPlanes, 0);
	debug_register_palette(gLogoPal, "LogoPal", 1 << kLogoPlanes, 0);
	debug_register_bitmap(gImageBpls, "ImageBpls", kImageWidth, kImageHeight, kImagePlanes, 0);
	debug_register_palette(gPalettes, "Palette", kPaletteSize, 0);
	debug_register_bitmap(gBobsBpls, "BobsBpls", kBobsWidth, kBobsHeight, kBobsPlanes, 0);
	debug_register_palette(gBobsPal, "BobsPal", 1 << kBobsPlanes, 0);
	debug_register_bitmap(gMasksBpls, "MasksBpls", kBobsWidth, kBobsHeight, 1, 0);
	debug_register_bitmap(sBufferBpls[0], "BufferBpls0", kBufferWidth, kBufferHeight, kBufferPlanes, 0);
	debug_register_bitmap(sBufferBpls[1], "BufferBpls1", kBufferWidth, kBufferHeight, kBufferPlanes, 0);

	LSP_MusicDriver_CIA_Start(gLSPMusic, gLSPBank);
	System_SetAudioFilter(false);

	// Wait for the top of the frame and prepare to start.
	System_WaitVbl();

	// Prime the copper prior to starting.
	custom.cop1lc = (int) &sCopList;

	// Wait for the top of the frame before starting.
	System_WaitVbl();

	// Enable DMA which will start the copper.
	custom.dmacon = DMAF_SETCLR | DMAF_MASTER | DMAF_RASTER | DMAF_COPPER | DMAF_BLITTER;

	// Enable interrupts when still nearly at the top of the frame.
	custom.intena = INTF_SETCLR | INTF_INTEN;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Intro_Deinit()
{
	System_WaitVbl();
	System_WaitBlt();

	// Disable DMA which will stop the copper.
	custom.dmacon = DMAF_RASTER | DMAF_COPPER | DMAF_BLITTER;

	LSP_MusicDriver_CIA_Stop();

	debug_unregister(gFontBpls);
	debug_unregister(gLogoBpls);
	debug_unregister(gLogoPal);
	debug_unregister(gImageBpls);
	debug_unregister(gPalettes);
	debug_unregister(gBobsBpls);
	debug_unregister(gBobsPal);
	debug_unregister(gMasksBpls);
	debug_unregister(sBufferBpls[0]);
	debug_unregister(sBufferBpls[1]);

	Palette_DeinitBlendTable();
	Font_Deinit();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void BlitClear()
{
	custom.bltcon0 = DEST | 0x00;
	custom.bltcon1 = 0;
	custom.bltdpt  = sBackBpls;
	custom.bltdmod = 0;
	custom.bltsize = (kBufferHeight << 6) + (kBufferWidth / 16);

	System_WaitBlt();

	custom.bltdpt  = sBackBpls + kBufferPlaneSize / 2;
	custom.bltsize = (kBufferHeight << 6) + (kBufferWidth / 16);

	System_WaitBlt();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void BlitBob(s16 x, s16 y)
{
	s16 minor = x & 15;
	s16 major = x >> 4;

	u16* bpls = sBackBpls + major / 2 + y * kBufferPitch / 2;

	if (minor != 0)
	{
		s16 shift = minor << ASHIFTSHIFT;

		custom.bltcon0 = shift | DEST | SRCA | SRCB | SRCC | 0xe2;
		custom.bltcon1 = shift;
		custom.bltafwm = 0xffff;
		custom.bltalwm = 0x0000;
		custom.bltapt  = (u16*) gBobsBpls;
		custom.bltbpt  = (u16*) gMasksBpls;
		custom.bltcpt  = bpls;
		custom.bltdpt  = bpls;
		custom.bltamod = (kBobsWidth - kBobSize) / 8 - 2;
		custom.bltbmod = (kBobsWidth - kBobSize) / 8 - 2;
		custom.bltcmod = (kBufferWidth - kBobSize) / 8 - 2;
		custom.bltdmod = (kBufferWidth - kBobSize) / 8 - 2;
		custom.bltsize = (kBobSize << 6) + (kBobSize / 16) + 1;

		System_WaitBlt();

		bpls += kBufferPlaneSize / 2;

		custom.bltapt  = (u16*) gBobsBpls + kBobsPlaneSize / 2;
		custom.bltbpt  = (u16*) gMasksBpls;
		custom.bltcpt  = bpls;
		custom.bltdpt  = bpls;
		custom.bltsize = (kBobSize << 6) + (kBobSize / 16) + 1;
	}
	else
	{
		custom.bltcon0 = DEST | SRCA | SRCB | SRCC | 0xe2;
		custom.bltcon1 = 0;
		custom.bltafwm = 0xffff;
		custom.bltafwm = 0xffff;
		custom.bltapt  = (u16*) gBobsBpls;
		custom.bltbpt  = (u16*) gMasksBpls;
		custom.bltcpt  = bpls;
		custom.bltdpt  = bpls;
		custom.bltamod = (kBobsWidth - kBobSize) / 8;
		custom.bltbmod = (kBobsWidth - kBobSize) / 8;
		custom.bltcmod = (kBufferWidth - kBobSize) / 8;
		custom.bltdmod = (kBufferWidth - kBobSize) / 8;
		custom.bltsize = (kBobSize << 6) + (kBobSize / 16);

		System_WaitBlt();

		bpls += kBufferPlaneSize / 2;

		custom.bltapt  = (u16*) gBobsBpls + kBobsPlaneSize / 2;
		custom.bltbpt  = (u16*) gMasksBpls;
		custom.bltcpt  = bpls;
		custom.bltdpt  = bpls;
		custom.bltsize = (kBobSize << 6) + (kBobSize / 16);
	}

	System_WaitBlt();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void BlitLine(s16 x1, s16 y1, s16 x2, s16 y2, s16 color)
{
	assert(color > 0);

	if (y1 > y2)
	{
		swap(x1, x2);
		swap(y1, y2);
	}

	s16 dx = x2 - x1;
	s16 dy = y2 - y1;

	u16 code;

	if (dx < 0)
	{
		dx = -dx;

		if (dy > dx)
		{
			swap(dx, dy);
			code = (2 << 2) | LINEMODE;
		}
		else
		{
			code = (5 << 2) | LINEMODE;
		}
	}
	else
	{
		if (dy > dx)
		{
			swap(dx, dy);
			code = (0 << 2) | LINEMODE;
		}
		else
		{
			code = (4 << 2) | LINEMODE;
		}
	}

	s16 dx2 = dx + dx;
	s16 dx4 = dx2 + dx2;
	s16 dy4 = dy << 2;
	s16 dy4_minus_dx4 = dy4 - dx4;
	s32 dy4_minus_dx2 = dy4 - dx2;

	u16 con0 = ((x1 & 15) << ASHIFTSHIFT) | SRCA | SRCC | DEST | 0x48;
	u16 con1 = ((dy4_minus_dx2 < 0) ? SIGNFLAG : 0) | code;

	u16* bpl = ((u16*) sBackBpls) + ((y1 << 5) + (y1 << 3) + (x1 >> 3)) / 2;

	u16 size = ((dx + 1) << HSIZEBITS) | 2;

	custom.bltadat = 0x8000;
	custom.bltbdat = 0xffff;
	custom.bltafwm = 0xffff;
	custom.bltalwm = 0xffff;
	custom.bltcmod = kBufferPitch;
	custom.bltamod = dy4_minus_dx4;
	custom.bltbmod = dy4;

	if (color & 1)
	{
		custom.bltapt  = (void*) dy4_minus_dx2;
		custom.bltcon0 = con0;
		custom.bltcon1 = con1;
		custom.bltcpt  = bpl;
		custom.bltdpt  = bpl;
		custom.bltsize = size;

		System_WaitBlt();
	}

	if (color & 2)
	{
		bpl += kBufferPlaneSize / 2;

		custom.bltapt  = (void*) dy4_minus_dx2;
		custom.bltcon0 = con0;
		custom.bltcon1 = con1;
		custom.bltcpt  = bpl;
		custom.bltdpt  = bpl;
		custom.bltsize = size;

		System_WaitBlt();
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool Intro_Update()
{
	System_WaitVbl();

	int scroll = (16384 - cos(sFrame << 9)) >> 8;

	const u16* imageBpl0 = (const u16*) gImageBpls + scroll * kImagePitch / 2;
	const u16* imageBpl1 = imageBpl0 + kImagePlaneSize / 2;

	sCopList.image.bplpth[0].data = ((int) imageBpl0) >> 16;
	sCopList.image.bplptl[0].data = ((int) imageBpl0) & 0xffff;
	sCopList.image.bplpth[1].data = ((int) imageBpl1) >> 16;
	sCopList.image.bplptl[1].data = ((int) imageBpl1) & 0xffff;

	int palIndex = (sFrame >> 1) & (kPaletteCount - 1);
	const u16* pal = &((const u16*) gPalettes)[palIndex * kPaletteSize];

	sCopList.image.topcolors[ 0].data = pal[0];
	sCopList.image.topcolors[ 1].data = pal[1];
	sCopList.image.topcolors[ 2].data = pal[2];
	sCopList.image.topcolors[ 3].data = pal[3];
	sCopList.image.topcolors[ 4].data = 0x000;
	sCopList.image.topcolors[ 5].data = 0x000;
	sCopList.image.topcolors[ 6].data = 0x000;
	sCopList.image.topcolors[ 7].data = 0x000;
	sCopList.image.topcolors[ 8].data = (pal[0] >> 1) & 0x777;
	sCopList.image.topcolors[ 9].data = (pal[1] >> 1) & 0x777;
	sCopList.image.topcolors[10].data = (pal[2] >> 1) & 0x777;
	sCopList.image.topcolors[11].data = (pal[3] >> 1) & 0x777;
	sCopList.image.topcolors[12].data = pal[0] | 0xf77;
	sCopList.image.topcolors[13].data = pal[1] | 0xf77;
	sCopList.image.topcolors[14].data = pal[2] | 0xf77;
	sCopList.image.topcolors[15].data = pal[3] | 0xf77;

	for (int i = 0; i < 4; i++)
	{
		sCopList.image.midcolors[i].data = ((sCopList.image.topcolors[i].data >> 1) & 0x770) | 0x00f;
	}

	u16* bufferBpl0 = (u16*) sFrontBpls;
	u16* bufferBpl1 = bufferBpl0 + kBufferPlaneSize / 2;

	sCopList.image.bplpth[2].data = ((int) bufferBpl0) >> 16;
	sCopList.image.bplptl[2].data = ((int) bufferBpl0) & 0xffff;
	sCopList.image.bplpth[3].data = ((int) bufferBpl1) >> 16;
	sCopList.image.bplptl[3].data = ((int) bufferBpl1) & 0xffff;

	BlitClear();

	s16 x1 = cos(sFrame << 9) >> 7;
	s16 y1 = sin(sFrame << 9) >> 8;
	s16 x2 = kBufferWidth  / 2 - x1;
	s16 y2 = kBufferHeight / 2 - y1;
	x1 += kBufferWidth  / 2;
	y1 += kBufferHeight / 2;
	BlitLine(x1, y1, x2, y2, 3);

	Font_SetBpls(sBackBpls, sBackBpls + kBufferPlaneSize / 2);
	Font_DrawMessage("Demo Lab!", 13, 7);
	Font_DrawMessage("WPI 2026!", 17, 9);

	for (int i = 0; i < 1; i++)
	{
//		s16 x = cos((sFrame - (i << 4)) << 7) >> 7;
//		s16 y = sin((sFrame - (i << 3)) << 9) >> 8;
//		x += kBufferWidth  / 2 - kBobSize / 2;
//		y += kBufferHeight / 2 - kBobSize / 2;
		BlitBob(sFrame & 255, 76);
	}

	swap(sFrontBpls, sBackBpls);

	sFrame++;

	return !System_TestLMB1();
}
