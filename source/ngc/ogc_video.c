/****************************************************************************
 *  ogc_video.c
 *
 *  SMS Plus GX video support
 *
 *  code by Softdev (2006), Eke-Eke (2007,2008)
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
 ***************************************************************************/


#include "shared.h"
#include "font.h"
#include "sms_ntsc.h"

/*** NTSC Filters ***/
sms_ntsc_setup_t sms_setup;
sms_ntsc_t sms_ntsc;

/*** PAL 50hz flag ***/
BOOL gc_pal = 0;

/*** VI ***/
unsigned int *xfb[2]; /*** Double buffered            ***/
int whichfb = 0;      /*** External framebuffer index ***/
GXRModeObj *vmode;    /*** Menu video mode            ***/
u8 *texturemem;       /*** Texture Data               ***/

/*** GX ***/
#define TEX_WIDTH         720
#define TEX_HEIGHT        288
#define TEX_SIZE          (TEX_WIDTH * TEX_HEIGHT * 2)
#define TEX_PITCH         180
#define DEFAULT_FIFO_SIZE 256 * 1024
#define HASPECT           320
#define VASPECT           240

static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static GXTexObj texobj;
static Mtx view;
static int vwidth, vheight, offset, shift;

/*** custom Video modes (used to emulate original console video modes) ***/
/* 288 lines progressive (PAL 50Hz) */
GXRModeObj TV50hz_288p = 
{
  VI_TVMODE_PAL_DS,       // viDisplayMode
  640,             // fbWidth
  286,             // efbHeight
  286,             // xfbHeight
  (VI_MAX_WIDTH_PAL - 720)/2,         // viXOrigin
  (VI_MAX_HEIGHT_PAL/2 - 572/2)/2,        // viYOrigin
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
GXRModeObj TV50hz_576i = 
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
GXRModeObj TV60hz_240p = 
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
GXRModeObj TV60hz_480i = 
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
  &TV60hz_240p, &TV60hz_480i, /* 60hz modes */
  &TV50hz_288p, &TV50hz_576i  /* 50Hz modes */
};

typedef struct tagcamera
{
  guVector pos;
  guVector up;
  guVector view;
} camera;

/*** Square Matrix
     This structure controls the size of the image on the screen.
   Think of the output as a -80 x 80 by -60 x 60 graph.
***/
static s16 square[] ATTRIBUTE_ALIGN (32) =
{
  /*
   * X,   Y,  Z
   * Values set are for roughly 4:3 aspect
   */
  -HASPECT,  VASPECT, 0,  // 0
   HASPECT,  VASPECT, 0,  // 1
   HASPECT, -VASPECT, 0,  // 2
  -HASPECT, -VASPECT, 0,  // 3
};

static camera cam = {
  {0.0F, 0.0F, -100.0F},
  {0.0F, -1.0F, 0.0F},
  {0.0F, 0.0F, 0.0F}
};

/* rendering initialization */
/* should be called each time you change quad aspect ratio */
static void draw_init(void)
{
  /* Clear all Vertex params */
  GX_ClearVtxDesc ();

  /* Set Position Params (quad aspect ratio) */
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

  /* Set Modelview */
  memset (&view, 0, sizeof (Mtx));
  guLookAt(view, &cam.pos, &cam.up, &cam.view);
  GX_LoadPosMtxImm (view, GX_PNMTX0);
}

/* vertex rendering */
static inline void draw_vert(u8 pos, f32 s, f32 t)
{
  GX_Position1x8 (pos);
  GX_TexCoord2f32 (s, t);
}

/* textured quad rendering */
static inline void draw_square (void)
{
  GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
  draw_vert (3, 0.0, 0.0);
  draw_vert (2, 1.0, 0.0);
  draw_vert (1, 1.0, 1.0);
  draw_vert (0, 0.0, 1.0);
  GX_End ();
}

/* retrace handler */
static void framestart(u32 retraceCnt)
{
  /* simply increment the tick counter */
  frameticker++;

  /* display OSD messages */
  if (osd.frames)
  {
    osd.frames--;
    write_font(300, option.render ? 400:200, osd.msg);
  }

}

/* Initialize GX */
static void gxStart(void)
{
  Mtx p;
  GXColor gxbackground = { 0, 0, 0, 0xff };

  /*** Clear out FIFO area ***/
  memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

  /*** GX default ***/
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

  /*** Reset XFB ***/
  GX_CopyDisp (xfb[whichfb ^ 1], GX_TRUE);

  /*** Initialize texture data ***/
  texturemem = memalign(32, TEX_WIDTH * TEX_HEIGHT * 2);
  memset (texturemem, 0, TEX_WIDTH * TEX_HEIGHT * 2);

  /*** Initialize renderer */
  draw_init();
}

/* set GX scaler */
static void gxScale(void)
{
  int xscale, yscale, xshift, yshift;

  /* Aspect Ratio (depending on current configuration) */
  if (option.aspect)
  {
    /* original aspect */
    if (option.overscan)
    {
      if (sms.console == CONSOLE_GGMS)
      {
        /* SMS display scaled into GG screen */
        xscale = 182;
        yscale = 72;
        if (gc_pal && !option.render) yscale = yscale * 288 / 243;
        xshift = 0;
        yshift = 0;
      }
      else
      {
        xscale = 320;
        yscale = sms.display ? ((gc_pal && !option.render) ? 144 : 121) : ((gc_pal && !option.render) ? 143 : 120);
        xshift = 0;
        yshift = sms.display ? (gc_pal ? 1 : 0) : 2;
      }
    }
    else
    {
      /* borders are emulated */
      if ((sms.console == CONSOLE_GGMS) || ((sms.console == CONSOLE_GG) && !option.extra_gg))
      {
        /* SMS display scaled into GG screen */
        xscale = 182;
        yscale = 72;
        if (gc_pal && !option.render) yscale = yscale * 288 / 243;
        xshift = 0;
        yshift = 0;
      }
      else
      {
        xscale = 290;
        yscale = bitmap.viewport.h / 2;
        if (sms.display && (!gc_pal || option.render)) yscale = yscale * 243 / 288;
        else if (!sms.display && gc_pal && !option.render) yscale = yscale * 288 / 243;
        xshift = 0;
        yshift = sms.display ? (gc_pal ? 1 : 0) : 2;
      }
    }
  }
  else
  {
    /* fit screen */
    xscale = (option.overscan) ? 320 : 290;
    yscale = (gc_pal && !option.render) ? 134 : 112;
    xshift = 0;
    yshift = gc_pal ? 1 : 2;
  }

  /* user configuration */
  if (!option.aspect)
  {
    xscale += option.xscale;
    yscale += option.yscale;
  }

  xshift += option.xshift;
  yshift += option.yshift;

  /* double resolution */
  if (option.render)
  {
    yscale *= 2;
    yshift *= 2;
  }

  /* update GX scaler (Vertex Position Matrix) */
  square[6] = square[3]  =  xscale + xshift;
  square[0] = square[9]  = -xscale + xshift;
  square[4] = square[1]  =  yscale + yshift;
  square[7] = square[10] = -yscale + yshift;

  DCFlushRange (square, 32);
  GX_InvVtxCache ();
}

/* Reinitialize Video */
void ogc_video__reset()
{
  Mtx p;
  GXRModeObj *rmode;

  /* 50Hz/60Hz mode */
  if ((option.tv_mode == 1) || ((option.tv_mode == 2) && sms.display)) gc_pal = 1;
  else gc_pal = 0;

  /* reset scaler */
  gxScale();

  /* reinitialize current TV mode */
  if (option.render == 2)
  {
    /* 480p */
    tvmodes[1]->viTVMode = VI_TVMODE_NTSC_PROG;
    tvmodes[1]->xfbMode = VI_XFBMODE_SF;
  }
  else
  {
    /* 480i */
    tvmodes[1]->viTVMode = tvmodes[0]->viTVMode & ~3;
    tvmodes[1]->xfbMode = VI_XFBMODE_DF;
  }

  /* Set current TV mode */  
  rmode = option.render ? tvmodes[gc_pal*2 + 1] : tvmodes[gc_pal*2];

    /* Configure VI */
  VIDEO_Configure (rmode);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  VIDEO_WaitVSync();

  /* reset frame counter */
  frameticker = 0;

  /* Configure GX */
  GX_SetViewport (0.0F, 0.0F, rmode->fbWidth, rmode->efbHeight, 0.0F, 1.0F);
  GX_SetScissor (0, 0, rmode->fbWidth, rmode->efbHeight);
  f32 yScale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
  u16 xfbHeight = GX_SetDispCopyYScale (yScale);
  GX_SetDispCopySrc (0, 0, rmode->fbWidth, rmode->efbHeight);
  GX_SetDispCopyDst (rmode->fbWidth, xfbHeight);
  GX_SetCopyFilter (rmode->aa, rmode->sample_pattern, option.render ? GX_TRUE : GX_FALSE, rmode->vfilter);
  GX_SetFieldMode (rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  GX_SetPixelFmt (rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  guOrtho(p, rmode->efbHeight/2, -(rmode->efbHeight/2), -(rmode->fbWidth/2), rmode->fbWidth/2, 100, 1000);
  GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);

  /* Software NTSC filter */
  if (option.ntsc == 1)
  {
    sms_setup = sms_ntsc_composite;
    sms_ntsc_init( &sms_ntsc, &sms_setup );
  }
  else if (option.ntsc == 2)
  {
    sms_setup = sms_ntsc_svideo;
    sms_ntsc_init( &sms_ntsc, &sms_setup );
  }
  else if (option.ntsc == 3)
  {
    sms_setup = sms_ntsc_rgb;
    sms_ntsc_init( &sms_ntsc, &sms_setup );
  }
}

/* GX render update */
void ogc_video__update()
{
  int h, w;

  if (bitmap.viewport.changed)
  {
    bitmap.viewport.changed = 0;
    
    /* update texture size */
    if ((sms.console == CONSOLE_GG) && !option.overscan && !option.extra_gg)
    {
      /* Game Gear display is 160 x 144 pixels */
      offset  = 48 * bitmap.granularity;
      vwidth  = 160;
      vheight = 144;
    }
    else if ((sms.console == CONSOLE_GGMS) && option.aspect && option.overscan)
    {
      /* original Game Gear SMS Mode: 256x240 cropped to 240x218 then downscaled to 160x144 */
      offset  = (22 * bitmap.granularity) + (10 * bitmap.pitch);
      vwidth  = 240;
      vheight = 220;  /* value need to be divisible by 4 */
    }
    else
    {
      offset  = 0;
      vwidth  = bitmap.viewport.w + 2*bitmap.viewport.x;
      vheight = bitmap.viewport.h + 2*bitmap.viewport.y;
    }

    /* ntsc filter */
    if (option.ntsc) vwidth = SMS_NTSC_OUT_WIDTH(vwidth);

    /* texture size must be multiple of 4 texels */
    vwidth = (vwidth / 4) * 4;
    vheight = (vheight / 4) * 4;

    /* final offset */
    shift   = (bitmap.width - vwidth) / 4;

    /* reset scaler (height has changed) */
    gxScale();
   
    /* reinitialize texture */
    GX_InitTexObj (&texobj, texturemem, vwidth, vheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);

    /* enable/disable bilinear filtering */
    if (!option.bilinear)
    {
      GX_InitTexObjLOD(&texobj,GX_NEAR,GX_NEAR_MIP_NEAR,2.5,9.0,0.0,GX_FALSE,GX_FALSE,GX_ANISO_1);
    }

    /* load texture */
    GX_LoadTexObj (&texobj, GX_TEXMAP0);
  }

  /* fill texture data */
  long long int *dst = (long long int *)texturemem;
  long long int *src1 = (long long int *)(bitmap.data + offset);
  long long int *src2 = src1 + TEX_PITCH;
  long long int *src3 = src2 + TEX_PITCH;
  long long int *src4 = src3 + TEX_PITCH;

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
    src2 = src1 + TEX_PITCH;
    src3 = src2 + TEX_PITCH;
    src4 = src3 + TEX_PITCH;
  }

  /* update texture cache */
  DCFlushRange (texturemem, TEX_SIZE);
  GX_InvalidateTexAll ();

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

/* Initialize VIDEO subsystem */
void ogc_video__init(void)
{
  /*
   * Before doing anything else under libogc,
   * Call VIDEO_Init
   */
  VIDEO_Init ();


  /* Get the current video mode then :
      - set menu video mode (480p, 480i or 576i)
      - set emulator rendering TV modes (PAL/MPAL/NTSC/EURGB60)
   */
  vmode = VIDEO_GetPreferredMode(NULL);

  /* Adjust display settings */
  switch (vmode->viTVMode >> 2)
  {
    case VI_PAL:  /* 576 lines (PAL 50Hz) */

      TV60hz_240p.viTVMode = VI_TVMODE_EURGB60_DS;
      TV60hz_480i.viTVMode = VI_TVMODE_EURGB60_INT;
      option.tv_mode = 1;

      /* display should be centered vertically (borders) */
      vmode = &TVPal574IntDfScale;
      vmode->xfbHeight = 480;
      vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - 480)/2;
      vmode->viHeight = 480;
      break;

    case VI_NTSC: /* 480 lines (NTSC 60hz) */
      TV60hz_240p.viTVMode = VI_TVMODE_NTSC_DS;
      TV60hz_480i.viTVMode = VI_TVMODE_NTSC_INT;
      option.tv_mode = 0;

#ifndef HW_RVL
      /* force 480p on NTSC GameCube if the Component Cable is present */
      if (VIDEO_HaveComponentCable()) vmode = &TVNtsc480Prog;
#endif
      break;

    default:  /* 480 lines (PAL 60Hz) */
      TV60hz_240p.viTVMode = VI_TVMODE(vmode->viTVMode >> 2, VI_NON_INTERLACE);
      TV60hz_480i.viTVMode = VI_TVMODE(vmode->viTVMode >> 2, VI_INTERLACE);
      option.tv_mode = 2;
      break;
  }
   
#ifdef HW_RVL
  /* Widescreen fix */
  if( CONF_GetAspectRatio() )
  {
    vmode->viWidth    = 678;
    vmode->viXOrigin  = (VI_MAX_WIDTH_NTSC - 678)/2;
  }
#endif

  /* Configure video mode */
  VIDEO_Configure (vmode);

  /* Configure the framebuffers (double-buffering) */
  xfb[0] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TV50hz_576i));
  xfb[1] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TV50hz_576i));

  /* Define a console */
  console_init(xfb[0], 20, 64, 640, 574, 574 * 2);

  /* Clear framebuffers to black */
  VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);

  /* Set the framebuffer to be displayed at next VBlank */
  VIDEO_SetNextFramebuffer (xfb[0]);

  /* Register Video Retrace handlers */
  VIDEO_SetPreRetraceCallback(framestart);

  /* Enable Video Interface */
  VIDEO_SetBlack (FALSE);

  /* Update video settings for next VBlank */
  VIDEO_Flush ();

  /* Wait for VBlank */
  VIDEO_WaitVSync();
  VIDEO_WaitVSync();

  /* Initialize GUI */
  unpackBackdrop ();
  init_font();

  /* Initialize GX */
  gxStart();
}
