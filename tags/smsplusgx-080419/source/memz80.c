/*
    memz80.c --
    Z80 port handlers.
*/
#include "shared.h"

/* Pull-up resistors on data bus */
uint8 data_bus_pullup   = 0x00;
uint8 data_bus_pulldown = 0x00;

/* Read unmapped memory */
uint8 z80_read_unmapped(void)
{
    int pc = Z80.pc.w.l;
    uint8 data;
    pc = (pc - 1) & 0xFFFF;
    data = cpu_readmap[pc >> 13][pc & 0x03FF];
    return ((data | data_bus_pullup) & ~data_bus_pulldown);
}

/* Port $3E (Memory Control Port) */
void memctrl_w(uint8 data)
{
	/* detect CARTRIDGE/BIOS enabled/disabled */
	if (IS_SMS)
	{
		/* autodetect loaded BIOS ROM */
		if ((bios.enabled != 3) && ((data & 0xE8) == 0xE8))
		{
			bios.enabled |= 2;
			memcpy(bios.rom, cart.rom, cart.size);
			bios.pages = cart.pages;
			cart.loaded = 0;
		}

		switch (data & 0x48)
		{
			case 0x00:
			case 0x08:
				/* enables CART */
				if (cart.loaded) slot.rom   = cart.rom;
				else slot.rom = NULL;
				slot.pages = cart.pages;
				slot.mapper = cart.mapper;
				slot.fcr = &cart.fcr[0];

				/* reset CART mapping */
				cart.fcr[0] = 0x00;
				cart.fcr[1] = 0x00;
				cart.fcr[2] = 0x01;
				cart.fcr[3] = 0x00;
				break;
			
			case 0x40:
				/* enables BIOS */
				slot.rom   = bios.rom;
				slot.pages = bios.pages;
				slot.mapper = MAPPER_SEGA;
				slot.fcr = &cart.fcr[0];
				break;
			
			default:
				/* disables CART & BIOS */
				slot.rom   = NULL;
				break;
		}
		
		/* reset SLOT mapping */
		if (slot.rom)
		{
			mapper_reset();
			cpu_readmap[0]  = slot.rom ? &slot.rom[0] : dummy_read;
			cpu_writemap[0] = dummy_write;
			sms_mapper_w(3, slot.fcr[3]);
			sms_mapper_w(2, slot.fcr[2]);
			sms_mapper_w(1, slot.fcr[1]);
			sms_mapper_w(0, slot.fcr[0]);
		}
		else
		{
			uint8 i;
			for(i = 0x00; i <= 0x2F; i++)
			{
				cpu_readmap[i]  = dummy_read;
				cpu_writemap[i] = dummy_write;
			}
		}
	}

    sms.memctrl = data;	
}

/*--------------------------------------------------------------------------*/
/* Sega Master System port handlers                                         */
/*--------------------------------------------------------------------------*/
void sms_port_w(uint16 port, uint8 data)
{
    port &= 0xFF;

    /* access FM unit */
	if(port >= 0xF0)
    {
        switch(port)
        {
            case 0xF0:
                fmunit_write(0, data);
                return;

            case 0xF1:
                fmunit_write(1, data);
                return;

            case 0xF2:
                fmunit_detect_w(data);
                return;
        }
    }

    switch(port & 0xC1)
    {
        case 0x00:
            memctrl_w(data);
            return;

        case 0x01:
            ioctrl_w(data);
            return;

        case 0x40:
        case 0x41:
            psg_write(data);
            return;

        case 0x80:
        case 0x81:
            vdp_write(port, data);
            return;

        case 0xC0:
        case 0xC1:
            return;
    }
}

uint8 sms_port_r(uint16 port)
{
    port &= 0xFF;

    /* IO port disabled: access FM unit (fixed) */
	if((port == 0xF2) /*&& (sms.memctrl & 4)*/)
        return fmunit_detect_r();

    switch(port & 0xC0)
    {
        case 0x00:
            return z80_read_unmapped();

        case 0x40:
            return vdp_counter_r(port);

        case 0x80:
            return vdp_read(port);

        case 0xC0:
            return input_r(port);
    }

    /* Just to please the compiler */
    return -1;
}

/*--------------------------------------------------------------------------*/
/* Game Gear port handlers                                                  */
/*--------------------------------------------------------------------------*/

void gg_port_w(uint16 port, uint8 data)
{
    port &= 0xFF;

    if(port <= 0x06) {
        sio_w(port, data);
        return;
    }

    switch(port & 0xC1)
    {
        case 0x00:
            memctrl_w(data);
            return;

        case 0x01:
            ioctrl_w(data);
            return;

        case 0x40:
        case 0x41:
            psg_write(data);
            return;

        case 0x80:
        case 0x81:
            gg_vdp_write(port, data);
            return;
    }
}


uint8 gg_port_r(uint16 port)
{
    port &= 0xFF;

    if(port <= 0x06)
        return sio_r(port);

    switch(port & 0xC0)
    {
        case 0x00:
            return z80_read_unmapped();

        case 0x40:
            return vdp_counter_r(port);

        case 0x80:
            return vdp_read(port);

        case 0xC0:
            switch(port)
            {
                case 0xC0:
                case 0xC1:
                case 0xDC:
                case 0xDD:
                    return input_r(port);
            }
            return z80_read_unmapped();
    }

    /* Just to please the compiler */
    return -1;
}

/*--------------------------------------------------------------------------*/
/* Game Gear (MS) port handlers                                             */
/*--------------------------------------------------------------------------*/

void ggms_port_w(uint16 port, uint8 data)
{
    port &= 0xFF;

    switch(port & 0xC1)
    {
        case 0x00:
            memctrl_w(data);
            return;

        case 0x01:
            ioctrl_w(data);
            return;

        case 0x40:
        case 0x41:
            psg_write(data);
            return;

        case 0x80:
        case 0x81:
            vdp_write(port, data); /* fixed */
            return;
    }
}

uint8 ggms_port_r(uint16 port)
{
    port &= 0xFF;

    switch(port & 0xC0)
    {
        case 0x00:
            return z80_read_unmapped();

        case 0x40:
            return vdp_counter_r(port);

        case 0x80:
            return vdp_read(port);

        case 0xC0:
            switch(port)
            {
                case 0xC0:
                case 0xC1:
                case 0xDC:
                case 0xDD:
                    return input_r(port);
            }
            return z80_read_unmapped();
    }

    /* Just to please the compiler */
    return -1;
}

/*--------------------------------------------------------------------------*/
/* MegaDrive / Genesis port handlers                                        */
/*--------------------------------------------------------------------------*/

void md_port_w(uint16 port, uint8 data)
{
    switch(port & 0xC1)
    {
        case 0x00:
            /* No memory control register */
            return;

        case 0x01:
            ioctrl_w(data);
            return;

        case 0x40:
        case 0x41:
            psg_write(data);
            return;

        case 0x80:
        case 0x81:
            md_vdp_write(port, data);
            return;
    }
}


uint8 md_port_r(uint16 port)
{
    switch(port & 0xC0)
    {
        case 0x00:
            return z80_read_unmapped();

        case 0x40:
            return vdp_counter_r(port);

        case 0x80:
            return vdp_read(port);

        case 0xC0:
            switch(port)
            {
                case 0xC0:
                case 0xC1:
                case 0xDC:
                case 0xDD:
                    return input_r(port);
            }
            return z80_read_unmapped();
    }

    /* Just to please the compiler */
    return -1;
}

