#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcmt0001.
//Building the MDC request.

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
    //Header
        ret= mapPutWord(batBatchNo,0); CHK;
        mapMove(acqBeg+1,7);
        ret= mapPutStr(acqMerchantNo,"999999999999"); CHK;
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
        ret= mapPutStr(appMerZipCode,"     "); CHK;
        ret= mapPutStr(appMerCountryCode,"784"); CHK;   
        ret= mapPutStr(appMerchantSecurityCode,"12345"); CHK;   

        //Detail
        mapMove(logBeg+1,0);

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
        ret= mapPut(logAuthorisationSourceCode,"1",1); CHK;
        ret= mapPutStr(logApprovalCode,"7458     "); CHK;
    ret= mapPutWord(logPosTxnNo,21); CHK;    
        

    ret= mapPut(traAmt,"500",lenAmt); CHK;
    ret= mapGet(traAmt,cAmt,lenAmt+1); CHK;
    ret= fmtPad(cAmt,-lenAmt,'0'); VERIFY(ret==lenAmt); //left-pad
    ret= hex2bin(bAmt,cAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt+1)/2);
    ret= mapPut(logCashbackAmount,bAmt,(lenAmt+1)/2); CHK;

        
        //Trailer
        ret= mapPutWord(regBatPos,0); CHK;
        ret= mapMove(batchBeg+1,0); CHK;
        ret= mapPutStr(batTrasRecCnt,"000000004"); CHK;
        ret= mapPutStr(batAmtTot,"0000000000000300"); CHK;
        ret= mapPutStr(batCashbackTot,"0000000000000200"); CHK;
        ret= mapPutStr(batNetDeposit,"0000000000000100"); CHK;


    //set permission to fullbatch transmission
        bByte = 0x02;
        ret = mapMove(perBeg+1,TerminalPer); 
        ret = mapPutByte(perBatchTrf,bByte); 


    return 1;    
lblKO:
    trcErr(ret);
        return -1;
}

static int printReq(const char *hdr,const char *req,word len){
    int ret;
    char tmp[prtW+1];
    
    ret= prtS(hdr); CHK;
    tmp[prtW]= 0;
    while(len){
       if(strlen(req)<=prtW){
            strcpy(tmp,req);
            len= 0;
       }else{
            memcpy(tmp,req,prtW);
            len-= prtW;
                        req+= prtW;
       }
       ret= prtS(tmp); CHK;       
    }
    return 1;
lblKO:
    trcErr(ret);
        return -1;
}

void tcmt0001(void){
    int ret;
    byte dReq[256];
    tBuffer bReq;
        word txnCount; 
        
        txnCount = 0;
    ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    
    ret= prepare01(); CHECK(ret>=0,lblKO);


    ret= dspLS(1,"build01..."); CHECK(ret>=0,lblKO);    
    bufInit(&bReq,dReq,256);

        ret= reqMdcHeader(&bReq); CHECK(ret>=0,lblKO);
        ret= reqMdcTermParam(&bReq); CHECK(ret>=0,lblKO);
        ret= reqMdcDetailTxn(&bReq,&txnCount); CHECK(ret>=0,lblKO);

    trcFS("req=[%s]\n",(char *)bufPtr(&bReq));
    ret= printReq("  request block01",(char *)bufPtr(&bReq),bufLen(&bReq)); CHECK(ret>=0,lblKO);
    
        bufInit(&bReq,dReq,256);
        mapMove(logBeg+1,0);
        ret= reqMdcTrailer(&bReq); CHECK(ret>=0,lblKO);
    trcFS("req=[%s]\n",(char *)bufPtr(&bReq));
    ret= printReq("  request block02",(char *)bufPtr(&bReq),bufLen(&bReq)); CHECK(ret>=0,lblKO);
  
        
        tmrPause(3);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    tmrStop(1);
}
#endif
