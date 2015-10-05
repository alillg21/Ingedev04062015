#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0011.
//Parsing response download emv.

#define CHK CHECK(ret>=0,lblKO)

/***********************************************************************************/
static int prepareCAKeyPrepare1(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "A000000003", 0);
    CHK;                        //RID
    ret = bufApp(rsp, (byte *) "030000", 0);
    CHK;                        //Exp
    ret = bufApp(rsp, (byte *) "01", 0);
    CHK;                        //lenExp
    ret = bufApp(rsp, (byte *) "C696034213D7D8546984579D1D0F0EA519", 0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *)
               "CAFF8DEFFC429354CF3A871A6F7183F1228DA5C7470C055387100CB935A712C4E2",
               0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *)
               "64DF5D64BA93FE7E63E71F25B1E5F5298575EBE1C63AA617706917911DC2A75AC2",
               0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *)
               "88B251C7EF40F2365912490B939BCA2124A30A28F54402C34AECA331AB", 0);
    CHK;                        //key module
    ret = bufApp(rsp, (byte *) "7E1E79B285DD5771B5D9FF79EA630B75", 0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *)
               "000000000000000000000000000000000000000000000000000000000000000000",
               0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *)
               "00000000000000000000000000000000000000000000000000000000000000",
               0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *)
               "00000000000000000000000000000000000000000000000000000000000000",
               0);
    CHK;                        //key module
    ret =
        bufApp(rsp,
               (byte *) "0000000000000000000000000000000000000000000000000000",
               0);
    CHK;                        //key module
    ret = bufApp(rsp, (byte *) "000000000000008001", 0);
    CHK;                        //key module

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/

static int printCAKEY(void) {
    int ret;
    int idx;
    card CAKeyCountNum;
    char buf[800];
    char tmp[800];
    char bByte;
    char CAKeyCount[4] = "";
    card num;

    //befor in this test case ,test case  tcnl0010 and tcnl0011 must be run
    ret = mapGet(glbCAKeyCnt, CAKeyCount, 3);
    CHK;
    dec2num(&CAKeyCountNum, CAKeyCount, 3);

    for (idx = 0; idx < (int) CAKeyCountNum; idx++) {
        ret = mapMove(pubBeg + 1, (byte) idx);
        strcpy(buf, "idx=");
        num2dec(buf + 4, idx + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        ret = prtS("pubRid:");
        CHK;
        ret = mapGet(pubRid, buf, lenRid);
        ret = bin2hex(tmp, (byte *) buf, lenRid);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

        ret = prtS("pubIdx:");
        CHK;
        ret = mapGetByte(pubIdx, bByte);
        ret = bin2hex(tmp, (byte *) & bByte, 1);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

        ret = prtS("pubExp:");
        CHK;
        ret = mapGetCard(pubExp, num);
        ret = num2dec(tmp, num, 4);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

        ret = prtS("pubMod:");
        CHK;
        ret = mapGet(pubMod, buf, lenMod);
        ret = bin2hex(tmp, (byte *) buf, lenMod);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

        ret = prtS("pubLng:");
        CHK;
        ret = mapGetByte(pubLng, bByte);
        ret = bin2hex(tmp, (byte *) & bByte, 1);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

        ret = prtS("pubExpLen:");
        CHK;
        ret = mapGetByte(pubExpLen, bByte);
        ret = bin2hex(tmp, (byte *) & bByte, 1);
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);

    }

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
void tcnl0012(void) {
    int ret;
    byte dRsp[2000];
    tBuffer bRsp;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "CA KEY...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 2000);

    ret = prepareCAKeyPrepare1(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse01...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl13(bufPtr(&bRsp), (word) (bufLen(&bRsp)));
    CHECK(ret >= 0, lblKO);

    ret = printCAKEY();
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
