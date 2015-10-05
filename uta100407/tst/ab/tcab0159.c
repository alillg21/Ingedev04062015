/** \file
 * Unitary test case tcab0159.
 * Moving shapes
 * \sa
 *  - drwBarInit()
 *  - drwTextInit()
 *  - drwMove()
 *  - drwShow()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0159.c $
 *
 * $Id: tcab0159.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)

enum { nShp = 3 };

static makeBars(tDrawing * drw, tShape * shp) {
    int ret;
    static tBar bar[nShp];
    static card rfp[nShp];
    byte idx;
    word x, y;

    VERIFY(drw);
    VERIFY(shp);

    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < nShp; idx++) {
        x = (word) (8 + idx * 3);
        y = (word) (5 + idx * 2);
        ret = drwBarInit(&bar[idx], x, y, 1);
        CHECK(ret >= 0, lblKO);
        trcFN("Bar[%d]: ", idx);
        trcFN("wdt=%d ", x);
        trcFN("hgt=%d ", y);

        ret = drwShapeInit(&shp[idx], shpBar, &bar[idx]);
        CHECK(ret >= 0, lblKO);

        x = 16 + 12 * idx;
        y = 32 + 15 * idx;
        rfp[idx] = CARDHL(x, y);
        trcFN("x=%d ", x);
        trcFN("y=%d ", y);
        trcS("\n");
    }
    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);

    return nShp;
  lblKO:return -1;
}

static makeTexts(tDrawing * drw, tShape * shp) {
    int ret;
    static tContainer cnt[nShp];
    static tText txt[nShp];
    static char *str[] = { "Generator", "Organizer", "Destroyer" };
    static card rfp[nShp];
    byte idx;

    VERIFY(drw);
    VERIFY(shp);

    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < nShp; idx++) {
        ret = cntInit(&cnt[idx], 's', str[idx]);
        CHECK(ret >= 0, lblKO);
        ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
        CHECK(ret >= 0, lblKO);

        ret = drwShapeInit(&shp[idx], shpText, &txt[idx]);
        CHECK(ret >= 0, lblKO);

        rfp[idx] = CARDHL(8 + 16 * idx, 16 + 32 * idx);
    }
    ret = drwDrawingInit(drw, shp, rfp);
    CHECK(ret >= 0, lblKO);

    return nShp;
  lblKO:return -1;
}

static int testMove(tDrawing * drw, tShape * shp) {
    int ret, kbd, tft, num;
    byte idx, cur, refresh;
    int tx, ty, sx, sy, dx, dy;
    char sta;

    VERIFY(drw);
    VERIFY(shp);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    tx = ty = sx = sy = dx = dy = -1;
    cur = nShp;
    sta = 0;                    //the state of tft
    num = 0;
    idx = 0;
    kbd = 0;
    tft = 0;

    refresh = 1;
    while(1) {
        if(refresh) {
            ret = cnvClear();
            CHECK(ret >= 0, lblKO);
            ret = drwDrawingDraw(drw, 0, 0);
            CHECK(ret >= 0, lblKO);
            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);
        }

        ret = tftStart();
        CHECK(ret >= 0, lblKO);
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);

        tmrStart(1, 100);
        while(tmrGet(1)) {
            kbd = kbdKey();
            if(kbd)
                break;
            tft = tftGet();
            ret = tftState();
            if(ret == 'd' && cur < nShp) {
                trcS(":::");
                while(ret != 'u') {
                    tft = tftGet();
                    ret = tftState();
                    trcS(".");
                }
                trcS("\n");
            }
            if(ret != sta)
                tft = tftLocation();
            if(tft) {
                VERIFY(tft);
                break;
            }
        }
        kbdStop();
        tftStop();
        refresh = kbd | tft;

        if(kbd)
            break;
        ret = tftState();
        if(ret != sta)
            refresh = 1;

        CHECK(tft != -1, lblKO);    //error during touch waiting
        if(!tft && (ret == sta))
            continue;           //if there is no changement in tft, continue
        sta = ret;

        tx = HWORD(tft);
        ty = LWORD(tft);

        trcFN("num=%d ", num++);
        trcFN("tx=%d ", tx);
        trcFN("ty=%d ", ty);
        trcFN("dx=%d ", dx);
        trcFN("dy=%d ", dy);
        trcFN("cur=%d ", cur);
        trcFN("sta=%c ", sta);
        trcS("\n");
        switch (sta) {
          case 'd':            //DOWN event, the pen touches screen
              if(cur >= nShp) { //if nothing is selected yet
                  for (idx = 0; idx < nShp; idx++) {
                      ret = drwShapeRel(&shp[idx], tx, ty);
                      if(ret == RELOUT)
                          continue;
                      dx = HWORD(ret);
                      dy = LWORD(ret);
                      sx = tx;
                      sy = ty;
                      break;
                  }
              }
              if(idx >= nShp)   //touched on free space
                  continue;

              cur = idx;
              drwHide(drw, cur);    //hide current shape: it is to be moved elsewhere
              trcS("Hide\n");
              break;
          case 'u':            //the pen is just released
          default:             //the pen does not touch screen any more
              if(cur < nShp) {  //if there is an object selected
                  drwMove(drw, cur, (word) (tx - dx), (word) (ty - dy));    //move it to a new place
                  drwShow(drw, cur);    //show it at a new place
                  trcS("Show\n");
                  cur = nShp;
              }
              break;
        }
    }
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    kbdStop();
    tftStop();
    return ret;
}

static int testBars(tDrawing * drw, tShape * shp) {
    int ret;

    VERIFY(drw);
    VERIFY(shp);

    ret = makeBars(drw, shp);
    CHECK(ret >= 0, lblKO);
    ret = testMove(drw, shp);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:return -1;
}

static int testTexts(tDrawing * drw, tShape * shp) {
    int ret;

    VERIFY(drw);
    VERIFY(shp);

    ret = makeTexts(drw, shp);
    CHECK(ret >= 0, lblKO);
    ret = testMove(drw, shp);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:return -1;
}

void tcab0159(void) {
    int ret;
    tShape shp[nShp + 1];
    tDrawing drw;

    trcS("tcab0159 Beg\n");

    RUN(dspStart);
    RUN(cnvStart);
    memset(shp, 0, sizeof(shp));

    ret = testBars(&drw, shp);
    CHECK(ret >= 0, lblKO);
    ret = testTexts(&drw, shp);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    dspStop();
    trcS("tcab0159 End\n");
}
#else
void tcab0159(void) {
}
#endif
