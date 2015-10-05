#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0045.
//Connectivity testing for Europlus
//#define __SALE__
//#define __VOID__
//#define __AUTHORISATION__

//Building requests - authorisation

#define CHK if(ret<0) return ret;

static int build01(tBuffer * buf) {
/*
0100
0011000000100000000001011000000000100000110000000000000000000100

(3) Processing Code                   00 00 00
(4) Transaction Amount                5000
(11) System Trace Audit Number        000001
(22) PAN Entry Mode/PIN Entry Cap     022
(24) Network ID                       123
(25) POS Condition Code               00
(35) Track II                         4054791810030115=07041211929752200000
(41) Card Acceptor Terminal ID        INGE001B
(42) Crd Accpt/Merchant ID            TRG00000INGEFFB
(62)       6 006452
*/
    int ret;

    ret = bufApp(buf, (byte *) "\x60\x01\x23\x00\x00", 5);  //\x60\x01\x23\x00\x00
    CHK;                        // TPDU

#ifdef __AUTHORISATION__
    ret = bufApp(buf, (byte *) "0100", 4);
    CHK;                        // Message Type 0100 Auth request
    ret = bufApp(buf, (byte *) "3020058020C00004", 16);
    CHK;                        //(3,4)(11)(22,24)(25)(35)(41,42)(62)
    ret = bufApp(buf, (byte *) "000000", 6);
    CHK;                        // 03 - Procs Code
    ret = bufApp(buf, (byte *) "000000005000", 12);
    CHK;                        // 04 - Amount
    ret = bufApp(buf, (byte *) "000001", 6);
    CHK;                        // 11 - STAN
    ret = bufApp(buf, (byte *) "022", 3);
    CHK;                        // 22 - POS Entry Mode
    ret = bufApp(buf, (byte *) "123", 3);
    CHK;                        // 24 - Network International identifier
    ret = bufApp(buf, (byte *) "00", 2);
    CHK;                        // 25 - POS condition code
    ret = bufApp(buf, (byte *) "4054791810030115=07041211929752200000", 32);
    CHK;                        // 35 - ISO2 Track
    ret = bufApp(buf, (byte *) "INGE001B", 8);
    CHK;                        // 41 - Card Acceptor Terminal Id Terminal ID: INGE001B
    ret = bufApp(buf, (byte *) "TRG00000INGEFFB", 15);
    CHK;                        // 42 - Card Acceptor Id Merchant ID: TRG00000INGEFFB
    ret = bufApp(buf, (byte *) "978", 3);
    CHK;                        // 49 - Currency code
#endif                          // __AUTHORISATION__

#ifdef __SALE__
    ret = bufApp(buf, (byte *) "0200", 4);
    CHK;                        // Message Type 0100 Auth request
    ret = bufApp(buf, (byte *) "3020050000C00000", 16);
    CHK;                        //(3,4)(11)(22,24)()()(41,42)()()
    ret = bufApp(buf, (byte *) "000000", 6);
    CHK;                        // 03 - Procs Code
    ret = bufApp(buf, (byte *) "000000005000", 12);
    CHK;                        // 04 - Amount
    ret = bufApp(buf, (byte *) "000001", 6);
    CHK;                        // 11 - STAN
    ret = bufApp(buf, (byte *) "022", 3);
    CHK;                        // 22 - POS Entry Mode
    ret = bufApp(buf, (byte *) "123", 3);
    CHK;                        // 24 - Network International identifier
    ret = bufApp(buf, (byte *) "00", 2);
    CHK;                        // 25 - POS condition code
    ret = bufApp(buf, (byte *) "4054791810030115=07041211929752200000", 32);
    CHK;                        // 35 - ISO2 Track
    ret = bufApp(buf, (byte *) "INGE001B", 8);
    CHK;                        // 41 - Card Acceptor Terminal Id Terminal ID: INGE001B
    ret = bufApp(buf, (byte *) "TRG00000INGEFFB", 15);
    CHK;                        // 42 - Card Acceptor Id Merchant ID: TRG00000INGEFFB
    ret = bufApp(buf, (byte *) "978", 3);
    CHK;                        // 49 - Currency code
#endif                          // __SALE__

/*
    //062
    ret = bufApp(buf, (byte *) "0004", 3);
    CHK;                        // Length Attribute
    ret = bufApp(buf, (byte *) "62", 2);
    CHK;                        // SubField ID
    ret = bufApp(buf, (byte *) "00", 6);
    CHK;                        // SubField Data: Batch number
*/
    return 1;
}

void tcik0045(void) {
    int ret;
    byte dReq[256];
    tBuffer bReq;
    char *ptr;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    bufInit(&bReq, dReq, 256);
    ret = dspLS(1, "build01...");
    ret = build01(&bReq);
    CHECK(ret >= 0, lblKO);
    trcFS("bReq: %s\n", bufPtr(&bReq) + 5);

    ptr = "8N11200|0-00302103397090";   // 2 Numbers 0030 210 3397846 and 0030 210 3397090
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclHdlc, ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclHdlcDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);
    do {
        ret = utaGclDialInfo();
        switch (ret) {
          case gclStaPreDial:
              ptr = "PreDial";
              break;
          case gclStaDial:
              ptr = "Dial";
              break;
          case gclStaConnect:
              ptr = "Connect";
              break;
          case gclStaLogin:
              ptr = "Login";
              break;
          case gclStaFinished:
              ptr = "Done";
              break;
          default:
              goto lblKO;
        }
        dspLS(2, ptr);
    } while(ret != gclStaFinished);
    ret = dspLS(2, "utaGclHdlcDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "utaGclHdlcSend..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclSendBuf(bufPtr(&bReq), bufLen(&bReq));
    CHECK(ret == bufLen(&bReq), lblKO);

    ret = dspLS(3, "utaGclHdlcRecv..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclRecvBuf(&bReq, 0, 255);

    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclHdlcStop");
    CHECK(ret >= 0, lblKO);
    ret = utaGclStop();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = utaGclDialErr();
    switch (ret) {
      case gclErrCancel:
          ptr = "User cancelled";
          break;
      case gclErrDial:
          ptr = "Dial error";
          break;
      case gclErrBusy:
          ptr = "BUSY";
          break;
      case gclErrNoDialTone:
          ptr = "NO DIAL TONE";
          break;
      case gclErrNoCarrier:
          ptr = "NO CARRIER";
          break;
      case gclErrNoAnswer:
          ptr = "NO ANSWER";
          break;
      case gclErrLogin:
          ptr = "LOGIN PBM";
          break;
      default:
          ptr = "KO!";
          break;
    }
    dspLS(0, ptr);

    goto lblEnd;

  lblEnd:
    tmrPause(3);
}
