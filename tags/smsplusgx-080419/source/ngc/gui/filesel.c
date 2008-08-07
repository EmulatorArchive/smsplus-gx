/****************************************************************************
 *  Sega Master System / GameGear Emulator
 *
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
 * File Selection
 ***************************************************************************/
#include "shared.h"
#include "dvd.h"
#include "iso9660.h"
#include "font.h"
#include "unzip.h"
#include "vfat.h"

#define PAGESIZE 12
#define PADCAL 30

static int maxfiles;
static int offset = 0;
static int selection = 0;
static int old_selection = 0;
static int old_offset = 0;

static char rootSDdir[SDCARD_MAX_PATH_LEN];
static u8 haveDVDdir = 0;
static u8 haveSDdir  = 0;
static VFATFS fs;

u8 sdslot = 0;
u8 UseSDCARD = 0;
sd_file * filehandle;
FSDIRENTRY fsfile;

static int LoadFile (unsigned char *buffer);

/***************************************************************************
 * Showfile screen
 ***************************************************************************/
static void ShowFiles (int offset, int selection) 
{
  int i, j;
  char text[MAXJOLIET+2];

  ClearScreen ();
  j = 0;

  for (i = offset; i < (offset + PAGESIZE) && (i < maxfiles); i++)
  {
	  memset(text,0,MAXJOLIET+2);
	  if (filelist[i].flags) sprintf(text, "[%s]", filelist[i].filename + filelist[i].filename_offset);
	  else sprintf (text, "%s", filelist[i].filename + filelist[i].filename_offset);
      if (j == (selection - offset)) WriteCentre_HL ((j * fheight) + 120, text);
      else WriteCentre ((j * fheight) + 120, text);
	  j++;
  }
  SetScreen ();
}

/***************************************************************************
 * Update SDCARD curent directory name 
 ***************************************************************************/ 
int updateSDdirname()
{
  int size=0;
  char *test;
  char temp[1024];

   /* current directory doesn't change */
   if (strcmp(filelist[selection].filename,".") == 0) return 0; 
   
   /* go up to parent directory */
   else if (strcmp(filelist[selection].filename,"..") == 0) 
   {
     /* determine last subdirectory namelength */
     sprintf(temp,"%s",rootSDdir);

    if (sdslot < 2)
    {
      test= strtok(temp,"\\");
      while (test != NULL)
      { 
        size = strlen(test);
        test = strtok(NULL,"\\");
      }
    }
    else
    {
      test= strtok(temp,"/");
      while (test != NULL)
      {
        size = strlen(test);
        test = strtok(NULL,"/");
      }
    }
  
    /* remove last subdirectory name */
    size = strlen(rootSDdir) - size - 1;
    rootSDdir[size] = 0;

    if (sdslot < 2)
    {
	  /* handles root name */
      if (strcmp(rootSDdir,sdslot ?  "dev1:":"dev0:") == 0)
        sprintf(rootSDdir,"dev%d:\\genplus\\..",sdslot);
    }
	 
     return 1;
   }
   else
   {
    if (sdslot < 2)
    {
      /* test new directory namelength */
      if ((strlen(rootSDdir)+1+strlen(filelist[selection].filename)) < SDCARD_MAX_PATH_LEN) 
      {
        /* handles root name */
        if (strcmp(rootSDdir,sdslot ? "dev1:\\smsplus\\..":"dev0:\\smsplus\\..") == 0)
          sprintf(rootSDdir,"dev%d:",sdslot);
	 
        /* update current directory name */
        sprintf(rootSDdir, "%s\\%s",rootSDdir, filelist[selection].filename);
      
        return 1;
      }
      else
      {
        WaitPrompt ("Dirname is too long !"); 
        return -1;
      }
    } 
    else
    {
      sprintf(rootSDdir, "%s/%s",rootSDdir, filelist[selection].filename);
      return 1;
    }
  }
}

/***************************************************************************
 * Browse SDCARD subdirectories 
 ***************************************************************************/ 
int parseSDdirectory()
{
  int nbfiles = 0;

  if (sdslot < 2)
  {
    int entries = 0;
    DIR *sddir = NULL;

	/* List directory */
    SDCARD_Init ();
    entries = SDCARD_ReadDir (rootSDdir, &sddir) - 1;
    if (entries < 0) entries = 0;   
    if (entries > MAXFILES) entries = MAXFILES;
    
    /* Move to File structure - this is required for the file selector */ 
    while (entries)
    {
      memset (&filelist[nbfiles], 0, sizeof (FILEENTRIES));
      strncpy(filelist[nbfiles].filename,(char *) sddir[nbfiles+1].fname,MAXJOLIET);
      filelist[nbfiles].filename[MAXJOLIET-1] = 0;
	  filelist[nbfiles].length = sddir[nbfiles+1].fsize;
	  filelist[nbfiles].flags = (char)(sddir[nbfiles+1].fattr & SDCARD_ATTR_DIR);
      nbfiles++;
      entries--;
    }
  
    /* Release memory */
    free(sddir);
  }
  else
  {
	  /* Front SD*/
    FSDIRENTRY fsdir;
    int res = VFAT_opendir(0, &fsdir, rootSDdir);
	  if (res != FS_SUCCESS)
	  {
      WaitPrompt ("Error opening dir");
      nbfiles = 0;
    }
    else
    {
      while (VFAT_readdir(&fsdir) == FS_SUCCESS)
      {
        memset (&filelist[nbfiles], 0, sizeof (FILEENTRIES));
        strncpy(filelist[nbfiles].filename,(char *)(fsdir.longname),MAX_LONG_NAME);
        filelist[nbfiles].filename[MAX_LONG_NAME-1] = 0;
        filelist[nbfiles].length = fsdir.fsize;
        filelist[nbfiles].flags = (char)(fsdir.dirent.attribute & ATTR_DIRECTORY);
        nbfiles++;
      }
      VFAT_closedir(&fsdir);
    }
  }
  
  return nbfiles;
}

/****************************************************************************
 * FileSelector
 *
 * Let user select a file from the File listing
 ****************************************************************************/
extern int smsromsize;
extern uint8 *smsrom;
extern void viewport_init();

void FileSelector () 
{
  short p;
  signed char a,b;
  int haverom = 0;
  int redraw = 1;
  int go_up = 0;
	int i,size;

  while (haverom == 0)
  {
    if (redraw) ShowFiles (offset, selection);
    redraw = 0;
    p = PAD_ButtonsDown (0);
    a = PAD_StickY (0);
		b = PAD_StickX (0);
     
		/* scroll displayed filename */
  		if ((p & PAD_BUTTON_LEFT) || (b < -PADCAL))
		{
			if (filelist[selection].filename_offset > 0)
			{
				filelist[selection].filename_offset --;
				redraw = 1;
			}
		}
		else if ((p & PAD_BUTTON_RIGHT) || (b > PADCAL))
		{
			size = 0;
			for (i=filelist[selection].filename_offset; i<strlen(filelist[selection].filename); i++)
				size += font_size[(int)filelist[selection].filename[i]];
		  
			if (size > back_framewidth)
			{
				filelist[selection].filename_offset ++;
				redraw = 1;
			}
    	}
    
		/* highlight next item */
		else if ((p & PAD_BUTTON_DOWN) || (a < -PADCAL))
		{
			filelist[selection].filename_offset = 0;
	  		selection++;
	  		if (selection == maxfiles) selection = offset = 0;
	  		if ((selection - offset) >= PAGESIZE) offset += PAGESIZE;
	  		redraw = 1;
		}

		/* highlight previous item */
		else if ((p & PAD_BUTTON_UP) || (a > PADCAL))
		{
			filelist[selection].filename_offset = 0;
	  		selection--;
		  	if (selection < 0)
	      	{
	        	selection = maxfiles - 1;
	        	offset = selection - PAGESIZE + 1;
	      	}
	      	if (selection < offset) offset -= PAGESIZE;
		  	if (offset < 0)  offset = 0;
		  	redraw = 1;
		}
     
		/* go back one page */
		else if (p & PAD_TRIGGER_L)
		{
			filelist[selection].filename_offset = 0;
	  		selection -= PAGESIZE;
	  		if (selection < 0)
	  		{
	      		selection = maxfiles - 1;
	      		offset = selection - PAGESIZE + 1;
	  		}
	  		if (selection < offset) offset -= PAGESIZE;
	  		if (offset < 0) offset = 0;
	  		redraw = 1;
		}

		/* go forward one page */
		else if (p & PAD_TRIGGER_R)
		{
			filelist[selection].filename_offset = 0;
	  		selection += PAGESIZE;
	  		if (selection > maxfiles - 1) selection = offset = 0;
	  		if ((selection - offset) >= PAGESIZE) offset += PAGESIZE;
	  		redraw = 1;
		}

		/* go up one directory or quit */
		if (p & PAD_BUTTON_B)
		{
			filelist[selection].filename_offset = 0;
            if (UseSDCARD)
            {
                if ((sdslot == 0) && (strcmp(rootSDdir,"dev0:\\smsplus\\..") == 0)) return;
                else if (((sdslot == 1) && (strcmp(rootSDdir,"dev1:\\smsplus\\..") == 0))) return;
                else if (((sdslot == 2) && (strcmp(rootSDdir,"/") == 0))) return;
            }
            else if (basedir == rootdir) return;
			go_up = 1;
		}

		/* quit */
		if (p & PAD_TRIGGER_Z)
		{
			filelist[selection].filename_offset = 0;
			return;
		}

		/* open selected file or directory */
    	if ((p & PAD_BUTTON_A) || go_up)
    	{
			filelist[selection].filename_offset = 0;
      		if (go_up)
      		{
                /* select item #1 */
	    		go_up = 0;
                selection = UseSDCARD ? 0 : 1;
      		}
	  
			/*** This is directory ***/
			if (filelist[selection].flags)
	  		{
				if (UseSDCARD) /* SDCARD directory handler */
        		{
					/* update current directory */
		  			int status = updateSDdirname();

					/* move to new directory */
		  			if (status == 1)
		  			{
						/* reinit selector (previous value is saved for one level) */
						if (selection == 0)
						{
							selection = old_selection;
							offset = old_offset;
							old_selection = 0;
							old_offset = 0;
						}
						else
						{
							/* save current selector value */
							old_selection = selection;
							old_offset = offset;
							selection = 0;
							offset = 0;
						}
						

						/* set new entry list */
		     			maxfiles = parseSDdirectory();
		     			if (!maxfiles)
		     			{
							/* quit */
			    			WaitPrompt ("Error reading directory !");
							haverom   = 1;
							haveSDdir = 0;
			 			}
		  			}
		  			else if (status == -1)
         			{
						/* quit */
						haverom   = 1;
						haveSDdir = 0;
          			}
        		}
				else /* DVD directory handler */
        		{
					/* move to a new directory */
					if (selection != 0)
					{
						/* update current directory */
	       				rootdir = filelist[selection].offset;
	       				rootdirlength = filelist[selection].length;
				  
						/* reinit selector (previous value is saved for one level) */
						if (selection == 1)
						{
							selection = old_selection;
							offset = old_offset;
							old_selection = 0;
							old_offset = 0;
						}
						else
						{
							/* save current selector value */
							old_selection = selection;
							old_offset = offset;
							selection = 0;
							offset = 0;
						}

						/* get new entry list */
						maxfiles = parseDVDdirectory ();
	    			}
	  			}
			}
			else /*** This is a file ***/
	  		{
	      		rootdir = filelist[selection].offset;
	      		rootdirlength = filelist[selection].length;
	      		memset(smsrom, 0, 1048576);
				smsromsize = LoadFile (smsrom);
	      		load_rom ("");
		  		viewport_init();
				system_poweron ();
				haverom = 1;
	  		}
	  		redraw = 1;
		}
  	}
}

/****************************************************************************
 * OpenDVD
 *
 * Function to load a DVD directory and display to user.
 ****************************************************************************/
void OpenDVD () 
{
  UseSDCARD = 0;
  if (!getpvd())
  {
	ShowAction("Mounting DVD ... Wait");
	DVD_Mount();
	haveDVDdir = 0;
	if (!getpvd())
	{
		WaitPrompt ("Failed to mount DVD");
        return;
	}
  }
  
  if (haveDVDdir == 0)
  {
    /* don't mess with SD entries */
	haveSDdir = 0;
	 
	/* reinit selector */
    rootdir = basedir;
    old_selection = selection = offset = old_offset = 0;
	 
	if ((maxfiles = parseDVDdirectory ()))
	{
	  FileSelector ();
	  haveDVDdir = 1;
	}
  }
  else FileSelector ();
}

/****************************************************************************
 * OpenSD updated to use the new libogc.  Written by softdev and pasted
 * into this code by Drack.
 * Modified for subdirectory browing & quick filelist recovery
 * Enjoy!
*****************************************************************************/
int OpenSD (uint8 slot) 
{
  UseSDCARD = 1;
  
  if (slot != sdslot) haveSDdir = 0;
  
  if (haveSDdir == 0)
  {
    /* don't mess with DVD entries */
	haveDVDdir = 0;
	 
    /* reinit selector */
	old_selection = selection = offset = old_offset = 0;
	
    /* Reset SDCARD root directory */
    if (slot == 2)
    {
      /* front SD */
      char msg[20];
      VFAT_unmount(0, &fs);
      int res = VFAT_mount(FS_SLOTA, &fs);
      if (res != FS_TYPE_FAT16)
      {
        sprintf(msg,"Error mounting SDCARD: %d", res);
        WaitPrompt (msg);
        return 0;
	  }
      
      sprintf(rootSDdir,"/smsplus/roms");
	}
    else sprintf(rootSDdir,"dev%d:\\smsplus\\roms", slot);

	 /* save SD slot */
	 sdslot = slot;
 
	 /* Parse initial root directory and get entries list */
     ShowAction("Reading Directory ...");
	 if ((maxfiles = parseSDdirectory ()))
	 {
       /* Select an entry */
	   FileSelector ();
    
       /* memorize last entries list, actual root directory and selection for next access */
	     haveSDdir = 1;
	 }
	 else
	 {
		/* no entries found */
		WaitPrompt ("Error reading \\smsplus\\roms");
		return 0;
	 }
  }
 
  /* Retrieve previous entries list and made a new selection */
  else  FileSelector ();
  
  return 1;
}


/****************************************************************************
 * LoadFile
 *
 * This function will load a file from DVD or SDCARD, in BIN, SMD or ZIP format.
 * The values for offset and length are inherited from rootdir and 
 * rootdirlength.
 *
 * The buffer parameter should re-use the initial ROM buffer.
 ****************************************************************************/ 
int LoadFile (unsigned char *buffer) 
{
  int readoffset;
  int blocks;
  int i;
  u64 discoffset = 0;
  char readbuffer[2048];
  
  /* SDCard Addition */ 
  if (UseSDCARD)
  {
    /* Open File */
    if (sdslot < 2)
    {
      char fname[SDCARD_MAX_PATH_LEN];
 
      /* Check filename length */
      if ((strlen(rootSDdir)+1+strlen(filelist[selection].filename)) < SDCARD_MAX_PATH_LEN)
        sprintf(fname, "%s\\%s",rootSDdir,filelist[selection].filename); 
      else
      {
        WaitPrompt ("Maximum Filename Length reached !"); 
        haveSDdir = 0; // reset everything before next access
      }

      filehandle = SDCARD_OpenFile (fname, "rb");
      if (filehandle == NULL)
      {
        WaitPrompt ("Unable to open file!");
        return 0;
      }

      /* is this really needed ? */
      rootdirlength = SDCARD_GetFileSize (filehandle);
    }
    else
    {
      char fname[MAX_LONG_NAME];

      sprintf(fname, "%s/%s",rootSDdir,filelist[selection].filename); 

      int res = VFAT_fopen(0, &fsfile, fname, FS_READ);
      if (res != FS_SUCCESS )
      {
          WaitPrompt ("Unable to open file!");
          return 0;
      }
    }
  }
  
  /* How many 2k blocks to read */ 
  if (rootdirlength == 0) return 0;
  blocks = rootdirlength / 2048;
  readoffset = 0;

  ShowAction ("Loading ... Wait");

  /* Read first data chunk */
  if (UseSDCARD)
  {
    if (sdslot < 2) SDCARD_ReadFile (filehandle, &readbuffer, 2048);
    else VFAT_fread(&fsfile, readbuffer, 2048);
  }
  else
  {
    discoffset = rootdir;
    dvd_read (&readbuffer, 2048, discoffset);
  }
  
  /* determine file type */
  if (!IsZipFile ((char *) readbuffer))
  {
    /* go back to file start */
    if (UseSDCARD)
    {
      if (sdslot < 2) SDCARD_SeekFile (filehandle, 0, SDCARD_SEEK_SET);
      else VFAT_fseek(&fsfile, 0, SEEK_SET);
    }
    
    /* read data chunks */
    for (i = 0; i < blocks; i++)
    {
	  if (UseSDCARD)
      {
        if (sdslot < 2) SDCARD_ReadFile (filehandle, &readbuffer, 2048);
        else VFAT_fread(&fsfile, readbuffer, 2048);
      }
	  else
      {
        dvd_read(readbuffer, 2048, discoffset);
   		discoffset += 2048;
      }

	  memcpy (buffer + readoffset, readbuffer, 2048);
	  readoffset += 2048;
	}

	/* final read */ 
    if (rootdirlength % 2048)
	{
	  i = rootdirlength % 2048;
	  if (UseSDCARD)
      {
        if (sdslot < 2) SDCARD_ReadFile (filehandle, &readbuffer, i);
        else VFAT_fread(&fsfile, readbuffer, i);
      }
      else
      {
        dvd_read (readbuffer, 2048, discoffset);
      }
      
	  memcpy (buffer + readoffset, readbuffer, i);
	}
  }
  else
  {
    /* unzip file */
    return UnZipBuffer (buffer, discoffset, rootdirlength);
  }
  
  /* close SD file */
  if (UseSDCARD)
  {
    if (sdslot < 2) SDCARD_CloseFile (filehandle);
    else VFAT_fclose(&fsfile);
  }

  return rootdirlength;
}
