
#ifndef _CONFIG_H_
#define _CONFIG_H_

/****************************************************************************
 * Config Option 
 *
 ****************************************************************************/
typedef struct
{
  char version[15];
  int sndrate;
  int country;
  int console;
  int display;
  int fm_enable;
  int fm_which;
  int codies;
  int16 xshift;
  int16 yshift;
  int16 xscale;
  int16 yscale;
  uint8 tv_mode;
  uint8 aspect;
  uint8 overscan;
  uint8 render;
  uint8 ntsc;
  uint8 bilinear;
  uint8 palette;
  int8 autofreeze;
  uint8 use_bios;
  uint8 spritelimit;
  uint8 extra_gg;
} t_option;

/* Global data */
t_option option;


extern void config_save();
extern void config_load();
extern void set_option_defaults ();
extern bool use_FAT;

#endif /* _CONFIG_H_ */
