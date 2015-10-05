#include <string.h>
#include "sys.h"
#include "tst.h"
#include <LNetFtp.h>

//#ifdef __CHN_FTP__

//static uint32 hFtp;

//Unitary test case tcik0043.
//Functions testing FTP via GPRS connection with GCL:
//  ftpStart
//  ftpStop
//  ftpDial
//  ftpCD
//  ftpGet
//  ftpPut

void tcik0043(void) {
    int ret;
    char *ptr;

    //char tmp[4];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Gcl conn ...");
    CHECK(ret >= 0, lblKO);

    //ptr= "0000|orange.fr|danfr|mq36noqt|81.80.165.104|22";
    ptr = "0000|orange.fr|danfr|mq36noqt|082.247.161.069|6789";
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    ret = utaGclStart(gclGprs, ptr);
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Gcl conn OK");
    CHECK(ret >= 0, lblKO);

    /* Create a new FTP client session */
    ret = dspLS(1, "ftpStart...");
    CHECK(ret >= 0, lblKO);

    ret = ftpStart();
    CHECK(ret > 0, lblKO);

    ret = dspLS(1, "ftpStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpConnect..");
    CHECK(ret >= 0, lblKO);

    //ret= ftpDial("192.168.53.50", "blacklist", "download",0);
    //ret = ftpDial("172.19.49.10", "FPLUS\ing_test", "Cmtch2006",0);
    //ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6",0);

    ret = ftpDial("81.80.165.104", "soner", "ve2mi1xo6", 0);
    //ret = ftpDial("082.247.161.069", "ingenico", "fedbo9d7",0);

    CHECK(ret == RET_OK, lblKO);

    ret = dspLS(2, "ftpConnect OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);
    ret = ftpCD("test");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpCD test");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = ftpGet(16, "5V09999999002.dnl");
    CHECK(ret > 0, lblKO);

    ret = dspLS(2, "ftpGet OK");
    CHECK(ret >= 0, lblKO);
    /*
       //Before use this part, put ftpPut.txt into terminal
       ret = dspLS(2, "ftpPut ...");
       CHECK(ret >= 0, lblKO);

       ret = ftpPut(16, "ftpPut.txt");
       CHECK(ret > 0, lblKO);

       ret = dspLS(2, "ftpPut OK");
       CHECK(ret >= 0, lblKO);
     */
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    //tmrPause(3);
  lblEnd:
    ftpStop();
    tmrPause(3);
    utaGclStop();
    prtStop();
    dspStop();
}

//#endif
