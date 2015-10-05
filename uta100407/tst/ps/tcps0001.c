#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unicapt.h>
#include <optic.h>
#include <opfont.h>

/*
// malloc/free declaration
#ifdef WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif
*/
#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

#if 0

static uint32 hHmi= 0;
int _dspStart(void){
    int ret;
    hmiOutputConfig_t cfg;    
    trcFS("%s\n","dspStart");

    ret= hmiOpen("DEFAULT",&hHmi);
    CHECK(ret==RET_OK,lblKO);
    
    //read the configuration to modify it
    ret= hmiOutputConfigRead(hHmi,&cfg);
    CHECK(ret==RET_OK,lblKO);
    
    //turn on backlighting
    cfg.backlight= 100;
    cfg.fields= HMI_OUTPUT_CONFIG_BACKLIGHT;
    ret= hmiOutputConfigWrite(hHmi,&cfg);
    CHECK(ret==RET_OK,lblKO);
    
    return 1;
lblKO:
    trcErr(ret);
    return -1;
}

int _dspStop(void){
    int ret;    
    trcFS("%s\n","dspStop");
    
    ret= hmiClose(hHmi);
    //CHECK(ret==RET_OK,lblKO);
    
    hHmi= 0;
    return hHmi;
//lblKO:
//    return -1;
}

//Unitary test case tcps0001.
// testing guiMBNW, guiMBTO, guiMB, guiPBPerct, guiPBMinMax

//===========================================================================
// GUI components
//===========================================================================

//===========================================================================
// messagebox component
//===========================================================================

// constants and return values for messageboxes 
enum eMb{
        mbBeg,                                                          // begin sentinel
        mbResBtn1,                                                      // button #1 chosen
        mbResBtn2,                                                      // button #2 chosen
        mbResBtn3,                                                      // button #3 chosen
        mbResTimeout,                                           // timeout expired
        mbIcoNone,                                                      // no icon shown
        mbIcoStop,                                                      // stop icon
        mbIcoHand,                                                      // hand icon
        mbIcoExcl,                                                      // exclamation icon
        mbIcoQuest,                                                     // questionmark icon
        mbIcoUser,                                                      // user defined icon
        mbBtnYes,                                                       // "Yes" button
        mbBtnNo,                                                        // "No" button
        mbBtnCancel,                                            // "Cancel" button
        mbBtnAbort,                                                     // "Abort" button
        mbBtnRetry,                                                     // "Retry" button
        mbBtnsPic,                                                      // Picture style buttons
        mbBtnsTxt,                                                      // Text buttons
        mbColorNormal,                                          // Normal colors (fore=black)
        mbColorReverse,                                         // Reverse colors (fore=white)
        mbTypeCustom,                                           // custom type messagebox
        mbErrOpenGui,                                           // error while opening the gui
        mbErrMallocBmp,                                         // error while malloc screen bmp
        mbEnd                                                           // end sentinel
};

//===========================================================================
// guiMBX: showing a messagebox (all of parameters could be given)
//===========================================================================
// return value : the choice or error if negative
//                                mbResBtn1, mbResBtn2, mbResBtn3 
//                                or mbResTimeout if no choice
//===========================================================================
// parameters   : see below
//===========================================================================
// Note                 : btnCnt==0 is not allowed when modal
//===========================================================================
// Modifications
// created              PS      22/04/2005
//===========================================================================
//int guiMBX(   const int type, const int btnCnt,const char* btns,const char * title,   const char * msg,const int icon,const int timeout);
int guiMBX(
        const int type,                                 // only type==mbTypeCustom is implemented
        const int btnCnt,                               // buttons' count (0, 1, 2, 3 allowed)
//      const Pchar * btns,                             // array of button text strings
        const char * btns,                              // buttons' text, semicolon (;) separated
        const char * title,                             // title (caption) of the messagebox
        const char * msg,                               // the message to show
        const int icon,                                 // icon's key, not implemented yet
        const int timeout                               // >0 - wait timeout secs or choice
                                                                        // 0 - nowait, just show the messagebox
                                                                        // <0 - infinite till choice (Modal)
);

//===========================================================================
// guiMBMD: showing a modal messagebox (infinite loop till choice)
//===========================================================================
// return value : the choice or error if negative
//                                or the choice: mbResBtn1, mbResBtn2, mbResBtn3 
//                                (if we have 2 buttons on position #2 and #3
//                                mbResBtn1 means pos#2 button chosen)
//===========================================================================
// parameters   : see below
//===========================================================================
// Modifications
// created              PS      22/04/2005
//===========================================================================
int guiMBMD(
        const int btnCnt,                               // buttons' count (1, 2, 3 allowed)
//      const Pchar * btns,                             // array of button text strings
        const char * btns,                              // buttons' text, semicolon (;) separated
        const char * title,                             // title (caption) of the messagebox
        const char * msg                                // the message to show
);

//===========================================================================
// guiMBNW              : showing (just drawing) a messagebox 
//                                (NW - nowait:without waiting on choice)
//===========================================================================
// returns              : 0 if ok or negative if error (no choice returned)
//===========================================================================
// parameters   : see below
//===========================================================================
// Note                 : Here btnCnt==0 is allowed
//===========================================================================
// Modifications
// created              PS      22/04/2005
//===========================================================================
int guiMBNW(
        const int btnCnt,                               // buttons' count (0, 1, 2, 3 allowed)
//      const Pchar * btns,                             // array of button text strings
        const char * btns,                              // buttons' text, semicolon (;) separated
        const char * title,                             // title (caption) of the messagebox
        const char * msg                                // the message to show
);

//===========================================================================
// guiMBTO: showing a messagebox with timeout
//===========================================================================
// return value : the choice or error if negative
//                                mbResBtn1, mbResBtn2, mbResBtn3 
//                                or mbResTimeout if no choice
//===========================================================================
// parameters   : see below
//===========================================================================
// Note                 : btnCnt==0 is allowed
//===========================================================================
// Modifications
// created              PS      22/04/2005
//===========================================================================
int guiMBTO(
        const int btnCnt,                               // buttons' count (0, 1, 2, 3 allowed)
//      const Pchar * btns,                             // array of button text strings
        const char * btns,                              // buttons' text, semicolon (;) separated
        const char * title,                             // title (caption) of the messagebox
        const char * msg,                               // the message to show
        const int timeout                               // >0 only - wait timeout secs or choice
);

//===========================================================================
// guiBox: showing a simple box
//===========================================================================
// return value : 0 or error if negative
//===========================================================================
// parameters   : coords, 
//                                type == 0 - draw simple frame
//                                type == 1 - draw frame without the edge 
//                                (stg like small rounding)
//                                fill: 1 fill, 0 no fill
//===========================================================================
// Modifications
// created              PS      22/04/2005
//===========================================================================
int guiBox(int x, int y, int w, int h, int type, int fill);

//===========================================================================
// guiBoxTitled: showing a title and detail box (no text shown)
//===========================================================================
// return value : 0 or error if negative
//===========================================================================
// parameters   : coords, h == total height
//                                type == 0 - draw simple frame
//                                type == 1 - draw frame without the edge 
//                                (stg like small rounding)
//                                titlesize: height of the upper box
//===========================================================================
// Modifications
// created              PS      22/04/2005
//===========================================================================
int guiBoxTitled(int x, int y, int w, int h, int type, int titlesize);

//===========================================================================
// guiPBPerct: showing a progressbar messagebox with percentage value
// guiPBMinMax: showing a progressbar messagebox with min, max and value
//===========================================================================
// return value : RET_OK or error if negative
//===========================================================================
// parameters:
//      style           : 
//      title           : msgbox's caption
//      msg                     : the text to be displayed
//      perct           : the percentage
//      minval          : the minimal value to be displayed
//      maxval          : the maximal value to be displayed
//      value           : the value to be displayed
//===========================================================================
int guiPBPerct(const int style, const char * perct, const char * title, const char * msg);
int guiPBMinMax(const int style, const char * min, const char * max, const char * value, const char * title, const char * msg);
int guiPBPure(const char * perct, const char * title, const char * msg);

//===========================================================================
// guiHG - NOT IMPLEMENTED
//      showing hourglas
//===========================================================================
// type: 0 only (extendible to clock, etc.)
// value: 0..360 grades (0 and 90 is allowed now)
//===========================================================================
int guiHG(int type, int value);

//===========================================================================
// guiPicX - NOT IMPLEMENTED
//      showing a picture in a window
//===========================================================================
// showing the given picture
//===========================================================================
int guiPicX(int x, int y, int w, int h, int style, void* pic, int size);

//===========================================================================
// guiAddPic, guiDelPic, guiSetPic, guiPicExists, guiGetNewPicID 
//                                              NOT IMPLEMENTED 
//      managing pictures in the db
//===========================================================================
int guiAddPic(int key, int w, int h, void* pic, int size);
int guiDelPic(int key);
int guiSetPic(int key, int w, int h, void* pic, int size);
int guiPicExists(int key);
int guiGetNewPicID();
//===========================================================================
// guiPic - NOT IMPLEMENTED
//      showing a picture in a window
//===========================================================================
// showing a picture given by the key
//===========================================================================
int guiPic(int key, int x, int y, int mode);

//===========================================================================
// prtPic - NOT IMPLEMENTED
//      showing a picture
//===========================================================================
// type = 0 custom picture, otherwise system picture:
// 1:single line, 2: dotted line, 4: double line, 
// 8: company logo, 16: ...
// x: horizontal position
// style: not specified yet
// pic's address if it's a custom picture
//===========================================================================
int prtPic(int type, int x, int style, void* pic, int size);

//===========================================================================
// dspCur - NOT IMPLEMENTED
//      get a key with blinking cursor (uses timer 0)
//===========================================================================
// parameters:
//      x, y : coords IN PIXEL
//===========================================================================
int guiCurGetKeyXY(int x, int y);

//===========================================================================
// guiEnterNumBase - enter a number with blinking cursor and MsgBox frame
//===========================================================================
// Enter a number, line#3, pos#3
//===========================================================================
// options              : mask character for password and PIN
//===========================================================================
//      uses            : hHmi
//===========================================================================
static int guiEnterNumBase(
        char * lpBuffer,                        // holds the result and can hold initial value
        const int maxLen,               // maximal length (without the '\0' terminator)
        const char mask                 // mask character for password and pin
);

int guiEnterPIN(char *str);
int guiEnterPwd(char *str, byte len);
int guiEnterAmt(char *amt, byte len, byte exp);
//===========================================================================
// GUI routines
//===========================================================================
static int guiDrawBox(Canvas* pCanv, GC* pGC, int x, int y, int w, int h, int type, int fill);
static int guiDrawBoxTitled(Canvas* pCanv, GC* pGC, int x, int y, int w, int h, int type, int titlesize);
static int guiDrawMsgBox(const int btnCnt, const char * btns, const char * title, const char * msg);
static int guiGetChoice(const int btnCnt, const int timeout);

//===========================================================================
// messagebox
//===========================================================================
int guiMBMD(const int btnCnt, const char * btns, const char * title, const char * msg){
        return guiMBX(mbTypeCustom, btnCnt, btns, title, msg, mbIcoNone, -1);
}

int guiMBNW(const int btnCnt, const char * btns, const char * title, const char * msg){
        return guiMBX(mbTypeCustom, btnCnt, btns, title, msg, mbIcoNone, 0);
}

int guiMBTO(const int btnCnt, const char * btns, const char * title, const char * msg, const int timeout){
        return guiMBX(mbTypeCustom, btnCnt, btns, title, msg, mbIcoNone, timeout);
}

int guiMBX(const int type, const int btnCnt, const char * btns, const char * title, const char * msg, const int icon, const int timeout){
        guiDrawMsgBox(btnCnt, btns, title, msg);
        return guiGetChoice(btnCnt, timeout);
}

int guiBox(int x, int y, int w, int h, int type, int fill)
{
        // Get the current hmi graphic context etc 
        hmiOutputConfig_t config; 
        if ( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) ) 
        { 
                GC* pGC = config.wgraphicContext; 
                Dimen dw = (Dimen)config.wwidth; 
                Dimen dh = (Dimen)config.wheight; 
                Canvas localCanvas; 
                Bitmap* pLocalBitmap = (Bitmap*)malloc(dw / 8 * dh); 
                if (pLocalBitmap) {
                        // Initialise the local structures 
                        // Call OpTIC to assign the bitmap to the canvas 
                        hmiOpDefineCanvas(&localCanvas, dw, dh, 2, // 2 colors 
                                pLocalBitmap); 
                        // Clear it 
                        hmiOpClearCanvas(&localCanvas, pGC); 

                        // drawing the frame and the title box
                        guiDrawBox(&localCanvas, pGC, x, y, w, h, type, fill);

                        // Copy the drawing to the screen bitmap 
                        hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                                0, // x
                                0, // y
                                128, //  
                                64); // 
                }
        }
        return RET_OK;
}

int guiBoxTitled(int x, int y, int w, int h, int type, int titlesize)
{
        // Get the current hmi graphic context etc 
        hmiOutputConfig_t config; 
        if ( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) ) 
        { 
                GC* pGC = config.wgraphicContext; 
                Dimen dw = (Dimen)config.wwidth; 
                Dimen dh = (Dimen)config.wheight; 
                Canvas localCanvas; 
                Bitmap* pLocalBitmap = (Bitmap*)malloc(dw / 8 * dh); 
                if (pLocalBitmap) {
                        // Initialise the local structures 
                        // Call OpTIC to assign the bitmap to the canvas 
                        hmiOpDefineCanvas(&localCanvas, dw, dh, 2, // 2 colors 
                                pLocalBitmap); 
                        // Clear it 
                        hmiOpClearCanvas(&localCanvas, pGC); 

                        // drawing the frame and the title box
                        guiDrawBox(&localCanvas, pGC, x, y, w, titlesize, type,1);
                        guiDrawBox(&localCanvas, pGC, x, y+titlesize-1, w, y+h-titlesize, type, 0);

                        // Copy the drawing to the screen bitmap 
                        hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                                0, // x
                                0, // y
                                128, //  
                                64); // 
                }
        }
        return RET_OK;
}

//===========================================================================
// progressbar
//===========================================================================

int guiPBPerct(const int style, const char * perct, const char * title, const char * msg)
{
        const minval = 5;
        const maxval = 100;
        const int showvalues = 1;
        int iPerc = atoi(perct);
        int iWidth = 1 + (int)((maxval-minval)*iPerc/100);

        // Get the current hmi graphic context etc 
        hmiOutputConfig_t config; 
        if ( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) ) 
        { 
                GC* pGC = config.wgraphicContext; 
                Dimen w = (Dimen)config.wwidth; 
                Dimen h = (Dimen)config.wheight; 
                Canvas localCanvas; 
                Bitmap* pLocalBitmap = (Bitmap*)malloc(w / 8 * h); 
                if (pLocalBitmap) {
                        // Initialise the local structures 
                        // Call OpTIC to assign the bitmap to the canvas 
                        hmiOpDefineCanvas(&localCanvas, w, h, 2, // 2 colors 
                                pLocalBitmap); 
                        // Clear it 
                        hmiOpClearCanvas(&localCanvas, pGC); 

                        // drawing the frame and the title box
                        guiDrawBoxTitled(&localCanvas, pGC, 1, 1, 126, 62,0,13);

                        // showing the progressbar
                        guiDrawBox(&localCanvas, pGC, minval, 48, iWidth, 13, 0, 1);
                        
                        // Copy the drawing to the screen bitmap 
                        hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                                0, // x
                                0, // y
                                128, //  
                                64); // 
                }

                // Display title (small chars, hungarian font, reverse mode)
                hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
                hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADCESSet(hHmi, HMI_CES_UTF8);
                hmiADDisplayText(hHmi, 3 | HMI_IN_PIXELS, 4 | HMI_IN_PIXELS, (char *)title);
                hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);

                // Display message
                hmiADFont(hHmi, HMI_INTERNAL_FONT_4); // Thin caraters
                hmiADDisplayText(hHmi, 24 | HMI_IN_PIXELS, 24 | HMI_IN_PIXELS, (char *)msg);

                if (showvalues)
                {
                        // display percentage captions 
                        hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
                        // show text on pos #1
                        hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, minval + iWidth + 1 | HMI_IN_PIXELS, (char *)perct);
                }
                // reset fg/bg colors
                hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);
        }
        return RET_OK;
}

int guiPBMinMax(const int style, const char * min, const char * max, const char * value, const char * title, const char * msg)
{
        const minval = 18;
        const maxval = 107;
        const int showvalues = 1;
        int iValue = atoi(value);
        int iMin  = atoi(min);
        int iMax  = atoi(max);
        int iPerc = (int)(100*(iValue-iMin)/(iMax-iMin));
        int iWidth = (int)(1 + (int)((maxval-minval)*iPerc/100));

        // Get the current hmi graphic context etc 
        hmiOutputConfig_t config; 
        if ( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) ) 
        { 
                GC* pGC = config.wgraphicContext; 
                Dimen w = (Dimen)config.wwidth; 
                Dimen h = (Dimen)config.wheight; 
                Canvas localCanvas; 
                Bitmap* pLocalBitmap = (Bitmap*)malloc(w / 8 * h); 
                if (pLocalBitmap) {
                        // Initialise the local structures 
                        // Call OpTIC to assign the bitmap to the canvas 
                        hmiOpDefineCanvas(&localCanvas, w, h, 2, // 2 colors 
                                pLocalBitmap); 
                        // Clear it 
                        hmiOpClearCanvas(&localCanvas, pGC); 

                        // drawing the frame and the title box
                        guiDrawBoxTitled(&localCanvas, pGC, 1, 1, 126, 62,0,13);

                        // showing the progressbar
                        guiDrawBox(&localCanvas, pGC, minval, 48, iWidth, 13, 0, 1);
                        
                        // Copy the drawing to the screen bitmap 
                        hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                                0, // x
                                0, // y
                                128, //  
                                64); // 
                }

                // Display title (small chars, hungarian font, reverse mode)
                hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
                hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADCESSet(hHmi, HMI_CES_UTF8);
                hmiADDisplayText(hHmi, 3 | HMI_IN_PIXELS, 4 | HMI_IN_PIXELS, (char *)title);
                hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);

                // Display message
                hmiADFont(hHmi, HMI_INTERNAL_FONT_4); // Thin caraters
                hmiADDisplayText(hHmi, 24 | HMI_IN_PIXELS, 24 | HMI_IN_PIXELS, (char *)msg);

                if (showvalues)
                {
                        // display percentage captions 
                        hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
                        hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, 5 | HMI_IN_PIXELS, (char *)min);
                        hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, maxval+1 | HMI_IN_PIXELS, (char *)max);
                        if (iWidth <30)
                        {
                                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, minval + iWidth + 2 | HMI_IN_PIXELS, (char *)value);
                        }
                        else
                        {
                                hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
                                hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
                                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, minval + 3 | HMI_IN_PIXELS, (char *)value);
                        }
                }
                // reset fg/bg colors
                hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);
        }
        return RET_OK;
}

int guiPBPure(const char * perct, const char * title, const char * msg)
{
        const minval = 5;
        const maxval = 123;
        const int showvalues = 0;
        int iPerc = atoi(perct);
        int iWidth = 1 + (int)((maxval-minval)*iPerc/100);

        // Get the current hmi graphic context etc 
        hmiOutputConfig_t config; 
        if ( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) ) 
        { 
                GC* pGC = config.wgraphicContext; 
                Dimen w = (Dimen)config.wwidth; 
                Dimen h = (Dimen)config.wheight; 
                Canvas localCanvas; 
                Bitmap* pLocalBitmap = (Bitmap*)malloc(w / 8 * h); 
                if (pLocalBitmap) {
                        // Initialise the local structures 
                        // Call OpTIC to assign the bitmap to the canvas 
                        hmiOpDefineCanvas(&localCanvas, w, h, 2, // 2 colors 
                                pLocalBitmap); 
                        // Clear it 
                        hmiOpClearCanvas(&localCanvas, pGC); 

                        // drawing the frame and the title box
                        guiDrawBoxTitled(&localCanvas, pGC, 1, 1, 126, 62,0,13);

                        // showing the progressbar
                        guiDrawBox(&localCanvas, pGC, minval, 48, iWidth, 13, 0, 1);
                        
                        // Copy the drawing to the screen bitmap 
                        hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                                0, // x
                                0, // y
                                128, //  
                                64); // 
                }

                // Display title (small chars, hungarian font, reverse mode)
                hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
                hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADCESSet(hHmi, HMI_CES_UTF8);
                hmiADDisplayText(hHmi, 3 | HMI_IN_PIXELS, 4 | HMI_IN_PIXELS, (char *)title);
                hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);

                // Display message
                hmiADFont(hHmi, HMI_INTERNAL_FONT_4); // Thin caraters
                hmiADDisplayText(hHmi, 24 | HMI_IN_PIXELS, 24 | HMI_IN_PIXELS, (char *)msg);

                if (showvalues)
                {
                        // display percentage captions 
                        hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
                        // show text on pos #1
                        hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, minval + iWidth + 1 | HMI_IN_PIXELS, (char *)perct);
                }
                // reset fg/bg colors
                hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
                hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);
        }
        return RET_OK;
}

//===========================================================================
// GUI - static members
//===========================================================================


//===========================================================================
// guiDrawBox (static)
//===========================================================================
// Parameters           : type == 0 - draw simple frame
//                                        type == 1 - draw frame without the edge 
//                                        (stg like small rounding)
//                                        fill==1 filled, fill==0 empty
//===========================================================================
static int guiDrawBox(Canvas* pCanv, GC* pGC, int x, int y, int w, int h, int type, int fill)                                                   
{
        if (fill==0)
        {
                if (type==0)
                {
                        hmiOpDrawLine(pCanv, pGC, x, y, x, y+h-1);              // left line
                        hmiOpDrawLine(pCanv, pGC, x+w-1, y, x+w-1, y+h-1);      // right line
                        hmiOpDrawLine(pCanv, pGC, x, y, x+w-1, y);              // top line
                        hmiOpDrawLine(pCanv, pGC, x, y+h-1, x+w-1, y+h-1);      // bottom line
                }
                else if (type == 1)
                {
                        hmiOpDrawLine(pCanv, pGC, x, y+1, x, y+h-2);                    // left line
                        hmiOpDrawLine(pCanv, pGC, x+w-1, y+1, x+w-1, y+h-2);    // right line
                        hmiOpDrawLine(pCanv, pGC, x+1, y, x+w-2, y);                    // top line
                        hmiOpDrawLine(pCanv, pGC, x+1, y+h-1, x+w-2, y+h-1);    // bottom line
                }
        }
        else // fill !=0
        {
                if (type==0)
                {
                        hmiOpDrawRect(pCanv, pGC, x, y, w, h);
                }
                else if (type==1)
                {
                        hmiOpDrawLine(pCanv, pGC, x, y+1, x, y+h-2);                    // left line
                        hmiOpDrawLine(pCanv, pGC, x+w-1, y+1, x+w-1, y+h-2);    // right line
                        hmiOpDrawLine(pCanv, pGC, x+1, y, x+w-2, y);                    // top line
                        hmiOpDrawLine(pCanv, pGC, x+1, y+h-1, x+w-2, y+h-1);    // bottom line
                        hmiOpDrawRect(pCanv, pGC, x+1, y+1, w-2, h-2);
                }
        }
        return RET_OK;
}

static int guiDrawBoxTitled(Canvas* pCanv, GC* pGC, int x, int y, int w, int h, int type, int titlesize)
{
        guiDrawBox(pCanv, pGC, x, y, w, titlesize, type,1);
        guiDrawBox(pCanv, pGC, x, y+titlesize-1, w, y+h-titlesize, type, 0);
        return RET_OK;
}

static int guiDrawMsgBox(const int btnCnt, const char * btns, const char * title, const char * msg)
{
        int             ch                      = ';' ;                 // separator
        char    *res;                                           // separator' position for btns
        char    *res1;                                          // separator' position for btns
        char    frag[20]        = "" ;                  // copy here btn's caption
        hmiOutputConfig_t config; 
        GC* pGC;
        Dimen w;
        Dimen h;
        Canvas localCanvas; 
        Bitmap* pLocalBitmap;

        // Get the current hmi graphic context etc 
        if (!( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) )) 
                return -mbErrOpenGui;

        pGC = config.wgraphicContext; 
        w = (Dimen)config.wwidth; 
        h = (Dimen)config.wheight; 
        pLocalBitmap = (Bitmap*)malloc(w / 8 * h); 

        // check bitmap allocation
        if (!pLocalBitmap)
                return -mbErrMallocBmp;

        // Initialise the local structures 
        // Call OpTIC to assign the bitmap to the canvas 
        hmiOpDefineCanvas(&localCanvas, w, h, 2, /* 2 colors */ pLocalBitmap); 
        // Clear it 
        hmiOpClearCanvas(&localCanvas, pGC); 

        // drawing the frame and the title box
        guiDrawBoxTitled(&localCanvas, pGC, 1, 1, 126, 62,0,13);

        // showing button(s)
        if (btnCnt == 1)
        {
                // show simple btn on pos #1
                guiDrawBox(&localCanvas, pGC, 44, 48, 40, 13, 1, 1);
        }
        else if (btnCnt == 2)
        {
                // show two btns on pos #1 and #2
                guiDrawBox(&localCanvas, pGC, 44, 48, 40, 13, 1, 1);
                guiDrawBox(&localCanvas, pGC, 85, 48, 40, 13, 1, 1);
        }
        else if (btnCnt == 3)
        {
                // show two btns on pos #0, #1 and #2
                guiDrawBox(&localCanvas, pGC,  3, 48, 40, 13, 1, 1);
                guiDrawBox(&localCanvas, pGC, 44, 48, 40, 13, 1, 1);
                guiDrawBox(&localCanvas, pGC, 85, 48, 40, 13, 1, 1);
        }
        
        // Copy the drawing to the screen bitmap 
        hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                0, // x
                0, // y
                128, //  
                64); // 
                

        // Display title (small chars, hungarian font, reverse mode)
        hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
        hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
        hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
        hmiADCESSet(hHmi, HMI_CES_UTF8);
        hmiADDisplayText(hHmi, 3 | HMI_IN_PIXELS, 4 | HMI_IN_PIXELS, (char *)title);
        hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
        hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);

        // Display message
        hmiADFont(hHmi, HMI_INTERNAL_FONT_4); // Thin caraters
        hmiADDisplayText(hHmi, 24 | HMI_IN_PIXELS, 10 | HMI_IN_PIXELS, (char *)msg);

        // display buttons' captions (reverse colors)
        hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
        hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
        hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
        if (btnCnt == 1)
        {
                // show text on pos #1
                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, 46 | HMI_IN_PIXELS, (char *)btns);
        }
        else if (btnCnt == 2)
        {
                // show texts on pos #1 and #2
                res = strchr(btns,ch);
                strncpy(frag,btns,res-btns); frag[res-btns] = 0;
                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, 49 | HMI_IN_PIXELS, frag);
                strncpy(frag,res+1,strlen(res+1)); frag[strlen(res+1)] = 0;
                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, 90 | HMI_IN_PIXELS, frag);
        }
        else if (btnCnt == 3)
        {
                // show texts on pos #0, #1 and #2
                res = strchr(btns,ch);
                strncpy(frag,btns,res-btns); frag[res-btns] = 0;
                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS,  6 | HMI_IN_PIXELS, frag);
                res1 = strchr(res+1,ch);
                strncpy(frag,res+1,res1-res-1); frag[res1-res-1] = 0;
                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, 49 | HMI_IN_PIXELS, frag);
                strncpy(frag,res1+1,strlen(res1+1)); frag[strlen(res1+1)] = 0;
                hmiADDisplayText(hHmi, 50 | HMI_IN_PIXELS, 90 | HMI_IN_PIXELS, frag);
        }
        // reset fg/bg colors
        hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
        hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);

        return RET_OK;
}

static int guiGetChoice(const int btnCnt, const int timeout)
{
        uint8   key;                                            // store the pressed key
        uint32  uTimeout;                                       // inner timeout in 10ms
        int             ret;

        if (timeout==0)
                return mbResTimeout;    // the return value doesn't matter

        // handle keyboard input

        if (timeout<0)                          // infinite timeout desired
        {
                kbdStart(1);
                while(1)
                {
            key= kbdKey(); //retrieve the key pressed if any
                        //hmiKeyWait (hHmi, &key, AMG_INFINITE);
                        //hmiBeep(hHmi,HMI_BEEP_CLICK, HMI_BEEP_LOW); 
                        switch(key)
                        {
                                // down - btn1
                                case HMI_KEY_F1 :
                                        if (btnCnt==3)
                                                return mbResBtn1;
                                        break;
                                // up - btn2
                                case HMI_KEY_F2 :
                                        if (btnCnt==3)
                                                return mbResBtn2;
                                        else
                                                return mbResBtn1;
                                        break;
                                // menu - btn3
                                case HMI_KEY_F3 :
                                        if (btnCnt==3)
                                                return mbResBtn3;
                                        else if (btnCnt==2)
                                                return mbResBtn2;
                                        break;
                        }       // switch(key)
                } // do
        } // if (timeout<0)
        else // wait "timeout" seconds
        {
                uTimeout = 100*timeout;
            tmrStart(1,uTimeout); 
        kbdStart(1);
        while(tmrGet(1)){
            key= kbdKey(); //retrieve the key pressed if any
                        switch(key)
                        {
                                // down - btn1
                                case HMI_KEY_F1 :
                                        if (btnCnt==3)
                                        {
                                                ret = mbResBtn1;
                                                goto lblEnd;
                                        }
                                        break;
                                // up - btn2
                                case HMI_KEY_F2 :
                                        if (btnCnt==3)
                                        {
                                                ret = mbResBtn2;
                                                goto lblEnd;
                                        }
                                        else
                                        {
                                                ret = mbResBtn1;
                                                goto lblEnd;
                                        }
                                        break;
                                // menu - btn3
                                case HMI_KEY_F3 :
                                        if (btnCnt==3)
                                        {
                                                return mbResBtn3;
                                                goto lblEnd;
                                        }
                                        else if (btnCnt==2)
                                        {
                                                return mbResBtn2;
                                                goto lblEnd;
                                        }
                                        break;
                                default:
                                        break;
                        }       // switch(key)
                        //key=0;
        } // while(tmrGet(0))
        }       // else, if(timeout<0)
        ret = mbResTimeout;     //
lblEnd:
    kbdStop();
    if(tmrGet(1)) Click();      //click sound
    tmrStop(1);
        return ret;
}

// ==========================================================================
// guiEnter components
// ==========================================================================
int guiEnterPIN(char *str){
        guiMBNW(0,"","Enter PIN","");
        return guiEnterNumBase(str, 4, '*');
}

int guiEnterPwd(char *str, byte len){
        guiMBNW(0,"","Enter Password:","");
        guiEnterNumBase(str, len, '*');
        return RET_OK;
}

int guiEnterAmt(char *amt, byte len, byte exp){
        guiMBNW(0,"","Enter Amount:","");
        guiEnterNumBase(amt, (const int)len, '\0');
        return RET_OK;
}

static int guiEnterNumBase(char * lpBuffer, const int maxLen, const char mask){
        bool bFlash = 0;                                        // 0 - space, 1 - reverse space
        uint8 nKey;                                                     // buffer to read and process a key
        uint8 nChars;                                           // text's length
        uint32 nPosX = 2;                                       // 3rd line, 3rd position (2,2)
        uint32 nPosY = 2;                                       // 
        uint32 nStartPosX = 2;                          // 
        uint32 nStartPosY = 2;                          // 
        uint32 nMaxPosX;                                        // 
        hmiOutputConfig_t hmiCfg;                       // 
        int bAcceptNumberOnly = 1;

        hmiADFont(hHmi, HMI_INTERNAL_FONT_6); // Normal

        if(hmiOutputConfigRead(hHmi, &hmiCfg)!=RET_OK)
                return -1;
        nStartPosX = nPosX; nStartPosY = nPosY;
        nMaxPosX = hmiCfg.wwidth / 8;
        nChars = strlen(lpBuffer);
        nPosX += nChars;
        hmiADGotoXY(hHmi, nStartPosX, nStartPosY);
        hmiADPutString(hHmi, lpBuffer);
        while(TRUE)
        {
                hmiADGotoXY(hHmi, nPosX, nPosY);
                
                // Display the cursor
                bFlash = TRUE - bFlash;
                if(bFlash)
                {
                        hmiOpSetForeground(hmiCfg.wgraphicContext, 0);
                        hmiOpSetBackground(hmiCfg.wgraphicContext, 1);
                        hmiADPutChar(hHmi, 0x20);
                        hmiOpSetForeground(hmiCfg.wgraphicContext, 1);
                        hmiOpSetBackground(hmiCfg.wgraphicContext, 0);
            } 
                else
                        hmiADPutChar(hHmi, 0x20);
        
                hmiADGotoXY(hHmi, nPosX, nPosY);
                if(hmiKeyWait(hHmi, &nKey, 53)==RET_OK)
                {
                        switch(nKey)
                        {
                        case HMI_KEY_ENTER:
                                hmiBeep(hHmi, HMI_BEEP_CLICK, HMI_BEEP_LOW);
                                lpBuffer[nChars] = 0;
                                return nChars;
                        case HMI_KEY_BCKSP:
                                if(nChars>0)
                                {
                                        hmiBeep(hHmi, HMI_BEEP_CLICK, HMI_BEEP_LOW);
                                        hmiADPutChar(hHmi, 0x20);
                                        if(nPosX==0)
                                        {
                                                nPosX = nMaxPosX - 1; nPosY--;
                                        } 
                                        else
                                                nPosX--;
                                        hmiADGotoXY(hHmi, nPosX, nPosY);
                                        nChars--;
                                }
                                break; // case HMI_KEY_BCKSP:
                        case HMI_KEY_CLEAR:
                                if(nChars>0)
                                {
                                        hmiBeep(hHmi, HMI_BEEP_CLICK, HMI_BEEP_LOW);
                                        hmiADPutChar(hHmi, 0x20);
                                        hmiADGotoXY(hHmi, nStartPosX, nStartPosY);
                                        while(nChars>0)
                                        {
                                                hmiADPutChar(hHmi, 0x20);
                                                nChars--;
                                        }
                                        nPosX = nStartPosX; nPosY = nStartPosY;
                                        hmiADGotoXY(hHmi, nStartPosX, nStartPosY);
                                }
                                break; // case HMI_KEY_CLEAR:
                        default:
                                if((nKey>0x2f && nKey<0x3a) && (nChars<maxLen))
                                {
                                        if(!bAcceptNumberOnly || !(nKey==0x30 && nChars==0))
                                        {
                                                hmiBeep(hHmi, HMI_BEEP_CLICK, HMI_BEEP_LOW);
                                                hmiADPutChar(hHmi, (mask)?mask:nKey);
                                                lpBuffer[nChars] = nKey;
                                                nChars++;
                                                nPosX++;
                                                if(nPosX>=nMaxPosX)
                                                {
                                                        nPosX = 0; nPosY++;
                                                }
                                        }
                                } // default: // if((nKey>0x2f && nKey<0x3a) && (nChars<maxLen))
                        } // switch
                } // if(hmiKeyWait(hHmi, &nKey, 53)==RET_OK)
        } // while (TRUE)
} // static guiEnterNumBase(...)
#endif

void tcps0001(void){
/*
        int ret;

#define title  "Communication in progress"
#define message  "  Please wait!"

                _dspStart();

        //==========================================================================
        // guiMBNW - "nowait" messagebox test with one/two/three buttons
        //==========================================================================
        // there is no result info, it just paints the msgbox
        // guiMBNW(...)
        // parameters   : 1 button, "OK", title, message
        ret = guiMBNW(1,"    OK","Nowait messagebox","It's just a picture");
        tmrPause(3);

        ret = guiMBNW(2,"    Yes;     No","Nowait messagebox","It's just a picture");
        tmrPause(3);

        ret = guiMBNW(3,"    Yes;     No; Cancel","Nowait messagebox","It's just a picture");
        tmrPause(3);

        //==========================================================================
        // guiMBTO - messagebox with timeout (10 secs now)
        //==========================================================================
        // ret = mbResBtn1..3, mbResTimeout, or error if negative 
        ret = guiMBTO(1,"    OK","10s timeout msgbox","It's a msgbox", 10);
        if (ret==mbResBtn1)
                guiMBTO(1,"Continue","Last choice info","Button#1",3);
        else if (ret==mbResTimeout)
                guiMBTO(1,"Continue","Last choice info","Timeout expired",3);
        else
                guiMBTO(1,"Continue","Last choice info","Error",3);

        ret = guiMBTO(2,"    Yes;     No","10s timeout msgbox","It's a msgbox", 10);
        if (ret==mbResBtn1)
                guiMBTO(1,"Continue","Last choice info","Button#1",3);
        else if (ret==mbResBtn2)
                guiMBTO(1,"Continue","Last choice info","Button#2",3);
        else if (ret==mbResTimeout)
                guiMBTO(1,"Continue","Last choice info","Timeout expired",3);
        else
                guiMBTO(1,"Continue","Last choice info","Error",3);

        ret = guiMBTO(3,"    Yes;     No; Cancel","10s timeout msgbox","It's a msgbox", 10);
        if (ret==mbResBtn1)
                guiMBTO(1,"Continue","Last choice info","Button#1",3);
        else if (ret==mbResBtn2)
                guiMBTO(1,"Continue","Last choice info","Button#2",3);
        else if (ret==mbResBtn3)
                guiMBTO(1,"Continue","Last choice info","Button#3",3);
        else if (ret==mbResTimeout)
                guiMBTO(1,"Continue","Last choice info","Timeout expired",3);
        else
                guiMBTO(1,"Continue","Last choice info","Error",3);

        //==========================================================================
        // guiMB - messagebox with infinite timeout
        //==========================================================================
        // ret = mbResBtn1..3, or error if negative 
        ret = guiMBMD(1,"    OK","Modem simulation","Simulation Finished");
        if (ret==mbResBtn1)
                guiMBTO(1,"Continue","Last choice info","Button#1",3);
        else if (ret==mbResTimeout)
                guiMBTO(1,"Continue","Last choice info","Timeout expired",3);
        else
                guiMBTO(1,"Continue","Last choice info","Error",3);

        ret = guiMBMD(2,"    Yes;     No","Modem simulation","Simulation Success?");
        if (ret==mbResBtn1)
                guiMBTO(1,"Continue","Last choice info","Button#1",3);
        else if (ret==mbResBtn2)
                guiMBTO(1,"Continue","Last choice info","Button#2",3);
        else if (ret==mbResTimeout)
                guiMBTO(1,"Continue","Last choice info","Timeout expired",3);
        else
                guiMBTO(1,"Continue","Last choice info","Error",3);

        ret = guiMBMD(3,"    Yes;     No; Cancel","Modem simulation","Simulation Success?");
        if (ret==mbResBtn1)
                guiMBTO(1,"Continue","Last choice info","Button#1",3);
        else if (ret==mbResBtn2)
                guiMBTO(1,"Continue","Last choice info","Button#2",3);
        else if (ret==mbResBtn3)
                guiMBTO(1,"Continue","Last choice info","Button#3",3);
        else if (ret==mbResTimeout)
                guiMBTO(1,"Continue","Last choice info","Timeout expired",3);
        else
                guiMBTO(1,"Continue","Last choice info","Error",3);


        dspClear();
    tmrPause(1);
        guiMBTO(1," Continue","Progressbar test","guiPBPerct #1", 3);
        dspClear();
        tmrPause(1);

        // progressbar test (percentage)
        guiPBPerct(0,"0%",title,message);
    tmrPause(1);
        guiPBPerct(0,"20%",title,message);
    tmrPause(1);
        guiPBPerct(0,"40%",title,message);
    tmrPause(1);
        guiPBPerct(0,"60%",title,message);
    tmrPause(1);
        guiPBPerct(0,"80%",title,message);
    tmrPause(1);
        guiPBPerct(0,"99%",title,message);
    tmrPause(1);
        guiPBPerct(0,"100%",title,message);
    tmrPause(3);

        dspClear();
    tmrPause(1);
        guiMBTO(1," Continue","Progressbar test","guiPBPerct #2", 3);
        dspClear();
        tmrPause(1);

        // progressbar test (percentage withot numbering)
        guiPBPure("0",title,message);
    tmrPause(1);
        guiPBPure("20",title,message);
    tmrPause(1);
        guiPBPure("40",title,message);
    tmrPause(1);
        guiPBPure("60",title,message);
    tmrPause(1);
        guiPBPure("80",title,message);
    tmrPause(1);
        guiPBPure("99",title,message);
    tmrPause(1);
        guiPBPure("100",title,message);
    tmrPause(3);

        dspClear();
    tmrPause(1);
        guiMBTO(1," Continue","Progressbar test","guiPBMinMax #1", 3);
        dspClear();
        tmrPause(1);

        // showing a progressbar from 10 up to 100
        guiPBMinMax(0,"10","200","10",title,message);
    tmrPause(1);
        guiPBMinMax(0,"10","200","40",title,message);
    tmrPause(1);
        guiPBMinMax(0,"10","200","80",title,message);
    tmrPause(1);
        guiPBMinMax(0,"10","200","120",title,message);
    tmrPause(1);
        guiPBMinMax(0,"10","200","160",title,message);
    tmrPause(1);
        guiPBMinMax(0,"10","200","200",title,message);
    tmrPause(3);


        dspClear();

        goto lblEnd;        
//lblKO:
//    trcErr(ret);
lblEnd:
//    tmrPause(3);
                _dspStop();
        return;
*/
}

void tcps0002(void){
/*
        char strToGet[100+1];

                _dspStart();
        dspClear();
        guiMBTO(1, " Continue", "Testing guiEnter...", "Enter PIN, Pwd, Amt", 5);
        
        strToGet[0] = 0;
        guiEnterPIN(strToGet);
        guiMBTO(1, "      OK", "Entered PIN:", strToGet, 5);

        strToGet[0] = 0;
        guiEnterPwd(strToGet, 5);
        guiMBTO(1, "      OK", "Entered Pwd:", strToGet, 5);

        strToGet[0] = 0;
        guiEnterAmt(strToGet, 10, 0);
        guiMBTO(1, "      OK", "Entered Amt:", strToGet, 5);

        // the following function guiEnterNumBase changed to static

        //strncpy(strToGet, "2332", 5); strToGet[4] = 0;
        //guiEnterNumBase(strToGet, 10, '\0');
        //guiMBTO(1, "      OK", "You entered:", strToGet, 5);

        //dspClear();

        //strToGet[0] = 0;
        //guiEnterNumBase(strToGet, 10, '*');
        //guiMBTO(1, "      OK", "You entered:", strToGet, 5);

                _dspStop();
        return;
*/
}

#endif
