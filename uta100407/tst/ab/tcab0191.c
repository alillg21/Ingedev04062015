#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__
//Unitary test case tcab0191.
//Drag and drop interface

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)

enum { nShp = 3 };

static makeBars(tDrawing * drw, tShape * shp) {
    int ret;
    static tBar bar[nShp];
    static card rfp[nShp];
    byte idx;

    VERIFY(drw);
    VERIFY(shp);

    memset(rfp, 0, sizeof(rfp));

    for (idx = 0; idx < nShp; idx++) {
        ret =
            drwBarInit(&bar[idx], (word) (8 + idx * 3), (word) (5 + idx * 2),
                       1);
        CHECK(ret >= 0, lblKO);

        ret = drwShapeInit(&shp[idx], shpBar, &bar[idx]);
        CHECK(ret >= 0, lblKO);

        rfp[idx] = CARDHL(16 + 12 * idx, 32 + 15 * idx);
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
    int ret;
    byte idx;
    char sta;
    int tx, ty, sx, sy;
    card loc;
    int tft;

    VERIFY(drw);
    VERIFY(shp);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    tx = ty = sx = sy = -1;
    idx = nShp;
    sta = 0;

    while(42) {
        ret = tmrStart(0, 50);
        CHECK(ret >= 0, lblKO);

        ret = kbdStart(1);      //start keyboard waiting
        CHECK(ret >= 0, lblKO);

        ret = tftStart();       //start touch waiting
        CHECK(ret >= 0, lblKO);

        while(tmrGet(0)) {
            ret = cnvClear();
            CHECK(ret >= 0, lblKO);
            trcMode(0);
            ret = drwDrawingDraw(drw, 0, 0);
            CHECK(ret >= 0, lblKO);
            trcMode(0xFF);
            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);

            if(kbdKey())
                break;

            tft = tftGet();
            CHECK(tft != -1, lblKO);    //error during touch waiting

            ret = kbdStop();    //stop keyboard waiting
            CHECK(ret >= 0, lblKO);

            ret = tftStop();    //stop touch waiting
            CHECK(ret >= 0, lblKO);

            tmrStop(0);

            if(!tft)
                continue;

            loc = (card) ret;
            tx = HWORD(loc);
            ty = LWORD(loc);
            ret = tftState();
            switch (ret) {
              case 'd':
                  sta = 'd';
                  break;
              case 'u':
                  sta = 'u';
                  break;
              default:
                  if(sta == 'd')
                      sta = 'u';
                  else if(sta == 'u')
                      sta = 0;
                  break;
            }
            trcFN("sta=%c ", sta);
            trcFN("loc=%08x\n", loc);

            if(sta == 'u' && idx < nShp) {
                drwHide(drw, idx);
                drwMove(drw, idx, (word) tx, (word) ty);
                drwShow(drw, idx);
                sx = -1;
                sy = -1;
                idx = nShp;
                sta = 0;
            } else if(sta == 'd') {
                for (idx = 0; idx < nShp; idx++) {
                    if(drwShapeRel(&shp[idx], tx, ty) != RELOUT)
                        break;
                }
                if(idx >= nShp)
                    continue;

                sx = tx;
                sy = ty;
            }
        }
    }
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    tftStop();
    kbdStop();
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

void tcab0191(void) {
    int ret;
    tShape shp[nShp + 1];
    tDrawing drw;

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
    kbdStop();
    tftStop();
    cnvStop();
    dspStop();
}
#else
void tcab0159(void) {
}
#endif
