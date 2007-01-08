/* This is defined by both Carbon and non-Carbon prefix headers */
#if  !defined(TARGET_API_MAC_CARBON)
   /* so if it is not defined yet, there is no prefix file, 
      and we are compiling the "Transitional" build. */
   /* Use MacHeaders.h until ready to convert this file.
      Then change to MacHeadersTransitional.h. */
#  include	"MacHeaders.h"
// #  include	"MacHeadersTransitional.h"
#endif

/*
 *	WETabHooks.c
 *
 *	Hooks for adding tab support to WASTE.
 *	
 *	Original code by Mark Alldritt
 *	Line breaking code by Dan Crevier
 *	Support for horizontal scrolling by Bert Seltzer
 *	Maintenance by John C. Daub
 *	Further modifications by Marco Piovanelli
 *
 */
 
#include "WETabHooks.h"

#ifndef kTabWidth
#define	kTabWidth	32
#endif

/*
	If kTabWidth is a power of two, we can exploit the fact that
		(x % kTabWidth) == (x & (kTabWidth - 1))
	and replace those expensive modulus (%) operators with much
	cheaper ANDs.  (From a suggestion by Mark Valence.)
*/

#define IS_POWER_OF_TWO(x)		(((x) & ((x) - 1)) == 0)

#if IS_POWER_OF_TWO(kTabWidth)
	/* inline short MOD(short x) { return (x & (kTabWidth - 1)); } */
	#define MOD(x)				(((short) (x)) & (kTabWidth - 1))
#else
	/* inline short MOD(short x) { return (x % kTabWidth); } */
	#define MOD(x)				(((short) (x)) % kTabWidth)
#endif

/* inline short FIXROUND(Fixed f) { return ((f + 0x00008000) >> 16); } */
#define FIXROUND(f)		((short) (((f) + 0x00008000) >> 16))

pascal void _WETabDrawText(Ptr pText, long textLength, Fixed slop,
				JustStyleCode styleRunPosition, WEHandle hWE)
{
#pragma unused(slop, styleRunPosition)

	long beginChar = 0;
	long ii;
	short tabWidth;
	short destLeft;
	Point penPos;
	
	destLeft = (short) (*hWE)->destRect.left;
	
	for ( ii = 0; ii < textLength; ii++ )
	{
		if (pText[ii] == '\t')
		{
			DrawText(pText, beginChar, ii - beginChar);

			/* advance the pen to the next tab stop */
			GetPen(&penPos);
			tabWidth = kTabWidth - MOD(penPos.h - destLeft);
			MoveTo(penPos.h + tabWidth, penPos.v);
			beginChar = ii + 1;
		}
	}	/* for */

	DrawText(pText, beginChar, textLength - beginChar);
}

pascal long _WETabPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE)
{
#pragma unused(hWE)

	long beginChar = 0;
	long offset = 0;
	long ii;
	Fixed lastWidth;
	Fixed tabWidth;
	
	/* loop through every character in the segment looking for tabs */
	for ( ii = 0; ii < textLength; ii++ )
	{
		/* exit now if width has gone negative */
		/* (i.e., if we have found which glyph was hit) */
		if (*width <= 0)
			break;

		/* tab found? */
		if (pText[ii] == '\t')
		{
			/* calculate the width of the sub-segment preceding the tab */
			lastWidth = *width;
			offset += PixelToChar(pText + beginChar, ii - beginChar, slop,
					lastWidth, (unsigned char *) edge, width, styleRunPosition,
					kOneToOneScaling, kOneToOneScaling);
			beginChar = ii + 1;

			/* hit point past sub-segment? */
			if (*width >= 0)
			{
				/* increment hPos by width of sub-segment preceding the tab */
				hPos += (lastWidth - *width);
				
				/* calculate the width of the tab "glyph" (as a Fixed value) */
				tabWidth = BSL(kTabWidth - MOD(FIXROUND(hPos)), 16);
				
				/* increment hPos by width of tab character */
				hPos += tabWidth;
				
				/* hit point within tab glyph? */
				if (*width < tabWidth)
				{
					/* yes: determine which half of tab glyph was hit */
					if (*width > (tabWidth >> 1))
					{
						*edge = kTrailingEdge;	/* second (trailing) edge of tab */
						offset++;
					}
					else
						*edge = kLeadingEdge;	/* first (leading) edge of tab */
					
					/* returning -1 (as Fixed) in width means we're finished */
					*width = 0xFFFF0000;
				}
				else {
					/* hit point is past tab: keep looping */
					offset++;
					*width -= tabWidth;
				}
			} /* if (*width >= 0) */
		} /* if tab found */
	} /* for */
	
	/* no more tabs in this segment: process the last sub-segment */
	if (*width >= 0)
	{
		lastWidth = *width;
		offset += PixelToChar(pText + beginChar, textLength - beginChar, slop,
					lastWidth, (unsigned char *) edge, width, styleRunPosition,
					kOneToOneScaling, kOneToOneScaling);
	}
	
	/* round width to nearest integer value */
	/* this is supposed to fix an incompatibility with the WorldScript Power Adapter */
	*width = (*width + 0x00008000) & 0xFFFF0000;
	
	return offset;
}

pascal short _WETabCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE)
{
#pragma unused(slop, direction, styleRunPosition)

	long beginChar = 0;
	long ii;
	short width;
	short destLeft;
	short totalWidth = 0;

	destLeft = (short) (*hWE)->destRect.left;

	/* measure text up to offset, if offset is within this segment */
	if (offset < textLength)
		textLength = offset;
	
	for ( ii = 0; ii < textLength; ii++ )
	{
		if (pText[ii] == '\t')
		{
			/* calculate the pixel width of the subsegment preceding the tab */
			width = TextWidth(pText, beginChar, ii - beginChar);
			totalWidth += width;
			hPos += width;
			
			/* calculate tab width */
			width = kTabWidth - MOD(hPos - destLeft);
			totalWidth += width;
			hPos += width;
			
			/* go to next subsegment */
			beginChar = ii + 1;
		}
	} /* for */
	
	/* calculate width of remaining characters */
	width = TextWidth(pText, beginChar, textLength - beginChar);
	totalWidth += width;
	
	return totalWidth;
}

pascal StyledLineBreakCode _WETabLineBreak(Ptr pText, long textLength,
			long textStart, long textEnd, Fixed *textWidth,
			long *textOffset, WEHandle hWE)
{
	long beginChar = textStart;
	long ii;
	Fixed tabWidth;
	short destWidth;
	StyledLineBreakCode breakCode = smBreakOverflow;
	
	destWidth = (short) ((*hWE)->destRect.right - (*hWE)->destRect.left);
	
	for ( ii = textStart; ii < textEnd; ii++ )
	{
		if (pText[ii] == '\t')
		{
			/* do previous "segment" */
			breakCode = StyledLineBreak(pText, textLength, beginChar,
							ii, 0, textWidth, textOffset);
			if ((breakCode != smBreakOverflow) || (ii >= textLength))
				break;
			beginChar = ii + 1;
			
			/* calculate tab width (as a Fixed value) */
			tabWidth = BSL(kTabWidth - MOD(destWidth - FIXROUND(*textWidth)), 16);
			
			/* if tabWidth > pixelWidth we break in tab */
			/* don't move tab to next line */
			if (tabWidth > *textWidth)
			{
				breakCode = smBreakWord;
				*textOffset = ii + 1;
				break;
			}
			else
				*textWidth -= tabWidth;
		}
	} /* for */

	/* do last sub-segment */
	if ((ii - beginChar >= 0) && (breakCode == smBreakOverflow))
		breakCode = StyledLineBreak(pText, textLength, beginChar, ii, 0, textWidth, textOffset);
	
	return breakCode;
}