#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__
//Unitary test case tcab0161.
//GUI testing:
// - guiInputDialog

byte volDba[128];
static tTable tab;
static tBox frm[10 + 1];
static char dgt[10][1 + 1];
static tContainer cnt[10 + 1];
static tText txt[10 + 1];
static tButton btn[10];
static tBox box;
static tBar bgd;
static tContainer fld;
static tText dlg;

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

static int buildDlg(tDrawing * drw) {
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

static int hdl(char mod, tBuffer * buf) {   //demonstrate virtual method processing
    char *ptr;

    trcFN("hdl mod= %d\n", mod);
    VERIFY(buf);
    switch (mod) {
      case 'k':
      case 'K':
          break;
      default:                 //no special processing: input from touch screen is processed "as is"
          return mod;

    }
    ptr = (char *) bufPtr(buf);
    switch (*ptr) {
      case '0':
      case '2':
      case '4':
      case '6':
      case '8':                //convert it to letters
          *ptr += 'A' - '0';
          break;
      case '1':                //reject it
          bufReset(buf);
          break;
      default:
          break;

    }
    return mod;
}

static int buildInp(tInput * inp, tDrawing * drw) {
    int ret;
    byte idx;
    static tDrawing kbd;
    static tDrawing dlg;
    static tShape shp[1 + 1 + 1];   //keyboard + input field + terminator
    static card rfp[1 + 1];
    static tVmtGui vmt;

    VERIFY(inp);
    VERIFY(drw);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));
    memset(&vmt, 0, sizeof(vmt));
    vmt.hdl = hdl;

    idx = 0;
    ret = buildKeyboard(&kbd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpDrawing, &kbd);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(0, 0);

    VERIFY(idx == 1);
    ret = buildDlg(&dlg);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpDrawing, &dlg);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(0, 0);

    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);

    ret = guiInputInit(inp, drw, 0, 0, "KT", &vmt, 60);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0161(void) {
    int ret;
    tDrawing drw;
    tInput inp;
    char buf[30 + 1];

    nvmStart();
    memset(volDba, 0, 128);
    dbaSetPtr(volDba);

    ret = tabInit(&tab, 0xFF, 0, 30, 1);

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = buildInp(&inp, &drw);
    CHECK(ret >= 0, lblKO);
    ret = guiInputDialog(&inp, &fld, 0, "", 0);
    CHECK(ret >= 0, lblKO);
    ret = tabGet(&tab, 0, buf, 30 + 1);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    prtStop();
    dspStop();
}
#else
void tcab0161(void) {
}
#endif
