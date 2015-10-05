/** \file
 * Supporting functions related to form management in CGUI
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/fom.c $
 *
 * $Id: fom.c 2598 2010-03-26 20:21:44Z bkwok $
 */

#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "sys.h"
#include "tst.h"
#include "fom.h"


#ifdef __cplusplus
extern "C"
{
#endif

Form_Handle_t formDef[MAX_NUM_FORM];
int formDefNext = 0;

Form_Handle_t * loadForm(char * filename, int type, bool enableMSR, bool enableRFID)
{
	unsigned int AccessMode = FS_WRITEONCE ;
	Form_Handle_t * ret;
	int retCode;
	 
	VERIFY(filename);
	
	if (formDefNext == MAX_NUM_FORM)
		return NULL;
	
	// If form already exists
	if ((ret = getForm(filename)) != NULL)
		return ret;
	
	retCode=FS_mount("/HOST", &AccessMode);
	
	if (retCode == FS_OK)
	{
		static S_FS_FILE * fd;
		
		fd = FS_open(filename, "r");
		
		if (fd != NULL)
		{
			unsigned long size = FS_length(fd);
			
			char * buf =  (char *) umalloc(size+5);
			
			memset(buf, 0, size+5);
			FS_read(buf, size, 1, fd);
			FS_close(fd);
			
			strncpy(formDef[formDefNext].name, filename, MAX_FILE_LENGTH);
			formDef[formDefNext].type = type;
			formDef[formDefNext].enable_msr = enableMSR;
			formDef[formDefNext].enable_rfid = enableRFID;
			formDef[formDefNext].html_code = buf;
			formDefNext++;
			FS_unmount("/HOST");
			return &formDef[formDefNext-1];
		}
	}
	return NULL;
}

Form_Handle_t *  getForm(char * filename)
{
	int i;
	
	VERIFY(filename);
	
	for (i=0;i<formDefNext;i++)
	{
		if (stricmp(filename, formDef[i].name) == 0)
		{
			return &formDef[i];
		}
	}
	return NULL;
}


#ifdef __cplusplus
}
#endif
