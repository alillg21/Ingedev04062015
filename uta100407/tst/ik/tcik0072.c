/** \file
 * Unitary test case tcik0072.
 * Functions testing:
 * \sa
 *  - hmiGraphicLutGet()
 *  - hmiGraphicLutSet()
 */

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include <hmi.h>

#include "sys.h"
#include "tst.h"

static uint32 handle;

#ifdef __LUT__
static uint32 hPrt = 0;
#endif
static Canvas *cnv = 0;
static GC ctxOpt;               //graphic context

static int drawPalette(void) {
    int ret;
    OpticError err;
    word x, y, w, h;
    byte i, j;
    byte clr;

    w = cnvW / 16;
    h = cnvH / 16;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            x = (word) (j * w);
            y = (word) (i * h);
            clr = i * 16 + j;

            err = hmiOpSetForeground(&ctxOpt, clr);
            CHECK(err == OPTIC_NO_ERROR, lblKO);

            hmiOpSetRectType(&ctxOpt, RectTypeFilled);

            err = hmiOpDrawRect(cnv, &ctxOpt, x, y, w, h);
            CHECK(err == OPTIC_NO_ERROR, lblKO);

            ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
            CHECK(ret == RET_OK, lblKO);

            psyTimerWakeAfter(2);
        }
    }
    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:
    return 1;
}

void tcik0072(void) {

#ifdef __LUT__
    int ret, idx;
    OpticError err;

    Dimen wdt, hgt;
    byte pal, bpp, clr, fnt;
    word len, x, y;
    char *txt;
    static hmiLutType_t type;
    hmiLutGet_t lut_get_ptr;
    hmiLutSet_t lut_set_ptr;
    static byte bmp[cnvW * cnvH];

    ret = hmiOpen("DEFAULT", &handle);
    CHECK(ret == RET_OK, lblKO);
    ret = prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    CHECK(ret == RET_OK, lblKO);

    wdt = cnvW;
    hgt = cnvH;
    pal = 255;
    bpp = HMI_BPP_8;
    fnt = 0;
    x = 16;
    y = 32;
    clr = clrWHITE;
    type = HMI_LUT_TYPE_8_BPP;

    cnv = &_cnv;
    err = hmiOpDefineCanvas(cnv, wdt, hgt, pal, (Bitmap *) bmp);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    ret = hmiGraphicBppSet(handle, (hmiBpp_t) bpp);
    CHECK(ret == RET_OK, lblKO);

    hmiOpInitGC(&ctxOpt);

    err = hmiOpSetFont(&ctxOpt, fnt);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpSetBackground(&ctxOpt, clrBgd);
    CHECK(err == OPTIC_NO_ERROR, lblKO);
    err = hmiOpClearCanvas(cnv, &ctxOpt);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpSetForeground(&ctxOpt, clr);
    CHECK(err == OPTIC_NO_ERROR, lblKO);
    txt = "Begin palette test";
    len = strlen(txt);
    err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    ret = hmiGraphicLutGet(handle, type, &lut_get_ptr);
    CHECK(ret == RET_OK, lblKO);

    ret = drawPalette();
    CHECK(ret >= 0, lblKO);

    //ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
    //CHECK(ret == RET_OK, lblKO);

    psyTimerWakeAfter(100);
    /*
     * err = hmiOpClearCanvas(cnv, &ctxOpt);
     CHECK(err == OPTIC_NO_ERROR, lblKO);
     txt="Black and White palette";
     len = strlen(txt);
     err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
     CHECK(err == OPTIC_NO_ERROR, lblKO);
     */
    for (idx = 0; idx < 255; idx++) {
        lut_set_ptr.lut.lut8[idx].blue = (idx);
        lut_set_ptr.lut.lut8[idx].red = (idx);
        lut_set_ptr.lut.lut8[idx].green = (idx);
    }
#ifndef WIN32
    ret = hmiGraphicLutSet(handle, type, &lut_set_ptr);
    CHECK(ret == RET_OK, lblKO);
#endif

    //ret=drawPalette();
    //CHECK(ret >=0, lblKO);

    psyTimerWakeAfter(200);
    /*
       err = hmiOpClearCanvas(cnv, &ctxOpt);
       CHECK(err == OPTIC_NO_ERROR, lblKO);

       txt="Blue palette";
       len = strlen(txt);
       err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
       CHECK(err == OPTIC_NO_ERROR, lblKO);
     */
    ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
    CHECK(ret == RET_OK, lblKO);

    for (idx = 0; idx < 255; idx++) {
        lut_set_ptr.lut.lut8[idx].blue = (idx);
        lut_set_ptr.lut.lut8[idx].red = (0);
        lut_set_ptr.lut.lut8[idx].green = (0);
    }
#ifndef WIN32
    ret = hmiGraphicLutSet(handle, type, &lut_set_ptr);
    CHECK(ret == RET_OK, lblKO);
#endif

    //ret=drawPalette();
    //CHECK(ret >=0, lblKO);

    psyTimerWakeAfter(200);
    /*
       err = hmiOpClearCanvas(cnv, &ctxOpt);
       CHECK(err == OPTIC_NO_ERROR, lblKO);

       txt="Red palette";
       len = strlen(txt);
       err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
       CHECK(err == OPTIC_NO_ERROR, lblKO);
     */
    ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
    CHECK(ret == RET_OK, lblKO);

    for (idx = 0; idx < 256; idx++) {
        lut_set_ptr.lut.lut8[idx].blue = (0);
        lut_set_ptr.lut.lut8[idx].red = (idx);
        lut_set_ptr.lut.lut8[idx].green = (0);
    }
#ifndef WIN32
    ret = hmiGraphicLutSet(handle, type, &lut_set_ptr);
    CHECK(ret == RET_OK, lblKO);
#endif
    //ret=drawPalette();
    //CHECK(ret >=0, lblKO);

    psyTimerWakeAfter(200);
    /*
       err = hmiOpClearCanvas(cnv, &ctxOpt);
       CHECK(err == OPTIC_NO_ERROR, lblKO);

       txt="Green palette";
       len = strlen(txt);
       err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
       CHECK(err == OPTIC_NO_ERROR, lblKO);

       ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
       CHECK(ret == RET_OK, lblKO);
     */
    for (idx = 0; idx < 256; idx++) {
        lut_set_ptr.lut.lut8[idx].blue = (0);
        lut_set_ptr.lut.lut8[idx].red = (0);
        lut_set_ptr.lut.lut8[idx].green = (idx);
    }
#ifndef WIN32
    ret = hmiGraphicLutSet(handle, type, &lut_set_ptr);
    CHECK(ret == RET_OK, lblKO);
#endif
    //ret=drawPalette();
    //CHECK(ret >=0, lblKO);

    psyTimerWakeAfter(200);
    /*
       err = hmiOpClearCanvas(cnv, &ctxOpt);
       CHECK(err == OPTIC_NO_ERROR, lblKO);

       txt="Other palette";
       len = strlen(txt);
       err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
       CHECK(err == OPTIC_NO_ERROR, lblKO);

       ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
       CHECK(ret == RET_OK, lblKO);
     */
    for (idx = 0; idx < 256; idx++) {
        lut_set_ptr.lut.lut8[idx].blue = (idx);
        lut_set_ptr.lut.lut8[idx].red = (idx);
        lut_set_ptr.lut.lut8[idx].green = (0);
    }
#ifndef WIN32
    ret = hmiGraphicLutSet(handle, type, &lut_set_ptr);
    CHECK(ret == RET_OK, lblKO);
#endif

    //ret=drawPalette();
    //CHECK(ret >=0, lblKO);

    psyTimerWakeAfter(200);
    /*
       err = hmiOpClearCanvas(cnv, &ctxOpt);
       CHECK(err == OPTIC_NO_ERROR, lblKO);
     */
    //hmiOpDrawText(cnv,&ctxOpt,16,8 + 16,"Hello",5);
    txt = "End palette test";
    len = strlen(txt);
    err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    ret = hmiGraphicDataWrite(handle, cnv->bm, 0, 0, cnv->w, cnv->h);
    CHECK(ret == RET_OK, lblKO);

    psyTimerWakeAfter(200);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = prnPrint(hPrt, "KO!");
    CHECK(ret == RET_OK, lblKO);
    psyTimerWakeAfter(100);
  lblEnd:
    hmiClose(handle);
    prnClose(hPrt);
#endif
}
