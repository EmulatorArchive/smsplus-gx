/*
    pio.c --
    I/O chip and peripheral emulation.
*/
#include "shared.h"
#include "math.h"

extern uint8 *hc_table[];

io_state io_lut[2][256];
io_state *io_current;

void pio_init(void)
{
    int i, j;

    /* Make pin state LUT */
    for(j = 0; j < 2; j++)
    {
        for(i = 0; i < 0x100; i++)
        {
            /* Common control: pin direction */
            io_lut[j][i].tr_dir[0]   = (i & 0x01) ? PIN_DIR_IN : PIN_DIR_OUT;
            io_lut[j][i].th_dir[0]   = (i & 0x02) ? PIN_DIR_IN : PIN_DIR_OUT;
            io_lut[j][i].tr_dir[1]   = (i & 0x04) ? PIN_DIR_IN : PIN_DIR_OUT;
            io_lut[j][i].th_dir[1]   = (i & 0x08) ? PIN_DIR_IN : PIN_DIR_OUT; 

            if(j == 1)
            {
                /* Programmable output state (Export machines only) */
                io_lut[j][i].tr_level[0] = (i & 0x01) ? PIN_LVL_HI : (i & 0x10) ? PIN_LVL_HI : PIN_LVL_LO;
                io_lut[j][i].th_level[0] = (i & 0x02) ? PIN_LVL_HI : (i & 0x20) ? PIN_LVL_HI : PIN_LVL_LO;
                io_lut[j][i].tr_level[1] = (i & 0x04) ? PIN_LVL_HI : (i & 0x40) ? PIN_LVL_HI : PIN_LVL_LO;
                io_lut[j][i].th_level[1] = (i & 0x08) ? PIN_LVL_HI : (i & 0x80) ? PIN_LVL_HI : PIN_LVL_LO;
            }
            else
            {
                /* Fixed output state (Domestic machines only) */
                io_lut[j][i].tr_level[0] = (i & 0x01) ? PIN_LVL_HI : PIN_LVL_LO;
                io_lut[j][i].th_level[0] = (i & 0x02) ? PIN_LVL_HI : PIN_LVL_LO;
                io_lut[j][i].tr_level[1] = (i & 0x04) ? PIN_LVL_HI : PIN_LVL_LO;
                io_lut[j][i].th_level[1] = (i & 0x08) ? PIN_LVL_HI : PIN_LVL_LO;
            }
        }
    }
}


void pio_reset(void)
{
    /* GG SIO power-on defaults */
    sms.sio.pdr     = 0x7F;
    sms.sio.ddr     = 0xFF;
    sms.sio.txdata  = 0x00;
    sms.sio.rxdata  = 0xFF;
    sms.sio.sctrl   = 0x00;

    /* SMS I/O power-on defaults */
    ioctrl_w(0xFF);
}


void pio_shutdown(void)
{
    /* Nothing to do */
}


void system_assign_device(int port, int type)
{
    sms.device[port].type = type;
}

void ioctrl_w(uint8 data)
{
	uint8 th_level[2];

	/* save old TH values */
    io_current = &io_lut[sms.territory][sms.ioctrl];
	th_level[0] = io_current->th_level[0];
	th_level[1] = io_current->th_level[1];
	
	/* check TH Low->High transition */
    io_current = &io_lut[sms.territory][data];
	if (((th_level[0] == PIN_LVL_LO) && (io_current->th_level[0] == PIN_LVL_HI)) ||
		((th_level[1] == PIN_LVL_LO) && (io_current->th_level[1] == PIN_LVL_HI)))
	{
		sms.hlatch = hc_table[0][z80_get_elapsed_cycles() % CYCLES_PER_LINE];
	}

    sms.ioctrl = data;
}

uint8 device_r(int offset)
{
    uint8 temp = 0x7F;

    switch(sms.device[offset].type)
    {
        case DEVICE_NONE:
            break;
        case DEVICE_PAD2B:
            break;
        case DEVICE_PADDLE:
            break;
    }

    return temp;
}

static int paddle_flip_flop = 0;

uint8 input_r(int offset)
{
    uint8 temp = 0xFF;

	int pixel;
	uint16 spot_x;
	uint16 spot_y;

    /* If I/O chip is disabled, reads return last byte of instruction that
       read the I/O port.
    */
    if(sms.memctrl & 0x04) return z80_read_unmapped();

    offset &= 1;
    
	/* Input port #0 ($DC register)*/
	if(offset == 0)
    {
        /* bits D7, D6 */
		if (IS_GG)
		{
			uint8 state = sio_r(0x01);
			temp = (temp & 0x3F) | (state & 0x03) << 6; /* Insert D1,D0 */
		}
		else
		{
			if(input.pad[1] & INPUT_UP) temp &= ~0x40; /* D0 */
			if(input.pad[1] & INPUT_DOWN) temp &= ~0x80; /* D1 */
		}

		/* bits D5-D0 */
		if (IS_SMS && sms.paddle)
		{			
			/* PADDLE (P1 only) */
			/* based on the documentation from smspower.org
			   http://www.smspower.org/dev/docs/wiki/InputAndOutput/Paddle
			   http://www.smspower.org/forums/viewtopic.php?t=6783
			*/

			/* TL bit */
			if(input.pad[0] & INPUT_BUTTON1) temp &= ~0x10;

			/* paddle type */
			if (sms.territory == TERRITORY_EXPORT)
			{
				/* non-japanese mode: TH output control */
				paddle_flip_flop = (io_current->th_level[0] == PIN_LVL_LO);
			}
			else
			{
				/* japanese mode: automatic flip-flop */
				paddle_flip_flop ^= 1;
			}

			/* get position */
			if (paddle_flip_flop)
			{
				/* position low bits */
				temp = (temp & 0xF0) | (input.paddle & 0x0F); 
					
				/* TR = 0 */
				temp &= ~0x20;
			}
			else
			{
				/* position high bits */
				temp = (temp & 0xF0) | ((input.paddle >> 4) & 0x0F);
			}
		}
		
		/* GAMEPAD #1 */
		else
		{
			if(input.pad[0] & INPUT_UP)         temp &= ~0x01; /* D0 */
			if(input.pad[0] & INPUT_DOWN)       temp &= ~0x02; /* D1 */
			if(input.pad[0] & INPUT_LEFT)       temp &= ~0x04; /* D2 */
			if(input.pad[0] & INPUT_RIGHT)      temp &= ~0x08; /* D3 */
			if(input.pad[0] & INPUT_BUTTON1)    temp &= ~0x10; /* TL */

			/* TR */
			if(io_current->tr_dir[0] == PIN_DIR_OUT)
			{
				/* Adjust TR state if it is an output */
				temp &= ~0x20;
				temp |= (io_current->tr_level[0] == PIN_LVL_HI) ? 0x20 : 0x00;
			}
			else if(input.pad[0] & INPUT_BUTTON2) temp &= ~0x20;
		}
    }
	else
    {
		/* Input port #1 ($DD register) */
        if (IS_GG)
        {
            uint8 state = sio_r(0x01);
            temp = (temp & 0xF0) | ((state & 0x3C) >> 2); /* Insert TR,TL,D3,D2 */
            temp = (temp & 0x7F) | ((state & 0x40) << 1); /* Insert TH */
        }
        else
        {
            if(input.pad[1] & INPUT_LEFT)       temp &= ~0x01; /* D2 */
            if(input.pad[1] & INPUT_RIGHT)      temp &= ~0x02; /* D3 */
            if(input.pad[1] & INPUT_BUTTON1)    temp &= ~0x04; /* TL */
            
            /* TR */
			if(io_current->tr_dir[1] == PIN_DIR_OUT)
			{
                /* Adjust TR state if it is an output */
				temp &= ~0x08;
                temp |= (io_current->tr_level[1] == PIN_LVL_HI) ? 0x08 : 0x00;
            }
			else if(input.pad[1] & INPUT_BUTTON2) temp &= ~0x08; /* TR */

			/* TH2 */
			if(io_current->th_dir[1] == PIN_DIR_OUT)
			{
                /* Adjust TH state if it is an output */
				temp &= ~0x80;
                temp |= (io_current->th_level[1] == PIN_LVL_HI) ? 0x80 : 0x00;
            }
			else if (sms.light_phaser)
			{
				/* 2P LightGun is activated */
				/* basically, the light phaser activates TH when it detects the TV light beam */
				/* we emulate this by comparing our current "LightGun" position with HV counter values */
				/* the value must be set long enough as the software generally make several access to */
				/* get an average value because the lightgun see a circular spot, not a single pixel */

				/* The system latches the current HV counter values on TH high-to-low transition  */
				/* This is used by software to enable/disable LightGuns and support 2 players simultaneously */
				
				/* check if current pixel beam is inside the circular spot */
				pixel = (((z80_get_elapsed_cycles() % CYCLES_PER_LINE) / 4) * 3) * 2;
				spot_y = abs(vdp.line - (input.lightgun_y[1]>>8));
 				if (spot_y <= 5)
				{
					spot_x = abs(pixel - (input.lightgun_x[1]>>9));
					if (spot_x <= ((5 - spot_y) * 7 / 5))
					{
						/* set TH = 0 */
						temp &= ~0x80;

						/* keep latched H value (left boundary of the circular spot) */
						pixel = (input.lightgun_x[1]>>9) - ((5 - spot_y) * 7 / 5);
						sms.hlatch = hc_table[0][(pixel >> 1) & 0x01FF];
					}
				}
			}

            if (input.system & INPUT_RESET)  temp &= ~0x10;
        }

        /* /CONT fixed at '1' for SMS/SMS2/GG */
        /* /CONT fixed at '0' for GEN/MD */
        if(IS_MD) temp &= ~0x20;

		/* TH1 */
		if(io_current->th_dir[0] == PIN_DIR_OUT)
		{
			/* Adjust TH state if it is an output */
			temp &= ~0x40;
            temp |= (io_current->th_level[0] == PIN_LVL_HI) ? 0x40 : 0x00;
        }
		else if (sms.light_phaser && IS_SMS)
		{
			/* 1P LightGun is activated */
			/* check if current pixel beam is inside the circular spot */
			pixel = (((z80_get_elapsed_cycles() % CYCLES_PER_LINE) / 4) * 3) * 2;
			spot_y = abs(vdp.line - (input.lightgun_y[0]>>8));
 			if (spot_y <= 5)
			{
				spot_x = abs(pixel - (input.lightgun_x[0]>>9));
				if (spot_x <= ((5 - spot_y) * 7 / 5))
				{
					/* set TH = 0 */
					temp &= ~0x80;

					/* keep latched H value (left boundary of the circular spot) */
					pixel = (input.lightgun_x[0]>>9) - ((5 - spot_y) * 7 / 5);
					sms.hlatch = hc_table[0][(pixel >> 1) & 0x01FF];
				}
			}
		}
	}
	return temp;
}

/* Game Gear specific IO ports */
uint8 sio_r(int offset)
{
    uint8 temp;

    switch(offset & 0xFF)
    {
        case 0: /* Input port #2 */
            temp = 0xE0;
            if(input.system & INPUT_START)          temp &= ~0x80;
            if(sms.territory == TERRITORY_DOMESTIC) temp &= ~0x40;
            if(sms.display == DISPLAY_NTSC)         temp &= ~0x20;
            return temp;

        case 1: /* Parallel data register */
            temp = 0x00;
            temp |= (sms.sio.ddr & 0x01) ? 0x01 : (sms.sio.pdr & 0x01);
            temp |= (sms.sio.ddr & 0x02) ? 0x02 : (sms.sio.pdr & 0x02);
            temp |= (sms.sio.ddr & 0x04) ? 0x04 : (sms.sio.pdr & 0x04);
            temp |= (sms.sio.ddr & 0x08) ? 0x08 : (sms.sio.pdr & 0x08);
            temp |= (sms.sio.ddr & 0x10) ? 0x10 : (sms.sio.pdr & 0x10);
            temp |= (sms.sio.ddr & 0x20) ? 0x20 : (sms.sio.pdr & 0x20);
            temp |= (sms.sio.ddr & 0x40) ? 0x40 : (sms.sio.pdr & 0x40);
            temp |= (sms.sio.pdr & 0x80);
            return temp;

        case 2: /* Data direction register and NMI enable */
            return sms.sio.ddr;

        case 3: /* Transmit data buffer */
            return sms.sio.txdata;

        case 4: /* Receive data buffer */
            return sms.sio.rxdata;

        case 5: /* Serial control */
            return sms.sio.sctrl;

        case 6: /* Stereo sound control */
            return 0xFF;
    }

    /* Just to please compiler */
    return -1;
}

void sio_w(int offset, int data)
{
    switch(offset & 0xFF)
    {
        case 0: /* Input port #2 (read-only) */
            return;

        case 1: /* Parallel data register */
            sms.sio.pdr = data;
            return;

        case 2: /* Data direction register and NMI enable */
            sms.sio.ddr = data;
            return;

        case 3: /* Transmit data buffer */
            sms.sio.txdata = data;
            return;

        case 4: /* Receive data buffer */
            return;

        case 5: /* Serial control */
            sms.sio.sctrl = data & 0xF8;
            return;
        
        case 6: /* Stereo output control */
            psg_stereo_w(data);
            return;
    }
}
