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

	/*** Look for up ***/
    if ((p & PAD_BUTTON_UP) || (a > 70))
	{
	  redraw = 1;
	  menu--;
      if (menu < 0) menu = maxitems - 1;
	}

	/*** Look for down ***/
    if ((p & PAD_BUTTON_DOWN) || (a < -70))
	{
	  redraw = 1;
	  menu++;
      if (menu == maxitems) menu = 0;
	}

    if ((p & PAD_BUTTON_A) || (b > 40) || (p & PAD_BUTTON_RIGHT))
	{
	  quit = 1;
	  ret = menu;
	}
	  
	if ((b < -40) || (p & PAD_BUTTON_LEFT))
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
 * Option menu
 *
 ****************************************************************************/
extern s16 square[];
extern int oldvwidth, oldvheight;
extern int smsromsize;
extern int loadsmsrom ();
int fm_type = 0;
int joy_type = 0;

void optionmenu ()
{
  int ret;
  int quit = 0;
  int misccount = 9;
  char miscmenu[9][20];
  int prevmenu = menu;
  uint8 reload = 0;

  strcpy (menutitle, "Emulator Options");

  /*** Capture running values ***/
  sprintf (miscmenu[0], "ScaleX -      %02d", square[3]);
  sprintf (miscmenu[1], "ScaleY -      %02d", square[1]);
  
  if (fm_type == 0) sprintf (miscmenu[2], "FM     -     OFF");
  else if (fm_type == 1) sprintf (miscmenu[2], "FM     - EMU2413");
  else if (fm_type == 2) sprintf (miscmenu[2], "FM     -  YM2413");

  if (sms_option.display == DISPLAY_PAL) sprintf (miscmenu[3], "CPU    -     PAL");
  else if (sms_option.display == DISPLAY_NTSC) sprintf (miscmenu[3], "CPU    -    NTSC");
  else sprintf (miscmenu[3], "CPU    -    AUTO");
  
  if (sms_option.country == TERRITORY_EXPORT) sprintf (miscmenu[4], "Region -  EXPORT");
  else if (sms_option.country == TERRITORY_DOMESTIC) sprintf (miscmenu[4], "Region -   JAPAN");
  else sprintf (miscmenu[4], "Region -    AUTO");

  if (sms_option.console == 0) sprintf (miscmenu[5], "Type   -     SMS");
  else if (sms_option.console == 1) sprintf (miscmenu[5], "Type   -   SMS-J");
  else if (sms_option.console == 2) sprintf (miscmenu[5], "Type   -    SMS2");
  else if (sms_option.console == 3) sprintf (miscmenu[5], "Type   -      GG");
  else if (sms_option.console == 4) sprintf (miscmenu[5], "Type   -  GG-SMS");
  else sprintf (miscmenu[5], "Type   -    AUTO");
  
  if (sms_option.codies == 0) sprintf (miscmenu[6], "Mapper -    NONE");
  else if (sms_option.codies == 1) sprintf (miscmenu[6], "Mapper -    SEGA");
  else if (sms_option.codies == 2) sprintf (miscmenu[6], "Mapper -  CODIES");
  else sprintf (miscmenu[6], "Mapper -    AUTO");
  
  sprintf (miscmenu[7], "Joypad -   %s",joy_type ? "TYPEA" : "TYPEB");
  strcpy (miscmenu[8], "Return to previous");

  menu = 0;

  while (quit == 0)
  {
	  ret = domenu (&miscmenu[0], misccount);

	  switch (ret)
	  {
		case 0:	/*** Scale X ***/
		case -2:
		    if (ret<0) square[3] -= 2;
			else square[3] += 2;
		    if (square[3] < 40) square[3] = 80;
			if (square[3] > 80) square[3] = 40;
			square[6] = square[3];
			square[0] = square[9] = -square[3];
			oldvwidth = -1;
			sprintf (miscmenu[0], "Scale X:      %02d", square[3]);
			break;

		case 1:	/*** Scale Y ***/
		case -3:
			if (ret<0) square[1] -= 2;
			else square[1] += 2;
			if (square[1] < 30) square[1] = 60;
			if (square[1] > 60) square[1] = 30;
			square[4] = square[1];
			square[7] = square[10] = -square[1];
			oldvheight = -1;
			sprintf (miscmenu[1], "Scale Y:      %02d", square[1]);
			break;

		case 2:	/*** FM Enabled ***/
			fm_type ++;
			if (fm_type > 2) fm_type = 0;
			if (fm_type == 0)
			{
				sprintf (miscmenu[2], "FM     -     OFF");
				sms_option.fm_enable = 0;
			}
			else if (fm_type == 1)
			{
				sprintf (miscmenu[2], "FM     - EMU2413");
				sms_option.fm_enable = 1;
				sms_option.fm_which = SND_EMU2413;
			}
			else if (fm_type == 2)
			{
				sprintf (miscmenu[2], "FM     -  YM2413");
				sms_option.fm_enable = 1;
				sms_option.fm_which = SND_YM2413;
			}
			reload = 1;
			break;

		case 3:  /*** DISPLAY Speed ***/
			sms_option.display ++;
			if (sms_option.display > 1) sms_option.display = -1;
			if (sms_option.display == DISPLAY_PAL) sprintf (miscmenu[3], "CPU    -     PAL");
			else if (sms_option.display == DISPLAY_NTSC) sprintf (miscmenu[3], "CPU    -    NTSC");
			else sprintf (miscmenu[3], "CPU    -    AUTO");
			reload = 1;
			break;

		case 4:  /*** Region ***/
			sms_option.country ++;
			if (sms_option.country > 1) sms_option.country = -1;
			if (sms_option.country == TERRITORY_EXPORT) sprintf (miscmenu[4], "Region -  EXPORT");
			else if (sms_option.country == TERRITORY_DOMESTIC) sprintf (miscmenu[4], "Region -   JAPAN");
			else sprintf (miscmenu[4], "Region -    AUTO");
			reload = 1;
			break;

		case 5:  /*** Console Type Detection ***/
			sms_option.console ++;
			if (sms_option.console > 4) sms_option.console = -1;
			if (sms_option.console == 0) sprintf (miscmenu[5], "Type   -     SMS");
			else if (sms_option.console == 1) sprintf (miscmenu[5], "Type   -   SMS-J");
			else if (sms_option.console == 2) sprintf (miscmenu[5], "Type   -    SMS2");
			else if (sms_option.console == 3) sprintf (miscmenu[5], "Type   -      GG");
			else if (sms_option.console == 4) sprintf (miscmenu[5], "Type   -  GG-SMS");
			else sprintf (miscmenu[5], "Type   -    AUTO");
			reload = 1;
			break;

		case 6: /*** Mapper Type ***/
			sms_option.codies ++;
			if (sms_option.codies > 2) sms_option.codies = -1;
			if (sms_option.codies == 0) sprintf (miscmenu[6], "Mapper -    NONE");
			else if (sms_option.codies == 1) sprintf (miscmenu[6], "Mapper -    SEGA");
			else if (sms_option.codies == 2) sprintf (miscmenu[6], "Mapper -  CODIES");
			else sprintf (miscmenu[6], "Mapper -    AUTO");
			reload = 1;
			break;
			
		case 7: /*** invert buttons ***/
			joy_type ^= 1;
			sprintf (miscmenu[7], "Joypad -   %s",joy_type ? "TYPEA" : "TYPEB");
			break;
		
		case -1:
		case 8:
			quit = 1;
			break;
	  }
  }

  if (smsromsize && reload)
  {
	  loadsmsrom ();
	  system_init ();
	  system_poweron ();
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
  int count = 5;
  char items[5][20];

  strcpy (menutitle, "");
  menu = 2;

  if (use_SDCARD) sprintf(items[0], "Device: SDCARD");
  else sprintf(items[0], "Device:  MCARD");

  if (CARDSLOT == CARD_SLOTA) sprintf(items[1], "Use: SLOT A");
  else sprintf(items[1], "Use: SLOT B");

  sprintf(items[2], "Save State");
  sprintf(items[3], "Load State");
  sprintf(items[4], "Return to previous");

  while (quit == 0)
  {
	  ret = domenu (&items[0], count);
	  switch (ret)
	  {
		  case -1:
		  case  4:
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
extern int OpenSD ();
extern u8 UseSDCARD;

void loadmenu ()
{
	int prevmenu = menu;
	int ret;
	int quit = 0;
	int count = 3;
	char item[3][20] = {
		{"Load from DVD"},
		{"Load from SDCARD"},
		{"Return to previous"}
	};

	menu = UseSDCARD ? 1 : 0;
	
	while (quit == 0)
	{
		strcpy (menutitle, "");
		ret = domenu (&item[0], count);
		switch (ret)
		{
			case -1: /*** Button B ***/
			case 2:  /*** Quit ***/
				quit = 1;
				menu = prevmenu;
				break;
			case 0:	 /*** Load from DVD ***/
				OpenDVD ();
				quit = 1;
				break;
			case 1:  /*** Load from SCDARD ***/
				OpenSD ();
				quit = 1;
				break;
		}
	}
}

/****************************************************************************
 * Main menu
 *
 ****************************************************************************/
void MainMenu ()
{
  menu = 0;
  int ret;
  int quit = 0;
  int *psoid = (int *) 0x80001800;
  void (*PSOReload) () = (void (*)()) 0x80001800; 	/*** Stock PSO/SD Reload call ***/
  int count = 7;
  char items[7][20] = {
	{"Play Game"},
	{"Reset Game"},
	{"Load New Game"},
	{"Emulator Options"},
	{"Savestate Manager"},
	{"Stop DVD Motor"},
	{"System Reboot"}
  };

  while (quit == 0)
  {
	strcpy (menutitle, "");
	ret = domenu (&items[0], count);
    switch (ret)
	{
	  case -1:  /*** Button B ***/
      case 0:	/*** Play Game ***/
	    quit = 1;
	    break;
	  case 1:
		system_reset();
		quit = 1;
		break;
	  case 2:
        loadmenu();
        break;
	  case 3:
 		optionmenu();
		break;
      case 4:
		quit = statemenu();
		break;
      case 5:
		  ShowAction("Stopping DVD Motor ...");
		  dvd_motor_off();
		break;
      case 6:
		if (psoid[0] == PSOSDLOADID) PSOReload ();
		else SYS_ResetSystem(SYS_HOTRESET,0,FALSE);
	    break;
	}
  }

  /*** Remove any still held buttons ***/
  while(PAD_ButtonsHeld(0)) VIDEO_WaitVSync();

  /*** Stop the DVD from causing clicks while playing ***/
  uselessinquiry ();
}
