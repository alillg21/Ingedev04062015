#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0009.
//Online call testing.

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(void) {
    int ret;

    ret = mapPutWord(traMnuItm, mnuSale);
    CHK;
    ret = mapPut(traEntMod, "c", 1);
    CHK;
    ret = mapPutStr(traTrk2, "5425364009408223=05121011126947400000");
    CHK;
    ret = mapPutStr(traPan, "5425364009408223");
    CHK;
    ret = mapPut(traExpDat, "0512", lenExpDat);
    CHK;
    ret = mapPutStr(traAmt, "234");
    CHK;
    ret = mapPutWord(regMsgNum, 1);
    CHK;
    ret = mapPut(traDiaInd, "1", 1);
    CHK;
    ret = mapMove(issBeg + 1, 0);
    CHK;
    mapMove(acqBeg + 1, 0);
    ret = mapPutStr(acqMerchantNo, "999999999   ");
    CHK;
    //Phone number 
    ret = mapMove(acqBeg + 1, 1);
    ret = mapPutStr(acqPriPhone, "5739374");
    ret = mapPutStr(acqSecPhone, "5739374");
    ret = mapPut(emvTVR, "\x05\x00\x80\x00\x00\x00", 1 + 0x05);
    ret = mapPut(emvCID, "\x01\x80", 1 + 0x01);
    ret = mapPut(emvTrnDat, "\x03\x04\x12\x14", 1 + 0x03);
    ret = mapPut(emvTrnTim, "\x03\x12\x26\x05", 1 + 0x03);
    ret =
        mapPut(emvIssTrnCrt, "\x08\x15\xB1\x6C\xA0\xCE\x56\x9F\x75", 1 + 0x08);
    ret = mapPut(emvAIP, "\x02\x5C\x00", 1 + 0x02);
    ret = mapPut(emvATC, "\x02\x01\x2C", 1 + 0x02);
    ret = mapPut(emvUnpNum, "\x04\x8C\x20\x47\x90", 1 + 0x04);
    ret = mapPut(emvTrnTyp, "\x01\x00", 1 + 0x01);
    ret = mapPut(emvIAD, "\x07\x06\x00\x0A\x03\x94\x00\x00", 1 + 0x07);
    ret = mapPut(emvCVMRes, "\x03\x01\x03\x32", 1 + 0x03);  //refer to emv 2000 Book 3

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

void tcnl0009(void) {
    int ret;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "prepare01...");
    CHECK(ret >= 0, lblKO);
    ret = prepare01();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "onlAutEMV...");
    CHECK(ret >= 0, lblKO);
    //ret= emvOnlAut(); CHECK(ret>=0,lblKO);
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
