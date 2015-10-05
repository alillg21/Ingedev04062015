/** \file
 * Unitary test case tcab0128.
 * Drawing scenes
 * \sa
 *  - drwButtonInit()
 *  - drwButtonDraw()
 *  - btnState()
 *  - btnPush()
 *  - btnRefresh()
 *  - drwSelectorInit()
 *  - drwSelectorDraw()
 *  - selPos()
 *  - selRefresh()
 *  - drwSliderInit()
 *  - drwSliderDraw()
 *  - sldDim()
 *  - sldPos()
 *  - sldRefresh()
 *  - drwVideoInit()
 *  - drwVideoDraw()
 *  - vidDim()
 *  - vidPos()
 *  - vidRefresh()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0128.c $
 *
 * $Id: tcab0128.c 2534 2010-01-06 10:35:08Z abarantsev $
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

static int test1(void) {        //button as shape
    int ret;

    tBox frmOK;
    tContainer cntOK;
    tText txtOK;
    tButton btnOK;

    tBox frmCancel;
    tContainer cntCancel;
    tText txtCancel;
    tButton btnCancel;

    byte wdtOK = 32;
    byte hgtOK = 17;
    byte xTxtOK = 2;
    byte yTxtOK = hgtOK;

    byte wdtCancel = 64;
    byte hgtCancel = 17;
    byte xTxtCancel = 2;
    byte yTxtCancel = hgtCancel;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = drwBoxInit(&frmOK, wdtOK, hgtOK, 1);
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cntOK, 's', " OK");
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txtOK, &cntOK, 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnOK, &frmOK, CARDHL(WORDHL(xTxtOK, yTxtOK), 0), &txtOK,
                      0, 0);
    ret = drwButtonDraw(&btnOK, 4, 4);
    CHECK(ret >= 0, lblKO);

    ret = drwBoxInit(&frmCancel, wdtCancel, hgtCancel, 1);
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cntCancel, 's', " CANCEL");
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txtCancel, &cntCancel, 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnCancel, &frmCancel,
                      CARDHL(WORDHL(xTxtCancel, yTxtCancel), 0), &txtCancel, 0,
                      0);
    ret = drwButtonDraw(&btnCancel, 50, 4);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(42) {
        ret = sysRand() % 2;
        if(ret) {
            btnState(&btnCancel, 'n');
            btnState(&btnOK, 'f');
            btnPush(&btnOK);
        } else {
            btnState(&btnOK, 'n');
            btnState(&btnCancel, 'f');
            btnPush(&btnCancel);
        }
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        if(kbdKey())
            break;
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

static int test2(void) {        //keyboard
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
        ret = drwTextInit(&txt[idx], &cnt[idx], 0, 0, 1);
        CHECK(ret >= 0, lblKO);
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
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
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
            ret = dspLight(100);
            CHECK(ret >= 0, lblKO);
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

static int test3(void) {        //progress indicator
    int ret;
    tBar beg;
    tBar end;
    tSlider sld;
    word idx, dim;

    //build slider
    ret = drwBarInit(&beg, 1, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&end, cnvW - 8, 8, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwSliderInit(&sld, &beg, &end, 0, 0);
    CHECK(ret >= 0, lblKO);
    dim = sldDim(&sld);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwSliderDraw(&sld, 4, 4);
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx <= dim; idx++) {
        ret = sldPos(&sld, idx);
        CHECK(ret >= 0, lblKO);
        ret = sldRefresh(&sld);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        dly();
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

static int test4(void) {        //scene with progress indicator
    int ret;
    tBar beg;
    tBar end;
    tSlider sld;
    char buf[64];
    tContainer cnt;
    tText txt;
    tBox box;
    tDrawing drw;
    tShape shp[3 + 1];          //text + box + slider + terminator
    card rfp[3 + 1];
    word idx, dim, num;
    const char *phases[] = {
        "Project",
        "Requirements",
        "Perimeter",
        "Feasibility",
        "Risks",
        "Concept",
        "Design",
        "Charges",
        "Esimation",
        "Scheduling",
        "Project plan",
        "Milestones",
        "Architecture",
        "Development",
        "Testing",
        "Validation",
        "Approval",
        "Champagne!",
    };

    //build slider
    ret = drwBarInit(&beg, 1, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&end, cnvW - 8, 8, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwSliderInit(&sld, &beg, &end, 0, 0);
    CHECK(ret >= 0, lblKO);
    dim = sldDim(&sld);

    //design scene
    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    idx = 0;

    ret = drwBoxInit(&box, cnvW - 4, 12, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpBox, &box);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(2, cnvH - 2 - 12);

    ret = cntInit(&cnt, 's', buf);
    CHECK(ret >= 0, lblKO);
    ret = drwTextInit(&txt, &cnt, 0, 0, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpText, &txt);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(0, cnvH / 2 - 8);

    ret = drwShapeInit(&shp[idx], shpSlider, &sld);
    CHECK(ret >= 0, lblKO);
    rfp[idx++] = CARDHL(4, cnvH - 12);

    strcpy(buf, phases[0]);
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = drwDrawingDraw(&drw, 0, 0);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    num = sizeof(phases) / sizeof(char *);
    for (idx = 0; idx <= dim; idx++) {
        strcpy(buf, phases[(idx * (num - 1)) / dim]);
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);
        ret = sldPos(&sld, idx);
        CHECK(ret >= 0, lblKO);
        ret = drwDrawingDraw(&drw, 0, 0);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        dly();
    }

    strcpy(buf, phases[num - 1]);
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
    kbdStop();
    tmrPause(3);
    return ret;
}

static int test5(void) {        //vertical and horizontal scrollbars
    int ret;
    tBar begHor, endHor, begVer, endVer;
    tSlider sldHor, sldVer;
    word idx, dim;

    //build horizontal scrollbar
    ret = drwBarInit(&begHor, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&endHor, 8, 8, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwSliderInit(&sldHor, &begHor, &endHor, cnvW - 16, 0);
    CHECK(ret >= 0, lblKO);
    dim = sldDim(&sldHor);

    //demonstrate it
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwSliderDraw(&sldHor, 4, 4);
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx <= dim; idx++) {
        ret = sldPos(&sldHor, idx);
        CHECK(ret >= 0, lblKO);
        //ret= sldRefresh(&sldHor); CHECK(ret>=0,lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        dly();
    }

    //build vertical scrollbar
    ret = drwBarInit(&begVer, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&endVer, 8, 8, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwSliderInit(&sldVer, &begVer, &endVer, 0, cnvH - 16);
    CHECK(ret >= 0, lblKO);
    dim = sldDim(&sldVer);

    //demonstrate it
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwSliderDraw(&sldVer, 4, 4);
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx <= dim; idx++) {
        ret = sldPos(&sldVer, idx);
        CHECK(ret >= 0, lblKO);
        //ret= sldRefresh(&sldVer); CHECK(ret>=0,lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        dly();
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
static const byte arrLf[] = {
    0x08,                       //....*...
    0x1E,                       //...****.
    0x3E,                       //..*****.
    0x7E,                       //.******.
    0x7E,                       //.******.
    0x3E,                       //..*****.
    0x1E,                       //...****.
    0x08,                       //....*...
};
static const byte arrRg[] = {
    0x10,                       //...*....
    0x78,                       //.****...
    0x7C,                       //.*****..
    0x7E,                       //.******.
    0x7E,                       //.******.
    0x7C,                       //.*****..
    0x78,                       //.****...
    0x10,                       //...*....
};

static int test6(void) {        //scene with vertical and horizontal scrollbars
    int ret;
    tBar begHor, endHor, begVer, endVer;
    tBox boxHor, boxVer;
    tSlider sldHor, sldVer;

    //char *bufVer[16+1][3];
    //tContainer cntVer[3];
    //tText txtVer[3];
    tTile icoUp, icoDn, icoLf, icoRg;
    tShape shpVer[1 + 1 + 2 + 3 + 1];   //boxVer+sldVer+2 tiles+3 texts+terminator
    card rfpVer[1 + 1 + 2 + 3];

    //char bufHor[256];
    //tContainer cntHor;
    //tText txtHor;
    tShape shpHor[1 + 1 + 2 + 1 + 1];   //boxVer+sldVer+2 tiles+1 text+terminator
    card rfpHor[1 + 1 + 2 + 1];
    tShape shp[1 + 1 + 1];      //drwVer+drwHor+terminator
    card rfp[1 + 1];
    tDrawing drwHor, drwVer, drw;
    word idx;
    const word hgtArr = 8;
    const word wdtArr = 8;
    const word wdtSldMar = 2;
    const word hgtSldMar = 2;
    const word wdtBoxMar = 1;
    const word hgtBoxMar = 1;

    const word wdtSld = wdtArr;
    const word hgtSld = hgtArr;

    const word wdtBoxVer = wdtSldMar + wdtSld + wdtSldMar;
    const word hgtBoxHor = hgtSldMar + hgtSld + hgtSldMar;

    const word hgtBoxVer = cnvH - hgtBoxMar * 2 - hgtBoxHor;
    const word rfyArrDn = hgtBoxVer - hgtSldMar - hgtArr - hgtSldMar;
    const word rfySldVer = hgtSldMar + hgtArr + hgtSldMar;

    const word wdtBoxHor = cnvW - wdtBoxMar * 2 - wdtBoxVer;
    const word rfxArrRg = wdtBoxHor - wdtSldMar - wdtArr - wdtSldMar;
    const word rfxSldHor = wdtSldMar + wdtArr + wdtSldMar;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    memset(shpVer, 0, sizeof(shpVer));
    memset(rfpVer, 0, sizeof(rfpVer));
    memset(shpHor, 0, sizeof(shpHor));
    memset(rfpHor, 0, sizeof(rfpHor));
    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    //design drwVer
    idx = 0;
    ret = drwBoxInit(&boxVer, wdtBoxVer, hgtBoxVer, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpVer[idx], shpBox, &boxVer);
    CHECK(ret >= 0, lblKO);
    rfpVer[idx] = CARDHL(0, 0);

    idx++;
    ret = drwTileInit(&icoUp, arrUp, wdtArr, hgtArr, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpVer[idx], shpTile, &icoUp);
    CHECK(ret >= 0, lblKO);
    rfpVer[idx] = CARDHL(wdtSldMar, hgtSldMar);

    idx++;
    ret = drwTileInit(&icoDn, arrDn, wdtArr, hgtArr, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpVer[idx], shpTile, &icoDn);
    CHECK(ret >= 0, lblKO);
    rfpVer[idx] = CARDHL(wdtSldMar, rfyArrDn);

    idx++;
    ret = drwBarInit(&begVer, wdtSld, hgtSld, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&endVer, wdtSld, hgtSld, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwSliderInit(&sldVer, &begVer, &endVer, 0,
                      cnvH - 2 * hgtArr - 2 * hgtSldMar);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpVer[idx], shpSlider, &sldVer);
    CHECK(ret >= 0, lblKO);
    rfpVer[idx] = CARDHL(wdtSldMar, rfySldVer);

    ret = drwDrawingInit(&drwVer, shpVer, rfpVer);
    CHECK(ret >= 0, lblKO);
    //ret= drwDrawingDraw(&drwVer,cnvW-wdtBoxVer-1,1); CHECK(ret>=0,lblKO);
    //ret= cnvDraw(); CHECK(ret>=0,lblKO);

    //design drwHor
    idx = 0;
    ret = drwBoxInit(&boxHor, wdtBoxHor, hgtBoxHor, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpHor[idx], shpBox, &boxHor);
    CHECK(ret >= 0, lblKO);
    rfpHor[idx] = CARDHL(0, 0);

    idx++;
    ret = drwTileInit(&icoLf, arrLf, wdtArr, hgtArr, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpHor[idx], shpTile, &icoLf);
    CHECK(ret >= 0, lblKO);
    rfpHor[idx] = CARDHL(wdtSldMar, hgtSldMar);

    idx++;
    ret = drwTileInit(&icoRg, arrRg, wdtArr, hgtArr, 1);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpHor[idx], shpTile, &icoRg);
    CHECK(ret >= 0, lblKO);
    rfpHor[idx] = CARDHL(rfxArrRg, hgtSldMar);

    idx++;
    ret = drwBarInit(&begHor, wdtSld, hgtSld, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&endHor, wdtSld, hgtSld, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwSliderInit(&sldHor, &begHor, &endHor,
                      cnvW - 2 * wdtArr - 2 * wdtSldMar, 0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shpHor[idx], shpSlider, &sldHor);
    CHECK(ret >= 0, lblKO);
    rfpHor[idx] = CARDHL(rfxSldHor, hgtSldMar);

    ret = drwDrawingInit(&drwHor, shpHor, rfpHor);
    CHECK(ret >= 0, lblKO);
    //ret= drwDrawingDraw(&drwHor,1,cnvH-hgtBoxHor-1); CHECK(ret>=0,lblKO);
    //ret= cnvDraw(); CHECK(ret>=0,lblKO);

    //design drw
    idx = 0;
    ret = drwShapeInit(&shp[idx], shpDrawing, &drwVer);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(cnvW - wdtBoxVer - 1, 1);

    idx++;
    ret = drwShapeInit(&shp[idx], shpDrawing, &drwHor);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(1, cnvH - hgtBoxHor - 1);

    ret = drwDrawingInit(&drw, shp, rfp);
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
    kbdStop();
    tmrPause(3);
    return ret;
}

static int test7(void) {        //video
    int ret;
    tBar bgd;
    tVideo vid;
    byte idx;

     /**/
#include "SWIFTY.fnt"
    const word wdt = 39;
    const word hgt = 57;

     /**/
        /*
           #include "Skello.fnt"
           const word wdt= 40;
           const word hgt= 55;
         */
        /*
           #include "Robo.fnt"
           const word wdt= 29;
           const word hgt= 38;
         */
        /*
           #include "DANCEMAN.fnt"
           const word wdt= 19;
           const word hgt= 28;
         */
        /*
           #include "DANCE_FX.fnt"
           const word wdt= 70;
           const word hgt= 28;
         */
        /*
           #include "PIE4MAP.fnt"
           const word wdt= 48;
           const word hgt= 47;
         */
#define DIM (sizeof(gly)/sizeof(void *)-1)
        tTile pic[DIM];
    tShape shp[1 + DIM + 1];
    card rfp[1 + DIM];
    tDrawing drw;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    ret = drwBarInit(&bgd, wdt, hgt, clrBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[0], shpBar, &bgd);
    CHECK(ret >= 0, lblKO);
    rfp[0] = CARDHL(0, 0);

    for (idx = 1; idx < DIM; idx++) {
        ret = drwTileInit(&pic[idx - 1], gly[idx - 1], wdt, hgt, clrFgd);
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp[idx], shpTile, &pic[idx - 1]);
        CHECK(ret >= 0, lblKO);
        rfp[idx] = CARDHL(0, 0);
    }
    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = drwVideoInit(&vid, &drw);
    CHECK(ret >= 0, lblKO);
    ret =
        drwVideoDraw(&vid, (word) ((cnvW - wdt) / 2),
                     (word) ((cnvH - hgt) / 2));
    CHECK(ret >= 0, lblKO);

    idx = 1;
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(42) {
        vidPos(&vid, idx);
        vidRefresh(&vid);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        if(kbdKey())
            break;
        dly();
        dly();
        dly();
        dly();
        dly();
        idx++;
        if(idx < vidDim(&vid))
            continue;
        idx = 1;
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
#undef DIM
}

static int test8(void) {        //button with icon
    int ret;

    tBox frm;
    tTile icoUp, icoDn, icoLf, icoRg;
    tButton btnUp, btnDn, btnLf, btnRg;

    byte xIco = 2;
    byte yIco = 2;

    byte wdt = xIco + 8 + xIco;
    byte hgt = yIco + 8 + yIco;

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwBoxInit(&frm, wdt, hgt, clrFgd);
    CHECK(ret >= 0, lblKO);

    ret = drwTileInit(&icoUp, arrUp, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnUp, &frm, CARDHL(0, WORDHL(xIco, yIco)), 0, &icoUp,
                      0);
    CHECK(ret >= 0, lblKO);
    ret = drwButtonDraw(&btnUp, 16, 16);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = drwTileInit(&icoDn, arrDn, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnDn, &frm, CARDHL(0, WORDHL(xIco, yIco)), 0, &icoDn,
                      0);
    CHECK(ret >= 0, lblKO);
    ret = drwButtonDraw(&btnDn, 16, 32);
    CHECK(ret >= 0, lblKO);

    ret = drwTileInit(&icoLf, arrLf, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnLf, &frm, CARDHL(0, WORDHL(xIco, yIco)), 0, &icoLf,
                      0);
    CHECK(ret >= 0, lblKO);
    ret = drwButtonDraw(&btnLf, 32, 16);
    CHECK(ret >= 0, lblKO);

    ret = drwTileInit(&icoRg, arrRg, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret =
        drwButtonInit(&btnRg, &frm, CARDHL(0, WORDHL(xIco, yIco)), 0, &icoRg,
                      0);
    CHECK(ret >= 0, lblKO);
    ret = drwButtonDraw(&btnRg, 32, 32);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(42) {
        ret = sysRand() % 4;
        switch (ret) {
          case 0:
              btnState(&btnUp, 'f');
              btnState(&btnDn, 'n');
              btnState(&btnLf, 'n');
              btnState(&btnRg, 'n');
              btnPush(&btnUp);
              break;
          case 1:
              btnState(&btnUp, 'n');
              btnState(&btnDn, 'f');
              btnState(&btnLf, 'n');
              btnState(&btnRg, 'n');
              btnPush(&btnDn);
              break;
          case 2:
              btnState(&btnUp, 'n');
              btnState(&btnDn, 'n');
              btnState(&btnLf, 'f');
              btnState(&btnRg, 'n');
              btnPush(&btnLf);
              break;
          case 3:
              btnState(&btnUp, 'n');
              btnState(&btnDn, 'n');
              btnState(&btnLf, 'n');
              btnState(&btnRg, 'f');
              btnPush(&btnRg);
              break;
          default:
              VERIFY(ret < 4);
              break;
        }
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        if(kbdKey())
            break;
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

static int test9(void) {        //selector as a shape
    int ret;

    tBar bgd;
    tBox foc;
    tSelector sel;
    Pchar str[] = {
        "BRAHMA",
        "VISHNU",
        "SHIVA",
        "KRISHNA",
        "GANESH",
        "BHAVA",
        "SHARVA",
        "PASHUPATI",
        "UGRA",
        "ASHANI",
        "CHANDA",
        "ISHANA",
        "BHIMA",
        "MAHADEVA",
        "CHANDIKA",
        "VIBHISHANA",
        0
    };
    byte wdt, hgt, rowHgt, dim, idx, top;
    word clrSelBgd = clrBgd;
    word clrSelFoc = clrFgd;

    wdt = (byte) ((cnvW / dspW - 2) * dspW);
    hgt = (cnvH / dspH - 1) * dspH;
    rowHgt = (cnvH / dspH);

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
    ret = drwSelectorDraw(&sel, (byte) (cnvW / dspW), 3);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    top = 0;
    idx = 0;
    while(42) {
        ret = selPos(&sel, str, WORDHL(top, idx));
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);
        if(kbdKey())
            break;
        idx++;
        if(idx >= dim) {
            idx = 0;
            top = 0;
        } else if(top + selDim(&sel) <= idx)
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
void tcab0128(void) {
    int ret;

    trcS("tcab0128 Beg\n");

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(test1);
    RUN(test2);
    RUN(test3);
    RUN(test4);
    RUN(test5);
    RUN(test6);
    RUN(test7);
    RUN(test8);
    RUN(test9);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0128 End\n");
}
#else
void tcab0128(void) {
}
#endif
