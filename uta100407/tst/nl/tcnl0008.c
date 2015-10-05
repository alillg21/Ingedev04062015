#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0008.
//Building the request for emv .

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
    ret = mapPutStr(traExpDat, "0512");
    CHK;
    ret = mapPutStr(traAmt, "234");
    CHK;
    ret = mapPutWord(regMsgNum, 1);
    CHK;
    ret = mapPut(traDiaInd, "1", 1);
    CHK;
    ret = mapPutByte(txnTypeMsgType, 10);
    CHK;
    ret = mapPutByte(traCfmCod, 1);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}
static int Emvprepare01(void) {
    int ret;

    //ret= mapPutStr(traDatTim,"20041214153447"); CHK;
    //  ret= mapPutWord(traSpdhIdx,0); CHK;
    // ret= mapPutByte(traEntMod,'c'); CHK;
    //ret= mapPutWord(traMnuItm,mnuNormalPurchase); CHK;
    // ret= mapPutCard(regMsgNum,1); CHK;
    // ret= mapPutStr(traTrk2,""); CHK;
    // ret= mapPutStr(traPan,"4568941111111119"); CHK;
    //ret= mapPutStr(traExpDat,"0412"); CHK;
    //ret= mapPutStr(traAmt,"111"); CHK;
    //ret= mapPutStr(traAmt2,""); CHK;
    //ret= mapPutStr(traRspCod,"001"); CHK;
    //ret= mapPutStr(traAppCod,""); CHK;
    //ret= mapPutStr(traRspDsp,""); CHK;
    //ret= mapPut(emvTrk2,"\x13\x45\x68\x94\x11\x11\x11\x11\x19\xD0\x41\x22\x01\x17\x88\x83\x73\x00\x00\x0F",1+0x13); CHK;
    //ret= mapPut(emvPANSeq,"\x01\x00",1+0x01); CHK;
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

    //  ret= mapPut(emvAutCod,"\x00",1+0x00); CHK;
    // ret= mapPut(emvIssAutDta,"\x00",1+0x00); CHK;
    // ret= mapPut(emvIssAutDta,"\x02\x59\x31",1+0x02); CHK;

    return 1;
}

static int printReq(const char *hdr, const char *req, word len) {
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
void tcnl0008(void) {
    int ret;
    byte dReq[512];
    tBuffer bReq;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "prepare Non Emv 01...");
    CHECK(ret >= 0, lblKO);
    ret = prepare01();
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "EMVprepare01...");
    CHECK(ret >= 0, lblKO);

    ret = Emvprepare01();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "buildEmv01...");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "build01...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bReq, dReq, 512);

    ret = reqAut(&bReq);
    CHECK(ret >= 0, lblKO);

    trcFS("req=[%s]\n", (char *) bufPtr(&bReq));
    ret = printReq("  request01", (char *) bufPtr(&bReq), bufLen(&bReq));
    CHECK(ret >= 0, lblKO);

    tmrPause(3);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrStop(1);
}
#endif
