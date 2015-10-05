 /** \file
 * Unitary UTA test case for integration of Modem/HDLC for APR6R/TST4Q/SGM6Y.
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
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0003.c $
 *
 * $Id: tcmp0003.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
 
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

static int testStart(byte idx) {
	  //The idx variable identifies which communication channel will be used for testing
	  // - 0: HDLC
	  // - 1: Modem
    switch (idx) {
      case 0:                  
          return comStart(chnHdlc);
      case 1:                  
          return comStart(chnMdm);
      default:
          break;
    }
    return -1;
}    

static void fillBuf(byte idx, tBuffer * buf) {
    switch (idx) {
      case 0:                  
      case 1:              
          bufApp(buf, (byte *) "\x60\x55\x55\x30\x00", 5);  // TPDU
          bufApp(buf, (byte *) "\x02\x00", 2);  // Message Type
          bufApp(buf, (byte *) "\x30\x20\x05\x80\x20\xC0\x00\x04", 8);  // Bitmap (3,4)(11)(22,24)(25)(35)(41,42)(62)
          bufApp(buf, (byte *) "\x00\x00\x00", 3);  // 3:Processing Code
          bufApp(buf, (byte *) "\x00\x00\x00\x00\x01\x00", 6);  // 4:Amount
          bufApp(buf, (byte *) "\x00\x00\x01", 3);  //11:STAN
          bufApp(buf, (byte *) "\x00\x10", 2);  //22:Point-of-Service Entry Mode
          bufApp(buf, (byte *) "\x00\x01", 2);  //24:Network International Identifier
          bufApp(buf, (byte *) "\x00", 1);  //25: Point-of-Service Condition Code
          bufApp(buf, (byte *) "\x34\x70\x01\x00\x00\x11\x00\x02\x03\x14\xd0\x80\x91\x26\x19\x12\x18\x69", 18); //35:Track 2 Data
          bufApp(buf, (byte *) "ITCTESP1", 8);  //41: Card Acceptor Terminal Identification
          bufApp(buf, (byte *) "16             ", 15);  //42:Card Acceptor Identification Code
          bufApp(buf, (byte *) "\x00\x06\x30\x30\x30\x30\x30\x31", 8);  //62:Private use
          break;
      default:
          break;
    }
}

static char *getSrv(byte idx) {
    switch (idx) {
      case 0:          
          //phone number where to dial
          return "9-7519206";
      case 1:                  
          return "9-7519220";          
      default:
          break;
    }
    return 0;
}

void tcmp0003(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[256];
    byte idx;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 256);
    bufReset(&buf);

    idx = 0; //for Hdlc
    //idx = 1;    //for Modem
    fillBuf(idx, &buf);
    ptr = getSrv(idx);

    prtS("comStart");
    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = testStart(idx);
    CHECK(ret >= 0, lblKO);

    prtS("comStart OK");
    ret = dspLS(0, "comStart OK");
    CHECK(ret >= 0, lblKO);    

    prtS("comSet");
    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);
    ret = comSet("8N11200");
    CHECK(ret >= 0, lblKO);

    prtS("comSet OK");
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);    

    prtS("comDial");
    ret = dspLS(0, "comDial");
    CHECK(ret >= 0, lblKO);        
    ret = dspLS(2, ptr + 5);
    CHECK(ret >= 0, lblKO);
    ret = comDial(ptr);
    CHECK(ret >= 0, lblKO);

    prtS("comDial OK");
    ret = dspLS(0, "comDial OK");
    CHECK(ret >= 0, lblKO);        

    prtS("comSend");
    ret = dspLS(0, "comSend...");
    CHECK(ret >= 0, lblKO);        
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf)+1);
    CHECK(ret == (bufLen(&buf)+1), lblKO);

    prtS("comSend OK");
    ret = dspLS(0, "comSend OK");
    CHECK(ret >= 0, lblKO); 
    
    bufReset(&buf);
    prtS("comRecv");
    ret = dspLS(0, "comRecvBuf...");
    CHECK(ret >= 0, lblKO); 
    ret = comRecvBuf(&buf, 0, 200);
    CHECK(ret >= 0, lblKO);

    prtS("comRecv OK");
    ret = dspLS(0, "comRecv OK");
    CHECK(ret >= 0, lblKO); 

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
