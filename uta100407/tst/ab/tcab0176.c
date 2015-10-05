/** \file
 * Unitary test case tcab0176.
 * GUI testing:
 * \sa
 *  - guiInputSelect()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0176.c $
 *
 * $Id: tcab0176.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__

static Pchar str[] = {
    "BMW",
    "PEUGEOUT",
    "RENAULT",
    "FIAT",
    "NISSAN",
    "LADA",
    "TATA",
    "CITROEN",
    "SEAT",
    "FERRARI",
    "LAMBORGHINI",
    "TOYOTA",
    "MERCEDES",
    "CHRYSLER",
    "HONDA",
    "SUZUKI",
    "MAZDA",
    "SKODA",
    "VOLKSWAGEN",
    0
};

static int test1(void) {        //selector only, without gadgets
    int ret;
    const char *ptr;
    tBar bgd;
    tBox foc;
    tSelector sel;
    tShape shp[1 + 1];
    card rfp[1];
    tDrawing drw;
    tInput inp;

    byte wdt, hgt, rowHgt, dim;
    word clrSelBgd = clrBgd;
    word clrSelFoc = clrFgd;

    wdt = (byte) ((cnvW / dspW - 2) * dspW);
    hgt = (cnvH / dspH - 1) * dspH;
    rowHgt = (cnvH / dspH);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    dim = 0;
    while(str[dim])
        dim++;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&bgd, wdt, hgt, clrSelBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBoxInit(&foc, wdt, rowHgt, clrSelFoc);
    CHECK(ret >= 0, lblKO);
    ret = drwSelectorInit(&sel, &bgd, &foc, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[0], shpSelector, &sel);
    CHECK(ret >= 0, lblKO);
    rfp[0] = CARDHL(4, 4);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = guiInputInit(&inp, &drw, 0, 0, "KT", 0, 60);
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = guiInputSelect(&inp, str, WORDHL(0, 0));
    CHECK(ret >= 0, lblKO);
    if(HBYTE(ret) >= dim)
        ptr = "Aborted";
    else
        ptr = str[LBYTE(ret)];
    ret = prtS(ptr);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

static const byte arrUp[] = {
    0x00,                       //........
    0x18,                       //...**...
    0x3C,                       //..****..
    0x7E,                       //.******.
    0xFF,                       //********
    0x7E,                       //.******.
    0x7E,                       //.******.
    0x00,                       //........
};
static const byte arrDn[] = {
    0x00,                       //........
    0x7E,                       //.******.
    0x7E,                       //.******.
    0xFF,                       //********
    0x7E,                       //.******.
    0x3C,                       //..****..
    0x18,                       //...**...
    0x00,                       //........
};

static int test2(void) {        //selector with buttons
    int ret;
    const char *ptr;
    tBar bgd;
    tBox foc;
    tSelector sel;

    tBox frm;
    tTile icoUp, icoDn;
    tButton btnUp, btnDn;

    tShape shp[3 + 1];
    card rfp[3];
    tDrawing drw;
    tInput inp;

    byte wdt, hgt, rowHgt, dim, idx;
    byte clrSelBgd = clrBgd;
    word clrSelFoc = clrFgd;

    byte xIco = 2;
    byte yIco = 2;
    byte wdtBtn = xIco + 8 + xIco;
    byte hgtBtn = yIco + 8 + yIco;

    wdt = (byte) ((cnvW / dspW - 2) * dspW);
    hgt = (cnvH / dspH - 1) * dspH;
    rowHgt = (cnvH / dspH);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    dim = 0;
    while(str[dim])
        dim++;

    ret = drwBoxInit(&frm, wdtBtn, hgtBtn, clrFgd);
    CHECK(ret >= 0, lblKO);

    idx = 0;
    ret = drwBarInit(&bgd, wdt, hgt, clrSelBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBoxInit(&foc, wdt, rowHgt, clrSelFoc);
    CHECK(ret >= 0, lblKO);
    ret = drwSelectorInit(&sel, &bgd, &foc, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpSelector, &sel);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(4, 4);

    idx++;
    ret = drwTileInit(&icoUp, arrUp, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnUp, &frm, CARDHL(0, WORDHL(xIco, yIco)), 0, &icoUp,
                      0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpButton, &btnUp);
    CHECK(ret >= 0, lblKO);
    drwShapeKey(&shp[idx], kbdJLF);
    rfp[idx] = CARDHL(4 + wdt + 2, 4);

    idx++;
    ret = drwTileInit(&icoDn, arrDn, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnDn, &frm, CARDHL(0, WORDHL(xIco, yIco)), 0, &icoDn,
                      0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpButton, &btnDn);
    CHECK(ret >= 0, lblKO);
    drwShapeKey(&shp[idx], kbdJRG);
    rfp[idx] = CARDHL(4 + wdt + 2, 4 + hgtBtn + 2);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = guiInputInit(&inp, &drw, 0, 0, "KT", 0, 60);
    CHECK(ret >= 0, lblKO);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = guiInputSelect(&inp, str, WORDHL(0, 0));
    CHECK(ret >= 0, lblKO);
    if(HBYTE(ret) >= dim)
        ptr = "Aborted";
    else
        ptr = str[LBYTE(ret)];
    ret = prtS(ptr);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

static int test3(void) {        //selector with slider
    int ret;
    const char *ptr;
    tBar bgd;
    tBox foc;
    tBar beg, end;
    tSlider sld;
    tSelector sel;
    tShape shp[2 + 1];
    card rfp[2];
    tDrawing drw;
    tInput inp;

    byte wdt, hgt, rowHgt, dim, idx;
    word clrSelBgd = clrBgd;
    word clrSelFoc = clrFgd;

    wdt = (byte) ((cnvW / dspW - 2) * dspW);
    hgt = (byte) ((cnvH / dspH - 1) * dspH);
    rowHgt = (cnvH / dspH);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    dim = 0;
    while(str[dim])
        dim++;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&beg, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&end, 8, 8, clrBgd);
    CHECK(ret >= 0, lblKO);

    ret = drwBarInit(&bgd, wdt, hgt, clrSelBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBoxInit(&foc, wdt, rowHgt, clrSelFoc);
    CHECK(ret >= 0, lblKO);

    idx = 0;
    ret = drwSelectorInit(&sel, &bgd, &foc, &sld, 0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpSelector, &sel);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(2, 2);

    idx++;
    ret = drwSliderInit(&sld, &beg, &end, 0, hgt - 8);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpSlider, &sld);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(2 + wdt + 2, 2);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = guiInputInit(&inp, &drw, 0, 0, "KT", 0, 60);
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = guiInputSelect(&inp, str, WORDHL(0, 0));
    CHECK(ret >= 0, lblKO);
    if(HBYTE(ret) >= dim)
        ptr = "Aborted";
    else
        ptr = str[LBYTE(ret)];
    ret = prtS(ptr);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

static int test4(void) {        //small number of items
    int ret;
    const char *ptr;
    tBar bgd;
    tBox foc;
    tSelector sel;
    tShape shp[1 + 1];
    card rfp[1];
    tDrawing drw;
    tInput inp;

    byte wdt, hgt, rowHgt, dim;
    word clrSelBgd = clrBgd;
    word clrSelFoc = clrFgd;

    Pchar itm[] = {
        "SENO",
        "SOLOMA",
        0
    };

    wdt = (byte) ((cnvW / dspW - 2) * dspW);
    hgt = (cnvH / dspH - 1) * dspH;
    rowHgt = (cnvH / dspH);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    dim = 0;
    while(itm[dim])
        dim++;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&bgd, wdt, hgt, clrSelBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBoxInit(&foc, wdt, rowHgt, clrSelFoc);
    CHECK(ret >= 0, lblKO);
    ret = drwSelectorInit(&sel, &bgd, &foc, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[0], shpSelector, &sel);
    CHECK(ret >= 0, lblKO);
    rfp[0] = CARDHL(4, 4);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = guiInputInit(&inp, &drw, 0, 0, "KT", 0, 60);
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = guiInputSelect(&inp, itm, WORDHL(0, 0));
    CHECK(ret >= 0, lblKO);
    if(HBYTE(ret) >= dim)
        ptr = "Aborted";
    else
        ptr = itm[LBYTE(ret)];
    ret = prtS(ptr);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0176(void) {
    int ret;

    trcS("tcab0176 Beg\n");

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(test1);
    RUN(test2);
    RUN(test3);
    RUN(test4);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    tmrPause(1);
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0176 End\n");
}
#else
void tcab0176(void) {
}
#endif
