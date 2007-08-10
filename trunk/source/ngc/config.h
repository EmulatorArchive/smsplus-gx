
#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct
{
  int joy_driver;

  int video_driver;
  int video_depth;
  int video_width;
  int video_height;

  int no_vga;
  int no_mmx;
  int expand;
  int blur;
  int scale;
  int scanlines;
  int tweak;

  int vsync;
  int throttle;
  int display;

  int sound;
  int sndcard;
  int sndrate;

  int country;
  int console;
  int fm_enable;
  int fm_which;
  int codies;
} t_option;

extern t_option sms_option;

#endif /* _CONFIG_H_ */
