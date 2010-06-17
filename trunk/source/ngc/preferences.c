/*
 * preferences.c
 * 
 *   Handle Preferences via XML
 *
 *   code by Talantyyr (2010) 
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
 *******************************************************************************/
 
#include <mxml.h>

#include "shared.h"
#include "preferences.h"
#include "font.h"

struct SSMSSettings SMSSettings;


static mxml_node_t *xml = NULL;
static mxml_node_t *data = NULL;
static mxml_node_t *section = NULL;
static mxml_node_t *item = NULL;
 
static void createXMLSection(const char * name, const char * description)
{
	section = mxmlNewElement(data, "section");
	mxmlElementSetAttr(section, "name", name);
	mxmlElementSetAttr(section, "description", description);
}

static void createXMLSetting(const char * name, const char * description, const char * value)
{
	item = mxmlNewElement(section, "setting");
	mxmlElementSetAttr(item, "name", name);
	mxmlElementSetAttr(item, "value", value);
	mxmlElementSetAttr(item, "description", description);
}


/***************************************************************************
 * saveSettings()
 *
 * Save Settings to XML File. 
 ***************************************************************************/ 
int saveSettings()
{
	xml = mxmlNewXML("1.0");
	mxmlSetWrapMargin(0); // disable line wrapping

	data = mxmlNewElement(xml, "file");
	
	createXMLSection("Network", "Network Settings");
	createXMLSetting("ip", "Share Computer IP", SMSSettings.ip);
	createXMLSetting("share", "Share Name", SMSSettings.share);
	createXMLSetting("user", "Share Username", SMSSettings.user);
	createXMLSetting("pwd", "Share Password", SMSSettings.pwd);
			
	FILE *file;
	file = fopen("/smsplus/settings.xml", "wb");
	
	if (file == NULL)
  {
	  WaitPrompt("File = NULL");
		fclose(file);
		return 0;
	} 
	else 
	{
		mxmlSaveFile(xml, file, MXML_NO_CALLBACK);
		fclose(file);
		mxmlDelete(data);
		mxmlDelete(xml);
		return 1;
	}
	return 0;
}


/***************************************************************************
 * loadSettings()
 *
 * Load Settings from XML File. 
 ***************************************************************************/ 
int loadSettings() 
{
	FILE *fp = fopen("/smsplus/settings.xml", "rb");
		
	if (fp == NULL) //File does not exist. create one!
	{
		fclose(fp);
		saveSettings();
	} 
	else 
	{
		fseek (fp , 0, SEEK_END);
		long settings_size = ftell (fp);
		rewind (fp);
		
		if (settings_size > 0) 
		{
			xml = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
			fclose(fp);
			
			data = mxmlFindElement(xml, xml, "settings", NULL, NULL,
			MXML_DESCEND);

			item = mxmlFindElement(xml, xml, "setting", "name", "ip", MXML_DESCEND);
			if(item)
			{
				sprintf(SMSSettings.ip, "%s", mxmlElementGetAttr(item,"value"));
			}
			
			item = mxmlFindElement(xml, xml, "setting", "name", "share", MXML_DESCEND);
			if(item)
			{
				sprintf(SMSSettings.share, "%s", mxmlElementGetAttr(item,"value"));
			}
			
			item = mxmlFindElement(xml, xml, "setting", "name", "user", MXML_DESCEND);
			if(item)
			{
				sprintf(SMSSettings.user, "%s", mxmlElementGetAttr(item,"value"));
			}
			
			item = mxmlFindElement(xml, xml, "setting", "name", "pwd", MXML_DESCEND);
			if(item)
			{
				sprintf(SMSSettings.pwd, "%s", mxmlElementGetAttr(item,"value"));
			}
			
			mxmlDelete(data);
			mxmlDelete(xml);
			return 1;
    } 
		else 
		{
			WaitPrompt("Settings.xml not OK. Please check your Settings.xml!");
			fclose(fp);
			return 0;
		}
	}
	return 0;
}
