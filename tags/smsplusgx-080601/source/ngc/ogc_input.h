/******************************************************************************
 *
 *  SMS Plus - Sega Master System / GameGear Emulator
 *
 *  NGC/Wii Controller support
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
 ***************************************************************************/
#ifndef _GC_INPUT_H_
#define _GC_INPUT_H_

extern int ConfigRequested;

extern void ogc_input__init(void);
extern void ogc_input__update(void);
extern void ogc_input__config(u8 pad, u8 type);
extern u16 ogc_input__getMenuButtons(void);

#endif
