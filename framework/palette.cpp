////////////////////////////////////////////////////////////////////////////////
// palette.cpp
////////////////////////////////////////////////////////////////////////////////

#include "palette.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static u8 sBlendTable[16 * 16 * (16 - 1)];

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Palette_InitBlendTable()
{
	s8 scaleTable[31][16];

	for (int diff = 0; diff < 16; diff++)
	{
		s8* blend_x_pos_diff = scaleTable[ diff + 15];
		s8* blend_x_neg_diff = scaleTable[-diff + 15];

		for (int blend = 0; blend < 16; blend++)
		{
			s8 value = (blend * diff) >> 4;

			*blend_x_pos_diff++ =  value;
			*blend_x_neg_diff++ = -value;
		}
	}

	u8* entry = sBlendTable;

	for (int to = 0; to < 16; to++)
	{
		for (int from = 0; from < 16; from++)
		{
			const s8* blend_x_diff = scaleTable[(to - from) + 15];

			for (int blend = 1; blend < 16; blend++)
			{
				entry[(blend - 1) << 8] = from + blend_x_diff[blend];
			}

			entry++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Palette_DeinitBlendTable()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Palette_BlendColors(Color* results, int resultPitch, const Color* from, const Color* to, u16 count, u16 blend)
{
	assert(isaligned(resultPitch, (int) sizeof(Color)));
	assert(blend <= 16);

	if (blend == 0)
	{
		for (int i = 0; i < count; i++)
		{
			*results = *from++;
			results += resultPitch / sizeof(Color);
		}

		return;
	}

	if (blend == 16)
	{
		for (int i = 0; i < count; i++)
		{
			*results = *to++;
			results += resultPitch / sizeof(Color);
		}

		return;
	}

	const u8* table = sBlendTable + ((blend - 1) << 8);

	for (int i = 0; i < count; i++)
	{
		Color f = *from++;
		Color t = *to++;

		Color r = ((t & 0xf00) >> 4) + ((f & 0xf00) >> 8);
		Color g = ((t & 0x0f0) >> 0) + ((f & 0x0f0) >> 4);
		Color b = ((t & 0x00f) << 4) + ((f & 0x00f) >> 0);

		*results = (table[r] << 8) + (table[g] << 4) + table[b];
		results += resultPitch / sizeof(Color);
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Palette_BlendColors(Color* results, int resultPitch, const Color* from, Color to, u16 count, u16 blend)
{
	assert(isaligned(resultPitch, (int) sizeof(Color)));
	assert(blend <= 16);

	if (blend == 0)
	{
		for (int i = 0; i < count; i++)
		{
			*results = *from++;
			results += resultPitch / sizeof(Color);
		}

		return;
	}

	if (blend == 16)
	{
		for (int i = 0; i < count; i++)
		{
			*results = to;
			results += resultPitch / sizeof(Color);
		}

		return;
	}

	const u8* table = sBlendTable + ((blend - 1) << 8);

	const u8* rtable = table + ((to & 0xf00) >> 4);
	const u8* gtable = table + ((to & 0x0f0) >> 0);
	const u8* btable = table + ((to & 0x00f) << 4);

	for (int i = 0; i < count; i++)
	{
		Color f = *from++;

		Color r = (f & 0xf00) >> 8;
		Color g = (f & 0x0f0) >> 4;
		Color b = (f & 0x00f) >> 0;

		*results = (rtable[r] << 8) + (gtable[g] << 4) + btable[b];
		results += resultPitch / sizeof(Color);
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Palette_BlendColors(Color* results, int resultPitch, Color from, const Color* to, u16 count, u16 blend)
{
	Palette_BlendColors(results, resultPitch, to, from, count, 16 - blend);
}
