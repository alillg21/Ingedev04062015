/** \file
 * Supporting functions for interfacing to CGUI
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/cgu.c $
 *
 * $Id: cgu.c 2598 2010-03-26 20:21:44Z bkwok $
 */


#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "sys.h"
#include "tst.h"
#include "cgu.h"
#include "cgs.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
	
/*  Magnetic Swipe Related Functions */
static void CloseMagnetic();
static void OpenMagnetic();
static int CheckSwipe(void); 
static void IsoError(int iSta, char *pcTrk);

#define MAX_HTML_RETURN_BUFFER_SIZE 4096
#define MAX_VAR_NAME_SIZE 256
#define MAX_VAR_VALUE_SIZE 1024

void SaveFormReturnValues(WGUI_HTML_EVENT_HANDLE evt)
{	
	char returnValue[MAX_HTML_RETURN_BUFFER_SIZE];
	char * prev_pch;
	char * pch;
	int count=0;
	char name[MAX_VAR_NAME_SIZE];
	char value[MAX_VAR_VALUE_SIZE];
	
	memset(returnValue, 0, sizeof(returnValue));
	memset(name, 0, sizeof(name));
	memset(value, 0, sizeof(value));
	
	WGUI_HtmlEvent_GetVariableAscii(evt, "return", returnValue, 4096);	
	
	// Add an ending ~ if not there
	if (strlen(returnValue))
	{
		if (returnValue[strlen(returnValue)-1] != '~')
			strcat(returnValue,"~");
	}
	
	prev_pch = returnValue;
	pch=strchr(returnValue,'~');
	
#ifdef __DEBUG__
			// Need a printf at the beginning for the first name value pair to show up
			font_size(4,8);        
			printf ("Variables\n");
			ttestall(0, 10);
#endif
			
	while (pch!=NULL)
	{
	    count++;

		if (count%2 != 0)
		{
			*pch='\0';
			strcpy(name, prev_pch);
		}
		else
		{
			*pch='\0';
			strcpy(value, prev_pch);
			// VarSaveNameValuePair();
#ifdef __DEBUG__
			font_size(4,8);        
			printf ("%s=%s\n",name, value);
			ttestall(0, 10);
#endif
			memset(name, 0, sizeof(name));
			memset(value, 0, sizeof(value));
		
		}
		prev_pch = pch+1;
	    pch=strchr(pch+1,'~');
	}
	
	// Save last variable if it has no value
	if (strlen(name))
	{
		// VarSaveNameValuePair();		
#ifdef __DEBUG__
		font_size(4,8);        
		printf ("%s=%s\n",name, value);
#endif
	}
	
#ifdef __DEBUG__
	ttestall(0, 100);
#endif
}

void StartCGUI()
{
	// Initialize WGUI library
	WGUI_DLL_Init();
	
	CGUI_CONTEXT_STRUCT CGUIContext;

	CGUIContext.CGUIStatus = CGUI_NEEDED;
	CGUIContext.HeaderStatus = _PERIPH_OFF;
	CGUIContext.FooterStatus = _PERIPH_OFF;
	CGUIContext.LedsStatus = _PERIPH_OFF;
	CGUIContext.CanvasStatus = CANVAS_MAXIMIZE;
	PushCGUIContext(&CGUIContext);
}

void EndCGUI()
{
	
	PopCGUIContext();
	
	// Terminate WGUI library
	WGUI_DLL_Terminate();
}

/** Handle Plain HTML Page
 * \param canvas (I) Canvas created by CGUI
 * \param browser (I) Browser created by CGUI
 * \param form (I) Pointer to a Form_Handle_t structure containing form properties
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
static int HandlePlainPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser, Form_Handle_t * form)
{
	WGUI_HTML_EVENT_HANDLE evt = 0;
   
    WGUI_Browser_WriteHtmlAscii(browser, form->html_code, 1);

    if (WGUI_Plugin_Register(browser, "MMPLAYER") == WGUI_OK)
    {
    	// Wait page submission
		do
		{
			int i;

			// Set the call ratio of WGUI_Canvas_DispatchEvent/ProcessSwipeEvent to 5:1 for UI performance
			for (i=0;i<5;++i)
			{
				// Dispatch all events (keyboard, mouse, timer) to the canvas
				WGUI_Canvas_DispatchEvent(canvas);
			}
			
			if (form->enable_msr)
			{
				if (ProcessSwipeEvent())
				{
					break;
				}
			}

			
			// Get page submission
			WGUI_Browser_GetHtmlEvent(browser, &evt);
		}
		while (evt == 0);
		
		// Save variables passed back from HTML
		SaveFormReturnValues(evt);
    }
    
	return 0;
}

/** Display a form in CGUI
 * \param form (I) Pointer to a Form_Handle_t structure containing form properties
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
int displayForm(Form_Handle_t * form)
{
	unsigned int width;
	unsigned int height;
	WGUI_ERROR display;
	FILE * keyboard=NULL;
	FILE * mouse=NULL;
	FILE * tscreen=NULL;
	int event = 0;
	unsigned int AccessMode = FS_WRITEMANY;
	int ret;
	
	keyboard = fopen("KEYBOARD","r*");
	mouse = fopen("MOUSE","r*");
	tscreen = fopen("TSCREEN", "r*");
	
    ret = FS_mount("/HOST", &AccessMode);
    
    if (ret == FS_OK)
    {
		// Get screen size info
		WGUI_Display_GetInfo(&width, &height, 0);
	
	    // Initialize the display
	    display = WGUI_Display_Create(0,0,width,height,WGUI_DISPLAY_COLORS);
	    if (display == WGUI_OK || display == WGUI_DISPLAY_ALREADY_CREATE_ERR)
		{
			WGUI_CANVAS_HANDLE canvas;
	
			// Create canvas
			canvas = WGUI_Canvas_Create(WGUI_CANVAS_SCREEN, 0, 0, width, height);
			
	        // Display mouse cursor
#ifdef __DEBUG__
			WGUI_Canvas_SetCursor(canvas, WGUI_CURSOR_CROSS);
#else
			WGUI_Canvas_SetCursor(canvas, WGUI_CURSOR_NONE);
#endif
	
			// If canvas created
			if (canvas)
			{
				WGUI_BROWSER_HANDLE browser;
	
				// Create browser
				browser = WGUI_Browser_Create(canvas);
	
				if (form->enable_msr)
				{
					OpenMagnetic();
				}
			    
				// If browser created
				if (browser)
				{
					if (form->type == FORM_TYPE_SIGNCAPT)
					{
						event = HandleSignCaptPage(canvas, browser, form);
					}
					else if (form->type == FORM_TYPE_PINENTRY)
					{
						event = HandlePinEntryPage(canvas, browser, form);
					}
					else if (form->type == FORM_TYPE_AJAX)
					{
						event = HandleAjaxPage(canvas, browser, form);
					}
					else
					{
						event = HandlePlainPage(canvas, browser, form);
					}
										
	            	// Destroy browser
	            	WGUI_Browser_Destroy(browser);
				}
				// Destroy canvas
				WGUI_Canvas_Destroy(canvas);
			}
			
			if (form->enable_msr)
			{
				CloseMagnetic();
			}
	
	        // Destroy display
	        if (display == WGUI_OK)
	        {
	            WGUI_Display_Destroy();
	        }		
		}
  	    FS_unmount("/HOST");
    }
    
	if(mouse) fclose(mouse);
	if(keyboard) fclose(keyboard);
	if (tscreen) fclose(tscreen);

	return event;
}	


/**********************************************************************/
// Magnetic Swipe Reader Related Functions
/**********************************************************************/

static FILE *pxMag13=NULL, *pxMag2=NULL, *pxMag3=NULL;
static char tcTrk1[128], tcTrk2[128], tcTrk3[128];
	
/** Open the Magnetic Card Reader
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
static void OpenMagnetic()
{
	// Open peripherals
	// ================
    pxMag2 = fopen("SWIPE2", "r*"); 						           // Open "mag2" channel (must be supported, check return code)
	pxMag13 = fopen("SWIPE31", "r*");                                  // Open "mag13" channel (Could not be supported)
    pxMag3 = fopen("SWIPE3", "r*");                                    // Open "mag3" channel (Could not be supported)
}

/** Close the Magnetic Card Reader
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
static void CloseMagnetic()
{
    if(pxMag2)
		fclose(pxMag2);                                                // Close "mag2" channel 
	if(pxMag3)
		fclose(pxMag3);                                                // Close "mag3" channel
	if(pxMag13)
		fclose(pxMag13);                                               // Close "mag13" channel
}

/** Check for swipe at the Magnetic Card Reader
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
static int CheckSwipe(void) 
{
	// Local variables 
    // ***************
	byte ucLen=0;
    int iSta=0;
    int iNum=0;
    char tcTmp[128];
    char *pcSrc, *pcDst, cKey;	
	int iRet;
	bool bSwiped = FALSE;

	iSta |= ttestall(iSta ^ (SWIPE31 | SWIPE2 | SWIPE3), 1);      // Wait for the second event ISO2
	
    if(iSta & KEYBOARD)                                            // Key event
	{
		cKey = getchar();
		return TRUE;
	}

	if(iSta & SWIPE31) 
	{
		bSwiped = TRUE;
		memset(tcTmp, 0, sizeof(tcTmp));
	    memset(tcTrk1, 0, sizeof(tcTrk1));
		iRet = is_iso1(pxMag13, &ucLen, tcTmp);                        // *** Read ISO1 to ascii format ***
		if (iRet != ISO_OK)
			IsoError (iRet, tcTrk1);
		else
		{
			CHECK(strlen(tcTmp)<128, lblKO); 
			iNum++;
			pcSrc = tcTmp;
			pcDst = tcTrk1;
			while(*pcSrc) {                                            // Find start sentinel
				if(*pcSrc++ == '%')
					break;
			}
			while(*pcSrc) {                                            // Copy all data between start and end sentinels
				if(*pcSrc == '?')
					break;
				*pcDst++ = *pcSrc++;
			}
		}
	}
		
	// Retrieve and analyse ISO2
	// =========================
	if(iSta & SWIPE2)  
	{
		bSwiped = TRUE;
		memset(tcTmp, 0, sizeof(tcTmp));
	    memset(tcTrk2, 0, sizeof(tcTrk2));                            
		iRet = is_iso2(pxMag2, &ucLen, tcTmp);                         // *** Read ISO2 to ascii format ***
		if (iRet != ISO_OK)   
			IsoError (iRet, tcTrk2);
		else
		{
			CHECK(strlen(tcTmp)<128, lblKO);
			iNum++;
			pcSrc = tcTmp;
			pcDst = tcTrk2;
			while(*pcSrc) {                                            // Find start sentinel
				if(*pcSrc++ == 'B')
					break;
			}
			while(*pcSrc) {                                            // Copy all data between start and end sentinels
				if(*pcSrc == 'F') 
					break;
				if(*pcSrc == 'D')
					*pcSrc = '=';
				*pcDst++ = *pcSrc++;
			}
		}
	}
		
	// Retrieve and analyse ISO3
	// =========================
	if(iSta & SWIPE3) 
	{
		bSwiped = TRUE;
		memset(tcTmp, 0, sizeof(tcTmp));
	    memset(tcTrk3, 0, sizeof(tcTrk3)); 
		iRet = is_iso3(pxMag3, &ucLen, tcTmp);                         // *** Read ISO3 to ascci format ***
		if (iRet != ISO_OK)
			IsoError (iRet, tcTrk3);
		else
		{
			CHECK(strlen(tcTmp)<128, lblKO);
			iNum++;
			pcSrc = tcTmp;
			pcDst = tcTrk3;
			while(*pcSrc) {                                            // Find start sentinel
				if(*pcSrc++ == 'B')
					break;
			}
			while(*pcSrc) {                                            // Copy all data between start and end sentinels
				if(*pcSrc == 'F')
					break;
				if(*pcSrc == 'D')
					*pcSrc = '=';
				*pcDst++ = *pcSrc++;
			}
		}
	}
    
    
	return bSwiped;
	
lblKO:
	return FALSE;
}

/** Check for ISO Error at the swipe data
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
static void IsoError(int iSta, char *pcTrk)
{
	switch (iSta)
	{
		case DEF_SEP: strcpy(pcTrk, "Separator fault");  break;
		case DEF_PAR: strcpy(pcTrk, "Parity fault");     break;
		case DEF_LRC: strcpy(pcTrk, "LRC fault");        break;
		case DEF_LUH: strcpy(pcTrk, "Luhn fault");       break;
		case DEF_NUM: strcpy(pcTrk, "Numerical fault");  break;
		case NO_DATA: strcpy(pcTrk, "No readable data"); break;
		default:      strcpy(pcTrk, "Unknown error");    break;
	}
}

/** Process Swipe Reader Events
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
int ProcessSwipeEvent()
{
	if (CheckSwipe())
	{
		return TRUE;
	}
	
	return FALSE;
}



#ifdef __cplusplus
}
#endif
