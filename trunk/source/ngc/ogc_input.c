/******************************************************************************
 *
 *  SMS Plus GX - Sega Master System / GameGear Emulator
 *
 *  SMS Plus - Sega Master System / GameGear Emulator
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
#include "font.h"

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif

/* configurable keys */
#define KEY_BUTTON1 0   
#define KEY_BUTTON2 1
#define KEY_PAUSE   2
#define KEY_RESET   3
#define KEY_MENU    4

int ConfigRequested = 1;

/* gamepad default map (this can be reconfigured) */
u16 pad_keymap[MAX_INPUTS][MAX_KEYS] =
{
  {PAD_BUTTON_B, PAD_BUTTON_A, PAD_BUTTON_START, PAD_TRIGGER_L, PAD_TRIGGER_Z},
  {PAD_BUTTON_B, PAD_BUTTON_A, PAD_BUTTON_START, PAD_TRIGGER_L, PAD_TRIGGER_Z}
};

/* gamepad available buttons */
static u16 pad_keys[9] =
{
  PAD_TRIGGER_Z,
  PAD_TRIGGER_R,
  PAD_TRIGGER_L,
  PAD_BUTTON_A,
  PAD_BUTTON_B,
  PAD_BUTTON_X,
  PAD_BUTTON_Y,
  PAD_BUTTON_START,
};

#ifdef HW_RVL

/* wiimote default map (this can be reconfigured) */
u32 wpad_keymap[MAX_INPUTS*3][MAX_KEYS] =
{
  /* Wiimote #1 */
  {WPAD_BUTTON_1, WPAD_BUTTON_2, WPAD_BUTTON_PLUS, WPAD_BUTTON_MINUS, WPAD_BUTTON_HOME},
  {WPAD_BUTTON_A, WPAD_BUTTON_B, WPAD_BUTTON_PLUS, WPAD_BUTTON_MINUS, WPAD_BUTTON_HOME},
  {WPAD_CLASSIC_BUTTON_B, WPAD_CLASSIC_BUTTON_A, WPAD_CLASSIC_BUTTON_PLUS, WPAD_CLASSIC_BUTTON_MINUS, WPAD_CLASSIC_BUTTON_HOME},
  
  /* Wiimote #2*/
  {WPAD_BUTTON_1, WPAD_BUTTON_2, WPAD_BUTTON_PLUS, WPAD_BUTTON_MINUS, WPAD_BUTTON_HOME},
  {WPAD_BUTTON_A, WPAD_BUTTON_B, WPAD_BUTTON_PLUS, WPAD_BUTTON_MINUS, WPAD_BUTTON_HOME},
  {WPAD_CLASSIC_BUTTON_B, WPAD_CLASSIC_BUTTON_A, WPAD_CLASSIC_BUTTON_PLUS, WPAD_CLASSIC_BUTTON_MINUS, WPAD_CLASSIC_BUTTON_HOME}
};

/* directional buttons default mapping (FIXED) */
#define PAD_UP    0   
#define PAD_DOWN  1
#define PAD_LEFT  2
#define PAD_RIGHT 3

#define MAX_HELD_CNT 15
static int held_cnt = 0;

static u32 wpad_dirmap[3][4] =
{
  {WPAD_BUTTON_RIGHT, WPAD_BUTTON_LEFT, WPAD_BUTTON_UP, WPAD_BUTTON_DOWN},                                // WIIMOTE only
  {WPAD_BUTTON_UP, WPAD_BUTTON_DOWN, WPAD_BUTTON_LEFT, WPAD_BUTTON_RIGHT},                                // WIIMOTE + NUNCHUK
  {WPAD_CLASSIC_BUTTON_UP, WPAD_CLASSIC_BUTTON_DOWN, WPAD_CLASSIC_BUTTON_LEFT, WPAD_CLASSIC_BUTTON_RIGHT} // CLASSIC
};

/* wiimote/expansion available buttons */
static u32 wpad_keys[20] =
{
  WPAD_BUTTON_2,
  WPAD_BUTTON_1,
  WPAD_BUTTON_B,
  WPAD_BUTTON_A,
  WPAD_BUTTON_MINUS,
  WPAD_BUTTON_HOME,
  WPAD_BUTTON_PLUS,
  WPAD_NUNCHUK_BUTTON_Z,
  WPAD_NUNCHUK_BUTTON_C,
  WPAD_CLASSIC_BUTTON_ZR,
  WPAD_CLASSIC_BUTTON_X,
  WPAD_CLASSIC_BUTTON_A,
  WPAD_CLASSIC_BUTTON_Y,
  WPAD_CLASSIC_BUTTON_B,
  WPAD_CLASSIC_BUTTON_ZL,
  WPAD_CLASSIC_BUTTON_FULL_R,
  WPAD_CLASSIC_BUTTON_PLUS,
  WPAD_CLASSIC_BUTTON_HOME,
  WPAD_CLASSIC_BUTTON_MINUS,
  WPAD_CLASSIC_BUTTON_FULL_L,
};
#endif

static const char *keys_name[MAX_KEYS] =
{
  "Button 1",
  "Button 2",
  "Pause",
  "SoftReset",
  "Menu"
};

u8 softreset = 0;
static void set_softreset(void)
{
  softreset = 1;
}

/*******************************
  gamepad support
*******************************/
static void pad_config(int num)
{
  int i,j;
  int max = MAX_KEYS;
  u16 p;
  u8 quit;
  char msg[30];

  u32 pad = PAD_ScanPads() & (1<<num);
  if (!pad)
  {
    sprintf(msg, "PAD #%d is not connected !", num+1);
    WaitPrompt(msg);
    return;
  }

  /* configure keys */
  for (i=0; i<max; i++)
  {
    /* remove any pending keys */
    while (PAD_ButtonsHeld(num))
    {
      VIDEO_WaitVSync();
      PAD_ScanPads();
    }

    ClearScreen();
    sprintf(msg,"Press key for %s",keys_name[i]);
    WriteCentre(254, msg);
    SetScreen();

    /* check buttons state */
    quit = 0;
    while (quit == 0)
    {
      VIDEO_WaitVSync();
      PAD_ScanPads();
      p = PAD_ButtonsDown(num);

      for (j=0; j<8; j++)
      {
        if (p & pad_keys[j])
        {
           pad_keymap[num][i] = pad_keys[j];
           quit = 1;
           j = 8;   /* exit loop */
        }
      }
    }
  }
}

static void pad_update()
{
  int i;
  u16 p;
	s8 x,y;

  /* update PAD status */
  PAD_ScanPads();

  for (i=0; i<MAX_INPUTS; i++)
  {
    x = PAD_StickX (i);
    y = PAD_StickY (i);
	  p = PAD_ButtonsHeld(i);

    /* check emulated device type */
    switch (sms.device[i])
    {
      /* digital gamepad */
      case DEVICE_PAD2B:

        /* directional buttons */
        if ((p & PAD_BUTTON_UP)    || (y >  70)) input.pad[i] |= INPUT_UP;
        else if ((p & PAD_BUTTON_DOWN)  || (y < -70)) input.pad[i] |= INPUT_DOWN;
        if ((p & PAD_BUTTON_LEFT)  || (x < -60)) input.pad[i] |= INPUT_LEFT;
        else if ((p & PAD_BUTTON_RIGHT) || (x >  60)) input.pad[i] |= INPUT_RIGHT;

        break;

      /* analog devices */
      case DEVICE_LIGHTGUN:
      case DEVICE_SPORTSPAD:
      case DEVICE_PADDLE:

        /* X position */
        if (p & PAD_BUTTON_LEFT) input.analog[i][0] --;
        else if (p & PAD_BUTTON_RIGHT) input.analog[i][0] ++;
        else if (x) input.analog[i][0] = (u8)(x + 128);

        /* Y position */
        if (p & PAD_BUTTON_UP) input.analog[i][1] --;
        else if (p & PAD_BUTTON_DOWN) input.analog[i][1] ++;
        else if (y) input.analog[i][1] = (u8)(128 - y);

        /* Limiters */
        if (input.analog[i][0] < 0) input.analog[i][0] = 0;
        else if (input.analog[i][0] > 0xFF) input.analog[i][0] = 0xFF;
        if (input.analog[i][1] < 0) input.analog[i][1] = 0;
        else if (input.analog[i][1] > 0xFF) input.analog[i][1] = 0xFF;

        break;
      
      /* none */
      default:
        break;
    }

    /* MENU */
    if (p & pad_keymap[i][KEY_MENU])
    {
      ConfigRequested = 1;
      return;
    }

    /* PAUSE & START button */
    if (p & pad_keymap[i][KEY_PAUSE])
      input.system |= (sms.console == CONSOLE_GG) ? INPUT_START : INPUT_PAUSE;

    /* SOFT RESET */
    if (((p & PAD_TRIGGER_R) && (p & PAD_TRIGGER_L)) || softreset)
    {
      input.system |= INPUT_RESET;
      softreset = 0;
      SYS_SetResetCallback(set_softreset);
    }

    /* BUTTONS 1&2 */
    if (p & pad_keymap[i][KEY_BUTTON1]) input.pad[i] |= INPUT_BUTTON1;
    if (p & pad_keymap[i][KEY_BUTTON2]) input.pad[i] |= INPUT_BUTTON2;
  }
}

/*******************************
  wiimote support
*******************************/
#ifdef HW_RVL

#define PI 3.14159265f

static s8 WPAD_StickX(u8 chan,u8 right)
{
  float mag = 0.0;
  float ang = 0.0;
  WPADData *data = WPAD_Data(chan);

  switch (data->exp.type)
  {
    case WPAD_EXP_NUNCHUK:
    case WPAD_EXP_GUITARHERO3:
      if (right == 0)
      {
        mag = data->exp.nunchuk.js.mag;
        ang = data->exp.nunchuk.js.ang;
      }
      break;

    case WPAD_EXP_CLASSIC:
      if (right == 0)
      {
        mag = data->exp.classic.ljs.mag;
        ang = data->exp.classic.ljs.ang;
      }
      else
      {
        mag = data->exp.classic.rjs.mag;
        ang = data->exp.classic.rjs.ang;
      }
      break;

    default:
      break;
  }

  /* calculate X value (angle need to be converted into radian) */
  if (mag > 1.0) mag = 1.0;
  else if (mag < -1.0) mag = -1.0;
  double val = mag * sin(PI * ang/180.0f);
 
  return (s8)(val * 128.0f);
}


static s8 WPAD_StickY(u8 chan, u8 right)
{
  float mag = 0.0;
  float ang = 0.0;
  WPADData *data = WPAD_Data(chan);

  switch (data->exp.type)
  {
    case WPAD_EXP_NUNCHUK:
    case WPAD_EXP_GUITARHERO3:
      if (right == 0)
      {
        mag = data->exp.nunchuk.js.mag;
        ang = data->exp.nunchuk.js.ang;
      }
      break;

    case WPAD_EXP_CLASSIC:
      if (right == 0)
      {
        mag = data->exp.classic.ljs.mag;
        ang = data->exp.classic.ljs.ang;
      }
      else
      {
        mag = data->exp.classic.rjs.mag;
        ang = data->exp.classic.rjs.ang;
      }
      break;

    default:
      break;
  }

  /* calculate X value (angle need to be converted into radian) */
  if (mag > 1.0) mag = 1.0;
  else if (mag < -1.0) mag = -1.0;
  double val = mag * cos(PI * ang/180.0f);
 
  return (s8)(val * 128.0f);
}

static void wpad_config(u8 num)
{
  int i,j;
  int max = MAX_KEYS;
  u8 quit;
  char msg[30];
  u32 current = 255;

  /* check wiimote status */
  if (WPAD_Probe(num, &current) != WPAD_ERR_NONE)
  {
    WaitPrompt("Wiimote is not connected !");
    return;
  }

  /* index for wpad_keymap */
  u8 index = current + (num * 3);

  /* loop on each mapped keys */
  for (i=0; i<max; i++)
  {
    /* remove any pending buttons */
    while (WPAD_ButtonsHeld(num))
    {
      WPAD_ScanPads();
      VIDEO_WaitVSync();
    }

    /* user information */
    ClearScreen();
    sprintf(msg,"Press key for %s",keys_name[i]);
    WriteCentre(254, msg);
    SetScreen();

    /* wait for input */
    quit = 0;
    while (quit == 0)
    {
      WPAD_ScanPads();

      /* get buttons */
      for (j=0; j<20; j++)
      {
        if (WPAD_ButtonsDown(num) & wpad_keys[j])
        {
          wpad_keymap[index][i]  = wpad_keys[j];
          quit = 1;
          j = 20;    /* leave loop */
        }
      }
    } /* wait for input */ 
  } /* loop for all keys */

  /* removed any pending buttons */
  while (WPAD_ButtonsHeld(num))
  {
    WPAD_ScanPads();
    VIDEO_WaitVSync();
  }
}

static void wpad_update(void)
{
  int i,use_wpad;
  u32 exp;
  u32 p;
  s8 x,y;
  struct ir_t ir;

  /* update WPAD data */
  WPAD_ScanPads();

  for (i=0; i<2; i++)
  {
    /* check WPAD status */
    if ((WPAD_Probe(i, &exp) == WPAD_ERR_NONE))
    {
      p = WPAD_ButtonsHeld(i);
      x = WPAD_StickX(i, 0);
      y = WPAD_StickY(i, 0);

      if ((i == 0) && (exp == WPAD_EXP_CLASSIC)) use_wpad = 1;
      else use_wpad = 0;

      /* check emulated device type */
      switch (sms.device[i])
      {
        /* digital gamepad */
        case DEVICE_PAD2B:

          /* directional buttons */
          if ((p & wpad_dirmap[exp][PAD_UP])    || (y >  70)) input.pad[i] |= INPUT_UP;
          else if ((p & wpad_dirmap[exp][PAD_DOWN])  || (y < -70)) input.pad[i] |= INPUT_DOWN;
          if ((p & wpad_dirmap[exp][PAD_LEFT])  || (x < -60)) input.pad[i] |= INPUT_LEFT;
          else if ((p & wpad_dirmap[exp][PAD_RIGHT]) || (x >  60)) input.pad[i] |= INPUT_RIGHT;

          if (use_wpad)
          {
            if ((p & wpad_dirmap[0][PAD_UP])    || (y >  70)) input.pad[1] |= INPUT_UP;
            else if ((p & wpad_dirmap[0][PAD_DOWN])  || (y < -70)) input.pad[1] |= INPUT_DOWN;
            if ((p & wpad_dirmap[0][PAD_LEFT])  || (x < -60)) input.pad[1] |= INPUT_LEFT;
            else if ((p & wpad_dirmap[0][PAD_RIGHT]) || (x >  60)) input.pad[1] |= INPUT_RIGHT;
          }

          break;

        /* analog devices */
        case DEVICE_LIGHTGUN:
        case DEVICE_SPORTSPAD:
        case DEVICE_PADDLE:

          /* X position */
          if (p & wpad_dirmap[exp][PAD_LEFT]) input.analog[i][0] --;
          else if (p & wpad_dirmap[exp][PAD_RIGHT]) input.analog[i][0] ++;
          else if (x) input.analog[i][0] = (u8)(x + 128);

          /* Y position */
          if (p & wpad_dirmap[exp][PAD_UP]) input.analog[i][1] --;
          else if (p & wpad_dirmap[exp][PAD_DOWN]) input.analog[i][1] ++;
          else if (y) input.analog[i][1] = (u8)(128 - y);

          /* by default, use IR pointing */
          WPAD_IR(i, &ir);
          if (ir.valid)
          {
            input.analog[i][0] = (ir.x * 4) / 10;
            input.analog[i][1] = ir.y / 2;
            if (p & WPAD_BUTTON_A) input.pad[i] |= INPUT_BUTTON1;
          }

          /* limiter */
          if (input.analog[i][0] < 0) input.analog[i][0] = 0;
          else if (input.analog[i][0] > 0xFF) input.analog[i][0] = 0xFF;
          if (input.analog[i][1] < 0) input.analog[i][1] = 0;
          else if (input.analog[i][1] > 0xFF) input.analog[i][1] = 0xFF;

          break;
      
        /* none */
        default:
          break;
      }

      /* retrieve current key mapping */
      u8 index = exp + (3 * i);
   
      /* MENU */
      if ((p & wpad_keymap[index][KEY_MENU]) || (p & WPAD_BUTTON_HOME))
      {
        ConfigRequested = 1;
        return;
      }

      /* PAUSE & START */
      if (p & wpad_keymap[index][KEY_PAUSE])
        input.system |= (sms.console == CONSOLE_GG) ? INPUT_START : INPUT_PAUSE;

      /* RESET */
      if (((p & WPAD_CLASSIC_BUTTON_PLUS) && (p & WPAD_CLASSIC_BUTTON_MINUS)) ||
          ((p & WPAD_BUTTON_PLUS) && (p & WPAD_BUTTON_MINUS)) || softreset)
      {
        input.system |= INPUT_RESET;
        softreset = 0;
        SYS_SetResetCallback(set_softreset);
      }
      
      /* BUTTON 1 */
      if (p & wpad_keymap[index][KEY_BUTTON1])
        input.pad[i] |= INPUT_BUTTON1;
      if (use_wpad && (p & wpad_keymap[0][KEY_BUTTON1]))
        input.pad[1] |= INPUT_BUTTON1;

      /* BUTTON 2 */
      if (p & wpad_keymap[index][KEY_BUTTON2])
        input.pad[i] |= INPUT_BUTTON2;
      if (use_wpad && (p & wpad_keymap[0][KEY_BUTTON2]))
        input.pad[1] |= INPUT_BUTTON2;
    }
  }
}
#endif

/*****************************************************************
                Generic input handlers 
******************************************************************/

void ogc_input__init(void)
{
  PAD_Init ();

#ifdef HW_RVL
  WPAD_Init();
	WPAD_SetIdleTimeout(60);
  WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
  WPAD_SetVRes(WPAD_CHAN_ALL,640,480);
#endif
  /* register SOFTRESET */
  SYS_SetResetCallback(set_softreset);
}

void ogc_input__update(void)
{
  /* reset inputs */
  input.pad[0] = 0;
  input.pad[1] = 0;
  input.system = 0;

  pad_update();
#ifdef HW_RVL
  wpad_update();
#endif
}

void ogc_input__config(u8 pad, u8 type)
{
  switch (type)
  {
    case 0:
      pad_config(pad);
      break;
    
#ifdef HW_RVL
    case 1:
      wpad_config(pad);
      break;
#endif
    
    default:
      break;
  }
}

u16 ogc_input__getMenuButtons(void)
{
  /* slowdown input updates */
  VIDEO_WaitVSync();

  /* get gamepad inputs */
  PAD_ScanPads();
  u16 p = PAD_ButtonsDown(0);
  s8 x  = PAD_StickX(0);
  s8 y  = PAD_StickY(0);
  if (x > 70) p |= PAD_BUTTON_RIGHT;
  else if (x < -70) p |= PAD_BUTTON_LEFT;
	if (y > 60) p |= PAD_BUTTON_UP;
  else if (y < -60) p |= PAD_BUTTON_DOWN;

#ifdef HW_RVL
  /* get wiimote + expansions inputs */
  WPAD_ScanPads();
  u32 q = WPAD_ButtonsDown(0);
  u32 h = WPAD_ButtonsHeld(0);
  x = WPAD_StickX(0, 0);
  y = WPAD_StickY(0, 0);

  /* is Wiimote directed toward screen (horizontal/vertical orientation) ? */
  struct ir_t ir;
  WPAD_IR(0, &ir);

  /* wiimote directions */
  if (q & WPAD_BUTTON_UP)         p |= ir.valid ? PAD_BUTTON_UP : PAD_BUTTON_LEFT;
  else if (q & WPAD_BUTTON_DOWN)  p |= ir.valid ? PAD_BUTTON_DOWN : PAD_BUTTON_RIGHT;
  else if (q & WPAD_BUTTON_LEFT)  p |= ir.valid ? PAD_BUTTON_LEFT : PAD_BUTTON_DOWN;
  else if (q & WPAD_BUTTON_RIGHT) p |= ir.valid ? PAD_BUTTON_RIGHT : PAD_BUTTON_UP;
  
  if (h & WPAD_BUTTON_UP)
  {
    held_cnt ++;
    if (held_cnt == MAX_HELD_CNT)
    {
      held_cnt = MAX_HELD_CNT - 2;
      p |= ir.valid ? PAD_BUTTON_UP : PAD_BUTTON_LEFT;
    }
  }
  else if (h & WPAD_BUTTON_DOWN)
  {
    held_cnt ++;
    if (held_cnt == MAX_HELD_CNT)
    {
      held_cnt = MAX_HELD_CNT - 2;
      p |= ir.valid ? PAD_BUTTON_DOWN : PAD_BUTTON_RIGHT;
    }
  }
  else if (h & WPAD_BUTTON_LEFT)
  {
    held_cnt ++;
    if (held_cnt == MAX_HELD_CNT)
    {
      held_cnt = MAX_HELD_CNT - 2;
      p |= ir.valid ? PAD_BUTTON_LEFT : PAD_BUTTON_DOWN;
    }
  }
  else if (h & WPAD_BUTTON_RIGHT)
  {
    held_cnt ++;
    if (held_cnt == MAX_HELD_CNT)
    {
      held_cnt = MAX_HELD_CNT - 2;
      p |= ir.valid ? PAD_BUTTON_RIGHT : PAD_BUTTON_UP;
    }
  }
  else
  {
    held_cnt = 0;
  }

  /* analog sticks */
  if (y > 70)       p |= PAD_BUTTON_UP;
  else if (y < -70) p |= PAD_BUTTON_DOWN;
  if (x < -60)      p |= PAD_BUTTON_LEFT;
  else if (x > 60)  p |= PAD_BUTTON_RIGHT;

  /* classic controller directions */
  if (q & WPAD_CLASSIC_BUTTON_UP)         p |= PAD_BUTTON_UP;
  else if (q & WPAD_CLASSIC_BUTTON_DOWN)  p |= PAD_BUTTON_DOWN;
  if (q & WPAD_CLASSIC_BUTTON_LEFT)       p |= PAD_BUTTON_LEFT;
  else if (q & WPAD_CLASSIC_BUTTON_RIGHT) p |= PAD_BUTTON_RIGHT;

  /* wiimote keys */
  if (q & WPAD_BUTTON_MINUS)  p |= PAD_TRIGGER_L;
  if (q & WPAD_BUTTON_PLUS)   p |= PAD_TRIGGER_R;
  if (q & WPAD_BUTTON_A)      p |= PAD_BUTTON_A;
  if (q & WPAD_BUTTON_B)      p |= PAD_BUTTON_B;
  if (q & WPAD_BUTTON_2)      p |= PAD_BUTTON_A;
  if (q & WPAD_BUTTON_1)      p |= PAD_BUTTON_B;
  if (q & WPAD_BUTTON_HOME)   p |= PAD_TRIGGER_Z;

  /* classic controller keys */
  if (q & WPAD_CLASSIC_BUTTON_FULL_L) p |= PAD_TRIGGER_L;
  if (q & WPAD_CLASSIC_BUTTON_FULL_R) p |= PAD_TRIGGER_R;
  if (q & WPAD_CLASSIC_BUTTON_A)      p |= PAD_BUTTON_A;
  if (q & WPAD_CLASSIC_BUTTON_B)      p |= PAD_BUTTON_B;
  if (q & WPAD_CLASSIC_BUTTON_HOME)   p |= PAD_TRIGGER_Z;

 #endif

  return p;
}
