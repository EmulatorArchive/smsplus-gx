
#ifndef _STATE_H_
#define _STATE_H_


#define STATE_VERSION   0x0103      /* Version 1.3 (BCD) */
#define STATE_HEADER    "SST\0"     /* State file header */

/* Function prototypes */
extern int system_save_state(void *mem);
extern void system_load_state(void *mem);

#endif /* _STATE_H_ */
