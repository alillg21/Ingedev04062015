/** \file
 * Supporting functions for handling signature capture in CGUI
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/cgs.c $
 *
 * $Id: cgs.c 2583 2010-02-22 21:59:11Z bkwok $
 */


#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "sys.h"
#include "tst.h"
#include "cgu.h"
#include "WGUI_Javascript.h"
#include "WGUI_Plugin.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
static void JS_mousemove (WGUI_JS_CallbackCtx * ctx);
static void JS_mousedown (WGUI_JS_CallbackCtx * ctx);
static void JS_mouseup (WGUI_JS_CallbackCtx * ctx);
static void appli_output (int x, int y);

/** Handle Signature Capture Page
 * \param canvas (I) Canvas created by CGUI
 * \param browser (I) Browser created by CGUI
 * \param form (I) Pointer to a Form_Handle_t structure containing form properties
 * \sa Header: cgs.h
 * \sa Source: cgs.c
*/
int HandleSignCaptPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser, Form_Handle_t * form)
{
	WGUI_HTML_EVENT_HANDLE evt = 0;
	
    // Write page to the display
    WGUI_Browser_WriteHtmlAscii(browser, form->html_code, 1);

	WGUI_JS_RegisterCallback(browser , "c_mousemove", JS_mousemove, 2);
	WGUI_JS_RegisterCallback(browser , "c_mousedown", JS_mousedown, 2);
	WGUI_JS_RegisterCallback(browser , "c_mouseup", JS_mouseup, 2);
	
	
	if (WGUI_Plugin_Register(browser, "SIGNCAPT") == WGUI_OK)
    {	// Wait page submission
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


static S_FS_FILE * fdout = NULL;
int appli_output_run = 0;

/** Callback function to catch the mouse movement
 * \param ctx (I) Pointer to a WGUI_JS_CallbackCtx structure contain the call back data
 * \sa Header: cgs.h
 * \sa Source: cgs.c
*/
static void JS_mousemove (WGUI_JS_CallbackCtx * ctx)
{
	long x;
	long y;
	
	WGUI_JS_GetParamInt(ctx, 0, &x);
	WGUI_JS_GetParamInt(ctx, 1, &y);
	
	if (NULL == fdout)
	{
//		unsigned int AccessMode = FS_WRITEMANY;
//		int ret;
		
//        ret=FS_mount("/HOST", &AccessMode);
//        if (ret == FS_OK)
//        {
//        	fdout = FS_open("/HOST/s_realtime.dat", "a");
//        }
	} //endif
	
	if (appli_output_run)
	{
		appli_output(x, y);
	}
	
	WGUI_JS_ReturnInt(ctx, 0 );
}

/** Callback function to catch the mouse press
 * \param ctx (I) Pointer to a WGUI_JS_CallbackCtx structure contain the call back data
 * \sa Header: cgs.h
 * \sa Source: cgs.c
*/
static void JS_mousedown (WGUI_JS_CallbackCtx * ctx)
{
	long x;
	long y;
	
	WGUI_JS_GetParamInt(ctx, 0, &x);
	WGUI_JS_GetParamInt(ctx, 1, &y);
	appli_output_run = 1;
	appli_output(x, y);
	WGUI_JS_ReturnInt(ctx, 0 );
}

/** Callback function to catch the mouse release
 * \param ctx (I) Pointer to a WGUI_JS_CallbackCtx structure contain the call back data
 * \sa Header: cgs.h
 * \sa Source: cgs.c
*/
static void JS_mouseup (WGUI_JS_CallbackCtx * ctx)
{
	long x;
	long y;
	
	WGUI_JS_GetParamInt(ctx, 0, &x);
	WGUI_JS_GetParamInt(ctx, 1, &y);
	
	if (appli_output_run)
	{
		appli_output(x, y);
	} //endif
	
	appli_output(-1, -1);
	appli_output_run = 0;
	
	if (NULL != fdout)
	{
		FS_close(fdout);
		fdout = NULL;
  	    FS_unmount("/HOST");
	} //endif
	
	WGUI_JS_ReturnInt(ctx, 0 );
}

/** Output the coordinates to an external device
 * \param ctx (I) Pointer to a WGUI_JS_CallbackCtx structure contain the call back data
 * \sa Header: cgs.h
 * \sa Source: cgs.c
*/
static void appli_output (int x, int y)
{
   char buffer[20];

   memset(buffer, 0, 20);
   
   if (fdout!=NULL)
   {
      if (x>=0 && y>=0)
      {
          sprintf(buffer, "%5d %5d\r\n", x, y);
   	  	  FS_write(buffer, strlen(buffer), 1, fdout);
      }
      else
      {
          sprintf(buffer, "---break---\r\n");
          FS_write(buffer, strlen(buffer), 1, fdout);
      } //endif
   } //endif
   
}

#ifdef __cplusplus
}
#endif
