/** \file
 * Unitary test case tcab0198.
 * Shape cloning
 * \sa
 *  - drwBoxInit()
 *  - cntInit()
 *  - drwButtonInit()
 *  - drwShapeInit()
 *  - btnState()
 *  - btnState()
 *  - btnPush()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0198.c $
 *
 * $Id: tcab0198.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

#ifdef __CANVAS__
static void dly(void) {
    tmrStart(0, 1);
    while(tmrGet(0));
    tmrStop(0);
}

static int test1(void) {        //keyboard
    int ret;
    byte idx, pos, sta;
    char buf[16 + 1];
    char tmp[16 + 1];

    tBox frm[12 + 1];
    char dgt[10][1 + 1];
    tContainer cnt[12 + 1];
    tText txt[12 + 1];
    tButton btn[12];

    byte wdt = 12;
    byte hgt = 17;
    byte xTxt = 2;
    byte yTxt = hgt;
    byte dx = 16;
    byte dy = 20;

    card n0, n1, n2;

    tShape shp[12 + 1 + 1 + 1]; //12 buttons + 1 box + 1 text + terminator
    card rfp[12 + 1 + 1];
    tDrawing drw;

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < 10; idx++) {
        num2dec(dgt[idx], idx, 1);
        ret = drwBoxInit(&frm[idx], wdt, hgt, 1);
        CHECK(ret >= 0, lblKO);
        ret = cntInit(&cnt[idx], 's', dgt[idx]);
        CHECK(ret >= 0, lblKO);
    }

    ret = drwTextInit(&txt[0], &cnt[0], 0, 0, 1);
    CHECK(ret >= 0, lblKO);

    ret = drwTextClone(txt, cnt, 0, 0, 0, 10);
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < 10; idx++) {
        ret =
            drwButtonInit(&btn[idx], &frm[idx], CARDHL(WORDHL(xTxt, yTxt), 0),
                          &txt[idx], 0, 0);
        ret = drwShapeInit(&shp[idx], shpButton, &btn[idx]);
        CHECK(ret >= 0, lblKO);
        rfp[idx] = CARDHL(4 + (idx % 5) * dx, 4 + (idx / 5) * dy);

    }

    idx = 10;
    ret = drwBoxInit(&frm[idx], wdt, hgt, 1);
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cnt[idx], 's', "+");
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btn[idx], &frm[idx], CARDHL(WORDHL(xTxt, yTxt), 0),
                      &txt[idx], 0, 0);
    ret = drwShapeInit(&shp[idx], shpButton, &btn[idx]);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(4 + (5 * dx) + 10, 4);

    idx = 11;
    ret = drwBoxInit(&frm[idx], wdt, hgt, 1);
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cnt[idx], 's', "=");
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btn[idx], &frm[idx], CARDHL(WORDHL(xTxt, yTxt), 0),
                      &txt[idx], 0, 0);
    ret = drwShapeInit(&shp[idx], shpButton, &btn[idx]);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(4 + (5 * dx) + 10, 4 + dy);

    idx = 12;
    ret = drwBoxInit(&frm[idx], cnvW - 4, hgt, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBox, &frm[idx]);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(2, cnvH - 2 - hgt);

    memset(buf, 0, 16 + 1);
    ret = cntInit(&cnt[idx], 's', buf);
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx + 1], shpText, &txt[idx]);
    CHECK(ret >= 0, lblKO);
    rfp[idx + 1] = CARDHL(2, cnvH - 2);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);

    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(42) {
        if(kbdKey())
            break;
        n1 = sysRand() % 1000;
        n2 = sysRand() % 1000;
        n0 = n1 + n2;

        num2dec(tmp, n1, 0);
        strcpy(buf, tmp);
        strcat(buf, "+");
        num2dec(tmp, n2, 0);
        strcat(buf, tmp);
        strcat(buf, "=");
        num2dec(tmp, n0, 0);
        strcat(buf, tmp);

        VERIFY(strlen(buf) <= 16);
        strcpy(tmp, buf);
        memset(buf, 0, 16 + 1);

        idx = 0;
        pos = 0;
        sta = 1;
        while(tmp[idx]) {
            btnState(&btn[pos], 'n');
            switch (tmp[idx]) {
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                  pos = tmp[idx] - '0';
                  break;
              case '+':
                  pos = 10;
                  break;
              case '=':
                  pos = 11;
                  break;
              default:
                  break;
            }
            buf[idx] = tmp[idx];
            if(sta) {
                btnState(&btn[pos], 'f');
                btnPush(&btn[pos]);
                ret = cnvClear();
                CHECK(ret >= 0, lblKO);
                ret = drwDrawingDraw(&drw, 0, 0);
                CHECK(ret >= 0, lblKO);
                ret = cnvDraw();
                CHECK(ret >= 0, lblKO);
                dly();
                dly();
                dly();
                dly();
                dly();
            }
            if(tmp[idx] == '=')
                sta = 0;
            idx++;
            if(kbdKey())
                goto lblEnd;
        }
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);
        ret = drwDrawingDraw(&drw, 0, 0);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);
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
void tcab0198(void) {
    int ret;

    trcS("tcab0198 Beg\n");

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(test1);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0198 End\n");
}
#else
void tcab0198(void) {
}
#endif
