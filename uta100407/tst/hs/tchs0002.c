/** \file
 * Unitary non UTA test case for ...
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/hs/tchs0002.c $
 *
 * $Id: tchs0002.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"




/* Checkreader 2500 RS-232 Interface Options: 9600, None, 8, 2    
 * OPOS configuration AX00808.cfg
 * 
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



void tchs0002(void) {	
    int ret = 0;
    char *para;
    byte msg[128*3];   
    trcS("#BEG tchs0002#\n");
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, " Slide Check");
    CHECK(ret >=0, lblKOSet);
    memset(msg,0,sizeof(msg));
    ret = prtStart();
    CHECK(ret >=0, lblKOSet);
   	ret = prtCtrl();   // Check paper preset
   	CHECK(ret >= 0, lblKO);
   	ret = dspLS(0, "Slide Check");
   	CHECK(ret >= 0, lblKO);
   	ret = enterCard(msg, "h");
	CHECK(ret >= 0, lblKO);   	
   	ret = prtES(BOLD, "   eN-Check 2500");
   	CHECK(ret >= 0, lblKO);
   	para = strtok(msg,"T");

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
   	CHECK(ret >= 0, lblKO);    	
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
  trcS("#END tchs0002()#\n");
    dspStop();                  //close resource
    prtStop();    
}
