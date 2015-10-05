#include <string.h>
#include "log.h"
#include "emv.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Functional test case tcik0046.
//ETEC matercard Testing ToolKit

#define CHK CHECK(ret>=0,lblKO)

int prepareCard(byte idx) {
    int ret;

    ret = mapPut(traEntMod, "c", 1);
    CHK;
    // ret= mapPut(emvTrmCap,"\x03\xE0\xF8\xC8",lenTrmCap); CHK;
    /*
       traReset();
       ret= mapPut(traEntMod,"c",1); CHK;
       ret= mapPutWord(traMnuItm,mnuEMVtest1); CHK;

       mapMove(acqBeg+1,VISACARD);
       ret = mapPutByte(txnTypeId,txnPurchase); CHK;
       ret= mapPutWord(regBatPos,0);
       switch(idx){
       case 1: ret= mapPutStr(traAmt,"111"); break;
       case 2: ret= mapPutStr(traAmt,"11111"); break;
       default: VERIFY(idx==1); break; //impossible
       }

       CHK;
     */
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int CheckChdNam(const char *str) {
    int ret;
    byte ChdNam[1 + lenChdNam];
    char buf[256];

    ret = dspLS(2, "ChdNam...");
    CHK;
    ret = mapGet(emvChdNam, ChdNam, 1 + lenChdNam);
    CHK;
    memset(buf, 0, 256);
    memcpy(buf, ChdNam + 1, *ChdNam);
    ret = prtS(buf + 5);
    CHECK(strcmp(buf, str) == 0, lblKO);
    ret = dspLS(2, "ChdNam OK");
    CHK;
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int CheckAppPrfNam(const char *str) {
    int ret;
    byte AppPrfNam[1 + lenAppPrfNam];
    char buf[256];

    ret = dspLS(2, "AppPrfNam...");
    CHK;
    ret = mapGet(emvAppPrfNam, AppPrfNam, 1 + lenAppPrfNam);
    CHK;
    memset(buf, 0, prtW + 1);
    memcpy(buf, AppPrfNam + 1, *AppPrfNam);
    ret = prtS(buf);
    CHECK(strcmp(buf, str) == 0, lblKO);
    ret = dspLS(2, "AppPrfNam OK");
    CHK;
    tmrPause(1);
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int printTraEmv(void) {
    int ret;
    word key;
    card tag;
    byte buf[1 + 256];
    char tmp[512 + 1];
    byte bin[2];

    ret = prtS("");
    CHK;
    ret = prtS("EMV tags");
    CHK;

    for (key = traEmvBeg + 1; key < traEnd; key++) {
        ret = prtS("   --- --- ---");
        CHK;
        ret = mapGet(key, buf, 1 + 256);
        CHK;
        if(*buf == 0)
            continue;
        tag = mapTagKey(key);
        num2bin(bin, tag, 2);
        bin2hex(tmp, bin, 2);
        strcat(tmp, ":");
        ret = prtS(tmp);
        CHK;
        bin2hex(tmp, buf + 1, *buf);
        ret = prtS(tmp);
        CHK;
    }
    ret = prtS("   --- --- ---");
    CHK;
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

int testCard(byte idx, const char *msg) {
    int ret;
    int res;
    byte buf[128 * 3];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    switch (idx) {
      case 1:
          ret = dspLS(0, "prepare 1...");
          break;
      case 2:
          ret = dspLS(0, "prepare 2...");
          break;
      default:
          VERIFY(idx == 1);
          break;                //impossible
    }
    CHECK(ret >= 0, lblKO);

    ret = emvInitAid();
    CHECK(ret >= 0, lblKO);

    ret = prepareCard(idx);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, msg);
    CHECK(ret >= 0, lblKO);
    ret = enterCard(buf, "c");
    CHECK(ret == 'c', lblKO);
    ret = dspLS(1, "pmtSmartCard...");
    CHECK(ret >= 0, lblKO);
    ret = pmtSmartCard();
    CHECK(ret >= 0, lblKO);
    res = ret;
    //ret= printTraEmv(); CHECK(ret>=0,lblKO);
    ret = usrInfo(infRemoveCard);
    CHECK(ret >= 0, lblKO);
    removeCard();
    ret = res;
    goto lblEnd;
  lblKO:trcErr(ret);
    ret = -1;
  lblEnd:iccStop(0);
    return ret;
}

static int lookData(byte idx) {
    int ret;
    byte tsi[1 + lenTSI];
    byte tvr[1 + lenTVR];
    byte RspCod[1 + lenRspCod];
    char buf[prtW + 1];

    ret = dspClear();
    CHK;
    switch (idx) {
      case 1:
          ret = dspLS(0, "<Floor Limit");
          break;
      case 2:
          ret = dspLS(0, ">Floor Limit");
          break;
      default:
          VERIFY(idx == 1);
          break;                //impossible
    }
    CHK;

    ret = dspLS(1, "TSI...");
    CHK;
    ret = mapGet(emvTSI, tsi, 1 + lenTSI);
    CHK;
    bin2hex(buf, tsi + 1, lenTSI);
    ret = prtS("TSI:");
    ret = prtS(buf);
    //ret= bitTest(tsi+1,tsiOflDtaAut); CHECK(ret,lblKO);  //Offline Data Authentication performed
    //ret= dspLS(1,"TSI OK"); CHK;
    //tmrPause(1);

    ret = dspLS(1, "TVR...");
    CHK;
    ret = mapGet(emvTVR, tvr, 1 + lenTVR);
    CHK;
    bin2hex(buf, tvr + 1, 1 + lenTVR);
    ret = prtS("TVR:");
    ret = prtS(buf);
    //ret= bitTest(tvr+1,tvrOflDtaAut); CHECK(ret,lblKO); //Offline Data Authentication was not performed
    //ret= bitTest(tvr+1,tvrOflSdtAut); CHECK(!ret,lblKO); //Offline Static Data Authentication did not fail
    /*
       if(idx==2){
       ret= bitTest(tvr+1,tvrFlrLimExc); CHECK(ret,lblKO); //transaction exceeds floor limit
       }
     */
    //ret= dspLS(1,"TVR OK"); CHK;
    //tmrPause(1);

    ret = CheckAppPrfNam("MasterCard");
    CHK;
    ret = CheckChdNam("MasterCard TIP Test Cards");
    CHK;

    ret = dspLS(3, "RspCod...");
    CHK;
    ret = mapGet(emvRspCod, RspCod, 1 + lenRspCod);
    CHK;
    memset(buf, 0, prtW + 1);
    memcpy(buf, RspCod + 1, *RspCod);
    ret = prtS(buf);
    switch (idx) {
      case 1:
          //CHECK(emvIsApproved()==1,lblKO);
          break;
      case 2:
          if(emvIsApproved() != 1) {
              CHECK(memcmp((char *) RspCod + 1, "Z3", lenRspCod) == 0, lblKO);
          }
          break;
      default:
          VERIFY(idx == 1);
          break;                //impossible
    }
    ret = dspLS(3, "RspCod OK");
    CHK;
    tmrPause(1);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcik0046(void) {
    int ret;
    const char *msg;
    byte idx;

    msg = "Insert REQ01 02";
    for (idx = 2; idx <= 2; idx++) {
        ret = testCard(idx, msg);
        CHECK(ret >= 0, lblKO);
        ret = lookData(idx);
        CHECK(ret >= 0, lblKO);
    }

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    usrInfo(infRemoveCard);
    removeCard();
  lblEnd:
    iccStop(0);
    tmrPause(3);
}
#endif
