/** \file
 * Unitary UTA test case for integration of TCPIP  for APR6R/TST4Q/SGM6Y.
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
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0004.c $
 *
 * $Id: tcmp0004.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static int testStart(byte idx) {
	  //The idx variable identifies which hardware is used for testing
	  // - 0: generic TCPIP terminal
	  // - 1: bluetooth 7780
    switch (idx) {
      case 0:                  
          return comStart(chnTcp5100);
      case 1:                  
          return comStart(chnTcp7780);
      default:
          break;
    }
    return -1;
}    

static int testDial(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
      case 1:      
          ptr = "10.17.32.162|2000";
          break;
      default:
          break;
    }
    VERIFY(ptr);
    //Parameters for comDial is (host address and host port)
    return comDial(ptr);
}

static int makeMsg(byte ctl, byte * msg) {
    int ret;
    card len;
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, 256);
    ret = -1;
    switch (ctl) {
      case 0:
      case 1:      
          bufApp(&buf, (byte *) "\x60\x55\x55\x80\x00", 5);  // TPDU
          bufApp(&buf, (byte *) "\x02\x00", 2);  // Message Type
          bufApp(&buf, (byte *) "\x30\x20\x05\x80\x20\xC0\x00\x04", 8);  // Bitmap (3,4)(11)(22,24)(25)(35)(41,42)(62)
          bufApp(&buf, (byte *) "\x00\x00\x00", 3);  // 3:Processing Code
          bufApp(&buf, (byte *) "\x00\x00\x00\x00\x01\x00", 6);  // 4:Amount
          bufApp(&buf, (byte *) "\x00\x00\x01", 3);  //11:STAN
          bufApp(&buf, (byte *) "\x00\x10", 2);  //22:Point-of-Service Entry Mode
          bufApp(&buf, (byte *) "\x00\x01", 2);  //24:Network International Identifier
          bufApp(&buf, (byte *) "\x00", 1);  //25: Point-of-Service Condition Code
          bufApp(&buf, (byte *) "\x34\x70\x01\x00\x00\x11\x00\x02\x03\x14\xd0\x80\x91\x26\x19\x12\x18\x69", 18); //35:Track 2 Data
          bufApp(&buf, (byte *) "ITCTESP1", 8);  //41: Card Acceptor Terminal Identification
          bufApp(&buf, (byte *) "16             ", 15);  //42:Card Acceptor Identification Code
          bufApp(&buf, (byte *) "\x00\x06\x30\x30\x30\x30\x30\x31", 8);  //62:Private use
        
          len = bufLen(&buf);
          num2bin(msg, len, 2);
          memcpy(msg + 2, bufPtr(&buf), bufLen(&buf));
          ret = 2 + bufLen(&buf);
          break;
      default:
          break;
    }
    return ret;
}

static int testRecv(byte ctl) {
    int ret;
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, 256);
    switch (ctl) {
      case 0:
      case 1:      
          bufReset(&buf);
          ret = comRecvBuf(&buf, 0, 50);
          break;
      default:
          ret = -1;
          break;
    }
    return ret;
}

void tcmp0004(void) {
    int ret;
    byte ctl;
    byte len;
    byte tmp[256];

    ctl = 0; //for 5100
    //ctl = 1; //for 7780
    
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    prtS("comStart");
    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = testStart(ctl);
    CHECK(ret >= 0, lblKO);

    prtS("comStart OK");
    ret = dspLS(0, "comStart OK");
    CHECK(ret >= 0, lblKO);

    prtS("comSet");
    /*
    - If comSet parameters is null, then use static IP address. If not,
    - It has to have the "mask-gateway-localAdress" format (use '|' for the separator)
    */
    ret = comSet("");
    CHECK(ret >= 0, lblKO);

    prtS("comSet OK");
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    prtS("comDial");
    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);
    ret = testDial(ctl);
    CHECK(ret >= 0, lblKO);

    prtS("comDial OK");
    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    prtS("comSend");
    ret = dspLS(2, "comSend...");
    ret = makeMsg(ctl, tmp);
    CHECK(ret >= 0, lblKO);
    len = (byte) ret;
    ret = comSendBuf(tmp, len);
    CHECK(ret == len, lblKO);

    prtS("comSend OK");
    ret = dspLS(2, "comSend OK");
    tmrPause(1);

    prtS("comRecv");
    ret = dspLS(3, "comRecv...");
    ret = testRecv(ctl);
    CHECK(ret > 0, lblKO);

    prtS("comRecv OK");
    ret = dspLS(3, "comRecv OK");
    tmrPause(1);

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    ret = comStop();            //close the port
    CHECK(ret >= 0, lblKO);
    
    comStop();
    //prtStop();
    dspStop();
}
