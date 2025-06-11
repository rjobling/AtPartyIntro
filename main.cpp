////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

#include "framework/core.h"
#include "framework/system.h"
#include "intro.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main()
{
	if (System_Init())
	{
		#if defined(DEBUG)
		KPrintF("Chip Avail: %ld (%ld largest)", AvailMem(MEMF_CHIP), AvailMem(MEMF_CHIP | MEMF_LARGEST));
		KPrintF("Fast Avail: %ld (%ld largest)", AvailMem(MEMF_FAST), AvailMem(MEMF_FAST | MEMF_LARGEST));
		KPrintF("Any  Avail: %ld (%ld largest)", AvailMem(MEMF_ANY) , AvailMem(MEMF_ANY  | MEMF_LARGEST));
		#endif

		if (Intro_Init())
		{
			while (Intro_Update()) {}

			Intro_Deinit();
		}

		System_Deinit();
	}
}
