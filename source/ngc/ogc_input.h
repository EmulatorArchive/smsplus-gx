/****************************************************************************
 *  ogc_input.c
 *
 *  SMS Plus GX input support
 *
 *  code by Eke-Eke (2008)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/


#ifndef _GC_INPUT_H_
#define _GC_INPUT_H_

/* max number of inputs */
#define MAX_INPUTS 2

/* number of configurable keys */
#define MAX_KEYS 5

typedef struct 
{
  char msg[16];
  u32 frames;
} t_osd;


extern int ConfigRequested;
extern t_osd osd;

extern u16 pad_keymap[MAX_INPUTS][MAX_KEYS];
#ifdef HW_RVL
extern u32 wpad_keymap[MAX_INPUTS*3][MAX_KEYS];
#endif

extern void ogc_input__init(void);
extern void ogc_input__update(void);
extern void ogc_input__config(u8 pad, u8 type);
extern u16 ogc_input__getMenuButtons(void);

#endif
