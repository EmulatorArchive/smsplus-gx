#ifndef _LOADROM_H_
#define _LOADROM_H_

/* Function prototypes */
int load_rom(char *filename);

#ifndef NGC
unsigned char *loadzip(char *archive, char *filename, int *filesize);
extern char game_name[PATH_MAX];
#endif

#endif /* _LOADROM_H_ */

