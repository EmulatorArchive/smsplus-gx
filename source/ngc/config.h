
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
  int fps;

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

/* Global data */
t_option option;

#ifndef NGC
/* Function prototypes*/
void do_config(char *file);
void parse_args(int argc, char **argv);
int parse_file(const char *filename, int *argc, char **argv);
void set_option_defaults(void);
#endif

extern uint8 overscan;

#endif /* _CONFIG_H_ */
