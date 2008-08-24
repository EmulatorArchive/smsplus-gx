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
 * Please put any user menus here! - softdev March 12 2006
 ***************************************************************************/
#include "shared.h"
#include "dvd.h"
#include "config.h"
#include "font.h"
#ifdef HW_RVL
#include "di/di.h"
#endif

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
int domenu (char items[][20], int maxitems, u8 fastmove)
{
  int redraw = 1;
  int quit = 0;
  short p;
  int ret = 0;

  while (quit == 0)
  {
    if (redraw)
    {
      drawmenu (&items[0], maxitems, menu);
      redraw = 0;
    }
    
    p = ogc_input__getMenuButtons();
    
    if (p & PAD_BUTTON_UP)
    {
      redraw = 1;
      menu--;
      if (menu < 0) menu = maxitems - 1;
    }
    else if (p & PAD_BUTTON_DOWN)
    {
      redraw = 1;
      menu++;
      if (menu == maxitems) menu = 0;
    }
    else if (p & PAD_BUTTON_A)
    {
      quit = 1;
      ret = menu;
    }
    else if (p & PAD_BUTTON_B)
    {
      quit = 1;
      ret = -1;
    }

    if (fastmove)
    {
      if (p & PAD_BUTTON_RIGHT)
      {
        quit = 1;
        ret = menu;
      }
      else if (p & PAD_BUTTON_LEFT)
      {
        quit = 1;
        ret = 0 - 2 - menu;
      }
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

/****************************************************************************
* Display options menu
*
****************************************************************************/
static uint8 old_overscan = 1;

void dispmenu ()
{
	s8 ret;
	u8 quit = 0;
	u8 count = option.aspect ? 7 : 9;
	u8 prevmenu = menu;
  int i;
	menu = 0;

	char items[9][20];

	while (quit == 0)
	{
    sprintf (items[0], "Aspect: %s", option.aspect ? "ORIGINAL" : "STRETCH");
		if (option.render == 1) sprintf (items[1], "Render: BILINEAR");
		else if (option.render == 2) sprintf (items[1], "Render: PROGRESS");
		else sprintf (items[1], "Render: ORIGINAL");
		if (option.tv_mode == 0) sprintf (items[2], "TV Mode: 60HZ");
		else if (option.tv_mode == 1) sprintf (items[2], "TV Mode: 50HZ");
		else sprintf (items[2], "TV Mode: 50/60HZ");
		sprintf (items[3], "Overscan: %s", option.overscan ? " ON" : "OFF");
		if (option.palette == 0) sprintf (items[4], "Palette: ORIGINAL");
		else if (option.palette == 1) sprintf (items[4], "Palette:  NORMAL");
		else if (option.palette == 2) sprintf (items[4], "Palette:  BRIGHT");
		sprintf (items[5], "Center X: %s%02d",option. xshift < 0 ? "-":"+", abs(option.xshift));
		sprintf (items[6], "Center Y: %s%02d", option.yshift < 0 ? "-":"+", abs(option.yshift));
		sprintf (items[7], "Scale  X:  %02d", option.xscale);
		sprintf (items[8], "Scale  Y:  %02d", option.yscale);

		ret = domenu (&items[0], count, 1);

		switch (ret)
		{
			case 0: /*** aspect ratio ***/
				option.aspect ^= 1;
				if (option.aspect) option.overscan = old_overscan;
				else
				{
					old_overscan = option.overscan;
					option.overscan = 0;
				}
				count = option.aspect ? 7 : 9;
				vdp_init();
				break;

			case 1: /*** rendering ***/
				option.render = (option.render + 1) % 3;
				if (option.render == 2)
				{
					if (VIDEO_HaveComponentCable())
					{
            /* progressive mode (60hz only) */
            option.tv_mode = 0;
          }
          else
          {
            /* do nothing if component cable is not detected */
            option.render = 0;
          }
				}
				bitmap.viewport.changed = 1;
				break;

			case 2: /*** tv mode ***/
				if (option.render == 2) break; /* 60hz progressive only */
				option.tv_mode = (option.tv_mode + 1) % 3;
				break;
		
			case 3: /*** overscan ***/
				if (option.aspect)
				{
					option.overscan ^= 1;
					vdp_init();
				}
				break;

			case 4:
				option.palette = (option.palette + 1) % 3;
				if (option.palette == 0)
				{
					sms_cram_expand_table[1] = (4 << 3)  + (1 << 2);
					sms_cram_expand_table[2] = (12 << 3) + (1 << 2);
					sms_cram_expand_table[3] = (22 << 3) + (1 << 2);
				}
				else if (option.palette == 1)
				{
					sms_cram_expand_table[1] = (5 << 3)  + (1 << 2);
					sms_cram_expand_table[2] = (15 << 3) + (1 << 2);
					sms_cram_expand_table[3] = (27 << 3) + (1 << 2);
				}
				else if (option.palette == 2)
				{
					sms_cram_expand_table[1] = (6 << 3)  + (1 << 2);
					sms_cram_expand_table[2] = (18 << 3) + (1 << 2);
					sms_cram_expand_table[3] = (31 << 3) + (1 << 2);
				}

				for(i = 0; i < PALETTE_SIZE; i++) palette_sync(i, 1);
				break;

      case 5:	/*** Center X ***/
			case -7:
				if (ret<0) option.xshift --;
				else option.xshift ++;
				break;

			case 6:	/*** Center Y ***/
			case -8:
				if (ret<0) option.yshift --;
				else option.yshift ++;
				break;
			
			case 7:	/*** Scale X ***/
			case -9:
				if (ret<0) option.xscale --;
				else option.xscale ++;
				break;

			case 8:	/*** Scale Y ***/
			case -10:
				if (ret<0) option.yscale --;
				else option.yscale ++;
				break;

		case -1:
			quit = 1;
			break;
	  }
  }
  menu = prevmenu;
}

/****************************************************************************
* System options menu
*
****************************************************************************/
static int fm_type = 1;

void sysmenu ()
{
  s8 ret;
  u8 quit = 0;
  u8 count = 7;
  u8 prevmenu = menu;
  menu = 0;

  char miscmenu[7][20];
  
  if (option.fm_enable)
  {
    fm_type = (option.fm_which == SND_EMU2413) ? 1 : 2;
  }
  else
  {
    fm_type = 0;
  }

  while (quit == 0)
  {
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
        
    sprintf (miscmenu[3], "Sprite Limit: %s", option.spritelimit ? " ON" : "OFF");
	  sprintf (miscmenu[4], "Use BIOS: %s", option.use_bios ? " ON" : "OFF");
		sprintf (miscmenu[5], "GG Extra: %s", option.extra_gg ? " ON" : "OFF");

		if (option.autofreeze == 0) sprintf (miscmenu[6], "Auto FREEZE: SDCARD");
		else if (option.autofreeze == 1) sprintf (miscmenu[6], "Auto FREEZE: MCARD A");
		else if (option.autofreeze == 2) sprintf (miscmenu[6], "Auto FREEZE: MCARD B");
		else sprintf (miscmenu[6], "Auto FREEZE: OFF");

	  ret = domenu (&miscmenu[0], count, 0);

	  switch (ret)
	  {
      case 0:	/*** FM Enabled ***/
        fm_type ++;
        if (fm_type > 2) fm_type = 0;
        if (fm_type == 0)
        {
          option.fm_enable = 0;
        }
        else if (fm_type == 1)
        {
          option.fm_enable = 1;
          option.fm_which = SND_EMU2413;
        }
        else if (fm_type == 2)
        {
          option.fm_enable = 1;
          option.fm_which = SND_YM2413;
        }
        system_init();
        break;

      case 1:  /*** Country ***/
        option.country ++;
        if (option.country > 3) option.country = 0;
        set_config();
        system_init();
        break;

      case 2:  /*** Console Type ***/
        option.console ++;
        if (option.console > 4) option.console = 0;
        set_config();
        system_init();
        break;

		  case 3: /*** sprite flickering ***/
				option.spritelimit ^= 1;
				break;

      case 4: /*** SMS BIOS ***/
        option.use_bios ^= 1;
        
        /* reset BIOS flag */
        bios.enabled &= ~1;
        if (option.use_bios) bios.enabled |= 1;

        if ((bios.enabled == 3) || smsromsize) system_poweron();
        break;

		  case 5: /*** GG Extra mode ***/
				option.extra_gg ^= 1;
        system_init();
				break;

      case 6:	/*** FreezeState autoload/autosave ***/
        option.autofreeze ++;
        if (option.autofreeze > 2) option.autofreeze = -1;
        break;

    case -1:
        quit = 1;
        break;
	  }
  }

  menu = prevmenu;
}

/****************************************************************************
* Controls configuration menu
*
****************************************************************************/
void ctrlmenu ()
{
  s8 ret;
  u8 quit = 0;
  u8 num = 0;
#ifdef HW_RVL
  u8 count = 5;
#else
  u8 count = 4;
#endif
  u8 prevmenu = menu;
  menu = 0;

  strcpy (menutitle, "Press B to return");
#ifdef HW_RVL
  char ctrlmenu[5][20];
  sprintf (ctrlmenu[3], "Set GAMEPAD");
  sprintf (ctrlmenu[4], "Set WIIMOTE");
#else
  char ctrlmenu[4][20];
  sprintf (ctrlmenu[3], "Set GAMEPAD");
#endif

  while (quit == 0)
  {
    if (sms.device[0] == DEVICE_LIGHTGUN) sprintf (ctrlmenu[0], "Port A: PHASER");
    else if (sms.device[0] == DEVICE_PADDLE) sprintf (ctrlmenu[0], "Port A: PADDLE");
    else if (sms.device[0] == DEVICE_NONE) sprintf (ctrlmenu[0], "Port A: NONE");
    else if (sms.device[0] == DEVICE_PAD2B) sprintf (ctrlmenu[0], "Port A: GAMEPAD");
    if (sms.device[1] == DEVICE_LIGHTGUN) sprintf (ctrlmenu[1], "Port B: PHASER");
    else if (sms.device[1] == DEVICE_PADDLE) sprintf (ctrlmenu[1], "Port B: PADDLE");
    else if (sms.device[1] == DEVICE_NONE) sprintf (ctrlmenu[1], "Port B: NONE");
    else if (sms.device[1] == DEVICE_PAD2B) sprintf (ctrlmenu[1], "Port B: GAMEPAD");
    sprintf (ctrlmenu[2], "Configure Player: %d", num+1);
	  ret = domenu (&ctrlmenu[0], count, 0);

	  switch (ret)
	  {
      case 0: /*** INPUT PORT A ***/
        sms.device[0] = (sms.device[0] + 1) % 4;
        break;

      case 1: /*** INPUT PORT B ***/
        sms.device[1] = (sms.device[1] + 1) % 4;
        break;

      case 2:	/*** Controller nr. ***/
			  num ^= 1;
			  break;

		  case 3:  /*** Gamepad ***/
			  ogc_input__config(num, 0);
			  break;

#ifdef HW_RVL
		  case 4:  /*** Wiimote ***/
        ogc_input__config(num, 1);
        break;
#endif

		  case -1:
        quit = 1;
        break;
	  }
  }

  menu = prevmenu;
}

/****************************************************************************
* Option menu
*
****************************************************************************/
void optionmenu ()
{
  s8 ret;
  u8 quit = 0;
  u8 count = 3;
  u8 prevmenu = menu;
  menu = 0;

  char miscmenu[3][20];
  strcpy (menutitle, "Press B to return");
  sprintf (miscmenu[0], "System Options");
  sprintf (miscmenu[1], "Display Options");
  sprintf (miscmenu[2], "Controls Options");

  while (quit == 0)
  {
	  ret = domenu (&miscmenu[0], count, 0);
    switch (ret)
	  {
      case 0:	/*** FM Enabled ***/
        sysmenu();
        break;

      case 1:  /*** Country ***/
        dispmenu();
        break;

      case 2:  /*** Controllers ***/
        ctrlmenu();
        break;

      case -1:
        quit = 1;
        break;
    }
  }

  /* save config file */
  config_save();

  menu = prevmenu;
}


/****************************************************************************
* Generic Load/Save menu
*
****************************************************************************/
static u8 device = 0;

int loadsavemenu ()
{
  u8 prevmenu = menu;
  u8 quit = 0;
  s8 ret;
  u8 count = 3;
  menu = 2;

  char items[3][20];
  strcpy (menutitle, "Press B to return");
  sprintf(items[1], "Save State");
  sprintf(items[2], "Load State");

  while (quit == 0)
  {
    if (device == 0) sprintf(items[0], "Device: SDCARD");
    else if (device == 1) sprintf(items[0], "Device: MCARD A");
    else if (device == 2) sprintf(items[0], "Device: MCARD B");

	  ret = domenu (&items[0], count, 0);

	  switch (ret)
	  {
		  case -1:
			  quit = 1;
			  break;

      case 0:
        device = (device + 1)%3;
			  break;

			case 1:
			case 2:
				quit = ManageState(ret-1, device);
        if (quit) return 1;
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
static u8 load_menu = 0;

void loadmenu ()
{
	int ret;
	int quit = 0;
  int count = 4;
  char item[4][20] = {
		{"Load Recent"},
		{"Load from SDCARD"},
		{"Load from DVD"},
    {"Stop DVD Motor"}
	};

	menu = load_menu;
	
	while (quit == 0)
	{
		strcpy (menutitle, "Press B to return");
		ret = domenu (&item[0], count, 0);
		switch (ret)
		{
			case -1: /*** Button B ***/
				quit = 1;
				break;

			case 0: /*** Load Recent ***/
				quit = OpenHistory();
				break;

			case 1:  /*** Load from SCDARD ***/
				quit = OpenSD();
				break;

      case 2:	 /*** Load from DVD ***/
  			quit = OpenDVD();
        break;
  
      case 3:  /*** Stop DVD Disc ***/
        dvd_motor_off();
				break;
    }
	}

	load_menu = menu;
}

/****************************************************************************
 * Main Menu
 *
 ****************************************************************************/
void MainMenu ()
{
	s8 ret;
	u8 quit = 0;
	menu = 0;
	u8 count = 7;
	char items[7][20] =
	{
		{"Play Game"},
		{"Hard Reset"},
		{"Load New Game"},
		{"Emulator Options"},
		{"Savestate Manager"},
		{"Return to Loader"},
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
		ret = domenu (&items[0], count,0);

    switch (ret)
		{
			case -1:  /*** Button B ***/
			case 0:	/*** Play Game ***/
				if (smsromsize) quit = 1;
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
				quit = loadsavemenu();
				break;

			case 5:
        memfile_autosave();
        VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
        VIDEO_Flush();
        VIDEO_WaitVSync();
#ifdef HW_RVL
        DI_Close();
#endif
        exit(0);
        break;

			case 6:
        memfile_autosave();
        VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
        VIDEO_Flush();
        VIDEO_WaitVSync();
#ifdef HW_RVL
        DI_Close();
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
#else
				SYS_ResetSystem(SYS_HOTRESET,0,0);
#endif
				break;
		}
	}

	/*** Remove any still held buttons ***/
  while (PAD_ButtonsHeld(0))  PAD_ScanPads();
#ifdef HW_RVL
  while (WPAD_ButtonsHeld(0)) WPAD_ScanPads();
#endif

	/*** Reinitalize GX ***/
  ogc_video__reset();

#ifndef HW_RVL
	/*** Stop the DVD from causing clicks while playing ***/
	uselessinquiry ();
#endif
}
