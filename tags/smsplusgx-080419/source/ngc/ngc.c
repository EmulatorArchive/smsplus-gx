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
 ***************************************************************************/
#include "shared.h"
#include "dvd.h"
#include "font.h"
#include <malloc.h>

#define ROMOFFSET 0x80700000

/***************************************************************************
 * Nintendo Gamecube Hardware Specific Functions
 *
 * V I D E O
 ***************************************************************************/
/*** VI ***/
unsigned int *xfb[2];	/*** Double buffered ***/
int whichfb = 0;		/*** Switch ***/
GXRModeObj *vmode;		/*** General video mode ***/

/*** GX ***/
#define TEX_WIDTH 284
#define TEX_HEIGHT 288
#define DEFAULT_FIFO_SIZE 256 * 1024
#define HASPECT 320
#define VASPECT 240

static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static u8 texturemem[TEX_WIDTH * (TEX_HEIGHT + 8) * 2] ATTRIBUTE_ALIGN (32);
GXTexObj texobj;
static Mtx view;
int vwidth, vheight, offset, shift;

/*** custom Video modes (used to emulate original console video modes) ***/
/* 288 lines progressive (PAL 50Hz) */
GXRModeObj TVPal_288p = 
{
    VI_TVMODE_PAL_DS,       // viDisplayMode
    640,             // fbWidth
    286,             // efbHeight
    286,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 720)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 572)/2,        // viYOrigin
    720,             // viWidth
    572,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},

    // vertical filter[7], 1/64 units, 6 bits each
	{
		 0,         // line n-1
		 0,         // line n-1
		21,         // line n
		22,         // line n
		21,         // line n
		 0,         // line n+1
		 0          // line n+1
	}
};

/* 576 lines interlaced (PAL 50Hz, scaled) */
GXRModeObj TVPal_576i_Scale = 
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    574,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 720)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 574)/2,        // viYOrigin
    720,             // viWidth
    574,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 8,         // line n-1
		 8,         // line n-1
		10,         // line n
		12,         // line n
		10,         // line n
		 8,         // line n+1
		 8          // line n+1
	}
};

/* 240 lines progressive (NTSC or PAL 60Hz) */
GXRModeObj TVNtsc_Rgb60_240p = 
{
    VI_TVMODE_EURGB60_DS,      // viDisplayMode
    640,             // fbWidth
    240,             // efbHeight
    240,             // xfbHeight
    (VI_MAX_WIDTH_NTSC - 720)/2,        // viXOrigin
    (VI_MAX_HEIGHT_NTSC/2 - 480/2)/2,       // viYOrigin
    720,             // viWidth
    480,             // viHeight
    VI_XFBMODE_SF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

     // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},

     // vertical filter[7], 1/64 units, 6 bits each
	{
		  0,         // line n-1
		  0,         // line n-1
		 21,         // line n
		 22,         // line n
		 21,         // line n
		  0,         // line n+1
		  0          // line n+1
	}
};

/* 480 lines interlaced (NTSC or PAL 60Hz) */
GXRModeObj TVNtsc_Rgb60_480i = 
{
    VI_TVMODE_EURGB60_INT,     // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    480,             // xfbHeight
    (VI_MAX_WIDTH_NTSC - 720)/2,        // viXOrigin
    (VI_MAX_HEIGHT_NTSC - 480)/2,       // viYOrigin
    720,             // viWidth
    480,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},

    // vertical filter[7], 1/64 units, 6 bits each
	{
		 8,         // line n-1
		 8,         // line n-1
		10,         // line n
		12,         // line n
		10,         // line n
		 8,         // line n+1
		 8          // line n+1
	}
};


/* TV Modes table */
GXRModeObj *tvmodes[4] = {
	 &TVNtsc_Rgb60_240p, &TVNtsc_Rgb60_480i, /* 60hz modes */
	 &TVPal_288p, &TVPal_576i_Scale                /* 50Hz modes */
};

typedef struct tagcamera
{
  Vector pos;
  Vector up;
  Vector view;
} camera;

/*** Square Matrix
     This structure controls the size of the image on the screen.
	 Think of the output as a -80 x 80 by -60 x 60 graph.
***/
s16 square[] ATTRIBUTE_ALIGN (32) =
{
  /*
   * X,   Y,  Z
   * Values set are for roughly 4:3 aspect
   */
	-HASPECT, VASPECT, 0,	// 0
	HASPECT, VASPECT, 0,	// 1
	HASPECT, -VASPECT, 0,	// 2
	-HASPECT, -VASPECT, 0,	// 3
};

static camera cam = { {0.0F, 0.0F, -100.0F},
{0.0F, -1.0F, 0.0F},
{0.0F, 0.0F, 0.0F}
};

/* init rendering */
/* should be called each time you change quad aspect ratio */
void draw_init (void)
{
  /* Clear all Vertex params */
  GX_ClearVtxDesc ();

  /* Set Position Params (set quad aspect ratio) */
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
  GX_SetVtxDesc (GX_VA_POS, GX_INDEX8);
  GX_SetArray (GX_VA_POS, square, 3 * sizeof (s16));

  /* Set Tex Coord Params */
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
  GX_SetTevOp (GX_TEVSTAGE0, GX_REPLACE);
  GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
  GX_SetNumTexGens (1);
  GX_SetNumChans(0);

  /** Set Modelview **/
  memset (&view, 0, sizeof (Mtx));
  guLookAt(view, &cam.pos, &cam.up, &cam.view);
  GX_LoadPosMtxImm (view, GX_PNMTX0);
}

/* vertex rendering */
static void draw_vert (u8 pos, f32 s, f32 t)
{
  GX_Position1x8 (pos);
  GX_TexCoord2f32 (s, t);
}

/* textured quad rendering */
static void draw_square ()
{
  GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
  draw_vert (3, 0.0, 0.0);
  draw_vert (2, 1.0, 0.0);
  draw_vert (1, 1.0, 1.0);
  draw_vert (0, 0.0, 1.0);
  GX_End ();
}

/* initialize GX rendering */
static void StartGX (void)
{
  Mtx p;
  GXColor gxbackground = { 0, 0, 0, 0xff };

  /*** Clear out FIFO area ***/
  memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

  /*** Initialise GX ***/
  GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
  GX_SetCopyClear (gxbackground, 0x00ffffff);
  GX_SetViewport (0.0F, 0.0F, vmode->fbWidth, vmode->efbHeight, 0.0F, 1.0F);
  GX_SetScissor (0, 0, vmode->fbWidth, vmode->efbHeight);
  f32 yScale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
  u16 xfbHeight = GX_SetDispCopyYScale (yScale);
  GX_SetDispCopySrc (0, 0, vmode->fbWidth, vmode->efbHeight);
  GX_SetDispCopyDst (vmode->fbWidth, xfbHeight);
  GX_SetCopyFilter (vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
  GX_SetFieldMode (vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  GX_SetPixelFmt (GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GX_SetCullMode (GX_CULL_NONE);
  GX_SetDispCopyGamma (GX_GM_1_0);
  GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
  GX_SetColorUpdate (GX_TRUE);
  guOrtho(p, vmode->efbHeight/2, -(vmode->efbHeight/2), -(vmode->fbWidth/2), vmode->fbWidth/2, 100, 1000);
  GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);

  /*** Copy EFB -> XFB ***/
  GX_CopyDisp (xfb[whichfb ^ 1], GX_TRUE);
  GX_Flush ();

  /*** Initialize texture data ***/
  memset (texturemem, 0, TEX_WIDTH * TEX_HEIGHT * 2);
}

/* PreRetrace handler */
static int frameticker = 0;
static void framestart()
{
  /* simply increment the tick counter */
  frameticker++;
}

/* Rendering options */
s16 xshift = 0;
s16 yshift = 0;
s16 xscale  = HASPECT;
s16 yscale  = VASPECT;
uint8 overscan = 1;
uint8 aspect = 1;
uint8 use_480i = 0;
uint8 tv_mode = 0;
uint8 gc_pal = 0;

/* init GX scaler */
void viewport_init()
{
	if (aspect)
	{
		/* original aspect */
		if (overscan)
		{
			if (sms.console == CONSOLE_GGMS)
			{
				/* SMS display scaled into GG screen */
				xscale = 182;
				yscale = 72;
				if (gc_pal && !use_480i) yscale = yscale * 288 / 243;
				xshift = 0;
				yshift = 0;
			}
			else
			{
				xscale = 320;
				yscale = sms.display ? ((gc_pal && !use_480i) ? 144 : 121) : ((gc_pal && !use_480i) ? 143 : 120);
				xshift = 8;
				yshift = sms.display ? (gc_pal ? 1 : 0) : 2;
			}
		}
		else
		{
			/* borders are emulated */
			if ((sms.console == CONSOLE_GGMS) || (sms.console == CONSOLE_GG))
			{
				/* SMS display scaled into GG screen */
				xscale = 182;
				yscale = 72;
				if (gc_pal && !use_480i) yscale = yscale * 288 / 243;
				xshift = 0;
				yshift = 0;
			}
			else
			{
				xscale = 290;
				yscale = bitmap.viewport.h / 2;
				if (sms.display && (!gc_pal || use_480i)) yscale = yscale * 243 / 288;
                else if (!sms.display && gc_pal && !use_480i) yscale = yscale * 288 / 243;
				xshift = 8;
				yshift = sms.display ? (gc_pal ? 1 : 0) : 2;
			}
		}
	}
	else
	{
		/* fit screen */
		xscale = 320;
		yscale = (gc_pal && !use_480i) ? 134 : 112;
		xshift = 0;
		yshift = gc_pal ? 1 : 2;
	}

	/* double resolution */
	if (use_480i)
	{
		 yscale *= 2;
		 yshift *= 2;
	}

	square[6] = square[3]  =  xscale + xshift;
	square[0] = square[9]  = -xscale + xshift;
	square[4] = square[1]  =  yscale + yshift;
	square[7] = square[10] = -yscale + yshift;

	draw_init();
}	

/* Update Display (called after each emulation frame) */
static void update_video ()
{
  int h, w;

  if (bitmap.viewport.changed)
  {
	  bitmap.viewport.changed = 0;
	  
      /* update texture size */
	  if ((sms.console == CONSOLE_GG) && !overscan)
	  {
		  /* Game Gear display is 160 x 144 pixels */
		  offset  = 96; /* 48 * bitmap.granularity */
		  vwidth  = 160;
		  vheight = 144;
		  shift   = 31; /* bitmap.granularity * (bitmap.width - width)/8 */	
	  }
	  else if ((sms.console == CONSOLE_GGMS) && aspect && overscan)
	  {
		  /* original Game Gear SMS Mode: 256x240 cropped to 240x218 then downscaled to 160x144 */
		  offset  = 5724; /* (22 * bitmap.granularity) + (10 * bitmap.pitch); */
		  vwidth  = 240;
		  vheight = 220;  /* value need to be divisible by 4 */
		  shift   = 11;	  /* bitmap.granularity * (bitmap.width - vwidth)/8 */
	  }
	  else
	  {
		  offset  = 0;
		  vwidth  = bitmap.viewport.w + 2*bitmap.viewport.x;
		  vheight = bitmap.viewport.h + 2*bitmap.viewport.y;
		  shift   = bitmap.granularity * (bitmap.width - vwidth) / 8;
	  }

      /* reinitialize texture */
	  GX_InvalidateTexAll ();
	  GX_InitTexObj (&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
  }

  /* fill texture data */
  long long int *dst = (long long int *)texturemem;
  long long int *src1 = (long long int *)(bitmap.data + offset);
  long long int *src2 = src1 + 71;
  long long int *src3 = src2 + 71;
  long long int *src4 = src3 + 71;
 
  
  GX_InvVtxCache ();
  GX_InvalidateTexAll ();
  
  /* update texture data */
  for (h = 0; h < vheight; h += 4)
  {
    for (w = 0; w < (vwidth >> 2); w++ )
	{
		*dst++ = *src1++;
		*dst++ = *src2++;
		*dst++ = *src3++;
		*dst++ = *src4++;
	}

    src1 = src4 + shift;
	src2 = src1 + 71;;
	src3 = src2 + 71;;
	src4 = src3 + 71;;
  }

  /* load texture into GX */
  DCFlushRange (texturemem, vwidth * vheight * 2);
  GX_LoadTexObj (&texobj, GX_TEXMAP0);
  
  /* render textured quad */
  draw_square ();
  GX_DrawDone ();

  /* switch external framebuffers then copy EFB to XFB */
  whichfb ^= 1;
  GX_CopyDisp (xfb[whichfb], GX_TRUE);
  GX_Flush ();

  /* set next XFB */
  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
}

/***************************************************************************
 *  Video initialization
 *  this function MUST be called at startup
 ***************************************************************************/
void scanpads (u32 count)
{
	PAD_ScanPads();
}

static void InitGCVideo ()
{
  extern GXRModeObj TVEurgb60Hz480IntDf;
    
  /*
   * Before doing anything else under libogc,
   * Call VIDEO_Init
   */
  VIDEO_Init ();

  /* Get the current video mode then :
      - set menu video mode (fullscreen, 480i or 576i)
      - set emulator rendering TV modes (PAL/MPAL/NTSC/EURGB60)
   */
  switch (VIDEO_GetCurrentTvMode())
  {
    case VI_PAL: /* only 480 lines */
      vmode = &TVPal574IntDfScale;
	  vmode->xfbHeight = 480;
	  vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - 480)/2;
	  vmode->viHeight = 480;
      TVNtsc_Rgb60_240p.viTVMode = VI_TVMODE_EURGB60_DS;
      TVNtsc_Rgb60_480i.viTVMode = VI_TVMODE_EURGB60_INT;
	  gc_pal = 1;
	  tv_mode = 1;
      break;

#ifdef FORCE_EURGB60
   default:
      vmode = &TVEurgb60Hz480IntDf;
      TVNtsc_Rgb60_240p.viTVMode = VI_TVMODE_EURGB60_DS;
      TVNtsc_Rgb60_480i.viTVMode = VI_TVMODE_EURGB60_INT;
	  gc_pal = 0;
	  tv_mode = 2;
      break;

#else
	case VI_MPAL:
      vmode = &TVMpal480IntDf;
      TVNtsc_Rgb60_240p.viTVMode = VI_TVMODE_MPAL_DS;
      TVNtsc_Rgb60_480i.viTVMode = VI_TVMODE_MPAL_INT;
	  gc_pal = 0;
	  tv_mode = 2;
      break;

	default:
      vmode = &TVNtsc480IntDf;
      TVNtsc_Rgb60_240p.viTVMode = VI_TVMODE_NTSC_DS;
      TVNtsc_Rgb60_480i.viTVMode = VI_TVMODE_NTSC_INT;
	  gc_pal = 0;
	  tv_mode = 0;
      break;
#endif
  }
  
  /* Set default video mode */
  VIDEO_Configure (vmode);

  /* Configure the framebuffers (double-buffering) */
  xfb[0] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TVPal_576i_Scale));
  xfb[1] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TVPal_576i_Scale));

  /* Define a console */
  console_init(xfb[0], 20, 64, 640, 574, 574 * 2);

  /* Clear framebuffers to black */
  VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);

  /* Set the framebuffer to be displayed at next VBlank */
  VIDEO_SetNextFramebuffer (xfb[0]);

  /* Register Video Retrace handlers */
  VIDEO_SetPreRetraceCallback(framestart);
  VIDEO_SetPostRetraceCallback(scanpads);

  /* Enable Video Interface */
  VIDEO_SetBlack (FALSE);
  
  /* Update video settings for next VBlank */
  VIDEO_Flush ();

  /* Wait for VBlank */
  VIDEO_WaitVSync();
  VIDEO_WaitVSync();

  /* Initialize everything else */
  PAD_Init ();
#ifndef HW_RVL
  DVD_Init ();
  dvd_drive_detect();
#endif
  SDCARD_Init ();
  unpackBackdrop ();
  init_font();
  StartGX ();
}

/***************************************************************************
 * Nintendo Gamecube Hardware Specific Functions
 *
 * A U D I O
 ***************************************************************************/
unsigned char soundbuffer[16][3840] ATTRIBUTE_ALIGN(32);
int mixbuffer = 0;
int playbuffer = 0;
int IsPlaying = 0;
int ConfigRequested = 0;

/*** AudioSwitchBuffers
     Genesis Plus only provides sound data on completion of each frame.
     To try to make the audio less choppy, this function is called from both the
     DMA completion and update_audio.
     Testing for data in the buffer ensures that there are no clashes.
 ***/
static void AudioSwitchBuffers()
{
	u32 dma_len = (sms.display) ? 3840 : 3200;

  if ( !ConfigRequested )
  {
    /* restart audio DMA with current soundbuffer */
    AUDIO_InitDMA((u32) soundbuffer[playbuffer], dma_len);
    DCFlushRange(soundbuffer[playbuffer], dma_len);
    AUDIO_StartDMA();
    
    /* increment soundbuffers index */
	playbuffer++;
	playbuffer &= 0xf;
    if (playbuffer == mixbuffer)
    {
      playbuffer--;
	  if ( playbuffer < 0 ) playbuffer = 15;
    }
	IsPlaying = 1;
  }
  else IsPlaying = 0;
}

/*** InitGCAudio
     Stock code to set the DSP at 48Khz
 ***/
static void InitGCAudio ()
{
	AUDIO_Init (NULL);
	AUDIO_SetDSPSampleRate (AI_SAMPLERATE_48KHZ);
	AUDIO_RegisterDMACallback (AudioSwitchBuffers);
	memset(soundbuffer, 0, 16 * 3840);
}


/***************************************************************************
 * Nintendo Gamecube Hardware Specific Functions
 *
 * I N P U T
 ***************************************************************************/
extern int joy_type;
uint8 analog_sensivity = 0;
int padcal = 30;

static void update_input (int i)
{
	signed char x, y;
	unsigned short p = PAD_ButtonsHeld (i);

	/* Check for menu combo */
	if (p & PAD_TRIGGER_Z)
	{
		ConfigRequested = 1;
		return;
	}

	/* Reset Inputs */
	input.pad[i] = 0;
	//input.paddle = 0x7F;
  
	/* Check for PAUSE/START */
	if (p & PAD_BUTTON_START) input.system |= (sms.console == CONSOLE_GG) ? INPUT_START : INPUT_PAUSE;

	/* Check for RESET combo */
	if ((p & PAD_TRIGGER_L) && (p & PAD_TRIGGER_R)) input.system |= INPUT_RESET;
  
	/* Check Left Analog Stick */
	x = PAD_StickX (i);
	y = PAD_StickY (i);

	if (sms.light_phaser)
	{
		/* light phaser */
		if (p & PAD_BUTTON_A) input.pad[i] |= (joy_type ? INPUT_BUTTON1 : 0);
		if (p & PAD_BUTTON_B) input.pad[i] |= (joy_type ? 0 : INPUT_BUTTON1);
		input.lightgun_x[i] += (x << 8) / (20 - analog_sensivity);
		input.lightgun_y[i] -= (y << 8) / (20 - analog_sensivity);
		if (input.lightgun_x[i] < 0) input.lightgun_x[i] = 0;
		if (input.lightgun_y[i] < 0) input.lightgun_y[i] = 0;
		if (input.lightgun_x[i] > 65535) input.lightgun_x[i] = 65535;
		if (input.lightgun_y[i] > 49151) input.lightgun_y[i] = 49151;
	}
	else if (sms.paddle)
	{
		/* paddle */
		if (joy_type) input.pad[i] |= ((p & PAD_BUTTON_A) ? INPUT_BUTTON1 : 0);
		else input.pad[i] |= ((p & PAD_BUTTON_B) ? INPUT_BUTTON1 : 0);
		input.paddle = x + 128;
	}
	else
	{
		/* classical gamepad */
		if ((p & PAD_BUTTON_UP)    || (y >  padcal)) input.pad[i] |= INPUT_UP;
		if ((p & PAD_BUTTON_DOWN)  || (y < -padcal)) input.pad[i] |= INPUT_DOWN;
		if ((p & PAD_BUTTON_LEFT)  || (x < -padcal)) input.pad[i] |= INPUT_LEFT;
		if ((p & PAD_BUTTON_RIGHT) || (x >  padcal)) input.pad[i] |= INPUT_RIGHT;
		if (p & PAD_BUTTON_A) input.pad[i] |= (joy_type ? INPUT_BUTTON1 : INPUT_BUTTON2);
		if (p & PAD_BUTTON_B) input.pad[i] |= (joy_type ? INPUT_BUTTON2 : INPUT_BUTTON1);
	}
}


/***************************************************************************
 * int SMS Plus VM
 *
 ***************************************************************************/
int smsromsize;
uint8 *smsrom;
static void init_machine ()
{
  /*** Allocate cart_rom here ***/
  smsrom = memalign(32, 0x100000);
  smsromsize = 0;

  /* Look for BIOS rom */
  bios.enabled = 0;
  bios.rom = memalign(32, 0x100000);
  sd_file *bios_file = SDCARD_OpenFile ("dev0:\\smsplus\\SMS_BIOS.sms", "rb");
  if (bios_file != NULL)
  {
	int offset, len, size;
	size = SDCARD_GetFileSize(bios_file);
	if (size < 0x4000) size = 0x4000;
	offset = 0;
	while ((len = SDCARD_ReadFile (bios_file, &bios.rom[offset], 2048)) > 0) offset += 2048;
	SDCARD_CloseFile (bios_file);
	bios.enabled = 2;
	bios.pages = size / 0x4000;
	sms.console = CONSOLE_SMS;
  }

  /* allocate global work bitmap */
  memset (&bitmap, 0, sizeof (bitmap));
  bitmap.width = 284;
  bitmap.height = 288;
  bitmap.depth = 16;
  bitmap.granularity = 2;
  bitmap.pitch = bitmap.width * bitmap.granularity;
  bitmap.viewport.w = 256;
  bitmap.viewport.h = 192;
  bitmap.viewport.x = 0;
  bitmap.viewport.y = 0;
  bitmap.data = malloc (bitmap.width * bitmap.height * bitmap.granularity);

}
  

/*****************************************************************************
 * set_option_defaults
 *****************************************************************************/
static void set_option_defaults ()
{
	option.sndrate = 48000;
	option.country = 0;
	option.console = 0;
	option.fm_enable = 1;
	option.fm_which = SND_EMU2413;
}

void system_manage_sram (uint8 * sram, int slot, int mode)
{
}

/***************************************************************************
 * M A I N
 *
 ***************************************************************************/
extern u32 diff_usec(long long start,long long end);
extern long long gettime();

extern void legal ();
extern void MainMenu ();

int main (int argc, char *argv[])
{
  u16 usBetweenFrames;
  long long now, prev;
  
  /* Initialize GC System */
  InitGCVideo ();
  InitGCAudio ();

  /* Initialize Virtual SMS */
  set_option_defaults ();
  init_machine ();

  /* Show Menu */
  legal ();
    
  /* wait for a rom to be loaded */
  MainMenu();
			
  /* Initialize Frame timings */
  frameticker = 0;
  prev = gettime();

  /* Emulation Loop */
  while (1)
  {
    /* Update Inputs */
    input.system = 0;
    update_input(0);
    update_input(1);

    /* Frame synchronization */
    if (gc_pal != sms.display)
    {
      /* use timers */
      usBetweenFrames = sms.display ? 20000 : 16667;
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
        update_video ();
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
        update_video ();
      }
      
      frameticker--;
    }

    /* Restart Audio Loop if needed */
    if (!IsPlaying) AudioSwitchBuffers();

    /* Check for Menu request */
    if (ConfigRequested)
    {
      /* stop AUDIO */
      AUDIO_StopDMA ();
      IsPlaying = 0;
      mixbuffer = 0;
      playbuffer = 0;
		
      /* go to menu */
      MainMenu ();
      ConfigRequested = 0;

      /* reset frame timings */
      frameticker = 0;
      prev = gettime();
    }
  }
  return 0;
}
