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
#include "config.h"
#include "dvd.h"
#include "font.h"

t_option sms_option;				/*** Option structure ***/
int frameticker = 0;
int ConfigRequested = 0;
int padcal = 70;
int RenderedFrameCount = 0;
int FrameCount = 0;
int FramesPerSecond = 0;
int smsromsize;
uint8 *smsrom;
u8 isWII = 0;

/**
 * Timer functions
 */
#define TB_CLOCK  40500000
#define mftb(rval) ({unsigned long u; do { \
         asm volatile ("mftbu %0" : "=r" (u)); \
         asm volatile ("mftb %0" : "=r" ((rval)->l)); \
         asm volatile ("mftbu %0" : "=r" ((rval)->u)); \
         } while(u != ((rval)->u)); })

typedef struct
{
	unsigned long l, u;
} tb_t;

unsigned long tb_diff_msec(tb_t *end, tb_t *start)
{
	unsigned long upper, lower;
	upper = end->u - start->u;
	if (start->l > end->l) upper--;
	lower = end->l - start->l;
	return ((upper*((unsigned long)0x80000000/(TB_CLOCK/2000))) + (lower/(TB_CLOCK/1000)));
}

int msBetweenFrames = 20;
tb_t now, prev;


/***************************************************************************
 * Nintendo Gamecube Hardware Specific Functions
 *
 * V I D E O
 ***************************************************************************/
/*** 2D Video ***/
unsigned int *xfb[2];	/*** Double buffered ***/
int whichfb = 0;		/*** Switch ***/
GXRModeObj *vmode;		/*** General video mode ***/

/*** GX ***/
#define TEX_WIDTH 512
#define TEX_HEIGHT 256
#define DEFAULT_FIFO_SIZE 256 * 1024

static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static u8 texturemem[TEX_WIDTH * (TEX_HEIGHT + 8) * 2] ATTRIBUTE_ALIGN (32);
GXTexObj texobj;
static Mtx view;
int vwidth, vheight, oldvwidth, oldvheight;

#define HASPECT 76
#define VASPECT 54

/* New texture based scaler */
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

static camera cam = { {0.0F, 0.0F, 0.0F},
{0.0F, 0.5F, 0.0F},
{0.0F, 0.0F, -0.5F}
};

/*** Framestart function
	 Simply increment the tick counter
 ***/
static void framestart()
{
  frameticker++;
}

/*** WIP3 - Scaler Support Functions
 ***/
static void draw_init (void)
{
  GX_ClearVtxDesc ();
  GX_SetVtxDesc (GX_VA_POS, GX_INDEX8);
  GX_SetVtxDesc (GX_VA_CLR0, GX_INDEX8);
  GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GX_SetArray (GX_VA_POS, square, 3 * sizeof (s16));
  GX_SetNumTexGens (1);
  GX_SetTexCoordGen (GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
  GX_InvalidateTexAll ();
  GX_InitTexObj (&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
}

static void draw_vert (u8 pos, u8 c, f32 s, f32 t)
{
  GX_Position1x8 (pos);
  GX_Color1x8 (c);
  GX_TexCoord2f32 (s, t);
}

static void draw_square (Mtx v)
{
  Mtx m;			// model matrix.
  Mtx mv;			// modelview matrix.

  guMtxIdentity (m);
  guMtxTransApply (m, m, 0, 0, -100);
  guMtxConcat (v, m, mv);
  GX_LoadPosMtxImm (mv, GX_PNMTX0);
  GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
  draw_vert (0, 0, 0.0, 0.0);
  draw_vert (1, 0, 1.0, 0.0);
  draw_vert (2, 0, 1.0, 1.0);
  draw_vert (3, 0, 0.0, 1.0);
  GX_End ();
}

/*** StartGX
	 This function initialises the GX.
     WIP3 - Based on texturetest from libOGC examples.
 ***/
static void StartGX (void)
{
  Mtx p;
  GXColor gxbackground = { 0, 0, 0, 0xff };

  /*** Clear out FIFO area ***/
  memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

  /*** Initialise GX ***/
  GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
  GX_SetCopyClear (gxbackground, 0x00ffffff);
  GX_SetViewport (0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
  GX_SetDispCopyYScale ((f32) vmode->xfbHeight / (f32) vmode->efbHeight);
  GX_SetScissor (0, 0, vmode->fbWidth, vmode->efbHeight);
  GX_SetDispCopySrc (0, 0, vmode->fbWidth, vmode->efbHeight);
  GX_SetDispCopyDst (vmode->fbWidth, vmode->xfbHeight);
  GX_SetCopyFilter (vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
  GX_SetFieldMode (vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  GX_SetPixelFmt (GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GX_SetCullMode (GX_CULL_NONE);
  GX_CopyDisp (xfb[whichfb ^ 1], GX_TRUE);
  GX_SetDispCopyGamma (GX_GM_1_0);
  guPerspective (p, 60, 1.33F, 10.0F, 1000.0F);
  GX_LoadProjectionMtx (p, GX_PERSPECTIVE);
  memset (texturemem, 0, TEX_WIDTH * TEX_HEIGHT * 2);
  vwidth = 100;
  vheight = 100;
}

/*** InitGCVideo
	 This function MUST be called at startup.
 ***/
static void InitGCVideo ()
{
  /*
   * Before doing anything else under libogc,
   * Call VIDEO_Init
   */
  VIDEO_Init ();
  PAD_Init ();

  /*
   * Reset the video mode
   * This is always set to 60hz
   * Whether your running PAL or NTSC
   */
  vmode = &TVNtsc480IntDf;
  VIDEO_Configure (vmode);

  /*** Now configure the framebuffer. 
	     Really a framebuffer is just a chunk of memory
	     to hold the display line by line.
   **/
  xfb[0] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(vmode));

  /*** I prefer also to have a second buffer for double-buffering.
	     This is not needed for the console demo.
   ***/
  xfb[1] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(vmode));

  /*** Define a console ***/
  console_init(xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);

  /*** Clear framebuffer to black ***/
  VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);

  /*** Set the framebuffer to be displayed at next VBlank ***/
  VIDEO_SetNextFramebuffer (xfb[0]);

  /*** Increment frameticker and timer ***/
  VIDEO_SetPreRetraceCallback(framestart);

  /*** Get the PAD status updated by libogc ***/
  VIDEO_SetPostRetraceCallback (PAD_ScanPads);
  VIDEO_SetBlack (FALSE);
  
  /*** Update the video for next vblank ***/
  VIDEO_Flush ();

  /*** Wait for VBL ***/
  VIDEO_WaitVSync();
  if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

  DVD_Init ();
  SDCARD_Init ();
  unpackBackdrop ();
  init_font();
  StartGX ();

  /* Wii drive detection for 4.7Gb support */
  int driveid = dvd_inquiry();
  if ((driveid == 4) || (driveid == 6) || (driveid == 8)) isWII = 0;
  else isWII = 1;
}

/*** Video Update
     called after each emulation frame
 ***/
uint8 lightgun_2p = 0;

static void update_video ()
{
  int h, w, offset;

  whichfb ^= 1;
  vwidth  = bitmap.viewport.w;
  vheight = bitmap.viewport.h;
  
  /* display offset: 0 (default) or 24 lines top & 48 pixels left (GG)
   * we use a precaluculate value for some speedup:
   *
   *	offset = (bitmap.viewport.y * bitmap.pitch) + (bitmap.viewport.x * bitmap.granularity);
   *	bitmap.viewport.y = 0 (default) or 24 lines (GG)
   *	bitmap.viewport.x = 0 (default) or 48 pixels (GG)
  */
  offset = (IS_GG) ? 12384 : 0;

  if ((oldvheight != vheight) || (oldvwidth != vwidth))
  {
	  /** Update scaling **/
      oldvwidth = vwidth;
      oldvheight = vheight;
      draw_init ();
      memset (&view, 0, sizeof (Mtx));
	  guLookAt(view, &cam.pos, &cam.up, &cam.view);
      GX_SetViewport (0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
  }

  GX_InvVtxCache ();
  GX_InvalidateTexAll ();
  GX_SetTevOp (GX_TEVSTAGE0, GX_DECAL);
  GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

  int *dst = (int *) &texturemem[0];
  int *src = (int *) &bitmap.data[offset];

  int hpos;
  int srcpitch = bitmap.width << 1;
  int lrow2 = bitmap.width >> 1;
  int lrow4 = ((bitmap.width << 2) - bitmap.width) >> 1;

  for (h = 0; h < vheight; h += 4)
  {
	  for (w = 0; w < (vwidth >> 1); w += 2)
	  {
		  hpos = w;
		  *dst++ = src[hpos++];
		  *dst++ = src[hpos];
		
		  hpos = w + lrow2;
		  *dst++ = src[hpos++];
		  *dst++ = src[hpos];
		  
		  hpos = w + bitmap.width;
		  *dst++ = src[hpos++];
		  *dst++ = src[hpos];
		  
		  hpos = w + lrow4;
		  *dst++ = src[hpos++];
		  *dst++ = src[hpos];
	  }

      src += srcpitch;
  }

  DCFlushRange (texturemem, TEX_WIDTH * TEX_HEIGHT * 2);
  GX_SetNumChans (1);
  GX_LoadTexObj (&texobj, GX_TEXMAP0);
  draw_square (view);
  GX_DrawDone ();
  GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
  GX_SetColorUpdate (GX_TRUE);
  GX_CopyDisp (xfb[whichfb], GX_TRUE);
  GX_Flush ();

  /* draw the crosshair */
  if (sms.light_phaser)
  {
	  fntDrawBoxFilled((((input.lightgun_x[0] >> 8) * 640) / 256) - 16,
					   (((input.lightgun_y[0] >> 8) * 480) / 256) - 16,
					   (((input.lightgun_x[0] >> 8) * 640) / 256) + 16,
					   (((input.lightgun_y[0] >> 8) * 480) / 256) + 16,
					   COLOR_BLACK);
	  if (lightgun_2p)
	  fntDrawBoxFilled((((input.lightgun_x[1] >> 8) * 640) / 256) - 16,
					   (((input.lightgun_y[1] >> 8) * 480) / 256) - 16,
					   (((input.lightgun_x[1] >> 8) * 640) / 256) + 16,
					   (((input.lightgun_y[1] >> 8) * 480) / 256) + 16,
					   COLOR_RED);
  }

  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
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
	 AUDIO_InitDMA((u32) soundbuffer[playbuffer], dma_len);
	 DCFlushRange(soundbuffer[playbuffer], dma_len);
	 AUDIO_StartDMA();
	 playbuffer++;
	 playbuffer &= 0xf;
	 if ( playbuffer == mixbuffer ) playbuffer--;
	 if ( playbuffer < 0 ) playbuffer = 15;
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

/*** Audio Update
     called after each emulation frame
 ***/
static void update_audio ()
{
  if (IsPlaying == 0) AudioSwitchBuffers ();
}

/***************************************************************************
 * Nintendo Gamecube Hardware Specific Functions
 *
 * I N P U T
 ***************************************************************************/
extern int joy_type;
uint8 analog_sensivity = 0;

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
  //input.analog[i] = 0x7F;

  /* Check for PAUSE/START */
  if (p & PAD_BUTTON_START) input.system |= IS_GG ? INPUT_START : INPUT_PAUSE;

  /* Check for RESET combo */
  if ((p & PAD_TRIGGER_L) && (p & PAD_TRIGGER_R)) input.system |= INPUT_RESET;
  
  /* Check Left Analog Stick */
  x = PAD_StickX (i);
  y = PAD_StickY (i);

  if (sms.light_phaser)
  {
      /* light phaser */
	  if ((p!=0)&&(i==1)) lightgun_2p = 1;
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
	  if (p & PAD_BUTTON_A) input.pad[i] |= (joy_type ? INPUT_BUTTON1 : 0);
	  if (p & PAD_BUTTON_B) input.pad[i] |= (joy_type ? 0 : INPUT_BUTTON1);
	  input.paddle = ((x+128) << 8) / (20 - analog_sensivity);
      if (input.paddle < 0) input.paddle = 0;
	  if (input.paddle > 65535) input.paddle = 65535;
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

/*
 * init_machine
 *
 * Initialise the VM
 */
static void init_machine ()
{
  InitGCVideo ();
  InitGCAudio ();

  /*** Clear global bitmap ***/
  memset (&bitmap, 0, sizeof (bitmap));

  /*** Allocate global work bitmap ***/
  bitmap.data = malloc (1024 * 512 * 2);
  bitmap.width = 256;
  bitmap.height = 256;
  bitmap.depth = sms_option.video_depth;
  bitmap.granularity = (bitmap.depth >> 3);
  bitmap.pitch = bitmap.width * bitmap.granularity;
  bitmap.viewport.w = 256;
  bitmap.viewport.h = 192;
  bitmap.viewport.x = 0;
  bitmap.viewport.y = 0;
 

  /*** Allocate cart_rom here ***/
  smsrom = malloc(1048576 + 32);
  if ((u32)smsrom & 0x1f) smsrom += 32 - ((u32)smsrom & 0x1f);
  memset(smsrom, 0, 1048576);
  smsromsize = 0;
}

/*****************************************************************************
 * set_option_defaults
 *****************************************************************************/
static void set_option_defaults ()
{
  sms_option.joy_driver = 0;
  sms_option.video_driver = 0;
  sms_option.video_depth = 16;
  sms_option.video_width = 320;
  sms_option.video_height = 200;
  sms_option.no_vga = 0;
  sms_option.no_mmx = 0;
  sms_option.expand = 0;
  sms_option.blur = 0;
  sms_option.scale = 0;
  sms_option.scanlines = 0;
  sms_option.tweak = 0;
  sms_option.vsync = 0;
  sms_option.throttle = 0;
  sms_option.display = -1;
  sms_option.sound = 0;
  sms_option.sndcard = -1;
  sms_option.sndrate = 48000;
  sms_option.country = -1;
  sms_option.console = -1;
  sms_option.fm_enable = 0;
  sms_option.fm_which = SND_YM2413;
  sms_option.codies = -1;
}

void system_manage_sram (uint8 * sram, int slot, int mode)
{
}

/***************************************************************************
 * M A I N
 *
 ***************************************************************************/
extern void legal ();
extern void MainMenu ();
extern int loadsmsrom ();

int main (int argc, char *argv[])
{
  set_option_defaults ();
  init_machine ();
  legal ();
    
  /* wait for a rom to be loaded */
  while (smsromsize == 0) MainMenu();
 
  /* Main emulation loop */
  frameticker = 0;
  mftb(&prev);

  while (1)
  {
	/** Update inputs **/
	input.system = 0;
	update_input(0);
	update_input(1);

	if (sms.display) /* PAL 50Hz (use timer) */
	{
		mftb(&now);
		if ( tb_diff_msec( &now, &prev ) > msBetweenFrames )
		{
			memcpy(&prev, &now, sizeof(tb_t) );
			system_frame(1);
		}
		else
		{
			while (tb_diff_msec( &now, &prev ) < msBetweenFrames) mftb(&now);
			memcpy(&prev, &now, sizeof(tb_t) );
			system_frame(0);
			RenderedFrameCount++;
		}
    }
    else /* NTSC 60Hz (use vsync) */
	{
		while (frameticker < 1)  usleep(10);	
        
		/** Simulate a frame **/
        if (frameticker > 1)
	    {
	      frameticker--;
	      if (frameticker > 5)
	      {
	        system_frame (0);
	        frameticker = 1;
	      }
	      else system_frame (1);
	    }
        else system_frame (0);

		frameticker--;
	}

	/** Draw the frame **/
	update_video ();

	/** add the audio **/
	update_audio ();

    if (ConfigRequested)
	{
		AUDIO_StopDMA ();
		IsPlaying = mixbuffer = playbuffer = 0;
		MainMenu ();
		ConfigRequested = 0;
	}
  }
  return 0;
}
