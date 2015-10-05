/** \file tcik0106.c
 * UTA Unitary test case for bar code reader with GUI.
 * \sa
 *  - guiInputInit
 *  - guiInputDialog
 */
#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__
//static char str[10+1];
static byte volDba[128];
static tContainer cnt[10 + 1];
static tTable tab;
static tBox frm[10 + 1];
static char dgt[10][1 + 1];
static tText txt[10 + 1];
static tButton btn[10];
static tBox box;
static tBar bgd;
static tText dlg;
static tContainer fld;

static int CnvText(void) {      //text drawing
    int ret;
    byte hgt = 16;
    int x, y;
    int dx, dy;
    card wh;
    char *s = "Test case tcik0106";

    x = 0;
    y = hgt;
    dx = 0;
    dy = 0;

    wh = cnvTextSize(s, 2);
    ret =
        cnvBar((word) x, (word) y, (word) (x + HWORD(wh)),
               (word) (y - LWORD(wh) + 1), __FGD__);
    CHECK(ret >= 0, lblKO);
    ret = cnvText((word) x, (word) y, s, 2, __BGD__);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return 1;
}

static int buildKeyboard(tDrawing * drw) {
    int ret;
    byte idx;
    byte wdt = 12;
    byte hgt = 17;
    byte xTxt = 2;
    byte yTxt = hgt;
    byte dx = 16;
    byte dy = 20;
    word hor = dx * 5;
    word rx = (cnvW - hor) / 2;
    word ry = 4;

    static tShape shp[10 + 1];  //10 buttons + terminator
    static card rfp[10];

    VERIFY(drw);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < 10; idx++) {
        num2dec(dgt[idx], idx, 1);
        ret = drwBoxInit(&frm[idx], wdt, hgt, clrFgd);
        CHECK(ret >= 0, lblKO);
        ret = cntInit(&cnt[idx], 's', dgt[idx]);
        CHECK(ret >= 0, lblKO);
        ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, clrFgd);
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

    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int buildInput(tDrawing * drw) {
    int ret;
    byte idx;
    byte hgt = 17;

    static tShape shp[1 + 1 + 1 + 1];   //background + frame + text + terminator
    static card rfp[1 + 1 + 1];

    VERIFY(drw);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    idx = 0;
    ret = drwBarInit(&bgd, cnvW - 4, hgt, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBar, &bgd);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(2, cnvH - 2 - hgt);

    VERIFY(idx == 1);
    ret = drwBoxInit(&box, cnvW - 4, hgt, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBox, &box);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(2, cnvH - 2 - hgt);

    VERIFY(idx == 2);
    ret = cntInit(&fld, 't', &tab);
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&dlg, &fld, 0, 0, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpText, &dlg);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(2, cnvH - 2);

    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int buildDrawing(tDrawing * drw) {
    int ret;
    byte idx;
    static tDrawing kbd;
    static tDrawing inp;
    static tShape shp[1 + 1 + 1];   //keyboard + input field + terminator
    static card rfp[1 + 1];

    VERIFY(drw);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    idx = 0;
    ret = buildKeyboard(&kbd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpDrawing, &kbd);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(0, 0);

    VERIFY(idx == 1);
    ret = buildInput(&inp);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpDrawing, &inp);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(0, 0);

    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcik0106(void) {
    int ret;
    tDrawing drw;
    tInput inp;
    char buf[30 + 1];
    const char *ptr;
    char k;
    tBuffer tmp;
    byte dat[1024];

    nvmStart();
    memset(volDba, 0, 128);
    dbaSetPtr(volDba);

    ret = tabInit(&tab, 0xFF, 0, 30, 1, 0);
    memset(buf, '\0', sizeof(buf));

    bufInit(&tmp, dat, sizeof(dat));

    RUN(dspStart);
    RUN(cnvStart);
    RUN(prtStart);
#ifdef __BCR__
    RUN(bcrStart);
#endif
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = prtS("--tcik0106--");
    CHECK(ret >= 0, lblKO);

    ret = CnvText();
    CHECK(ret >= 0, lblKO);
    ret = buildDrawing(&drw);
    CHECK(ret >= 0, lblKO);
    ret = guiInputInit(&inp, &drw, 0, 0, "bkt", 0, 60);
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = prtS("Init ok");
    CHECK(ret >= 0, lblKO);

    //Call Wait before dialogue
    ret = guiInputWait(&inp, 0, &tmp);
    if(ret == 0) {
        ret = prtS("Timeout");
        CHECK(ret >= 0, lblKO);
        goto lblEnd;
    }
    ptr = (char *) bufPtr(&tmp);
    prtS(ptr);
    switch (ret) {
      case 'b':
          ret = prtS("Bar code read");
          CHECK(ret >= 0, lblKO);
          ret = prtS(ptr);
          CHECK(ret >= 0, lblKO);
          goto lblEnd;
      case 'k':
      case 'K':
          k = *ptr;
          prtS("key pressed");
          break;
      case 't':
      case 'T':
          prtS("screen touch");
          k = guiGetTftKey(&tmp);
          break;
      default:
          ret = -1;
          prtS("default");
          goto lblEnd;
    }
    switch (k) {
      case kbdVAL:
      case kbdANN:
      case kbdCOR:
      case kbdINI:
          goto lblEnd;
      default:
          ptr = &k;
          ret = cntPutStr(&fld, 0, ptr);
          CHECK(ret >= 0, lblKO);
          break;
    }
    prtS("call dialogue");
    ret = guiInputInit(&inp, &drw, 0, 0, "kt", 0, 60);
    CHECK(ret >= 0, lblKO);
    ret = guiInputDialog(&inp, &fld, 0, "", 0);
    CHECK(ret >= 0, lblKO);
    if(ret != 0) {
        ret = prtS("Dialog ok");
        CHECK(ret >= 0, lblKO);
        ret = tabGet(&tab, 0, buf, 30 + 1);
        CHECK(ret >= 0, lblKO);
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);
        ret = prtS("tabGet ok");
        CHECK(ret >= 0, lblKO);
    }
    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
#ifdef __BCR__
    bcrStop();
#endif
    prtS("__test end__");
    cnvStop();
    prtStop();
    dspStop();
}
#else
void tcik0106(void) {
}
#endif
