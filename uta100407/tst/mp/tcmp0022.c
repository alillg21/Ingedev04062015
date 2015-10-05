#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"
#include "emv.h"

#define CLESS_SAMPLE_TRANSACTION_TYPE_DEBIT			0x00		/*!< Definition of debit transaction type. */
#define CLESS_SAMPLE_TRANSACTION_TYPE_REFUND		0x20		/*!< Definition of refund transaction type. */

static void loadAidParameter( tQueue * que ) {
    quePutTlv(que, cmvTagDetectType, 4 ,"\x03\x00\x00\x00") ;
    quePutTlv(que, cmvTagDetectNBtoDetect, 1, (byte*)"\x01") ;
    quePutTlv(que, cmvTagGlobalTO, 4 , "\x00\x00\x17\x70") ;

    quePutTlv(que, cmvTagAidInfo , 1, "\x00");	
    quePutTlv(que, tagAIDT , 7, "\xA0\x00\x00\x00\x04\x10\x10 ");	
    quePutTlv(que, cmvTagKernelToUse, 2, "\x00\x02");	
    quePutTlv(que, cmvTagAidOpt, 4, "\x05\x01\x00\x00");//partial aid and ppse of payPass	
    quePutTlv(que, cmvTagClessTxnLim , 6, "\x00\x00\x00\x00\x20\x00");     
    quePutTlv(que, cmvTagClessCvmReqdLimit , 6, "\x00\x00\x00\x00\x10\x00");	
    quePutTlv(que, cmvTagClessFlrLim, 6, "\x00\x00\x00\x00\x15\x00");		
    quePutTlv(que, tagTrmFlrLim , 4, "\x00\x00\x05\xAA"); 			

    quePutTlv(que, tagAmtBin , 4, "\x00\x00\x00\x7B" ); 	
}

static void loadTransData( tQueue * que ) {	
    quePutTlv(que, tagTrnDat , 3, "\x10\x10\x20"); 
    quePutTlv(que, tagTrnTim , 3, "\x00\x01\x02");
    quePutTlv(que, tagAmtBin , 4, "\x00\x00\x00\x7B" ); 
    quePutTlv(que, tagAmtNum, 6, "\x00\x00\x00\x00\x01\x23"); 
    quePutTlv(que, tagAmtOthBin , 4, "\x00\x00\x00\x00" ); 
    quePutTlv(que, tagAmtOthNum, 6, "\x00\x00\x00\x00\x00\x00");     
    quePutTlv(que, tagTrnCurCod , 2, "\x00\x56"); 
    quePutTlv(que, tagTrnCurExp , 1, "\x02");
    quePutTlv(que, tagPPassIntTxnType , 1, "\x00");
    quePutTlv(que, tagTrmTyp, 1, "\x00");
    quePutTlv(que, tagIFDSerNum, 8, "\x12\x34\x56\x78\x90\x09\x87\x65");// Serial Number
    quePutTlv(que, tagTrnSeqCnt , 4, "\x00\x00\x00\x02");    

    //paypass internal tags	
    quePutTlv(que, tagPPassIntMagTrmAVNList , 6, "\x01\x06\x02\x01\x02\x06"); 		
    quePutTlv(que, tagPPassIntChipTrmAVNList , 6, "\x01\x05\x02\x00\x02\x05"); 	
    quePutTlv(que, tagPPassTrmCapCvmReq , 3, "\xE0\x68\xC8"); 				
    quePutTlv(que, tagPPassTrmCapNoCvmReq , 3, "\xE0\x08\xC8"); 		
    quePutTlv(que, tagPPassDftUDOL , 3, "\x9F\x6A\x04");
    quePutTlv(que, tagPPassMagIndicator , 1, "\x01");	
    quePutTlv(que, tagPPassIntTACDft , 5, "\x00\x00\x00\x00\x00");		
    quePutTlv(que, tagPPassIntTACDnl , 5, "\x00\x00\x00\x00\x00");		
    quePutTlv(que, tagPPassIntTACOnl , 5, "\x00\x00\x00\x00\x00");		
    quePutTlv(que, tagPPassTxnCatCod , 1, "\x52");
    quePutTlv(que, tagPPassIntDftTDOL , 3, "\x9F\x08\x02");	
    quePutTlv(que, cmvKernelTrmLang , 4, "\x65\x6E\x66\x72");			
}

void tcmp0022(void) {
    int ret;    
    byte queBuf[4096];	
    char tagCBuf[256];    
    tQueue que;
    tBuffer tBuf;
    
    prtStart();
    dspStart(); 
    clsStart();

    bufInit(&tBuf ,queBuf , 4096);
    queInit(&que, &tBuf);	
		
    loadAidParameter(&que);
    ret = clsMVInit(&que,1);
    CHECK(ret > 0, lblKO);
	
    ret = clsMVDetect();
    CHECK(ret > 0, lblKO);	
	
    clsMVSelect();
    CHECK(ret > 0, lblKO);	
	
    bufInit(&tBuf ,queBuf , 4096);
    queInit(&que, &tBuf);	

    loadTransData(&que);

    ret = clsMVFinalSelect(&que);
    CHECK(ret > 0, lblKO);		
	
    ret = clsMVContext(&que);
    CHECK(ret > 0, lblKO);		
    
    ret = clsMVPayPassPerformTransaction();
    sprintf(tagCBuf, "Paypass Trans:%d", ret);
    prtS(tagCBuf);
    switch( ret ) {
        case cmvKernelStatusOfflineTC: 
            prtS("Approved Offline");
            break;
        
        case cmvKernelStatusOfflineAAC:
            prtS("Decline Offline");
            break;
        
        case cmvKernelStatusARQC:
            prtS("Approved Offline");
            break;
        
        case cmvKernelStatusUseContactInterface:
            prtS("Use Contact Interface");
            break;
        
        case cmvKernelStatusCommError:
            prtS("Communication Error");
            break;
        
        case cmvKernelStatusCardBlock:
            prtS("Card Block");
            break;
        
        case cmvKernelStatusAppBlock:
            prtS("App Block");
            break;
        
        default: // Error case
            prtS("Unknown Error");
            break;        
    }
    
    goto lblEnd;
    lblKO:
        ret= dspLS(1, "CLSMV KO!"); 
	 CHECK(ret>=0,lblKO);
        trcErr(ret);
    lblEnd:
        clsStop() ;
        prtStop();
        dspStop();
        tmrPause(3);	
        trcS("tcmp0022 End\n");
}

