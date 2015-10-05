#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcab0037.
//Building the request.

#define CHK CHECK(ret>=0,lblKO)
/*static int prepare01(void){
    int ret;
    ret= mapPutWord(traMnuItm,mnuSale); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPutStr(traPan,"5425364009408223"); CHK;
    ret= mapPutStr(traExpDat,"0512"); CHK;
    ret= mapPutStr(traAmt,"234"); CHK;
    ret= mapPutWord(regMsgNum,1); CHK;
    ret= mapPut(traDiaInd,"1",1); CHK;
    return 1;    
lblKO:
    trcErr(ret);
        return -1;

}
*/

static int printReqDnl(const char *hdr, const char *req, word len) {

    int ret;
    char tmp[prtW + 1];

    ret = prtS(hdr);
    CHK;
    tmp[prtW] = 0;
    while(len) {
        if(strlen(req) <= prtW) {
            strcpy(tmp, req);
            len = 0;
        } else {
            memcpy(tmp, req, prtW);
            len -= prtW;
            req += prtW;
        }
        ret = prtS(tmp);
        CHK;
    }
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcnl0001(void) {
    int ret;
    byte dReq[256];
    tBuffer bReq;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "prepare01...");
    CHECK(ret >= 0, lblKO);

    //Non EMV
    //  ret= prepare01(); CHECK(ret>=0,lblKO);

    ret = dspLS(1, "build01...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bReq, dReq, 256);
    ret = reqDnl(&bReq);
    CHECK(ret >= 0, lblKO);

    trcFS("req=[%s]\n", (char *) bufPtr(&bReq));
    ret = printReqDnl("  request01", (char *) bufPtr(&bReq), bufLen(&bReq));
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    // tmrPause(3);
  lblEnd:
    tmrStop(1);
}
#endif
