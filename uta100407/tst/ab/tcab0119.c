#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __CANVAS__
static int testHor(void) {
    int ret;

    trcS("testHor: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(1, 20, 235, 20, 1);
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

    ret = cnvLine(120, 30, 120, 200, 1);
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

    ret = drawBox(55, 50, 180, 185);
    CHECK(ret >= 0, lblKO);

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

    ret = cnvBar(55, 50, 180, 185, 1);
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

    ret = cnvBar(20, 185, 70, 215, 1);
    CHECK(ret >= 0, lblKO);

    ret = drawBox(180, 185, 230, 215);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(37, 209, "OK", 1, 0); //inverted
    CHECK(ret >= 0, lblKO);

    ret = cnvText(201, 209, "OK", 1, 1);
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
    tBox box[20];
    word x, y, w, h;
    byte i, j, k;
    char buf[2 + 1];

    trcS("testScreen: Beg\n");

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);

    x = 4;
    y = 50;
    w = 30;
    h = 30;
    for (i = 0; i < 18; i++) {
        box[i].x1 = x;
        box[i].y1 = y;
        box[i].x2 = x + w;
        box[i].y2 = y + h;
        x += w + 10;
        switch (i) {
          case 5:
              x = 4;
              y = 90;
              break;
          case 11:
              x = 4;
              y = 130;
              break;
          default:
              break;
        }

    }

    ret = cnvLine(0, 16, 239, 16, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvLine(0, 17, 239, 17, 1);
    CHECK(ret >= 0, lblKO);

    for (i = 0; i < 18; i++) {
        if((i < 6) || (i > 11)) {
            ret = drawBox(box[i].x1, box[i].y1, box[i].x2, box[i].y2);
            CHECK(ret >= 0, lblKO);
        } else {
            ret = cnvBar(box[i].x1, box[i].y1, box[i].x2, box[i].y2, 1);
            CHECK(ret >= 0, lblKO);
        }
    }

    ret = cnvBar(20, 200, 70, 230, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvBar(180, 200, 230, 230, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(35, 207 + 16, "CAN", 1, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(197, 207 + 16, "OK", 1, 0);
    CHECK(ret >= 0, lblKO);

    ret = cnvText(55, 16, " SAMPLE SCREEN", 1, 1);
    CHECK(ret >= 0, lblKO);

    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    for (i = 0; i < 3; i++) {
        memset(buf, 0, sizeof(buf));
        while(!kbdKey());
        kbdStop();
        kbdStart(1);
        for (j = 0; j < 6; j++) {
            num2dec(buf, (card) (i * 6 + j + 10), 2);
            k = (i == 1) ? 0 : 1;
            ret =
                cnvText((word) (box[i * 6 + j].x1 + 8),
                        (word) (box[i * 6 + j].y2 - 8), buf, 1, k);
            CHECK(ret >= 0, lblKO);
            ret = cnvDraw();
            CHECK(ret >= 0, lblKO);
        }
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

/*
static void SampleScreen()
{
	int x,y,height,width,i,j;
	int ret;

	typedef struct 
	{
		int BXpos;
		int BYpos;
		int width;
		int height;
	}BOX;

	BOX BOXES[20];

	char buf[100];

	memset(buf,0,sizeof(buf));

	
	x = 4;
	y = 50;
	width = 20+10;
	height = 20+10;
	ret = 0;

	for(i=0;i<19;i++)
    {
        
        BOXES[i].BXpos = x;
        BOXES[i].BYpos = y;
        BOXES[i].width = width;
        BOXES[i].height = height;
        
        x = x+20+10+10;

		if((i==5) || (i==11))
		{
			x = 4;
			
			y = 50+40;
			if(i == 11)
			{
				y = 50+40+40;
			}
		}
		
    }

	SetConfig();
	DrawLineHorizontal(0,16,240);
	DrawLineHorizontal(0,17,240);
	

	for(i=0;i<19;i++)
	{
		if((i<6) || (i>11))
			DrawBox(BOXES[i].BXpos,BOXES[i].BYpos,BOXES[i].width,BOXES[i].height);
		else
			DrawFillBox(BOXES[i].BXpos,BOXES[i].BYpos,BOXES[i].width,BOXES[i].height);

	}
	
		
	DrawFillBox(20,200,50,30);
	DrawFillBox(180,200,50,30);
	

	DrawDisplay(1);
	DisplayText(0,55,"SAMPLE SCREEN",0);

	DisplayText(207,35,"CAN",1);
	DisplayText(207,197,"OK",1);

	for(i=0;i<3;i++)
	{
		memset(buf,0,sizeof(buf));
		getKey();
		for(j=0;j<6;j++)
		{
			num2dec(buf,(card)(i*6+j+10),2);
			prtS(buf);

			if(i==1)
			{
				DisplayText(BOXES[i*6+j].BYpos+8,BOXES[i*6+j].BXpos+8,buf,1);
				continue;
			}
			
			DisplayText(BOXES[i*6+j].BYpos+8,BOXES[i*6+j].BXpos+8,buf,0);
			
		}
		
		//prtS(buf);
	}

}
*/

void tcab0119(void) {
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
void tcab0119(void) {
}
#endif
