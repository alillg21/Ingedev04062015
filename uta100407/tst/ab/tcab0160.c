/** \file
 * Unitary test case tcab0160.
 * GUI testing:
 * \sa
 *  - guiInputInit()
 *  - guiInputWait()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0160.c $
 *
 * $Id: tcab0160.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__
static char str[10 + 1];
static int buildDrawing(tDrawing * drw) {
    int ret;
    byte idx;

    static tBox frm[11 + 1];
    static char dgt[10][1 + 1];
    static tContainer cnt[11 + 1];
    static tText txt[11 + 1];
    static tButton btn[10];

    byte wdt = 12;
    byte hgt = 17;
    byte xTxt = 2;
    byte yTxt = hgt;
    byte dx = 16;
    byte dy = 20;
    word hor = dx * 5;

    //word ver= dy*2;
    word rx = (cnvW - hor) / 2;
    word ry = 4;

    static tShape shp[10 + 1 + 1 + 1];  //10 buttons + frame + text + terminator
    static card rfp[10 + 1 + 1];

    VERIFY(drw);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < 10; idx++) {
        num2dec(dgt[idx], idx, 1);
        ret = drwBoxInit(&frm[idx], wdt, hgt, 1);
        CHECK(ret >= 0, lblKO);
        ret = cntInit(&cnt[idx], 's', dgt[idx]);
        CHECK(ret >= 0, lblKO);
        ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
        CHECK(ret >= 0, lblKO);
        ret =
            drwButtonInit(&btn[idx], &frm[idx], CARDHL(WORDHL(xTxt, yTxt), 0),
                          &txt[idx], 0, 0);
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp[idx], shpButton, &btn[idx]);
        CHECK(ret >= 0, lblKO);
        rfp[idx] = CARDHL(rx + (idx % 5) * dx, ry + (idx / 5) * dy);
        drwShapeKey(&shp[idx], (char) ('0' + idx));
    }

    idx = 10;
    ret = drwBoxInit(&frm[idx], cnvW - 4, hgt, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBox, &frm[idx]);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(2, cnvH - 2 - hgt);

    memset(str, 0, 10 + 1);
    ret = cntInit(&cnt[idx], 's', str);
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx + 1], shpText, &txt[idx]);
    CHECK(ret >= 0, lblKO);
    rfp[idx + 1] = CARDHL(2, cnvH - 2);

    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int poll(card idx) {
    trcFN("poll idx= %d\n", idx);
    return 1;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0160(void) {
    int ret;
    tDrawing drw;
    tInput inp;
    byte dat[256];
    tBuffer buf;
    const char *ptr;
    tVmtGui vmt;

    trcS("tcab0160 Beg\n");

    RUN(dspStart);
    RUN(cnvStart);
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = buildDrawing(&drw);
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);

    vmt.poll = poll;
    vmt.hdl = 0;
    ret = guiInputInit(&inp, &drw, 0, 0, "kT", &vmt, 60);
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 256);
    while(42) {
        ret = guiInputWait(&inp, 1, &buf);
        CHECK(ret >= 0, lblKO);
        if(!ret)
            break;
        ptr = (char *) bufPtr(&buf);
        if(ret == 'k') {
            if(*ptr == kbdANN)
                break;
        }
        switch (ret) {
          case 'k':
          case 'K':
              if(strlen(str) >= 10) {
                  *str = 0;
                  cnvClear();
              }
              strcat(str, ptr);
              break;
          default:             //touch screen is ignored
              break;
        }
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0160 End\n");
}
#else
void tcab0160(void) {
}
#endif
