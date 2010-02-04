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
 *   ROM File Loading support
 *
 ******************************************************************************/

#include "shared.h"

#define GAME_DATABASE_CNT 86

typedef struct
{
  uint32 crc;
  uint8 glasses_3d;
  uint8 device;
  uint8 mapper;
  uint8 display;
  uint8 territory;
  uint8 console;
  char *name;
} rominfo_t;

rominfo_t game_list[GAME_DATABASE_CNT] =
{
  /* games requiring CODEMASTER mapper */
  {0x29822980, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Cosmic Spacehead"},
  {0x6CAA625B, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GG,
   "Cosmic Spacehead (GG)"}, 
  {0xea5c3a6f, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Dinobasher - Starring Bignose the Caveman [Proto]"}, 
  {0x152F0DCC, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Drop Zone"}, 
  {0x5E53C7F7, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GG,
   "Ernie Els Golf"}, 
  {0x8813514B, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Excellent Dizzy Collection, The [Proto]"},
  {0xAA140C9C, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Excellent Dizzy Collection, The [SMS-GG]"},
  {0xB9664AE1, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Fantastic Dizzy"},
  {0xC888222B, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Fantastic Dizzy [SMS-GG]"},
  {0x76c5bdfb, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Jang Pung II [SMS-GG]"},
  {0xD9A7F170, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Man Overboard!"}, 
  {0xA577CE46, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Micro Machines"}, 
  {0xF7C524F6, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Micro Machines [BAD DUMP]"}, 
  {0xDBE8895C, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Micro Machines 2 - Turbo Tournament"},
  {0xC1756BEE, 0, DEVICE_PAD2B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Pete Sampras Tennis"},

  /* games requiring KOREAN mapper */
  {0x97d03541, 0, DEVICE_PAD2B, MAPPER_KOREAN, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Sangokushi 3 (KR)"},
  {0x89b79e77, 0, DEVICE_PAD2B, MAPPER_KOREAN, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Dodgeball King (KR)"},
  {0x67c2f0ff, 0, DEVICE_PAD2B, MAPPER_KOREAN, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Super Boy II (KR)"},

  /* games that require PAL timings (from MEKA.nam by Omar Cornut) */
  {0x72420f38, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Addams Familly"},
  {0x2d48c1d3, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Back to the Future Part III"},
  {0x1CBB7BF1, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Battlemaniacs (BR)"}, 
  {0x1b10a951, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Bram Stoker's Dracula"},
  {0xC0E25D62, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "California Games II"}, 
  {0xc9dbf936, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Home Alone"},
  {0x0047B615, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Predator2"},
  {0xF42E145C, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Quest for the Shaven Yak Starring Ren Hoek & Stimpy (BR)"}, 
  {0x9F951756, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "RoboCop 3"}, 
  {0x1575581D, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Shadow of the Beast"}, 
  {0x96B3F29E, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Sonic Blast (BR)"}, 
  {0x5B3B922C, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Sonic the Hedgehog 2 [V0]"},
  {0xd6f2bfca, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Sonic the Hedgehog 2 [V1]"},
  {0xCA1D3752, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Space Harrier [50 Hz]"}, 
  {0x85cfc9c9, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Taito Chase H.Q."},  

  /* games requiring 315-5124 VDP (Mark-III, Sega Master System) */
  {0x32759751, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Y's (J)"},

  /* games requiring Game Gear SMS compatibility mode */
  {0x59840fd6, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Castle of Illusion - Starring Mickey Mouse"},
  {0x9942b69b, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_GGMS,
   "Castle of Illusion - Starring Mickey Mouse (J)"},
  {0x5877b10d, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_GGMS,
   "Castle of Illusion - Starring Mickey Mouse (J) [HACK]"},
  {0x9c76fb3a, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Rastan Saga [SMS-GG]"},
  {0x7bb81e3d, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Taito Chase H.Q. [SMS-GG]"},
  {0x44fbe8f6, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Taito Chase H.Q. [SMS-GG][HACK]"},
  {0x18086b70, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Taito Chase H.Q. [SMS-GG][HACK][BAD]"},
  {0xda8e95a9, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "WWF Wrestlemania Steel Cage Challenge [SMS-GG]"},
  {0xcb42bd33, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "WWF Wrestlemania Steel Cage Challenge [SMS-GG] [BAD DUMP]"},
  {0x1d93246e, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Olympic Gold [SMS-GG] [A]"},
  {0xa2f9c7af, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Olympic Gold [SMS-GG] [B]"},
  {0xf037ec00, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Out Run Europa [SMS-GG]"},
  {0xe5f789b9, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Predator 2 [SMS-GG]"},
  {0x311d2863, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Prince of Persia [SMS-GG] [A]"},
  {0x45f058d6, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Prince of Persia [SMS-GG] [B]"},
  {0x56201996, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "R.C. Grand Prix [SMS-GG]"},
  {0x10dbbef4, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_GGMS,
   "Super Kick Off [SMS-GG]"},
  {0xbd1cc7df, 0, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_GGMS,
   "Super Tetris (KR)"},

  /* games requiring 3D Glasses */
  {0xFBF96C81, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Blade Eagle 3-D (BR)"},
  {0x8ECD201C, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Blade Eagle 3-D"},
  {0x31B8040B, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Maze Hunter 3-D"},
  {0x871562b0, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Maze Walker"},
  {0xABD48AD2, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Poseidon Wars 3-D"},
  {0x6BD5C2BF, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Space Harrier 3-D"},
  {0x156948f9, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Space Harrier 3-D (J)"},
  {0xA3EF13CB, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Zaxxon 3-D"},
  {0xbba74147, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Zaxxon 3-D [Proto]"},
  {0xd6f43dda, 1, DEVICE_PAD2B, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Out Run 3-D"},

  /* games requiring Light Phaser & 3D Glasses */
  {0xFBE5CFBB, 1, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Missile Defense 3D"},
  {0xe79bb689, 1, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Missile Defense 3D [BIOS]"},
  
  /* games requiring Light Phaser */
  {0x861b6e79, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Assault City [Light Phaser]"},
  {0x5fc74d2a, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Gangster Town"},
  {0xe167a561, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Hang-On / Safari Hunt"},
  {0xc5083000, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Hang-On / Safari Hunt [BAD DUMP]"},
  {0x91e93385, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Hang-On / Safari Hunt [BIOS]"},
  {0xe8ea842c, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Marksman Shooting / Trap Shooting"},
  {0xe8215c2e, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Marksman Shooting / Trap Shooting / Safari Hunt"},
  {0x205caae8, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Operation Wolf"}, /* can be also played using the PLAYER2 gamepad */
  {0x23283f37, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Operation Wolf [A]"}, /* can be also played using the PLAYER2 gamepad */
  {0xda5a7013, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Rambo 3"},
  {0x79ac8e7f, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Rescue Mission"},
  {0x4B051022, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Shooting Gallery"},
  {0xa908cff5, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Spacegun"},
  {0x5359762D, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Wanted"},
  {0x0ca95637, 0, DEVICE_LIGHTGUN, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Laser Ghost"},

  /* games requiring Paddle */
  {0xf9dbb533, 0, DEVICE_PADDLE, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Alex Kidd BMX Trial"},
  {0xa6fa42d0, 0, DEVICE_PADDLE, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Galactic Protector"},
  {0x29bc7fad, 0, DEVICE_PADDLE, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Megumi Rescue"},
  {0x315917d4, 0, DEVICE_PADDLE, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC, CONSOLE_SMS,
   "Woody Pop"},

    /* games requiring Sport Pad (NOT EMULATED YET) */
  {0x946b8c4a, 0, DEVICE_SPORTSPAD, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Great Ice Hockey"},
  {0xe42e4998, 0, DEVICE_SPORTSPAD, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Sports Pad Football"},
  {0x41C948BF, 0, DEVICE_SPORTSPAD, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS2,
   "Sports Pad Soccer"}

};

void set_config()
{
  int i;

  /* default sms settings */
  cart.mapper = MAPPER_SEGA;
  sms.console = CONSOLE_SMS2;
  sms.territory = TERRITORY_EXPORT;
  sms.display = DISPLAY_NTSC;
  sms.glasses_3d = 0;
  sms.device[0] = DEVICE_PAD2B;
  sms.device[1] = DEVICE_PAD2B;
  sms.use_fm = option.fm_enable;

  /* console type detection */
  /* SMS Header is located at 0x7ff0 */
  if ((cart.size > 0x7000) && (!memcmp (&cart.rom[0x7ff0], "TMR SEGA", 8)))
  {
    uint8 region = (cart.rom[0x7fff] & 0xf0) >> 4;

    switch (region)
    {
      case 5:
        sms.console = CONSOLE_GG;
        sms.territory = TERRITORY_DOMESTIC;
        break;

      case 6:
      case 7:
        sms.console = CONSOLE_GG;
        sms.territory = TERRITORY_EXPORT;
        break;

      case 3:
        sms.console = CONSOLE_SMS;
        sms.territory = TERRITORY_DOMESTIC;
        break;
    
      default:
        sms.console = CONSOLE_SMS2;
        sms.territory = TERRITORY_EXPORT;
        break;
    }
  }

  sms.gun_offset = 20; /* default offset */

  /* retrieve game settings from database */
  for (i = 0; i < GAME_DATABASE_CNT; i++)
  {
    if (cart.crc == game_list[i].crc)
    {
      cart.mapper = game_list[i].mapper;
      sms.display = game_list[i].display;
      sms.territory = game_list[i].territory;
      sms.glasses_3d = game_list[i].glasses_3d;
      sms.console =  game_list[i].console;
      sms.device[0] = game_list[i].device;
      if (game_list[i].device != DEVICE_LIGHTGUN) sms.device[1] = game_list[i].device;

      if ((strcmp(game_list[i].name, "Spacegun") == 0) ||
          (strcmp(game_list[i].name, "Gangster Town") == 0))
      {
        /* these games seem to use different gun position calculation method */
        sms.gun_offset = 16;
      }
      i = GAME_DATABASE_CNT;
    }
  }

  /* reinit bios */
  if (IS_SMS)
    bios.enabled |= (option.use_bios & 1);
  else
    bios.enabled &= ~1;

  /* force settings if AUTO is not set*/
  if (option.console == 1)
    sms.console = CONSOLE_SMS;
  else if (option.console == 2)
    sms.console = CONSOLE_SMS2;
  else if (option.console == 3)
    sms.console = CONSOLE_GG;
  else if (option.console == 4)
    sms.console = CONSOLE_GGMS;
  else if (option.console == 5)
  {
    sms.console = CONSOLE_SG1000;
    cart.mapper = MAPPER_NONE;
  }
  else if (option.console == 6)
  {
    sms.console = CONSOLE_COLECO;
    cart.mapper = MAPPER_NONE;
  }

  if (option.country == 1) /* USA */
  {
    sms.display = DISPLAY_NTSC;
    sms.territory = TERRITORY_EXPORT;
  }
  else if (option.country == 2) /* EUROPE */
  {
    sms.display = DISPLAY_PAL;
    sms.territory = TERRITORY_EXPORT;
  }
  else if (option.country == 3) /* JAPAN */
  {
    sms.display = DISPLAY_NTSC;
    sms.territory = TERRITORY_DOMESTIC;
  }
}

int load_rom (char *filename)
{
#ifdef NGC
  memset (&cart, 0, sizeof (cart));
  cart.rom = &smsrom[0];
  cart.size = smsromsize;
  if (cart.size < 0x4000) cart.size = 0x4000;
#else
  if(cart.rom)
  {
    free(cart.rom);
    cart.rom = NULL;
  }

  if(check_zip(filename))
  {
    char name[PATH_MAX];
    int size = cart.size;
    cart.rom = loadFromZipByName(filename, name, &size);
    if(!cart.rom) return 0;
    strcpy(game_name, name);
  }
  else
  {
    FILE *fd = NULL;
    fd = fopen(filename, "rb");
    if(!fd) return 0;

    /* Seek to end of file, and get size */
    fseek(fd, 0, SEEK_END);
    cart.size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    if (cart.size < 0x4000) cart.size = 0x4000;
    cart.rom = malloc(cart.size);
    if(!cart.rom) return 0;
    fread(cart.rom, cart.size, 1, fd);

    fclose(fd);
  }
#endif

  /* Take care of image header, if present */
  if ((cart.size / 512) & 1)
  {
    cart.size -= 512;
    memcpy (cart.rom, cart.rom + 512, cart.size);
  }

  cart.pages = cart.size / 0x4000;
  cart.crc = crc32 (0, cart.rom, cart.size);
  cart.loaded = 1;

  set_config();

  return 1;
}
