/** \file
 * Unitary test case.
 * Test Case for custom font printing
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/kd/tckd0005.c $
 *
 * $Id: tckd0005.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#include "sdk30.h"
#include "Libgr.h"

#include <string.h>
#include "sys.h"
#include "tst.h"

#undef prtW
#define prtW 48

static byte prtWait = 0;
static char prtPrefix[16]="";
static FILE *hPrt = 0;

static int initializeFont(void) {
 #ifdef __ICT220__
	
	const char *fnt = "/SYSTEM/COUR.PGN";
 #else
    const char *fnt = "/SYSTEM/COUR.SGN";
 #endif
    char *dft = 0;

    trcFS("%s\n", "initializeFont");
 #ifdef __FONT__
     fnt = __FONT__;
 #endif
    dft = LoadFont((char *) fnt);
    CHECK(dft, lblKO);
    DefCurrentFont((char *) dft);

    return 1;
  lblKO:
    return -1;
}

static int prtStartTst(void) {
    trcFS("%s\n", "prtStart");

    hPrt = fopen("PRINTER", "w-");
    CHECK(hPrt, lblKO);
    prtWait = 0;

    return 1;
  lblKO:
    return -1;
}

static int prtStopTst(void) {
    int ret;

    trcFS("%s", "prtStop\n");
    if(hPrt) {
        if(prtWait)
            ttestall(PRINTER, 0);
        ret = fclose(hPrt);
        //CHECK(ret==RET_OK,lblKO);
        hPrt = 0;
    }
    prtWait = 0;

    return 1;
}

static int prtSTst(const char *str) {
    int ret;
    char buf[prtW + 1 + 1];     //one more character for line break at the end
    char tmp[255];

    VERIFY(str);
    trcFS("prtS str=%s\n", str);

    memset(buf, 0, prtW + 1);
    if(strlen(str) <= prtW)
        strcpy(buf, str);
    else
        memcpy(buf, str, prtW);

    strcat(buf, "\n");
	  strcpy(tmp,prtPrefix);
	  strcat(tmp,buf);
	  *prtPrefix= 0;
	  //if(prtWait)
		//  ttestall(PRINTER, 0);       //without it the next printing is not stable sometimes

	    /*
    ret = fprintf( hPrt ,  "aAa\n" ) ;
    ret = fprintf( hPrt , "BbB\n" ) ;
    ret = fprintf( hPrt , "cCc\n" ) ;

    ret = pprintf8859( "dNORMAL: LaLaLa!!!!\n" , _OFF_ , "_dNORMAL_" , _FIXED_WIDTH_ ) ;
    ret = pprintf8859( "dNORMAL: ::::::LuLu\n" , _OFF_ , "_dNORMAL_" , _FIXED_WIDTH_ ) ;

    ret = pprintf8859( "pNORMAL: TiTi!!!!!!\n" , _OFF_ , "_pNORMAL_" , _FIXED_WIDTH_ ) ;
    ret = pprintf8859( "pNORMAL: TaTaTa!!!!\n" , _OFF_ , "_pNORMAL_" , _FIXED_WIDTH_ ) ;

    ret = pprintf8859( "dBIG: 0123456789\n" , _OFF_ , "_dBIG_" , _FIXED_WIDTH_ ) ;
    ret = pprintf8859( "dBIG: ..!!;;:://\n" , _OFF_ , "_dBIG_" , _FIXED_WIDTH_ ) ;
    */
	  
    ret = pprintf8859( tmp , _OFF_ , "_pNORMAL_" , _FIXED_WIDTH_  ) ;
    trcFN( "pprintf8859 ret=%d\n",ret ) ;    
    CHECK(ret >= 0, lblKO);
    
    prtWait = 1;
    return strlen(buf);
  lblKO:
    return -1;
}
/*
static int prtESTst(const char *esc, const char *str) {
    int ret;
    int len;
    byte diff;
    char tmp[255];
	char *ptr;
	byte ctl= 0;

    VERIFY(str);
    //VERIFY(esc);
    trcFS("prtES str=%s\n", str);
    strcpy(tmp,prtPrefix);
    *prtPrefix= 0;
	  ptr= tmp+strlen(tmp);

    //Reformat the control ESC to suit Sagem Printer
    if(esc) {
        switch (esc[1]) {
          case 0x1A:           //BOLD
              strcpy(ptr, "\x1B" "E");
              break;
          case 0x22:           //REVERSE
              strcpy(ptr, "\x1B" "B1");
              break;
          case 0x1C:           //2 X WIDTH
              strcpy(ptr, "\x0E");
              break;
          case 0x1D:           //2 X HEIGHT
              strcpy(ptr, "\x1B" "H");
              break;
          case 0x1E:           //2 X SIZE
              strcpy(ptr, "\x0E\x1B" "H");
              break;
          case 0x20:           //48 columns
              strcpy(ptr, "\x0F");
              break;
          default:             //not known
              strcpy(ptr, esc);
              break;
        }

		ctl= esc[1];
		if(strcmp(ptr,"\x1B" "B1")==0) //reversed
			ctl= 0x22;
		else if(strcmp(ptr,"\x0E")==0) //2 X WIDTH
			ctl= 0x1C;
		else if(strcmp(ptr,"\x1B" "H")==0) //2 X HEIGHT
			ctl= 0x1D;
		else if(strcmp(ptr,"\x0E\x1B" "H")==0) //2 X SIZE
			ctl= 0x1E;
		else if(strcmp(ptr,"\x0F")==0) //48 columns
			ctl= 0x20;
        strcat(tmp, str);

        switch (ctl) {
          case 0x17:           //CENTER
              len = strlen(tmp);
              diff = (prtW - len) / 2;
              if(diff) {
                  memset(tmp, ' ', len);
                  strncpy(tmp + diff, str, len + 1);
              }
              break;
          case 0x18:           //RIGHT
              len = strlen(tmp);
              diff = prtW - len;
              if(diff) {
                  memset(tmp, ' ', prtW);
                  strncpy(tmp + diff, str, len + 1);
              }
              break;
          case 0x1A:           //BOLD
              strcpy(prtPrefix, "\x1B" "F" "\x1B" "@");
              break;
          case 0x22:           //REVERSE
              strcpy(prtPrefix, "\x1B" "B0" "\x1B" "@");
              break;
          case 0x1C:           //2 X WIDTH
              strcpy(prtPrefix, "\x0F" "\x1B" "@");
              break;
          case 0x1D:           //2 X HEIGHT
              strcpy(prtPrefix, "\x0F" "\x1B" "@");
              break;
          case 0x1E:           //2 X SIZE
              strcpy(prtPrefix, "\x1B" "@" "\x1B" "@");
              break;
          case 0x20:           //48 Columns
              strcpy(prtPrefix, "\x0E" "\x1B" "@");
              break;
		  default:
			  break;
        }
    }else
		strcat(tmp,str);

    len = strlen(tmp);
	if(tmp[len] != 0x0D && tmp[len] != 0x0A)
		strcat(tmp, "\n");

	  if(prtWait)
		  ttestall(PRINTER, 0);       //without it the next printing is not stable sometimes
    ret = pprintf8859( tmp , _OFF_ , _pNORMAL_ , _FIXED_WIDTH_ ) ;
    CHECK(ret > 0, lblKO);
    prtWait = 1;

    return strlen(tmp);
  lblKO:
    tmrPause(100);
    return -1;
}
*/

void tckd0005(void) {
    int ret;
    const char *alphaCaps[] = {
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
    };
    const char *alphaSmall[] = {
      "abcdefghijklmnopqrstuvwxyz" 
    };
    const char *numbers[] = {
      "012345678901234567890123" 
    };
    const char *special[] = {
      "~!@#$%^&*()-+=/.,<>?;:'\"[]{}|" 
    };

    trcS("tckd000 Beg\n");

    ret= prtStartTst();
    CHECK(ret >= 0, lblKO);
    
    ret = initializeFont();
    CHECK(ret >= 0, lblKO);

    ret= prtSTst(*numbers) ;
    CHECK(ret >= 0, lblKO);
    ret= prtSTst(*alphaCaps) ;
    CHECK(ret >= 0, lblKO);
    ret= prtSTst(*alphaSmall) ;
    CHECK(ret >= 0, lblKO);
    ret= prtSTst(*special) ;
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();
    prtStopTst();
    trcS("tckd0005 End\n");
}    
