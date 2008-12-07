#include "shared.h"
#include "font.h"

#include <fat.h>
#include <sys/dir.h>

#ifdef HW_RVL
#define CONFIG_VERSION "SMSPLUS 1.3.1W"
#else
#define CONFIG_VERSION "SMSPLUS 1.3.1G"
#endif

void config_save()
{
  if (!use_FAT) return;

  /* first check if directory exist */
  DIR_ITER *dir = diropen("/smsplus");
  if (dir == NULL) mkdir("/smsplus",S_IRWXU);
  else dirclose(dir);

  /* open configuration file */
  FILE *fp = fopen("/smsplus/smsplus.ini", "wb");
  if (fp == NULL) return;

  /* save options */
  fwrite(&option, sizeof(option), 1, fp);

  /* save key mapping */
  fwrite(pad_keymap, sizeof(pad_keymap), 1, fp);
#ifdef HW_RVL
  fwrite(wpad_keymap, sizeof(wpad_keymap), 1, fp);
#endif

  fclose(fp);
}

void config_load()
{
  /* open configuration file */
  FILE *fp = fopen("/smsplus/smsplus.ini", "rb");
  if (fp == NULL) return;

  /* read version */
  char version[15];
  fread(version, 15, 1, fp); 
  fclose(fp);
  if (strcmp(version,CONFIG_VERSION)) return;
  
  /* read file */
  fp = fopen("/smsplus/smsplus.ini", "rb");
  fread(&option, sizeof(option), 1, fp);

  /* load key mapping */
  fread(pad_keymap, sizeof(pad_keymap), 1, fp);
#ifdef HW_RVL
  fread(wpad_keymap, sizeof(wpad_keymap), 1, fp);
#endif

  fclose(fp);
}

/*****************************************************************************
 * set_option_defaults
 *****************************************************************************/
void set_option_defaults ()
{
  /* version TAG */
  strncpy(option.version,CONFIG_VERSION,15);

	option.sndrate      = 48000;
	option.country      = 0;
	option.console      = 0;
	option.fm_enable    = 1;
	option.fm_which     = SND_EMU2413;
  option.overscan     = 1;
  option.xshift       = 0;
  option.yshift       = 0;
  option.xscale       = 0;
  option.yscale       = 0;
  option.aspect       = 1;
  option.render       = VIDEO_HaveComponentCable() ? 2 : 0;
  option.ntsc         = 0;
  option.bilinear     = 1;
  option.tv_mode      = 0;
  option.palette      = 1;
  option.autofreeze   = -1;
  option.spritelimit  = 1;
  option.extra_gg     = 0;
}

