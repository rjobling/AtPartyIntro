////////////////////////////////////////////////////////////////////////////////
// font.cpp
////////////////////////////////////////////////////////////////////////////////

#include "font.h"

#define USE_MASKED_DRAW

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static u8* sTextBpls[2];
static s16 sTextPitch;
static s16 sTextOffsets[64];

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static const u8* sFontBpls;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Font_Init(u16* textBpl1, u16* textBpl2, s16 textPitch, const u16* fontBpls)
{
	Font_SetBpls(textBpl1, textBpl2);

	sTextPitch = textPitch;

	s16 offset = 0;
	for (int i = 0; i < countof(sTextOffsets); i++)
	{
		sTextOffsets[i] = offset;
		offset += sTextPitch * Font_kSpacing;
	}

	sFontBpls = (const u8*) fontBpls;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Font_Deinit()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Font_SetBpls(u16* textBpl1, u16* textBpl2)
{
	sTextBpls[0] = (u8*) textBpl1;
	sTextBpls[1] = (u8*) textBpl2;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Font_DrawChar(u16 x, u16 y, char c)
{
	static_assert(Font_kSize == 16);
	static_assert(Font_kBplWidth == 256);
	static_assert(Font_kBplPlanes == 2);

	assert((c >= Font_kFirstChar) && (c <= Font_kLastChar));
	assert((y >= 0) && (y < countof(sTextOffsets)));

	u16 index = c - Font_kFirstChar;

	const u8* bits = sFontBpls + ((index & 15) << 1) + (index >> 4) * Font_kSize * Font_kBplPitch;

	s16 offset = x + sTextOffsets[y];

	u8* bpl1 = sTextBpls[0] + offset;
	u8* bpl2 = sTextBpls[1] + offset;

	for (int i = 0; i < Font_kHeight; i++)
	{
		#if defined(USE_MASKED_DRAW)

		u8 bits1 = *bits++;
		u8 bits2 = *bits++;

		bits += Font_kBplWidth / 8 - 2;

		u8 bits3 = *bits++;
		u8 bits4 = *bits++;

		bits += Font_kBplWidth / 8 - 2;

		u8 mask1 = ~(bits1 | bits3);
		u8 mask2 = ~(bits2 | bits4);

		*bpl1 = (*bpl1 & mask1) | bits1; bpl1++;
		*bpl1 = (*bpl1 & mask2) | bits2; bpl1++;
		*bpl2 = (*bpl2 & mask1) | bits3; bpl2++;
		*bpl2 = (*bpl2 & mask2) | bits4; bpl2++;

		bpl1 += sTextPitch - 2;
		bpl2 += sTextPitch - 2;

		#else

		*bpl1 = *bits;

		bits += Font_kBplWidth / 8;

		*bpl2 = *bits;

		bits += Font_kBplWidth / 8;

		bpl1 += sTextPitch;
		bpl2 += sTextPitch;

		#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char* Font_DrawMessage(const char* message, u16 x, u16 y)
{
	assert_pointer(message);
	assert_pointer(sFontBpls);
	assert_pointer(sTextBpls[0]);
	assert_pointer(sTextBpls[1]);

	u16 cx = x;
	u16 cy = y;

	for (char c = *message++; c != 'f'; c = *message++)
	{
		switch (c)
		{
			case '\0': return nullptr;

			case ' ':  cx++;		 break;
			case '\n': cx = x; cy++; break;
			case '\t': cx += 4;		 break;

			default:
				Font_DrawChar(cx, cy, c);
				cx++;
				break;
		}
	}

	return message;
}
