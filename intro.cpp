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
#include "framework/lsp.h"
#include "framework/system.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kViewWidth	 = 320;
static constexpr int kViewHeight = 256;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static constexpr int kImageWidth	 = 320;
static constexpr int kImageHeight	 = 320;
static constexpr int kImagePlanes	 = 2;
static constexpr int kImagePlaneSize = (kImageWidth / 8) * kImageHeight;
static constexpr int kImagePitch	 = (kImageWidth / 8) * kImagePlanes;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN_CHIP(gImageBpls, "data/image_bpls.bin");
INCBIN(gImagePal, "data/image_pal.bin");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
INCBIN_CHIP(gLSPMusic, "data/statetrue.lsmusic");
INCBIN_CHIP(gLSPBank, "data/statetrue.lsbank");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct CopList
{
	CopCommand topcolor[1 << kImagePlanes] = {
		CopMove(color[0], ((const u16*) gImagePal)[0]),
		CopMove(color[1], ((const u16*) gImagePal)[1]),
		CopMove(color[2], ((const u16*) gImagePal)[2]),
		CopMove(color[3], ((const u16*) gImagePal)[3]),
	};

	CopCommand topbpl1mod = CopMove(bpl1mod, 0);
	CopCommand topbpl2mod = CopMove(bpl2mod, 0);

	CopCommand midwait = CopWait(0, 44 + 192);

	CopCommand midcolor[1 << kImagePlanes] = {
		CopMove(color[0], ~((const u16*) gImagePal)[0]),
		CopMove(color[1], ~((const u16*) gImagePal)[1]),
		CopMove(color[2], ~((const u16*) gImagePal)[2]),
		CopMove(color[3], ~((const u16*) gImagePal)[3]),
	};

	CopCommand midbpl1mod = CopMove(bpl1mod, -kImagePitch * 2);
	CopCommand midbpl2mod = CopMove(bpl2mod, -kImagePitch * 2);

	CopCommand end = CopEnd();
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static CopList sCopList __attribute__((section(".MEMF_CHIP")));

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct Intro_Data
{
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	int frame = 0;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static Intro_Data sData;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#if defined(DEBUG)
static constexpr u16 kDebugPalette[] = {
	0x123, 0x800, 0x830, 0x860, 0x880, 0x480, 0x080, 0x088, 0x068, 0x048, 0x028, 0x008, 0x308, 0x608, 0x808, 0x804,
	0x246, 0xf00, 0xf50, 0xfa0, 0xff0, 0x8f0, 0x0f0, 0x0ff, 0x0cf, 0x08f, 0x04f, 0x00f, 0x50f, 0xa0f, 0xf0f, 0xf08,
};
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool Intro_Init()
{
	custom.bplcon0 = PackBplcon0(kImagePlanes);
	custom.bplcon1 = PackBplcon1(0, 0);
	custom.bplcon2 = PackBplcon2(false, 4, 4);
	custom.diwstrt = PackDiwstrt(0, 0);
	custom.diwstop = PackDiwstop(kViewWidth, kViewHeight);
	custom.ddfstrt = PackDdfstrt(0);
	custom.ddfstop = PackDdfstop(kViewWidth);

	debug_register_bitmap(gImageBpls, "ImageBpls", kImageWidth, kImageHeight, kImagePlanes, 0);
	debug_register_palette(gImagePal, "ImagePal", 1 << kImagePlanes, 0);
	debug_register_palette(kDebugPalette, "Debug", countof(kDebugPalette), 0);

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

	// Disable DMA which will stop the copper.
	custom.dmacon = DMAF_RASTER | DMAF_COPPER | DMAF_BLITTER | DMAF_BLITHOG;

	LSP_MusicDriver_CIA_Stop();

	debug_unregister(kDebugPalette);
	debug_unregister(gImageBpls);
	debug_unregister(gImagePal);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool Intro_Update()
{
	System_WaitVbl();

	int scroll = (16384 - cos(sData.frame << 9)) >> 9;
	int offset = scroll * kImagePitch;

	const u8* bpls = ((const u8*) gImageBpls) + offset;

	custom.bplpt[0] = (void*) (bpls + kImagePlaneSize * 0);
	custom.bplpt[1] = (void*) (bpls + kImagePlaneSize * 1);

	sData.frame++;

	return !System_TestLMB1();
}
