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
 *   Nintendo Gamecube State Management
 *
 ******************************************************************************/

#include "shared.h"

static unsigned char state[0x10000];
static unsigned int bufferptr;


int system_save_state(void *mem)
{
  int i;

  /* allocate temporary buffer */
  bufferptr = 0;

  /*** Save VDP state ***/
  memcpy (&state[bufferptr], &vdp, sizeof (vdp_t));
  bufferptr += sizeof (vdp_t);

  /*** Save SMS Context ***/
  memcpy (&state[bufferptr], &sms, sizeof (sms_t));
  bufferptr += sizeof (sms_t);

  /*** Save cart info ***/
  for (i = 0; i < 4; i++)
  {
      memcpy (&state[bufferptr], &cart.fcr[i], 1);
      bufferptr++;
  }

  /*** Save SRAM ***/
  memcpy (&state[bufferptr], &cart.sram, 0x8000);
  bufferptr += 0x8000;

  /*** Save Z80 Context ***/
  memcpy (&state[bufferptr], &Z80, sizeof (Z80_Regs));
  bufferptr += sizeof (Z80_Regs);

  /*** Save YM2413 ***/
  memcpy (&state[bufferptr], FM_GetContextPtr (), FM_GetContextSize ());
  bufferptr += FM_GetContextSize ();

  /*** Save SN76489 ***/
  memcpy (&state[bufferptr], SN76489_GetContextPtr (0),SN76489_GetContextSize ());
  bufferptr += SN76489_GetContextSize ();

#ifdef NGC
  /* compress state file */
  unsigned long inbytes  = bufferptr;
  unsigned long outbytes = 0x12000;
  compress2 (mem + sizeof(outbytes), &outbytes, (char *) &state[0], inbytes, 9);

  /* write compressed size in the first 32 bits for decompression */
  memcpy(mem, &outbytes, sizeof(outbytes));

  /* return total size */
  return (sizeof(outbytes) + outbytes);

#else
  /* write to FILE */
  fwrite(&state[0], 0x10000, 1, mem);
  return 0;
#endif

}

void system_load_state(void *mem)
{
  int i;
  uint8 *buf;

#ifdef NGC
  unsigned long inbytes, outbytes;

  /* get compressed state size */
  memcpy(&inbytes,mem,sizeof(inbytes));

  /* uncompress state file */
  outbytes = 0x10000;
  uncompress ((char *) &state[0], &outbytes, mem + sizeof(inbytes), inbytes);
#else
  /* write to FILE */
  fread(&state[0], 0x10000, 1, mem);
#endif
	
  /* Initialize everything */
  bufferptr = 0;
  system_reset();
   
  /*** Set vdp state ***/
  memcpy (&vdp, &state[bufferptr], sizeof (vdp_t));
  bufferptr += sizeof (vdp_t);
 
  /*** Set SMS Context ***/
  memcpy (&sms, &state[bufferptr], sizeof (sms_t));
  bufferptr += sizeof (sms_t);

  /** restore video & audio settings (needed if timing changed) ***/
  vdp_init();
  sound_init();

  /*** Set cart info ***/
  for (i = 0; i < 4; i++)
  {
     memcpy (&cart.fcr[i], &state[bufferptr], 1);
     bufferptr++;
  }

  /*** Set SRAM ***/
  memcpy (&cart.sram, &state[bufferptr], 0x8000);
  bufferptr += 0x8000;

  /*** Set Z80 Context ***/
  memcpy (&Z80, &state[bufferptr], sizeof (Z80_Regs));
  bufferptr += sizeof (Z80_Regs);

  /*** Set YM2413 ***/
  buf = malloc(FM_GetContextSize());
  memcpy (buf, &state[bufferptr], FM_GetContextSize ());
  FM_SetContext(buf);
  free(buf);
  bufferptr += FM_GetContextSize ();

  /*** Set SN76489 ***/
  memcpy (SN76489_GetContextPtr(0), &state[bufferptr], SN76489_GetContextSize ());
  bufferptr += SN76489_GetContextSize ();

  /* Restore mapping */
  slot.rom    = cart.rom;
  slot.pages  = cart.pages;
  slot.mapper = cart.mapper;
  slot.fcr = &cart.fcr[0];
  mapper_reset();

  for(i = 0x00; i <= 0x2F; i++)
  {
	  cpu_readmap[i]  = &cart.rom[(i & 0x1F) << 10];
      cpu_writemap[i] = dummy_write;
  }

  for(i = 0x30; i <= 0x3F; i++)
  {
     cpu_readmap[i] = &sms.wram[(i & 0x07) << 10];
     cpu_writemap[i] = &sms.wram[(i & 0x07) << 10];
  }

  sms_mapper_w(3, cart.fcr[3]);
  sms_mapper_w(2, cart.fcr[2]);
  sms_mapper_w(1, cart.fcr[1]);
  sms_mapper_w(0, cart.fcr[0]);

  /* Force full pattern cache update */
  bg_list_index = 0x200;
  for(i = 0; i < 0x200; i++)
  {
     bg_name_list[i] = i;
     bg_name_dirty[i] = -1;
  }

  /* Restore palette */
  for(i = 0; i < PALETTE_SIZE; i++) palette_sync(i, 1);

}
