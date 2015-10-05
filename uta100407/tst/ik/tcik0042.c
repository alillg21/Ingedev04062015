#include <string.h>
//#include <unicapt.h>
#include "def.h"
//#include "pri.h"
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0042.
//Functions testing FTP via GPRS connection with chn.c:
//  ftpStart
//  ftpStop
//  ftpDial
//  ftpCD
//  ftpGet
//  ftpPut

void tcik0042(void) {
    int ret;
    char *ptr;

    //char buf[15+1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);

    ret = comStart(chnGprs);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);

    ptr = "0000|orange.fr|danfr|mq36noqt|";
    //ptr= "5670|corpmts064|mts|064|";
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    /* Create a new FTP client session */
    ret = dspLS(1, "ftpStart...");
    CHECK(ret >= 0, lblKO);

    ret = ftpStart();
    CHECK(ret > 0, lblKO);

    ret = dspLS(1, "ftpStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpDial..");
    CHECK(ret >= 0, lblKO);

    //ret= ftpDial("192.168.53.50", "blacklist", "download",0);
    //ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6",0);
    //ret = ftpDial("172.19.49.10", "FPLUS\\ing_test", "Cmtch2006",0);//for info "\" must be "\\"
    ret = ftpDial("82.247.161.69", "ingenico", "fedbo9d7", 0);
    //ret = ftpDial("81.80.165.104", "soner", "ve2mi1xo6",0);
    CHECK(ret > 0, lblKO);

    ret = dspLS(2, "ftpDial OK");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("Incoming");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpCD Incoming");
    CHECK(ret >= 0, lblKO);

    /*
       ret = ftpCD("utatest");
       ret = dspLS(3, "ftpCD utatest");
       CHECK(ret >= 0, lblKO);
     */

    ret = ftpGet(16, "lalala.txt");
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
  lblEnd:
    ftpStop();
    tmrPause(3);
    comStop();
    prtStop();
    dspStop();
}
