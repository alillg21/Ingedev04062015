#include <string.h>
#include "sys.h"
#include "tst.h"
#include <LNet.h>
//#include <LNetDns.h>
//#include <LNetTcpip.h> 
#include <LNetSocket.h>
#include <LNet.h>
#include <LNetTcpip.h>

static code const char *srcFile = __FILE__;

//Unitary test case tcnl0020.
//Functions testing for ethernet:
//  comStart
//  comSet
//  comStop
//  comDial
//  comHangStart
//  comHangWait
//  comSendBuf
//  comRecvBuf

//use for static address and if terminal is a server

void tcnl0020(void) {
    int ret, retErr;
    char *ptr;
    byte *msg = (byte *) "HELLO!";
    tBuffer buf;
    byte dat[128];
    char myBuf[100] = "";
    char msgbuf[100] = "";
    int retErr;

    //  ret= dspStart(); CHECK(ret>=0,lblKO);
    // ret= prtStart(); CHECK(ret>=0,lblKO);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnTcp5100);
    CHECK(ret >= 0, lblKO);

    strcpy(myBuf, "192.1.1.2-8000-255.255.255.0");
    ret = comSet(myBuf);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comListen...");
    CHECK(ret >= 0, lblKO);
    ptr = "192.1.1.2-8000-255.255.255.0";   //NL
    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comListen...");
    ret = comListen(ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comListen done");

    ret = dspLS(1, "comListen OK");
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = comAccept(ptr);
    CHECK(ret >= 0, lblKO);
    prtS("comAccept OK");

    bufInit(&buf, dat, 128);
    ret = comRecvBuf(&buf, (byte *) "!", 20);
    CHECK(ret >= 0, lblKO);
    CHECK(ret == 6, lblKO);

    prtS((char *) msg);
    ret = comSendBuf(msg, 6);
    CHECK(ret == 6, lblKO);

    tmrPause(1);

    ret = dspLS(2, (char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    comStop();
    comCloseNet();

    tmrPause(3);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comStop();
    comCloseNet();
    //  prtStop();
    // dspStop();
}
