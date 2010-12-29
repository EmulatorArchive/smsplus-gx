
/*
    Copyright (c) 2002, 2003 Gregory Montoir

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "sdlsms.h"
#include "filters.h"
#include "shared.h"


 t_config option;

/* Game file name */
char game_name[PATH_MAX];

 static t_named_filter filters[] = {
  {"2xsai",      FILTER_2XSAI},
  {"super2xsai", FILTER_SUPER2XSAI},
  {"supereagle", FILTER_SUPEREAGLE},
  {"advmame2x",  FILTER_ADVMAME2X},
  {"tv2x",       FILTER_TV2X},
  {"2x",         FILTER_NORMAL2X},
  {"bilinear",   FILTER_BILINEAR},
  {"dotmatrix",  FILTER_DOTMATRIX}
};

static int parse_args(int argc, char **argv)
{
  int i;

  /* default virtual console emulation settings */
  option.fm = 0;
  option.spritelimit  = 1;
  option.filter = -1;
  option.country = 0;
  option.overscan = 1;
  option.tms_pal = 2;
  option.console = 0;

  strcpy(option.game_name, argv[1]);

  for(i = 2; i < argc; ++i) {
    if(strcasecmp(argv[i], "--fm") == 0) {
      option.fm = SND_EMU2413;
    }
    else if(strcasecmp(argv[i], "--fskip") == 0) {
      if(++i<argc) {
        option.frameskip = atoi(argv[i]);
      }
    }
    else if(strcasecmp(argv[i], "--fullspeed") == 0)
      option.fullspeed = 1;
    else if(strcasecmp(argv[i], "--fullscreen") == 0)
      option.fullscreen = 1;
    else if(strcasecmp(argv[i], "--nosound") == 0)
      option.nosound = 1;
    else if(strcasecmp(argv[i], "--joystick") == 0)
      option.joystick = 1;
    else if(strcasecmp(argv[i], "--filter") == 0) {
      i++;
      if(i < argc) {
        int j = 0;
        for( ; j < sizeof(filters) / sizeof(filters[0]); ++j) {
          if(strcasecmp(filters[j].name, argv[i]) == 0) {
            option.filter = filters[j].type;
            break;
          }
        }
      }
    }
    else 
      printf("WARNING: unknown option '%s'.\n", argv[i]);
  }
  return 1;
}

/* Save or load SRAM */
void system_manage_sram(uint8 *sram, int slot, int mode)
{
    char name[PATH_MAX];
    FILE *fd;
    strcpy(name, option.game_name);
    strcpy(strrchr(name, '.'), ".sav");

    switch(mode)
    {
        case SRAM_SAVE:
            if(sms.save)
            {
                fd = fopen(name, "wb");
                if(fd)
                {
                    fwrite(sram, 0x8000, 1, fd);
                    fclose(fd);
                }
            }
            break;

        case SRAM_LOAD:
            fd = fopen(name, "rb");
            if(fd)
            {
                sms.save = 1;
                fread(sram, 0x8000, 1, fd);
                fclose(fd);
            }
            else
            {
                /* No SRAM file, so initialize memory */
                memset(sram, 0x00, 0x8000);
            }
            break;
    }
}

int main(int argc, char **argv)
{
  printf("%s (Build date %s)\n", SMSSDL_TITLE, __DATE__);
  printf("Copyright (C) 1998-2007  Charles MacDonald\n");
  printf("Additional code by Eke-Eke, 2007,2008,2009\n");
  printf("Original SDL Version by Gregory Montoir (cyx@frenchkiss.net)\n");
  printf("\n");

  if(argc < 2) {
    int i;
    printf("Usage: %s <filename.<SMS|GG>> [--options]\n", argv[0]);
    printf("Options:\n");
    printf(" --fm           \t enable YM2413 sound.\n");
    printf(" --fskip <n>    \t specify the number of frames to skip.\n");
    printf(" --fullspeed    \t do not limit to 60 frames per second.\n");
    printf(" --fullscreen   \t start in fullscreen mode.\n");
    printf(" --joystick     \t use joystick.\n");
    printf(" --nosound      \t disable sound.\n");
    printf(" --filter <mode>\t render using a filter: ");
    for(i = 0; i < sizeof(filters) / sizeof(filters[0]) - 1; ++i)
      printf("%s,", filters[i].name);
    printf("%s.", filters[i].name);
    return 1;
  }

  memset(&option, 0, sizeof(option));
  if(!parse_args(argc, argv))
    return 0;

  if(sdlsms_init()) {
    sdlsms_emulate();
    sdlsms_shutdown();
  }

  return 0;
}
