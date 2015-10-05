/** \file
 * Supporting functions for handling ajax in CGUI
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/cgj.c $
 *
 * $Id: cgs.c 2583 2010-02-22 21:59:11Z bkwok $
 */


#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "sys.h"
#include "tst.h"
#include "cgj.h"
#include "WGUI_Javascript.h"
#include "WGUI_Plugin.h"
#include "cgu.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
/** Handle Signature Capture Page
 * \param canvas (I) Canvas created by CGUI
 * \param browser (I) Browser created by CGUI
 * \param form (I) Pointer to a Form_Handle_t structure containing form properties
 * \sa Header: cgs.h
 * \sa Source: cgs.c
*/
int HandleAjaxPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser, Form_Handle_t * form)
{
	WGUI_HTML_EVENT_HANDLE evt = 0;
	
    // Write page to the display
    WGUI_Browser_WriteHtmlAscii(browser, form->html_code, 1);

	
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
		
		// Get events
		WGUI_Browser_GetHtmlEvent(browser, &evt);

        if (evt)
        {
			WGUI_HTML_EVENT_TYPE type = 0;
			WGUI_HtmlEvent_GetType(evt, &type);
			if(type == WGUI_HTML_EVENT_XHR_GET)
			{
				char valueStr[32];
				WGUI_JSON_NODE json;
				WGUI_HtmlEvent_GetVariableAscii(evt,"variables",valueStr,sizeof(valueStr));
				json = WGUI_JSON_CreateObject();
				WGUI_JSON_AddItemStringAscii(json,"linedisplay1","Hello AJAX");
				WGUI_JSON_SendResponse(browser,json,evt);
				WGUI_JSON_DestroyObject(json);
				evt = 0;
			}
			else if (type == WGUI_HTML_EVENT_GET)
			{
				break;
			}
        }


	}
	while (evt == 0);
	
    // Save variables passed back from HTML
	SaveFormReturnValues(evt);
	
	return 0;
}


#ifdef __cplusplus
}
#endif
