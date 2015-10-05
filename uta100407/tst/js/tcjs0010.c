#include <string.h>
#include "pri.h"
#include "tst.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unicapt.h>
#include <optic.h>
#include <opfont.h>

static uint32 hHmi = 0;
#define CHK CHECK(ret>=0,lblKO)
static hmiOutputConfig_t config;
static GC* pGC;
static Canvas localCanvas;
static Bitmap* pLocalBitmap;
static Dimen dw;
static Dimen dh;

static int SetConfig()
{
	if ( (hmiOutputConfigRead(hHmi, &config) == RET_OK) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_WIDTH) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_HEIGHT) 
                && (config.fields & HMI_OUTPUT_CONFIG_WINDOW_GC) ) 
    { 
		pGC = config.wgraphicContext;
		dw = (Dimen)config.wwidth; 
        dh = (Dimen)config.wheight; 
		pLocalBitmap = (Bitmap*)malloc(dw / 8 * dh); 
		if (pLocalBitmap) {
			

			hmiOpDefineCanvas(&localCanvas, dw, dh, 2,  
                                pLocalBitmap); 
						
                       
            hmiOpClearCanvas(&localCanvas, pGC);

		}

	}
	return RET_OK;
}

static int DrawLineHorizontal(int xPos, int yPos, int width)
{
	hmiOpDrawLine(&localCanvas, pGC, (Coord)(xPos), (Coord)yPos, (Coord)(xPos+width), (Coord)yPos);
	return RET_OK;
}

static int DrawLineVertical(int xPos, int yPos, int height)
{
	hmiOpDrawLine(&localCanvas, pGC, (Coord)xPos, (Coord)(yPos), (Coord)xPos, (Coord)(yPos+height)); 
	return RET_OK;
}

static int DrawBox(int xPos, int yPos, int width, int height)                                                   
{

		DrawLineHorizontal(xPos, yPos, width);
		DrawLineVertical(xPos, yPos, height);
		DrawLineHorizontal(xPos,yPos+height, width);
		DrawLineVertical(xPos+width, yPos,height);       
        return RET_OK;

}
static int DrawFillBox(int xPos, int yPos, int width, int height)
{
	hmiOpDrawRect(&localCanvas, pGC, (Coord)xPos, (Coord)yPos, (Coord)(width+1), (Coord)(height+1));
	return RET_OK;
}
static void DrawDisplay(int iFreememory)
{
	hmiGraphicDataWrite(hHmi, pLocalBitmap, 
                0, // x
                0, // y
                dw, //  128, //                  
                dh); //64); //
	if(iFreememory)
	free(pLocalBitmap);
}


static int DisplayText(int loc,int y,const char *str,int rev) {
    int ret;
    char buf[dspW + 1];

    VERIFY(str);
    //VERIFY((loc & 0x0F) < dspH);
    trcFN("dspLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    /*if(!hmiStarted) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }*/
#define FONT1 HMI_INTERNAL_FONT_1
#define FONT2 HMI_INTERNAL_FONT_2

    memcpy(buf, str, dspW);
    /*buf[dspW] = 0;              ////truncate the string if it is too long
    if(loc & 0x20) {            //right justified
        byte len;

        len = (byte) strlen(buf);
        if(len < dspW) {
            memmove(buf + dspW - len, buf, len);
            memset(buf, ' ', dspW - len);
        }
    }*/

    /*if(loc & 0x40) {            //bold font
        ret = hmiADFont(hHmi, FONT2);
    } else {                    //normal font
        ret = hmiADFont(hHmi, FONT1);
    }
    CHECK(ret == RET_OK, lblKO);*/
	ret = hmiADFont(hHmi, FONT1);

    if(rev) {            //reversed mode
        //hmiOutputConfig_t cfg;

        //read the configuration to modify it
        //ret = hmiOutputConfigRead(hHmi, &config);
        //CHECK(ret == RET_OK, lblKO);
//#ifdef jai
        //configure reversed mode
        ret = OpSetForeground(config.wgraphicContext, 0);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
        ret = OpSetBackground(config.wgraphicContext, 1);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);

        //clear line and display text
        //loc &= 0x0F;
        //ret = hmiADClearLine(hHmi, (uint16) loc);
        //CHECK(ret == RET_OK, lblKO);
        ret = hmiADDisplayText(hHmi, loc|HMI_IN_PIXELS, y|HMI_IN_PIXELS, buf);
        CHECK(ret == RET_OK, lblKO);

        //restore default normal mode
        ret = OpSetForeground(config.wgraphicContext, 1);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
        ret = OpSetBackground(config.wgraphicContext, 0);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
//#endif
/*		hmiADFont(hHmi, HMI_INTERNAL_FONT_3); // Small caraters
        hmiADSetForegroundColor(hHmi,HMI_COLOR_WHITE);
        hmiADSetBackgroundColor(hHmi,HMI_COLOR_BLACK);
        hmiADCESSet(hHmi, HMI_CES_UTF8);
        hmiADDisplayText(hHmi, loc | HMI_IN_PIXELS, y | HMI_IN_PIXELS, buf);
        hmiADSetForegroundColor(hHmi,HMI_COLOR_BLACK);
        hmiADSetBackgroundColor(hHmi,HMI_COLOR_WHITE);*/

       
    } else {
        //loc &= 0x0F;
        //just clear line and display text
       // ret = hmiADClearLine(hHmi, (uint16) loc);
       // CHECK(ret == RET_OK, lblKO);
        ret = hmiADDisplayText(hHmi, loc|HMI_IN_PIXELS, y|HMI_IN_PIXELS, buf);
        CHECK(ret == RET_OK, lblKO);
    }

    //restore normal font
    ret = hmiADFont(hHmi, HMI_INTERNAL_FONT_1);
    CHECK(ret == RET_OK, lblKO);

    return strlen(buf);
  lblKO:
    return -1;
}

static void tcjs0005(void)
{
	SetConfig();
	
	DrawLineHorizontal(1,20,235);

	DrawDisplay(1);

	DisplayText(0,70,"HORIZONTAL LINE",0);


}

static void tcjs0006(void)
{
	SetConfig();
	
	DrawLineVertical(120,30,200);

	DrawDisplay(1);

	DisplayText(0,70,"VERTICAL LINE",0);


}

static void tcjs0007(void)
{
	SetConfig();
	
	DrawBox(55,50,25+100,35+100);

	DrawDisplay(1);

	DisplayText(0,110,"BOX",0);


}

static void tcjs0008(void)
{
	SetConfig();
	
	DrawFillBox(55,50,25+100,35+100);

	DrawDisplay(1);

	DisplayText(0,60,"FILLED BOX",0);


}

static void tcjs0009(void)
{
	SetConfig();
	
	
	DrawBox(20,185,50,30);

	DrawDisplay(0);

	DrawFillBox(180,185,50,30);
	
	DrawDisplay(1);
	
	DisplayText(0,85,"BUTTONS",0);

	DisplayText(193,37,"OK",0);
	DisplayText(193,201,"OK",1);
	
	


}

#define RSP_LEN 1024
typedef struct{ int16 sta; uint16 len; char buf[RSP_LEN];}tResponse;

static int getKey(void){
    int ret;
    tResponse rsp;
    ret= hmiKeyReq(hHmi);
    while(ret==RET_RUNNING) ret= hmiResultGet(hHmi,sizeof(rsp),&rsp);
    hmiCancel(hHmi);
    if(ret==RET_OK) ret= rsp.buf[0]; else ret= -1;
    return ret;
}
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
//Test Case To use the graphical items
void tcjs0010(void){
	//dspStart();
	prtStart();
    hmiOpen("DEFAULT", &hHmi);
	
	tcjs0005();
	tmrPause(2);
	tcjs0006();
	tmrPause(2);
	tcjs0007();
	tmrPause(2);
	tcjs0008();
	tmrPause(2);
	tcjs0009();
	tmrPause(3);

	SampleScreen();
	
	tmrPause(3);	

	goto lblEnd;

//lblKO:	
	//dspLS(0,"KO!");
lblEnd:	
	//dspStop();
	hmiClose(hHmi);
	prtStop();

}

