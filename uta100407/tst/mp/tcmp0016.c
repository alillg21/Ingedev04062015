#ifdef __TELIUM__
#include <sdk30.h>
#include "oem_public.h"
#else
#include <unicapt.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

// BAR DRAWING DEFINITIONS
#ifdef __TELIUM__
#define BAR_WIDTH (1L)
#else
#define BAR_WIDTH (2L)
#endif

// CHARACTERS DEFINITIONS for interleaved 25
#define BCD25_START_LEN (4L)
#define BCD25_CHAR_LEN  (5L)
#define BCD25_END_LEN   (3L)

static const byte INTERLEAVED_25_START[BCD25_START_LEN] = {
   1, 1, 1, 1
};

static const byte INTERLEAVED_25[10][BCD25_CHAR_LEN] = {
   {1, 1, 3, 3, 1},   // 0
   {3, 1, 1, 1, 3},   // 1
   {1, 3, 1, 1, 3},   // 2
   {3, 3, 1, 1, 1},   // 3
   {1, 1, 3, 1, 3},   // 4
   {3, 1, 3, 1, 1},   // 5
   {1, 3, 3, 1, 1},   // 6
   {1, 1, 1, 3, 3},   // 7
   {3, 1, 1, 3, 1},   // 8
   {1, 3, 1, 3, 1},   // 9
};

static const byte INTERLEAVED_25_END[BCD25_END_LEN] = {
   3, 1, 1
};

static byte isDigit(char c) {
	if( (c >= '0') && (c <= '9'))
		return 1;
	else
		return 0;
}

static word formatBmp(byte *bmp, byte *pBmp, word height) {
    #ifndef __TELIUM__
	word j;
    #endif
	word i;
	byte  *pBmp2;
	byte  tmp;

	#ifdef __TELIUM__
	for(i=0, pBmp2=bmp; pBmp2<pBmp; i++) {
		tmp = 0;
		tmp = *pBmp2++;	
		if (tmp == 1)
			tmp = 0xFF;
		bmp[i] = tmp;
	}	
	#else	
	for(i=0, pBmp2=bmp; pBmp2<pBmp; i++) {
		tmp = 0;
		for(j=0; j<8; j++) {
			tmp += *pBmp2++ * 1<<(7-j);
		}
		bmp[i] = tmp;		
	}
	#endif

	trcS("Bmp After Format = \n");
	trcBN(bmp, i);
	trcS("\n");
	return i;
}

static int printBarCode(byte *bmp, byte *pBmp, word height,  const char* ctl) {
	word i, tmpHgt;
	int ctr;
	byte pBmp2[4096];
	int  ret = 1;
	 
    trcS("printBarCode\n");
	i = formatBmp(bmp, pBmp, height);
	trcFN("Barcode Width = %d\n", i);   

    #ifdef __TELIUM__
	tmpHgt = height/10;
    #else
	tmpHgt = height;
    #endif
	ctr = 0;
	memset(pBmp2, 0 , sizeof(pBmp2));
	while (tmpHgt--) {
      #ifdef __TELIUM__
      pBmp2[ctr] = 0;
      ctr++;
      pBmp2[ctr] = i;
      ctr++;
      memcpy(&pBmp2[ctr], bmp, i);
      ctr = ctr + i;
      pBmp2[ctr] = 0xFF;
      ctr++;
      pBmp2[ctr] = 0;
      ctr++;      
      #else
      memcpy(&pBmp2[ctr], bmp, i);
      ctr = ctr + i;
      #endif
	}

	ret = prtImg(pBmp2, height, i, ctl);
	
    trcS("printBarCode End\n");
	goto lblEnd;
	
	lblKO:
		trcS("printBarCode KO\n");
	 	ret = -1;
	lblEnd:
		return ret;
}

static int bcdInterleaved25(char *code, word height,  const char* ctl) {
	word i, j, k, l;
	int  ret;
	byte  *bmp = NULL, *pBmp = NULL;
	byte  codeLen;
	byte  color;

	trcS("bcdInterleaved25\n");
	codeLen = strlen(code);

	if(codeLen % 2 != 0) {
		goto lblKO;
	}

	for(i=0; i<codeLen; i++) {
		if(!isDigit(code[i])) {
			goto lblKO;
		}
	}

	#ifdef __TELIUM__
	bmp = (byte*)umalloc(height * (BCD25_START_LEN+BCD25_END_LEN+BCD25_CHAR_LEN*codeLen)*BAR_WIDTH*3*sizeof(byte));	
	#else
	bmp = (byte*)calloc((BCD25_START_LEN+BCD25_END_LEN+BCD25_CHAR_LEN*codeLen)*BAR_WIDTH*3, sizeof(byte));
	#endif
	if(bmp == NULL) {
		trcS("Out of memory!\n");
		goto lblKO;
	}
	pBmp = bmp;

	// BEGIN
	for(j=0; j<BCD25_START_LEN; j++) {
		if(j%2 == 0) {
			color = 1; // Black
		}
		else {
			color = 0; // White
		}

		for(k=0; k<INTERLEAVED_25_START[j] * BAR_WIDTH; k++) {
		   *pBmp++ = color;
		}
	}

	// BAR CODE
	codeLen = codeLen >> 1; // /!\ /!\ Warning!
	for(i=0, l=0; i<codeLen; i++) {
		for(j=0; j<BCD25_CHAR_LEN; j++) {
			for(k=0; k<INTERLEAVED_25[code[2*i]-0x30][j] * BAR_WIDTH; k++) {
			  *pBmp++ = 1;
			}
			for(k=0; k<INTERLEAVED_25[code[2*i+1]-0x30][j] * BAR_WIDTH; k++) {
			  *pBmp++ = 0;
			}
		}
	}

	// END
	for(j=0; j<BCD25_END_LEN; j++) {
		if(j%2 == 0) {
			color = 1; // Black
		}
		else {
			color = 0; // White
		}
	  
		for(k=0; k<INTERLEAVED_25_END[j] * BAR_WIDTH; k++) {
			*pBmp++ = color;
		}
	}

	// PRINTING
	ret = printBarCode(bmp, pBmp, height, ctl);
	CHECK(ret >= 0, lblKO);
	
	trcS("bcdInterleaved25 End\n");
	goto lblEnd;
	
	lblKO:
		trcS("bcdInterleaved25 KO\n");
		ret = -1;		
	lblEnd:
	#ifdef __TELIUM__
	ufree(bmp);
	#else
	free(bmp); 
	#endif
	bmp = NULL;
	return ret;
}

void tcmp0016(void) {
    int ret;

	trcS("tcmp0016 Beg\n");
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    prtS("- interleaved -");
    ret = bcdInterleaved25("1234567890", 80, "c1");
    CHECK(ret >= 0, lblKO);
		
    dspLS(0, "print OK!");		
	goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "print KO!");
  lblEnd:    
    tmrPause(2);
	trcS("tcmp0016 End\n");		
    prtStop();
    dspStop();
}

