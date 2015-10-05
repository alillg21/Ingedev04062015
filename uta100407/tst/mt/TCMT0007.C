#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcmt0007.
//Receipt formatting.

#define CHK if(ret<0) return ret

//prepare tra table for payment report
static int prepare01(void){
    int ret;

    ret= mapPutWord(regBatPos,0); CHK;
        ret= mapPutWord(regMsgNum,1); CHK;
        ret= mapPutWord(traSeqNo,1); CHK;
    ret= mapMove(issBeg+1,MASTERCARD); CHK;
        ret = mapPutByte(traAuthSrcCod,3); CHK;
        ret = mapPutByte(traAccDataSrcCod,'D'); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPutStr(traExpDat,"0512"); CHK;
    ret= mapPutStr(traAmt,"234"); CHK;
    ret= mapPutStr(traAutCod,"123123"); CHK;
        ret= mapPutStr(acqMerchantNo,"999999999"); CHK;
        ret= mapPutByte(issPrintFullCardNo,1);
    ret= mapPutByte(issPrintCardholderName,1); CHK;
        ret= mapPutStr(traCrdHldrName,"MELINEH TABIBZADEH"); CHK;
        ret= mapPutStr(appMerName,"Solution Gulf"); CHK;
        ret= mapPutStr(appMerchantAdd1,"NBD BUILDING,4TH FLOOR"); CHK;
        ret= mapPutStr(appMerchantAdd2,"SHARJAH,UAE"); CHK;
        ret= mapPutStr(traOpName,""); CHK;
        ret= mapPutStr(appMerCategoryCode,CarRental_category);
        ret= mapPutStr(traCtgFld,"1111"); CHK;
        
        //appCustSlogan1
        ret= mapPutStr(appFirstHeaderSlogan,"Have a nice day");
        
        //sign
        ret= mapPutByte(traSgnReq,1); 
        
        //pan
        ret= mapPutByte(issPrintFullCardNo,1); 
        
        //void
        ret= mapPutByte(traVoidIndicator,'V');

    return 1;
}

//prepare log table for detail report
static int prepare02(void){
    int ret;
    char cPan[lenPan+1];
    byte bPan[(lenPan/2)+1];

    char cAmt[lenAmt+1];
    byte bAmt[(lenAmt/2)+1];

    char cDatTim[lenDatTim+1];
    byte bDatTim[3+3];


        ret= mapPutWord(traMnuItm,mnuDetails); CHK;
        ret= mapPutStr(acqMerchantNo,"800026785643"); CHK;

        // First record in log
        ret= mapMove(logBeg+1,0);
        ret= mapPutWord(logBatchNo,1); CHK;
    ret= mapPutWord(logPosTxnNo,1);CHK;
    ret= mapPutWord(regLogPos,1);CHK;

    ret= mapPutStr(traPan,"1111608223152969FFFFFF"); CHK;
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

        ret= mapPutStr(logApprovalCode,"047287   "); CHK;

        ret= mapPutByte(logIssRow,2); CHK;
        ret= mapPutByte(logTxnTypeRow,2); CHK;

        ret= mapPutByte(logVoidIndicator,' ');
        ret= mapPutByte(logAuthorisationSourceCode,'1'); CHK;
    ret= mapPutByte(logAccountDataSourceCode,'D'); CHK;
    ret= mapPutByte(logCrdHldrIDCod,'@'); CHK;


        ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

        ret= mapPutByte(appExp,2); CHK;

        ret= mapPut(traAmt,"3712",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;

        // Second record in log
        ret= mapMove(logBeg+1,1);
        ret= mapPutWord(logBatchNo,1); CHK;
    ret= mapPutWord(logPosTxnNo,2);CHK;
    ret= mapPutWord(regLogPos,2);CHK;

    ret= mapPutStr(traPan,"2222608223152969FFFFFF"); CHK;
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

        ret= mapPutByte(logIssRow,2); CHK;
        ret= mapPutByte(logTxnTypeRow,2); CHK;

        ret= mapPutByte(logVoidIndicator,' ');
        ret= mapPut(logAuthorisationSourceCode,"3",1); CHK;
    ret= mapPut(logAccountDataSourceCode,"T",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;


        ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

        ret= mapPutByte(appExp,2); CHK;

        ret= mapPut(traAmt,"1500",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;


        // Third record in log
        ret= mapMove(logBeg+1,2);
        ret= mapPutWord(logBatchNo,1); CHK;
    ret= mapPutWord(logPosTxnNo,3);CHK;
    ret= mapPutWord(regLogPos,3);CHK;

    ret= mapPutStr(traPan,"3333608223152969FFFFFF"); CHK;
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

        ret= mapPutByte(logIssRow,2); CHK;
        ret= mapPutByte(logTxnTypeRow,7); CHK;

        ret= mapPut(logAuthorisationSourceCode,"3",1); CHK;
    ret= mapPut(logAccountDataSourceCode,"T",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;

        ret= mapPutByte(logVoidIndicator,' ');
        ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

        ret= mapPutByte(appExp,2); CHK;

        ret= mapPut(traAmt,"1201",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;

        return 1;
}

//prepare log table for summary report
static int prepare03(void){
    int ret;
    char cPan[lenPan+1];
    byte bPan[(lenPan/2)+1];

    char cAmt[lenAmt+1];
    byte bAmt[(lenAmt/2)+1];

    char cDatTim[lenDatTim+1];
    byte bDatTim[3+3];


        ret= mapPutWord(traMnuItm,mnuSummary); CHK;
        ret= mapPutStr(acqMerchantNo,"800026785643"); CHK;

        // First record in log
        ret= mapMove(logBeg+1,0);
        ret= mapPutWord(logBatchNo,1); CHK;
    ret= mapPutWord(logPosTxnNo,1);CHK;
    ret= mapPutWord(regLogPos,1);CHK;
        ret= mapPutWord(traSeqNo,1); CHK;

    ret= mapPutStr(traPan,"1111608223152969"); CHK;
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

        ret= mapPutStr(logApprovalCode,"047287   "); CHK;

        ret= mapPutByte(logIssRow,2); CHK;
        ret= mapPutByte(logTxnTypeRow,2); CHK;

        ret= mapPutByte(logVoidIndicator,' ');
        ret= mapPut(logAuthorisationSourceCode,"1",1); CHK;
    ret= mapPut(logAccountDataSourceCode,"D",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;


        ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

        ret= mapPutByte(appExp,2); CHK;

        ret= mapPut(traAmt,"3712",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;

        // Second record in log
        ret= mapMove(logBeg+1,1);
        ret= mapPutWord(logBatchNo,1); CHK;
    ret= mapPutWord(logPosTxnNo,2);CHK;
    ret= mapPutWord(regLogPos,2);CHK;
        ret= mapPutWord(traSeqNo,2); CHK;

    ret= mapPutStr(traPan,"2222608223152969"); CHK;
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

        ret= mapPutByte(logIssRow,2); CHK;
        ret= mapPutByte(logTxnTypeRow,txnPreAuth); CHK;

        ret= mapPutByte(logVoidIndicator,' ');
        ret= mapPut(logAuthorisationSourceCode,"3",1); CHK;
    ret= mapPut(logAccountDataSourceCode,"T",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;


        ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

        ret= mapPutByte(appExp,2); CHK;

        ret= mapPut(traAmt,"1500",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;


        // Third record in log
        ret= mapMove(logBeg+1,2);
        ret= mapPutWord(logBatchNo,1); CHK;
    ret= mapPutWord(logPosTxnNo,3);CHK;
    ret= mapPutWord(regLogPos,3);CHK;
        ret= mapPutWord(traSeqNo,3); CHK;

    ret= mapPutStr(traPan,"3333608223152969"); CHK;
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

        ret= mapPutByte(logIssRow,2); CHK;
        ret= mapPutByte(logTxnTypeRow,7); CHK;

        ret= mapPut(logAuthorisationSourceCode,"3",1); CHK;
    ret= mapPut(logAccountDataSourceCode,"T",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;

        ret= mapPutByte(logVoidIndicator,' ');
        ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

        ret= mapPutByte(appExp,2); CHK;

        ret= mapPut(traAmt,"1201",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;

        ret= calcTotals(0); CHK;
        return 1;
}

//prepare batch table for old batches report
static int prepare04(void){
        int ret;

        ret= mapPutWord(traMnuItm,mnuOldBatches); CHK;
        ret= mapPutWord(regBatPos,3);

        ret= mapMove(batchBeg+1,0);
    ret= mapPutWord(batBatchNo,0); CHK;
        ret= mapPutStr(batTrasRecCnt,"4"); CHK;
        ret= mapPutStr(batAmtTot,"1000"); CHK;
        ret= mapPutStr(batNetDeposit,"600"); CHK;
    ret= mapPutStr(batDate,"1004"); CHK;


        ret= mapMove(batchBeg+1,1);
    ret= mapPutWord(batBatchNo,1); CHK;
        ret= mapPutStr(batTrasRecCnt,"6"); CHK;
        ret= mapPutStr(batAmtTot,"2000"); CHK;
        ret= mapPutStr(batNetDeposit,"300"); CHK;
    ret= mapPutStr(batDate,"1104"); CHK;

        ret= mapMove(batchBeg+1,2);
    ret= mapPutWord(batBatchNo,2); CHK;
        ret= mapPutStr(batTrasRecCnt,"5"); CHK;
        ret= mapPutStr(batAmtTot,"3000"); CHK;
        ret= mapPutStr(batNetDeposit,"800"); CHK;
    ret= mapPutStr(batDate,"1204"); CHK;


        return 1;

}

void tcmt0007(void){
    int ret;
        byte bByte;

        mapMove(acqBeg+1,7);
        ret= mapPutStr(acqMerchantNo,"999999999   ");

        //sets terminal permission for detial and summary report
        bByte = 0x03;
        ret = mapMove(perBeg+1,TerminalPer);
        ret = mapPutByte(perBatchReport,bByte);

        //sets operator
        ret = mapPutStr(operPassword,"1234");
        ret = mapPutCard(operPermissions,0x00FFFFFF);


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
        ret= mapPutByte(tPerAppearInReport,0);
        ret= mapMove(tPerBeg+1,txnPreAuth);
        ret= mapPutByte(tPerAppearInReport,0);
        ret= mapMove(tPerBeg+1,txnRefundOnl);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnRefundOff);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnTip);
        ret= mapPutByte(tPerAppearInReport,1);
        ret= mapMove(tPerBeg+1,txnMailOrder);
        ret= mapPutByte(tPerAppearInReport,1);  
        
        ret= dspClear();
    CHECK(ret>=0,lblKO);

    ret= dspLS(0,"prepare01...");
    CHECK(ret>=0,lblKO);

    ret= prepare01();
    CHECK(ret>=0,lblKO);

/*
        //Sale
    ret= mapPutWord(traMnuItm,mnuSale);
    CHECK(ret>=0,lblKO);
    ret= mapMove(txnTypeBeg+1,txnPurchase);
    CHECK(ret>=0,lblKO);
        ret= rptReceipt();
    CHECK(ret>=0,lblKO);

        //Pre Auth
    ret= mapPutWord(traMnuItm,mnuPreaut);
    ret= mapMove(txnTypeBeg+1,txnPreAuth);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);
        //Pre Auth complete
    ret= mapPutWord(traMnuItm,mnuOffline);
    ret= mapMove(txnTypeBeg+1,txnOffline);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);
        //Pre Receipt
    ret= mapPutWord(traMnuItm,mnuPrereceipt);
    ret= mapMove(txnTypeBeg+1,txnPreReceipt);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);
        //Tip
    ret= mapPutWord(traMnuItm,mnuTip);
    ret= mapMove(txnTypeBeg+1,txnTip);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);
        //Online Refund
    ret= mapPutWord(traMnuItm,mnuRefundOnl);
    ret= mapMove(txnTypeBeg+1,txnRefundOnl);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);
        //Offline Refund
    ret= mapPutWord(traMnuItm,mnuRefundOff);
    ret= mapMove(txnTypeBeg+1,txnRefundOff);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);
        //Duplicate
    ret= mapPutWord(traMnuItm,mnuDuplicate);
    ret= mapMove(txnTypeBeg+1,txnPurchase);
    ret= rptReceipt();
        CHECK(ret>=0,lblKO);

*/
    ret= dspLS(0,"prepare02...");
    CHECK(ret>=0,lblKO);

    ret= prepare02();
    CHECK(ret>=0,lblKO);
    ret= rptReceipt();
    ret= dspLS(0,"prepare03...");
    CHECK(ret>=0,lblKO);

    ret= prepare03();
    CHECK(ret>=0,lblKO);
    ret= rptReceipt();
    CHECK(ret>=0,lblKO);

    ret= dspLS(0,"prepare04...");
    CHECK(ret>=0,lblKO);

        ret= prepare04();
    CHECK(ret>=0,lblKO);
    ret= rptReceipt();
    CHECK(ret>=0,lblKO);

        goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
}
#endif
