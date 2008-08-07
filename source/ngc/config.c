#include "shared.h"
#include "font.h"

#include <fat.h>
#include <sys/dir.h>

void config_save()
{
  /* first check if directory exist */
  DIR_ITER *dir = diropen("/smsplus");
  if (dir == NULL) mkdir("/smsplus",S_IRWXU);
  else dirclose(dir);

  /* open configuration file */
  FILE *fp = fopen("/smsplus/smsplus.ini", "wb");
  if (fp == NULL) return;

  /* save options */
  fwrite(&option, sizeof(option), 1, fp);
  fclose(fp);
}

void config_load()
{
  /* open configuration file */
  FILE *fp = fopen("/smsplus/smsplus.ini", "rb");
  if (fp == NULL) return;

  /* read file */
  fread(&option, sizeof(option), 1, fp);
  fclose(fp);
}

/*****************************************************************************
 * set_option_defaults
 *****************************************************************************/
void set_option_defaults ()
{
	option.sndrate    = 48000;
	option.country    = 0;
	option.console    = 0;
	option.fm_enable  = 1;
	option.fm_which   = SND_EMU2413;
  option.overscan   = 1;
  option.xshift     = 0;
  option.yshift     = 0;
  option.xscale     = 0;
  option.yscale     = 0;
  option.aspect     = 1;
  option.render     = (vmode->viTVMode == VI_TVMODE_NTSC_PROG) ? 2 : 0;
  option.tv_mode    = 0;
  option.palette    = 1;
  option.autofreeze = -1;
}

