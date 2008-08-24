#ifndef _OSD_H_
#define _OSD_H_

#define NGC 1

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fat.h>
#include <sys/dir.h>

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif

#include "ogc_input.h"
#include "ogc_audio.h"
#include "ogc_video.h"
#include "history.h"

/* globals */
extern u32 diff_usec(long long start,long long end);
extern long long gettime();

extern void legal();
extern void set_config();
extern void MainMenu();
extern int ManageState(u8 direction, u8 device);
extern int OpenDVD();
extern int OpenSD();
extern int OpenHistory();
extern void memfile_autoload();
extern void memfile_autosave();

extern int smsromsize;
extern u8 *smsrom;
extern int frameticker;

#endif /* _OSD_H_ */
