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
 *   Nintendo Gamecube Rom Loading support
 *
 ******************************************************************************/

#include "shared.h"
#include "config.h"

typedef struct
{
  uint32 crc;
  uint8 light_phaser;
  uint8 glasses_3d;
  uint8 paddle;
  uint8 sport_pad;
  int mapper;
  int display;
  int territory;
  char *name;
} rominfo_t;

int gamecount = 60;
rominfo_t game_list[60] =
{
  /* games that require CODEMASTER mapper & PAL system (13)*/
  {0x29822980, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Cosmic Spacehead"},
  {0x6CAA625B, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Cosmic Spacehead (GG)"}, 
  {0x152F0DCC, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Drop Zone"}, 
  {0x5E53C7F7, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Ernie Els Golf"}, 
  {0x8813514B, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Excellent Dizzy (Proto)"},
  {0xAA140C9C, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Excellent Dizzy (Proto - GG)"},
  {0xB9664AE1, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Fantastic Dizzy"},
  {0xC888222B, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Fantastic Dizzy (GG)"},
  {0xD9A7F170, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Man Overboard!"}, 
  {0xA577CE46, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Micro Machines"}, 
  {0xF7C524F6, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Micro Machines [BAD DUMP]"}, 
  {0xDBE8895C, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Micro Machines 2 - Turbo Tournament"},
  {0xC1756BEE, 0, 0, 0, 0, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT,
   "Pete Sampras Tennis"},

  /* games that require PAL system (15)*/
  {0x72420f38, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Addams Familly"},
  {0x2d48c1d3, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Back to the Future Part III"},
  {0x1CBB7BF1, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Battlemaniacs (BR)"}, 
  {0x1b10a951, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Bram Stoker's Dracula"},
  {0xC0E25D62, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "California Games II"}, 
  {0xc9dbf936, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Home Alone"},
  {0x0047B615, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT, 
   "Predator2"},
  {0xF42E145C, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Quest for the Shaven Yak Starring Ren Hoek & Stimpy (BR)"}, 
  {0x9F951756, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "RoboCop 3"}, 
  {0x1575581D, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Shadow of the Beast"}, 
  {0x96B3F29E, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Sonic Blast (BR)"}, 
  {0x5B3B922C, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Sonic the Hedgehog 2 [V0]"},
  {0xd6f2bfca, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Sonic the Hedgehog 2 [V1]"},
  {0xCA1D3752, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Space Harrier [50 Hz]"}, 
  {0x85cfc9c9, 0, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Taito Chase H.Q."},  

  /* games that require 3D Glasses (10) */
  {0xFBF96C81, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Blade Eagle 3-D (BR)"},
  {0x8ECD201C, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Blade Eagle 3-D"},
  {0x31B8040B, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Maze Hunter 3-D"},
  {0x871562b0, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC,
   "Maze Walker"},
  {0xABD48AD2, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Poseidon Wars 3-D"},
  {0x6BD5C2BF, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Space Harrier 3-D"},
  {0x156948f9, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC,
   "Space Harrier 3-D (J)"},
  {0xA3EF13CB, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Zaxxon 3-D"},
  {0xbba74147, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Zaxxon 3-D [Proto]"},
  {0xd6f43dda, 0, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Out Run 3-D"},

  /* games that require Light Phaser & 3D Glasses (2) */
  {0xFBE5CFBB, 1, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Missile Defense 3D"},
  {0xe79bb689, 1, 1, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Missile Defense 3D [BIOS]"},
  
  /* games that require Light Phaser (13) */
  {0x861b6e79, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Assault City [Light Phaser]"},
  {0x5fc74d2a, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Gangster Town"},
  {0xe167a561, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Hang-On / Safari Hunt"},
  {0xc5083000, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Hang-On / Safari Hunt [BAD DUMP]"},
  {0xe8ea842c, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Marksman Shooting / Trap Shooting"},
  {0xe8215c2e, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Marksman Shooting / Trap Shooting / Safari Hunt"},
  {0x205caae8, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_PAL, TERRITORY_EXPORT,
   "Operation Wolf"}, /* can be also played using the PLAYER2 gamepad */
  {0x23283f37, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Operation Wolf [A]"}, /* can be also played using the PLAYER2 gamepad */
  {0xda5a7013, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Rambo 3"},
  {0x79ac8e7f, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Rescue Mission"},
  {0x4B051022, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Shooting Gallery"},
  {0xa908cff5, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Spacegun"},
  {0x5359762D, 1, 0, 0, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Wanted"},

  /* games that require Paddle (4) */
  {0xf9dbb533, 0, 0, 1, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Alex Kidd BMX Trial"},
  {0xa6fa42d0, 0, 0, 1, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Galactic Protector"},
  {0x29bc7fad, 0, 0, 1, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Megumi Rescue"},
  {0x315917d4, 0, 0, 1, 0, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Woody Pop"},

    /* games that require Sport Pad (3) */
  {0x946b8c4a, 0, 0, 0, 1, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Great Ice Hockey"},
  {0xe42e4998, 0, 0, 0, 1, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_EXPORT,
   "Sports Pad Football"},
  {0x41C948BF, 0, 0, 0, 1, MAPPER_SEGA, DISPLAY_NTSC, TERRITORY_DOMESTIC,
   "Sports Pad Soccer"}

};

extern int smsromsize;
extern uint8 *smsrom;

int loadsmsrom ()
{
  int i;
  int size;

  /*** Initialize everything ***/
  memset (&cart, 0, sizeof (cart));
  cart.rom = &smsrom[0];
  size = smsromsize;
  
  /* Don't load games smaller than 16K */
  if (size < 0x4000) return 0;
  
  /* Take care of image header, if present */
  if ((size / 512) & 1)
  {
      size -= 512;
      memcpy (cart.rom, cart.rom + 512, size);
  }

  cart.pages = size / 0x4000;
  cart.crc = crc32 (0, cart.rom, size);
  cart.mapper = MAPPER_SEGA;

  /* default sms settings */
  sms.use_fm = 0;
  sms.console = CONSOLE_SMS;
  sms.territory = TERRITORY_EXPORT;
  sms.display = DISPLAY_NTSC;
  sms.light_phaser = 0;
  sms.paddle = 0;
  sms.glasses_3d = 0;
  sms.sport_pad = 0;

  /* console type detection */
  /* SMS Header is located at 0x7ff0 */
  if (!memcmp (&cart.rom[0x7ff0], "TMR SEGA", 8))
  {
	  uint8 region = (cart.rom[0x7fff] & 0xf0) >> 4;

      switch (region)
	  {
	    case 2:
	    case 3:
	      sms.console = CONSOLE_SMS;
	      break;

        case 5:
	    case 6:
	    case 7:
	      sms.console = CONSOLE_GG;
	      break;

	    default:
	      sms.console = CONSOLE_SMS;
	      break;
	  }
  }

  /* automatic settings if fm is enabled & console is SMS */
  if (sms_option.fm_enable && IS_SMS)
  {
	  sms.use_fm = 1;
	  sms.console = CONSOLE_SMSJ;
	  sms.territory = TERRITORY_DOMESTIC;
	  sms.display = DISPLAY_NTSC;
  }
 
  /* retrieve game settings from database */
  for (i = 0; i < gamecount; i++)
  {
	  if (cart.crc == game_list[i].crc)
	  {
		  cart.mapper = game_list[i].mapper;
	      sms.display = game_list[i].display;
	      sms.territory = game_list[i].territory;
		  sms.light_phaser = game_list[i].light_phaser;
		  sms.paddle = game_list[i].paddle;
		  sms.glasses_3d = game_list[i].glasses_3d;
		  sms.sport_pad = game_list[i].sport_pad;
      }
  }

  /* force user settings if AUTO is not set*/
  if (sms_option.console == 0)      sms.console = CONSOLE_SMS;
  else if (sms_option.console == 1) sms.console = CONSOLE_SMSJ;
  else if (sms_option.console == 2) sms.console = CONSOLE_SMS2;
  else if (sms_option.console == 3) sms.console = CONSOLE_GG;
  else if (sms_option.console == 4) sms.console = CONSOLE_GGMS;
  if (sms_option.display != -1) sms.display = sms_option.display;
  if (sms_option.country != -1) sms.territory = sms_option.country;
  if (sms_option.codies != -1) cart.mapper = sms_option.codies;

  return 1;
}
