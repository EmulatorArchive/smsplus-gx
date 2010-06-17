/****************************************************************************
 *  menu.c
 *
 *  SMS Plus GX menu
 *
 *  code by Softdev (March 2006), Eke-Eke (2007,2008)
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

#include "shared.h"
#include "dvd.h"
#include "font.h"
#include "file_dvd.h"
#include "file_fat.h"
#include "filesel.h"

#ifdef HW_RVL
#include <errno.h>
#include <network.h>
#include <smb.h>
#include <wiiuse/wpad.h>
#include <di/di.h>
#include "preferences.h"
static int networkInit;
#endif

static char menutitle[60] = { "" };
static int menu = 0;

/***************************************************************************
 * drawmenu
 *
 * As it says, simply draws the menu with a highlight on the currently
 * selected item :)
 ***************************************************************************/
void drawmenu (char items[][25], int maxitems, int selected)
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
int domenu (char items[][25], int maxitems, u8 fastmove)
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

    if (p & PAD_BUTTON_A)
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
  char miscmenu[6][25];
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

void dispmenu ()
{
  s8 ret;
  u8 quit = 0;
  u8 count = option.aspect ? 8 : 10;
  u8 prevmenu = menu;
  menu = 0;

  char items[10][25];

  while (quit == 0)
  {
    strcpy (menutitle, option.aspect ? "Press B to return":"");
    sprintf (items[0], "Aspect: %s", option.aspect ? "ORIGINAL" : "STRETCHED");
    if (option.render == 1) sprintf (items[1], "Render: INTERLACED");
    else if (option.render == 2) sprintf (items[1], "Render: PROGRESSIVE");
    else sprintf (items[1], "Render: ORIGINAL");
    if (option.tv_mode == 0) sprintf (items[2], "TV Mode: 60HZ");
    else if (option.tv_mode == 1) sprintf (items[2], "TV Mode: 50HZ");
    else sprintf (items[2], "TV Mode: 50/60HZ");
    sprintf (items[3], "Bilinear Filter: %s", option.bilinear ? " ON" : "OFF");
    if (option.ntsc == 1) sprintf (items[4], "NTSC Filter: COMPOSITE");
    else if (option.ntsc == 2) sprintf (items[4], "NTSC Filter: S-VIDEO");
    else if (option.ntsc == 3) sprintf (items[4], "NTSC Filter: RGB");
    else sprintf (items[4], "NTSC Filter: OFF");
    sprintf (items[5], "Borders: %s", option.overscan ? " ON" : "OFF");
    sprintf (items[6], "Center X: %s%02d",option. xshift < 0 ? "-":"+", abs(option.xshift));
    sprintf (items[7], "Center Y: %s%02d", option.yshift < 0 ? "-":"+", abs(option.yshift));
    sprintf (items[8], "Scale  X:  %02d", option.xscale);
    sprintf (items[9], "Scale  Y:  %02d", option.yscale);
	  sprintf (items[9], "Scale  Y:  %02d", option.yscale);

    ret = domenu (&items[0], count, 1);

    switch (ret)
    {
      case 0: /*** config.aspect ratio ***/
        option.aspect ^= 1;
        count = option.aspect ? 8 : 10;
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
        break;

      case 2: /*** tv mode ***/
        if (option.render == 2) break; /* 60hz progressive only */
        option.tv_mode = (option.tv_mode + 1) % 3;
        break;
    
      case 3: /*** bilinear filtering ***/
        option.bilinear ^= 1;
        break;

      case 4: /*** NTSC filter ***/
        option.ntsc ++;
        if (option.ntsc > 3) option.ntsc = 0;
        break;
        
      case 5: /*** overscan emulation ***/
        option.overscan ^= 1;
        vdp_init();
        break;

      case 6:  /*** Center X ***/
      case -8:
        if (ret<0) option.xshift --;
        else option.xshift ++;
        break;

      case 7:  /*** Center Y ***/
      case -9:
        if (ret<0) option.yshift --;
        else option.yshift ++;
        break;
      
      case 8:  /*** Scale X ***/
      case -10:
        if (ret<0) option.xscale --;
        else option.xscale ++;
        break;

      case 9:  /*** Scale Y ***/
      case -11:
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
void sysmenu ()
{
  char items[9][25];
  s8 ret;
  u8 quit = 0;
  u8 count = 9;
  u8 prevmenu = menu;
  int i;
  menu = 0;
  
  while (quit == 0)
  {
    strcpy (menutitle, "Press B to return");
      
    if (option.fm == 0) sprintf (items[0], "FM     -     OFF");
    else if (option.fm == 1) sprintf (items[0], "FM     - EMU2413");
    else if (option.fm == 2) sprintf (items[0], "FM     -  YM2413");

    if (option.country == 1) sprintf (items[1], "Country -    USA");
    else if (option.country == 2) sprintf (items[1], "Country -    EUR");
    else if (option.country == 3) sprintf (items[1], "Country -    JAP");
    else sprintf (items[1], "Country -   AUTO");
        
    if (option.console == 1) sprintf (items[2], "Console -    SMS");
    else if (option.console == 2) sprintf (items[2], "Console -   SMS2");
    else if (option.console == 3) sprintf (items[2], "Console -     GG");
    else if (option.console == 4) sprintf (items[2], "Console - GG-SMS");
    else if (option.console == 5) sprintf (items[2], "Console - SG1000");
    else if (option.console == 6) sprintf (items[2], "Console - COLECO");
    else sprintf (items[2], "Console -   AUTO");
        
    sprintf (items[3], "Sprite Limit: %s", option.spritelimit ? " ON" : "OFF");

    if (option.sms_pal == 0) sprintf (items[4], "SMS Palette: ORIGINAL");
    else if (option.sms_pal == 1) sprintf (items[4], "SMS Palette:  NORMAL");
    else if (option.sms_pal == 2) sprintf (items[4], "SMS Palette:  BRIGHT");

    if (option.tms_pal == 0) sprintf (items[5], "TMS Palette: TYPE1");
    else if (option.tms_pal == 1) sprintf (items[5], "TMS Palette:  TYPE2");
    else if (option.tms_pal == 2) sprintf (items[5], "TMS Palette:  TYPE3");

    sprintf (items[6], "SMS BIOS: %s", option.use_bios ? " ON" : "OFF");

    sprintf (items[7], "GG Extra: %s", option.extra_gg ? " ON" : "OFF");

    if (option.autofreeze == 0) sprintf (items[8], "Auto FREEZE: FAT");
    else if (option.autofreeze == 1) sprintf (items[8], "Auto FREEZE: MCARD A");
    else if (option.autofreeze == 2) sprintf (items[8], "Auto FREEZE: MCARD B");
    else sprintf (items[8], "Auto FREEZE: OFF");

    ret = domenu (&items[0], count, 0);

    switch (ret)
    {
      case 0:  /*** FM chip emulation ***/
        option.fm = (option.fm + 1) % 3;
        if ((bios.enabled == 3) || smsromsize)
        {
          set_config();
          system_init();
        }
        break;

      case 1:  /*** Console Region ***/
        option.country  = (option.country + 1) % 4;
        if ((bios.enabled == 3) || smsromsize)
        {
          set_config();
          system_init();
        }
        break;

      case 2:  /*** Console Type ***/
        option.console = (option.console + 1) % 7;
        if ((bios.enabled == 3) || smsromsize) 
        {
          set_config();
          system_poweron();
        }
        break;

      case 3: /*** Sprite flickering ***/
        option.spritelimit ^= 1;
        break;

      case 4: /*** SMS palette intensity ***/
        option.sms_pal = (option.sms_pal + 1) % 3;
        if (option.sms_pal == 0)
        {
          sms_cram_expand_table[1] = (4 << 3)  + (1 << 2);
          sms_cram_expand_table[2] = (12 << 3) + (1 << 2);
          sms_cram_expand_table[3] = (22 << 3) + (1 << 2);
        }
        else if (option.sms_pal == 1)
        {
          sms_cram_expand_table[1] = (5 << 3)  + (1 << 2);
          sms_cram_expand_table[2] = (15 << 3) + (1 << 2);
          sms_cram_expand_table[3] = (27 << 3) + (1 << 2);
        }
        else if (option.sms_pal == 2)
        {
          sms_cram_expand_table[1] = (6 << 3)  + (1 << 2);
          sms_cram_expand_table[2] = (18 << 3) + (1 << 2);
          sms_cram_expand_table[3] = (31 << 3) + (1 << 2);
        }
        for(i = 0; i < PALETTE_SIZE; i++)
          palette_sync(i);
        break;

     case 5: /*** TMS9918 palette intensity ***/
        option.tms_pal = (option.tms_pal + 1) % 3;
        for(i = 0; i < PALETTE_SIZE; i++) palette_sync(i);
        break;

     case 6: /*** SMS BIOS support ***/
        option.use_bios ^= 1;

        /* enable BIOS on SMS only */
        bios.enabled &= 2;
        if (IS_SMS) bios.enabled |= option.use_bios;
        if ((bios.enabled == 3) || smsromsize)
        {
          system_poweron();
        }
        break;

      case 7: /*** GG screen extra mode ***/
        option.extra_gg ^= 1;
        if ((bios.enabled == 3) || smsromsize)
        {
          system_init();
        }
        break;

      case 8:  /*** Auto SaveState ***/
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
  char ctrlmenu[5][25];
  sprintf (ctrlmenu[3], "Set GAMEPAD");
  sprintf (ctrlmenu[4], "Set WIIMOTE");
#else
  char ctrlmenu[4][25];
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

      case 2:  /*** Controller nr. ***/
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
 * SMB Settings Menu
 *
 ****************************************************************************/
#ifdef HW_RVL

void initNetwork()
{
	ShowAction("initializing network...");
	while (net_init() == -EAGAIN);
	char myIP[16];

	if (if_config(myIP, NULL, NULL, true) < 0) 
	{
		WaitPrompt("failed to init network interface\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		networkInit = 1;
		if ((strlen(SMSSettings.share)) > 0 && (strlen(SMSSettings.ip) > 0))
		{
			if (!smbInit(SMSSettings.user, SMSSettings.pwd, SMSSettings.share, SMSSettings.ip)) 
			{
				printf("failed to connect to SMB share\n");
				exit(EXIT_FAILURE);
			}
		}
		else
			WaitPrompt("Wrong Parameters - Check your Settings.xml");
	}
}

void closeNetwork()
{
	if(networkInit) smbClose("smb");
	networkInit = false;
} 

 
int smbmenu ()
{
  int prevmenu = menu;
  int quit = 0;
  int ret;
  int count = 5;
  char items[5][25];

  menu = 1;

  while (quit == 0)
  {
    sprintf (items[0], "IP: %s", SMSSettings.ip);
    sprintf (items[1], "Share: %s", SMSSettings.share);
	sprintf (items[2], "Username: %s", SMSSettings.user);
    sprintf (items[3], "Password: %s", SMSSettings.pwd);	
	sprintf (items[4],"Return to previous");


    ret = domenu (&items[0], count, 0);
    switch (ret)
    {
      case -1:
	  case 4:
	    quit = 1;
        break;

      case 0:
	  // open OnScreenKeyboard for user Settings in future versions...
      case 1:
	  case 2:
	  case 3:	  
        break;

    }
  }
  menu = prevmenu;
  return 0;
}
#endif

/****************************************************************************
 * Main Option menu
*
****************************************************************************/
void optionmenu ()
{
  s8 ret;
  u8 quit = 0;
  u8 count = 3;
  u8 prevmenu = menu;
  menu = 0;

#ifdef HW_RVL
  char miscmenu[4][25];
  count = 4;
#else
  char miscmenu[3][25];
#endif
  strcpy (menutitle, "Press B to return");
  sprintf (miscmenu[0], "System Options");
  sprintf (miscmenu[1], "Display Options");
  sprintf (miscmenu[2], "Controls Options");
  sprintf (miscmenu[3], "SMB Settings");

  while (quit == 0)
  {
    ret = domenu (&miscmenu[0], count, 0);
    switch (ret)
    {
      case 0:  /*** FM Enabled ***/
        sysmenu();
        break;

      case 1:  /*** Country ***/
        dispmenu();
        break;

      case 2:  /*** Controllers ***/
        ctrlmenu();
        break;
#ifdef HW_RVL		
	  case 3:  /*** SMB ***/
		smbmenu();
		break;
#endif
      case -1:
        quit = 1;
        break;
    }
  }

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

  char items[3][25];
  strcpy (menutitle, "Press B to return");
  sprintf(items[1], "Save State");
  sprintf(items[2], "Load State");

  while (quit == 0)
  {
    if (device == 0) sprintf(items[0], "Device: FAT");
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
extern char rom_filename[MAXJOLIET];
static u8 load_menu = 0;
static u8 dvd_on = 0;

int loadmenu ()
{
  int prevmenu = menu;
  int ret,count,size;
  int quit = 0;
#ifdef HW_RVL
  char item[6][25] = {
    {"Load Recent"},
    {"Load from SD"},
    {"Load from USB"},
    {"Load from DVD"},
	{"Load from SMB"},
    {"Stop DVD Motor"}
  };
#else
    char item[4][25] = {
    {"Load Recent"},
    {"Load from SD"},
    {"Load from DVD"},
    {"Stop DVD Motor"}
  };
#endif

  menu = load_menu;
  
  while (quit == 0)
  {
#ifdef HW_RVL
    count = 5 + dvd_on;
#else
    count = 3 + dvd_on;
#endif
    strcpy (menutitle, "Press B to return");
    ret = domenu (&item[0], count, 0);
    switch (ret)
    {
      /*** Button B ***/
      case -1: 
        quit = 1;
        break;

      /*** Load from DVD ***/
#ifdef HW_RVL
      case 3:
#else
      case 2:
#endif
        load_menu = menu;
        size = DVD_Open(smsrom);
        if (size)
        {
          dvd_on = 1;
          memfile_autosave();
          smsromsize = size;
          load_rom("");
          system_poweron ();
          sprintf(rom_filename,"%s",filelist[selection].filename);
          rom_filename[strlen(rom_filename) - 4] = 0;
          memfile_autoload();
          return 1;
        }
        break;
  
 
#ifdef HW_RVL

      /*** Load from SMB Share ***/
      case 4:
  
        if (!networkInit) initNetwork();
        load_menu = menu;
        size = FAT_Open(ret,smsrom);
        if (size)
        {
          memfile_autosave();
          smsromsize = size;
          load_rom ("");
          system_poweron ();
          sprintf(rom_filename,"%s",filelist[selection].filename);
          rom_filename[strlen(rom_filename) - 4] = 0;
          memfile_autoload();
          return 1;
        }
        break;

       /*** Stop DVD Disc ***/
      case 5:  
#else
      case 3:
#endif
        dvd_motor_off();
        dvd_on = 0;
        menu = load_menu;
        break;

      /*** Load from FAT device ***/
      default:
        load_menu = menu;
        size = FAT_Open(ret,smsrom);
        if (size)
        {
          memfile_autosave();
          smsromsize = size;
          load_rom ("");
          system_poweron ();
          sprintf(rom_filename,"%s",filelist[selection].filename);
          rom_filename[strlen(rom_filename) - 4] = 0;
          memfile_autoload();
          return 1;
        }
        break;
    }
  }

  menu = prevmenu;
  return 0;
}

/****************************************************************************
 * Main Menu
 *
 ****************************************************************************/
void MainMenu ()
{
  menu = 0;
  int ret;
  int quit = 0;
  int count = 7;
  char items[7][25] =
  {
    {"Play Game"},
    {"Hard Reset"},
    {"Load New Game"},
    {"Savestate Manager"},
    {"Emulator Options"},
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
      case 0:  /*** Play Game ***/
        if (smsromsize) quit = 1;
        break;

      case 1: /*** Emulator Reset ***/
        if ((bios.enabled == 3) || smsromsize)
        {
          system_poweron();
          quit = 1;
        }
        break;

      case 2:  /*** Load ROM Menu ***/
            quit = loadmenu();
        break;

      case 3:   /*** SaveState Manager ***/
        quit = loadsavemenu();
        break;

      case 4:
	    optionmenu();
        break;

      case 5:
        memfile_autosave();
        system_shutdown();
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
        system_shutdown();
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

  /*** Reinitialize GX ***/
  VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  VIDEO_WaitVSync();

#ifndef HW_RVL
  /*** Stop the DVD from causing clicks while playing ***/
  uselessinquiry ();
#endif
}
