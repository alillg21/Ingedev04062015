/** \file
 * Unitary non UTA test case for ...
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/hs/tchs0000.c $
 *
 * $Id: tchs0000.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "sys.h"
#include "tst.h"



/* VIVOtech 4500 Contactless Reader RS-232 Interface Options: 19200, None, 8, 1  
 * 
 * Interface set to polling at all time.
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
#define CHAR_NUL    0x00
#define CHAR_SOH    0x01
#define CHAR_STX    0x02
#define CHAR_ETX    0x03
#define CHAR_EOT    0x04
#define CHAR_ENQ    0x05
#define CHAR_ACK    0x06

static int magTrack1(char *trk1, char *src, char *dst) {
    int ret = 0;                //to be returned: number of tracks read
    unsigned char len = 0;
    char tmp[128];

        dst = trk1;
        while(*src) {           //find start sentinel
            if(*src++ == '%')
                break;
        }
        while(*src) {           //copy all data between start and end sentinels
            if(*src == '?')
                break;
            *dst++ = *src++;
        }
        trcFS("Track 1 : %s\n", trk1);

  lblEnd:
    return ret;
}

static int magTrack2(char *trk2, char *src, char *dst) {
    int ret = 0;                //to be returned: number of tracks read
    unsigned char len = 0;
    char tmp[128];

        dst = trk2;
        while(*src) {           //find start sentinel
            if(*src++ == 'B')
                break;
        }
        while(*src) {           //copy all data between start and end sentinels
            if(*src == 'F')
                break;
            if(*src == 'D')
                *src = '=';
            *dst++ = *src++;
        }
        trcFS("Track 2 : %s\n", trk2);
        CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    return -1;
  lblEnd:
    return ret;
}


void tchs0000(void) {
	
    int ret = -1;
    byte b;
    int end = 0;
    byte msg[128*3];
    tBuffer buf;
    byte dat[256];
    int nCount = 0;
    char *ptr = NULL;
    char *temp = 0;
    char *para = NULL;
    byte date[5];
    byte mmyy[6];
    trcS("#BEG tchs0000\n");
    bufInit(&buf, dat, sizeof(dat));
    char *track1=0;
    char track2[128];
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, " Tab VIVOTech");
    tmrPause(0);
    ret = comStart(chnCom1);
    CHECK(ret >= 0, lblKOStart);
    ptr = "8N119200";
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKOSet);
    TRCDT("comSet done");

    ret = comDial("");
    CHECK(ret >= 0, lblKODial);

    //memset(msg,0,sizeof(msg));
    int x = 0;
    int s = 0;
    int skipB = 0;
    int skipfirst = 0;
    bufInit(&buf, dat, 256);
    bufReset(&buf);

    ret = comRecvBuf(&buf, 0, 60);
    CHECK(ret >= 0, lblKOReceive);
    memcpy(msg,bufPtr(&buf), bufLen(&buf));

    /*do {
        ret = comRecv(&b, 60);
        CHECK(ret >= 0, lblKOReceive);
        if (skipB == 0) {
	        if (b == 'B') {
	        	x = 1;
	        	skipB = 1;
	        	skipfirst = 1;
	        }
        }
        if (x == 1) { 
        	if (skipfirst == 0) {
        		if (b != ';')
        			msg[s++] = b;  
        	}
        	skipfirst = 0;
        }
        if ((end == 1) && (b == '?')) {
           //ret = comSend(CHAR_ACK);
           //CHECK(ret >= 0, lblKOSend);
           break;
        }
        if (b == '?') {
        	end = 1;
        }
        if(ret < 0)
            break;
        if(ret)
            nCount += ret;
    } while(s < 256);*/
    ret = prtStart();
   	CHECK(ret >=0, lblKO);
   	ret = prtCtrl();   // Check paper preset
   	CHECK(ret >= 0, lblKO);
   	//memset(track1,0,sizeoft);
   	//memset(track2,0,128);
   	
   	//trcS("parse track1\n");
   	//para = strtok(msg,"?");
   	//memcpy(track1,para,strlen(para));
   	//trcS("parse track2\n");
   	//para = strtok(NULL,"?");
   	//memcpy(track2,para,strlen(para));
   	ret = magTrack1(track1,(char *)msg,temp);
   	CHECK(ret >=0, lblKO);
   	ret = magTrack2(track2,msg,temp);
   	CHECK(ret >= 0, lblKO);

   	ret = prtES(BOLD, "      VIVOtech");
   	CHECK(ret >= 0, lblKO);
   	//para = strtok(msg,"B");
   	para = strtok(track1,"^");
   	if (para > 0){
	   ret = prtES(BOLD, "Account Number:");
	    CHECK(ret >= 0, lblKO);
	    ret = prtS(para);
	    CHECK(ret >= 0, lblKO);
	    ret = prtES(BOLD,"Account Name:");
    	CHECK(ret >= 0, lblKO);
    	para = strtok(NULL, "^");
    	ret = prtS(para);
    	CHECK(ret >= 0, lblKO);
    	para = strtok(NULL,"?");
    	ret = prtES(BOLD,"Exp:");
    	if (strlen(para) > 4) {
    		memset(date,0,sizeof(date));
    		memcpy(date,para,4);
    		memset(mmyy,0,sizeof(mmyy));
    		mmyy[0] = date[2];
    		mmyy[1] = date[3];
    		mmyy[2] = '/';
    		mmyy[3] = date[0];
    		mmyy[4] = date[1];
    		mmyy[5] = '\0';
    		ret = prtS(mmyy);
    		CHECK(ret >= 0, lblKO);
    	}
    	ret = prtS("\n\n\n");
    	CHECK(ret >= 0, lblKO);    	
   }
  
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
    trcS("#END tchs0000\n");
    
}

