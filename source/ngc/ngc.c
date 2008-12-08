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
 ***************************************************************************/
#include "shared.h"
#include "dvd.h"
#include "font.h"
#include "history.h"

#ifdef HW_DOL
#include "dvd.h"
#else
#include <di/di.h>
#endif

int Shutdown = 0;

#ifdef HW_RVL
/* Power Button callback */
void Power_Off(void)
{
  Shutdown = 1;
  ConfigRequested = 1;
}
#endif


/***************************************************************************
 * SMSPlus Virtual Machine
 *
 ***************************************************************************/
int smsromsize;
uint8 *smsrom;

static void load_bios()
{
  /* reset BIOS flag */
  bios.enabled  = 0;

  /* open BIOS file */
  FILE *fp = fopen("/smsplus/SMS_BIOS.sms", "rb");
  if (fp == NULL) return;

  /* get BIOS size */
  fseek(fp , 0 , SEEK_END);
  int filesize = ftell (fp);
  fseek(fp, 0, SEEK_SET);

  /* read BIOS file */
  fread(bios.rom, 1, filesize, fp);
  fclose(fp);

  /* set BIOS size */
  if (filesize < 0x4000) filesize = 0x4000;
  bios.pages = filesize / 0x4000;

  /* set BIOS flag */
  if (option.use_bios) bios.enabled = 3;
  else bios.enabled = 2;

  set_config();
}

static void init_machine (void)
{
  /*** Allocate cart_rom here ***/
  smsrom = memalign(32, 0x100000);
  smsromsize = 0;

  /* Look for BIOS rom */
  bios.rom = memalign(32, 0x100000);
  load_bios();

  /* allocate global work bitmap */
  memset (&bitmap, 0, sizeof (bitmap));
  bitmap.width = 720;
  bitmap.height = 288;
  bitmap.depth = 16;
  bitmap.granularity = 2;
  bitmap.pitch = bitmap.width * bitmap.granularity;
  bitmap.viewport.w = 256;
  bitmap.viewport.h = 192;
  bitmap.viewport.x = 0;
  bitmap.viewport.y = 0;
  bitmap.data =  memalign(32, bitmap.pitch * bitmap.height);
}
 
/***************************************************************************
 * M A I N
 *
 ***************************************************************************/
int frameticker = 0;
bool use_FAT = 0;

int main (int argc, char *argv[])
{
#ifdef HW_RVL
	/* initialize Wii DVD interface first */
  DI_Close();
  DI_Init();
#endif

  u16 usBetweenFrames;
  long long now, prev;
  
  /* Initialize OGC subsystems */
  ogc_video__init();
  ogc_input__init();
  ogc_audio__init();

#ifdef HW_DOL
  /* Initialize GC DVD interface */
  DVD_Init ();
  dvd_drive_detect();
#endif

#ifdef HW_RVL
  /* Power Button callback */
  SYS_SetPowerCallback(Power_Off);
#endif

  /* Initialize FAT Interface */
  if (fatInitDefault() == true)
  {
    use_FAT = 1;
  }

  /* Default Config */
  set_option_defaults ();
  config_load();

  /* Restore Recent Files list */
  set_history_defaults();
  history_load();

  /* Initialize SMS Virtual Machine */
  init_machine ();

  /* Show Menu */
  legal ();
  MainMenu();
  ConfigRequested = 0;
			
  /* Initialize Frame timings */
  frameticker = 0;
  prev = gettime();

  /* Emulation Loop */
  while (1)
  {
    /* update inputs */
    ogc_input__update();

    /* Frame synchronization */
    if (gc_pal != sms.display)
    {
      /* use timers */
      usBetweenFrames = sms.display ? 20000 : 16666;
      now = gettime();
      if (diff_usec(prev, now) > usBetweenFrames)
	    {
        /* Frame skipping */
        prev = now;
		    system_frame(1);
	    }
      else
      {
        /* Delay */
        while (diff_usec(prev, now) < usBetweenFrames) now = gettime();

        /* Render Frame */
        prev = now;
        system_frame(0);
      }
    }
    else
    {		  
      /* use VSync */
      if (frameticker > 1)
      {
        /* Frame skipping */
        frameticker--;
        system_frame (1);
      }
      else
      {
        /* Delay */
        while (!frameticker) usleep(10);	
        
        system_frame (0);
      }
      
      frameticker--;
    }

    /* update video & audio */
    ogc_video__update();
    ogc_audio__update();

    /* Check for Menu request */
    if (ConfigRequested)
    {
      /* reset AUDIO */
      ogc_audio__reset();

      /* go to menu */
      MainMenu ();
      ConfigRequested = 0;

      /* reset frame timings */
      frameticker = 0;
      prev = gettime();
    }
  }
  return 0;
}
