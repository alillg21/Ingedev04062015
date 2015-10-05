#include <string.h>
#include "log.h"
#include "emv.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Functional test case tcik0049.
//ETEC matercard Testing ToolKit

#define CHK CHECK(ret>=0,lblKO)

static int prepareCard(byte idx) {
    int ret;

    ret = mapPut(traEntMod, "c", 1);
    CHK;
    ret = mapPut(emvTrmCap, "\x03\xE0\xB8\xC8", lenTrmCap + 1);
    CHK;
    ret = mapPut(emvTACDft, "\x05\xF1\x00\xF0\xA0\x01", lenTVR + 1);
    CHK;
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

static int lookData(byte idx) {
    int ret;

    //byte tsi[1+lenTSI];
    byte tvr[1 + lenTVR];

    //byte RspCod[1+lenRspCod];
    char buf[prtW + 1];

    ret = dspClear();
    CHK;
    /*
       switch(idx){
       case 1: ret= dspLS(0,"<Floor Limit"); break;
       case 2: ret= dspLS(0,">Floor Limit"); break;
       default: VERIFY(idx==1); break; //impossible
       }
       CHK;

       ret= dspLS(1,"TSI..."); CHK;
       ret= mapGet(emvTSI,tsi,1+lenTSI); CHK;
       bin2hex(buf,tsi+1,lenTSI);
       ret=prtS("TSI: ");
       ret= prtS(buf);
       ret= bitTest(tsi+1,tsiOflDtaAut); CHECK(!ret,lblKO);  //Offline Data Authentication was not performed
       ret= dspLS(1,"TSI OK"); CHK;
       tmrPause(1);
     */
    //ret= dspLS(1,"TVR..."); CHK;
    ret = mapGet(emvTVR, tvr, 1 + lenTVR);
    CHK;
    bin2hex(buf, tvr + 1, 1 + lenTVR);
    ret = prtS("TVR: ");
    ret = prtS(buf);
    /*
       ret= bitTest(tvr+1,tvrCrdVrf); CHECK(!ret,lblKO); //Cardholder verification was successful

       if(idx==2){
       ret= bitTest(tvr+1,tvrFlrLimExc); CHECK(ret,lblKO); //transaction exceeds floor limit
       }
     */
    //ret= dspLS(1,"TVR OK"); CHK;
    //tmrPause(1);
/*
    ret= CheckAppPrfNam("MasterCard"); CHK;
    ret= CheckChdNam("MasterCard TIP Test Cards"); CHK;

    ret= dspLS(3,"RspCod..."); CHK;
    ret= mapGet(emvRspCod,RspCod,1+lenRspCod); CHK;
    memset(buf,0,prtW+1);
    memcpy(buf,RspCod+1,*RspCod);
    ret= prtS(buf);
    switch(idx){
        case 1:
        			//CHECK(emvIsApproved()==1,lblKO);
        			break;
        case 2:
            if(emvIsApproved()!=1){
                CHECK(memcmp((char *)RspCod+1,"Z3",lenRspCod)==0,lblKO);
            }
            break;
        default: VERIFY(idx==1); break; //impossible
    }
    ret= dspLS(3,"RspCod OK"); CHK;
    tmrPause(1);
*/
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcik0049(void) {
    int ret;
    const char *msg;
    byte idx;

    msg = "Insert REQ07 01";
    for (idx = 2; idx <= 2; idx++) {
        ret = prepareCard(idx);
        ret = testCard(idx, msg);
        CHECK(ret >= 0, lblKO);
        //pmtPayment();
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
