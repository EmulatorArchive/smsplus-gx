
#ifndef _OSD_H_
#define _OSD_H_

#define NGC 1

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>

#include "ogc_input.h"
#include "ogc_audio.h"
#include "ogc_video.h"


#define DEFAULT_PATH "/smsplus"

#define FATCHUNK 2048


/* globals */
extern u32 diff_usec(long long start,long long end);
extern long long gettime();

extern void legal();
extern void set_config();
extern void MainMenu();
extern int ManageState(u8 direction, u8 device);
extern void memfile_autosave();
extern void memfile_autoload();

extern int smsromsize;
extern u8 *smsrom;
extern int frameticker;
extern int Shutdown;
extern bool fat_enabled;

#endif /* _OSD_H_ */
