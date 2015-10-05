/** \file
 * Unitary test case tcab0127.
 * Simple drawing
 * \sa
 *  - drwPixelInit()
 *  - drwShapeInit()
 *  - drwLineInit()
 *  - drwBarInit()
 *  - drwTextInit()
 *  - drwTileInit()
 *  - drwDrawingInit()
 *  - drwDrawingDraw()
 *  - drwIdx()
 *  - drwBoxInit()
 *  - drwCircleInit()
 *  - drwDiscInit()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0127.c $
 *
 * $Id: tcab0127.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

#ifdef __CANVAS__
static void dly(void) {
    tmrStart(0, 1);
    while(tmrGet(0));
    tmrStop(0);
    dspLight(100);
}

static int testBasicShapes(void) {  //basic shapes drawing
    int ret;
    enum {
        nPix = 9,
        nLin = 4,
        nBar = 2,
        nTxt = 2,
        nTil = 1,
        nDim = nPix + nLin + nBar + nTxt + nTil
    };
    tDrawing drw;
    tPixel pix[nPix];
    tLine lin[nLin];
    tBar bar[nBar];
    tContainer cnt[2];
    tText txt[nTxt];
    tTile bmp[nTil];
    tShape shp[nDim + 1];       //one more sentinel shape (zero pointer)
    card rfp[nDim];
    byte idx, tmp, idx1, idx2;
    word x, y;

#define __cardVisaSmall__
#include "logo.fnt"

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < nPix; idx++) {
        //initialize pixel shapes
        ret = drwPixelInit(&pix[idx], (byte) ((idx == 4) ? clrFgd : clrBgd));
        CHECK(ret >= 0, lblKO);

        //put it into drawing array
        ret = drwShapeInit(&shp[idx], shpPixel, &pix[idx]);
        CHECK(ret >= 0, lblKO);

        //calculate reference points for pixel shapes
        x = cnvW / 2 + 10;
        y = cnvH / 2;
        switch (idx) {
          case 0:
              rfp[idx] = CARDHL(x - 1, y - 1);
              break;
          case 1:
              rfp[idx] = CARDHL(x, y - 1);
              break;
          case 2:
              rfp[idx] = CARDHL(x + 1, y - 1);
              break;
          case 3:
              rfp[idx] = CARDHL(x - 1, y);
              break;
          case 4:
              rfp[idx] = CARDHL(x, y);
              break;
          case 5:
              rfp[idx] = CARDHL(x + 1, y);
              break;
          case 6:
              rfp[idx] = CARDHL(x - 1, y + 1);
              break;
          case 7:
              rfp[idx] = CARDHL(x, y + 1);
              break;
          case 8:
              rfp[idx] = CARDHL(x + 1, y + 1);
              break;
          default:
              break;
        }
    }

    //initialize line shapes
    ret = drwLineInit(&lin[0], cnvW - 3, 0, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwLineInit(&lin[1], 0, cnvH - 3, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwLineInit(&lin[2], cnvW - 3, 0, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwLineInit(&lin[3], 0, cnvH - 3, clrFgd);
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < nLin; idx++) {
        //put it into drawing array
        ret = drwShapeInit(&shp[nPix + idx], shpLine, &lin[idx]);
        CHECK(ret >= 0, lblKO);
    }
    //calculate reference points for line shapes
    rfp[nPix + 0] = CARDHL(1, 1);
    rfp[nPix + 1] = CARDHL(1, 1);
    rfp[nPix + 2] = CARDHL(1, cnvH - 2);
    rfp[nPix + 3] = CARDHL(cnvW - 2, 1);

    //prepare bar shapes
    idx = 0;
    ret = drwBarInit(&bar[idx], cnvW - 6, cnvH - 6, clrFgd);
    CHECK(ret >= 0, lblKO);     //initialize bar shape
    ret = drwShapeInit(&shp[nPix + nLin + idx], shpBar, &bar[idx]); //put it into drawing array
    rfp[nPix + nLin + idx] = CARDHL(3, 3);  //calculate reference point

    idx = 1;
    ret = drwBarInit(&bar[idx], 60, 40, clrBgd);
    CHECK(ret >= 0, lblKO);     //initialize bar shape
    ret = drwShapeInit(&shp[nPix + nLin + idx], shpBar, &bar[idx]); //put it into drawing array
    rfp[nPix + nLin + idx] = CARDHL(4, 4);  //calculate reference point

    //prepare text shapes
    idx = 0;
    ret = cntInit(&cnt[idx], 's', "Pixels covered");
    CHECK(ret >= 0, lblKO);     //the message is showed when pixels are covered by bar
    ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, clrBgd);
    CHECK(ret >= 0, lblKO);     //initialize text shape
    idx1 = nPix + nLin + nBar + idx;
    ret = drwShapeInit(&shp[idx1], shpText, &txt[idx]); //put it into drawing array
    rfp[idx1] = CARDHL(2, cnvH - 2);    //calculate reference point

    idx = 1;
    ret = cntInit(&cnt[idx], 's', "Pixels visible");
    CHECK(ret >= 0, lblKO);     //the message is showed when pixels are not covered by bar
    ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, clrBgd);
    CHECK(ret >= 0, lblKO);     //initialize text shape
    idx2 = nPix + nLin + nBar + idx;
    ret = drwShapeInit(&shp[idx2], shpText, &txt[idx]); //put it into drawing array
    rfp[idx2] = CARDHL(2, cnvH - 2);    //calculate reference point

    ret = drwTileInit(&bmp[0], cardVisaSmall, 57, 35, clrFgd);
    CHECK(ret >= 0, lblKO);     //initialize tile shape
    for (idx = 0; idx < nTil; idx++) {
        ret = drwShapeInit(&shp[nPix + nLin + nBar + nTxt + idx], shpTile, &bmp[idx]);  //put it into drawing array
        CHECK(ret >= 0, lblKO);
        rfp[nPix + nLin + nBar + nTxt + idx] = CARDHL(4, 4);    //calculate reference point
    }

    ret = drwDrawingInit(&drw, shp, rfp);   //initialize drawing scene
    CHECK(ret >= 0, lblKO);

    drwHide(&drw, idx2);        //hide second message

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);   //pix shapes are covered by bar that is displayed later
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    tmrPause(2);

    idx = drwIdx(&drw, &txt[0]);    //find where the first text message is located in drawing array
    VERIFY(idx == idx1);
    idx = drwIdx(&drw, &txt[1]);    //find where the second text message is located in drawing array
    VERIFY(idx == idx2);

    for (idx = 0; idx < nPix; idx++) {  //push pix shapes on the top of the drawing
        tmp = drwIdx(&drw, &pix[idx]);
        VERIFY(tmp < drwDim(&drw));
        drwPush(&drw, tmp, (byte) (drwDim(&drw) - 1));  //push it to the end of array; it will be drawn last on the top
    }

    idx = drwIdx(&drw, &txt[0]);
    drwHide(&drw, idx);

    idx = drwIdx(&drw, &txt[1]);
    drwShow(&drw, idx);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(3);
    return ret;
}

static int testBoxShapes(void) {    //box shapes
    int ret;
    enum {
        nBox = cnvH / 4
    };
    tBox box[nBox];
    tShape shp[nBox + 1];
    card rfp[nBox];
    tDrawing drw;
    word x, y, wdt, hgt;
    byte idx;

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    x = 0;
    y = 0;
    wdt = cnvW;
    hgt = cnvH;
    for (idx = 0; idx < nBox; idx++) {
        ret = drwBoxInit(&box[idx], wdt, hgt, clrFgd);
        CHECK(ret >= 0, lblKO);

        ret = drwShapeInit(&shp[idx], shpBox, &box[idx]);
        CHECK(ret >= 0, lblKO);

        rfp[idx] = CARDHL(x, y);

        x += 2;
        y += 2;
        wdt -= 4;
        hgt -= 4;
    }

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    idx = nBox - 1;
    while(42) {
        drwHide(&drw, idx);
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);
        ret = drwDrawingDraw(&drw, 0, 0);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        if(kbdKey())
            break;
        drwShow(&drw, idx);
        //idx= (byte)sysRand()%nBox;
        if(!idx)
            idx = nBox;
        idx--;
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    kbdStop();
    tmrPause(3);
    return ret;
}

static int testButton(void) {   //button as drawing
    int ret;
    enum {
        btnShadow,
        btnBgd,
        btnFrame,
        btnFocus,
        btnText,
        btnIcon,
        btnEnd
    };
    tBar shw;
    tBar bgd;
    tBox frm;
    tBar foc;
    tContainer cnt;
    tText txt;

    //tTile ico;
    tShape shp[btnEnd + 1];
    card rfp[btnEnd];
    tDrawing btn;

    byte wdt = 32;
    byte hgt = 17;
    byte xTxt = 2;
    byte yTxt = hgt;

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    ret = drwBarInit(&shw, wdt, hgt, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[btnShadow], shpBar, &shw);
    CHECK(ret >= 0, lblKO);
    rfp[btnShadow] = CARDHL(2, 2);

    ret = drwBarInit(&bgd, wdt, hgt, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[btnBgd], shpBar, &bgd);
    CHECK(ret >= 0, lblKO);
    rfp[btnBgd] = CARDHL(0, 0);

    ret = drwBoxInit(&frm, wdt, hgt, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[btnFrame], shpBox, &frm);
    CHECK(ret >= 0, lblKO);
    rfp[btnFrame] = CARDHL(0, 0);

    ret = drwBarInit(&foc, (word) (wdt - 4), (word) (hgt - 4), clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[btnFocus], shpBar, &foc);
    CHECK(ret >= 0, lblKO);
    rfp[btnFocus] = CARDHL(2, 2);

    ret = cntInit(&cnt, 's', " OK");
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txt, &cnt, 0, 0, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[btnText], shpText, &txt);
    CHECK(ret >= 0, lblKO);
    rfp[btnText] = CARDHL(xTxt, yTxt);

    ret = drwDrawingInit(&btn, shp, rfp);
    CHECK(ret >= 0, lblKO);

    //Draw the button in the idle state
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&btn, 4, 4);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //Make the button focused
    foc.clr = clrFgd;
    txt.clr = clrBgd;
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&btn, 4, 4);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //Press the button
    rfp[btnFrame] = CARDHL(0 + 2, 0 + 2);
    rfp[btnText] = CARDHL(xTxt + 2, yTxt + 2);
    rfp[btnFocus] = CARDHL(2 + 2, 2 + 2);
    shw.clr = clrBgd;
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&btn, 4, 4);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //Free it
    rfp[btnFrame] = CARDHL(0, 0);
    rfp[btnText] = CARDHL(xTxt, yTxt);
    rfp[btnFocus] = CARDHL(2, 2);
    shw.clr = clrFgd;
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&btn, 4, 4);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //Return to normal state
    foc.clr = clrBgd;
    txt.clr = clrFgd;
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&btn, 4, 4);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    kbdStop();
    tmrPause(3);
    return ret;
}

#include "teevee15.fnt"
static int testGame15(void) {   //game15
    int ret;
    tBox frm;
    tTile fig[15];
    tShape shp[15 + 1 + 1];     //15 figs + 1 frame + terminator
    card rfp[15 + 1];           //15 figs + 1 frame
    tDrawing drw;
    byte cell[15 + 1];          //15 figs + empty cell
    const byte *tv[] = {
        tv01, tv02, tv03, tv04, tv05,
        tv06, tv07, tv08, tv09, tv10,
        tv11, tv12, tv13, tv14, tv15
    };
    byte i, j, k, idx;

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));
    memset(cell, 15, sizeof(cell));
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < 15; idx++) {
        ret = drwTileInit(&fig[idx], tv[idx], 15, 15, clrFgd);
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp[idx], shpTile, &fig[idx]);
        CHECK(ret >= 0, lblKO);
        ret = sysRand() % (15 - idx);
        k = (byte) ret;
        for (i = 0; i < 15; i++) {
            if(cell[i] == 15) {
                if(!k)
                    break;
                k--;
            }
        }
        VERIFY(cell[i] == 15);
        VERIFY(!k);
        k = i;
        cell[k] = idx;
        i = k % 4;
        j = k / 4;
        rfp[idx] = CARDHL(i * 16, j * 16);
    }

    idx = 15;
    ret = drwBoxInit(&frm, 64, 64, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBox, &frm);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(0, 0);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    while(42) {
        if(kbdKey())
            break;
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);
        ret = drwDrawingDraw(&drw, (cnvW - 64) / 2, (cnvH - 64) / 2);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        dly();
        dly();
        dly();
        dly();
        dly();

        //find empty place
        for (idx = 0; idx < 16; idx++) {
            if(cell[idx] == 15)
                break;
        }
        VERIFY(idx < 16);

        //get number of neighbours
        k = 0;
        for (i = 0; i < 15; i++) {
            if((i / 4 + 1 == idx / 4) && (i % 4 == idx % 4))
                k++;
            else if((i / 4 - 1 == idx / 4) && (i % 4 == idx % 4))
                k++;
            else if((i / 4 == idx / 4) && (i % 4 + 1 == idx % 4))
                k++;
            else if((i / 4 == idx / 4) && (i % 4 - 1 == idx % 4))
                k++;
        }
        VERIFY(k <= 4);
        VERIFY(k >= 2);

        //choose neighbour
        ret = sysRand() % k;
        k = (byte) ret;
        for (i = 0; i < 15; i++) {
            if((i / 4 + 1 == idx / 4) && (i % 4 == idx % 4)) {
                if(!k)
                    break;
                k--;
            } else if((i / 4 - 1 == idx / 4) && (i % 4 == idx % 4)) {
                if(!k)
                    break;
                k--;
            } else if((i / 4 == idx / 4) && (i % 4 + 1 == idx % 4)) {
                if(!k)
                    break;
                k--;
            } else if((i / 4 == idx / 4) && (i % 4 - 1 == idx % 4)) {
                if(!k)
                    break;
                k--;
            }
        }

        //Move
        cell[idx] = cell[i];
        cell[i] = 15;

        i = idx % 4;
        j = idx / 4;
        rfp[cell[idx]] = CARDHL(i * 16, j * 16);
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    kbdStop();
    tmrPause(3);
    return ret;
}

static int testCircle(void) {   //circle shapes
    int ret;
    tCircle cir[4];
    tDisc dsc[4];
    byte idx;
    tShape shp[4 + 4 + 1];      //4 circles + 4 discs + terminator
    card rfp[4 + 4];            //15 figs + 1 frame
    tDrawing drw;
    word x, y;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < 4; idx++) {
        ret = sysRand() % 16 + 2;
        ret = drwCircleInit(&cir[idx], (word) ret, (card) (sysRand() % cnvP));
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp[idx], shpCircle, &cir[idx]);
        CHECK(ret >= 0, lblKO);
        ret = sysRand() % cnvW;
        rfp[idx] = CARDHL(ret, cnvH - 1);

        ret = sysRand() % 16 + 2;
        ret = drwDiscInit(&dsc[idx], (word) ret, (card) (sysRand() % cnvP));
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp[4 + idx], shpDisc, &dsc[idx]);
        CHECK(ret >= 0, lblKO);
        ret = sysRand() % cnvW;
        rfp[4 + idx] = CARDHL(ret, 0);
    }
    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    while(42) {
        if(kbdKey())
            break;
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);
        ret = drwDrawingDraw(&drw, 0, 0);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        for (idx = 0; idx < 4; idx++) {
            ret = sysRand() % 3 - 1;
            if(cir[idx].rad + ret > 1 && cir[idx].rad + ret < 15)
                cir[idx].rad += ret;
            x = HWORD(rfp[idx]);
            y = LWORD(rfp[idx]);
            ret = y - (cir[idx].rad) % 4 - 1;
            if(ret < 0) {
                x = (word) (sysRand() % cnvW);
                y = cnvH - 1;
            } else
                y = ret;
            rfp[idx] = CARDHL(x, y);

            ret = sysRand() % 3 - 1;
            if(dsc[idx].rad + ret > 1 && dsc[idx].rad + ret < 15)
                dsc[idx].rad += ret;
            x = HWORD(rfp[4 + idx]);
            y = LWORD(rfp[4 + idx]);
            ret = y + (dsc[idx].rad) % 4 + 1;
            if(ret > cnvH) {
                x = (word) (sysRand() % cnvW);
                y = 0;
            } else
                y = ret;
            rfp[4 + idx] = CARDHL(x, y);
        }
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    kbdStop();
    tmrPause(3);
    return ret;
}

static int testBezier(void) {   //bezier curves
    int ret;
    card ctl[7][4];
    tBezier bzr;
    int x, y;
    byte idx, crv;

    for (crv = 0; crv < 7; crv++) {
        ctl[crv][0] = CARDHL(0, 0);
        x = sysRand() % cnvW;
        y = sysRand() % cnvH;
        ctl[crv][1] = CARDHL(x, y);
        x = sysRand() % cnvW;
        y = sysRand() % cnvH;
        ctl[crv][2] = CARDHL(x, y);
        ctl[crv][3] = CARDHL(cnvW - 1, cnvH - 1);
    }

    ret = kbdStart(1);
    while(42) {
        if(kbdKey())
            break;
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);

        for (crv = 0; crv < 7; crv++) {
            for (idx = 0; idx < 4; idx++) {
                x = HWORD(ctl[crv][idx]) + (sysRand() % 5 - 2);
                if(x < 0)
                    x = 0;
                if(x >= cnvW)
                    x = cnvW - 1;
                y = LWORD(ctl[crv][idx]) + (sysRand() % 5 - 2);
                if(y < 0)
                    y = 0;
                if(y >= cnvH)
                    y = cnvH - 1;
                ctl[crv][idx] = CARDHL(x, y);
            }

            ret = drwBezierInit(&bzr, ctl[crv], (byte) (sysRand() % cnvP));
            CHECK(ret >= 0, lblKO);
            ret = drwBezierDraw(&bzr, 0, 0);
            CHECK(ret >= 0, lblKO);
        }
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    kbdStop();
    tmrPause(3);
    return ret;
}

static int testSelector(void) { //selector as drawing
    int ret;
    enum {
        selBgd,
        selBak,
        selFoc,
        selEnd
    };
    enum {
        wdtChr = 8,             //single character width
        hgtRow = (cnvH / dspH), //single row height
        wdtMax = 10,            //maximal number of characters
        hgtMax = 3,             //maximal number of rows
        wdtSel = wdtMax * wdtChr,   //selector width in pixels
        hgtSel = hgtMax * hgtRow,   //selector height in pixels
        dimSel = (idxSelEnd + dspH + 1),    //number of shapes in selector drawing
        eEnd
    };
    tBar bgd;
    tBar bak;
    tBox foc;
    char itm[dspH][wdtMax];
    tContainer cnt[dspH];
    tText txt[dspH];
    tShape shp[dimSel + 1];
    card rfp[dimSel];
    tDrawing sel;
    enum { palTxt, palBgd, palBak, palCur, palFoc, palFnt, palEnd };
    card pal[] = { clrFgd, clrBgd, clrFgd, clrBgd, clrFgd, 0 };
    char *str[__MNUMAX__];
    byte idx, dim, row, top;

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));
    memset(str, 0, sizeof(str));

    dim = 0;
    str[dim++] = "MERCURY";
    str[dim++] = "VENUS";
    str[dim++] = "EARTH";
    str[dim++] = "MARS";
    str[dim++] = "JUPITER";
    str[dim++] = "SATURNE";
    str[dim++] = "URANUS";
    str[dim++] = "NEPTUN";
    str[dim++] = "PLUTON";

    idx = 0;
    VERIFY(idx == selBgd);
    ret = drwBarInit(&bgd, wdtSel, hgtSel, pal[palBgd]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBar, &bgd);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(0, 0);

    idx++;
    VERIFY(idx == selBak);
    ret = drwBarInit(&bak, wdtSel, hgtRow, pal[palBak]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBar, &bak);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(0, 0);

    idx++;
    VERIFY(idx == selFoc);
    ret = drwBoxInit(&foc, wdtSel, hgtRow, pal[palFoc]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBox, &foc);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(0, 0);

    for (row = 0; row < hgtMax; row++) {
        idx++;
        strcpy(itm[row], str[row]);
        ret = cntInit(&cnt[row], 's', itm[row]);
        CHECK(ret >= 0, lblKO);
        ret = drwTextInit(&txt[row], &cnt[row], 0, pal[palFnt], pal[palTxt]);
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp[idx], shpText, &txt[row]);
        CHECK(ret >= 0, lblKO);
        rfp[idx] = CARDHL(0, (row + 1) * hgtRow);
    }

    ret = drwDrawingInit(&sel, shp, rfp);
    CHECK(ret >= 0, lblKO);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    idx = 0;
    top = 0;
    kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(42) {
        VERIFY(top <= idx);
        VERIFY(idx - top < hgtMax);
        for (row = 0; row < hgtMax; row++)
            strcpy(itm[row], str[top + row]);
        row = idx - top;
        txt[row].clr = pal[palCur];
        rfp[selBak] = CARDHL(0, row * hgtRow);
        rfp[selFoc] = CARDHL(0, row * hgtRow);

        ret = drwDrawingDraw(&sel, 4, 4);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        tmrPause(1);

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        if(kbdKey())
            break;

        txt[row].clr = pal[palTxt];

        idx++;
        if(idx >= dim) {
            idx = 0;
            top = 0;
        } else if(top + hgtMax <= idx)
            top++;
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    kbdStop();
    tmrPause(3);
    return ret;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0127(void) {
    int ret;

    trcS("tcab0127 Beg\n");

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(testSelector);

    RUN(testBasicShapes);
    RUN(testBoxShapes);
    RUN(testButton);
    RUN(testGame15);
    RUN(testCircle);
    RUN(testBezier);
    RUN(testSelector);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0127 End\n");
}
#else
void tcab0127(void) {
}
#endif
