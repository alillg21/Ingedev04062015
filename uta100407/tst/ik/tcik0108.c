/// \file tcik0108.c

/** UTA Unitary test case of barcode reader usage.
 * \sa
 *  - bcrStart()
 *  - bcrRead()
 *  - bcrStop()
 */

#include <unicapt.h>
#include <string.h>
#include <stdio.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcik0108(void) {

#ifdef __BCR__
    int ret;
    char buf[256];
    char dateTime[12 + 1];
    char *ptr;

    memset(buf, '\0', 256);
    nvmStart();
    prtStart();
    prtS("--TCIK0108--");
    dspStart();
    cnvStart();
    dspLS(1, "Start tcik0108");

    ret = getDateTime(dateTime);
    prtS(dateTime);
    ret = bcrStart();
    if(ret != RET_OK)
        goto lblEnd;
    prtS(" bcr start ok");

    ret = 5;

    switch (ret) {
      case 0:
          dspLS(1, "Read normal BC");
          ptr = NULL;
          break;
      case 1:
          dspLS(1, "Please wait");
          dspLS(2, "Read  code 39");
          ptr = "PRECA2;PREBK201;C39ENA1.";
          break;
      case 2:
          dspLS(1, "Please wait");
          dspLS(2, "Read PDF417");
          ptr = "PRECA2;PREBK202;PDFDFT;MPDDFT.";
          break;
      case 3:
          dspLS(1, "Please wait");
          dspLS(2, "Read Barcode");
          ptr = "PRECA2;PREBK201;CBRENA1.";
          break;
      case 4:
          dspLS(1, "Please wait");
          dspLS(2, "Read Code 49");
          ptr = "PRECA2;PREBK201;C49DFT.";
          break;
      case 5:
          dspLS(1, "Please wait");
          dspLS(2, "Read Aztec");
          ptr = "PRECA2;PREBK201;AZTDFT.";
          break;
      case 6:
          dspLS(1, "Please wait");
          dspLS(2, "Read QR Code");
          ptr = "PRECA2;PREBK201;QRCENA1.";
          break;
      default:
          goto lblEnd;
    }

    ret = bcrRead(ptr, 50, buf);
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
