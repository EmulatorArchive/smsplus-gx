/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *  additionnal code by Eke-Eke (SMS Plus GX)
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
 *   Sega Master System manager
 *
 ******************************************************************************/

#include "shared.h"

bitmap_t bitmap;
cart_t cart;
input_t input;

static int line_z80 = 0;

/* Run the virtual console emulation for one frame */
void system_frame(int skip_render)
{
  int iline;

  /* adjust Z80 cycle count from previous frame */
  z80_cycle_count -= line_z80;
  line_z80 = 0;

  /* Debounce pause key */
  if(input.system & INPUT_PAUSE)
  {
    if(!sms.paused)
    {
      sms.paused = 1;
      z80_set_irq_line(INPUT_LINE_NMI, ASSERT_LINE);
      z80_set_irq_line(INPUT_LINE_NMI, CLEAR_LINE);
    }
  }
  else
  {
     sms.paused = 0;
  }

  /* reset TMS Text offset counter */
  text_counter = 0;

  /* End of frame, parse sprites for line 0 on line 261 (VCount=$FF) */
  if(vdp.mode <= 7) parse_line(0);
  else  parse_satb(0);

  /* 3D glasses faking */
  if (sms.glasses_3d) skip_render = sms.wram[0x1ffb];

  /* VDP register 9 is latched during VBLANK */
  vdp.vscroll = vdp.reg[9];

  /* reset Horizontal counter */
  vdp.left = vdp.reg[0x0A];

  /* reset collision flag infos */
  vdp.spr_col = 0xff00;

  for(vdp.line = 0; vdp.line < vdp.lpf; vdp.line++)
  {
    iline = vdp.height;

    if(!skip_render)
    {
      render_line(vdp.line);
    }

    line_z80 += CYCLES_PER_LINE;

    /* Horizontal Interrupt */
    if (sms.console >= CONSOLE_SMS)
    {
      if(vdp.line <= iline)
      {
        if(--vdp.left < 0)
        {
          vdp.left = vdp.reg[0x0A];
          vdp.hint_pending = 1;
          if(vdp.reg[0x00] & 0x10)
          {
            /* IRQ line is latched between instructions, on instruction last cycle          */
            /* This means that if Z80 cycle count is exactly a multiple of CYCLES_PER_LINE, */
            /* interrupt would be triggered  AFTER the next instruction.                    */
            if (!(z80_get_elapsed_cycles()%CYCLES_PER_LINE))
              z80_execute(1);
              
            z80_set_irq_line(0, ASSERT_LINE);
          }
        }
      }
    }

    z80_execute(line_z80 - z80_cycle_count);

    sound_update(vdp.line);

    /* Vertical Interrupt */
    if(vdp.line == iline)
    {
      vdp.status |= 0x80;
      vdp.vint_pending = 1;
      if(vdp.reg[0x01] & 0x20)
      {
        z80_set_irq_line(vdp.irq, ASSERT_LINE);
      }
    }
  }
}

void system_init(void)
{
  error_init();
  sms_init();
  pio_init();
  vdp_init();
  render_init();
  sound_init();
}

void system_shutdown(void)
{
  sms_shutdown();
  pio_shutdown();
  vdp_shutdown();
  render_shutdown();
  sound_shutdown();
  error_shutdown();
}

void system_reset(void)
{
  sms_reset();
  pio_reset();
  vdp_reset();
  render_reset();
  sound_reset();
  system_manage_sram(cart.sram, SLOT_CART, SRAM_LOAD);
  line_z80 = 0;
}


void system_poweron(void)
{
  system_init();
  system_reset();
}

void system_poweroff(void)
{
  system_manage_sram(cart.sram, SLOT_CART, SRAM_SAVE);
}
