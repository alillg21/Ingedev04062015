/// \file tcik0104.c
#include <stdio.h>
#include <string.h>             // for memset, strcpy, etc
#include <miltypes.h>
#include <unicapt.h>
#include "optic.h"
#include "ctx.h"

/** NON UTA Unitary test case for signature capture.
 * \sa
 *  -
 */

static uint32 hPrt = 0;
static uint32 hKbd = 0;

static uint32 hmi_handle;
static hmiOutputConfig_t output_config;
static hmiWndPosition_t wpos;
static hmiTouchConfig_t tch_config;

typedef struct {
    int16 sta;
    uint16 len;
    char buf[1024];
} tResponse;

static hmiTouchSource_t tch_source;
static uint32 box_id;
static hmiTouchDrawBdConfig_t db_config;
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
    if(tch_config.touchMode == HMI_TOUCH_MODE_SINGLE) {
        tch_config.touchMode == HMI_TOUCH_MODE_CONT;
        hmiTouchConfigWrite(hmi_handle, &tch_config);
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
    if((hmiGraphicPixelFill(hmi_handle,
                            0,
                            0,
                            240,
                            240,
                            0, TRUE) != RET_OK) ||
       (hmiDrawingBoardReq(hmi_handle,
                           box_id, 0, 0, tch_source) != RET_RUNNING))
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

/* Algorithme de Bresenham (testé pour x1,x2,y1,y2 > 0) */
static int trace_ligne_bresenham(int x1, int y1, int x2, int y2,
                                 char *image_buffer, uint32 image_width,
                                 uint32 image_height) {

    int e, x, y, dx, dy, incy = 0, incx = 0, i;
    uint32 coord;

    //char buf[256];

    dx = x2 - x1;
    dy = y2 - y1;

    if(dy < 0) {
        dy = -dy;
        incy = -1;
    } else if(dy > 0)
        incy = 1;
    else if(dy == 0)
        incy = 0;

    if(dx < 0) {
        dx = -dx;
        incx = -1;
    } else if(dx > 0)
        incx = 1;
    else if(dx == 0)
        incx = 0;

    if(dx > dy) {
        y = y1;
        e = -dx;
        x = x1;
        for (i = 0; i < dx; i++) {
            x = x + incx;
            y = y + incy;
            coord = (image_height - y - 1) * image_width + x;
            //sprintf(buf,"Xpoint x=%d,y=%d",x,y);
            //prnPrint(hPrt, buf);
            //sprintf(buf,"Xpoint incx=%d,incy=%d",incx,incy);
            //prnPrint(hPrt, buf);
            if(incy == 0)
                image_buffer[coord] = 0xFF; /* dessine le point */
            else {
                image_buffer[coord] = 0x01; /* dessine le point */
                incy = 0;
            }
            //x = x + incx;
        }                       /* for */
    } else {
        x = x1;
        e = -dy;
        y = y1;
        for (i = 0; i < dy; i++) {
            y = y + incy;
            x = x + incx;
            coord = (image_height - y - 1) * image_width + x;
            //sprintf(buf,"Ypoint x=%d,y=%d",x,y);
            //prnPrint(hPrt, buf);
            //sprintf(buf,"Ypoint incx=%d,incy=%d",incx,incy);
            //prnPrint(hPrt, buf);
            if(incx == 0)
                image_buffer[coord] = 0x01; /* dessine le point */
            else {
                image_buffer[coord] = 0xFF; /* dessine le point */
                incx = 0;
            }
            //y = y + incy;
        }                       /* for */
    }
    return 1;
}

static draw_raw4(uint8 * CupBuf, uint32 CupBufSize, char *image_buffer,
                 uint32 image_width, uint32 image_height) {
    uint32 i;
    int relier_au_precedent;
    unsigned short flags, flags_prec;
    unsigned long x, y, x_prec, y_prec;
    uint32 coord;
    char buf[24];

    memset(image_buffer, 0, image_width * image_height);
    x_prec = y_prec = 0;
    relier_au_precedent = 0;
    flags_prec = 0;

    for (i = 0; i < CupBufSize / 4; i++) {
        /* read the 4-byte */
        x = CupBuf[4 * i + 0] | ((CupBuf[4 * i + 1] & 0x0F) << 8);
        y = CupBuf[4 * i + 2] | ((CupBuf[4 * i + 3] & 0x0F) << 8);
        flags = (CupBuf[4 * i + 1] & 0x70) >> 4;
        /* Si I=1, on passe */
        if(flags & 1) {
            relier_au_precedent = 0;
            continue;
        }
        /* Si C=0 */
        if((flags & 4) == 0) {
            relier_au_precedent = 0;
        } else {
            if(flags_prec & 4)
                relier_au_precedent = 1;
        }

        x = x / 32;
        y = y / 8;

        /* Le cas échéant on dessine la ligne jusqu'au point précédent */
        if(relier_au_precedent) {
            if((image_height > y) && (image_width > x))
                trace_ligne_bresenham(x, y, x_prec, y_prec, image_buffer,
                                      image_width, image_height);
        } else {
            /* Affiche le point */
            if((image_height > y) && (image_width > x)) {
                sprintf(buf, "general x=%d,y=%d", x, y);
                prnPrint(hPrt, buf);
                coord = (image_height - y - 1) * image_width + x;
                image_buffer[coord] = 0x01;
            }
        }
        x_prec = x;
        y_prec = y;
        flags_prec = flags;
    }                           /* for */
    return 0;
}

static draw_B2(uint8 * CupBuf, char *image_buffer, uint32 image_width,
               uint32 image_height) {
    int32 ret;
    uint16 nbCoord, i, Xmin, Ymin, Xf, Yf;
    uint32 coord;
    uint32 x, y, x_prec, y_prec;
    int relier_au_precedent;

    if(CupBuf) {
        nbCoord =
            ((uint16) ((((uint16) CupBuf[2]) << 8) | ((uint16) CupBuf[3])));
        nbCoord = nbCoord & 0x3FF;  /* [1..1023] */
        Xmin = ((uint16) ((((uint16) CupBuf[4]) << 8) | ((uint16) CupBuf[5])));
        Xmin = Xmin & 0xFFF;
        Ymin = ((uint16) ((((uint16) CupBuf[6]) << 8) | ((uint16) CupBuf[7])));
        Ymin = Ymin & 0xFFF;
        Xf = ((uint16) ((((uint16) CupBuf[8]) << 8) | ((uint16) CupBuf[9])));
        Yf = ((uint16) ((((uint16) CupBuf[10]) << 8) | ((uint16) CupBuf[11])));
        i = 12;
        relier_au_precedent = 0;
        x_prec = y_prec = 0;
        for (i = 0; i < nbCoord; i++) {
            coord = (CupBuf[12 + 2 * i] << 8) | (CupBuf[12 + 1 + 2 * i]);
            x = (coord >> 7) & 0x1FF;
            i++;
            y = coord & 0x7F;
            i++;
            if((x == 0) && (y == 0)) {
                relier_au_precedent = 0;
                continue;
            }
            x = (x * (Xf >> 8)) + (x * (Xf & 0xFF) / 255) + Xmin;
            y = (y * (Yf >> 8)) + (y * (Yf & 0xFF) / 255) + Ymin;
            x = x / 32;
            y = y / 8;
            if((image_width > x) && (image_height > y)) {
                coord = (image_height - y - 1) * image_width + x;
                image_buffer[coord] = 15;
            }
            /* Le cas échéant on dessine la ligne jusqu'au point précédent */
            if(relier_au_precedent) {
                if((image_width > x) && (image_height > y))
                    ret =
                        trace_ligne_bresenham(x, y, x_prec, y_prec,
                                              image_buffer, image_width,
                                              image_height);
            }
            if(i != 0)
                relier_au_precedent = 1;
            x_prec = x;
            y_prec = y;
        }
    }
}

int tcik0104(void) {
    uint8 CupBuf[16000];
    uint32 pSize;
    int32 ret;
    tResponse rsp;
    const uint32 image_width = 80, image_height = 174;
    char image_buffer[(image_width * image_height)];

    ret = prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    if(ret < 0)
        return;

    ret = prnPrint(hPrt, "--tcik0104--");
    if(ret < 0)
        return;

    ret = hmiOpenShared("DEFAULT", &hKbd, HMI_CH_KEYBOARD, 0);
    if(ret < 0)
        goto lblKO;

    memset(image_buffer, 0, (image_width * image_height));
    memset(CupBuf, 0, 6144);
    if((BoardAlloc() == FALSE) || (StartDrawingBoard() == FALSE))
        goto lblKO;

    hmiADClearLine(hmi_handle, HMI_ALL_LINES);
    hmiADDisplayText(hmi_handle, 1, 0, "Please signe");

    ret = hmiKeyReq(hKbd);
    if(ret != RET_RUNNING)
        goto lblKO;

    while(1) {
        //Check Key board press
        ret = hmiResultGet(hKbd, sizeof(rsp), &rsp);
        switch (ret) {
          case RET_RUNNING:
              ret = 0;
              break;            //nothing is pressed until now
          case RET_OK:
              ret = rsp.buf[0];
              break;            //a key is pressed, return it
          default:
              ret = '\xFF';
              goto lblKO;       //an error encountered
        }
        if(ret == 0x3A) {       //Press kbdVAL
            ret = hmiResultGet(hmi_handle, sizeof(CupBuf), CupBuf);
            if(ret < 0)
                goto lblKO;
            prnPrint(hPrt, "Retrieve Signture");
            ret =
                hmiDrawingBoardRetrieve(hmi_handle, box_id, CupBuf,
                                        sizeof(CupBuf), HMI_TOUCH_FORMAT_RAW_4,
                                        0, &pSize);
            if(ret < 0)
                goto lblKO;
            prnPrint(hPrt, "Retrieve ok");
            prnPrint(hPrt, "Fill image_buffer");
            draw_raw4(CupBuf, pSize, image_buffer, image_width, image_height);
            prnImagePrint(hPrt, (uint8 *) image_buffer, image_height,
                          image_width, 0, PRN_AT_LEFT | PRN_MULT1);
            memset(image_buffer, 0, (image_width * image_height));
            if((StopDrawingBoard() == FALSE) || (StartDrawingBoard() == FALSE))
                goto lblKO;
        }
        if(ret == 0x3B)         //press kbdANN
        {
            StopDrawingBoard();
            goto lblEnd;
        }
    }
    goto lblEnd;
  lblKO:
    prnPrint(hPrt, "tcik0104 fail");
  lblEnd:
    hmiCancel(hKbd);
    hmiClose(hKbd);
    hmiDrawingBoardDealloc(hmi_handle, box_id);
    hmiCancel(hmi_handle);
    hmiClose(hmi_handle);
    prnPrint(hPrt, "---- tcik0104 End ----");
    prnClose(hPrt);
}
