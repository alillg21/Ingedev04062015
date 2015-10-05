#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0013.
//Emv Response
//Online call testing.

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(tBuffer * rsp) {
    int ret;
    byte us;
    byte fs;

    us = cUS;
    fs = cFS;

    ret = bufApp(rsp, (byte *) "0", 0);
    CHK;                        //Dial Indicator
    ret = bufApp(rsp, (byte *) "00050040", 0);
    CHK;                        //Terminal identity
    ret = bufApp(rsp, (byte *) "0004", 0);
    CHK;                        //Message number
    ret = bufApp(rsp, (byte *) "12", 0);
    CHK;                        //Message type identification
    ret = bufApp(rsp, (byte *) "00", 0);
    CHK;                        //Acquirer response code
    ret = bufApp(rsp, (byte *) "0", 0);
    CHK;                        //Confirmation request
    ret = bufApp(rsp, (byte *) "002423", 0);
    CHK;                        //Authorisation Code
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, (byte *) "1000", 0);
    CHK;                        //Amount
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, (byte *) "AUTH CODE:02610", 0);
    CHK;                        //Message
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS    
    ret = bufApp(rsp, (byte *) "5D92187339BBEFA5", 16);
    CHK;                        //ARPC
    ret = bufApp(rsp, (byte *) "3030", 4);
    CHK;                        //ARC   
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS
    ret = bufApp(rsp, &fs, 1);
    CHK;                        //FS    

    ret = mapPut(traEntMod, "c", 1);
    CHK;
    mapMove(acqBeg + 1, VISACARD);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static int printData(void) {
    int ret;
    char ARC[3] = "";
    char ARPC[20] = "";
    char IssAutDta[32] = "";
    char buf[256] = "";

    ret = prtS("RspCod:");
    CHK;
    ret = mapGet(traRspCod, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("CfmReq:");
    CHK;
    ret = mapGet(traCfmReq, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("AutCod:");
    CHK;
    ret = mapGet(traAutCod, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("SrvMsg:");
    CHK;
    ret = mapGet(traSrvMsg, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("EmvIssAutDta:");
    CHK;
    ret = mapGet(emvIssAutDta, buf, 256);
    CHK;
    bin2hex(IssAutDta, (byte *) buf + 1, 8);
    ret = prtS(IssAutDta);
    CHK;

    ret = prtS("EmvARPC:");
    CHK;
    bin2hex(ARPC, (byte *) buf + 1, 8);
    ARPC[16] = 0;
    ret = prtS(ARPC);
    CHK;

    ret = prtS("EmvIssuerRespCode:");
    CHK;
    bin2num((card *) ARC, (byte *) (buf + 1 + 8), 2);
    ARC[2] = 0;
    ret = prtS(ARC);
    CHK;

    ret = prtS("EmvSCRIPT1:");
    CHK;
    ret = mapGet(emvIssSc1, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("EmvSCRIPT2:");
    CHK;
    ret = mapGet(emvIssSc2, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcnl0013(void) {
    int ret;
    byte dRsp[256];
    tBuffer bRsp;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "EMV Response...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);

    ret = dspLS(0, "prepare01...");
    CHECK(ret >= 0, lblKO);
    ret = prepare01(&bRsp);
    CHECK(ret >= 0, lblKO);

    ret = rspAut(bufPtr(&bRsp), bufLen(&bRsp));
    CHECK(ret >= 0, lblKO);
    ret = emvResult(0x80, 1);
    CHECK(ret >= 0, lblKO);
    //  ret= emvResult(0x80,2); CHECK(ret>0,lblKO);

    ret = printData();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
}
#endif
