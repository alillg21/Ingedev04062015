#include <string.h>
#include "sys.h"
#include "tst.h"

int printBar(byte *bmp,int height,int weight)
{
	int ret;
	ret = 0;

	//trcS("printBar = \n");
	//trcBAN(bmp, height * weight); crashes (to investigate)
   
	// PRINTING (only one line is generated : this saves memory.
	// If it is too slow, the whole bmp can be generated at once...)
	while (height--)
		ret = prtImg(bmp, 1, weight, "c1");
   
	CHECK(ret >= 0, lblKO);
	return 1;
   
lblKO:
	trcErr(ret);
	return -1;
}

//
// BAR DRAWING DEFINITIONS
#define BAR_WIDTH (1L)

// SEPARATOR DEFINITIONS
#define SEP_LEN (8L)

//WIDTH OF NARROW AND WIDE BAR

#define NARROWBAR_WIDTH		(2L) 
#define WIDEBAR_WIDTH		(4L)

typedef enum { SEP_BEG = 0, SEP_END } SEPARATORS; //THERE ARE ONLY TWO SEPARATORS FOR INTERLEAVED 2 OF 5 BARCODE

static const byte Inter2of5_SEP[2][SEP_LEN] = {
   { 1, 1, 0, 0, 1, 1, 0, 0 }, //BAR START
   { 1, 1, 1, 1, 0, 0, 1, 1 }, //BAR END  
};

/* PATTERN FOR INTERLEAVED 2of5 
0 NNWWN 
1 WNNNW 
2 NWNNW 
3 WWNNN 
4 NNWNW 
5 WNWNN 
6 NWWNN 
7 NNNWW 
8 WNNWN 
9 NWNWN 

WHERE N - NARROW BAR
	  W - WIDE BAR
*/
static const byte Inter2of5[10][5] = {
   { 0, 0, 1, 1, 0 },	//0
   { 1, 0, 0, 0, 1 },	//1
   { 0, 1, 0, 0, 1 },	//2
   { 1, 1, 0, 0, 0 },	//3
   { 0, 0, 1, 0, 1 },	//4
   { 1, 0, 1, 0, 0 },	//5
   { 0, 1, 1, 0, 0 },	//6
   { 0, 0, 0, 1, 1 },	//7
   { 1, 0, 0, 1, 0 },	//8
   { 0, 1, 0, 1, 0 },	//9
};


//FILLING THE SEPARATORS IN THE BARCODE
static void separator(SEPARATORS separatorType, byte **pBmp)
{
   int j, k;

   for (j = 0; j < SEP_LEN; j++)
   {
      for (k = 0; k < BAR_WIDTH ; k++, (*pBmp)++)
      {
         *(*pBmp) = Inter2of5_SEP[separatorType][j];
      }
   }
}

//FORMATTING THE BITMAP TO GET PRINTED

static int formatBmp1(byte *bmp1, byte *pBmp1)
{
   int i, j;
   byte *pBmp2;
   byte  tmp;

   for (i = 0, pBmp2 = bmp1; pBmp2 < pBmp1; i++)
   {
      tmp = 0;

	  for (j = 0; j < 8; j++)
      {
         tmp += *pBmp2++ * 1 << (7 - j);
      }
	  
      bmp1[i] = tmp;
   }

   return i;
}

//THIS IS THE FUNCTION TO BE CALLED TO PRINT INTERLEAVED 2 OF 5
// PrintCode is the buffer to be filled by numbers
//
int NewprintI2Of5(char *PrintCode,int iheight)
{
   int i, j, k;
   byte bmp[1000], *pBmp;
   int  codeLen;  
   int iWeight;
   int ret;

   iWeight = 0;
   ret = 0;
   
   memset(bmp, 0, sizeof(bmp));
   pBmp = bmp;

   codeLen = strlen(PrintCode); 
   trcFN("codeLen=%02d\n", codeLen);
   
   if (codeLen % 2) //CHECK ALWAYS FOR THE EVEN CODE
   {
	  trcS("ERROR! codeLen%2 = odd");
	  prtS("Invalid code!");
	  return -1;
   }

   if (codeLen > 24)
   {
	  trcS("ERROR! codeLen > 24");
	  prtS("Invalid code1!");
	  return -1;
   }

    separator(SEP_BEG, &pBmp);//FILLING WITH BARCODE START

	for (i = 0; i < codeLen; i += 2)
	{
		for (j = 0; j < 5; j++)
		{
			if (PrintCode[i] < '0' || PrintCode[i] > '9') //check for numeric data
			{
				prtS("Invalid code!");
				prtS("NUMERIC DATA ALLOWED");
				return -1;
			}
			
			if (PrintCode[i+1] < '0' || PrintCode[i+1] > '9') //check for numeric data
			{
				prtS("Invalid code!");
				prtS("NUMERIC DATA ALLOWED");
				return -1;
			}

			if (Inter2of5[PrintCode[i]-0x30][j] == 0)  
			{
				for (k = 0; k < NARROWBAR_WIDTH; k++) //FILLING WITH NARROW BAR
				{					
					*pBmp++ = 1;
				}
			}
			else
			{
				for (k = 0; k< WIDEBAR_WIDTH; k++)  //FILLING WITH NARROW BAR
				{
					*pBmp++ = 1;
				}
			}
			if (Inter2of5[PrintCode[i+1]-0x30][j] == 0)  
			{
				for (k = 0; k < NARROWBAR_WIDTH; k++) //FILLING WITH NARROW SPACE
				{
					*pBmp++ = 0;
				}
			}
			else
			{
				for (k = 0; k < WIDEBAR_WIDTH; k++)	//FILLING WITH WIDE SPACE
				{
					*pBmp++ = 0;
				}
			}	
		}
	}
	
	separator(SEP_END, &pBmp); //FILLING WITH BARCODE END

	iWeight = formatBmp1(bmp, pBmp);

	ret = printBar(bmp, iheight,iWeight);
	CHECK(ret > 0, lblKO);
	prtS(PrintCode);
	return 1;
	
  lblKO:
    trcErr(ret);
    return -1;
}

// Test Case To print the Barcode Interleaved 2 of 5
void tcjd0025(void){
	// first parameter ia the barcode to be printed
	// Second parameter is the height to be printed
	
	int ret;

	ret = prtStart();
	CHECK(ret > 0,lblKO);
	ret = dspStart();
	CHECK(ret > 0,lblKO);

	ret = NewprintI2Of5("100010041506154647010112", 50); //will print the barcode
	CHECK(ret == 1,lblKO);
	prtS(" ");
	ret = NewprintI2Of5("1234567890", 50); //will print the barcode
	CHECK(ret == 1,lblKO);
	prtS(" ");
	ret = NewprintI2Of5("11111111112", 50); //will print Invalid Barcode as the length is odd
	CHECK(ret == 1,lblKO);
	prtS(" ");

	dspLS(0,"OK!");
	goto lblEnd;
	
lblKO:
	trcErr(ret);
	dspLS(0,"KO!");
lblEnd:
	prtStop();
	dspStop();
}
