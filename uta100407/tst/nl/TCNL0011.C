#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0011.
//Parsing response download emv.

#define CHK CHECK(ret>=0,lblKO)

/***********************************************************************************/
static int prepareEmvHeader(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "454D565445524D504152", 0);
    CHK;                        //EMVTERPAR
    ret = bufApp(rsp, (byte *) "3030303530303335", 0);
    CHK;                        //TermianlID
    ret = bufApp(rsp, (byte *) "31", 0);
    CHK;                        //Param update
    ret = bufApp(rsp, (byte *) "3030303030303030", 0);
    CHK;                        //EmvParamVer
    ret = bufApp(rsp, (byte *) "3533333931", 0);
    CHK;                        //EmvParCrc
    ret = bufApp(rsp, (byte *) "31", 0);
    CHK;                        //Key Update
    ret = bufApp(rsp, (byte *) "3030303030303036", 0);
    CHK;                        //EmvKeyVer
    ret = bufApp(rsp, (byte *) "303032", 0);
    CHK;                        //CAKeyCnt
    ret = bufApp(rsp, (byte *) "3333343139", 0);
    CHK;                        //CAKeyCrc

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare01(tBuffer * rsp) {
    int ret;

    //ret= bufApp(rsp,(byte *)"00",0); CHK; //Block No
    ret = bufApp(rsp, (byte *) "0500F1000000", 0);
    CHK;                        //TACDft
    ret = bufApp(rsp, (byte *) "050000000012", 0);
    CHK;                        //TACDen
    ret = bufApp(rsp, (byte *) "050000000012", 0);
    CHK;                        //TACOnl
    ret = bufApp(rsp, (byte *) "060000FFC99A3B", 0);
    CHK;                        //ThrVal   
    ret = bufApp(rsp, (byte *) "010A", 0);
    CHK;                        //TarPer   
    ret = bufApp(rsp, (byte *) "010C", 0);
    CHK;                        //MaxTarPer  
    ret =
        bufApp(rsp,
               (byte *)
               "209F37049F47018F019F32019F03049F37049F37049F37049F37049F37049F3704",
               0);
    CHK;                        //DftValDDOL
    ret =
        bufApp(rsp,
               (byte *)
               "1E9F08039F37039F37039F37039F37039F37039F37039F37039F37039F37030000",
               0);
    CHK;                        //DftValTDOL
    ret =
        bufApp(rsp,
               (byte *) "3F9F08059F08059F08059F08059F08059F08059F08059F0805",
               0);
    CHK;                        //DftValDOL//24 byte
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare02(tBuffer * rsp) {
    int ret;

    //   ret= bufApp(rsp,(byte *)"01",0); CHK; //Block No
    ret =
        bufApp(rsp,
               (byte *)
               "3F9F08059F08059F08059F08059F08059F08059F08059F08059F08059F08059F08059F08059F0800",
               0);
    CHK;                        //DftValDOL 40 byte
    ret = bufApp(rsp, (byte *) "020784", 0);
    CHK;                        //TrnCurCod
    ret = bufApp(rsp, (byte *) "020784", 0);
    CHK;                        //AccCntCod
    ret = bufApp(rsp, (byte *) "04F4010000", 0);
    CHK;                        //TrmFlrLim
    ret = bufApp(rsp, (byte *) "0188", 0);
    CHK;                        //VisaAppVN
    ret = bufApp(rsp, (byte *) "0107", 0);
    CHK;                        //MasterCardAppVN
    ret = bufApp(rsp, (byte *) "02", 0);
    CHK;                        //NumKeyRevocCnt
    ret = bufApp(rsp, (byte *) "100000000308004008", 0);
    CHK;                        //RevokKey1
    ret = bufApp(rsp, (byte *) "100000000301003005", 0);
    CHK;                        //RevokKey2
    ret = bufApp(rsp, (byte *) "000000000000000000", 0);
    CHK;                        //RevokKey3       
    ret = bufApp(rsp, (byte *) "000000000000000000", 0);
    CHK;                        //RevokKey4       
    ret = bufApp(rsp, (byte *) "000000000000000000", 0);
    CHK;                        //RevokKey5       
    ret = bufApp(rsp, (byte *) "000000000000000000", 0);
    CHK;                        //RevokKey6       
    ret = bufApp(rsp, (byte *) "000000000000000000", 0);
    CHK;                        //RevokKey7       
    ret = bufApp(rsp, (byte *) "24", 0);
    CHK;                        //lenEmvAPI (All API)     
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare03(tBuffer * rsp) {
    int ret;

    // ret= bufApp(rsp,(byte *)"02",0); CHK; //Block No
    ret = bufApp(rsp, (byte *) "07A000000003101000000000000000000001", 0);
    CHK;                        //AID1
    ret = bufApp(rsp, (byte *) "07A000000003080000000000000000000008", 0);
    CHK;                        //AID2
    ret = bufApp(rsp, (byte *) "000000000000000000000000000000000000", 0);
    CHK;                        //AID3
    ret = bufApp(rsp, (byte *) "000000000000000000000000000000000000", 0);
    CHK;                        //AID4
    ret = bufApp(rsp, (byte *) "000000000000000000000000000000000000", 0);
    CHK;                        //AID5
    ret = bufApp(rsp, (byte *) "000000000000000000000000000000000000", 0);
    CHK;                        //AID6
    ret = bufApp(rsp, (byte *) "000000000000000000000000", 0);
    CHK;                        //AID712

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}
static int printEmvHeaderRspDnl(void) {
    int ret;
    char buf[256];
    card num;

    ret = prtS("EMV Header:");
    CHK;

    ret = prtS("TerminalID:");
    CHK;
    ret = mapGet(appTID, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("Param update:");
    CHK;
    ret = mapGetByte(glbParamUpdate, buf);
    CHK;
    buf[1] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("EmvParamVer:");
    CHK;
    ret = mapGetCard(glbTermVer, num);
    CHK;
    num2hex(buf, num, 0);
    buf[8] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("EmvParCrc:");
    CHK;
    ret = mapGet(glbParamCrc, buf, 256);
    CHK;
    buf[5] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("Key Update:");
    CHK;
    ret = mapGetByte(glbKeyUpdate, buf);
    CHK;
    buf[1] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("EmvCAKeyVer:");
    CHK;
    ret = mapGetCard(glbCAKeyVer, num);
    CHK;
    num2hex(buf, num, 0);
    buf[8] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("CAKeyCnt:");
    CHK;
    ret = mapGet(glbCAKeyCnt, buf, 256);
    CHK;
    buf[3] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("CAKeyCrc:");
    CHK;
    ret = mapGet(glbCAKeyCrc, buf, 256);
    CHK;
    buf[5] = 0;
    ret = prtS(buf);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspEmvDnl01(void) {
    int ret;
    char buf[512];
    byte tmp[512];

    ret = prtS("Block No:");
    CHK;
    ret = prtS("00");
    CHK;

    ret = prtS("emvTACDft:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvTACDft, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvTACDen:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvTACDen, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvTACOnl:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvTACOnl, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvThrVal:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvThrVal, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, (unsigned char) (*tmp - 2));
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvTarPer:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvTarPer, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvMaxTarPer:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvMaxTarPer, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvDftValDDOL:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvDftValDDOL, tmp, 512);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf + 24);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf + 48);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvDftValTDOL:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvDftValTDOL, tmp, 512);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf + 24);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf + 48);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspEmvDnl02(void) {
    int ret;
    char buf[512];
    byte tmp[512];

    ret = prtS("emvTrnCurCod:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvTrnCurCod, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvAccCntCod:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvAccCntCod, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvTrmFlrLim:");
    CHECK(ret >= 0, lblKO);
    ret = mapGet(emvTrmFlrLim, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvTrmAvn For Visa:");
    CHECK(ret >= 0, lblKO);
    mapMove(issBeg + 1, (word) VISACARD);
    ret = mapGet(issEmvAppVN, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvTrmAvn For MasterCard:");
    CHECK(ret >= 0, lblKO);
    mapMove(issBeg + 1, (word) MASTERCARD);
    ret = mapGet(issEmvAppVN, tmp, 128);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, tmp + 1, *tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("emvNumKeyRevocCnt:");
    CHECK(ret >= 0, lblKO);
    ret = mapGetByte(glbEmvNumKeyRevocCnt, tmp[0]);
    CHECK(ret > 0, lblKO);
    VERIFY(*tmp < 128);
    ret = bin2hex(buf, &tmp[0], 1);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspEmvDnl03(void) {
    int ret;
    byte buf[256];
    char tmp[2 * 256 + 1];
    word idx;
    byte aidNo;

    ret = mapGetByte(glbAIDNO, aidNo);
    CHECK(ret >= 0, lblKO);
    num2dec(tmp, aidNo, 2);
    prtS("tmp");
    prtS(tmp);
    for (idx = 0; idx < aidNo; idx++) {
        ret = mapMove(aidBeg + 1, idx);

        ret = mapGet(emvAid, buf, 1 + lenAid);
        CHECK(ret == lenAid + 1, lblKO);
        VERIFY(*buf <= lenAid);
        ret = bin2hex(tmp, buf + 1, *buf);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

    }

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
void tcnl0011(void) {
    int ret;
    byte dRsp[256];
    tBuffer bRsp;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "EMV Header...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);

    ret = prepareEmvHeader(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) (ret / 2));

    ret = dspLS(1, "parse01...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl09(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printEmvHeaderRspDnl();
    CHECK(ret >= 0, lblKO);

    //Block 1
    ret = dspLS(0, "prepare01...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare01(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) (ret / 2));

    ret = dspLS(1, "parse01...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl10(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspEmvDnl01();
    CHECK(ret >= 0, lblKO);

    //Block 2       
    ret = dspLS(0, "prepare02...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare02(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) (bufLen(&bRsp) / 2));

    ret = dspLS(1, "parse02...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl11(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspEmvDnl02();
    CHECK(ret >= 0, lblKO);

    //Block 3
    ret = dspLS(0, "prepare03...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare03(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse03...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl12(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspEmvDnl03();
    CHECK(ret >= 0, lblKO);

    mapPutByte(regDnlPerformed, 1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrStop(1);
    tmrPause(3);
}
#endif
