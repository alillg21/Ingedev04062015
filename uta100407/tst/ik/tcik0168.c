/** \file
 * UTA test case for integration of cmm module(tcik0168).
 * Functions testing for GCL Asyn modem communication; receiving of data with and without terminators:
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comSendBuf()
 *  - comRecvBuf()
 *  - comStop()
 */

#include <string.h>
#include "tst.h"
#include "sys.h"

#ifdef __CMM__

static card hHmi = 0;
static void tc_show(const char *str, int idx) {
    static word row = 0;

    if(idx == 0)
        row = 0;
    hmiADClearLine(hHmi, row);
    hmiADDisplayText(hHmi, row, 0, (char *) str);
    row += 1;
    if(row >= 15)
        row = 0;
}

void tcik0168(void) {
    int ret;
    char text[16 + 1];
    byte connType;
    char *ptr;
    tBuffer buf;
    byte dat[256];
    byte b;

    hmiOpen("DEFAULT", &hHmi);
    tc_show("fill parameters", 1);

    connType = 1;
    switch (connType) {
      case 0:                  //modem
          ret = comStart(chnMdm);
          CHECK(ret >= 0, lblKO);
          ptr = "8N119200";
          ret = comSet(ptr);
          CHECK(ret >= 0, lblKO);
          ptr = "0-0836065555"; //France
          break;
      case 1:                  //ethernet
          ret = comStart(chnTcp5100);
          CHECK(ret >= 0, lblKO);
          //ptr = "10.110.1.125|9000|10.110.1.1|225.255.255.0";
          ptr = "";
          ret = comSet(ptr);
          CHECK(ret >= 0, lblKO);
          ptr = "10.110.1.104|9702";
          //ptr = "127.0.0.1|5001";
          break;
      default:
          goto lblKO;
    }

    ret = comDial(ptr);
    if(ret < 0) {
        switch (-ret) {
          case comAnn:
              tc_show("comDial Aborted", 2);
              break;
          case comBusy:
              tc_show("comDial BUSY", 2);
              break;
          case comNoDialTone:
              tc_show("NO TONE", 2);
              break;
          case comNoCarrier:
              tc_show("NO CARRIER", 2);
              break;
          default:
              tc_show("UNKNOWN PB", 2);
              break;
        }
        goto lblEnd;
    }

    tc_show("comDial OK", 2);

    memcpy(text, "testCC", 6 + 1);
    ret = comSendBuf((byte *) text, strlen(text));
    memset(text, 0, sizeof(text));
    sprintf(text, "comSend=%d", ret);
    tc_show(text, 3);
    CHECK(ret >= 0, lblKO);

    tc_show("Receiving byte", 1);
    ret = comRecv(&b, 20);
    memset(text, 0, sizeof(text));
    sprintf(text, "comRecv=%d", ret);
    tc_show(text, 1);
    CHECK(ret >= 0, lblKO);

    tc_show("Receiving buf", 1);
    bufInit(&buf, dat, 256);
    ret = comRecvBuf(&buf, (byte *) "\x00", 10);
    memset(text, 0, sizeof(text));
    sprintf(text, "comRecv=%d", ret);
    tc_show(text, 1);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    tc_show("tcik0168 KO", 0);
  lblEnd:
    ret = comStop();
    memset(text, 0, sizeof(text));
    sprintf(text, "cmmEnd=%d", ret);
    tc_show(text, 0);
    tc_show("tcik0168 End", 0);
    hmiClose(hHmi);
    return;
}
#else
void tcik0168(void) {
}
#endif
