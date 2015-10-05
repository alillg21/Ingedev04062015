
#include <stdio.h>
#include <unicapt.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "tst.h"

//unitary test case
//Paper detect

static uint32 hPrt = 0;

static int printStart(void) {
    int ret;

    trcFS("%s\n", "prtStart");
    //ret = selectFontPrt();
    //CHECK(ret >= 0, lblKO);

    ret = prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int printStop(void) {
    int ret;

    trcFS("%s\n", "prtStop");
    ret = prnClose(hPrt);
    hPrt = 0;
    return hPrt;
}

static int PAPER_PB(void) {
    int ret;
    char key;

    dspClear();
    dspLS(0, "PLEASE ADD NEW PAPER");
    ret = tmrStart(0, 500);
    CHECK(ret >= 0, lblKO);

    ret = kbdStart(1);          //start keyboard waiting
    CHECK(ret >= 0, lblKO);

    key = 0;
    while(tmrGet(0)) {
        key = kbdKey();         //retrieve a key if pressed
        CHECK(key != '\xFF', lblKO);    //error during key waiting
        if(key)
            break;              //quit the loop if a key pressed
    }

    ret = kbdStop();            //stop keyboard waiting
    CHECK(ret >= 0, lblKO);
    tmrStop(0);
    return 1;
  lblKO:
    return -1;
}

static int printString(const char *str) {
    int ret;
    int idx;
    char preChar;
    byte maxLength = prtW;      //maximum number of printable character
    byte numberNorm = 0;        //number of normal characters (only they are printed)
    char buf[48 + 1 + 40];      //40 control character enabled in half size mode

    VERIFY(str);
    trcFS("prtS str=%s\n", str);

    memcpy(buf, str, sizeof(buf));

//first two characters must be the size control character
    if(buf[0] == 0x1b && buf[1] >= 0x1b && buf[1] <= 0x21 && strlen(buf) > 1) {
        switch (buf[1]) {
          case 0x1c:
          case 0x1e:
              maxLength = 12;
              break;            //double size
          case 0x1f:
              maxLength = 6;
              break;            //quarddouble size
          case 0x20:
              maxLength = 48;
              break;            //half size (48 columns)
          case 0x21:
              maxLength = 42;
              break;            //42 columns
          case 0x1d:
              maxLength = 24;
              break;            //24 columns
          case 0x1b:           //normal size (it may be needed when the default isn't normal)
              maxLength = 24;
              for (idx = 0; idx < sizeof(buf) - 2; idx++)
                  buf[idx] = buf[idx + 2];
              break;
        }
    } else {
        switch (prtW) {
          case 6:
              preChar = 0x1f;
              break;
          case 12:
              preChar = 0x1e;
              break;
          case 24:
              preChar = 0;
              break;
          case 42:
              preChar = 0x21;
              break;
          case 48:
              preChar = 0x20;
              break;
          default:
              goto lblKO;       //unexpected printing default size
        }
        maxLength = prtW;
        if(preChar) {
            for (idx = sizeof(buf) - 1; idx >= 2; idx--)
                buf[idx] = buf[idx - 2];
            buf[0] = 0x1b;
            buf[1] = preChar;
        }
    }

    for (idx = 0; numberNorm < maxLength && buf[idx] != 0 && idx < sizeof(buf);
         idx++) {
        if(buf[idx] < 0x17 || buf[idx] > 0x21)
            numberNorm++;
        // 0x20, 0x21 and 0x22 can be control and normal character as well
        if(buf[idx] == 0x20 || buf[idx] == 0x21 || buf[idx] == 0x22) {
            if(idx != 0) {
                if(buf[idx - 1] != 0x1b)
                    numberNorm++;
            } else
                numberNorm++;
        }
    }

    memset(buf + idx, 0, sizeof(buf) - idx);
    memset(buf + sizeof(buf) - 1, 0, 1);    //turncate the last character always

    ret = prnPrint(hPrt, buf);
    CHECK(ret == RET_OK, lblKO);

    return strlen(buf);
  lblKO:
    return -1;
}

static void OnPrnctrl(void) {
    int i;
    prnControlReq_t PtReq;
    prnControlRes_t PtRes;
    int16 retCode;

    for (i = 0; i < 10; i++) {
        PtReq.type = PRN_CTRL_STATUS_PAPER;
        PtRes.type = PRN_CTRL_STATUS_PAPER;
        prnControl("DEFAULT", &PtReq, &PtRes);

        switch (PtRes.Res.StatusPaper) {
              dspClear();
          case PRN_CTRL_PAPER_DETECT:
              dspLS(0, "PAPER DETECTED");
              retCode = printString("1111111111");
              if(retCode < 0) {
                  dspLS(0, "Printing error");
                  psyTimerWakeAfter(100);
                  PAPER_PB();
              }
              prnControl("DEFAULT", &PtReq, &PtRes);
              if(PtRes.Res.StatusPaper == PRN_CTRL_PAPER_DETECT) {
                  dspLS(0, "PAPER DETECTED");
                  PtReq.type = PRN_CTRL_STATUS_LEVEL_PAPER;
                  PtRes.type = PRN_CTRL_STATUS_LEVEL_PAPER;
                  prnControl("DEFAULT", &PtReq, &PtRes);
                  switch (PtRes.Res.RegValue) {
                    case 'y':
                        dspLS(0, "PAPER IS LOW");
                        break;
                    case 'n':
                        dspLS(0, "PAPER IS NOT LOW");
                        break;
                    case 'a':
                        dspLS(0, "PAPER ABSENT");
                        break;
                    case '?':
                        dspLS(0, "UNKNOWN STATUS");
                        break;
                    default:
                        dspLS(0, "UNDEFINED");
                        break;
                  }
              } else {
                  dspLS(0, "PAPER NOT DETECT");
                  psyTimerWakeAfter(200);
                  PAPER_PB();
              }
              break;
          case PRN_CTRL_PAPER_NOT_DETECT:
              dspLS(0, "PAPER NOT DETECT");
              psyTimerWakeAfter(200);
              break;
          case PRN_CTRL_PAPER_DETECT_UNAVAILABLE:
              dspLS(0, "PAPER UNAVAILABLE");
              psyTimerWakeAfter(200);
              break;
        }
    }
  lblEnd:
    return;
}

void tcik0069(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = printStart();
    CHECK(ret >= 0, lblKO);

    OnPrnctrl();                //Conttrol if paper is present before printing
    printString("Test finished");

    tmrPause(1);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(2);
    printStop();
    dspStop();
}
