/** \file
 * Unitary non UTA test case for ...
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/hs/tchs0001.c $
 *
 * $Id: tchs0001.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "sys.h"
#include "tst.h"




/* Checkreader 2500 RS-232 Interface Options: 9600, None, 8, 2    
 * OPOS configuration AX00808.cfg
 * Interface send 3 retries if it does not received an ACK
 */

#define CENTER "\x1b\x17"
#define RIGHT "\x1b\x18"
#define LEFT "\x1b\x19"
#define BOLD "\x1b\x1a"
#define REVERSE "\x1b\x22"
#define NORMAL "\x1b\xb0"
#define DWIDTH "\x1b\x1c";    //only for UNICAPT
#define DHEIGHT "\x1b\x1d"
#define DSIZE "\x1b\x1e" //Double Size");
#define QUADRO "\x1b\x1f"  //Quadro only for UNICAPT
#define CONDENSED "\x1b\x20"
#define COLUMNS "\x1b\x21"  //only for UNICAPT
#define MEDIUM "\x1b\x2A"  //only for TELIUM


void tchs0001(void) {
	
	
     int ret = 0;
    int length = 0;
    byte b;
    byte msg[256];
   // char * msg = "CMET031100649T621170371309*212985 L076G094H109V001EC00EX00x";
    tBuffer buf;
    byte dat[256];
    int nCount = 0;
    char *ptr = NULL;
    char *para = NULL;
    byte date[5];
    byte mmyy[6];
    int iETXFound = 0;
    int iSTXFound = 0;
    trcS("#BEG tchs0001#\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
  
    ret = dspLS(0, " Slide Check");
    memset(msg,0,sizeof(msg));
    ret = getCheck("7E11200", msg);
    CHECK(ret >=0, lblKOSet);
    ret = prtStart();
   	ret = prtCtrl();   // Check paper preset
   	CHECK(ret >= 0, lblKO);
   
   	ret = prtES(BOLD, "   eN-Check 2500");
   	CHECK(ret >= 0, lblKO);
   	para = strtok(msg,"T");
   //para = strtok(NULL,"T");
   if (para >= 0){
	   ret = prtES(BOLD, "Bank Routing #:");
   		CHECK(ret >= 0, lblKO);
   		ret = prtS(para);
   		CHECK(ret >= 0, lblKO);
   		ret = prtES(BOLD,"Bank Account #:");
   		CHECK(ret >= 0, lblKO);
   		para = strtok(NULL, "O");
   		ret = prtS(para);
   		CHECK(ret >= 0, lblKO);
   		para = strtok(NULL,"O");
   		ret = prtES(BOLD,"Check #:");
   		ret = prtS(para);
   		CHECK(ret >= 0, lblKO);
   		ret = prtS("\n\n\n");
   		CHECK(ret >= 0, lblKO);    	
   	}
   	ret = prtS("\n\n\n");
   	HECK(ret >= 0, lblKO);    	
    goto lblEnd;

  lblKO:
    trcS("Pre-req Failed\n");
    goto lblEnd;
  lblKOStart:
    trcS("comStart Failed\n");
    goto lblEnd;
  lblKOSet:
    trcS("comSet Failed\n");
    goto lblEnd;
  lblKODial:
    trcS("comDial Failed\n");
    goto lblEnd;
  lblKOReceive:
    trcS("comReceive Failed\n");
    goto lblEnd;
  lblKOSend:
    trcS("comSend Failed\n");
    goto lblEnd;
  lblEnd:
    trcErr(ret);
    comStop();
    dspStop();                  //close resource
    prtStop();
    trcS("#END tchs0001()#\n");
    
}