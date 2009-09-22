/*
    input.c --
    DOS input management.

    To do:
    - Mouse is assigned to player #1 inputs.
*/
#include "osd.h"


/* Check if a key is pressed */
int check_key(int code)
{
    static int lastbuf[0x100] = {0};

    if(!key[code] && lastbuf[code] == 1)
        lastbuf[code] = 0;

    if(key[code] && lastbuf[code] == 0)
    {
        lastbuf[code] = 1;
        return 1;
    }

    return 0;
}


void osd_update_inputs(void)
{
    /* Reset input states */
    input.system    = 0;
    input.pad[0]    = 0;
    input.pad[1]    = 0;

    /* Alt+W : Dump work RAM */
    if((key[KEY_ALT] || key[KEY_ALTGR]) && check_key(KEY_W))
        dump_wram();

    /* Alt+V : Dump video RAM */
    if((key[KEY_ALT] || key[KEY_ALTGR]) && check_key(KEY_V))
        dump_vram();

    /* DEL : User-requested hard reset */
    if(check_key(KEY_DEL))
        system_reset();

    /* ENTER | SPACE : SMS PAUSE or GG START button */
    if(key[KEY_ENTER] || key[KEY_SPACE])
        input.system |= (sms.console == CONSOLE_GG) ? INPUT_START : INPUT_PAUSE;

    /* Player #1 inputs */
    if (sms.device[0] == DEVICE_PAD2B)
    {
      if(key[KEY_UP])         input.pad[0] |= INPUT_UP;
      else
      if(key[KEY_DOWN])       input.pad[0] |= INPUT_DOWN;
      if(key[KEY_LEFT])       input.pad[0] |= INPUT_LEFT;
      else
      if(key[KEY_RIGHT])      input.pad[0] |= INPUT_RIGHT;
      if(key[KEY_A])          input.pad[0] |= INPUT_BUTTON2;
      if(key[KEY_S])          input.pad[0] |= INPUT_BUTTON1;
    }

    if (sms.console == CONSOLE_COLECO)
    {
      input.system = 0;
      coleco.keypad[0] = 0xff;
      coleco.keypad[1] = 0xff;

      if(key[KEY_0])            coleco.keypad[0] = 0;
      else if(key[KEY_1])       coleco.keypad[0] = 1;
      else if(key[KEY_2])       coleco.keypad[0] = 2;
      else if(key[KEY_3])       coleco.keypad[0] = 3;
      else if(key[KEY_4])       coleco.keypad[0] = 4;
      else if(key[KEY_5])       coleco.keypad[0] = 5;
      else if(key[KEY_6])       coleco.keypad[0] = 6;
      else if(key[KEY_7])       coleco.keypad[0] = 7;
      else if(key[KEY_8])       coleco.keypad[0] = 8;
      else if(key[KEY_9])       coleco.keypad[0] = 9;
      else if(key[KEY_RSHIFT])  coleco.keypad[0] = 10;
      else if(key[KEY_LSHIFT])  coleco.keypad[0] = 11;
    }

    if (sms.device[1] == DEVICE_PAD2B)
    {
      /* Player #2 inputs */
      if(key[KEY_8_PAD])
      {
        input.pad[1] |= INPUT_UP;
        if(input.analog[1][1] > 0) input.analog[1][0] --;
      }
      else
      if(key[KEY_2_PAD])
      {
        input.pad[1] |= INPUT_DOWN;
        if(input.analog[1][1] < 0xFF) input.analog[1][0] ++;
      }
      if(key[KEY_4_PAD])
      {
        input.pad[1] |= INPUT_LEFT;
        if(input.analog[1][0] > 0) input.analog[1][0] --;
      }
      else
      if(key[KEY_6_PAD])
      {
        input.pad[1] |= INPUT_RIGHT;
       if(input.analog[1][0] < 0xFF) input.analog[1][0] ++;
      }
      
      if(key[KEY_1_PAD])      input.pad[1] |= INPUT_BUTTON2;
      if(key[KEY_3_PAD])      input.pad[1] |= INPUT_BUTTON1;

    }
    
    /* TAB : SMS RESET button */
    if(key[KEY_TAB]) input.system |= INPUT_RESET;

    /* Check for any joysticks present */
    if(option.joy_driver != JOY_TYPE_NONE)
    {              
        poll_joystick();

        /* Parse player 1 joystick state */
        if(joy[0].stick[0].axis[1].d1) input.pad[0] |= INPUT_UP;
        else
        if(joy[0].stick[0].axis[1].d2) input.pad[0] |= INPUT_DOWN;
        if(joy[0].stick[0].axis[0].d1) input.pad[0] |= INPUT_LEFT;
        else
        if(joy[0].stick[0].axis[0].d2) input.pad[0] |= INPUT_RIGHT;
        if(joy[0].button[0].b)  input.pad[0] |= INPUT_BUTTON2;
        if(joy[0].button[1].b)  input.pad[0] |= INPUT_BUTTON1;
        if(joy[0].button[2].b)  input.system |= (IS_GG) ? INPUT_START : INPUT_PAUSE;

        /* Check if a 2nd joystick is present */
        if(num_joysticks > 2)
        {
            /* Parse player 2 joystick state  */
            if(joy[1].stick[0].axis[1].d1) input.pad[1] |= INPUT_UP;
            else
            if(joy[1].stick[0].axis[1].d2) input.pad[1] |= INPUT_DOWN;
            if(joy[1].stick[0].axis[0].d1) input.pad[1] |= INPUT_LEFT;
            else
            if(joy[1].stick[0].axis[0].d1) input.pad[1] |= INPUT_RIGHT;
            if(joy[1].button[0].b)  input.pad[1] |= INPUT_BUTTON2;
            if(joy[1].button[1].b)  input.pad[1] |= INPUT_BUTTON1;
        }
    }

    /* Check if a mouse is present. */
    if(use_mouse)
    {
        int temp;

        /* Poll mouse if necessary */
        if(mouse_needs_poll() == TRUE)
            poll_mouse();

        /* Calculate X axis value */
        temp = mouse_x;
        if(temp > 0xFF) temp = 0xFF;
        if(temp < 0x00) temp = 0x00;
        input.analog[0][0] = temp;

        /* Calculate Y axis value */
        temp = mouse_y;
        if(temp > 0xFF) temp = 0xFF;
        if(temp < 0x00) temp = 0x00;
        input.analog[0][1] = temp;

        /* Map mouse buttons to player #1 inputs */
        if(mouse_b & 4) input.pad[0] |= (IS_GG) ? INPUT_START : INPUT_PAUSE;
        if(mouse_b & 2) input.pad[0] |= INPUT_BUTTON2;
        if(mouse_b & 1) input.pad[0] |= INPUT_BUTTON1;
    }
}

