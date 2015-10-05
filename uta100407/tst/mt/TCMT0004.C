#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;


//Online call testing.

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(void){
    int ret;
    char cDatTim[lenDatTim+1];
    byte bDatTim[3+3];

    char cAmt[lenAmt+1];
    byte bAmt[(lenAmt+1)/2];

    char cPan[lenPan+1]; 
    byte bPan[(lenPan/2)+1];

        byte bByte;
/*
    //Header
        ret= mapPutWord(batBatchNo,0); CHK;
        mapMove(acqBeg+1,7);
        ret= mapPutStr(acqMerchantNo,"999999999   "); CHK;
    ret= mapPutStr(appStoreNo,"1111"); CHK;
    ret= mapPutStr(appTermNo,"2222"); CHK;
    ret= mapPutWord(appBchBlcNum,3); CHK;    
        ret= mapPutStr(appCurrencyCode,"784"); CHK;

    //Parameter 
        ret= mapPutStr(appMerName,"MERCHANT                 "); CHK;
        ret= mapPutStr(appMerCityName,"Dubai        "); CHK;
        ret= mapPutStr(appMerStateCode,"AE"); CHK;
        ret= mapPutStr(appMerCategoryCode,"5999"); CHK;
        ret= mapPutStr(appMerLocationCode,"0000"); CHK;
        //ret= mapPutStr(glbTermID,"00050040"); CHK;
        ret= mapPutStr(glbTermID,"10000002"); CHK;
        ret= mapPutStr(appMerZipCode,"     "); CHK;
        ret= mapPutStr(appMerCountryCode,"784"); CHK;   
        ret= mapPutStr(appMerchantSecurityCode,"12345"); CHK;   

        //Detail
        ret= mapPutByte(logTxnTypeRow,1); CHK;  
    ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;
 

    ret= mapPutStr(traPan,"5438608223152969"); CHK;
        //Sets logPAN
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

    ret= mapPut(logAccountDataSourceCode,"D",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;


    ret= mapPut(traAmt,"2500",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;
    
        ret= mapPut(logVoidIndicator," ",1); CHK;
        ret= mapPut(logAuthorisationSourceCode,"5",1); CHK;
        ret= mapPutStr(logApprovalCode,"7458     "); CHK;
    ret= mapPutWord(logPosTxnNo,21); CHK;    
        

    ret= mapPut(traAmt,"0",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logCashbackAmount,bAmt,(lenAmt+1)/2); CHK;

        
        //Trailer
        ret= mapPutWord(regBatPos,0); CHK;
        ret= mapMove(batchBeg+1,0); CHK;
        ret= mapPutStr(batTrasRecCnt,"000000001"); CHK;
        ret= mapPutStr(batAmtTot,"0000000000002500"); CHK;
        ret= mapPutStr(batCashbackTot,"0000000000000000"); CHK;
        ret= mapPutStr(batNetDeposit,"0000000000002500"); CHK;

*/
    //Header
        mapMove(acqBeg+1,7);
        ret= mapPutStr(acqMerchantNo,"999999999   "); CHK;
        ret= mapPutStr(appTerminalMerID,"999999999   "); CHK; 
    ret= mapPutStr(appStoreNo,"1111"); CHK;
    ret= mapPutStr(appTermNo,"2222"); CHK;
    ret= mapPutWord(appBchBlcNum,3); CHK;    
        ret= mapPutStr(appCurrencyCode,"784"); CHK;


    //Parameter 
        ret= mapPutStr(appMerName,"MERCHANT                 "); CHK;
        ret= mapPutStr(appMerCityName,"Dubai        "); CHK;
        ret= mapPutStr(appMerStateCode,"AE"); CHK;
        ret= mapPutStr(appMerCategoryCode,"5999"); CHK;
        ret= mapPutStr(appMerLocationCode,"0000"); CHK;
        ret= mapPutStr(appTID,"00050035"); CHK;
    ret= mapPutStr(glbTermID,"00050035"); CHK;
        ret= mapPutStr(appMerZipCode,"     "); CHK;
        ret= mapPutStr(appMerCountryCode,"784"); CHK;   
        ret= mapPutStr(appMerchantSecurityCode,"12345"); CHK;   
    //ret= mapPutStr(appPOSSriNo,"003121"); CHK;
        ret= mapPutStr(appPOSSriNo,"523527"); CHK;

        //Detail
        ret= mapPutWord(regLogPos,0);
        mapMove(logBeg+1,0);
        ret= mapPutByte(logTxnTypeRow,1); CHK;  
    ret= mapGet(traDatTim,cDatTim,lenDatTim+1); CHK;
    ret= hex2bin(bDatTim,cDatTim+2,3+3); CHK;
    ret= mapPut(logTxnDate,bDatTim,3);  CHK;
    ret= mapPut(logTxnTime,bDatTim+3,3);  CHK;

    ret= mapPut(logExpDat,"0505",lenExpDat); CHK;

    //ret= mapPutStr(traPan,"5438608223152969"); CHK;
        ret= mapPutStr(traPan,"5437579983998878"); CHK;
        //Sets logPAN
    ret= mapGet(traPan,cPan,lenPan+1);  CHK;
    ret= fmtPad(cPan,lenPan,'F'); VERIFY(ret==lenPan); //right-pad
    ret= hex2bin(bPan,cPan,(lenPan/2)); VERIFY(ret== (lenPan/2));
    ret= mapPut(logPAN,bPan,lenPan/2);  CHK;

    ret= mapPut(logAccountDataSourceCode,"D",1); CHK;
    ret= mapPut(logCrdHldrIDCod,"@",1); CHK;


    ret= mapPut(traAmt,"800",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;
    
        ret= mapPut(logVoidIndicator," ",1); CHK;
        ret= mapPut(logAuthorisationSourceCode,"3",1); CHK;
        ret= mapPutStr(logApprovalCode,"SVC101"); CHK;
    ret= mapPutWord(logPosTxnNo,6); CHK;    
        

    ret= mapPut(traAmt,"0",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logCashbackAmount,bAmt,(lenAmt+1)/2); 
    ret= mapPut(logTxnAmount2,bAmt,(lenAmt+1)/2); 

        //Trailer
        ret= mapPutWord(regBatPos,0); CHK;
        ret= mapMove(batchBeg+1,0); CHK;
        ret= mapPutWord(batBatchNo,2); CHK;
        ret= mapPutStr(batTrasRecCnt,"000000001"); CHK;
        ret= mapPutStr(batAmtTot,"0000000000000800"); CHK;
        ret= mapPutStr(batCashbackTot,"0000000000000000"); CHK;
        ret= mapPutStr(batNetDeposit,"0000000000002500"); CHK;
  
        ret= mapPutWord(regLogPos,1);   

    //set permission to fullbatch transmission
        bByte = 0x02;
        ret = mapMove(perBeg+1,TerminalPer); 
        ret = mapPutByte(perBatchTrf,bByte); 


        //Phone number 
        ret= mapMove(acqBeg+1,7); 
        //ret= mapPutStr(acqPriPhone,"4001487");        
        ret= mapPutStr(acqPriPhone,"5739374");  
    ret= mapPutStr(acqSecPhone,"5739374");      
        return 1;    
lblKO:
    trcErr(ret);
        return -1;
}    



static int printData(void){
    int ret;
    char buf[256];
        
    ret= prtS("RspCod:"); CHK;
    ret= mapGet(traRspCod,buf,256); CHK;
    ret= prtS(buf); CHK;

    ret= prtS("CfmReq:"); CHK;
    ret= mapGet(traCfmReq,buf,256); CHK;
    ret= prtS(buf); CHK;

    ret= prtS("AutCod:"); CHK;
    ret= mapGet(traAutCod,buf,256); CHK;
    ret= prtS(buf); CHK;

    ret= prtS("SrvMsg:"); CHK;
    ret= mapGet(traSrvMsg,buf,256); CHK;
    ret= prtS(buf); CHK;

    return 1;    
lblKO:
    trcErr(ret);
    return -1;
}

void tcmt0004(void){
    int ret;
        byte bByte;
    //set permission to fullbatch transmission
        bByte = 0x02;
        ret = mapMove(perBeg+1,TerminalPer); 
        ret = mapPutByte(perBatchTrf,bByte); 

        ret= mapPutWord(regLogPos,1);   

    ret= dspClear(); CHECK(ret>=0,lblKO);

    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    
    ret= prepare01(); CHECK(ret>=0,lblKO);
    
    ret= dspLS(1,"onlMdc..."); CHECK(ret>=0,lblKO);    
    ret= onlMdc(); CHECK(ret>=0,lblKO);
    ret= printData(); CHECK(ret>=0,lblKO);

    goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
}
#endif
