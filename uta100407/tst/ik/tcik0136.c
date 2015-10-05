/** \file
 * Unitary test case tcik136.
 * Functions testing for GCL Asyn modem communication; receiving of data with and without terminators:
 * \sa
 *  - utaGclRecvBuf()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcik0136(void) {
    int ret, sav;
    char *ptr;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];

    byte trm[] = { 0x03, 0x0D, ',', 0 };
    char key;
    byte pat[256];

    bufInit(&buf, dat, 256);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    memset(msg, 0, dspW + 1);

    ptr = "8N12400|0-0146258134";   //NCC
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclAsyn, ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);
    sav = 0;
    key = 0;
    trcS("Start");
    do {
        ret = utaGclDialInfo();
        if(sav == ret)
            continue;
        sav = ret;
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
        key = kbdKey();
        trcFN("key=%02x\n", (word) key);
        CHECK(key >= 0, lblKO);
        if(key == 0)
            continue;
        if(key == kbdANN)
            break;
        kbdStop();
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);
    } while(ret != gclStaFinished);
    trcS("End");
    kbdStop();
    if(key == kbdANN) {
        ret = dspLS(2, "Aborted");
        CHECK(ret >= 0, lblKO);
        goto lblEnd;
    }
    ret = dspLS(2, "utaGclDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Send SET");
    CHECK(ret >= 0, lblKO);
    memset(pat, 0, sizeof(pat));
    strcpy((char *) pat, "SET 1:0,2:0,3:0,4:8,5:0\x0D\x0A\x0A");
    ret = utaGclSendBuf(pat, strlen((char *) pat));
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving buf");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    ret = utaGclRecvBuf(&buf, trm, 20);
    CHECK(ret >= 0, lblKO);
    prtS((char *) bufPtr(&buf));

    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "utaGclStop");
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
  lblEnd:
    tmrPause(3);
    utaGclStop();
    kbdStop();
    prtStop();
    dspStop();
}
