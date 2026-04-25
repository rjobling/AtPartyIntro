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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kBufferWidth	  = 320;
static constexpr int kBufferHeight	  = 320;
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
static u8 sBufferBpls[kBufferSize][2] __attribute__((section(".MEMF_CHIP")));

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
			CopMoveH(bplpt[0], (const u8*) gLogoBpls + kLogoPlaneSize * 0),
			CopMoveH(bplpt[1], (const u8*) gLogoBpls + kLogoPlaneSize * 1),
			CopMoveH(bplpt[2], (const u8*) gLogoBpls + kLogoPlaneSize * 2),
			CopMoveH(bplpt[3], (const u8*) gLogoBpls + kLogoPlaneSize * 3),
		};
		CopCommand bplptl[kLogoPlanes] = {
			CopMoveL(bplpt[0], (const u8*) gLogoBpls + kLogoPlaneSize * 0),
			CopMoveL(bplpt[1], (const u8*) gLogoBpls + kLogoPlaneSize * 1),
			CopMoveL(bplpt[2], (const u8*) gLogoBpls + kLogoPlaneSize * 2),
			CopMoveL(bplpt[3], (const u8*) gLogoBpls + kLogoPlaneSize * 3),
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
			CopMoveH(bplpt[0], (const u8*) gImageBpls + kImagePlaneSize * 0),
			CopMoveH(bplpt[1], (const u8*) gImageBpls + kImagePlaneSize * 1),
			CopMoveH(bplpt[2], sBufferBpls[0] + kBufferPlaneSize * 0),
			CopMoveH(bplpt[3], sBufferBpls[0] + kBufferPlaneSize * 1),
		};
		CopCommand bplptl[kImagePlanes + kBufferPlanes] = {
			CopMoveL(bplpt[0], (const u8*) gImageBpls + kImagePlaneSize * 0),
			CopMoveL(bplpt[1], (const u8*) gImageBpls + kImagePlaneSize * 1),
			CopMoveL(bplpt[2], sBufferBpls[0] + kBufferPlaneSize * 0),
			CopMoveL(bplpt[3], sBufferBpls[0] + kBufferPlaneSize * 1),
		};

		CopCommand topcolors[1 << (kImagePlanes + kBufferPlanes)] = {
			CopMove(color[ 0], ((const u16*) gPalettes)[0]),
			CopMove(color[ 1], ((const u16*) gPalettes)[1]),
			CopMove(color[ 2], ((const u16*) gPalettes)[2]),
			CopMove(color[ 3], ((const u16*) gPalettes)[3]),
			CopMove(color[ 4], ((const u16*) gBobsPal)[1]),
			CopMove(color[ 5], ((const u16*) gBobsPal)[1]),
			CopMove(color[ 6], ((const u16*) gBobsPal)[1]),
			CopMove(color[ 7], ((const u16*) gBobsPal)[1]),
			CopMove(color[ 8], ((const u16*) gBobsPal)[2]),
			CopMove(color[ 9], ((const u16*) gBobsPal)[2]),
			CopMove(color[10], ((const u16*) gBobsPal)[2]),
			CopMove(color[11], ((const u16*) gBobsPal)[2]),
			CopMove(color[12], ((const u16*) gBobsPal)[3]),
			CopMove(color[13], ((const u16*) gBobsPal)[3]),
			CopMove(color[14], ((const u16*) gBobsPal)[3]),
			CopMove(color[15], ((const u16*) gBobsPal)[3]),
		};

		CopCommand midwait1 = CopWait(4, 255);
		CopCommand midwait2 = CopWait(4, 44 + 224);

		CopCommand midbpl1mod = CopMove(bpl1mod, -kImagePitch * 3);
		CopCommand midbpl2mod = CopMove(bpl2mod, -kImagePitch * 3);

		CopCommand midcolors[1 << kImagePlanes] = {
			CopMove(color[0], ~((const u16*) gPalettes)[0]),
			CopMove(color[1], ~((const u16*) gPalettes)[1]),
			CopMove(color[2], ~((const u16*) gPalettes)[2]),
			CopMove(color[3], ~((const u16*) gPalettes)[3]),
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
static int sFrame = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool Intro_Init()
{
	Font_Init((u16*) gImageBpls, (u16*) gImageBpls + kImagePlaneSize / 2, kImagePitch, (const u16*) gFontBpls);

	const char* message = "Demo Lab!fWPI 2026!";
	message = Font_DrawMessage(message, 10, 12);
	message = Font_DrawMessage(message, 20, 14);

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
	custom.dmacon = DMAF_SETCLR | DMAF_MASTER | DMAF_RASTER | DMAF_COPPER;

	// Enable interrupts when still nearly at the top of the frame.
	custom.intena = INTF_SETCLR | INTF_INTEN;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Intro_Deinit()
{
	System_WaitVbl();

	// Disable DMA which will stop the copper.
	custom.dmacon = DMAF_RASTER | DMAF_COPPER;

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
bool Intro_Update()
{
	System_WaitVbl();

	int scroll = (16384 - cos(sFrame << 9)) >> 8;

	const u8* bpl0 = (const u8*) gImageBpls + scroll * kImagePitch;
	const u8* bpl1 = bpl0 + kImagePlaneSize;

	sCopList.image.bplpth[0].data = ((int) bpl0) >> 16;
	sCopList.image.bplptl[0].data = ((int) bpl0) & 0xffff;
	sCopList.image.bplpth[1].data = ((int) bpl1) >> 16;
	sCopList.image.bplptl[1].data = ((int) bpl1) & 0xffff;

	int palIndex = (sFrame >> 1) & (kPaletteCount - 1);
	const u16* pal = &((const u16*) gPalettes)[palIndex * kPaletteSize];

	sCopList.image.topcolors[0].data = pal[0];
	sCopList.image.topcolors[1].data = pal[1];
	sCopList.image.topcolors[2].data = pal[2];
	sCopList.image.topcolors[3].data = pal[3];
	sCopList.image.midcolors[0].data = ~pal[0];
	sCopList.image.midcolors[1].data = ~pal[1];
	sCopList.image.midcolors[2].data = ~pal[2];
	sCopList.image.midcolors[3].data = ~pal[3];

	sFrame++;

	return !System_TestLMB1();
}
