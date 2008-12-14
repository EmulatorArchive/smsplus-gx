/****************************************************************************
* FAT16 - VFAT Support
*
* NOTE: Only supports FAT16 with Long File Names
*       I have no interest in adding FAT32
*
* Reference Documentation:
*
*	FAT: General Overview of On-Disk Format
*	Version 1.02 May 05, 1999
*	Microsoft Corporation
*
*       FAT: General Overview of On-Disk Format
*	Version 1.03 December 06, 2000
*	Microsoft Corporation
*
* This is targetted at MMC/SD cards.
*
* Copyright softdev 2007. All rights reserved.
*
* Diskio Module
* -------------
*
* This module is almost identical to the one found in ChaN's TinyFAT FS.
* It's a logical abstration after all :)
*
* This covers stdio file on a SD image file
*
* 03/08: quickly modified by eke-eke to support Wii Front SD
****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sdio.h>
#include "vfat.h"

#define CARDIO_ERROR_READY				 0


/* End of not so public exports */

/****************************************************************************
* DISKIO_Init
*
* Initialise communication with the disc
****************************************************************************/
int DISKIO_Init( int drive )
{
  int res;

  if ( drive < 0 || drive > 1 )
    return FS_ERR_PARAM;

  res = sd_init();

  if ( res == CARDIO_ERROR_READY )
    return FS_SUCCESS;

  return FS_ERR_IO;

}

/****************************************************************************
* DISKIO_ReadSectors
*
* Read sectors from the disc
****************************************************************************/
int DISKIO_ReadSectors( int drive, void *buffer, int sector, int count )
{
  int res = -1;
  int i;
  int bytes = 0;

  if ( drive != 0 && drive != 1 )
    return FS_ERR_PARAM;		/* Must be 0 or 1 */

  /* libOGC appears to only read in single sectors */
  for( i = 0; i < count; i++ )
    {
		res = sd_read(sector + i, buffer + bytes);
	  if ( res != CARDIO_ERROR_READY )
        return FS_ERR_IO;
      bytes += SECTOR_SIZE;
    }

  if ( res == CARDIO_ERROR_READY )
    return FS_SUCCESS;

  return FS_ERR_IO;

}

