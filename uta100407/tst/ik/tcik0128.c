/// \file tcik0128.c
#include <stdio.h>
#include <string.h>             // for memset, strcpy, etc
#include <miltypes.h>
#include <unicapt.h>
#include "optic.h"
#include "ctx.h"

/** NON UTA Unitary test case for drag and drop.
 * \sa
 *  -
 */

static uint32 hPrt = 0;
static uint32 hKbd = 0;

static uint32 hmi_handle;
static hmiOutputConfig_t output_config;
static hmiWndPosition_t wpos;
static hmiTouchConfig_t tch_config;;

static hmiTouchSource_t tch_source;
static uint32 box_id;
static hmiTouchDrawBdConfig_t db_config;
static int16 BoardAlloc(void);
static int16 StartDrawingBoard(void);
static int16 MenuStopDrawingBoard(void);

typedef struct {
    int16 sta;
    uint16 len;
    char buf[1024];
} tResponse;

static int16 BoardAlloc(void);
static int16 StartDrawingBoard(void);
static int16 MenuStopDrawingBoard(void);

static int16 graphicOpen(void) {
    hmiBpp_t hmi_bpp;
    int a = 0;

    wpos.x = 0;
    wpos.y = 0;
    wpos.width = 240;
    wpos.height = 240;

    if(hmiOpenShared("MERCHANT", &hmi_handle,
                     HMI_CH_DISPLAY | HMI_CH_TOUCH, &wpos) != RET_OK) {
        if(a = hmiOpenShared("DEFAULT", &hmi_handle,
                             HMI_CH_DISPLAY | HMI_CH_TOUCH, &wpos) != RET_OK)
            return (FALSE);
    }

    hmi_bpp = HMI_BPP_8;

    if(hmiOutputConfigRead(hmi_handle, &output_config) != RET_OK) {
        hmiClose(hmi_handle);
        return (FALSE);
    }

    return (TRUE);
}

static int16 BoardAlloc(void) {
    uint32 x, y, width, height;
    int32 ret;

    x = 0;
    y = 0;
    width = height = 240;
    if((graphicOpen() == FALSE)
       || (hmiTouchConfigRead(hmi_handle, &tch_config) != RET_OK))
        return (FALSE);

    if((tch_config.isFingerAvailable == TRUE) &&
       (tch_config.isPenAvailable == TRUE))
        tch_source = HMI_TOUCH_SOURCE_PEN_FINGER;
    else if(tch_config.isFingerAvailable == TRUE)
        tch_source = HMI_TOUCH_SOURCE_FINGER;
    else if(tch_config.isPenAvailable == TRUE)
        tch_source = HMI_TOUCH_SOURCE_PEN;
    else {
        psyPeripheralResultWait(0, 200, 0);
        return (FALSE);
    }
    db_config.wndPosition.x = x;
    db_config.wndPosition.y = y;
    db_config.wndPosition.width = width;
    db_config.wndPosition.height = height;
    db_config.color = 1;
    db_config.maxBufSize = 0;
    db_config.autoStopTimeout = 0;
    db_config.filterLevel = 0;
    db_config.captureMode = HMI_TOUCH_CAPTURE_DOWN_UP;  //HMI_TOUCH_CAPTURE_NONE; //
    ret = hmiDrawingBoardAlloc(hmi_handle, &db_config, &box_id);
    if(ret != RET_OK)
        return (FALSE);
    return (TRUE);
}

static int16 StartDrawingBoard(void) {
    if(hmiGraphicPixelFill(hmi_handle, 0, 0, 240, 240, 0, TRUE) != RET_OK)
        return (FALSE);
    return (TRUE);
}

static int16 StopDrawingBoard(void) {
    uint32 wait_ret;
    hmiTouchDrawBdResult_t result;

    if(hmiDrawingBoardAbort(hmi_handle, box_id) != RET_OK)
        return (FALSE);
    else
        return (TRUE);

    while(1) {
        wait_ret = psyPeripheralResultWait(HMI_WAIT, 10, 0);

        if(wait_ret < 0)
            continue;
        if((wait_ret & 0xff000000) == PSY_WAIT_RUNNING)
            return (FALSE);
        if(PSY_PER_EXECUTED_OK(wait_ret, HMI_WAIT) == FALSE)
            continue;
        if(hmiResultGet(hmi_handle, sizeof(hmiTouchDrawBdResult_t),
                        (void *) &result) != RET_OK)
            continue;
        if((result.status == RET_OK) &&
           (result.eventType == HMI_TOUCH_EV_DB_END))
            return (TRUE);
    }
}

int tcik0128(void) {
    int32 ret;
    tResponse rsp;
    hmiTouchCoordResult_t tchResult;
    char buf[128];
    uint8 CupBuf[16000];
    uint32 pSize;
    unsigned long x_beg, y_beg, x_end, y_end, coord, NbCoord, Xf, Yf, Xmin,
        Ymin;

    ret = prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    if(ret < 0)
        goto lblKO;

    ret = prnPrint(hPrt, "--tcik0128--");
    if(ret < 0)
        goto lblKO;

    ret = hmiOpenShared("DEFAULT", &hKbd, HMI_CH_KEYBOARD, 0);
    if(ret < 0)
        goto lblKO;

    if((BoardAlloc() == FALSE) || (StartDrawingBoard() == FALSE))
        goto lblKO;

    hmiADClearLine(hmi_handle, HMI_ALL_LINES);
    hmiADDisplayText(hmi_handle, 0, 0, "Please touch the screen");

    ret = hmiKeyReq(hKbd);
    if(ret != RET_RUNNING)
        goto lblKO;

    ret = hmiDrawingBoardReq(hmi_handle, box_id, 0, 0, tch_source);
    if(ret != RET_RUNNING)
        goto lblKO;

    while(1) {
        ret = psyPeripheralResultWait(HMI_WAIT, PSY_INFINITE_TIMEOUT, 0);
        if(ret < 0)
            goto lblKO;

        if(PSY_PER_EXECUTED_OK(ret, HMI_WAIT)) {

            //Check Key board press
            ret = hmiResultGet(hKbd, sizeof(rsp), &rsp);
            switch (ret) {
              case RET_RUNNING:
                  ret = 0;
                  break;        //nothing is pressed until now
              case RET_OK:
                  ret = rsp.buf[0];
                  break;        //a key is pressed, return it
              default:
                  ret = '\xFF';
                  goto lblKO;   //an error encountered
            }
            if((ret == 0x3A) || (ret == 0x3B)) {    //press kbdANN or kbdVAL
                StopDrawingBoard();
                goto lblEnd;
            }
            //Check touch screen event
            ret =
                hmiResultGet(hmi_handle, sizeof(tchResult),
                             (void *) &tchResult);
            if(ret == RET_OK) {
                if((tchResult.status == RET_OK) &&
                   (tchResult.eventType == HMI_TOUCH_EV_DB_START)) {
                    hmiADDisplayText(hmi_handle, 1, 0, "Screen touched!");
                    psyTimerWakeAfter(100);
                    ret =
                        hmiDrawingBoardRetrieve(hmi_handle, box_id, CupBuf,
                                                sizeof(CupBuf),
                                                HMI_TOUCH_FORMAT_BINARY_2, 0,
                                                &pSize);
                    if(ret < 0)
                        goto lblKO;
                    NbCoord = (CupBuf[2] << 8) | CupBuf[3];
                    NbCoord = NbCoord & 0x3FF;
                    Xmin = ((CupBuf[4] << 8) | CupBuf[5]) & 0xFFF;
                    Ymin = ((CupBuf[6] << 8) | CupBuf[7]) & 0xFFF;
                    Xf = (CupBuf[8] << 8) | CupBuf[9];
                    Yf = (CupBuf[10] << 8) | CupBuf[11];
                    coord = (CupBuf[12] << 8) | (CupBuf[12 + 1]);
                    x_beg = (coord >> 7) & 0x1FF;
                    y_beg = coord & 0x7F;
                    x_beg =
                        (x_beg * (Xf >> 8)) + (x_beg * (Xf & 0xFF) / 255) +
                        Xmin;
                    y_beg =
                        (y_beg * (Yf >> 8)) + (y_beg * (Yf & 0xFF) / 255) +
                        Ymin;
                    x_beg = x_beg * 240 / 1500;
                    y_beg = y_beg * 240 / 1500;

                    coord =
                        (CupBuf[12 + 2 * (NbCoord - 1)] << 8) |
                        (CupBuf[12 + 1 + 2 * (NbCoord - 1)]);
                    x_end = (coord >> 7) & 0x1FF;
                    y_end = coord & 0x7F;
                    x_end =
                        (x_end * (Xf >> 8)) + (x_end * (Xf & 0xFF) / 255) +
                        Xmin;
                    y_end =
                        (y_end * (Yf >> 8)) + (y_end * (Yf & 0xFF) / 255) +
                        Ymin;
                    x_end = x_end * 240 / 1500;
                    y_end = y_end * 240 / 1500;

                    sprintf(buf, "x_beg=%u,y_beg=%u", x_beg, y_beg);
                    hmiADDisplayText(hmi_handle, 2, 0, (char *) buf);
                    sprintf(buf, "x_end=%u,y_end=%u", x_end, y_end);
                    hmiADDisplayText(hmi_handle, 3, 0, (char *) buf);
                    psyTimerWakeAfter(600);
                    if(StopDrawingBoard() == FALSE)
                        goto lblKO;
                    goto lblEnd;
                }
            }
        }
    }
    goto lblEnd;
  lblKO:
    prnPrint(hPrt, "tcik0128 fail");
  lblEnd:
    hmiCancel(hKbd);
    hmiClose(hKbd);
    hmiDrawingBoardDealloc(hmi_handle, box_id);
    hmiCancel(hmi_handle);
    hmiClose(hmi_handle);
    prnPrint(hPrt, "---- tcik0128 End ----");
    prnClose(hPrt);
}
