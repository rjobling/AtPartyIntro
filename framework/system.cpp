////////////////////////////////////////////////////////////////////////////////
// system.cpp
////////////////////////////////////////////////////////////////////////////////

#include "system.h"
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <hardware/adkbits.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static Custom& sCustom = *((Custom*) 0xdff000);
static CIA& sCiaa = *((CIA*) 0xbfe001);
static CIA& sCiab = *((CIA*) 0xbfd000);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Custom& custom = sCustom;
CIA& ciaa = sCiaa;
CIA& ciab = sCiab;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct ExecBase* SysBase;
struct DosLibrary* DOSBase;
struct GfxBase* GfxBase;
struct IntuitionBase* IntuitionBase;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static const char* sError;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void* sVBR;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static bool sSavedWorkbench;
static View* sSavedActiView;
static u16 sSavedADKCON;
static u16 sSavedDMACON;
static u16 sSavedINTENA;
static u8 sSavedCIAAICR;
static u8 sSavedCIAACRA;
static u8 sSavedCIAACRB;
static u8 sSavedCIABICR;
static u8 sSavedCIABCRA;
static u8 sSavedCIABCRB;
static System_IrqFunc* sSavedIrq1Handler;
static System_IrqFunc* sSavedIrq2Handler;
static System_IrqFunc* sSavedIrq3Handler;
static System_IrqFunc* sSavedIrq6Handler;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_Init()
{
	sError = nullptr;

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Warray-bounds"
	SysBase = *((struct ExecBase**) 4);
	#pragma GCC diagnostic pop

	sVBR = 0;
	if (SysBase->AttnFlags & AFF_68010)
	{
		u16 getvbr[] = {0x4e7a, 0x0801, 0x4e73}; // movec.l vbr,d0 rte
		sVBR = (void*) Supervisor((ULONG (*)()) getvbr);
	}

	DOSBase = (struct DosLibrary*) OpenLibrary((CONST_STRPTR) "dos.library", 0);
	if (DOSBase == nullptr)
	{
		return false;
	}

	GfxBase = (struct GfxBase*) OpenLibrary((CONST_STRPTR) "graphics.library", 0);
	if (GfxBase == nullptr)
	{
		CloseLibrary((Library*) DOSBase);

		return false;
	}

	IntuitionBase = (struct IntuitionBase*) OpenLibrary((CONST_STRPTR) "intuition.library", 0);
	if (IntuitionBase == nullptr)
	{
		CloseLibrary((Library*) GfxBase);
		CloseLibrary((Library*) DOSBase);

		return false;
	}

	sSavedWorkbench = CloseWorkBench();

	OwnBlitter();
	WaitBlit();

	Forbid();

	sSavedActiView = GfxBase->ActiView;

	LoadView(nullptr);
	WaitTOF();
	WaitTOF();

	Disable();

	// Save current interrupt and DMA settings.
	sSavedADKCON = custom.adkconr;
	sSavedDMACON = custom.dmaconr;
	sSavedINTENA = custom.intenar;

	// Disable all interrupts.
	custom.intena = (u16) ~INTF_SETCLR;

	// Disable all DMA channels.
	custom.dmacon = (u16) ~DMAF_SETCLR;

	// Disable all CIA interrupts.
	ciaa.ciaicr = (u8) ~CIAICRF_SETCLR;
	ciab.ciaicr = (u8) ~CIAICRF_SETCLR;

	// Save current CIA interrupts, which clears any pending.
	sSavedCIAAICR = ciaa.ciaicr;
	sSavedCIABICR = ciab.ciaicr;

	// Save current CIA controls.
	sSavedCIAACRA = ciaa.ciacra;
	sSavedCIAACRB = ciaa.ciacrb;
	sSavedCIABCRA = ciab.ciacra;
	sSavedCIABCRB = ciab.ciacrb;

	// Clear CIA Alarm count.
	ciab.ciacrb = CIACRBF_ALARM;
	ciab.ciatodhi = 0;
	ciab.ciatodmid = 0;
	ciab.ciatodlow = 0;

	// Clear all CIA controls.
	ciaa.ciacra = 0;
	ciaa.ciacrb = 0;
	ciab.ciacra = 0;
	ciab.ciacrb = 0;

	// Clear CIA TOD count.
	ciab.ciatodhi = 0;
	ciab.ciatodmid = 0;
	ciab.ciatodlow = 0;

	// Pause CIA TOD count.
	ciab.ciatodhi = 0;

	// Set all colors to black.
	for (int i = 0; i < countof(custom.color); i++)
	{
		custom.color[i] = 0x000;
	}

	System_WaitVbl();

	// Make sure toggle is set to long frame.
	custom.vposw = 0x8000;

	// Save current interrupt handlers.
	sSavedIrq1Handler = System_GetIrq1Handler();
	sSavedIrq2Handler = System_GetIrq2Handler();
	sSavedIrq3Handler = System_GetIrq3Handler();
	sSavedIrq6Handler = System_GetIrq6Handler();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_Deinit()
{
	System_WaitBlt();
	System_WaitVbl();

	// Disable all interrupts.
	custom.intena = (u16) ~INTF_SETCLR;

	// Disable all DMA channels.
	custom.dmacon = (u16) ~DMAF_SETCLR;

	// Disable all CIA interrupts.
	ciaa.ciaicr = (u8) ~CIAICRF_SETCLR;
	ciab.ciaicr = (u8) ~CIAICRF_SETCLR;

	// Clear all pending interrupts.
	custom.intreq = (u16) ~INTF_SETCLR;

	// Clear all pending CIA interrupts.
	u8 pending;
	pending = ciaa.ciaicr;
	pending = ciab.ciaicr;
	unused(pending);

	// Restore interrupt handlers.
	System_SetIrq1Handler(sSavedIrq1Handler);
	System_SetIrq2Handler(sSavedIrq2Handler);
	System_SetIrq3Handler(sSavedIrq3Handler);
	System_SetIrq6Handler(sSavedIrq6Handler);

	// Restore copper lists.
	custom.cop1lc = (u32) GfxBase->copinit;
	custom.cop2lc = (u32) GfxBase->LOFlist;
	custom.copjmp1 = 0;

	// Restore interrupts and DMA settings.
	custom.adkcon = ADKF_SETCLR | sSavedADKCON;
	custom.dmacon = DMAF_SETCLR | sSavedDMACON;
	custom.intena = INTF_SETCLR | sSavedINTENA;

	// Restore CIA timer settings.
	ciaa.ciacra = sSavedCIAACRA;
	ciaa.ciacrb = sSavedCIAACRB;
	ciab.ciacra = sSavedCIABCRA;
	ciab.ciacrb = sSavedCIABCRB;

	// Restore CIA interrupts.
	ciaa.ciaicr = sSavedCIAAICR;
	ciab.ciaicr = sSavedCIABICR;

	// Enable CIAA interrupts for keyboard inputs.
	ciaa.ciaicr = CIAICRF_SETCLR | CIAICRF_SP;

	Enable();

	LoadView(sSavedActiView);
	WaitTOF();
	WaitTOF();

	Permit();

	WaitBlit();
	DisownBlitter();

	if (sSavedWorkbench)
	{
		OpenWorkBench();
	}

	if (sError != nullptr)
	{
		Write(Output(), (APTR) sError, strlen(sError) + 1);
	}

	CloseLibrary((Library*) IntuitionBase);
	CloseLibrary((Library*) GfxBase);
	CloseLibrary((Library*) DOSBase);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_SetError(const char* error)
{
	sError = error;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_IsAGA()
{
	return (custom.vposr & 0x0100);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void* System_GetVBR()
{
	return sVBR;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
System_IrqFunc* System_GetIrq1Handler()
{
	return *((System_IrqFunc**) (((u8*) sVBR) + 0x64));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
System_IrqFunc* System_GetIrq2Handler()
{
	return *((System_IrqFunc**) (((u8*) sVBR) + 0x68));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
System_IrqFunc* System_GetIrq3Handler()
{
	return *((System_IrqFunc**) (((u8*) sVBR) + 0x6c));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
System_IrqFunc* System_GetIrq6Handler()
{
	return *((System_IrqFunc**) (((u8*) sVBR) + 0x78));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_SetIrq1Handler(System_IrqFunc* func)
{
	*((System_IrqFunc**) (((u8*) sVBR) + 0x64)) = func;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_SetIrq2Handler(System_IrqFunc* func)
{
	*((System_IrqFunc**) (((u8*) sVBR) + 0x68)) = func;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_SetIrq3Handler(System_IrqFunc* func)
{
	*((System_IrqFunc**) (((u8*) sVBR) + 0x6c)) = func;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_SetIrq6Handler(System_IrqFunc* func)
{
	*((System_IrqFunc**) (((u8*) sVBR) + 0x78)) = func;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_WaitVbl()
{
	debug_start_idle();

	asm volatile(
	"zero%=:						\n"
	"		tst.b	(%[vposr_l])	\n"
	"		beq.b	zero%=			\n"
	"one%=:							\n"
	"		tst.b	(%[vposr_l])	\n"
	"		bne.b	one%=			\n"
	:
	:
	[vposr_l] "a" (&custom.vposr_l)
	:
	"cc"
	);

	debug_stop_idle();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_WaitBlt()
{
	u16 test = custom.dmaconr; // For compatiblity with A1000.
	unused(test);

	debug_start_idle();
	while (custom.dmaconr & DMAF_BLTDONE) {}
	debug_stop_idle();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_TestLMB1()
{
	return !(ciaa.ciapra & CIAF_GAMEPORT0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_TestRMB1()
{
	return !(custom.potinp & (1 << 10));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_TestLMB2()
{
	return !(ciaa.ciapra & CIAF_GAMEPORT1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_TestRMB2()
{
	return !(custom.potinp & (1 << 14));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool System_GetAudioFilter()
{
	return ((ciaa.ciapra & CIAF_LED) != 0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void System_SetAudioFilter(bool on)
{
	ciaa.ciapra = on ? (ciaa.ciapra & ~CIAF_LED) : (ciaa.ciapra | CIAF_LED);
}
