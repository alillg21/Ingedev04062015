/** \file
 * Unitary non UTA test case for simulating UPOS Line Display
 *
 * Subversion reference
 *
 * $HeadURL:  $
 *
 * $Id:  $
 */

#include "SDK30.H"
#include "WGUI.h"


#ifdef __cplusplus
extern "C"
{
#endif
   
typedef struct {
   bool new;
   int x;
   int y;
   int cx;
   int cy;
   char* command;
   char* element_id;
   char* text;
} lineDisplayMsg;



#define MAX_LD_MSG 14

lineDisplayMsg ldMsg[MAX_LD_MSG] = {
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Display Hello World at 0,3"},
   {TRUE, 0, 3, 0, 0, "display_text_at", "line_display", "Hello World"},
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Display Hello World at 7,6"},
   {TRUE, 7, 6, 0, 0, "display_text_at", "line_display", "Hello World"},
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Display Hello World at 7,7"},
   {TRUE, 7, 7, 0, 0, "display_text_at", "line_display", "Hello World"},
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Display Hello World at 20,0"},
   {TRUE, 20, 0, 0, 0, "display_text_at", "line_display", "Hello World"},
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Display Hello World at 20,1"},
   {TRUE, 20, 1, 0, 0, "display_text_at", "line_display", "Hello World"},
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Clear lines 6-7"},
   {TRUE, 0, 6, 0, 2, "clear_rows", "line_display", " "},
   {TRUE, 0, 0, 0, 0, "display_text", "scroll_display", "Clear all lines"},
   {TRUE, 0, 0, 0, 0, "clear_all", "line_display", " "},
   
}; 

void sendMsgToJavaScript(lineDisplayMsg ldMsg, WGUI_BROWSER_HANDLE browser) {
/* JSON change in SDK 7.1

   WGUI_ERROR wError;
   WGUI_JSON_HANDLE json_array;
   WGUI_JSON_HANDLE json_x;
   WGUI_JSON_HANDLE json_y;
   WGUI_JSON_HANDLE json_cx;
   WGUI_JSON_HANDLE json_cy;
   WGUI_JSON_HANDLE json_command;
   WGUI_JSON_HANDLE json_element_id;
   WGUI_JSON_HANDLE json_text;

   json_array = WGUI_JSON_CreateArray();

   json_x = WGUI_JSON_CreateInt(ldMsg.x);
   json_y = WGUI_JSON_CreateInt(ldMsg.y);
   json_cx = WGUI_JSON_CreateInt(ldMsg.cx);
   json_cy = WGUI_JSON_CreateInt(ldMsg.cy);
   json_command = WGUI_JSON_CreateStringAscii(ldMsg.command);
   json_element_id = WGUI_JSON_CreateStringAscii(ldMsg.element_id);
   json_text = WGUI_JSON_CreateStringAscii(ldMsg.text);

   wError = WGUI_JSON_AddArray(json_array, json_x);
   wError = WGUI_JSON_AddArray(json_array, json_y);
   wError = WGUI_JSON_AddArray(json_array, json_cx);
   wError = WGUI_JSON_AddArray(json_array, json_cy);
   wError = WGUI_JSON_AddArray(json_array, json_command);
   wError = WGUI_JSON_AddArray(json_array, json_element_id);
   wError = WGUI_JSON_AddArray(json_array, json_text);


   wError = WGUI_JSON_SendResponse(json_array, browser);
   wError = WGUI_JSON_DestroyObject(json_x);
   wError = WGUI_JSON_DestroyObject(json_y);
   wError = WGUI_JSON_DestroyObject(json_cx);
   wError = WGUI_JSON_DestroyObject(json_cy);
   wError = WGUI_JSON_DestroyObject(json_command);
   wError = WGUI_JSON_DestroyObject(json_element_id);
   wError = WGUI_JSON_DestroyObject(json_text);
   wError = WGUI_JSON_DestroyObject(json_array);
*/
}

/** Handle Plain HTML Page
 * \param canvas (I) Canvas created by CGUI
 * \param browser (I) Browser created by CGUI
*/
static int HandlePlainPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser)
{
   WGUI_HTML_EVENT_HANDLE evt = 0;
   int i = 0;
   
    // Write page to the display
   WGUI_Browser_LoadUrl(browser, "file:///flash/HOST/LINEDISPLAY.HTM", 1);

   // Wait page submission
   do
   {
      // Dispatch all events (keyboard, mouse, timer) to the canvas
      WGUI_Canvas_DispatchEvent(canvas);
      

      
      // Get page submission
      WGUI_Browser_GetHtmlEvent(browser, &evt);
      
      if (evt)
      {
          WGUI_HTML_EVENT_TYPE type = 0;
          WGUI_HtmlEvent_GetType(evt, &type);
          if(type == WGUI_HTML_EVENT_XHR_GET && i < MAX_LD_MSG)
          {
             WGUI_Tool_Sleep(2000);
             sendMsgToJavaScript(ldMsg[i], browser);
          }
      }
   }
   while (evt == 0);
   
   return 0;
}

/** Display a form in CGUI
 * \param none
*/
int displayMyForm(void)
{
   unsigned int width;
   unsigned int height;
   WGUI_ERROR display;
   FILE * keyboard=NULL;
   FILE * mouse=NULL;
   int event = 0;
   
   keyboard = fopen("KEYBOARD","r*");
   mouse = fopen("MOUSE","r*");
   
   // Initialize WGUI library
   WGUI_DLL_Init();

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
        WGUI_Canvas_SetCursor(canvas, WGUI_CURSOR_CROSS);

      // If canvas created
      if (canvas)
      {
         WGUI_BROWSER_HANDLE browser;

         // Create browser
         browser = WGUI_Browser_Create(canvas);

          
         // If browser created
         if (browser)
         {
            event = HandlePlainPage(canvas, browser);   
            // Destroy browser
            WGUI_Browser_Destroy(browser);
         }
         // Destroy canvas
         WGUI_Canvas_Destroy(canvas);
      }
      
        // Destroy display
        if (display == WGUI_OK)
        {
            WGUI_Display_Destroy();
        }      
   }
    
   if(mouse) fclose(mouse);
   if(keyboard) fclose(keyboard);
   
   // Terminate WGUI library
   WGUI_DLL_Terminate();

   return event;
}  


void tcgz002(void)
{
     int event = displayMyForm();
}

#ifdef __cplusplus
}
#endif

