#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __CANVAS__
static int testHor(void) {
    int ret;

    trcS("testHor: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(1, 20, 120, 20, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "HORIZONTAL LINE");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testHor: End\n");
    tmrPause(2);
    return 1;
}

static int testVer(void) {
    int ret;

    trcS("testVer: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(20, 20, 20, 40, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "VERTICAL LINE");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testVer: End\n");
    tmrPause(2);
    return 1;
}

static int drawBox(word x1, word y1, word x2, word y2) {
    int ret;

    ret = cnvLine(x1, y1, x2, y1, 1);   //top
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(x2, y1, x2, y2, 1);   //right
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(x2, y2, x1, y2, 1);   //bottom
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(x1, y2, x1, y1, 1);   //left
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return 1;
}

static int testBox(void) {
    int ret;

    trcS("testBox: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = drawBox(25, 20, 50, 55);
    CHECK(ret >= 0, lblKO);

    /*
       ret= cnvLine(25,20,50,20,1);
       CHECK(ret>=0,lblKO);

       ret= cnvLine(50,20,50,55,1);
       CHECK(ret>=0,lblKO);

       ret= cnvLine(50,55,25,55,1);
       CHECK(ret>=0,lblKO);

       ret= cnvLine(25,55,25,20,1);
       CHECK(ret>=0,lblKO);
     */

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "    BOX");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testBox: End\n");
    tmrPause(2);
    return 1;
}

static int testFilledBox(void) {
    int ret;

    trcS("testFilledBox: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = cnvBar(25, 20, 50, 55, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "FILLED BOX");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testFilledBox: End\n");
    tmrPause(2);
    return 1;
}

static int testButtons(void) {
    int ret;

    trcS("testButtons: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = cnvBar(80, 43, 120, 63, 1);
    CHECK(ret >= 0, lblKO);

    ret = drawBox(20, 43, 60, 63);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(93, 63, "OK", 1, 0);  //inverted
    CHECK(ret >= 0, lblKO);

    ret = cnvText(33, 63, "OK", 1, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "BUTTONS");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testButtons: End\n");
    tmrPause(2);
    return 1;
}

static int testScreen(void) {
    int ret;
    typedef struct {
        word x1;
        word y1;
        word x2;
        word y2;
    } tBox;
    tBox box[5];
    word x, y, w, h;
    byte i;
    char buf[2 + 1];

    trcS("testScreen: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    x = 3;
    y = 19;
    w = 20;
    h = 20;
    for (i = 0; i < 5; i++) {
        box[i].x1 = x;
        box[i].y1 = y;
        box[i].x2 = x + w;
        box[i].y2 = y + h;
        x += w + 5;
    }

    ret = cnvLine(1, 16, 127, 16, 1);
    CHECK(ret >= 0, lblKO);

    for (i = 0; i < 5; i++)
        drawBox(box[i].x1, box[i].y1, box[i].x2, box[i].y2);

    ret = cnvBar(7, 43, 37, 61, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvBar(90, 43, 120, 61, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(10, 45 + 16, "CAN", 1, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(98, 45 + 16, "OK", 1, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(0, 16, " SAMPLE SCREEN", 1, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    for (i = 0; i < 5; i++) {
        memset(buf, 0, sizeof(buf));
        while(!kbdKey());
        kbdStop();
        kbdStart(1);
        num2dec(buf, (card) (i + 10), 2);
        ret =
            cnvText((word) (box[i].x1 + 3), (word) (box[i].y2 - 3), buf, 1, 1);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    trcS("testScreen: End\n");
    kbdStop();
    tmrPause(2);
    return 1;
}

void tcab0118(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = cnvStart();
    CHECK(ret >= 0, lblKO);

    ret = testHor();
    CHECK(ret >= 0, lblKO);

    ret = testVer();
    CHECK(ret >= 0, lblKO);

    ret = testBox();
    CHECK(ret >= 0, lblKO);

    ret = testFilledBox();
    CHECK(ret >= 0, lblKO);

    ret = testButtons();
    CHECK(ret >= 0, lblKO);

    ret = testScreen();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    cnvStop();
    prtStop();
    kbdStop();
    dspStop();
}
#else
void tcab0118(void) {
}
#endif
