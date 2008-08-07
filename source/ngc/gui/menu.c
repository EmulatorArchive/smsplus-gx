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


/* basically, this is copying image data into the framebuffer */
#ifdef USE_NEWMENU
#include "gfx.h"
typedef struct 
{
  u8 *data;
  u32 size;
} t_buttons;

s8 menu = 0;
u8 raw_data[14400] ATTRIBUTE_ALIGN (32);

void drawbutton(int xoffset, int yoffset, int width, int height)
{
  int rows, cols;
  int fboffset = (xoffset /2 ) + (yoffset * 320);
  int i = 0;
  for (rows = 0; rows < height; rows++)
  {
    for (cols = 0; cols < (width /2); cols++)
    {
      xfb[whichfb][fboffset + cols] = (raw_data[i] << 24) | (raw_data[i+1] << 16) | (raw_data[i+2] << 8) | raw_data[i+3];
      i+=4;
    }
    fboffset += 320;
  }
}	

/* draw each button menu */
void drawmenu (t_buttons items[], u8 maxitems, u8 selected, u8 draw_column)
{
  int i,num,xoffset,yoffset,width;
  unsigned long outbytes;
  ClearScreen ();

  yoffset = 116 + ((308 - (maxitems*40 - 10))/2);

  /* two-column menu */
  if (draw_column)
  {
    for (i = 0; i < maxitems; i++)
    {
      /* first column */
      num = i * 3;
      outbytes = 18000;
      uncompress ((Bytef *)(&raw_data[0]), &outbytes, (Bytef *)(items[num].data), items[num].size);
      drawbutton(196,yoffset,100,30);

      /* second column */
      num = (i * 3) + 1;
      xoffset = 316;
      width = 120;
      if (i == selected)
      {
        num += 1;
        xoffset = 304;
        width = 140;
      }

      if (items[num].data != NULL)
      {
        outbytes = 18000;
        uncompress ((Bytef *)(&raw_data[0]), &outbytes, (Bytef *)(items[num].data), items[num].size);
        drawbutton(xoffset,yoffset,width,30);
      }
      yoffset += 40;
    }
  }
  else
  {
    /* single column menu */
    for (i = 0; i < maxitems; i++)
    {
      num = i * 2;
      xoffset = 210;
      width = 220;
      if (i == selected)
      {
        num += 1;
        xoffset = 200;
        width = 240;
      }

      outbytes = 18000;
      uncompress ((Bytef *)(&raw_data[0]), &outbytes, (Bytef *)(items[num].data), items[num].size);
      drawbutton(xoffset,yoffset,width,30);
      yoffset += 40;
    }
  }

  SetScreen ();
}

int domenu (t_buttons items[], int maxitems, u8 fast)
{
  int redraw = 1;
  int quit = 0;
  short p;
  int ret = 0;

  while (quit == 0)
  {
    if (redraw)
	  {
	    drawmenu (items, maxitems, menu, draw_column);
	    redraw = 0;
	  }

    p = ogc_input__getMenuButtons();

    if (p & PAD_BUTTON_UP)
	  {
	    redraw = 1;
	    menu--;
      if (menu < 0) menu = maxitems - 1;
	  }

    if (p & PAD_BUTTON_DOWN)
	  {
      redraw = 1;
      menu++;
      if (menu == maxitems) menu = 0;
  	}

    if ((p & PAD_BUTTON_A) || (p & PAD_BUTTON_RIGHT))
  	{
      quit = 1;
      ret = menu;
    }
	  
    if (p & PAD_BUTTON_LEFT)
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
#else
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
#endif


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

#ifdef USE_NEWMENU
  t_buttons options_1_buttons[27] = 
  {
    {&image_aspect[0], aspect_COMPRESSED},
    {&image_original_1[0], original_1_COMPRESSED},
    {&image_original_2[0], original_2_COMPRESSED},
    {&image_render[0], render_COMPRESSED},
    {&image_original_1[0], original_1_COMPRESSED},
    {&image_original_2[0], original_2_COMPRESSED},
    {&image_tvmode[0], tvmode_COMPRESSED},
    {&image_tv_60hz_1[0], tv_60hz_1_COMPRESSED},
    {&image_tv_60hz_2[0], tv_60hz_2_COMPRESSED},
    {&image_borders[0], borders_COMPRESSED},
    {&image_enabled_1[0], enabled_1_COMPRESSED},
    {&image_enabled_2[0], enabled_2_COMPRESSED},
    {&image_palette[0], palette_COMPRESSED},
    {&image_pal_normal_1[0], pal_normal_1_COMPRESSED},
    {&image_pal_normal_2[0], pal_normal_2_COMPRESSED},
    {&image_center_x[0], center_x_COMPRESSED},
    {NULL, option.xshift},
    {NULL, option.xshift},
    {&image_center_y[0], center_y_COMPRESSED},
    {NULL, option.yshift},
    {NULL, option.yshift},
    {&image_scale_x[0], scale_x_COMPRESSED},
    {NULL, option.xscale},
    {NULL, option.xscale},
    {&image_scale_y[0], scale_y_COMPRESSED},
    {NULL, option.yscale},
    {NULL, option.yscale}
  };
#else
	char items[9][20];
#endif

	while (quit == 0)
	{
#ifdef USE_NEWMENU
    if (option.aspect)
    {
      options_1_buttons[1].data = &image_original_1[0];
      options_1_buttons[2].data = &image_original_2[0];
      options_1_buttons[1].size = original_1_COMPRESSED;
      options_1_buttons[2].size = original_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[1].data = &image_stretch_1[0];
      options_1_buttons[2].data = &image_stretch_2[0];
      options_1_buttons[1].size = stretch_1_COMPRESSED;
      options_1_buttons[2].size = stretch_2_COMPRESSED;
    }

    if (option.render == 1)
    {
      options_1_buttons[4].data = &image_interlaced_1[0];
      options_1_buttons[5].data = &image_interlaced_2[0];
      options_1_buttons[4].size = interlaced_1_COMPRESSED;
      options_1_buttons[5].size = interlaced_2_COMPRESSED;
    }
    else if (option.render == 2)
    {
      options_1_buttons[4].data = &image_progressive_1[0];
      options_1_buttons[5].data = &image_progressive_2[0];
      options_1_buttons[4].size = progressive_1_COMPRESSED;
      options_1_buttons[5].size = progressive_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[4].data = &image_original_1[0];
      options_1_buttons[5].data = &image_original_2[0];
      options_1_buttons[4].size = original_1_COMPRESSED;
      options_1_buttons[5].size = original_2_COMPRESSED;
    }

    if (option.tv_mode == 1)
    {
      options_1_buttons[7].data = &image_tv_50hz_1[0];
      options_1_buttons[8].data = &image_tv_50hz_2[0];
      options_1_buttons[7].size = tv_50hz_1_COMPRESSED;
      options_1_buttons[8].size = tv_50hz_2_COMPRESSED;
    }
    else if (option.tv_mode == 2)
    {
      options_1_buttons[7].data = &image_tv_auto_1[0];
      options_1_buttons[8].data = &image_tv_auto_2[0];
      options_1_buttons[7].size = tv_auto_1_COMPRESSED;
      options_1_buttons[8].size = tv_auto_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[7].data = &image_tv_60hz_1[0];
      options_1_buttons[8].data = &image_tv_60hz_2[0];
      options_1_buttons[7].size = tv_60hz_1_COMPRESSED;
      options_1_buttons[8].size = tv_60hz_2_COMPRESSED;
    }

    if (option.overscan)
    {
      options_1_buttons[10].data = &image_enabled_1[0];
      options_1_buttons[11].data = &image_enabled_2[0];
      options_1_buttons[10].size = enabled_1_COMPRESSED;
      options_1_buttons[11].size = enabled_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[10].data = &image_disabled_1[0];
      options_1_buttons[11].data = &image_disabled_2[0];
      options_1_buttons[10].size = disabled_1_COMPRESSED;
      options_1_buttons[11].size = disabled_2_COMPRESSED;
    }

    if (option.palette == 1)
    {
      options_1_buttons[13].data = &image_pal_normal_1[0];
      options_1_buttons[14].data = &image_pal_normal_2[0];
      options_1_buttons[13].size = pal_normal_1_COMPRESSED;
      options_1_buttons[14].size = pal_normal_2_COMPRESSED;
    }
    else if (option.palette == 2)
    {
      options_1_buttons[13].data = &image_pal_bright_1[0];
      options_1_buttons[14].data = &image_pal_bright_2[0];
      options_1_buttons[13].size = pal_bright_1_COMPRESSED;
      options_1_buttons[14].size = pal_bright_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[13].data = &image_original_1[0];
      options_1_buttons[14].data = &image_original_2[0];
      options_1_buttons[13].size = original_1_COMPRESSED;
      options_1_buttons[14].size = original_2_COMPRESSED;
    }
    ret = domenu (&options_1_buttons[0], count, 1);
#else
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
#endif

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
  u8 count = 5;
  u8 prevmenu = menu;
  menu = 0;

#ifdef USE_NEWMENU
  t_buttons options_1_buttons[12] = 
  {
    {&image_fm_core[0], fm_core_COMPRESSED},
    {&image_fm_1_1[0],  fm_1_1_COMPRESSED},
    {&image_fm_1_2[0],  fm_1_2_COMPRESSED},
    {&image_region[0],  region_COMPRESSED},
    {&image_auto_1[0],  auto_1_COMPRESSED},
    {&image_auto_2[0],  auto_2_COMPRESSED},
    {&image_console[0], console_COMPRESSED},
    {&image_auto_1[0],  auto_1_COMPRESSED},
    {&image_auto_2[0],  auto_2_COMPRESSED},
    {&image_bios[0],    borders_COMPRESSED},
    {&image_disabled_1[0], disabled_1_COMPRESSED},
    {&image_disabled_2[0], disabled_2_COMPRESSED}
  };
#else
  char miscmenu[5][20];
#endif
  
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
#ifdef USE_NEWMENU
    if (fm_type == 1)
    {
      options_1_buttons[1].data = &image_fm_1_1[0];
      options_1_buttons[2].data = &image_fm_1_2[0];
      options_1_buttons[1].size = fm_1_1_COMPRESSED;
      options_1_buttons[2].size = fm_1_2_COMPRESSED;
    }
    else if (fm_type == 2)
    {
      options_1_buttons[1].data = &image_fm_2_1[0];
      options_1_buttons[2].data = &image_fm_2_2[0];
      options_1_buttons[1].size = fm_2_1_COMPRESSED;
      options_1_buttons[2].size = fm_2_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[1].data = &image_disabled_1[0];
      options_1_buttons[2].data = &image_disabled_2[0];
      options_1_buttons[1].size = disabled_1_COMPRESSED;
      options_1_buttons[2].size = disabled_2_COMPRESSED;
    }

    if (option.country == 1)
    {
      options_1_buttons[4].data = &image_region_usa_1[0];
      options_1_buttons[5].data = &image_region_usa_2[0];
      options_1_buttons[4].size = region_usa_1_COMPRESSED;
      options_1_buttons[5].size = region_usa_2_COMPRESSED;
    }
    else if (option.country == 2)
    {
      options_1_buttons[4].data = &image_region_europe_1[0];
      options_1_buttons[5].data = &image_region_europe_2[0];
      options_1_buttons[4].size = region_europe_1_COMPRESSED;
      options_1_buttons[5].size = region_europe_2_COMPRESSED;
    }
    else if (option.country == 3)
    {
      options_1_buttons[4].data = &image_region_japan_1[0];
      options_1_buttons[5].data = &image_region_japan_2[0];
      options_1_buttons[4].size = region_japan_1_COMPRESSED;
      options_1_buttons[5].size = region_japan_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[4].data = &image_auto_1[0];
      options_1_buttons[5].data = &image_auto_2[0];
      options_1_buttons[4].size = auto_1_COMPRESSED;
      options_1_buttons[5].size = auto_2_COMPRESSED;
    }

    if (option.console == 1)
    {
      options_1_buttons[7].data = &image_system_sms1_1[0];
      options_1_buttons[8].data = &image_system_sms1_2[0];
      options_1_buttons[7].size = system_sms1_1_COMPRESSED;
      options_1_buttons[8].size = system_sms1_2_COMPRESSED;
    }
    else if (option.console == 2)
    {
      options_1_buttons[7].data = &image_system_sms2_1[0];
      options_1_buttons[8].data = &image_system_sms2_2[0];
      options_1_buttons[7].size = system_sms2_1_COMPRESSED;
      options_1_buttons[8].size = system_sms2_2_COMPRESSED;
    }
    else if (option.console == 3)
    {
      options_1_buttons[7].data = &image_system_gg_1[0];
      options_1_buttons[8].data = &image_system_gg_2[0];
      options_1_buttons[7].size = system_gg_1_COMPRESSED;
      options_1_buttons[8].size = system_gg_2_COMPRESSED;
    }
    else if (option.console == 4)
    {
      options_1_buttons[7].data = &image_system_ggms_1[0];
      options_1_buttons[8].data = &image_system_ggms_2[0];
      options_1_buttons[7].size = system_ggms_1_COMPRESSED;
      options_1_buttons[8].size = system_ggms_2_COMPRESSED;
    }
    else
    {
      options_1_buttons[7].data = &image_auto_1[0];
      options_1_buttons[8].data = &image_auto_2[0];
      options_1_buttons[7].size = auto_1_COMPRESSED;
      options_1_buttons[8].size = auto_2_COMPRESSED;
    }
        
    if (option.use_bios == 0)
    {
      options_1_buttons[10].data = &image_disabled_1[0];
      options_1_buttons[11].data = &image_disabled_2[0];
      options_1_buttons[10].size = disabled_1_COMPRESSED;
      options_1_buttons[11].size = disabled_2_COMPRESSED;
    }
    else if (option.use_bios == 1)
    {
      options_1_buttons[10].data = &image_slota_1[0];
      options_1_buttons[11].data = &image_slota_2[0];
      options_1_buttons[10].size = slota_1_COMPRESSED;
      options_1_buttons[11].size = slota_2_COMPRESSED;
    }
    else if (option.use_bios == 2)
    {
      options_1_buttons[10].data = &image_slotb_1[0];
      options_1_buttons[11].data = &image_slotb_2[0];
      options_1_buttons[10].size = slotb_1_COMPRESSED;
      options_1_buttons[11].size = slotb_2_COMPRESSED;
    }
#ifdef HW_RVL
    else if (option.use_bios == 3)
    {
      options_1_buttons[10].data = &image_front_1[0];
      options_1_buttons[11].data = &image_front_2[0];
      options_1_buttons[10].size = front_1_COMPRESSED;
      options_1_buttons[11].size = front_2_COMPRESSED;
    }

	  ret = domenu (&options_1_buttons[0], count, 1);
#endif

#else
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
        
		if (option.use_bios) sprintf (miscmenu[3], "Use BIOS: ON");
    else sprintf (miscmenu[3], "Use BIOS: OFF");

		if (option.autofreeze == 0) sprintf (miscmenu[4], "Auto FREEZE: SDCARD");
		else if (option.autofreeze == 1) sprintf (miscmenu[4], "Auto FREEZE: MCARD A");
		else if (option.autofreeze == 2) sprintf (miscmenu[4], "Auto FREEZE: MCARD B");
		else sprintf (miscmenu[4], "Auto FREEZE: OFF");

	  ret = domenu (&miscmenu[0], count, 0);
#endif

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

      case 3: /*** SMS BIOS ***/
        option.use_bios ^= 1;
        
        /* reset BIOS flag */
        bios.enabled &= ~1;
        if (option.use_bios) bios.enabled |= 1;
        break;

      case 4:	/*** FreezeState autoload/autosave ***/
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
  u8 count = 3;
#else
  u8 count = 2;
#endif
  u8 prevmenu = menu;
  menu = 0;

#ifdef USE_NEWMENU
  t_buttons ctrl_buttons[6] = 
  {
    {&image_player1_1[0], player1_1_COMPRESSED},
    {&image_player1_2[0], player1_2_COMPRESSED},
    {&image_gamepad_1[0], gamepad_1_COMPRESSED},
    {&image_gamepad_2[0], gamepad_2_COMPRESSED},
    {&image_wiimote_1[0], wiimote_1_COMPRESSED},
    {&image_wiimote_2[0], wiimote_2_COMPRESSED}
  };
#else
  strcpy (menutitle, "Press B to return");
#ifdef HW_RVL
  char ctrlmenu[3][20];
  sprintf (ctrlmenu[1], "Set GAMEPAD");
  sprintf (ctrlmenu[2], "Set WIIMOTE");
#else
  char ctrlmenu[2][20];
  sprintf (ctrlmenu[1], "Set GAMEPAD");
#endif
#endif

  while (quit == 0)
  {
#ifdef USE_NEWMENU
    if (num)
    {
      ctrl_buttons[0].data = &image_player2_1[0];
      ctrl_buttons[0].size = player2_1_COMPRESSED;
      ctrl_buttons[1].data = &image_player2_2[0];
      ctrl_buttons[1].size = player2_2_COMPRESSED;
    }
    else
    {
      ctrl_buttons[0].data = &image_player1_1[0];
      ctrl_buttons[0].size = player1_1_COMPRESSED;
      ctrl_buttons[1].data = &image_player1_2[0];
      ctrl_buttons[1].size = player1_2_COMPRESSED;
    }
    ret = domenu (&ctrl_buttons[0], count, 0);

#else
    sprintf (ctrlmenu[0], "Configure Player: %d", num+1);
	  ret = domenu (&ctrlmenu[0], count, 0);
#endif

	  switch (ret)
	  {
		  case 0:	/*** Controller nr. ***/
			  num ^= 1;
			  break;

		  case 1:  /*** Gamepad ***/
			  ogc_input__config(num, 0);
			  break;

#ifdef HW_RVL
		  case 2:  /*** Wiimote ***/
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

 #ifdef USE_NEWMENU
  t_buttons options_buttons[6] = 
  {
    {&image_system_1[0],   system_1_COMPRESSED},
    {&image_system_2[0],   system_2_COMPRESSED},
    {&image_display_1[0],  display_1_COMPRESSED},
    {&image_display_2[0],  display_2_COMPRESSED},
    {&image_controls_1[0], controls_1_COMPRESSED},
    {&image_controls_2[0], controls_2_COMPRESSED}
  };
#else
  char miscmenu[3][20];
  strcpy (menutitle, "Press B to return");
  sprintf (miscmenu[0], "System Options");
  sprintf (miscmenu[1], "Display Options");
  sprintf (miscmenu[2], "Controls Options");
#endif

  while (quit == 0)
  {
#ifdef USE_NEWMENU
    ret = domenu (&options_buttons[0], count, 0);
#else
	  ret = domenu (&miscmenu[0], count, 0);
#endif
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

#ifdef USE_NEWMENU
  t_buttons state_buttons[8] = 
  {
    {&image_mcard_1[0], mcard_1_COMPRESSED},
    {&image_mcard_2[0], mcard_2_COMPRESSED},
    {&image_slota_1[0], slota_1_COMPRESSED},
    {&image_slota_2[0], slota_2_COMPRESSED},
    {&image_statesave_1[0], statesave_1_COMPRESSED},
    {&image_statesave_2[0], statesave_2_COMPRESSED},
    {&image_stateload_1[0], stateload_1_COMPRESSED},
    {&image_stateload_2[0], stateload_2_COMPRESSED}
  };
#else
  char items[3][20];
  strcpy (menutitle, "Press B to return");
  sprintf(items[1], "Save State");
  sprintf(items[2], "Load State");
#endif

  while (quit == 0)
  {
 #ifdef USE_NEWMENU
   if (use_SDCARD)
    {
      state_buttons[0].data = &image_sdcard_1[0];
      state_buttons[0].size = sdcard_1_COMPRESSED;
      state_buttons[1].data = &image_sdcard_2[0];
      state_buttons[1].size = sdcard_2_COMPRESSED;
    }
    else
    {
      state_buttons[0].data = &image_mcard_1[0];
      state_buttons[0].size = mcard_1_COMPRESSED;
      state_buttons[1].data = &image_mcard_2[0];
      state_buttons[1].size = mcard_2_COMPRESSED;
    }

    if (CARDSLOT == 0)
    {
      state_buttons[2].data = &image_slota_1[0];
      state_buttons[2].size = slota_1_COMPRESSED;
      state_buttons[3].data = &image_slota_2[0];
      state_buttons[3].size = slota_2_COMPRESSED;
    }
    else if (CARDSLOT == 1)
    {
      state_buttons[2].data = &image_slotb_1[0];
      state_buttons[2].size = slotb_1_COMPRESSED;
      state_buttons[3].data = &image_slotb_2[0];
      state_buttons[3].size = slotb_2_COMPRESSED;
    }
#ifdef HW_RVL
    else if (CARDSLOT == 2)
    {
      state_buttons[2].data = &image_front_1[0];
      state_buttons[2].size = front_1_COMPRESSED;
      state_buttons[3].data = &image_front_2[0];
      state_buttons[3].size = front_2_COMPRESSED;
    }
#endif

    ret = domenu (&state_buttons[0], count, 0);

#else
    if (device == 0) sprintf(items[0], "Device: SDCARD");
    else if (device == 1) sprintf(items[0], "Device: MCARD A");
    else if (device == 2) sprintf(items[0], "Device: MCARD B");

	  ret = domenu (&items[0], count, 0);
#endif

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
#ifndef HW_RVL
static u8 load_menu = 0;
void loadmenu ()
{
	s8 ret;
	u8 quit = 0;
	u8 count = 2;
	menu = load_menu;
  
#ifdef USE_NEWMENU
  t_buttons load_buttons[6] = 
  {
    {&image_front_1[0], front_1_COMPRESSED},
    {&image_front_2[0], front_2_COMPRESSED},
    {&image_slota_1[0], slota_1_COMPRESSED},
    {&image_slota_2[0], slota_2_COMPRESSED},
    {&image_slotb_1[0], slotb_1_COMPRESSED},
    {&image_slotb_2[0], slotb_2_COMPRESSED}
  };
#else
	char item[2][20] = {
		{"Load from DVD"},
		{"Load from SDCARD"}
	};
#endif

	while (quit == 0)
	{
#ifdef USE_NEWMENU
    ret = domenu (&load_buttons[0], count, 0);
#else
    ret = domenu (&item[0], count, 0);
#endif
    
    switch (ret)
		{
			case -1: /*** Button B ***/
				quit = 1;
				break;
			case 0:	 /*** Load from DVD ***/
				OpenDVD();
				quit = 1;
				break;
			case 1:  /*** Load from SCDARD ***/
				OpenSD();
				quit = 1;
				break;
		}
	}

	load_menu = menu;
}
#endif

/****************************************************************************
 * Main menu
 *
 ****************************************************************************/
void MainMenu ()
{
	s8 ret;
	u8 quit = 0;
	menu = 0;

#ifdef USE_NEWMENU
	u8 count = 7;
  t_buttons main_buttons[14] = 
  {
    {&image_play_1[0],  play_1_COMPRESSED},
    {&image_play_2[0],  play_2_COMPRESSED},
    {&image_reset_1[0],  reset_1_COMPRESSED},
    {&image_reset_2[0],  reset_2_COMPRESSED},
    {&image_load_1[0],  load_1_COMPRESSED},
    {&image_load_2[0],  load_2_COMPRESSED},
    {&image_options_1[0],  options_1_COMPRESSED},
    {&image_options_2[0],  options_2_COMPRESSED},
    {&image_save_1[0],  save_1_COMPRESSED},
    {&image_save_2[0],  save_2_COMPRESSED},
    {&image_loader_1[0],  loader_1_COMPRESSED},
    {&image_loader_2[0],  loader_2_COMPRESSED},
    {&image_exit_1[0],  exit_1_COMPRESSED},
    {&image_exit_2[0],  exit_2_COMPRESSED}
  };
#else
#ifdef HW_RVL
	u8 count = 7;
	char items[7][20] =
#else
	u8 count = 8;
	char items[8][20] =
#endif
	{
		{"Play Game"},
		{"Hard Reset"},
		{"Load New Game"},
		{"Emulator Options"},
		{"Savestate Manager"},
#ifdef HW_RVL
		{"Return to Loader"},
		{"System Menu"}
#else
		{"Stop DVD Motor"},
		{"SD/PSO Reload"},
		{"System Reboot"}
#endif
	};
#endif

	/* Switch to menu default rendering mode (60hz or 50hz, but always 480 lines) */
	VIDEO_Configure (vmode);
	VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();

	while (quit == 0)
	{
#ifdef USE_NEWMENU
    ret = domenu (&main_buttons[0], count, 0);
#else
    strcpy (menutitle, "");
		ret = domenu (&items[0], count,0);
#endif

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
#ifdef HW_RVL
        OpenSD();
#else
        loadmenu();
#endif
        menu = 0;
				break;

			case 3:
				optionmenu();
				break;

			case 4:
				quit = loadsavemenu();
				break;

#ifdef HW_RVL
			case 5:
        memfile_autosave();
        VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        exit(0);
        break;

			case 6:
        memfile_autosave();
        VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
        VIDEO_Flush();
        VIDEO_WaitVSync();
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
				break;
#else
			case 5:  /*** Stop DVD Motor ***/
				ShowAction("Stopping DVD Motor ...");
				dvd_motor_off();
				break;

      case 6:
        memfile_autosave();
        VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        exit(0);
        break;

			case 7:
        memfile_autosave();
				SYS_ResetSystem(SYS_HOTRESET,0,0);
				break;
#endif
		}
	}

	/*** Remove any still held buttons ***/
  while (PAD_ButtonsHeld(0))  PAD_ScanPads();
#ifdef HW_RVL
  while (WPAD_ButtonsHeld(0)) WPAD_ScanPads();
  WPAD_SetDataFormat(0,WPAD_FMT_CORE_ACC_IR);
  WPAD_SetVRes(0,640,480);
  WPAD_SetDataFormat(1,WPAD_FMT_CORE_ACC_IR);
  WPAD_SetVRes(1,640,480);
#endif

	/*** Reinitalize GX ***/
  ogc_video__reset();

#ifndef HW_RVL
	/*** Stop the DVD from causing clicks while playing ***/
	uselessinquiry ();
#endif
}
