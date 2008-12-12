/*
    sms.c --
    Sega Master System console emulation.
*/
#include "shared.h"

/* SMS context */
sms_t sms;

/* BIOS/CART ROM */
bios_t bios;
slot_t slot;

uint8 dummy_write[0x400];
uint8 dummy_read[0x400];

void writemem_mapper_none(int offset, int data)
{
  cpu_writemap[offset >> 10][offset & 0x03FF] = data;
}

void writemem_mapper_sega(int offset, int data)
{
  cpu_writemap[offset >> 10][offset & 0x03FF] = data;
  if(offset >= 0xFFFC) sms_mapper_w(offset & 3, data);
}

void writemem_mapper_codies(int offset, int data)
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

void writemem_mapper_korean(int offset, int data)
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

  z80_reset();
  z80_set_irq_line (0, ASSERT_LINE);
  z80_set_irq_line (0, CLEAR_LINE);

  /* Clear SMS context */
  memset(dummy_write, 0, sizeof(dummy_write));
  memset(dummy_read,  0, sizeof(dummy_read));
  memset(sms.wram,    0, sizeof(sms.wram));

  sms.paused    = 0x00;
  sms.save      = 0x00;
  sms.fm_detect = 0x00;
  sms.ioctrl    = 0xFF;
  sms.hlatch    = 0x00;
  slot.rom      = NULL;

  if (IS_SMS && (bios.enabled == 3))
  {
    sms.memctrl = 0xE0;
    slot.rom    = bios.rom;
    slot.pages  = bios.pages;
    slot.mapper = MAPPER_SEGA;
    slot.fcr    = &bios.fcr[0];
  }
  else
  {
    cart.loaded = 1;
    sms.memctrl = 0xAB;
    slot.rom    = cart.rom;
    slot.pages  = cart.pages;
    slot.mapper = cart.mapper;
    slot.fcr    = &cart.fcr[0];
  }

  mapper_reset();

  for(i = 0x00; i <= 0x2F; i++)
  {
    cpu_readmap[i]  = (i < (slot.pages * 0x4000)) ? &slot.rom[((i & 0x1F) << 10)] : dummy_read;
    cpu_writemap[i] = dummy_write;
  }

  for(i = 0x30; i <= 0x3F; i++)
  {
    cpu_readmap[i]  = &sms.wram[(i & 0x07) << 10];
    cpu_writemap[i] = &sms.wram[(i & 0x07) << 10];
  }

  slot.fcr[0] = 0x00;
  slot.fcr[1] = 0x00;
  slot.fcr[2] = 0x01;
  slot.fcr[3] = 0x00;
}

void sms_mapper_w(int address, int data)
{
  int i;

  /* Calculate ROM page index */
  uint8 page = (data % slot.pages);

  /* Save frame control register data */
  slot.fcr[address] = data;

  switch(address)
  {
    case 0:
      if (slot.mapper == MAPPER_SEGA)
      {
        if(data & 8)
        {
          uint32 offset = (data & 4) ? 0x4000 : 0x0000;
          sms.save = 1;

          for(i = 0x20; i <= 0x2F; i++)
          {
            cpu_writemap[i] = cpu_readmap[i]  = &cart.sram[offset + ((i & 0x0F) << 10)];
          }
        }
        else
        {
          for(i = 0x20; i <= 0x2F; i++)
          {
              cpu_readmap[i] = &slot.rom[((slot.fcr[3] % slot.pages) << 14) | ((i & 0x0F) << 10)];
              cpu_writemap[i] = dummy_write;
          }
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
          /* enable internal RAM at A000h-C000h */
          sms.save = 1;
          for(i = 0x28; i <= 0x2F; i++)
          {
            cpu_writemap[i] = cpu_readmap[i]  = &cart.sram[(i & 0x0F) << 10];
          }
        }
        else
        {
          for(i = 0x10; i <= 0x1F; i++)
          {
            cpu_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
          }

          /* enable ROM at A000h-C000h */
          for(i = 0x28; i <= 0x2F; i++)
          {
            cpu_readmap[i] = &slot.rom[((slot.fcr[3] % slot.pages) << 14) | ((i & 0x0F) << 10)];
            cpu_writemap[i] = dummy_write;
          }
        }
      }
      else
      {
        for(i = 0x10; i <= 0x1F; i++)
        {
          cpu_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
        }
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
