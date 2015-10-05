#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcab0178.
//Testing gclSck and FTP

void tcab0178(void) {
    int ret;
    byte tmp[256];
    const char *ptr;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclSck, "");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "ftpStart...");
    CHECK(ret >= 0, lblKO);
    ret = ftpStart();
    CHECK(ret > 0, lblKO);
    ret = dspLS(0, "ftpStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpDial..");
    CHECK(ret >= 0, lblKO);
    ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6", 0);
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpDial OK");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("Incoming");
    ret = dspLS(2, "ftpCD Incoming");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("utatest");
    ret = dspLS(3, "ftpCD utatest");
    CHECK(ret >= 0, lblKO);

    nvmRemove(0x10);
    ret = ftpGet(0x10, "test.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpGet OK");
    CHECK(ret >= 0, lblKO);

    ret = getDateTime((char *) tmp);
    CHECK(ret >= 0, lblKO);
    strcat((char *) tmp, ": tcab0178");
    ret = nvmSave(0x10, tmp, 0, (word) strlen((char *) tmp));
    CHECK(ret >= 0, lblKO);

    ret = ftpPut(0x10, "ftpPut.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpPut OK");
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
    prtStop();
    dspStop();
}
