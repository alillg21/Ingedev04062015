#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcab0039.
//Connection.
//Building the request.
//Launching transport layer engine.
//Parsing response.
//Disconnection.

#define CHK CHECK(ret>=0,lblKO)

static int prepareData(tBuffer * bReq) {
    int ret;

    ret =
        bufApp(bReq,
               (byte *)
               "AENDPARAMETERS00050040045050@VERSION  5.30@15/06/04 09:30EMV",
               0);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcnl0003(void) {
    int ret;
    char *chn = "8N11200";
    char *srv = "5739374";
    tBuffer bReq;
    byte dReq[256];

    bufInit(&bReq, dReq, 256);
    ret = prepareData(&bReq);
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comConnect...");
    CHECK(ret >= 0, lblKO);

    ret = comConnect(0, chn, srv);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "DNLTle...");
    CHECK(ret >= 0, lblKO);

    ret = tleDnl(PARAM);        //perform DNL request and get response
    CHECK(ret >= 0, lblKO);

    //save in db
    ret = saveDB(PARAM);
    CHECK(ret >= 0, lblKO);

    ret = tleDnl(BINPARAM);     //perform DNL request and get response
    CHECK(ret >= 0, lblKO);

    //save in db
    ret = saveDB(BINPARAM);
    CHECK(ret >= 0, lblKO);

    mapPutByte(regDnlPerformed, 1);

    ret = dspLS(2, "Disconnect...");
    CHECK(ret >= 0, lblKO);

    ret = comDisconnect(0, chn, srv);
    CHECK(ret >= 0, lblKO);

    // tmrPause(3);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    // tmrPause(3);
    comDisconnect(0, chn, srv);
  lblEnd:
    tmrStop(1);
}

#endif
