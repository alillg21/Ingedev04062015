#include <string.h>
#include "sys.h"
#include "tst.h"

//static code const char *srcFile= __FILE__;

//Unitary test case tcik0086.
//Functions testing GCLSend/Receive via Socket connection with GCL

void tcik0086(void) {
    int ret;
    char *ptr;

    //char Text[20+1];
    byte dReq[1024];

    //tBuffer bReq;
    //int idx;
    card len;

    //byte b;
    card dly;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "gclPgSupReset ...");
    CHECK(ret >= 0, lblKO);
    ret = gclPgSupReset();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "gclReset ...");
    CHECK(ret >= 0, lblKO);
    ret = gclReset();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Gcl conn ...");
    CHECK(ret >= 0, lblKO);

    ptr = "0000|orange.fr|0|0|212.27.42.12|110";
    ret = dspLS(4, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    ret = utaGclStart(gclSck, ptr);
    prtS("utaGclStart OK");
    CHECK(ret >= 0, lblKO);

    dly = 60;
    ret = dspLS(0, "gclPgSupAttach ...");
    CHECK(ret >= 0, lblKO);
    ret = gclPgSupAttach(dly);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "gclStart ...");
    CHECK(ret >= 0, lblKO);
    ret = gclStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "gclStartConnection ...");
    CHECK(ret >= 0, lblKO);
    ret = gclStartConnection();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "gclWaitConnection ...");
    CHECK(ret >= 0, lblKO);
    ret = gclWaitConnection();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Gcl conn OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "gclSend..");
    CHECK(ret >= 0, lblKO);
    ret = gclSend("stat\x0D\x0A", 6);
    CHECK(ret >= 0, lblKO);
    prtS("gclSend OK");

    len = 100;
    ret = gclReceive(dReq, len, &len);  //receive 
    CHECK(ret >= 0, lblKO);
    ret = prtS("response:");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) dReq);
    CHECK(ret >= 0, lblKO);

    prtS("gclReceive OK");
    trcS("\n");
    ret = dspLS(1, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    prtS("KO");
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(1);
    utaGclStop();
    prtS("utaGclStop OK");
    prtStop();
    dspStop();
}
