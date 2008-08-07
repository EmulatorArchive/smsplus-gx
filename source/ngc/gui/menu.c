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
 * Nintendo Gamecube Menus
 *
 * Please put any user menus here! - softdev March 12 2006
 ***************************************************************************/
#include "shared.h"
#include "dvd.h"
#include "config.h"
#include "font.h"

#define PSOSDLOADID 0x7c6000a6

extern unsigned int *xfb[2];
extern int whichfb;

/***************************************************************************
 * drawmenu
 *
 * As it says, simply draws the menu with a highlight on the currently
 * selected item :)
 ***************************************************************************/
char menutitle[60] = { "" };
int menu = 0;

void drawmenu (char items[][20], int maxitems, int selected)
{
  int i;
  int ypos;

  ypos = (310 - (fheight * maxitems)) >> 1;
  ypos += 130;

  ClearScreen ();
  WriteCentre (134, menutitle);

  for (i = 0; i < maxitems; i++)
  {
    if (i == selected) WriteCentre_HL (i * fheight + ypos, (char *) items[i]);
    else WriteCentre (i * fheight + ypos, (char *) items[i]);
  }

  SetScreen ();
}

/****************************************************************************
 * domenu
 *
 * Returns index into menu array when A is pressed, -1 for B
 ****************************************************************************/
int domenu (char items[][20], int maxitems)
{
  int redraw = 1;
  int quit = 0;
  short p;
  int ret = 0;
  signed char a,b;

  while (quit == 0)
  {
    if (redraw)
	{
	  drawmenu (&items[0], maxitems, menu);
	  redraw = 0;
	}

    p = PAD_ButtonsDown (0);
    a = PAD_StickY (0);
	b = PAD_StickX (0);

    if ((p & PAD_BUTTON_UP) || (a > 70))
	{
	  redraw = 1;
	  menu--;
      if (menu < 0) menu = maxitems - 1;
	}

    if ((p & PAD_BUTTON_DOWN) || (a < -70))
	{
	  redraw = 1;
	  menu++;
      if (menu == maxitems) menu = 0;
	}

    if ((p & PAD_BUTTON_A) || (b > 60) || (p & PAD_BUTTON_RIGHT))
	{
	  quit = 1;
	  ret = menu;
	}
	  
    if ((b < -60) || (p & PAD_BUTTON_LEFT))
	{
	   quit = 1;
	   ret = 0 - 2 - menu;
	}

    if (p & PAD_BUTTON_B)
	{
	  quit = 1;
	  ret = -1;
	}
    }

  return ret;
}

/****************************************************************************
 * Options menu
 *
 ****************************************************************************/
#ifdef DEBUG
extern uint8 sms_cram_expand_table[4];

void palmenu ()
{
  int ret;
  int quit = 0;
  int misccount = 6;
  char miscmenu[6][20];
  int prevmenu = menu;

  strcpy (menutitle, "Press B to return");

  menu = 0;

  while (quit == 0)
  {
	  int8 val;

	  sprintf (miscmenu[0], "R,G (1/3) = %d", (sms_cram_expand_table[1] >> 3) & 0x1F);
	  sprintf (miscmenu[1], "B   (1/3) = %d", (sms_cram_expand_table[1] >> 2) & 0x3F);
	  sprintf (miscmenu[2], "R,G (2/3) = %d", (sms_cram_expand_table[2] >> 3) & 0x1F);
	  sprintf (miscmenu[3], "B   (2/3) = %d", (sms_cram_expand_table[2] >> 2) & 0x3F);
	  sprintf (miscmenu[4], "R,G (3/3) = %d", (sms_cram_expand_table[3] >> 3) & 0x1F);
	  sprintf (miscmenu[5], "B   (3/3) = %d", (sms_cram_expand_table[3] >> 2) & 0x3F);

	  ret = domenu (&miscmenu[0], misccount);

	  switch (ret)
	  {
		case 0:
		case -2:
			val = (sms_cram_expand_table[1] >> 3) & 0x1F;
			if (ret < 0) val --;
			else val ++;
			if (val < 0) val = 31;
			if (val > 31) val = 0;
			sms_cram_expand_table[1] = val << 3;
			break;

		case 1:
		case -3:
			val = (sms_cram_expand_table[1] >> 2) & 0x3F;
			if (ret < 0) val --;
			else val ++;
			if (val < 0) val = 63;
			if (val > 63) val = 0;
			sms_cram_expand_table[1] = val << 2;
			break;

		case 2:
		case -4:
			val = (sms_cram_expand_table[2] >> 3) & 0x1F;
			if (ret < 0) val --;
			else val ++;
			if (val < 0) val = 31;
			if (val > 31) val = 0;
			sms_cram_expand_table[2] = val << 3;
			break;

		case 3:
		case -5:
			val = (sms_cram_expand_table[2] >> 2) & 0x3F;
			if (ret < 0) val --;
			else val ++;
			if (val < 0) val = 63;
			if (val > 63) val = 0;
			sms_cram_expand_table[2] = val << 2;
			break;

		case 4:
		case -6:
			val = (sms_cram_expand_table[3] >> 3) & 0x1F;
			if (ret < 0) val --;
			else val ++;
			if (val < 0) val = 31;
			if (val > 31) val = 0;
			sms_cram_expand_table[3] = val << 3;
			break;

		case 5:
		case -7:
			val = (sms_cram_expand_table[3] >> 2) & 0x3F;
			if (ret < 0) val --;
			else val ++;
			if (val < 0) val = 63;
			if (val > 63) val = 0;
			sms_cram_expand_table[3] = val << 2;
			break;
		
		case -1:
			quit = 1;
			break;
	  }
  }

  menu = prevmenu;

  int i;
  for(i = 0; i < PALETTE_SIZE; i++) palette_sync(i, 1);
}
#endif

extern s16 xshift;
extern s16 yshift;
extern s16 xscale;
extern s16 yscale;
extern uint8 overscan;
extern uint8 aspect;
extern uint8 use_480i;
extern uint8 tv_mode;
extern uint8 gc_pal;
extern s16 square[];
extern void viewport_init();
extern void draw_init (void);
extern GXRModeObj *tvmodes[4];

static uint8 palette = 1;
static uint8 old_overscan = 1;

void dispmenu ()
{
	int ret, i;
	int quit = 0;
	int count = aspect ? 7 : 9;
	char items[9][20];
	int prevmenu = menu;

	strcpy (menutitle, "Press B to return");
	menu = 0;

	while (quit == 0)
	{
		sprintf (items[0], "Aspect: %s", aspect ? "ORIGINAL" : "STRETCH");
		if (use_480i == 1) sprintf (items[1], "Render: BILINEAR");
		else if (use_480i == 2) sprintf (items[1], "Render: PROGRESS");
		else sprintf (items[1], "Render: ORIGINAL");
		if (tv_mode == 0) sprintf (items[2], "TV Mode: 60HZ");
		else if (tv_mode == 1) sprintf (items[2], "TV Mode: 50HZ");
		else sprintf (items[2], "TV Mode: 50/60HZ");
		sprintf (items[3], "Overscan: %s", overscan ? " ON" : "OFF");
		if (palette == 0) sprintf (items[4], "Palette: ORIGINAL");
		else if (palette == 1) sprintf (items[4], "Palette:  NORMAL");
		else if (palette == 2) sprintf (items[4], "Palette:  BRIGHT");
		sprintf (items[5], "Center X: %s%02d", xshift < 0 ? "-":"+", abs(xshift));
		sprintf (items[6], "Center Y: %s%02d", yshift < 0 ? "-":"+", abs(yshift));
		sprintf (items[7], "Scale  X:  %02d", xscale);
		sprintf (items[8], "Scale  Y:  %02d", yscale);

		ret = domenu (&items[0], count);

		switch (ret)
		{
			case 0: /*** aspect ratio ***/
				aspect ^= 1;
				if (aspect) overscan = old_overscan;
				else
				{
					old_overscan = overscan;
					overscan = 0;
				}
				count = aspect ? 7 : 9;
				vdp_init();
				viewport_init();
				break;

			case 1: /*** rendering ***/
#ifdef FORCE_EURGB60
				use_480i ^= 1;
#else
				use_480i = (use_480i + 1) % 3;
				if (use_480i == 2)
				{
					/* progressive mode (60hz only) */
					tv_mode = 0;
					gc_pal = 0;
					tvmodes[1]->viTVMode |= VI_PROGRESSIVE;
					tvmodes[1]->xfbMode = VI_XFBMODE_SF;
				}
				else
				{
					/* reset video mode */
					tvmodes[1]->viTVMode &= ~VI_PROGRESSIVE;
					tvmodes[1]->xfbMode = VI_XFBMODE_DF;
				}
#endif
				bitmap.viewport.changed = 1;
				viewport_init();
				break;

			case 2: /*** tv mode ***/
				tv_mode = (tv_mode + 1) % 3;
				if ((tv_mode == 1) || ((tv_mode == 2) && sms.display)) gc_pal = 1;
				else gc_pal = 0;
				viewport_init();
				break;
		
			case 3: /*** overscan ***/
				if (aspect)
				{
					overscan ^= 1;
					vdp_init();
					viewport_init();
				}
				break;

			case 5:	/*** Center X ***/
			case -7:
				if (ret<0) xshift --;
				else xshift ++;
				square[6] = square[3]  =  xscale + xshift;
				square[0] = square[9]  = -xscale + xshift;
				draw_init();
				break;

			case 6:	/*** Center Y ***/
			case -8:
				if (ret<0) yshift --;
				else yshift ++;
				square[4] = square[1]  =  yscale + yshift;
				square[7] = square[10] = -yscale + yshift;
				draw_init();
				break;
			
			case 7:	/*** Scale X ***/
			case -9:
				if (ret<0) xscale --;
				else xscale ++;
				square[6] = square[3]  =  xscale + xshift;
				square[0] = square[9]  = -xscale + xshift;
				draw_init();
				break;

			case 8:	/*** Scale Y ***/
			case -10:
				if (ret<0) yscale --;
				else yscale ++;
				square[4] = square[1]  =  yscale + yshift;
				square[7] = square[10] = -yscale + yshift;
				draw_init();
				break;

			case 4:
				palette = (palette + 1) % 3;
				if (palette == 0)
				{
					sms_cram_expand_table[1] = (4 << 3)  + (1 << 2);
					sms_cram_expand_table[2] = (12 << 3) + (1 << 2);
					sms_cram_expand_table[3] = (22 << 3) + (1 << 2);
				}
				else if (palette == 1)
				{
					sms_cram_expand_table[1] = (5 << 3)  + (1 << 2);
					sms_cram_expand_table[2] = (15 << 3) + (1 << 2);
					sms_cram_expand_table[3] = (27 << 3) + (1 << 2);
				}
				else if (palette == 2)
				{
					sms_cram_expand_table[1] = (6 << 3)  + (1 << 2);
					sms_cram_expand_table[2] = (18 << 3) + (1 << 2);
					sms_cram_expand_table[3] = (31 << 3) + (1 << 2);
				}

				for(i = 0; i < PALETTE_SIZE; i++) palette_sync(i, 1);
				break;

		case -1:
			quit = 1;
			break;
	  }
  }
  menu = prevmenu;
}

/****************************************************************************
 ****************************************************************************/

extern void set_config();
extern int smsromsize;
int fm_type = 1;
int joy_type = 0;

void sysmenu ()
{
  int ret;
  int quit = 0;
  int misccount = 5;
  char miscmenu[5][20];
  int prevmenu = menu;

 strcpy (menutitle, "Press B to return");
		
  if (fm_type == 0) sprintf (miscmenu[0], "FM     -     OFF");
  else if (fm_type == 1) sprintf (miscmenu[0], "FM     - EMU2413");
  else if (fm_type == 2) sprintf (miscmenu[0], "FM     -  YM2413");

  if (option.country == 1) sprintf (miscmenu[1], "Country -    USA");
  else if (option.country == 2) sprintf (miscmenu[1], "Country -    EUR");
  else if (option.country == 3) sprintf (miscmenu[1], "Country -    JAP");
  else sprintf (miscmenu[1], "Country -   AUTO");
			
  if (option.console == 1) sprintf (miscmenu[2], "Console -    SMS");
  else if (option.console == 2) sprintf (miscmenu[2], "Console -   SMS2");
  else if (option.console == 3) sprintf (miscmenu[2], "Console -     GG");
  else if (option.console == 4) sprintf (miscmenu[2], "Console - GG-SMS");
  else sprintf (miscmenu[2], "Console -   AUTO");
			
  sprintf (miscmenu[3], "Joypad -   %s",joy_type ? "TYPEA" : "TYPEB");
  sprintf (miscmenu[4], "SMS Bios -    %s",(bios.enabled & 1) ? "ON" : "OFF");

  menu = 0;

  while (quit == 0)
  {
	  ret = domenu (&miscmenu[0], misccount);

	  switch (ret)
	  {

		case 0:	/*** FM Enabled ***/
			fm_type ++;
			if (fm_type > 2) fm_type = 0;
			if (fm_type == 0)
			{
				sprintf (miscmenu[0], "FM     -     OFF");
				option.fm_enable = 0;
			}
			else if (fm_type == 1)
			{
				sprintf (miscmenu[0], "FM     - EMU2413");
				option.fm_enable = 1;
				option.fm_which = SND_EMU2413;
			}
			else if (fm_type == 2)
			{
				sprintf (miscmenu[0], "FM     -  YM2413");
				option.fm_enable = 1;
				option.fm_which = SND_YM2413;
			}
			system_init();
			break;

		case 1:  /*** Country ***/
			option.country ++;
			if (option.country > 3) option.country = 0;
			if (option.country == 1) sprintf (miscmenu[1], "Country -    USA");
			else if (option.country == 2) sprintf (miscmenu[1], "Country -    EUR");
			else if (option.country == 3) sprintf (miscmenu[1], "Country -    JAP");
			else sprintf (miscmenu[1], "Country -   AUTO");
			set_config();
			system_init();
			viewport_init();
			break;

		case 2:  /*** Console Type ***/
			option.console ++;
			if (option.console > 4) option.console = 0;
			if (option.console == 1) sprintf (miscmenu[2], "Console -    SMS");
			else if (option.console == 2) sprintf (miscmenu[2], "Console -   SMS2");
			else if (option.console == 3) sprintf (miscmenu[2], "Console -     GG");
			else if (option.console == 4) sprintf (miscmenu[2], "Console - GG-SMS");
			else sprintf (miscmenu[2], "Console -   AUTO");
			set_config();
			system_init();
			viewport_init();
			break;

		case 3: /*** invert buttons ***/
			joy_type ^= 1;
			sprintf (miscmenu[3], "Joypad -   %s",joy_type ? "TYPEA" : "TYPEB");
			break;

		case 4: /*** SMS BIOS ***/
			bios.enabled ^= 1;
			sprintf (miscmenu[4], "SMS Bios -    %s",bios.enabled&1 ? "ON" : "OFF");
			break;

		case -1:
			quit = 1;
			break;
	  }
  }

  menu = prevmenu;
}

void optionmenu ()
{
  int ret;
  int quit = 0;
  int misccount = 2;
  char miscmenu[2][20];
  int prevmenu = menu;

  strcpy (menutitle, "Press B to return");
		
  sprintf (miscmenu[0], "System Options");
  sprintf (miscmenu[1], "Display Options");

  menu = 0;

  while (quit == 0)
  {
	  ret = domenu (&miscmenu[0], misccount);

	  switch (ret)
	  {

		case 0:	/*** FM Enabled ***/
			sysmenu();
			break;

		case 1:  /*** Country ***/
			dispmenu();
			break;

		case -1:
			quit = 1;
			break;
	  }
  }

  menu = prevmenu;
}


/****************************************************************************
* Generic Load/Save menu
*
****************************************************************************/
int CARDSLOT = CARD_SLOTB;
int use_SDCARD = 0;
extern int ManageState (int direction);

int statemenu ()
{
  int prevmenu = menu;
  int quit = 0;
  int ret;
  int count = 4;
  char items[4][20];

  strcpy (menutitle, "Press B to return");

  if (use_SDCARD) sprintf(items[0], "Device: SDCARD");
  else sprintf(items[0], "Device:  MCARD");

  if (CARDSLOT == CARD_SLOTA) sprintf(items[1], "Use: SLOT A");
  else sprintf(items[1], "Use: SLOT B");

  sprintf(items[2], "Save State");
  sprintf(items[3], "Load State");
  menu = 2;

  while (quit == 0)
  {
	  ret = domenu (&items[0], count);
	  switch (ret)
	  {
		  case -1:
			  quit = 1;
			  break;
		  case 0:
			  use_SDCARD ^= 1;
			  if (use_SDCARD) sprintf(items[0], "Device: SDCARD");
			  else sprintf(items[0], "Device:  MCARD");
			  break;
		  case 1:
			  CARDSLOT ^= 1;
			  if (CARDSLOT == CARD_SLOTA) sprintf(items[1], "Use: SLOT A");
			  else sprintf(items[1], "Use: SLOT B");
			  break;
		  case 2:
			  if (ManageState(0)) return 1;
			  break;
		  case 3:
			  if (ManageState(1)) return 1;
			  break;
	  }
  }

  menu = prevmenu;
  return 0;
}


/****************************************************************************
 * Load Rom menu
 *
 ****************************************************************************/
extern void OpenDVD ();
extern int OpenSD (uint8 slot);
extern u8 UseSDCARD;

static u8 load_menu = 0;
void loadmenu ()
{
	int ret;
	int quit = 0;
	int count = 3;
	char item[3][20] = {
#ifdef HW_RVL
		{"Load from FRONT SD"},
#else
		{"Load from DVD"},
#endif
		{"Load from SD SLOTA"},
		{"Load from SD SLOTB"}
	};

	menu = load_menu;
	
	while (quit == 0)
	{
		strcpy (menutitle, "Press B to return");
		ret = domenu (&item[0], count);
		switch (ret)
		{
			case -1: /*** Button B ***/
				quit = 1;
				break;
			case 0:	 /*** Load from DVD ***/
#ifndef HW_RVL
				OpenDVD ();
#else
				OpenSD (2);
#endif
				quit = 1;
				break;
			case 1:  /*** Load from SCDARD ***/
			case 2: 
				OpenSD (ret - 1);
				quit = 1;
				break;
		}
	}

	load_menu = menu;
}

/****************************************************************************
 * Main menu
 *
 ****************************************************************************/
extern GXRModeObj *vmode;

void MainMenu ()
{
	GXRModeObj *rmode;
    Mtx p;
	menu = 0;
	int ret;
	int quit = 0;
#if defined(HW_RVL)
	void (*TPreload)() = (void(*)())0x90000020;
	int count = 7;
	char items[7][20] =
#else
	int *psoid = (int *) 0x80001800;
	void (*PSOReload) () = (void (*)()) 0x80001800;
	int count = 8;
	char items[8][20] =
#endif
	{
		{"Play Game"},
		{"Hard Reset"},
		{"Load New Game"},
		{"Emulator Options"},
		{"Savestate Manager"},
#ifdef HW_RVL
		{"TP/Hack Reload"},
#else
		{"Stop DVD Motor"},
		{"SD/PSO Reload"},
#endif
		{"System Reboot"}
	};

	/* Switch to menu default rendering mode (60hz or 50hz, but always 480 lines) */
	VIDEO_Configure (vmode);
	VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();

	while (quit == 0)
	{
		strcpy (menutitle, "");
		ret = domenu (&items[0], count);
		switch (ret)
		{
			case -1:  /*** Button B ***/
			case 0:	/*** Play Game ***/
				if (smsromsize || (bios.enabled == 3))
				{
				   quit = 1;
				}
				break;

			case 1:
				if ((bios.enabled == 3) || smsromsize)
				{
					system_poweron();
					quit = 1;
				}
				break;

			case 2:
				loadmenu();
				menu = 0;
				break;

			case 3:
				optionmenu();
				break;

			case 4:
				if (smsromsize) quit = statemenu();
				break;

#ifdef HW_RVL
			case 5:
				TPreload();
				break;

			case 6:
				SYS_ResetSystem(SYS_RESTART, 0, 0);
				break;
#else
			case 5:
				ShowAction("Stopping DVD Motor ...");
				dvd_motor_off();
				break;

			case 6:
				if (psoid[0] == PSOSDLOADID) PSOReload ();
				break;

			case 7:
				SYS_ResetSystem(SYS_HOTRESET,0,0);
				break;
#endif
		}
	}

	/*** Remove any still held buttons ***/
	while(PAD_ButtonsHeld(0)) VIDEO_WaitVSync();

	/*** Reinitialize current TV mode ***/
	rmode = tvmodes[gc_pal*2 + use_480i];
	rmode->viWidth = aspect ? 720 : 640;
	rmode->viXOrigin = (720 - rmode->viWidth) / 2;
	VIDEO_Configure (rmode);
	VIDEO_ClearFrameBuffer(rmode, xfb[whichfb], COLOR_BLACK);
	VIDEO_Flush();
	VIDEO_WaitVSync();
    VIDEO_WaitVSync();

	/*** Reinitalize GX ***/ 
    GX_SetViewport (0.0F, 0.0F, rmode->fbWidth, rmode->efbHeight, 0.0F, 1.0F);
	GX_SetScissor (0, 0, rmode->fbWidth, rmode->efbHeight);
    f32 yScale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    u16 xfbHeight  = GX_SetDispCopyYScale (yScale);
	GX_SetDispCopySrc (0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopyDst (rmode->fbWidth, xfbHeight);
	GX_SetCopyFilter (rmode->aa, rmode->sample_pattern, use_480i ? GX_TRUE : GX_FALSE, rmode->vfilter);
	GX_SetFieldMode (rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt (rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	guOrtho(p, rmode->efbHeight/2, -(rmode->efbHeight/2), -(rmode->fbWidth/2), rmode->fbWidth/2, 100, 1000);
	GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);
	GX_Flush();

#ifndef HW_RVL
	/*** Stop the DVD from causing clicks while playing ***/
	uselessinquiry ();
#endif
}
