/** \file
 * Unitary test case tcab0117.
 * Functions testing:
 * \sa
 *  - cnvStart()
 *  - cnvStop()
 *  - cnvDraw()
 *  - cnvClear()
 *  - cnvPixel()
 *  - cnvLine()
 *  - cnvBar()
 *  - cnvText()
 *  - cnvTextSize()
 *  - cnvTile()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0117.c $
 *
 * $Id: tcab0117.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __CANVAS__
static void dly(void) {
    tmrStart(0, 1);
    while(tmrGet(0));
    tmrStop(0);
}

static int testCnvPixel1(void) {    //random pixels
    int ret;
    word x, y;
    word c;

    trcS("testCnvPixel1: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    while(42) {
        x = (word) sysRand() % cnvW;
        y = (word) sysRand() % cnvH;
        c = (word) sysRand() % cnvP;

        ret = cnvPixel(x, y, c);
        CHECK(ret >= 0, lblKO);

        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);

        if(kbdKey())
            break;

        //dly();
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvPixel1: End\n");
    return 1;
}

static int testCnvLine1(void) { //random lines
    int ret;
    word x1, y1;
    word x2, y2;
    word clr;

    trcS("testCnvLine1: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    x1 = (word) sysRand() % cnvW;
    y1 = (word) sysRand() % cnvH;

    while(42) {
        x2 = (word) sysRand() % cnvW;
        y2 = (word) sysRand() % cnvH;
        clr = (word) sysRand() % cnvP;

        ret = cnvLine(x1, y1, x2, y2, clr);
        CHECK(ret >= 0, lblKO);

        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);

        if(kbdKey())
            break;

        x1 = x2;
        y1 = y2;

        dly();
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvLine1: End\n");
    return 1;
}

static int testCnvLine2(void) { //rotating lines
    int ret;
    word x1, y1;
    word x2, y2;
    word dx, dy;
    word clr;
    word idx;

    trcS("testCnvLine2: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    dx = cnvW / 16;
    dy = cnvH / 16;

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    clr = __FGD__;
    while(42) {
        y1 = 0;
        x2 = cnvW - 1;
        for (idx = 1; idx < 16; idx++) {
            x1 = idx * dx;
            y2 = idx * dy;

            ret = cnvLine(x1, y1, x2, y2, clr);
            CHECK(ret >= 0, lblKO);

            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);

            dly();
        }

        x1 = cnvW - 1;
        y2 = cnvH - 1;
        for (idx = 1; idx < 16; idx++) {
            y1 = idx * dy;
            x2 = cnvW - idx * dx;

            ret = cnvLine(x1, y1, x2, y2, clr);
            CHECK(ret >= 0, lblKO);

            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);

            dly();
        }

        y1 = cnvH - 1;
        x2 = 0;
        for (idx = 1; idx < 16; idx++) {
            x1 = cnvW - idx * dx;
            y2 = cnvH - idx * dy;

            ret = cnvLine(x1, y1, x2, y2, clr);
            CHECK(ret >= 0, lblKO);

            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);

            dly();
        }

        x1 = 0;
        y2 = 0;
        for (idx = 1; idx < 16; idx++) {
            y1 = cnvH - idx * dy;
            x2 = idx * dx;

            ret = cnvLine(x1, y1, x2, y2, clr);
            CHECK(ret >= 0, lblKO);

            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);

            dly();
        }

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);

        if(kbdKey())
            break;
        if(clr == __FGD__)
            clr = __BGD__;
        else
            clr = __FGD__;
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvLine2: End\n");
    return 1;
}

static int testCnvBar1(void) {  //random rectangles
    int ret;
    word x1, y1;
    word x2, y2;
    word clr;

    trcS("testCnvBar1: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    while(42) {
        x1 = (word) sysRand() % cnvW;
        y1 = (word) sysRand() % cnvH;
        //x2= x1; while(x2==x1 || x2==x1+1 || x1==x2+1) x2= (word)sysRand()%cnvW;
        //y2= y1; while(y2==y1 || y2==y1+1 || y1==y2+1) y2= (word)sysRand()%cnvH;
        x2 = (word) sysRand() % cnvW;
        y2 = (word) sysRand() % cnvH;
        clr = (word) sysRand() % cnvP;
        ret = (word) sysRand() % 4;

        ret = cnvBar(x1, y1, x2, y2, clr);
        CHECK(ret >= 0, lblKO);

        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);

        if(kbdKey())
            break;

        dly();
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvBar1: End\n");
    return 1;
}

static int testCnvBar2(void) {  //random moving rectangle
    int ret;
    word x1, y1;
    word x2, y2;
    word clr;
    char k;
    const char *s = "123456789A@";
    char buf[prtW + 1];

    trcS("testCnvBar2: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    x1 = cnvW / 2 - 1;
    y1 = cnvH / 2 - 1;
    x2 = cnvW / 2 + 1;
    y2 = cnvH / 2 + 1;
    clr = 1;
    memset(buf, 0, prtW + 1);
    while(42) {
        clr = (word) (sysRand() % cnvP);

        ret = cnvBar(x1, y1, x2, y2, clr);
        CHECK(ret >= 0, lblKO);

        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        dly();

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);

        k = kbdKey();
        if(k == kbdANN)
            break;
        if(!k) {
            ret = sysRand();
            ret %= 11;
            k = s[ret];
        } else {
            kbdStop();
            kbdStart(0);
        }
        buf[strlen(buf)] = k;
        if(strlen(buf) >= prtW) {
//          prtS(buf);
            memset(buf, 0, prtW + 1);
        }
        switch (k) {
          case '4':            //left
              if(!x1)
                  continue;
              x1--;
              x2--;
              break;
          case '6':            //right
              if(x2 >= cnvW - 1)
                  continue;
              x1++;
              x2++;
              break;
          case '8':            //up
              if(!y1)
                  continue;
              y1--;
              y2--;
              break;
          case '2':            //down
              if(y2 >= cnvH - 1)
                  continue;
              y1++;
              y2++;
              break;
          case '7':            //NW
              if(!x1)
                  continue;
              if(!y1)
                  continue;
              x1--;
              x2--;
              y1--;
              y2--;
              break;
          case '9':            //NE
              if(x2 >= cnvW - 1)
                  continue;
              if(!y1)
                  continue;
              x1++;
              x2++;
              y1--;
              y2--;
              break;
          case '1':            //SW
              if(!x1)
                  continue;
              if(y2 >= cnvH - 1)
                  continue;
              x1--;
              x2--;
              y1++;
              y2++;
              break;
          case '3':            //SE
              if(x2 >= cnvW - 1)
                  continue;
              if(y2 >= cnvH - 1)
                  continue;
              x1++;
              x2++;
              y1++;
              y2++;
              break;
          case 'A':            //grow
              if(!x1)
                  continue;
              if(x2 >= cnvW - 1)
                  continue;
              if(!y1)
                  continue;
              if(y2 >= cnvH - 1)
                  continue;
              x1--;
              x2++;
              y1--;
              y2++;
              break;
          case '@':            //shrink
              if(x1 + 1 >= x2 - 1)
                  continue;
              if(y1 + 1 >= y2 - 1)
                  continue;
              x1++;
              x2--;
              y1++;
              y2--;
              break;
          case '5':            //blink
              clr = 1 - clr;
              break;
        }
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvBar2: End\n");
    return 1;
}

static int testCnvText(void) {  //text drawing
    int ret;
    byte hgt = 16;
    int x[4], y[4];
    int dx[4], dy[4];
    byte i;
    card wh;
    char *s[4] = {
        "Requirements",
        "Everything",
        "For nothing",
        "Yesterday"
    };
#ifdef __COLOR12__
    card c0 = 0x0FF0000F;       //blue on yellow
    card c1 = 0x0F000FFF;       //white on red
#else
    card c0 = __FGD__;
    card c1 = __BGD__;
#endif

    trcS("testCnvText: Beg\n");

    for (i = 0; i < 4; i++) {
        x[i] = 0;
        y[i] = hgt * (i + 1) - 1;
        dx[i] = 0;
        dy[i] = 0;
    }

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    wh = cnvTextSize(s[0], 1);
    while(42) {
        ret = cnvClear();
        CHECK(ret >= 0, lblKO);

        for (i = 0; i < 4; i++) {
            if(!i) {
                ret =
                    cnvBar((word) x[i], (word) y[i], (word) (x[i] + HWORD(wh)),
                           (word) (y[i] - LWORD(wh) + 1), __FGD__);
                CHECK(ret >= 0, lblKO);
                ret = cnvText((word) x[i], (word) y[i], s[i], 1, c0);
                CHECK(ret >= 0, lblKO);
            } else {
                ret = cnvText((word) x[i], (word) y[i], s[i], 0, c1);
                CHECK(ret >= 0, lblKO);
            }

            x[i] += dx[i];
            if(x[i] < 0) {
                x[i] -= dx[i];
                dx[i] = -dx[i];
            } else if(x[i] > cnvW - 1) {
                x[i] -= dx[i];
                dx[i] = -dx[i];
            }

            y[i] += dy[i];
            if(y[i] < 0) {
                y[i] -= dy[i];
                dy[i] = -dy[i];
            } else if(y[i] > cnvH - 1) {
                y[i] -= dy[i];
                dy[i] = -dy[i];
            }
        }

        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        i = (byte) sysRand() % 3;
        i++;
        ret = sysRand() % 4;
        switch (ret) {
          case 0:
              if(dx[i] > -5)
                  dx[i]--;
              break;
          case 1:
              if(dy[i] > -5)
                  dy[i]--;
              break;
          case 2:
              if(dx[i] < 5)
                  dx[i]++;
              break;
          case 3:
              if(dy[i] < 5)
                  dy[i]++;
              break;
          default:
              break;
        }

        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);

        if(kbdKey())
            break;

        dly();
        dly();
        dly();
    }
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvText: End\n");
    return 1;
}

static int testCnvTile0(void) { //various bitmaps
    int ret;
    word x, y;
    card c;
    static const unsigned char dck0[] = {   //DOCKED0_TILE H=10 W=8
        0x3C,                   //..****.. 0, 0, 1, 1, 1, 1, 0, 0,
        0x3C,                   //..****.. 0, 0, 1, 1, 1, 1, 0, 0,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1,
        0x7E,                   //.******. 0, 1, 1, 1, 1, 1, 1, 0,
        0x3C,                   //..****.. 0, 0, 1, 1, 1, 1, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0, 0,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1 
    };
    static const unsigned char dck1[] = {   //DOCKED1_TILE H=10 W=8
        0xC3,                   //**....** 1, 1, 0, 0, 0, 0, 1, 1,
        0xC3,                   //**....** 1, 1, 0, 0, 0, 0, 1, 1,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x81,                   //*......* 1, 0, 0, 0, 0, 0, 0, 1,
        0xC3,                   //**....** 1, 1, 0, 0, 0, 0, 1, 1,
        0xE7,                   //***..*** 1, 1, 1, 0, 0, 1, 1, 1,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1,
        0xFF,                   //******** 1, 1, 1, 1, 1, 1, 1, 1
    };
    static const unsigned char lq0[] = {    //LQ0_TILE H=10 W=8
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
    };
    static const unsigned char lq1[] = {    //LQ1_TILE H=10 W=8
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0xC0,                   //**...... 1, 1, 0, 0, 0, 0, 0, 0,
        0xC0,                   //**...... 1, 1, 0, 0, 0, 0, 0, 0,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1 
    };
    static const unsigned char lq2[] = {    //LQ2_TILE H=10 W=8
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0, 0,
        0xD8,                   //**.**... 1, 1, 0, 1, 1, 0, 0, 0,
        0xD8,                   //**.**... 1, 1, 0, 1, 1, 0, 0, 0,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1 
    };
    static const unsigned char lq3[] = {    //LQ3_TILE H=10 W=8
        0x03,                   //......** 0, 0, 0, 0, 0, 0, 1, 1,
        0x03,                   //......** 0, 0, 0, 0, 0, 0, 1, 1,
        0x03,                   //......** 0, 0, 0, 0, 0, 0, 1, 1,
        0x1B,                   //...**.** 0, 0, 0, 1, 1, 0, 1, 1,
        0x1B,                   //...**.** 0, 0, 0, 1, 1, 0, 1, 1,
        0x1B,                   //...**.** 0, 0, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1,
        0xDB,                   //**.**.** 1, 1, 0, 1, 1, 0, 1, 1 
    };
    static const unsigned char trp[] = {    //TRANSPARENT_TILE H=10 W=8
        0x7E,                   //.******. 0, 1, 1, 1, 1, 1, 1, 0,
        0x81,                   //*......* 1, 0, 0, 0, 0, 0, 0, 1,
        0xBD,                   //*.****.* 1, 0, 1, 1, 1, 1, 0, 1,
        0xBD,                   //*.****.* 1, 0, 1, 1, 1, 1, 0, 1,
        0x99,                   //*..**..* 1, 0, 0, 1, 1, 0, 0, 1,
        0x99,                   //*..**..* 1, 0, 0, 1, 1, 0, 0, 1,
        0x99,                   //*..**..* 1, 0, 0, 1, 1, 0, 0, 1,
        0x99,                   //*..**..* 1, 0, 0, 1, 1, 0, 0, 1,
        0x81,                   //*......* 1, 0, 0, 0, 0, 0, 0, 1,
        0x7E,                   //.******. 0, 1, 1, 1, 1, 1, 1, 0,
    };
    static const unsigned char ctc[] = {    //CONTACT_TILE H=10 W=8
        0x7E,                   //.******. 0, 1, 1, 1, 1, 1, 1, 0,
        0x81,                   //*......* 1, 0, 0, 0, 0, 0, 0, 1,
        0x9D,                   //*..***.* 1, 0, 0, 1, 1, 1, 0, 1,
        0xBD,                   //*.****.* 1, 0, 1, 1, 1, 1, 0, 1,
        0xB1,                   //*.**...* 1, 0, 1, 1, 0, 0, 0, 1,
        0xB1,                   //*.**...* 1, 0, 1, 1, 0, 0, 0, 1,
        0xBD,                   //*.****.* 1, 0, 1, 1, 1, 1, 0, 1,
        0x9D,                   //*..***.* 1, 0, 0, 1, 1, 1, 0, 1,
        0x81,                   //*......* 1, 0, 0, 0, 0, 0, 0, 1,
        0x7E,                   //.******. 0, 1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char lk0[] = {    //LOCKED0_TILE H=10 W=7
        0x02,                   //......*. 0, 0, 0, 0, 0, 0, 1,
        0x08,                   //....*... 0, 0, 0, 0, 1, 0, 0,
        0xA4,                   //*.*..*.. 1, 0, 1, 0, 0, 1, 0,
        0x90,                   //*..*.... 1, 0, 0, 1, 0, 0, 0,
        0x48,                   //*...*... 0, 1, 0, 0, 1, 0, 0,
        0x60,                   //0**..... 0, 1, 1, 0, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0xE0,                   //***..... 1, 1, 1, 0, 0, 0, 0 
    };
    static const unsigned char lk1[] = {    //LOCKED1_TILE H=10 W=7
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0,
        0x80,                   //*....... 1, 0, 0, 0, 0, 0, 0,
        0x80,                   //*....... 1, 0, 0, 0, 0, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0x60,                   //0**..... 0, 1, 1, 0, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0xE0,                   //***..... 1, 1, 1, 0, 0, 0, 0 
    };
    static const unsigned char lk2[] = {    //LOCKED2_TILE H=10 W=7
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0,
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0,
        0xA0,                   //*.*..... 1, 0, 1, 0, 0, 0, 0,
        0x90,                   //*..*.... 1, 0, 0, 1, 0, 0, 0,
        0x48,                   //.*..*... 0, 1, 0, 0, 1, 0, 0,
        0x60,                   //0**..... 0, 1, 1, 0, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0xE0,                   //***..... 1, 1, 1, 0, 0, 0, 0 
    };
    static const unsigned char lk3[] = {    //LOCKED3_TILE H=10 W=7
        0x00,                   //........ 0, 0, 0, 0, 0, 0, 0,
        0x08,                   //....*... 0, 0, 0, 0, 1, 0, 0,
        0xA4,                   //*.*...*. 1, 0, 1, 0, 0, 1, 0,
        0x90,                   //*..*.... 1, 0, 0, 1, 0, 0, 0,
        0x48,                   //.*..*... 0, 1, 0, 0, 1, 0, 0,
        0x60,                   //0**..... 0, 1, 1, 0, 0, 0, 0,
        0x18,                   //...**... 0, 0, 0, 1, 1, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0x40,                   //.*...... 0, 1, 0, 0, 0, 0, 0,
        0xE0,                   //***..... 1, 1, 1, 0, 0, 0, 0 
    };
    static const unsigned char bat0[] = {   //BAT0_TILE H=10 W=16
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0x80, 0x03,             //*....... ......** 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x03,             //*....... ......** 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x03,             //*....... ......** 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x03,             //*....... ......** 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x03,             //*....... ......** 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x03,             //*....... ......** 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char bat1[] = {   //BAT1_TILE H=10 W=16
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xB0, 0x03,             //*.**.... ......** 1, 0, 1, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB0, 0x03,             //*.**.... ......** 1, 0, 1, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB0, 0x03,             //*.**.... ......** 1, 0, 1, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB0, 0x03,             //*.**.... ......** 1, 0, 1, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB0, 0x03,             //*.**.... ......** 1, 0, 1, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB0, 0x03,             //*.**.... ......** 1, 0, 1, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char bat2[] = {   //BAT2_TILE H=10 W=16
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xB6, 0x03,             //*.**.**. ......** 1, 0, 1, 1, 0, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB6, 0x03,             //*.**.**. ......** 1, 0, 1, 1, 0, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB6, 0x03,             //*.**.**. ......** 1, 0, 1, 1, 0, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB6, 0x03,             //*.**.**. ......** 1, 0, 1, 1, 0, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB6, 0x03,             //*.**.**. ......** 1, 0, 1, 1, 0, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0xB6, 0x03,             //*.**.**. ......** 1, 0, 1, 1, 0, 1, 1, 0,  0, 0, 0, 0, 0, 0, 1, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char bat3[] = {   //BAT3_TILE H=10 W=16
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xB6, 0xC3,             //*.**.**. **....** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 0, 0, 0, 1, 1,
        0xB6, 0xC3,             //*.**.**. **....** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 0, 0, 0, 1, 1,
        0xB6, 0xC3,             //*.**.**. **....** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 0, 0, 0, 1, 1,
        0xB6, 0xC3,             //*.**.**. **....** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 0, 0, 0, 1, 1,
        0xB6, 0xC3,             //*.**.**. **....** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 0, 0, 0, 1, 1,
        0xB6, 0xC3,             //*.**.**. **....** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 0, 0, 0, 1, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char bat4[] = {   //BAT4_TILE H=10 W=16
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xB6, 0xDB,             //*.**.**. **.**.** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 1, 1, 0, 1, 1,
        0xB6, 0xDB,             //*.**.**. **.**.** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 1, 1, 0, 1, 1,
        0xB6, 0xDB,             //*.**.**. **.**.** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 1, 1, 0, 1, 1,
        0xB6, 0xDB,             //*.**.**. **.**.** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 1, 1, 0, 1, 1,
        0xB6, 0xDB,             //*.**.**. **.**.** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 1, 1, 0, 1, 1,
        0xB6, 0xDB,             //*.**.**. **.**.** 1, 0, 1, 1, 0, 1, 1, 0,  1, 1, 0, 1, 1, 0, 1, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0,
        0xFF, 0xFE,             //******** *******. 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char eth[] = {    //ETH_TILE H=10 W=15
        0x00, 0x00,             //........ ........ 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0,
        0x7F, 0xFC,             //.******* ******.. 0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1,
        0xBD, 0xAA,             //*.***.** *.*.*.*. 1, 0, 1, 1, 1, 0, 1, 1,  1, 0, 1, 0, 1, 0, 1,
        0xA1, 0x2A,             //*.*....* ..*.*.*. 1, 0, 1, 0, 0, 0, 0, 1,  0, 0, 1, 0, 1, 0, 1,
        0xB1, 0x3A,             //*.**...* ..***.*. 1, 0, 1, 1, 0, 0, 0, 1,  0, 0, 1, 1, 1, 0, 1,
        0xA1, 0x2A,             //*.*....* ..*.*.*. 1, 0, 1, 0, 0, 0, 0, 1,  0, 0, 1, 0, 1, 0, 1,
        0xB9, 0x2A,             //*.***..* ..*.*.*. 1, 0, 1, 1, 1, 0, 0, 1,  0, 0, 1, 0, 1, 0, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1,
        0xFF, 0xFE,             //******** *******. 0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char wifi[] = {   //WIFI_TILE H=10 W=17
        0x00, 0x00, 0x00,       //........ ........ . 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0,
        0x7F, 0xFF, 0x00,       //.******* ******** . 0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  0,
        0x80, 0x00, 0x80,       //*....... ........ * 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  1,
        0xA2, 0xBA, 0x80,       //*.*...*. *.***.*. * 1, 0, 1, 0, 0, 0, 1, 0,  1, 0, 1, 1, 1, 0, 1, 0,  1,
        0xA2, 0x40, 0x80,       //*.*...*. ..*..... * 1, 0, 1, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 0, 0,  1,
        0xAA, 0xB2, 0x80,       //*.*.*.*. *.**..*. * 1, 0, 1, 0, 1, 0, 1, 0,  1, 0, 1, 1, 0, 0, 1, 0,  1,
        0xB6, 0xA2, 0x80,       //*.**.**. *.*...*. * 1, 0, 1, 1, 0, 1, 1, 0,  1, 0, 1, 0, 0, 0, 1, 0,  1,
        0xA2, 0xA2, 0x80,       //*.*...*. *.*...*. * 1, 0, 1, 0, 0, 0, 1, 0,  1, 0, 1, 0, 0, 0, 1, 0,  1,
        0x80, 0x00, 0x80,       //*....... ........ * 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  1,
        0x7F, 0xFF, 0x00,       //.******* ******** . 0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  0,
    };
    static const unsigned char ppp[] = {    //PPP_TILE H=10 W=15
        0x00, 0x00,             //........ ........ 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0,
        0x7F, 0xFC,             //.******* ******.. 0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 0,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1,
        0xBB, 0xBA,             //*.***.** *.***.*. 1, 0, 1, 1, 1, 0, 1, 1,  1, 0, 1, 1, 1, 0, 1,
        0xAA, 0xAA,             //*.*.*.*. *.*.*.*. 1, 0, 1, 0, 1, 0, 1, 0,  1, 0, 1, 0, 1, 0, 1,
        0xBB, 0xBA,             //*.***.** *.***.*. 1, 0, 1, 1, 1, 0, 1, 1,  1, 0, 1, 1, 1, 0, 1,
        0xA2, 0x22,             //*.*...*. ..*...*. 1, 0, 1, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 1,
        0xA2, 0x22,             //*.*...*. ..*...*. 1, 0, 1, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 1,
        0x80, 0x02,             //*....... ......*. 1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1,
        0xFF, 0xFE,             //******** *******. 0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 0 
    };
    static const unsigned char alarm[] = {  //ALARM_TILE H=10 W=9
        0x00, 0x00,             //........ ........ 0, 0, 0, 0, 0, 0, 0, 0,  0,
        0x63, 0x00,             //.**...** ........ 0, 1, 1, 0, 0, 0, 1, 1,  0,
        0x9C, 0x80,             //*..***.. *....... 1, 0, 0, 1, 1, 1, 0, 0,  1,
        0xEB, 0x80,             //***.*.** *....... 1, 1, 1, 0, 1, 0, 1, 1,  1,
        0x49, 0x00,             //.*..*..* ........ 0, 1, 0, 0, 1, 0, 0, 1,  0,
        0x4D, 0x00,             //.*..**.* ........ 0, 1, 0, 0, 1, 1, 0, 1,  0,
        0x41, 0x00,             //.*.....* ........ 0, 1, 0, 0, 0, 0, 0, 1,  0,
        0x22, 0x00,             //..*...*. ........ 0, 0, 1, 0, 0, 0, 1, 0,  0,
        0x5D, 0x00,             //.*.***.* ........ 0, 1, 0, 1, 1, 1, 0, 1,  0,
        0xC1, 0x80,             //**.....* *....... 1, 1, 0, 0, 0, 0, 0, 1,  1 
    };

    trcS("testCnvTile0: Beg\n");

    c = __FGD__;
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    x = 0;
    y = 8;
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, dck0);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, dck1);
    CHECK(ret >= 0, lblKO);

    x = 0;
    y += 16;
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, lq0);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, lq1);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, lq2);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, lq3);
    CHECK(ret >= 0, lblKO);

    x = 0;
    y += 16;
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, trp);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 8, 10, c, ctc);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(!kbdKey())
        dspLight(100);
    kbdStop();
    cnvClear();

    x = 0;
    y = 8;
    x += 16;
    ret = cnvTile(x, y, 7, 10, c, lk0);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 7, 10, c, lk1);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 7, 10, c, lk2);
    CHECK(ret >= 0, lblKO);
    x += 16;
    ret = cnvTile(x, y, 7, 10, c, lk3);
    CHECK(ret >= 0, lblKO);

    x = 0;
    y += 16;
    x += 20;
    ret = cnvTile(x, y, 16, 10, c, bat0);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 16, 10, c, bat1);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 16, 10, c, bat2);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 16, 10, c, bat3);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 16, 10, c, bat4);
    CHECK(ret >= 0, lblKO);

    x = 0;
    y += 16;
    x += 20;
    ret = cnvTile(x, y, 15, 10, c, eth);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 17, 10, c, wifi);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 15, 10, c, ppp);
    CHECK(ret >= 0, lblKO);
    x += 20;
    ret = cnvTile(x, y, 9, 10, c, alarm);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(!kbdKey())
        dspLight(100);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvTile0: End\n");
    return 1;
}

static int testCnvTile1(void) { //random bitmap placing
    int ret;

#include "fazing.fnt"
    word x, y;
    word n;
    card c;
    byte h, w;
    const byte *ptr;

    trcS("testCnvTile1: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    c = __FGD__;
    while(42) {
        x = (word) sysRand() % (cnvW - 8);
        y = (word) sysRand() % (cnvH - 8);

        w = 27;
        h = 18;
        n = sizeof(gly) / sizeof(void *) - 1;
        n = (byte) sysRand() % n;
        ptr = gly[n];

        if(x + w < cnvW && y + h < cnvH) {
            ret = cnvTile(x, y, w, h, c, ptr);
            CHECK(ret >= 0, lblKO);

            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);
        }

        if(kbdKey())
            break;
        ret = dspLight(100);
        CHECK(ret >= 0, lblKO);
        dly();

        n = (byte) sysRand() % 8;
        if(n)
            continue;

        ret = cnvClear();
        CHECK(ret >= 0, lblKO);
    }
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvTile1: End\n");
    return 1;
}

static int showCenteredTile(const byte * bmp, word wdt, word hgt) {
    int ret;
    word x, y;

    VERIFY(wdt <= cnvW);
    VERIFY(hgt <= cnvH);

    x = (cnvW - wdt) / 2;
    y = (cnvH - hgt) / 2;
    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = cnvTile(x, y, wdt, hgt, __FGD__, bmp);
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    while(!kbdKey())
        dspLight(100);

    ret = 1;
    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:
    kbdStop();
    return 1;
}

static int testCnvTile2(void) { //bitmap display
    int ret;

#define __cardAmex__
#define __cardMC1__
#define __cardDC__
#define __cardMC2__
#define __cardVisa__
#define __logoSG__
#define __logoICICI__
#define __logoBORICA__
#define __logoCBAO__

#include "logo.fnt"
    trcS("testCnvTile2: Beg\n");

    ret = showCenteredTile(logoSG, 16 * 8, 48);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(logoICICI, 96, 48);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(logoBORICA, 96, 32);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(logoCBAO, 72, 54);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(cardVisa, 107, 61);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(cardAmex, 108, 61);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(cardMC1, 108, 62);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(cardMC2, 110, 62);
    CHECK(ret >= 0, lblKO);
    ret = showCenteredTile(cardDC, 63, 62);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    kbdStop();
    trcS("testCnvTile2: End\n");
    return 1;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0117(void) {
    int ret;

    trcS("tcab0117 Beg\n");

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(testCnvPixel1);
    RUN(testCnvBar1);
    RUN(testCnvBar2);
    RUN(testCnvLine1);
    RUN(testCnvLine2);
    RUN(testCnvText);

    RUN(testCnvTile0);
    RUN(testCnvTile1);
    RUN(testCnvTile2);

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0117 End\n");
}
#else
void tcab0117(void) {
}
#endif
