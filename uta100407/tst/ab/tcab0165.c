/** \file
 * Unitary test case for pri and sys components: color bitmaps.
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0165.c $
 *
 * $Id: tcab0165.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#include <string.h>
#include "pri.h"
#include "tst.h"

#if __CNVD__ >1
static void dly(void) {
    tmrStart(0, 1);
    while(tmrGet(0));
    tmrStop(0);
}

static void wait(void) {
    kbdStart(1);
    while(!kbdKey())
        dspLight(100);
    kbdStop();
}

static int testCnvColorBars(void) { //present colors
    int ret;
    word x, y, w, h;
    card c;
    byte i, j;

    trcS("testCnvColorBars: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    w = cnvW / 16;
    h = cnvH / 16;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            x = (word) (j * w);
            y = (word) (i * h);
            c = i * 16 + j;

            ret = cnvBar(x, y, (word) (x + w - 1), (word) (y + h - 1), c);
            CHECK(ret >= 0, lblKO);

            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);

            dspLight(100);
            if(kbdKey())
                break;

            dly();
        }
    }
    wait();
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvColorBars: End\n");
    return 1;
}

#define N 128
static int testCnvColorTile(void) { //display color tile
    int ret;
    byte tile[N * N];
    byte i, j;

    trcS("testCnvColorTile: Beg\n");

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            tile[j * N + i] = i + j;
        }
    }

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret =
        cnvTile((word) ((cnvW - N) / 2), (word) ((cnvH - N) / 2), N, N, -1,
                tile);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    wait();

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testCnvColorTile: End\n");
    return 1;
}

#undef N

static int testBmp(const byte * bmp) {  //display color bitmap
    int ret;
    tTile img;

    ret = drwTileInit(&img, bmp, 0, 0, -255);
    CHECK(ret >= 0, lblKO);

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = drwTileDraw(&img, 0, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    wait();

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return 1;
}

extern const unsigned char BMP_btnBak[];
extern const unsigned char BMP_btnBak2[];
extern const unsigned char BMP_btnCap[];
extern const unsigned char BMP_btnKO[];
extern const unsigned char BMP_btnNo[];
extern const unsigned char BMP_btnOK[];
extern const unsigned char BMP_btnSa0[];

extern const unsigned char BMP_Moebius[];
extern const unsigned char BMP_Logo[];
extern const unsigned char BMP_Fond[];
extern const unsigned char BMP_picto[];
extern const unsigned char BMP_ingenico[];
extern const unsigned char BMP_uta[];
extern const unsigned char BMP_bgd[];

static int testCnvColorImage(void) {    //display various color images
    int ret;

    trcS("testCnvColorImage: Beg\n");

    ret = testBmp(BMP_btnBak);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_btnBak2);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_btnCap);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_btnKO);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_btnNo);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_btnOK);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_btnSa0);
    CHECK(ret >= 0, lblKO);

    ret = testBmp(BMP_Moebius);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_bgd);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_uta);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_ingenico);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_Logo);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_Fond);
    CHECK(ret >= 0, lblKO);
    ret = testBmp(BMP_picto);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testCnvColorImage: End\n");
    return 1;
}

static int testDrwButtons(void) {   //display various color buttons
    int ret;
    byte idx;
    const byte *img[] =
        { BMP_btnBak, BMP_btnBak2, BMP_btnCap, BMP_btnKO, BMP_btnNo, BMP_btnOK,
        BMP_btnSa0
    };
#define DIM (sizeof(img)/sizeof(byte *))
    tBox frm[DIM];
    tTile ico[DIM];
    tButton btn[DIM];
    byte wdt, hgt;
    word x, y;
    const card pal[palBtnEnd] = { clrBgd, clrBgd, clrBgd };

    trcS("testDrwButtons: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    x = 4;
    y = 4;
    for (idx = 0; idx < DIM; idx++) {
        wdt = (byte) bmpWdt(img[idx]);
        hgt = (byte) bmpHgt(img[idx]);
        ret =
            drwBoxInit(&frm[idx], (byte) (2 + wdt + 2), (byte) (2 + hgt + 2),
                       clrFgd);
        CHECK(ret >= 0, lblKO);
        ret = drwTileInit(&ico[idx], bmpDat(img[idx]), wdt, hgt, -255);
        CHECK(ret >= 0, lblKO);
        ret =
            drwButtonInit(&btn[idx], &frm[idx], CARDHL(0, WORDHL(1, 1)), 0,
                          &ico[idx], pal);
        CHECK(ret >= 0, lblKO);
        ret = drwButtonDraw(&btn[idx], x, y);
        CHECK(ret >= 0, lblKO);
        y += 4 + hgt + 4;
    }

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(42) {
        ret = sysRand() % DIM;
        for (idx = 0; idx < DIM; idx++)
            btnState(&btn[idx], 'n');
        btnState(&btn[ret], 'f');
        btnPush(&btn[ret]);
        if(kbdKey())
            break;
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testDrwButtons: End\n");
    return 1;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0165(void) {
    int ret;

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(testCnvColorBars);
    RUN(testCnvColorTile);
    RUN(testCnvColorImage);
    RUN(testDrwButtons);

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    cnvStop();
    prtStop();
    dspStop();
}
#else
void tcab0165(void) {
}
#endif
