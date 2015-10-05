#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcmt0008.
//Transaction log saving, reading, printing.

//Please don't use sprintf!

#define CHK CHECK(ret>=0,lblKO)
static int prepare00(void){
    int ret;
    ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPutWord(traMnuItm,mnuSale); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"111",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
    ret= mapPutByte(traAuthSrcCod,'1'); CHK;
    ret= mapPutStr(traAutCod,"7458     "); CHK;
    ret= mapMove(issBeg+1,1); 
    ret= mapPutByte(issId,1); CHK;
    ret= mapMove(txnTypeBeg+1,1); 
    ret= mapPutWord(regMsgNum,1); CHK;
    ret= mapPutByte(traMsgType,10); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
    ret= mapPutWord(regMsgNum,1);    
        ret= mapPutWord(traSeqNo,1); CHK;        
        return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

static int prepare01(void){
    int ret;
    ret= mapPutStr(traPan,"5425364009408223FFFFFF"); CHK;
    ret= mapPutWord(traMnuItm,mnuSale); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"222",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'1'); CHK;
        ret= mapPutStr(traAutCod,"7459     "); CHK;
    ret= mapMove(issBeg+1,1); 
    ret= mapPutByte(issId,2); CHK;
    ret= mapMove(txnTypeBeg+1,2); 
    ret= mapPutWord(regMsgNum,2); CHK;
    ret= mapPutByte(traMsgType,20); CHK;
    ret= mapPutStr(traOpName,"melineh"); CHK;
    ret= mapPutWord(regMsgNum,2);
        ret= mapPutWord(traSeqNo,2); CHK;        
    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

static int printData(void){
    int ret;
        int i;
    char buf[256]="";
        word num = 0;
    byte b;
    byte TxnDate[3];
    char CharTxnDate[2];
    byte TxnTime[3];
    byte bPan[(lenPan/2)+1];
    char cPan[lenPan+1];
    byte bAmt[(lenAmt+1)/2];
    char cAmt[lenAmt+1];

        for(i = 0 ; i< 2 ; ++i){
                ret = mapMove(logBeg+1,(word)i);

                ret= prtS("logBatchNo:"); CHK;
                ret= mapGetWord(logBatchNo,buf); CHK;
                ret= num2dec(buf,num,3); CHK;
                ret= prtS(buf); CHK;

                ret= prtS("logPosTxnNo:"); CHK;
                ret= mapGet(logPosTxnNo,buf,0); CHK;
                ret= num2dec(buf,num,4);      
                ret= prtS(buf); CHK;

                buf[0]= '\0';
                ret= prtS("logTxnDate:"); CHK;
                ret= mapGet(logTxnDate,TxnDate,3); CHK;
                bin2hex(CharTxnDate,TxnDate+1,2);
                //sprintf(buf,"%.2s %.2s",CharTxnDate+2,CharTxnDate);
                ret= prtS(buf); CHK;

                ret= prtS("logPAN:"); CHK;
                ret= mapGet(logPAN,bPan,(lenPan/2)+1); CHK;
                ret= bin2hex(cPan,bPan,(lenPan/2)); VERIFY(ret==(lenPan));
                ret= prtS(cPan); CHK;


                ret= prtS("logAccountDataSourceCode:"); CHK;
                ret= mapGetByte(logAccountDataSourceCode,b); CHK;
                //sprintf(buf,"%c",b);
                ret= prtS(buf); CHK;

                ret= prtS("logCrdHldrIDCod:"); CHK;
                ret= mapGetByte(logCrdHldrIDCod,b); CHK;
                //sprintf(buf,"%c",b);
                ret= prtS(buf); CHK;

                ret= prtS("logTxnAmount:"); CHK;
                ret= mapGet(logTxnAmount,bAmt,(lenAmt+1)/2); CHK;
                ret= bin2hex(cAmt,bAmt,(lenAmt+1)/2); VERIFY(ret==(lenAmt));
                ret= prtS(cAmt); CHK;

                ret= prtS("logVoidIndicator:"); CHK;
                ret= mapGetByte(logVoidIndicator,b); CHK;
                //sprintf(buf,"%c",b);
                ret= prtS(buf); CHK;

                ret= prtS("logAuthorisationSourceCode:"); CHK;
                ret= mapGetByte(logAuthorisationSourceCode,b); CHK;
                //sprintf(buf,"%c",b);
                ret= prtS(buf); CHK;

                ret= prtS("logApprovalCode:"); CHK;
                ret= mapGet(logApprovalCode,buf,0); CHK;
                ret= prtS(buf); CHK;
          
                ret= prtS("logTxnTime:"); CHK;
                ret= mapGet(logTxnTime,TxnTime,3); CHK;
                bin2hex(buf,TxnTime,2); CHK;  
                ret= prtS(buf); CHK;

                ret= prtS("logMsgType:"); CHK;
                ret= mapGetByte(logMsgType,b); CHK;
                num2dec(buf,b,2); CHK;        
                ret= prtS(buf); CHK;

                ret= prtS("logIssRow:"); CHK;
                ret= mapGetByte(logIssRow,b); CHK;
                num2dec(buf,b,0); CHK;        
                ret= prtS(buf); CHK;

                ret= prtS("logTxnTypeRow:"); CHK;
                ret= mapGetByte(logTxnTypeRow,b); CHK;
                num2dec(buf,b,0); CHK;        
                ret= prtS(buf); CHK;


                ret= prtS("logOpName:"); CHK;
                ret= mapGet(logOpName,buf,0); CHK;
                ret= prtS(buf); CHK;

                ret= prtS("-----------------------"); CHK;
        }

    return 1;    
lblKO:
   trcErr(ret);
  return -1;
}

void tcmt0008(void){
    int ret;

    ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= mapPutWord(regLogPos,0); CHK; //reset transaction log position

    ret= dspLS(0,"prepare00..."); CHECK(ret>=0,lblKO);    
    ret= prepare00(); CHECK(ret>=0,lblKO);
        tmrPause(1);
    
    ret= dspLS(1,"Save 00..."); CHECK(ret>=0,lblKO);    
//    ret= logSave(INSERT); CHECK(ret>=0,lblKO);
        tmrPause(1);
    
    ret= dspLS(2,"prepare01..."); CHECK(ret>=0,lblKO);    
    ret= prepare01(); CHECK(ret>=0,lblKO);
        tmrPause(1);
    
    ret= dspLS(3,"Save 01..."); CHECK(ret>=0,lblKO);    
//    ret= logSave(INSERT); CHECK(ret>=0,lblKO);
        tmrPause(1);
    
    ret= printData(); CHECK(ret>=0,lblKO);

    goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
}
#endif
