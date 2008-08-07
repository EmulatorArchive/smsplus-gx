/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/
/*****************************************************************************
 * IPL FONT Engine
 *
 * Based on Qoob MP3 Player Font
 * Added IPL font extraction
 *****************************************************************************/

extern void init_font(void);
extern void WriteCentre_HL( int y, const unsigned char *string);
extern void WriteCentre (int y, const unsigned char *text);
extern void write_font (int x, int y, const unsigned char *text);
extern void WaitPrompt (char *msg);
extern void ShowAction (char *msg);
extern void WaitButtonA ();
extern void unpackBackdrop ();
extern void ClearScreen ();
extern void SetScreen ();
extern void fntDrawBoxFilled (int x1, int y1, int x2, int y2, int color);
extern void setfontcolour (int fcolour);
extern int fheight;
extern int font_size[256];
extern u16 back_framewidth;
