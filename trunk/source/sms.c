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
 *   Sega Master System console emulation
 *
 ******************************************************************************/

#include "shared.h"

/* SMS context */
sms_t sms;

/* BIOS/CART ROM */
bios_t bios;
slot_t slot;

uint8 dummy_write[0x400];
uint8 dummy_read[0x400];

static void writemem_mapper_none(int offset, int data)
{
  cpu_writemap[offset >> 10][offset & 0x03FF] = data;
}

static void writemem_mapper_sega(int offset, int data)
{
  cpu_writemap[offset >> 10][offset & 0x03FF] = data;
  if(offset >= 0xFFFC) sms_mapper_w(offset & 3, data);
}

static void writemem_mapper_codies(int offset, int data)
{
  switch(offset)
  {
    case 0x0000:
      sms_mapper_w(1, data);
      return;
    case 0x4000:
      sms_mapper_w(2, data);
      return;
    case 0x8000:
      sms_mapper_w(3, data);
      return;
    default:
      cpu_writemap[offset >> 10][offset & 0x03FF] = data;
      return;
  }
}

static void writemem_mapper_korean(int offset, int data)
{
  if (offset == 0xA000) sms_mapper_w(3, data);
  else cpu_writemap[offset >> 10][offset & 0x03FF] = data;
}


void mapper_reset(void)
{
  switch(slot.mapper)
  {
    case MAPPER_CODIES:
      cpu_writemem16 = writemem_mapper_codies;
      break;

    case MAPPER_NONE:
      cpu_writemem16 = writemem_mapper_none;
      break;

    case MAPPER_KOREAN:
      cpu_writemem16 = writemem_mapper_korean;
      break;

    case MAPPER_SEGA:
    default:
      cpu_writemem16 = writemem_mapper_sega;
      break;
  }
}

void sms_init(void)
{
  z80_init(0,0,0,sms_irq_callback);

  /* Default: open bus */
  data_bus_pullup   = 0x00;
  data_bus_pulldown = 0x00;

  /* FM unit */
  sms.use_fm = option.fm_enable;

  /* Initialize selected console emulation */
  switch(sms.console)
  {
    case CONSOLE_SG1000:
    case CONSOLE_SC3000:
    case CONSOLE_SF7000:
      cpu_writeport16 = tms_port_w;
      cpu_readport16 = tms_port_r;
      break;

    case CONSOLE_SMS:
      cpu_writeport16 = sms_port_w;
      cpu_readport16 = sms_port_r;
      break;

    case CONSOLE_SMS2:
      cpu_writeport16 = sms_port_w;
      cpu_readport16 = sms_port_r;
      data_bus_pullup = 0xFF;
      break;

    case CONSOLE_GG:
      cpu_writeport16 = gg_port_w;
      cpu_readport16 = gg_port_r;
      data_bus_pullup = 0xFF;
      break;

    case CONSOLE_GGMS:
      cpu_writeport16 = ggms_port_w;
      cpu_readport16 = ggms_port_r;
      data_bus_pullup = 0xFF;
      break;

    case CONSOLE_GEN:
    case CONSOLE_MD:
      cpu_writeport16 = md_port_w;
      cpu_readport16 = md_port_r;
      break;

    case CONSOLE_GENPBC:
    case CONSOLE_MDPBC:
      cpu_writeport16 = md_port_w;
      cpu_readport16 = md_port_r;
      data_bus_pullup = 0xFF;
      break;
  }
}

void sms_shutdown(void)
{
  /* Nothing to do */
}

void sms_reset(void)
{
  int i;

  /* Reset Z80 state */
  z80_reset();
  z80_reset_cycle_count();
  z80_set_irq_line (0, CLEAR_LINE);

  /* Clear SMS context */
  memset(dummy_write,data_bus_pullup, sizeof(dummy_write));
  memset(dummy_read,data_bus_pullup, sizeof(dummy_read));
  memset(sms.wram,0, sizeof(sms.wram));
  sms.paused    = 0x00;
  sms.save      = 0x00;
  sms.fm_detect = 0x00;
  sms.ioctrl    = 0xFF;
  sms.hlatch    = 0x00;

  /* reset BIOS paging */
  bios.fcr[0] = 0;
  bios.fcr[1] = 0;
  bios.fcr[2] = 1;
  bios.fcr[3] = 2;

  /* reset CART paging */
  switch(cart.mapper)
  {
    case MAPPER_CODIES:
    case MAPPER_KOREAN:
      cart.fcr[0] = 0;
      cart.fcr[1] = 0;
      cart.fcr[2] = 1;
      cart.fcr[3] = 0;
      break;

    default:
      cart.fcr[0] = 0;
      cart.fcr[1] = 0;
      cart.fcr[2] = 1;
      cart.fcr[3] = 2;
      break;
  }

  /* pick default SLOT */
  if (bios.enabled == 3)
  {
    /* Bios ROM */
    sms.memctrl = 0xE0;
    slot.rom    = bios.rom;
    slot.pages  = bios.pages;
    slot.mapper = MAPPER_SEGA;
    slot.fcr    = &bios.fcr[0];
  }
  else
  {
    /* Cartridge ROM */
    sms.memctrl = 0xAB;
    slot.rom    = cart.rom;
    slot.pages  = cart.pages;
    slot.mapper = cart.mapper;
    slot.fcr    = &cart.fcr[0];
    cart.loaded = 1;

    /* save Memory Control register value */
    if (IS_SMS)
    {
      sms.wram[0] = sms.memctrl;
    }
  }

  /* reset SLOT mapper */
  mapper_reset();

  /* reset Memory Map */
  if (sms.console < CONSOLE_SMS)
  {
    /* no mapper (SG-1000, SC-3000, SF-7000) */
    /* $0000-$7FFF mapped to cartridge ROM (max. 32K) */
    for(i = 0x00; i <= 0x1F; i++)
    {
      cpu_readmap[i]  = &cart.rom[(i << 10)];
      cpu_writemap[i] = dummy_write;
    }

    /* $8000-$BFFF mapped to external RAM (lower 16K) */
    for(i = 0x20; i <= 0x2F; i++)
    {
      cpu_readmap[i]  = &cart.sram[((i & 0x0F) << 10)];
      cpu_writemap[i] = &cart.sram[((i & 0x0F) << 10)];
    }

    /* $C000-$BFFF mapped to internal RAM (2K) or external RAM (upper 16K) */
    for(i = 0x30; i <= 0x3F; i++)
    {
      cpu_readmap[i]  = &cart.sram[0x4000 + ((i & 0x0F) << 10)];
      cpu_writemap[i] = &cart.sram[0x4000 + ((i & 0x0F) << 10)];
    }
  }
  else
  {
    /* $0000-$03ff (ROM) */
    cpu_readmap[0]  = &slot.rom[0];
    cpu_writemap[0] = dummy_write;

    /* $0400-$FFFF (ROM/RAM)*/
    sms_mapper_w(0,slot.fcr[0]);
    sms_mapper_w(1,slot.fcr[1]);
    sms_mapper_w(2,slot.fcr[2]);
    sms_mapper_w(3,slot.fcr[3]);
  }
}

void sms_mapper_w(int address, int data)
{
  int i;

  /* Calculate ROM page index (incl. previous increment set in $FFFC register) */
  uint8 page = (slot.fcr[0] & 3) ? ((data + (4 - (slot.fcr[0] & 3))*8) % slot.pages) : (data % slot.pages);

  /* Save frame control register data */
  slot.fcr[address] = data;

  switch(address)
  {
    case 0:
      if(data & 0x08)
      {
        /* external RAM (upper or lower 16K) mapped at $8000-$BFFF */
        sms.save = 1;
        uint32 offset = (data & 0x04) ? 0x4000 : 0x0000;
        for(i = 0x20; i <= 0x2F; i++)
        {
          cpu_writemap[i] = cpu_readmap[i]  = &cart.sram[offset + ((i & 0x0F) << 10)];
        }
      }
      else
      {
        /* ROM mapped at $8000-$BFFF */
        for(i = 0x20; i <= 0x2F; i++)
        {
          cpu_readmap[i] = &slot.rom[((slot.fcr[3] % slot.pages) << 14) | ((i & 0x0F) << 10)];
          cpu_writemap[i] = dummy_write;
        }
      }

      if(data & 0x10)
      {
        /* external RAM (upper 16K) mapped at $C000-$FFFF */
        sms.save = 1;
        for(i = 0x30; i <= 0x3F; i++)
        {
          cpu_writemap[i] = cpu_readmap[i]  = &cart.sram[0x4000 + ((i & 0x0F) << 10)];
        }
      }
      else
      {
        /* internal RAM (8K mirrorred) mapped at $8000-$BFFF */
        for(i = 0x30; i <= 0x3F; i++)
        {
          cpu_writemap[i] = cpu_readmap[i] = &sms.wram[(i & 0x07) << 10];
        }
      }
      break;

    case 1:
      if (slot.mapper == MAPPER_CODIES)
      {
        /* First 1 Kbyte is not fixed */
        cpu_readmap[0] = &slot.rom[(page << 14)];
      }

      for(i = 0x01; i <= 0x0F; i++)
      {
        cpu_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
      }
      break;

    case 2:
      if (slot.mapper == MAPPER_CODIES)
      {
        if (data & 0x80)
        {
          /* internal RAM mapped at $A000-$C000 */
          sms.save = 1;
          for(i = 0x28; i <= 0x2F; i++)
          {
            cpu_writemap[i] = cpu_readmap[i]  = &cart.sram[(i & 0x0F) << 10];
          }
        }
        else
        {
          /* ROM mapped at $A000-$C000 */
          for(i = 0x28; i <= 0x2F; i++)
          {
            cpu_readmap[i] = &slot.rom[((slot.fcr[3] % slot.pages) << 14) | ((i & 0x0F) << 10)];
            cpu_writemap[i] = dummy_write;
          }
        }
      }

      for(i = 0x10; i <= 0x1F; i++)
      {
        cpu_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
      }
      break;

    case 3:
      if(!(slot.fcr[0] & 0x08))
      {
        for(i = 0x20; i <= 0x2F; i++)
        {
          cpu_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
          cpu_writemap[i] = dummy_write;
        }
      }
      break;
  }
}

int sms_irq_callback(int param)
{
  return 0xFF;
}
