/// \file tcik0105.c

/** UTA Unitary test case of barcode reader usage.
 * \sa
 *  - bcrStart()
 *  - bcrReq()
 *  - bcrGet()
 *  - bcrStop()
 */

#include <unicapt.h>
#include <string.h>
#include <stdio.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcik0105(void) {

#ifdef __BCR__
    int ret;
    char buf[256];
    char dateTime[12 + 1];
    byte idx = 0;

    nvmStart();
    prtStart();
    prtS("--TCIK0105--");
    dspStart();
    cnvStart();
    dspLS(1, "Start tcik0105");

    while(42) {
        ret = getDateTime(dateTime);
        prtS(dateTime);
        ret = bcrStart();
        if(ret != RET_OK)
            goto lblEnd;
        prtS(" bcr start ok");
        dspLS(1, "Please wait");
        bcrReq();
        memset(buf, '\0', 256);
        ret = bcrGet(buf);
        if(ret < 0)
            goto lblEnd;
        if(ret == 0) {
            prtS("No bar code");
            dspLS(1, "No bar code");
            goto lblEnd;
        }
        prtS(" bcr get :");
        prtS(buf);
        dspLS(1, "Bar code :");
        dspLS(2, buf);
        bcrStop();
        switch (idx) {
          case 0:
              //case 1:
              //prtS("wait 1 min");
              tmrSleep(6000);
              break;
              /*
                 case 2:
                 case 3:
                 prtS("wait 5 min");
                 tmrSleep(30000);
                 break;
                 case 4:
                 case 5:
                 prtS("wait 10 min");
                 tmrSleep(60000);
                 break;
               */
          default:
              goto lblEnd;
        }
        idx++;
    }
    tmrPause(1);
  lblEnd:
    bcrStop();
    cnvStop();
    dspStop();
    prtS(" ");
    prtS(" ");
    prtS("____fin test____");
    prtStop();
#endif
}
