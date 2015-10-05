// tcgs0016
//
// author:      Gábor Sárospataki
// date:        01.02.2006
// description: test case to enterTxt3 function

#include "pri.h"
#include <string.h>
#define CHK CHECK(ret>=0,lblKO)

void tcgs0016(void) {
    int ret;
    char str[3 * dspW + 1];
    char buf[dspW + 1];
    int idx;

    strcpy(str, "I am the editalbe string.");
    ret = dspStart();
    CHK;
    ret = dspClear();
    CHK;
    ret = dspLS(0, "Edit the string:");
    CHK;
    ret = enterTxt3(1, str, 3 * dspW, 0xff);
    CHK;
    switch (ret) {
      case kbdVAL:
          ret = prtStart();
          CHK;
          ret = dspClear();
          CHK;
          ret = dspLS(1, "   Printing...");
          ret = prtS("The new string:");
          CHK;
          for (idx = 0; idx < (int) strlen(str); idx += dspW) {
              memset(buf, 0, sizeof(buf));
              strncpy(buf, str + idx, dspW);
              ret = prtS(buf);
              CHK;
          }
          //feed lines
          ret = prtS(" ");
          CHK;
          ret = prtS(" ");
          CHK;
          ret = prtS(" ");
          CHK;
          prtStop();
          break;
      case kbdANN:
          ret = dspClear();
          CHK;
          ret = dspLS(0, "Cancelled");
          tmrPause(2);
          break;
      case 0:
          ret = dspClear();
          CHK;
          ret = dspLS(0, "Timeout");
          tmrPause(2);
          break;
      default:
          goto lblKO;
    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    Beep();
    tmrPause(2);
  lblEnd:
    dspStop();
    prtStop();
}
