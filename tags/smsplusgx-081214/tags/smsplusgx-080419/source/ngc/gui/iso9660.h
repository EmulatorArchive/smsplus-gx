/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
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
 * DVD ISO9660/Joliet Parsing
 *
 * This is not intended as a complete guide to ISO9660.
 * Here I use the bare minimum!
 ***************************************************************************/

#define MAXJOLIET 256
#define MAXFILES 1000 /** Restrict to 1000 files per dir **/

typedef struct
{
  u64 offset;
  unsigned int length;
  char flags;
  char filename[MAXJOLIET];
  u16 filename_offset;
} FILEENTRIES;

extern u64 basedir;
extern u64 rootdir;
extern int rootdirlength;

extern int getpvd ();
extern int parseDVDdirectory ();
extern FILEENTRIES filelist[MAXFILES];
