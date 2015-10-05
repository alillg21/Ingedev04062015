  /** \file
 * Unitary UTA test case for EAST HOST SPEC.
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comStop()
 *  - comDial()
 *  - comHangStart()
 *  - comHangWait()
 *  - comRecv()
 *  - comRecvBuf()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0007.c $
 *
 * $Id: tcmp0007.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
 
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

static int testStart(byte ctl) {
    int ret = -1;

    switch (ctl) {
      case 0:
	   ret = comStart(chnMdm);
          break;
      case 1:
	   ret = comStart(chnCom1);
          break;
      case 2:
	   ret = comStart(chnTcp5100);
          break;
      case 3:
	   ret = comStart(chnGprs);
          break;		  
      default:
          break;
    }

    return ret;
}

static int testSet(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
      case 1:
	   ptr = "8N11200";	  	
          break;
      case 2:
	   ptr = "";
	   break;
      case 3:
          //format: APN|username|password|
    	   ptr = "|internet.globe.com.ph|||";	  	
          break;
          break;		  
      default:
          break;
    }
    VERIFY(ptr);
    return comSet(ptr);
}


static int testDial(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
          ptr = "9-7519206";
	   break;	  	
      case 1:  	
    	   ptr = "";		  
          break;
      case 2:
      case 3:  	   
          ptr = "192.168.0.1|2000";
          break;  
      default:
          break;
    }
    VERIFY(ptr);
    return comDial(ptr);
}

static int testSend(byte ctl, byte * msg) {
    int ret = 0;
    card len;
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, 256);
	
    bufApp(&buf, (byte *) "\x12\x00", 2);  // Message Type
    //bufApp(&buf, (byte *) "\x30\x20\x04\x01\x20\x40\x00\x02", 8);  // Bitmap (3,4)(11)(22)(32,35)(42)(63)
    bufApp(&buf, (byte *) "\x30\x20\x04\x00\x20\x40\x00\x02", 8);  // Bitmap (3,4)(11)(22)(35)(42)(63)
    bufApp(&buf, (byte *) "\x00\x00\x00", 3);  // 3:Processing Code
    bufApp(&buf, (byte *) "\x00\x00\x00\x00\x01\x00", 6);  // 4:Amount
    bufApp(&buf, (byte *) "\x00\x00\x01", 3);  //11:STAN
    bufApp(&buf, (byte *) "\x00\x10", 2);  //22:Point-of-Service Entry Mode
    //bufApp(&buf, (byte *) "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30",11);  //32:Acquirer Institution ID (Prior to migration: Required, After MIgration: Not Used)
    bufApp(&buf, (byte *) "\x34\x70\x01\x00\x00\x11\x00\x02\x03\x14\xd0\x80\x91\x26\x19\x12\x18\x69", 18); //35:Track 2 Data
    bufApp(&buf, (byte *) "16             ", 15);  //42:Card Acceptor Identification Code
    bufApp(&buf, (byte *) "\x30\x32\x1C\x30\x30\x31\x1C\x31\x1C\x30\x30\x30\x31", 13);  //63: Global Payments Additional Data
    
    len = bufLen(&buf);
    num2bin(msg, len, 2);
    memcpy(msg + 2, bufPtr(&buf), bufLen(&buf));
    ret = 2 + bufLen(&buf);	
    return ret;
}

static int testRecv(byte ctl) {
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, 256);
    return comRecvBuf(&buf, 0, 50);
}

void tcmp0007(void) {
    int ret;
    byte ctl;
    byte len;
    byte tmp[256];
    char *ptr;

    //0 for modem; 1 for  COM1; 2 for TCPIP; 3 for GPRS
    ctl = 0;	

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    //////////COM START//////////
    prtS("comStart");
    dspLS(0, "comStart...");
    ret = testStart(ctl);    
    CHECK(ret >= 0, lblKO);
    prtS("comStart OK");
    dspLS(0, "comStart OK");

    //////////COM SET//////////
    prtS("comSet");
    dspLS(0, "comSet...");
    ret = testSet(ctl);
    CHECK(ret >= 0, lblKO);
    prtS("comSetOK");
    dspLS(0, "comSet OK");

    //////////COM DIAL//////////
    prtS("comDial...");
    dspLS(1, "comDial...");
    ret = testDial(ctl);
    CHECK(ret >= 0, lblKO);
    prtS("comDial OK");
    dspLS(1, "comDial OK");

    //////////COM SEND//////////
    prtS("comSend...");
    dspLS(2, "comSend...");
    ret = testSend(ctl, tmp);
    CHECK(ret >= 0, lblKO);
    len = (byte) ret;
    ret = comSendBuf(tmp, len+1);
    CHECK(ret == len+1, lblKO);
    prtS("comSend OK");
    dspLS(2, "comSend OK");

    //////////COM RECEIVE//////////
    prtS("comRecv...");
    dspLS(3, "comRecv...");
    ret = testRecv(ctl);
    CHECK(ret > 0, lblKO);
    prtS("comRecv OK");
    dspLS(3, "comRecv OK");

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("KO!");
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    ret = comStop();            //close the port
    CHECK(ret >= 0, lblKO);
    
    prtStop();
    dspStop();
}


