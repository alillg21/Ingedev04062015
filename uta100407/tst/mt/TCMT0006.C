#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcmt0006.
//logSendBatch

#define CHK CHECK(ret>=0,lblKO)

static int prepare01(void){
    int ret;



    ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPutWord(traMnuItm,mnuTip); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"100",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'5'); CHK;
        ret= mapPutStr(traAutCod,"7458     "); CHK;
    ret= mapMove(issBeg+1,6);
        ret= mapPutByte(issId,6); CHK;
    ret= mapMove(txnTypeBeg+1,txnPurchase); //txnTip**
        ret= mapPutByte(traMsgType,10); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
        ret= mapPutWord(regMsgNum,1);
        ret= mapPutWord(traSeqNo,1); CHK;
        ret= mapPutByte(traVoidIndicator,' '); CHK
      ret= logSave(INSERT); CHECK(ret>=0,lblKO);

        ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPutWord(traMnuItm,mnuSale); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"100",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'1'); CHK;
        ret= mapPutStr(traAutCod,"7459     "); CHK;
    ret= mapMove(issBeg+1,6);
        ret= mapPutByte(issId,6); CHK; //master card
    ret= mapMove(txnTypeBeg+1,1); //purchase
        ret= mapPutByte(traMsgType,20); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
        ret= mapPutWord(regMsgNum,2);
        ret= mapPutWord(traSeqNo,2); CHK;
        ret= mapPutByte(traVoidIndicator,' ');  CHK;
    ret= logSave(INSERT); CHECK(ret>=0,lblKO);

    ret= mapPutWord(traMnuItm,mnuPreaut); CHK;
        ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"102",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'1'); CHK;
        ret= mapPutStr(traAutCod,"7459     "); CHK;
    ret= mapMove(issBeg+1,6);//master card
        ret= mapPutByte(issId,6); CHK;
    ret= mapMove(txnTypeBeg+1,txnPreAuth);//txnPreAuth **
        ret= mapPutByte(traMsgType,20); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
        ret= mapPutWord(regMsgNum,3); CHK;
        ret= mapPutWord(traSeqNo,3); CHK;
        ret= mapPutByte(traVoidIndicator,' ');  CHK;
      ret= logSave(INSERT); CHECK(ret>=0,lblKO);


    ret= mapPutWord(traMnuItm,mnuOffline); CHK;
        ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"200",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'5'); CHK;
        ret= mapPutStr(traAutCod,"7459     "); CHK;
    ret= mapMove(issBeg+1,6);//master card
        ret= mapPutByte(issId,6); CHK;
    ret= mapMove(txnTypeBeg+1,txnOffline);//txnOffline**
        ret= mapPutByte(traMsgType,20); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
        ret= mapPutWord(regMsgNum,3); CHK;
        ret= mapPutWord(traSeqNo,3); CHK;
        ret= mapPutByte(traVoidIndicator,' ');  CHK;
      ret= logSave(INSERT); CHECK(ret>=0,lblKO);



    ret= mapPutWord(traMnuItm,mnuSale); CHK;
    ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"500",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'1'); CHK;
        ret= mapPutStr(traAutCod,"7459     "); CHK;
    ret= mapMove(issBeg+1,6); //master card
        ret= mapPutByte(issId,6); CHK;
    ret= mapMove(txnTypeBeg+1,1);//purchase
        ret= mapPutByte(traMsgType,20); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
        ret= mapPutWord(regMsgNum,4);
        ret= mapPutWord(traSeqNo,4); CHK;
        ret= mapPutByte(traVoidIndicator,' '); CHK;
      ret= logSave(INSERT); CHECK(ret>=0,lblKO);


        return 1;
lblKO:
    trcErr(ret);
 return -1;
}


static int prepare02(void){
        int ret;
        byte bByte;


        //sets permission for Appear in report  
        ret= mapMove(tPerBeg+1,txnPurchase);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnOffline);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnPreReceipt);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnVoid);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnCashAdvance);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnPreReceipt);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnPreReceipt);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnPreAuth);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnRefundOnl);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnRefundOff);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnTip);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnMailOrder);
        ret= mapPutByte(tPerAppearInReport,1);


        //sets terminal id
        ret= mapPutStr(appTID,"03050040"); CHK;
        ret= mapPutStr(appPOSSriNo,"789012"); CHK;
        ret= mapPutStr(appTerminalMerID,"800020005"); CHK; 
        //reset batch table position
        ret= mapPutWord(regBatPos,0);

        //sets Phone number for MDC host
        ret= mapMove(acqBeg+1,7); CHK;
        ret= mapPutStr(acqPriPhone,"5739374"); CHK;
        ret= mapPutStr(acqSecPhone,"5739374"); CHK;

//      ret= mapPutStr(acqPriPhone,"5724901"); CHK;
//      ret= mapPutStr(acqSecPhone,"5724901"); CHK;

//      ret= mapPutStr(acqPriPhone,"5722071"); CHK;
//      ret= mapPutStr(acqSecPhone,"5722071"); CHK;

        
        //sets terminal permission for transfer batch
        ret= mapMove(perBeg+1,1); CHK;
        bByte = 0x01;
        ret = mapPutByte(perBatchTrf,bByte); CHK;

        //sets terminal permission for detial and summary report
        bByte = 0x03;
        ret = mapPutByte(perBatchReport,bByte); CHK;

        //sets operator
        ret = mapPutStr(operPassword,"1234");
        ret = mapPutCard(operPermissions,0x00FFFFFF);


        return 1;
lblKO:
    trcErr(ret);
 return -1;
}


void tcmt0006(void){
    int ret;

    ret= dspClear(); CHECK(ret>=0,lblKO);
    //ret= mapPutWord(regLogPos,0); CHK; //reset transaction log position

    ret= dspLS(1,"Save txns..."); CHECK(ret>=0,lblKO);
   // ret= prepare01(); CHECK(ret>=0,lblKO);
        tmrPause(1);

        ret= dspLS(2,"prepare..."); CHECK(ret>=0,lblKO);
    ret= prepare02(); CHECK(ret>=0,lblKO);
        tmrPause(1);

        ret= dspLS(3,"logSendBatch..."); CHECK(ret>=0,lblKO);
		ret= mapPutWord(appBchBlcNum,1);

        logSendBatch(0);
        tmrPause(1);

    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
}
#endif
