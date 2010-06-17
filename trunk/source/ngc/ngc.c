/****************************************************************************
 *  ngc.c
 *
 *  SMS Plus GX main
 *
 *  code by Softdev (2006), Eke-Eke (2007,2008)
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
#include "font.h"
#include "history.h"

#ifdef HW_DOL
#include "dvd.h"
#else
#include <di/di.h>
#include "preferences.h"
#endif

#include <fat.h>

int Shutdown = 0;

#ifdef HW_RVL
/* Power Button callback */
void Power_Off(void)
{
  Shutdown = 1;
  ConfigRequested = 1;
}
#endif

void system_manage_sram(uint8 *sram, int slot, int mode)
{
}

/***************************************************************************
 * SMS Plus Virtual Machine
 *
 ***************************************************************************/
int smsromsize;
uint8 *smsrom;

static void load_bios()
{
  char pathname[MAXPATHLEN];

  /* Colecovision BIOS */
  sprintf (pathname, "%s/BIOS.col",DEFAULT_PATH);
  FILE *fp = fopen(pathname, "rb");
  if (fp)
  {
    fread(coleco.rom, 0x2000, 1, fp);
    fclose(fp);
  }

  /* Master System BIOS */
  bios.enabled = 0;
  sprintf (pathname, "%s/BIOS.sms",DEFAULT_PATH);
  fp = fopen(pathname, "rb");
  if (fp)
  {
    /* get BIOS size */
    fseek(fp , 0 , SEEK_END);
    int filesize = ftell (fp);
    fseek(fp, 0, SEEK_SET);

    /* read BIOS file */
    int done = 0;
    while (filesize > FATCHUNK)
    {
      fread(bios.rom + done, FATCHUNK, 1, fp);
      done+=FATCHUNK;
      filesize-=FATCHUNK;
    }
    fread(bios.rom + done, filesize, 1, fp);
    fclose(fp);
  
    /* set BIOS size */
    if (filesize < 0x4000) filesize = 0x4000;
    bios.pages = filesize / 0x4000;

    /* set BIOS flag */
    bios.enabled = option.use_bios | 2;
    set_config();
  }

}

static void init_machine (void)
{
  /* allocate Cartridge ROM */
  smsrom = memalign(32, 1048576);
  smsromsize = 0;

  /* allocate internal BIOS ROM */
  bios.rom = memalign(32, 1048576);
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
bool fat_enabled = 0;

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
    fat_enabled = 1;
  }

  /* Default Config */
  legal();
  set_option_defaults ();
  config_load();
#ifdef HW_RVL
  /* Load SMB Settings */
  loadSettings();
#endif

  /* Restore Recent Files list */
  set_history_defaults();
  history_load();

  /* Initialize Virtual Machine */
  init_machine ();

  /* Show Menu */
  MainMenu();
  ConfigRequested = 0;

  /* Initialize Frame timings */
  frameticker = 0;
  usBetweenFrames = sms.display ? 20000 : 16666;
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
      ogc_video__reset();

      /* reset frame timings */
      frameticker = 0;
      usBetweenFrames = sms.display ? 20000 : 16666;
      prev = gettime();
    }
  }
  return 0;
}
