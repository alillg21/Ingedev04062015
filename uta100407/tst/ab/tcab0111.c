/** \file
 * Unitary test case tcab0111.
 * Functions testing:
 * \sa
 *  - ftpStart()
 *  - ftpStop()
 *  - ftpDial
 *  - ftpCD()
 *  - ftpGet()
 *  - ftpPut()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0111.c $
 *
 * $Id: tcab0111.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __CHN_FTP__

int testIngenicoFtp(void) {
    int ret;
    char buf[256];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "ftpDial Ing...");
    CHECK(ret >= 0, lblKO);
    //ret= ftpDial("192.168.53.50", "blacklist", "download");
    ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6", 0);
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "ftpDial OK");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("Incoming");
    ret = dspLS(2, "ftpCD Incoming");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("utatest");
    ret = dspLS(3, "ftpCD utatest");
    CHECK(ret >= 0, lblKO);

    /*
       nvmRemove(0x10);
       ret = ftpGet(0x10, "test.txt");
       CHECK(ret > 0, lblKO);
       ret = dspLS(0, "ftpGet OK");
       CHECK(ret >= 0, lblKO);
     */

    ret = getDateTime(buf);
    CHECK(ret >= 0, lblKO);
    ret = nvmSave(0x10, buf, 0, (word) strlen(buf));
    CHECK(ret >= 0, lblKO);

    ret = ftpPut(0x10, "ftpPut.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "ftpPut OK");
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

int testDvkFtp(void) {
    int ret;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "ftpDial Dvk...");
    CHECK(ret >= 0, lblKO);
    ret = ftpDial("91.121.18.221", "ingenico", "fedbo9d7", 0);
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "ftpDial OK");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("temp");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "ftpCD temp");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpGet file 1");
    CHECK(ret >= 0, lblKO);
    nvmRemove(0x11);
    ret = ftpGet(0x11, "test.nvm");

    CHECK(ret > 0, lblKO);
    ret = dspLS(3, "ftpGet OK");
    CHECK(ret >= 0, lblKO);

    ret = ftpPut(0x11, "list.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(3, "ftpPut OK");
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0111(void) {
    int ret;
    char *ptr;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "nvmStart");
    CHECK(ret >= 0, lblKO);
    nvmStart();
    nvmRemove(0x11);
    nvmSave(0x11, "LALALA", 0, 6);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);

    ret = 1;
    switch (ret) {
      case 0:                  //TCP IP
          ret = comStart(chnTcp5100);
          CHECK(ret >= 0, lblKO);
          ret = comSet("");
          CHECK(ret >= 0, lblKO);
          ret = dspLS(0, "comSet OK");
          CHECK(ret >= 0, lblKO);
          break;
      case 1:                  //GPRS
          ret = comStart(chnGprs);
          CHECK(ret >= 0, lblKO);
          ptr = "0000|orange.fr||0000|";
          tmrPause(1);
          ret = comSet(ptr);
          CHECK(ret >= 0, lblKO);

          ptr = "91.121.18.221|6789";
          ret = comDial(ptr);
          CHECK(ret >= 0, lblKO);
          ret = dspLS(0, "comDial OK");
          CHECK(ret >= 0, lblKO);
      default:
          break;
    }

    // Create a new FTP client session
    ret = dspLS(1, "ftpStart...");
    CHECK(ret >= 0, lblKO);
    ret = ftpStart();
    CHECK(ret > 0, lblKO);
    ret = dspLS(0, "ftpStart OK");
    CHECK(ret >= 0, lblKO);

    //ret = testIngenicoFtp();
    //CHECK(ret >= 0, lblKO);
    ret = testDvkFtp();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    ftpStop();
    ret = comHangStart();
    trcFN("comHangStart : ret=%d\n", ret);
    ret = comHangWait();
    trcFN("comHangWait : ret=%d\n", ret);
    comStop();
    prtStop();
    dspStop();
}
#else
void tcab0111(void) {
}
#endif
