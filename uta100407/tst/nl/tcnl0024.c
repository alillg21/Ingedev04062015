#include <string.h>
#include "sys.h"
#include "tst.h"
#include "ctx.h"
#include <ssaStd.h>

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0024.
//changing 24 * 16 bit Bitmap printing to 32 * 12 bit 
//Application sp2p5

#define CHK CHECK(ret>=0,lblKO)

/*

static const unsigned char gly004E[]= { 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //......};
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //......};
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //......};
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //......};
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //......};
    0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30, //..**..};
    0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38, //..***.};
    0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40, //.*....};
    0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40, //.*....};
    0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30, //..**..};
    0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30, //..**..};
};
*/
/*
static const unsigned char gly0042[]= { 
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x20,0x00, //..*........};
    0x20,0x00, //..*........};
    0x20,0x00, //..*........};
    0x20,0x00, //..*........};
    0x20,0x00, //..*........};
    0x20,0x00, //..*........};
    0x10,0x00, //...*.......};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
    0x00,0x00, //...........};
};

*/
void tcnl0024(void) {
    //#include "talin.fnt"
#include "2_talin.fnt"
//  #include "tcnl0024.fnt"
    int ret;
    int n;
    int loc;

//  int i;
    int hgt = 11;
    byte tmp1 = 0;
    byte tmp2 = 0;
    byte a, b, c, d;
    int cnt = 0;
    int count = 0;
    int sftN = 4;
    byte pos;
    byte sav;
    int glyNo;
    int newCnt = 0;
    static unsigned char out[1000];
    static unsigned char in1[1000];
    static unsigned char in2[1000];
    static unsigned char result[5000] = "";
    int i, j, k;

    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    n = sizeof(gly) / sizeof(void *) - 1;

    memcpy(&in1, gly[0], 48);
    memcpy(&in2, gly[1], 48);

    for (loc = 0; loc < NoRow; loc++) {
        glyNo = n;
        cnt = 0;
        while(glyNo > 0)        // this if for connecting only one row of some glyps     
        {

            pos = cnt % 4;
            switch (pos)        //har 4 ta bayad tekrar she
            {
              case 0:
                  a = in1[loc * 2];
                  b = in1[loc * 2 + 1];
                  c = in2[loc * 2];
                  d = in2[loc * 2 + 1];
                  //keep d in temp memory
                  sav = d;
                  tmp1 = c >> sftN;
                  tmp2 = b | tmp1;

                  b = tmp2;
                  d = sav;
                  c = (c << sftN) | (d >> sftN);
                  d = d << sftN;

                  result[pos * 2 + count] = a;
                  result[pos * 2 + 1 + count] = b;
                  result[pos * 2 + 2 + count] = c;
                  result[pos * 2 + 3 + count] = d;
                  glyNo = glyNo - 2;

                  break;
              case 1:
                  a = c;
                  b = d;

                  memcpy(&in2, gly[cnt + 2 - 1], NoRow * noClmn);
                  c = in2[loc * 2];
                  d = in2[loc * 2 + 1];
                  sav = d;
                  tmp1 = c >> sftN;
                  tmp2 = b | tmp1;
                  b = tmp2;
                  d = sav;
                  c = (c << sftN) | (d >> sftN);
                  d = d << sftN;

                  b = b << 4 | c >> 4;
                  c = c << 4;
                  d = d << 4;

                  result[pos * 2 + count] = a;
                  result[pos * 2 + 1 + count] = b;
                  result[pos * 2 + 2 + count] = c;
                  result[pos * 2 + 3 + count] = d;
                  glyNo--;

                  break;
              case 2:
                  a = c;
                  b = d;
                  memcpy(&in2, gly[cnt + 2 - 1], NoRow * noClmn);
                  c = in2[loc * 2];
                  d = in2[loc * 2 + 1];

                  sav = d;
                  tmp1 = c >> sftN;
                  tmp2 = b | tmp1;
                  b = tmp2;
                  d = sav;
                  c = (c << sftN) | (d >> sftN);
                  d = d << sftN;

                  b = b << sftN | c >> sftN;
                  c = c << sftN;
                  d = d << sftN;

                  a = b >> sftN | a;
                  b = b << sftN | c >> sftN;
                  c = c << sftN;

                  result[pos * 2 + count] = a;
                  result[pos * 2 + 1 + count] = b;
                  result[pos * 2 + 2 + count] = c;
                  result[pos * 2 + 3 + count] = d;
                  glyNo--;
                  break;
              case 3:

                  count = count + pos * 2;
                  if(cnt < n - 1) {
                      memcpy(&in1, gly[cnt + 1], NoRow * noClmn);

                  } else {
                      in1[loc * 2] = 0;
                      in1[loc * 2 + 1] = 0;
                  }
                  if(cnt < n - 2) {
                      memcpy(&in2, gly[cnt + 1 + 1], NoRow * noClmn);
                  } else {
                      in2[loc * 2] = 0;
                      in2[loc * 2 + 1] = 0;
                  }

                  break;

            }
            cnt++;
        }

        memcpy(&in1, gly[0], NoRow * noClmn);
        memcpy(&in2, gly[1], NoRow * noClmn);

        count = (n * 3) / 2 * (loc + 1);
        if(n % 2 != 0) {
            count = count + 1 + loc;
        }

    }

    if(n % 2 == 0) {
        count = (n * 3) / 2;
        ret = prtImg(result, NoRow, count, "r1");
    } else {
        count = (n * 3) / 2 + 1;
        ret = prtImg(result, NoRow, count, "r1");

    }
    // prtS("\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf\xaf");
    prtS("  \xa2\x93\xe3\xf5\xf6\x91\x9B\xa4\xc1 \xa7\x91\xa8");
    prtS("\n\n\n\n");

    //this bellow lines  must be used for attaching some bitmap and making the image
/*	 for(k=0;k<NoRow;k++) //number of rows in each bitmap
	 {
		 for(j=0;j<noBitmap;j++) //number of bitmap
		 {
			 memcpy(&in2,gly[j],noClmn*NoRow);//16*2	(number of rows in each bitmap *number of column in each bitmap)
			 for(i=0;i<noClmn;i++) //number of column in each bitmap
			 {
				 result[i+j*noClmn+k*noBitmap*noClmn]= in2[i+k*noClmn];//result[i+j*2+k*32*2]= in2[i+k*2]
			 }
		 }		 
	 }
	 ret= prtImg(result,NoRow,noBitmap*noClmn,"r1"); //number of bitmap * number of column in each bitmap

*/

    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
  lblEnd:
    prtStop();

    tmrPause(3);
}
#endif
