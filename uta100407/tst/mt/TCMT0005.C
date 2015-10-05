#include <string.h>
#include <stdio.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;


//Integration test case tcab0005.
//save one transaction log,calculate totalas and Save Batch 

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(void){
    int ret;
        //frist record
    ret= mapPutStr(traPan,"3333364009408223FFFFFF"); CHK;
    ret= mapPutWord(traMnuItm,mnuSale); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    ret= mapPutStr(traPan,"3333364009408223FFFFFF"); CHK;
    ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    ret= mapPut(traAmt,"3112",lenAmt); CHK;
    ret= mapPutByte(traCrdHldrIDCod,'@'); CHK;
        ret= mapPutByte(traAuthSrcCod,'1'); CHK;
        ret= mapPutStr(traAutCod,"7458     "); CHK;
    ret= mapMove(issBeg+1,1); 
        ret= mapPutByte(issId,1); CHK;
    ret= mapMove(txnTypeBeg+1,txnPreAuth); 
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




static int printData(void){
    int ret;
        word num;
        char Buf[256]="";

        ret= mapMove(batchBeg+1,0); CHK; 
        //todo
        //get date from batch table
        ret= mapGetWord(batBatchNo,num); CHK;
        num2dec(Buf,num,3);     
        prtS("batchNo:");
        prtS(Buf);

        ret= mapGet(batTrasRecCnt,Buf,lenTrasRecCnt); CHK;
        prtS("batTrasRecCnt:");
        prtS(Buf);

        ret= mapGet(batAmtTot,Buf,0); CHK;
        prtS("batAmtTot:");
        prtS(Buf);

        ret= mapGet(batNetDeposit,Buf,0); CHK;
        prtS("batNetDeposit:");
        prtS(Buf);

    return 1;    
lblKO:
    trcErr(ret);
  return -1;
}

void tcmt0005(void){
        int ret;
        
        ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= mapPutWord(regLogPos,0); CHK; //reset transaction log position
        ret= mapPutWord(regBatPos,0); CHK; //reset batch position
        

    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    
    ret= prepare01(); CHECK(ret>=0,lblKO);
        tmrPause(1);
    
    ret= dspLS(1,"Save 01..."); CHECK(ret>=0,lblKO);    
//    ret= logSave(INSERT); CHECK(ret>=0,lblKO);
        tmrPause(1);
    

        ret= dspLS(3,"calcTotals..."); CHECK(ret>=0,lblKO);    
        ret= calcTotals(0); CHECK(ret>=0,lblKO);
        tmrPause(1);

        ret= mapMove(batchBeg+1,0); CHK; //move to the first record in batch
        ret= mapPutWord(batBatchNo,1); CHK; //set batch no to 1
        ret= dspLS(3,"saveBatch..."); CHECK(ret>=0,lblKO);    
        ret= batchSave(); CHECK(ret>=0,lblKO);
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
