// tcgs0023
//
// author:      Gábor Sárospataki
// date:        01.03.2007
// description: test case to modified enterCard funcion (bcr added)

#include "pri.h"
#include <string.h>

#define CHK CHECK(ret>=0,lblKO)

void tcgs0023(void) {
#ifdef __BCR__
    char buf[3 * 128];
    int ret;
    char ent;

    memset(buf, 0, sizeof(buf));

    ret = bcrStart();
    CHK;                        // must be started before the enter card

    ret = dspStart();
    CHK;
    ret = dspClear();
    CHK;
    ret = dspLS(0, "Mag card, key or bar code");
    ret = ent = enterCard((byte *) buf, "bmk");
    CHK;
    Click();
    switch (ent) {
      case 'm':
          dspLS(1, "Card was swipped");
          break;
      case 'k':
          dspLS(1, "Numeric key was pressed:");
          dspLS(2, buf);
          break;
      case 'b':
          dspLS(1, "Bar code was readed:");
          dspLS(2, buf);
          break;
      case 0:
          dspLS(1, "Aborted by user!");
          break;
      default:
          break;
    }
    goto lblEnd;
  lblKO:
    Beep();
    Beep();
    Beep();
    dspLS(0, "KO!");
    goto lblEnd;
  lblEnd:
    tmrPause(5);
    dspStop();
    bcrStop();
    return;
#endif
}
