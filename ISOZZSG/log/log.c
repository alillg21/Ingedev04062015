//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/log.c $
//$Id: log.c 1917 2009-04-22 13:24:29Z abarantsev $

/** \file
 * Transaction log processing
 * Returns 1 if succesful, <0 if it encountered some errors
 */

#include <string.h>
#include <stdio.h>
#include "log.h"
#include "bri.h"
#include "prepaid.h"

#define CHK if(ret<0) return ret;

static byte setDuplicate;       //Check if Receipt settlement is a Duplicate

static word logKey[logEnd - logBeg - 1] = {
            traVoidFlag,                ///<logVoidFlag: Indicator if this txn is already void
            traTipFlag,
            traMti,
            traMnuItm,                  ///<logMnuItm: menu item selected
            traDatTim,                  ///<logDatTim: current date and time CCYYMMDDhhmmss
            traBinIdx,                  ///<logBinIdx: index in Bin multirecord
            traAcqIdx,                  ///<logAcqIdx: index in Acq multirecord
            traIssIdx,                  ///<logIssIdx: index in Iss multirecord
            traPan,                     ///<logPan: card number
            traExpDat,                  ///<logExpDat: expiry date
            traExpDatFmt,			    //<logExpDatFmt
            traTrk2,                    ///<logTrk2: rack 2 of the card
            traAutCod,                  ///<logAutCod: Authorization (approval) code
            regAutCodOff,
            acqCurBat,                  ///<logBatNum: batch number
            traAmt,                     ///<logTraAmt: Transaction Amount
            traFmtAmt,		            //amount formatted
            traTipAmt,                  ///<logTipAmt:  Transaction Tip Amount
            traFmtTip,
            traEmvICC,		            //revICCData, bit 55
            regSTAN,                    ///<logSTAN: System Trace Audit Number 1..999999
            traRrn,                     ///<logRrn: Retrieval Reference Number
            traRspCod,		            //response code
            traPinBlk,
            regInvNum,                  ///<logROC: invoice number / R.O.C.
            traEntMod,                  ///<logEntMod: POS Entry Mode
            traConCode,                 ///<logConCod: POS Condition Code
            traCardType,                 ///<logCardType: Card Type
//++ @agmr - BRI
            traTrk1,
            emvAppPrfNam,
            emvAid,
            emvTrk2,
            traEmvFullFlag,
            traEmvTVR,
            traEmvIssTrnCrt,
//-- @agmr - BRI
            traInstallmentPlan, ///<Installment plan
            traInstallmentTerm, ///<Installment term
            traInstallmentCicilan, //BRI7
            traSisaSaldo,       //untuk sisa saldo brizzi
            traPasswordPetugas,
            traPeriode,
            traUserId,
            traPokokHutang,
            traNoVoucher,
            traBuffer1,
            traBuffer2,         //@agmr - brizzi2

            logIsUnusedRec,      //@agmr - multi settle
            logIsUploaded,
            traKategoriKartu,

	     traTotalAmount,
	     appBriziDiscFlag,
	     appBriziDiscValue
        };

static word logKeyContext[logEnd - logBeg - 1] = {  //for some unknown reason , logEnd - logBeg doesnt work... for now hardcoded...
            traVoidFlagContext,         ///<logVoidFlag: Indicator if this txn is already void
            traMnuItmContext,           ///<logMnuItm: menu item selected
            traDatTimContext,           ///<logDatTim: current date and time CCYYMMDDhhmmss
            traBinIdxContext,           ///<logBinIdx: index in Bin multirecord
            traAcqIdxContext,           ///<logAcqIdx: index in Acq multirecord
            traIssIdxContext,           ///<logIssIdx: index in Iss multirecord
            traPanContext,              ///<logPan: card number
            traExpDatContext,           ///<logExpDat: expiry date
            traTrk2Context,             ///<logTrk2: rack 2 of the card
            traAutCodContext,           ///<logAutCod: Authorization (approval) code
            regBatNumContext,           ///<logBatNum: batch number
            traAmtContext,              ///<logTraAmt: Transaction Amount
            traTipAmtContext,           ///<logTipAmt:  Transaction Tip Amount
            regSTANContext,             ///<logSTAN: System Trace Audit Number 1..999999
            traRrnContext,              ///<logRrn: Retrieval Reference Number
            regInvNumContext,           ///<logROC: invoice number / R.O.C.
            traEntModContext,           ///<logEntMod: POS Entry Mode
            traConCodeContext,          ///<logConCod: POS Condition Code
            traCardTypeContext          ///<logCardType: Card Type
        };

extern byte txnAsli;

/** Save current transaction data ( tra / reg)elements into a tra for temporary storage.
 * Used in calling logLoad as the data from log will be placed into their respective source as from log.
 * \header log\\log.h
 * \source log\\log.c
 */
int logSaveCurContext(void)
{
	int ret;
	byte idx;
	word key;
	byte buf[256];

	trcS("logSaveCurContext Beg\n");
	for (key = logBeg + 1; key < logEnd; key++) // Loop around each field from the record of the "log" table
	{
		idx = (byte) (key - logBeg - 1);    // Index key field
		VERIFY(isSorted(traBeg, logKey[idx], traEnd)
		       || isSorted(regBeg, logKey[idx], regEnd)
		       || isSorted(acqBeg, logKey[idx], acqEnd));   //all the keys in logKey array can only be from tra or reg records
		key = logKey[idx];
		MAPGET(key, buf, lblDBA);
		ret = mapPut(logKeyContext[idx], buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}
	ret = 1;
	goto lblEnd;
lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("logSaveCurContext: ret=%d #\n", ret);
	return ret;

}

/** Load data elements from temporary storage to transaction data( tra / reg).
 * Used to restore the current transaction data before calling logLoad.
 * \header log\\log.h
 * \source log\\log.c
 */

int logLoadCurContext(void)
{
	int ret;
	word key;
	byte idx;
	byte buf[256];
	card dupStan, dupROC;

	trcS("logLoadCurContext Beg\n");

	for (key = logBeg + 1; key < logEnd; key++) // Loop around each field from the record of the "log" table
	{
		idx = (byte) (key - logBeg - 1);    // Index key field
		VERIFY(isSorted(traBeg, logKey[idx], traEnd)
		       || isSorted(regBeg, logKey[idx], regEnd)
		       || isSorted(acqBeg, logKey[idx], acqEnd));   //all the keys in logKey array can only be from tra or reg records
		key = logKeyContext[idx];
		MAPGET(key, buf, lblDBA);
		ret = mapPut(logKey[idx], buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}

	//-Create Copy-
	MAPGETCARD(regDupSTAN, dupStan, lblDBA);
	if(dupStan >= 1000000) //@agmr
	    dupStan-= 1000000;
	mapPutCard(regSTAN, dupStan);
	MAPGETCARD(regDupInvNum, dupROC, lblDBA);
	if(dupROC >= 1000000) //@agmr
	    dupROC -= 1000000;
	mapPutCard(regInvNum, dupROC);

	ret = 1;
	goto lblEnd;

lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("logLoadCurContext: ret=%d #\n", ret);
	return ret;
}

/** Save current transaction data elements into a record of the Log table
 * \header log\\log.h
 * \source log\\log.c
 */
static int logAppend(void)
{
	word key, trxNo;
	byte idx;
	byte buf[256];
	int ret;

	// Save transaction from tra record into record of the log table
	trcS("logAppend Beg\n");

	MAPPUTBYTE(traVoidFlag, 0, lblDBA);
	MAPGETWORD(regTrxNo, trxNo, lblDBA);

//  ++@agmr - BRI1
	ret = whichBRITransaction();
    if(ret!=0)
    {
        return(briLogSave(ret));
    }
//  --@agmr - BRI1

	mapMove(logBeg, trxNo);
	//mapCache(logBeg);       //AB: performance issue(later)  Enable cache on log
	for (key = logBeg + 1; key < logEnd; key++)
	{   // Loop around each field from the record of the "log" table
		idx = (byte) (key - logBeg - 1);    //index within logKey array
		VERIFY(isSorted(traBeg, logKey[idx], traEnd)
		       || isSorted(regBeg, logKey[idx], regEnd)
		       || isSorted(acqBeg, logKey[idx], acqEnd));   //all the keys in logKey array can only be from tra or reg records
		MAPGET(logKey[idx], buf, lblDBA);
		ret = mapPut(key, buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}

//++ @agmr
    MAPPUTBYTE(logIsUploaded,0,lblDBA);

	memset(buf,0,sizeof(buf));
    MAPGET(acqName, buf, lblEnd);
    if(memcmp(buf,"BRIZZI",6)==0)
    {
        MAPPUTWORD(appPrepaidPrintLastTransIdx, trxNo, lblDBA);
    }
    else
    {
	    MAPPUTWORD(appPrintLastTransIdx, trxNo, lblDBA);
	}
//-- @agmr

    ret = incCard(regTrxNo);

	//ret = mapSave(logBeg); //AB: performance issue(later)  // Log copy from RAM to DFS
	//CHECK(ret >= 0, lblDBA);

	MAPGETWORD(regTrxNo, trxNo, lblDBA);
	MAPPUTWORD(regCurrIdxPtr, trxNo, lblDBA);
	CHK;

	ret = 1;
	goto lblEnd;

lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	//mapCache(-logBeg);      ////AB: performance issue(later) Disable cache on log
	trcFN("logAppend: ret=%d #\n", ret);
	return ret;
}

/** Save current transaction data elements into a record of the Log table
 * \header log\\log.h
 * \source log\\log.c
 */
static int logUpdate(void)
{
	word key, trxNo;
	byte idx;
	byte logIdx;
	byte buf[256];
	char rocRef[lenInvNum + 1];
	card rocNum, logRoc;
	int ret;
//	word keyTemp/*,trxTempNo*/;

	// Save transaction from tra record into record of the log table
	trcS("logUpdate Beg\n");
//++ @agmr - BRI
	ret = whichBRITransaction();
    if(ret!=0)
    {
        return(briLogUpdate(ret));
    }
//-- @agmr - BRI

	MAPGETWORD(regTrxNo, trxNo, lblDBA);
	MAPGET(traFindRoc, rocRef, lblDBA);
	dec2num(&rocNum, rocRef, sizeof(rocRef));

	//Search the record with ROC from traFindRoc
	for (logIdx = 0; logIdx < trxNo; logIdx++)
	{
		mapMove(logBeg, logIdx);
		mapMove(logTempBeg,logIdx); //@agmr - multi settle
		MAPGETCARD(logROC, logRoc, lblDBA);
		if(logRoc == rocNum)
			break;
	}
	CHECK(logIdx <= trxNo, lblDBA);

//++ @agmr
	memset(buf,0,sizeof(buf));
    MAPGET(acqName, buf, lblEnd);
    if(memcmp(buf,"BRIZZI",6)==0)
    {
        MAPPUTWORD(appPrepaidPrintLastTransIdx, logIdx, lblDBA);
//        MAPPUTWORD(appPrepaidPrintLastTransIdx, trxNo, lblDBA);
    }
    else
    {
        MAPPUTWORD(appPrintLastTransIdx, logIdx, lblDBA);
//	    MAPPUTWORD(appPrintLastTransIdx, trxNo, lblDBA);
	}
//-- @agmr

	//mapCache(logBeg);       //AB: performance issue(later)  Enable cache on log
	for (key = logBeg + 1; key < logEnd; key++)
	{   // Loop around each field from the record of the "log" table
		idx = (byte) (key - logBeg - 1);    //index within logKey array
		VERIFY(isSorted(traBeg, logKey[idx], traEnd)
		       || isSorted(regBeg, logKey[idx], regEnd)
		       || isSorted(acqBeg, logKey[idx], acqEnd));   //all the keys in logKey array can only be from tra or reg records
		MAPGET(logKey[idx], buf, lblDBA);
		ret = mapPut(key, buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}
	//ret = mapSave(logBeg); //AB: performance issue(later)  // Log copy from RAM to DFS
	//CHECK(ret >= 0, lblDBA);

//	MAPPUTWORD(regCurrIdxPtr, logIdx, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	//mapCache(-logBeg);      ////AB: performance issue(later) Disable cache on log
	trcFN("logUpdate: ret=%d #\n", ret);
	return ret;
}

/** Function that will determine if a transaction will be updated or appended in the log database.
 * \header log\\log.h
 * \source log\\log.c
 */
int logSave(void)
{
	int ret;
	word mnuItem, offlineCnt;

	MAPGETWORD(traMnuItm, mnuItem, lblDBA);

	switch (mnuItem)
	{
//++ @agmr - auth tidak disimpan ke batch
	    	case mnuAut:
		case mnuPreAuth:
		case mnuPreCancel:
		//case mnuPreCom:
	       	return 1;

//-- @agmr - auth tidak disimpan ke batch
			/// if the transaction will be voided, update the record in traFindRoc
		case mnuVoid:
			/// enable void flag
			MAPPUTBYTE(traVoidFlag, 1, lblDBA);
			/// get original transaction type and place in buffer
			MAPGETWORD(traMnuItmContext, mnuItem, lblDBA);
			/// temporary replace it so the original transactin type will be used
			MAPPUTWORD(traMnuItm, mnuItem, lblDBA);
			{
			    byte txn;

			    MAPGETBYTE(traOriginalTxnType,txn,lblDBA);
			    switch(txn)
			    {
			        case trtSale:
			            MAPPUTWORD(traMnuItm, mnuSale, lblDBA);
			            break;
				 case trtSaleRedeem: //@ar -BRI
			            MAPPUTWORD(traMnuItm, mnuSaleRedeem, lblDBA);
			            break;
			        case trtInstallment:
			            MAPPUTWORD(traMnuItm, mnuInstallment, lblDBA);
			            break;
			        case trtRefund:
			            MAPPUTWORD(traMnuItm, mnuRefund, lblDBA);
			            break;
				case trtPreCom:
			            MAPPUTWORD(traMnuItm, mnuPreCom, lblDBA);
			            break;
			    }
			}
			ret = logUpdate();
			/// return the original transaction
			MAPPUTWORD(traMnuItm, mnuVoid, lblDBA);
			/// disable void flag again for next transaction
			MAPPUTBYTE(traVoidFlag, 0, lblDBA);
			CHECK(ret >= 0, lblDBA);
			break;
		case mnuVoidTarikTunai: //@agmr - BRI
			/// enable void flag
			MAPPUTBYTE(traVoidFlag, 1, lblDBA);
			/// get original transaction type and place in buffer
			MAPGETWORD(traMnuItmContext, mnuItem, lblDBA);
			/// temporary replace it so the original transactin type will be used
			MAPPUTWORD(traMnuItm, mnuItem, lblDBA);
			ret = logUpdate();
			/// return the original transaction
			MAPPUTWORD(traMnuItm, mnuVoidTarikTunai, lblDBA);
			/// disable void flag again for next transaction
			MAPPUTBYTE(traVoidFlag, 0, lblDBA);
			CHECK(ret >= 0, lblDBA);

		    MAPPUTBYTE(traTxnType,trtVoidTarikTunai,lblDBA);

			break;
#ifdef PREPAID
		//++ @agmr - brizzi2
        case mnuPrepaidVoid:
			/// enable void flag
			MAPPUTBYTE(traVoidFlag, 1, lblDBA);
			/// get original transaction type and place in buffer
			MAPGETWORD(traMnuItmContext, mnuItem, lblDBA);
			/// temporary replace it so the original transactin type will be used
			MAPPUTWORD(traMnuItm, mnuItem, lblDBA);
			ret = logUpdate();
			/// return the original transaction
			MAPPUTWORD(traMnuItm, mnuVoidTarikTunai, lblDBA);
			/// disable void flag again for next transaction
			MAPPUTBYTE(traVoidFlag, 0, lblDBA);
			CHECK(ret >= 0, lblDBA);

		    MAPPUTBYTE(traTxnType,trtPrepaidVoid,lblDBA);
            break;
//-- @agmr - brizzi2
		case mnuInfoDeposit:
		case mnuPrepaidInfoSaldo:
		case mnuPrepaidSettlement:
		case mnuPrepaidInfoKartu:
		    return 1;
#endif
		case mnuOffline:
			MAPGETWORD(regOfflineCnt, offlineCnt, lblDBA);
			offlineCnt++;
			MAPPUTWORD(regOfflineCnt, offlineCnt, lblDBA);
			///else just add this transaction
		default:
			ret = logAppend();
			CHECK(ret >= 0, lblDBA);
			break;
	}
	ret = 1;
	goto lblEnd;

lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("logSave: ret=%d #\n", ret);
	return ret;
}

/** Retrieve the data from log with the reference from TraFindRoc to tra/reg records
 * \header log\\log.h
 * \source log\\log.c
 */
int logLoad(void)
{
	word key, trxNo;
	byte idx;
	byte logIdx;
	byte buf[256];
	char rocRef[lenInvNum + 1];
	card rocNum, logRoc, dupStan, dupROC;
	int ret;

	// Retrieve transaction from a record of the log table into respective tra/reg record
	trcS("logLoad Beg\n");
	//mapCache(logBeg);       // Enable cache on log
	//ret = mapLoad(logBeg);

	MAPGET(traFindRoc, rocRef, lblDBA);
	dec2num(&rocNum, rocRef, sizeof(rocRef));

	MAPGETWORD(regTrxNo, trxNo, lblDBA);

	///Search the record with ROC from traFindRoc
	for (logIdx = 0; logIdx < trxNo; logIdx++)
	{
		mapMove(logBeg, logIdx);
		MAPGETCARD(logROC, logRoc, lblDBA);
		if(logRoc == rocNum)
			break;
	}
	CHECK(logIdx < trxNo, lblTraceNotFound);

	//-Create Copy-
//	MAPGETCARD(regSTAN, dupStan, lblDBA);
//	MAPPUTCARD(regDupSTAN, dupStan, lblDBA);
//	MAPGETCARD(regInvNum, dupROC, lblDBA);
//	MAPPUTCARD(regDupInvNum, dupROC, lblDBA);
	MAPGETCARD(regSTAN, dupStan, lblDBA);
	MAPPUTCARD(regDupSTAN, dupStan, lblDBA);
//    MAPPUTCARD(regDupSTAN, dupStan+1000000, lblDBA); //@agmr - untuk perbandingan nanti
	MAPGETCARD(regInvNum, dupROC, lblDBA);
//	MAPPUTCARD(regDupInvNum, dupROC, lblDBA);
	MAPPUTCARD(regDupInvNum, dupROC+1000000, lblDBA);//@agmr - untuk perbandingan nanti


	// Save current data from tra/reg
	ret = logSaveCurContext();
	CHECK(ret >= 0, lblDBA);

	CHECK(ret >= 0, lblDBA);    // Log copy from DFS to RAM
	for (key = logBeg + 1; key < logEnd; key++) // Loop around each field from the record of the "log" table
	{
		idx = (byte) (key - logBeg - 1);    // Index key field
		VERIFY(isSorted(traBeg, logKey[idx], traEnd)    // Check that the field transaction is part of "tra" record
		       || isSorted(regBeg, logKey[idx], regEnd)
		       || isSorted(acqBeg, logKey[idx], acqEnd));   // Check that the field transaction is part of "reg" record
		ret = mapGet(key, buf, sizeof(buf));
		CHECK(ret >= 0, lblDBA);    // Retrieve the field transaction from "log" record
		ret = mapPut(logKey[idx], buf, sizeof(buf));
		CHECK(ret >= 0, lblDBA);    // Copy this field inside "tra" record
	}
//++ @agmr
{
//    byte trans;
    word mnu;

    MAPGETWORD(traMnuItm,mnu,lblDBA);
    switch(mnu)
    {
        case mnuSale:
            MAPPUTBYTE(traOriginalTxnType,trtSale,lblDBA);
            break;
	 	case mnuSaleRedeem:
            MAPPUTBYTE(traOriginalTxnType,trtSaleRedeem,lblDBA);
            break; //@ar - BRI
        case mnuInstallment:
            MAPPUTBYTE(traOriginalTxnType,trtInstallment,lblDBA);
            break;
        case mnuRefund:
            MAPPUTBYTE(traOriginalTxnType,trtRefund,lblDBA);
            break;
	 case mnuPreCom:
            MAPPUTBYTE(traOriginalTxnType,trtPreCom,lblDBA);
            break;
#ifdef PREPAID
        case mnuInfoDeposit:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidInfoDeposit,lblDBA);
            break;
        case mnuPrepaidPayment:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidPayment,lblDBA);
            break;
        case mnuTopUpDeposit:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidDeposit,lblDBA);
            break;
        case mnuTopUpOnline:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidTopUp,lblDBA);
            break;
        case mnuPrepaidRedeem:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidRedeem,lblDBA);
            break;
	//++@agmr - brizzi2
        case mnuPrepaidReaktivasi:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidReaktivasi,lblDBA);
            break;
        case mnuPrepaidVoid:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidVoid,lblDBA);
            break;
	//--@agmr - brizzi2
        case mnuAktivasiDeposit:
            MAPPUTBYTE(traOriginalTxnType,trtPrepaidAktivasi,lblDBA);
            break;
	 	case mnuPrepaidPaymentDisc:
	     	MAPPUTBYTE(traOriginalTxnType,trtPrepaidPaymentDisc,lblDBA);
	    	break;
#endif
    }


}
//-- @agmr
	ret = 1;
	goto lblEnd;

lblTraceNotFound:
	ret = 0;
	usrInfo(infTraceNotFound);
	goto lblEnd;
lblDBA:                      // Data base access error encountered
	trcErr(ret);                // Error tracing
	ret = -1;
	goto lblEnd;
lblEnd:
	//mapCache(-logBeg);      // Disable cache on log
	trcFN("logLoad: ret=%d \n", ret);
	return ret;
}


//@@SIMAS-CRDB - start
int checkDebitIssuer(byte bIssIndex)
{

    int ret,stat=0;
    byte issOption,IssId;
    byte IssuerID;

    bIssIndex++; //Actual Issuer Database Index is 1 lower than IssuerID

    for (IssId = 0; IssId < dimIss; IssId++)
   	{
   		ret = mapMove(issBeg, (word) IssId);
   		CHECK(ret >= 0, lblDBA);    // Move to the next record

   		MAPGETBYTE(issID, IssuerID, lblDBA);
   		if (IssuerID==(bIssIndex)) {
   			MAPGETBYTE(issOpt4, issOption, lblDBA);
			if (issOption & 0x08)
				stat=1;
			break;
		}
   	}
    goto lblEnd;
    lblDBA:
    	usrInfo(infDataAccessError);
    lblEnd:
    	return (stat);
    	;
}
//@@SIMAS-CRDB - end


/** Calculates total amount and total count depending on card name and current acquirer
 * \return 1 if succesful, <0 otherwise
 * \source log\\log.c
 */
static int logCalculateTotalsByCardTypeAndCurAcq(void)
{
	// totals variable for acquirer
	char 	totDbAmt[lenTotAmt + 1], totCrAmt[lenTotAmt + 1],
			totPrecomAmt[lenTotAmt + 1], totPrecomDbAmt[lenTotAmt + 1], totPrecomCrAmt[lenTotAmt + 1],
			totRefundAmt[lenTotAmt + 1], totRefundDbAmt[lenTotAmt + 1], totRefundCrAmt[lenTotAmt + 1],
			totVoidAmt[lenTotAmt + 1], totVoidDbAmt[lenTotAmt + 1], totVoidCrAmt[lenTotAmt + 1],
			totVoidPrecomAmt[lenTotAmt + 1], totVoidPrecomDbAmt[lenTotAmt + 1], totVoidPrecomCrAmt[lenTotAmt + 1],
			totVoidRefundAmt[lenTotAmt + 1], totVoidRefundDbAmt[lenTotAmt + 1], totVoidRefundCrAmt[lenTotAmt + 1],
			totManualAmt[lenTotAmt + 1], totVoidManualAmt[lenTotAmt + 1], totTipAmt[lenTotAmt + 1];


	word 	totDbCnt = 0, totCrCnt = 0,
			totPrecomCnt = 0, totPrecomDbCnt = 0, totPrecomCrCnt = 0,
			totRefundCnt = 0, totRefundDbCnt = 0, totRefundCrCnt = 0,
			totVoidCnt = 0, totVoidDbCnt = 0, totVoidCrCnt = 0,
			totVoidPrecomCnt = 0, totVoidPrecomDbCnt = 0, totVoidPrecomCrCnt = 0,
			totVoidRefundCnt = 0, totVoidRefundDbCnt = 0, totVoidRefundCrCnt = 0,
			totManualCnt = 0, totVoidManualCnt = 0, totTipCnt = 0  ;

	// totals variable for a specific bin
	char 	totAcqDbAmt[lenTotAmt + 1], totAcqCrAmt[lenTotAmt + 1],totAcqRefundAmt[lenTotAmt + 1],
			totAcqTipAmt[lenTotAmt + 1],totAcqVoidAmt[lenTotAmt + 1],totAcqVoidRefundAmt[lenTotAmt + 1],
			totAcqManualAmt[lenTotAmt + 1],totAcqVoidManualAmt[lenTotAmt + 1], totAcqPrecomAmt[lenTotAmt + 1],
			totAcqVoidPrecomAmt[lenTotAmt + 1];

	word 	totAcqDbCnt = 0, totAcqCrCnt = 0, totAcqTipCnt = 0, totAcqVoidCnt = 0,
			totAcqVoidRefundCnt = 0, totAcqManualCnt = 0, totAcqVoidManualCnt = 0, totAcqRefundCnt = 0,
			totAcqPrecomCnt = 0, totAcqVoidPrecomCnt = 0;

	// totals variable for a specific bin
	char 	totIssDbAmt[lenTotAmt + 1], totIssCrAmt[lenTotAmt + 1],totIssRefundDbAmt[lenTotAmt + 1],
			totIssTipDbAmt[lenTotAmt + 1],totIssVoidDbAmt[lenTotAmt + 1],totIssVoidRefundDbAmt[lenTotAmt + 1],
			totIssManualDbAmt[lenTotAmt + 1],totIssVoidManualDbAmt[lenTotAmt + 1], totIssPrecomDbAmt[lenTotAmt + 1],
			totIssVoidPrecomDbAmt[lenTotAmt + 1];

	word 	totIssDbCnt = 0, totIssCrCnt = 0, totIssTipDbCnt = 0, totIssVoidDbCnt = 0,
			totIssVoidRefundDbCnt = 0, totIssManualDbCnt = 0, totIssVoidManualDbCnt = 0, totIssRefundDbCnt = 0,
			totIssPrecomDbCnt = 0, totIssVoidPrecomDbCnt = 0;

	// totals variable for a specific bin
	char 	totIssRefundCrAmt[lenTotAmt + 1],
			totIssTipCrAmt[lenTotAmt + 1],totIssVoidCrAmt[lenTotAmt + 1],totIssVoidRefundCrAmt[lenTotAmt + 1],
			totIssManualCrAmt[lenTotAmt + 1],totIssVoidManualCrAmt[lenTotAmt + 1], totIssPrecomCrAmt[lenTotAmt + 1],
			totIssVoidPrecomCrAmt[lenTotAmt + 1];

	word 	totIssTipCrCnt = 0, totIssVoidCrCnt = 0,
			totIssVoidRefundCrCnt = 0, totIssManualCrCnt = 0, totIssVoidManualCrCnt = 0, totIssRefundCrCnt = 0,
			totIssPrecomCrCnt = 0, totIssVoidPrecomCrCnt = 0;

	// operation buffers
	char 	txnAmt[lenAmt + 1], tipAmt[lenAmt + 1];
	char 	zeroCmp[lenTotAmt + 1];
	byte 	txnType = ' ', acqCurIdx, acqTxnIdx, issCurIdx, issTxnIdx;
	word 	trxNum, curMnuItem, idx, binIdx, voidFlag = 0, transTypeOri, binCnt;
	word	count,  countChk1, countChk2, countChk3, countChk4, countChk5, countChk6, countChk7;
	int 		ret = 0, ctr = 0;
	char 	entMod;

	trcS("logCalculateTotalsByCardTypeAndCurAcq Beg\n");

	//Initialization of bin totals
	for (binIdx = 0; binIdx < dimBin; binIdx++)
	{
		//Place the bin pointer with the bin index of this record
		ret = mapMove(binBeg, binIdx);
		MAPPUTSTR(binTotAppDbAmt, "0", lblKO);
		MAPPUTSTR(binTotAppCrAmt, "0", lblKO);
		MAPPUTSTR(binTotAppPrecomAmt, "0", lblKO);
		MAPPUTSTR(binTotAppPrecomDbAmt, "0", lblKO);
		MAPPUTSTR(binTotAppPrecomCrAmt, "0", lblKO);
		MAPPUTSTR(binTotAppRefundAmt, "0", lblKO);
		MAPPUTSTR(binTotAppRefundDbAmt, "0", lblKO);
		MAPPUTSTR(binTotAppRefundCrAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidDbAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidCrAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidPrecomAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidPrecomDbAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidPrecomCrAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidRefundAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidRefundDbAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidRefundCrAmt, "0", lblKO);
		MAPPUTSTR(binTotAppManualAmt, "0", lblKO);
		MAPPUTSTR(binTotAppVoidManualAmt, "0", lblKO);
		MAPPUTSTR(binTotAppTpAmt, "0", lblKO);


		MAPPUTWORD(binTotAppDbCnt, 0, lblKO);
		MAPPUTWORD(binTotAppCrCnt, 0, lblKO);
		MAPPUTWORD(binTotAppPrecomCnt, 0, lblKO);
		MAPPUTWORD(binTotAppPrecomDbCnt, 0, lblKO);
		MAPPUTWORD(binTotAppPrecomCrCnt, 0, lblKO);
		MAPPUTWORD(binTotAppRefundCnt, 0, lblKO);
		MAPPUTWORD(binTotAppRefundDbCnt, 0, lblKO);
		MAPPUTWORD(binTotAppRefundCrCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidDbCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidCrCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidPrecomCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidPrecomDbCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidPrecomCrCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidRefundCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidRefundDbCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidRefundCrCnt, 0, lblKO);
		MAPPUTWORD(binTotAppManualCnt, 0, lblKO);
		MAPPUTWORD(binTotAppVoidManualCnt, 0, lblKO);
		MAPPUTWORD(binTotAppTpCnt, 0, lblKO);
	}

	//Initialization of acquirer totals
	MAPPUTSTR(acqTotAppDbAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppCrAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppTpAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppRefundAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppManualAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppVoidManualAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppVoidAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppVoidRefundAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppPrecomAmt, "0", lblKO);
	MAPPUTSTR(acqTotAppVoidPrecomAmt, "0", lblKO);

	MAPPUTWORD(acqTotAppDbCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppCrCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppVoidCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppVoidRefundCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppRefundCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppManualCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppVoidManualCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppPrecomCnt, 0, lblKO);
	MAPPUTWORD(acqTotAppVoidPrecomCnt, 0, lblKO);

	memset(totAcqDbAmt, 0, sizeof(totAcqDbAmt));
	memset(totAcqCrAmt, 0, sizeof(totAcqCrAmt));
	memset(totAcqTipAmt, 0, sizeof(totAcqTipAmt));
	memset(totAcqVoidAmt, 0, sizeof(totAcqVoidAmt));
	memset(totAcqVoidRefundAmt, 0, sizeof(totAcqVoidRefundAmt));
	memset(totAcqManualAmt, 0, sizeof(totAcqManualAmt));
	memset(totAcqVoidManualAmt, 0, sizeof(totAcqVoidManualAmt));
	memset(totAcqRefundAmt, 0, sizeof(totAcqRefundAmt));
	memset(totAcqPrecomAmt, 0, sizeof(totAcqPrecomAmt));
	memset(totAcqVoidPrecomAmt, 0, sizeof(totAcqVoidPrecomAmt));
	memset(zeroCmp, 0, sizeof(zeroCmp));



	//Initialization of issuers totals
	MAPPUTSTR(issTotAppDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppTpAmt, "0", lblKO);
	MAPPUTSTR(issTotAppRefundDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppManualDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidManualDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidRefundDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppPrecomDbAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidPrecomDbAmt, "0", lblKO);

	memset(totIssDbAmt, 0, sizeof(totIssDbAmt));
	memset(totIssCrAmt, 0, sizeof(totIssCrAmt));
	memset(totIssTipDbAmt, 0, sizeof(totIssTipDbAmt));
	memset(totIssVoidDbAmt, 0, sizeof(totIssVoidDbAmt));
	memset(totIssVoidRefundDbAmt, 0, sizeof(totIssVoidRefundDbAmt));
	memset(totIssManualDbAmt, 0, sizeof(totIssManualDbAmt));
	memset(totIssVoidManualDbAmt, 0, sizeof(totIssVoidManualDbAmt));
	memset(totIssRefundDbAmt, 0, sizeof(totIssRefundDbAmt));
	memset(totIssPrecomDbAmt, 0, sizeof(totIssPrecomDbAmt));
	memset(totIssVoidPrecomDbAmt, 0, sizeof(totIssVoidPrecomDbAmt));


	MAPPUTWORD(issTotAppDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppCrCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidRefundDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppRefundDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppManualDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidManualDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppPrecomDbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidPrecomDbCnt, 0, lblKO);

	//Initialization of issuers total
	MAPPUTSTR(issTotAppTpAmt, "0", lblKO);
	MAPPUTSTR(issTotAppRefundCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppManualCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidManualCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidRefundCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppPrecomCrAmt, "0", lblKO);
	MAPPUTSTR(issTotAppVoidPrecomCrAmt, "0", lblKO);

	memset(totIssVoidCrAmt, 0, sizeof(totIssVoidCrAmt));
	memset(totIssVoidRefundCrAmt, 0, sizeof(totIssVoidRefundCrAmt));
	memset(totIssManualCrAmt, 0, sizeof(totIssManualCrAmt));
	memset(totIssVoidManualCrAmt, 0, sizeof(totIssVoidManualCrAmt));
	memset(totIssRefundCrAmt, 0, sizeof(totIssRefundCrAmt));
	memset(totIssPrecomCrAmt, 0, sizeof(totIssPrecomCrAmt));
	memset(totIssVoidPrecomCrAmt, 0, sizeof(totIssVoidPrecomCrAmt));

	MAPPUTWORD(issTotAppVoidCrCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidRefundCrCnt, 0, lblKO);
	MAPPUTWORD(issTotAppRefundCrbCnt, 0, lblKO);
	MAPPUTWORD(issTotAppManualCrCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidManualCrCnt, 0, lblKO);
	MAPPUTWORD(issTotAppPrecomCrCnt, 0, lblKO);
	MAPPUTWORD(issTotAppVoidPrecomCrCnt, 0, lblKO);



	//Get current transaction number
	MAPGETWORD(regTrxNo, trxNum, lblKO);

	//Computation loop
	for (ctr = 0; ctr < trxNum; ctr++)
	{
		mapMove(logBeg, ctr);

		// Get amount and transaction type from LOG
		MAPGETBYTE(logAcqIdx, acqTxnIdx, lblKO);
		MAPGETBYTE(acqID, acqCurIdx, lblKO);

		// Get amount and transaction type from LOG
		MAPGETBYTE(logIssIdx, issTxnIdx, lblKO);
		MAPGETBYTE(issID, issCurIdx, lblKO);


		//Check if current acquirer is the same for this record
		if(acqTxnIdx != acqCurIdx - 1)
			continue;

		// Get data from this record
		MAPGETWORD(logMnuItm, curMnuItem, lblKO);
		MAPGET(logTraAmt, txnAmt, lblKO);
		MAPGET(logTipAmt, tipAmt, lblKO);
		MAPGETBYTE(logBinIdx, binIdx, lblKO);

		//Place the bin pointer with the bin index of this record
		ret = mapMove(binBeg, (word) binIdx);

		memset(totDbAmt, 0, sizeof(totDbAmt));
		memset(totCrAmt, 0, sizeof(totCrAmt));
		memset(totPrecomAmt, 0, sizeof(totPrecomAmt));
		memset(totPrecomDbAmt, 0, sizeof(totPrecomDbAmt));
		memset(totPrecomCrAmt, 0, sizeof(totPrecomCrAmt));
		memset(totRefundAmt, 0, sizeof(totRefundAmt));
		memset(totRefundDbAmt, 0, sizeof(totRefundDbAmt));
		memset(totRefundCrAmt, 0, sizeof(totRefundCrAmt));
		memset(totVoidAmt, 0, sizeof(totVoidAmt));
		memset(totVoidDbAmt, 0, sizeof(totVoidDbAmt));
		memset(totVoidCrAmt, 0, sizeof(totVoidCrAmt));
		memset(totVoidPrecomAmt, 0, sizeof(totVoidPrecomAmt));
		memset(totVoidPrecomDbAmt, 0, sizeof(totVoidPrecomDbAmt));
		memset(totVoidPrecomCrAmt, 0, sizeof(totVoidPrecomCrAmt));
		memset(totVoidRefundAmt, 0, sizeof(totVoidRefundAmt));
		memset(totVoidRefundDbAmt, 0, sizeof(totVoidRefundDbAmt));
		memset(totVoidRefundCrAmt, 0, sizeof(totVoidRefundCrAmt));
		memset(totManualAmt, 0, sizeof(totManualAmt));
		memset(totVoidManualAmt, 0, sizeof(totVoidManualAmt));
		memset(totTipAmt, 0, sizeof(totTipAmt));

		totDbCnt        			= 0;
		totCrCnt        			= 0;
		totPrecomCnt    		= 0;
		totPrecomDbCnt    		= 0;
		totPrecomCrCnt    		= 0;
		totRefundCnt    		= 0;
		totRefundDbCnt    		= 0;
		totRefundCrCnt    		= 0;
		totVoidCnt      		= 0;
		totVoidDbCnt      		= 0;
		totVoidCrCnt      		= 0;
		totVoidPrecomCnt  		= 0;
		totVoidPrecomDbCnt  	= 0;
		totVoidPrecomCrCnt  	= 0;
		totVoidRefundCnt      	= 0;
		totVoidRefundDbCnt      = 0;
		totVoidRefundCrCnt      	= 0;
		totManualCnt    		= 0;
		totVoidManualCnt  		= 0;
		totTipCnt       			= 0;

		// Get the previous totals from this bin
		MAPGET(binTotAppDbAmt, totDbAmt, lblKO);
		MAPGET(binTotAppCrAmt, totCrAmt, lblKO);
		MAPGET(binTotAppPrecomAmt, totPrecomAmt, lblKO);
		MAPGET(binTotAppPrecomDbAmt, totPrecomDbAmt, lblKO);
		MAPGET(binTotAppPrecomCrAmt, totPrecomCrAmt, lblKO);
		MAPGET(binTotAppRefundAmt, totRefundAmt, lblKO);
		MAPGET(binTotAppRefundDbAmt, totRefundAmt, lblKO);
		MAPGET(binTotAppRefundCrAmt, totRefundAmt, lblKO);
		MAPGET(binTotAppVoidAmt, totVoidAmt, lblKO);
		MAPGET(binTotAppVoidDbAmt, totVoidDbAmt, lblKO);
		MAPGET(binTotAppVoidCrAmt, totVoidCrAmt, lblKO);
		MAPGET(binTotAppVoidPrecomAmt, totVoidPrecomAmt, lblKO);
		MAPGET(binTotAppVoidPrecomDbAmt, totVoidPrecomDbAmt, lblKO);
		MAPGET(binTotAppVoidPrecomCrAmt, totVoidPrecomCrAmt, lblKO);
		MAPGET(binTotAppVoidRefundAmt, totVoidRefundAmt, lblKO);
		MAPGET(binTotAppVoidRefundDbAmt, totVoidRefundDbAmt, lblKO);
		MAPGET(binTotAppVoidRefundCrAmt, totVoidRefundCrAmt, lblKO);
		MAPGET(binTotAppManualAmt, totManualAmt, lblKO);
		MAPGET(binTotAppVoidManualAmt, totVoidManualAmt, lblKO);
		MAPGET(binTotAppTpAmt, totTipAmt, lblKO);

		MAPGETWORD(binTotAppDbCnt, totDbCnt, lblKO);
		MAPGETWORD(binTotAppCrCnt, totCrCnt, lblKO);
		MAPGETWORD(binTotAppPrecomCnt, totPrecomCnt, lblKO);
		MAPGETWORD(binTotAppPrecomDbCnt, totPrecomDbCnt, lblKO);
		MAPGETWORD(binTotAppPrecomCrCnt, totPrecomCrCnt, lblKO);
		MAPGETWORD(binTotAppRefundCnt, totRefundCnt, lblKO);
		MAPGETWORD(binTotAppRefundDbCnt, totRefundDbCnt, lblKO);
		MAPGETWORD(binTotAppRefundCrCnt, totRefundCrCnt, lblKO);
		MAPGETWORD(binTotAppVoidCnt, totVoidCnt, lblKO);
		MAPGETWORD(binTotAppVoidDbCnt, totVoidDbCnt, lblKO);
		MAPGETWORD(binTotAppVoidCrCnt, totVoidCrCnt, lblKO);
		MAPGETWORD(binTotAppVoidPrecomCnt, totVoidPrecomCnt, lblKO);
		MAPGETWORD(binTotAppVoidPrecomDbCnt, totVoidPrecomDbCnt, lblKO);
		MAPGETWORD(binTotAppVoidPrecomCrCnt, totVoidPrecomCrCnt, lblKO);
		MAPGETWORD(binTotAppVoidRefundCnt, totVoidRefundCnt, lblKO);
		MAPGETWORD(binTotAppVoidRefundDbCnt, totVoidRefundDbCnt, lblKO);
		MAPGETWORD(binTotAppVoidRefundCrCnt, totVoidRefundCrCnt, lblKO);
		MAPGETWORD(binTotAppManualCnt, totManualCnt, lblKO);
		MAPGETWORD(binTotAppVoidManualCnt, totVoidManualCnt, lblKO);
		MAPGETWORD(binTotAppTpCnt, totTipCnt, lblKO);
		MAPGETBYTE(logVoidFlag, voidFlag, lblKO);

		MAPGETBYTE(logEntMod, entMod, lblKO);
		if((entMod == 'k' || entMod == 'K')&&(voidFlag != 1))
		{
			addStr(totManualAmt,totManualAmt,txnAmt);
			totManualCnt++;
		}

		if(voidFlag == 1)
			curMnuItem = mnuVoid;

		//Determine index of equivalent menu item transaction x
		switch (curMnuItem)
		{
			case mnuRefund:
				idx = 3;        //@agmr - BRI
				addStr(totRefundAmt,totRefundAmt,txnAmt);
				totRefundCnt++;
				//if(issTxnIdx==1)
				if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
				{
					addStr(totIssRefundDbAmt, totIssRefundDbAmt, txnAmt);
					totIssRefundDbCnt++;
					addStr(totRefundDbAmt, totRefundDbAmt, txnAmt);
					totRefundDbCnt++;

				}
				else
				{
					addStr(totIssRefundCrAmt, totIssRefundCrAmt, txnAmt);
					totIssRefundCrCnt++;
					addStr(totRefundCrAmt, totRefundCrAmt, txnAmt);
					totRefundCrCnt++;
				}
				break;

			case mnuPreCom:
				idx = 3;        //@agmr - BRI
				addStr(totPrecomAmt,totPrecomAmt,txnAmt);
				addStr(totAcqPrecomAmt, totAcqPrecomAmt, txnAmt);
				totAcqPrecomCnt++;
				totPrecomCnt++;
				//if(issTxnIdx==1)
				if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
				{
					addStr(totIssPrecomDbAmt, totIssPrecomDbAmt, txnAmt);
					totIssPrecomDbCnt++;
					addStr(totPrecomDbAmt, totPrecomDbAmt, txnAmt);
					totPrecomDbCnt++;

				}
				else
				{
					addStr(totIssPrecomCrAmt, totIssPrecomCrAmt, txnAmt);
					totIssPrecomCrCnt++;
					addStr(totPrecomCrAmt, totPrecomCrAmt, txnAmt);
					totPrecomCrCnt++;
				}
				break;

			case mnuVoid:
				MAPGETWORD(logMnuItm, transTypeOri, lblKO);
				if(((transTypeOri == mnuSale)|| (transTypeOri == mnuSaleRedeem))&& ((entMod != 'k') && (entMod != 'K'))){
					idx = 7;        //@agmr - BRI
					addStr(totVoidAmt,totVoidAmt,txnAmt);
					totVoidCnt++;
					//if(issTxnIdx==1)
					if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
					{
						addStr(totIssVoidDbAmt, totIssVoidDbAmt, txnAmt);
						totIssVoidDbCnt++;
						addStr(totVoidDbAmt, totVoidDbAmt, txnAmt);
						totVoidDbCnt++;

					}
					else
					{
						addStr(totIssVoidCrAmt, totIssVoidCrAmt, txnAmt);
						totIssVoidCrCnt++;
						addStr(totVoidCrAmt, totVoidCrAmt, txnAmt);
						totVoidCrCnt++;
					}
				}
				else if(transTypeOri == mnuRefund){
					idx = 7;        //@agmr - BRI
					addStr(totVoidRefundAmt,totVoidRefundAmt,txnAmt);
					addStr(totAcqVoidRefundAmt, totAcqVoidRefundAmt, txnAmt);
					totVoidRefundCnt++;
					totAcqVoidRefundCnt++;
					//if(issCurIdx==1)
					if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
					{
						addStr(totIssVoidRefundCrAmt, totIssVoidRefundCrAmt, txnAmt);
						totIssVoidRefundCrCnt++;
						addStr(totVoidRefundCrAmt, totVoidRefundCrAmt, txnAmt);
						totVoidRefundCrCnt++;
					}
					else
					{
						addStr(totIssVoidRefundDbAmt, totIssVoidRefundDbAmt, txnAmt);
						totIssVoidRefundDbCnt++;
						addStr(totVoidRefundDbAmt, totVoidRefundDbAmt, txnAmt);
						totVoidRefundDbCnt++;
					}
				}
				else if(transTypeOri == mnuPreCom)
				{
					idx = 7;        //@agmr - BRI
					addStr(totVoidPrecomAmt,totVoidPrecomAmt,txnAmt);
					addStr(totAcqVoidPrecomAmt, totAcqVoidPrecomAmt, txnAmt);
					totVoidPrecomCnt++;
					totAcqVoidPrecomCnt++;

					//if(issTxnIdx==1)
					if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
					{
						addStr(totIssVoidPrecomDbAmt, totIssVoidPrecomDbAmt, txnAmt);
						totIssVoidPrecomDbCnt++;
						addStr(totVoidPrecomDbAmt, totVoidPrecomDbAmt, txnAmt);
						totVoidPrecomDbCnt++;
					}
					else
					{
						addStr(totIssVoidPrecomCrAmt, totIssVoidPrecomCrAmt, txnAmt);
						totIssVoidPrecomCrCnt++;
						addStr(totVoidPrecomCrAmt, totVoidPrecomCrAmt, txnAmt);
						totVoidPrecomCrCnt++;
					}
				}
				if((entMod == 'k' || entMod == 'K')&&(transTypeOri != mnuRefund)&&(transTypeOri != mnuSale)&&(transTypeOri != mnuSaleRedeem))
				{
					addStr(totVoidManualAmt,totVoidManualAmt,txnAmt);
					totVoidManualCnt++;
				}

				break;
#ifdef PREPAID
            case mnuInfoDeposit:
            case mnuPrepaidPayment:
            case mnuTopUpDeposit:
            case mnuTopUpOnline:
            case mnuPrepaidRedeem:
            case mnuAktivasiDeposit:
	     //++@agmr - brizzi2
            case mnuPrepaidReaktivasi:
            case mnuPrepaidVoid:
	     //--@agmr - brizzi2
	     case mnuPrepaidPaymentDisc:
#endif
            case mnuInstallment: //@agmr - BRI
			case mnuSale:
			case mnuSaleRedeem: //@ar - BRI
			default:
				idx = 0;
				break;
		}

		// Get the transaction type
		mapMove(rqsBeg, idx);
		MAPGETBYTE(rqsDbCr, txnType, lblKO);

		//Calculate acquirer and specific bin totals
		switch (txnType)
		{
			case 'D':
				if((curMnuItem != mnuPreCom) ||((curMnuItem != mnuVoid) && (transTypeOri == mnuPreCom)))
				{
					addStr(totDbAmt, totDbAmt, txnAmt);
					addStr(totAcqDbAmt, totAcqDbAmt, txnAmt);
					totDbCnt++;
					totAcqDbCnt++;
					/*
					if((issTxnIdx==1)&&(curMnuItem != mnuVoid))
					{
						addStr(totIssDbAmt, totIssDbAmt, txnAmt);
						totIssDbCnt++;
					}
					else if((issTxnIdx!=1)&&(curMnuItem != mnuVoid))
					{
						addStr(totIssCrAmt, totIssCrAmt, txnAmt);
						totIssCrCnt++;
					}
					*/
					//@@SIMAS-CRDB - start
					if (curMnuItem != mnuVoid) {
						if (checkDebitIssuer(issTxnIdx)) {
							addStr(totIssDbAmt, totIssDbAmt, txnAmt);
							totIssDbCnt++;
						}
						else {
							addStr(totIssCrAmt, totIssCrAmt, txnAmt);
							totIssCrCnt++;
						}
					}
					//@@SIMAS-CRDB - end
				}
				break;

			case 'C':
				if((curMnuItem != mnuPreCom) ||((curMnuItem != mnuVoid) && (transTypeOri == mnuPreCom)))
				{
					addStr(totCrAmt, totCrAmt, txnAmt);
					addStr(totAcqCrAmt, totAcqCrAmt, txnAmt);
					totCrCnt++;
					totAcqCrCnt++;
					/*
					if((issTxnIdx==1)&&(curMnuItem != mnuVoid))
					{
						addStr(totIssDbAmt, totIssDbAmt, txnAmt);
						totIssDbCnt++;
					}
					else if((issTxnIdx!=1)&&(curMnuItem != mnuVoid))
					{
						addStr(totIssCrAmt, totIssCrAmt, txnAmt);
						totIssCrCnt++;
					}
					*/
					//@@SIMAS-CRDB - start
					if (curMnuItem != mnuVoid) {
						if (checkDebitIssuer(issTxnIdx)) {
							addStr(totIssDbAmt, totIssDbAmt, txnAmt);
							totIssDbCnt++;
						}
						else {
							addStr(totIssCrAmt, totIssCrAmt, txnAmt);
							totIssCrCnt++;
						}
					}
					//@@SIMAS-CRDB - end
				}
				break;

			default:
				break;
		}

		//Calculate Tip
		if(memcmp(tipAmt, zeroCmp, sizeof(zeroCmp) != 0))
		{
			addStr(totTipAmt, totTipAmt, tipAmt);
			addStr(totAcqTipAmt, totAcqTipAmt, tipAmt);
			totTipCnt++;
			totAcqTipCnt++;
		}

		MAPGETBYTE(rqsDbCr, txnType, lblKO);

		//Place totals in respective bin
		MAPPUTSTR(binTotAppCrAmt, totCrAmt, lblKO);
		MAPPUTSTR(binTotAppDbAmt, totDbAmt, lblKO);
		MAPPUTSTR(binTotAppPrecomAmt, totPrecomAmt, lblKO);
		MAPPUTSTR(binTotAppPrecomDbAmt, totPrecomDbAmt, lblKO);
		MAPPUTSTR(binTotAppPrecomCrAmt, totPrecomCrAmt, lblKO);
		MAPPUTSTR(binTotAppRefundAmt, totRefundAmt, lblKO);
		MAPPUTSTR(binTotAppRefundDbAmt, totRefundDbAmt, lblKO);
		MAPPUTSTR(binTotAppRefundCrAmt, totRefundCrAmt, lblKO);
		MAPPUTSTR(binTotAppVoidAmt, totVoidAmt, lblKO);
		MAPPUTSTR(binTotAppVoidDbAmt, totVoidDbAmt, lblKO);
		MAPPUTSTR(binTotAppVoidCrAmt, totVoidCrAmt, lblKO);
		MAPPUTSTR(binTotAppVoidPrecomAmt, totVoidPrecomAmt, lblKO);
		MAPPUTSTR(binTotAppVoidPrecomDbAmt, totVoidPrecomDbAmt, lblKO);
		MAPPUTSTR(binTotAppVoidPrecomCrAmt, totVoidPrecomCrAmt, lblKO);
		MAPPUTSTR(binTotAppVoidRefundAmt, totVoidRefundAmt, lblKO);
		MAPPUTSTR(binTotAppVoidRefundDbAmt, totVoidRefundDbAmt, lblKO);
		MAPPUTSTR(binTotAppVoidRefundCrAmt, totVoidRefundCrAmt, lblKO);
		MAPPUTSTR(binTotAppManualAmt, totManualAmt, lblKO);
		MAPPUTSTR(binTotAppVoidManualAmt, totVoidManualAmt, lblKO);
		MAPPUTSTR(binTotAppTpAmt, totTipAmt, lblKO);

		MAPPUTWORD(binTotAppCrCnt, totCrCnt, lblKO);
		MAPPUTWORD(binTotAppDbCnt, totDbCnt, lblKO);
		MAPPUTWORD(binTotAppPrecomCnt, totPrecomCnt, lblKO);
		MAPPUTWORD(binTotAppPrecomDbCnt, totPrecomDbCnt, lblKO);
		MAPPUTWORD(binTotAppPrecomCrCnt, totPrecomCrCnt, lblKO);
		MAPPUTWORD(binTotAppRefundCnt, totRefundCnt, lblKO);
		MAPPUTWORD(binTotAppRefundDbCnt, totRefundDbCnt, lblKO);
		MAPPUTWORD(binTotAppRefundCrCnt, totRefundCrCnt, lblKO);
		MAPPUTWORD(binTotAppVoidCnt, totVoidCnt, lblKO);
		MAPPUTWORD(binTotAppVoidDbCnt, totVoidDbCnt, lblKO);
		MAPPUTWORD(binTotAppVoidCrCnt, totVoidCrCnt, lblKO);
		MAPPUTWORD(binTotAppVoidPrecomCnt, totVoidPrecomCnt, lblKO);
		MAPPUTWORD(binTotAppVoidPrecomDbCnt, totVoidPrecomDbCnt, lblKO);
		MAPPUTWORD(binTotAppVoidPrecomCrCnt, totVoidPrecomCrCnt, lblKO);
		MAPPUTWORD(binTotAppVoidRefundCnt, totVoidRefundCnt, lblKO);
		MAPPUTWORD(binTotAppVoidRefundDbCnt, totVoidRefundDbCnt, lblKO);
		MAPPUTWORD(binTotAppVoidRefundCrCnt, totVoidRefundCrCnt, lblKO);
		MAPPUTWORD(binTotAppManualCnt, totManualCnt, lblKO);
		MAPPUTWORD(binTotAppVoidManualCnt, totVoidManualCnt, lblKO);
		MAPPUTWORD(binTotAppTpCnt, totTipCnt, lblKO);

	}

	//Place totals in current acquirer
	MAPPUTSTR(acqTotAppCrAmt, totAcqCrAmt, lblKO);
	MAPPUTSTR(acqTotAppDbAmt, totAcqDbAmt, lblKO);
	MAPPUTSTR(acqTotAppTpAmt, totAcqTipAmt, lblKO);
	MAPPUTSTR(acqTotAppVoidAmt, totAcqVoidAmt, lblKO);
	MAPPUTSTR(acqTotAppVoidRefundAmt, totAcqVoidRefundAmt, lblKO);
	MAPPUTSTR(acqTotAppRefundAmt, totAcqRefundAmt, lblKO);
	MAPPUTSTR(acqTotAppManualAmt, totAcqManualAmt, lblKO);
	MAPPUTSTR(acqTotAppVoidManualAmt, totAcqVoidManualAmt, lblKO);
	MAPPUTSTR(acqTotAppPrecomAmt, totAcqPrecomAmt, lblKO);
	MAPPUTSTR(acqTotAppVoidPrecomAmt, totAcqVoidPrecomAmt, lblKO);

	MAPPUTWORD(acqTotAppCrCnt, totAcqCrCnt, lblKO);
	MAPPUTWORD(acqTotAppDbCnt, totAcqDbCnt, lblKO);
	MAPPUTWORD(acqTotAppTpCnt, totAcqTipCnt, lblKO);
	MAPPUTWORD(acqTotAppVoidCnt, totAcqVoidCnt, lblKO);
	MAPPUTWORD(acqTotAppVoidRefundCnt, totAcqVoidRefundCnt, lblKO);
	MAPPUTWORD(acqTotAppRefundCnt, totAcqRefundCnt, lblKO);
	MAPPUTWORD(acqTotAppManualCnt, totAcqManualCnt, lblKO);
	MAPPUTWORD(acqTotAppVoidManualCnt, totAcqVoidManualCnt, lblKO);
	MAPPUTWORD(acqTotAppPrecomCnt, totAcqPrecomCnt, lblKO);
	MAPPUTWORD(acqTotAppVoidPrecomCnt, totAcqVoidPrecomCnt, lblKO);


	//Place totals in current acquirer
	MAPPUTSTR(issTotAppCrAmt, totIssCrAmt, lblKO);
	MAPPUTSTR(issTotAppDbAmt, totIssDbAmt, lblKO);
	MAPPUTSTR(issTotAppTpAmt, totIssTipDbAmt, lblKO);
	MAPPUTSTR(issTotAppVoidDbAmt, totIssVoidDbAmt, lblKO);
	MAPPUTSTR(issTotAppVoidRefundDbAmt, totIssVoidRefundDbAmt, lblKO);
	MAPPUTSTR(issTotAppRefundDbAmt, totIssRefundDbAmt, lblKO);
	MAPPUTSTR(issTotAppManualDbAmt, totIssManualDbAmt, lblKO);
	MAPPUTSTR(issTotAppVoidManualDbAmt, totIssVoidManualDbAmt, lblKO);
	MAPPUTSTR(issTotAppPrecomDbAmt, totIssPrecomDbAmt, lblKO);
	MAPPUTSTR(issTotAppVoidPrecomDbAmt, totIssVoidPrecomDbAmt, lblKO);

	MAPPUTWORD(issTotAppCrCnt, totIssCrCnt, lblKO);
	MAPPUTWORD(issTotAppDbCnt, totIssDbCnt, lblKO);
	MAPPUTWORD(issTotAppTpCnt, totIssTipDbCnt, lblKO);
	MAPPUTWORD(issTotAppVoidDbCnt, totIssVoidDbCnt, lblKO);
	MAPPUTWORD(issTotAppVoidRefundDbCnt, totIssVoidRefundDbCnt, lblKO);
	MAPPUTWORD(issTotAppRefundDbCnt, totIssRefundDbCnt, lblKO);
	MAPPUTWORD(issTotAppManualDbCnt, totIssManualDbCnt, lblKO);
	MAPPUTWORD(issTotAppVoidManualDbCnt, totIssVoidManualDbCnt, lblKO);
	MAPPUTWORD(issTotAppPrecomDbCnt, totIssPrecomDbCnt, lblKO);
	MAPPUTWORD(issTotAppVoidPrecomDbCnt, totIssVoidPrecomDbCnt, lblKO);


	MAPPUTSTR(issTotAppVoidCrAmt, totIssVoidCrAmt, lblKO);
	MAPPUTSTR(issTotAppVoidRefundCrAmt, totIssVoidRefundCrAmt, lblKO);
	MAPPUTSTR(issTotAppRefundCrAmt, totIssRefundCrAmt, lblKO);
	MAPPUTSTR(issTotAppManualCrAmt, totIssManualCrAmt, lblKO);
	MAPPUTSTR(issTotAppVoidManualCrAmt, totIssVoidManualCrAmt, lblKO);
	MAPPUTSTR(issTotAppPrecomCrAmt, totIssPrecomCrAmt, lblKO);
	MAPPUTSTR(issTotAppVoidPrecomCrAmt, totIssVoidPrecomCrAmt, lblKO);

	MAPPUTWORD(issTotAppVoidCrCnt, totIssVoidCrCnt, lblKO);
	MAPPUTWORD(issTotAppVoidRefundCrCnt, totIssVoidRefundCrCnt, lblKO);
	MAPPUTWORD(issTotAppRefundCrbCnt, totIssRefundCrCnt, lblKO);
	MAPPUTWORD(issTotAppManualCrCnt, totIssManualCrCnt, lblKO);
	MAPPUTWORD(issTotAppVoidManualCrCnt, totIssVoidManualCrCnt, lblKO);
	MAPPUTWORD(issTotAppPrecomCrCnt, totIssPrecomCrCnt, lblKO);
	MAPPUTWORD(issTotAppVoidPrecomCrCnt, totIssVoidPrecomCrCnt, lblKO);

	//Print Total Card Summary
	//MAPGETBYTE(BinCnt, binCnt, lblKO);
	for (idx = 0; idx < dimBin; idx++)
	{
		ret = mapMove(binBeg, idx);
		MAPGETWORD(binTotAppDbCnt, count, lblKO);
		MAPGETWORD(binTotAppCrCnt, countChk1, lblKO);
		MAPGETWORD(binTotAppPrecomCnt, countChk2, lblKO);
		MAPGETWORD(binTotAppRefundCnt, countChk3, lblKO);
		MAPGETWORD(binTotAppVoidCnt, countChk4, lblKO);
		MAPGETWORD(binTotAppVoidPrecomCnt, countChk5, lblKO);
		MAPGETWORD(binTotAppVoidRefundCnt, countChk6, lblKO);
		if(count != 0 || countChk1 != 0 || countChk2 != 0 || countChk3 != 0 ||
			countChk4 != 0 || countChk5 != 0 || countChk6 != 0 )
			break;
	}

	ret = 0;
	goto lblEnd;

lblKO:
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("logCalculateTotalsByCardTypeAndCurAcq: ret=%d \n", ret);
	return ret;

}

/** Printing of txn receipt
 * \header log\\log.h
 * \source log\\log.c
 */
//void logPrintTxn(void) //BRI7
//{
//	int ret, logCount;
//	char rocRef[lenInvNum + 1];
//	char CrdType[lenCardTransType + 1];
//	char traName[prtW + 1];
//	char entMod;
//	word txnType, trxNum, key, idx = 0;
////	card traAmt, tipAmt;
//	byte voidTxn;
//
////++ @agmr - BRI
////    byte singleReceipt=0;
////    char str[128];
////    char buf[128],buf1[128];
////    char amount[20];
//    word transType;
//    byte singleReceipt;
//    char amtS[lenAmt + 1],amtS1[lenAmt + 1]; //@agmr - BRI
////    transType = key;
////    card Amt;
////-- @agmr - BRI
//
//
//	trcS("logPrintTxn Beg\n");
//	memset(rocRef, 0x00, sizeof(rocRef));
//
//	MAPGETWORD(regTrxNo, trxNum, lblKO);
//	logCount = trxNum;
//	CHECK(logCount > 0, lblNoTran);
//
//	ret = scrDlg(infEnterTrace, traFindRoc);
//	CHECK(ret >= 0, lblKO);
//
//	ret = logLoad();
//	CHECK(ret >= 0, lblKO);
//
//	if(ret == 0)
//		goto lblEnd;
//
//	ret = mapGetByte(logEntMod, entMod);    //Get Entry Mode
//	CHECK(ret >= 0, lblKO);
//
//	MAPGET(logCardType, CrdType, lblKO);
//	MAPPUTSTR(traCardType, CrdType, lblKO);
//
//	if(entMod == 'k')
//		key = rloManualSaleDup;
//	else
//	{
//		MAPGETWORD(logMnuItm, txnType, lblKO);  //Get Transaction Type
//
//		key = 0;
//		getBriTransTypeName(&transType, NULL, txnType);
//		ret = briPrintData(transType, &key, &singleReceipt,1);
//		if(ret < 0)
//		    goto lblKO;
//		else if(ret > 0)
//		    idx = transType;
//		else
//		{
//
//    		switch (txnType)
//    		{
//    			case mnuSale:
//    				key = rloTrxDup;
//    				idx = trtSale;
//    				break;
//    			case mnuPreaut:
//    				key = rloPreAuthDup;
//    				idx = trtPreAut;
//    				break;
//    //++ @agmr - BRI5
//    			case mnuAut:
//    		    {
//
//                	char str[128];
//                    char buf[128];
//                    char amount[25];
//
//    		        memset(str,0,sizeof(str));
//    		        memset(buf,0,sizeof(buf));
//    		        memset(amount,0,sizeof(amount));
//    		        MAPPUTSTR(rptBuf1,"",lblKO);
//        		    mapGet(traTotAmt,amount,sizeof(amount));
//                    strcpy(buf,"Rp. ");
//                    strcat(buf,amount);
////                    fmtAmt(buf+4, amount, 0, ",.");
//                    sprintf(str,"TOTAL: %17s",buf);
//        		    MAPPUTSTR(rptBuf1,str,lblKO);
//    		    }
//    				key = rloPreAuthDup;
//    				idx = trtAut;
//    				break;
//    //-- @agmr - BRI5
//
//    			case mnuRefund:
//    				key = rloTrxDup;//rloRefundDup;
//    				idx = trtRefund;
//    				break;
//    				//      case mnuCash:
//    				//          key = rloTrx;
//    				//          idx = trtCash;
//    				//          break;
//    				//      case mnuVerify:
//    				//         key = rloCrdVrfy;
//    				//         idx = trtCrdVrfy;
//    				//         break;
//    			case mnuOffline:
//    				key = rloOfflineSaleDup;
//    				idx = trtOffline;
//    				break;
//    			default:
//    				break;
//    		}
//    	}
//    }
//	VERIFY(key);
//
//	MAPGETBYTE(logVoidFlag, voidTxn, lblKO);
//
//	ret = usrInfo(infDuplicatePrinting);
//	CHECK(ret >= 0, lblKO);
//
//	if(voidTxn)
//		key = rloVoidDup;
//
//	mapMove(rqsBeg, idx - 1);
//
//	MAPGET(rqsRpt, traName, lblKO);
//	MAPPUTSTR(traTransType, traName, lblKO);
//
//	ret = strFormatDatTim(logDatTim);
//	CHECK(ret >= 0, lblKO);
//
////	MAPGET(logTraAmt, amtS, lblKO);
////	ret = dec2num(&traAmt, amtS, lenAmt);
////	MAPGET(logTipAmt, amtS, lblKO);
////	ret = dec2num(&tipAmt, amtS, lenAmt);
////	ret = num2dec(amtS, (traAmt + tipAmt), lenAmt);
////	MAPPUTSTR(traTotAmt, amtS, lblKO);
//    memset(amtS,0,sizeof(amtS));
//    memset(amtS1,0,sizeof(amtS1));
//	MAPGET(logTraAmt, amtS, lblKO);
//	MAPGET(logTipAmt, amtS1, lblKO);
//    addStr(amtS,amtS,amtS1); //@agmr - BRI
//	MAPPUTSTR(traTotAmt, amtS, lblKO);
//
//    //++ @agmr
//    ret = fmtAmtReceipt2(traTotAmt, amtS);
//	CHECK(ret > 0, lblKO);
//	//-- @agmr
//
//	trcS("Printing Receipt\n");
//	ret = rptReceipt(key);      // Print transaction receipt via Trace Number
////	if(emvFullProcessCheck() == 1)
//	{
//    	int ret;
//    	char IccData[lenEmvICC + 1];
//    	char TrnCrt[lenIssTrnCrt + 1];
//
//        //TC
//	    ret = mapGet(traEmvICC, IccData, lenEmvICC);
//	    if(memcmp(IccData,"\x00\x00\x00\x00\x00\x00\x00\x00",8) != 0)
//	    {
//	        TrnCrt[0] = 8;
//	        memcpy(TrnCrt+1,IccData+7,8);
//	        ret = mapPut(emvIssTrnCrt,TrnCrt,lenIssTrnCrt+1);
//            CHECK(ret >= 0, lblKO);
//
//	        if(txnType != mnuInstallment)
//	            rptReceipt(rloSpace);
//
//		    ret = rptReceipt(rloEmvTags);
//		    CHECK(ret >= 0, lblKO);
//		}
//	}
////	if(txnType == mnuInstallment)
////	{
//	    rptReceipt(rloDuplicateLine);
//	    rptReceipt(rloFeed); //BRI9
////	}
//
//	ret = logLoadCurContext();
//	CHECK(ret >= 0, lblKO);
//
//	goto lblEnd;
//
//lblNoTran:
//	usrInfo(infNoTran);
//	goto lblEnd;
//lblKO:
//	usrInfo(infProcessingError);
//	goto lblEnd;
//lblEnd:
//	trcS("logPrintTxn End\n");
//}
void logPrintTxn(byte jenisLog) //BRI7
{
	int ret, logCount;
	char rocRef[lenInvNum + 1];
	char CrdType[lenCardTransType + 1];
//	char traName[prtW + 1];
	char entMod;
	word txnType, trxNum, key;
//	card traAmt, tipAmt;
//	byte voidTxn;

//++ @agmr - BRI
//    byte singleReceipt=0;
//    char str[128];
//    char buf[128],buf1[128];
//    char amount[20];
    word transType;
    byte singleReceipt;
//    char amtS[lenAmt + 1],amtS1[lenAmt + 1]; //@agmr - BRI
//    transType = key;
//    card Amt;
//-- @agmr - BRI


	trcS("logPrintTxn Beg\n");
	memset(rocRef, 0x00, sizeof(rocRef));

	MAPGETWORD(regTrxNo, trxNum, lblKO);
	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	ret = scrDlg(infEnterTrace, traFindRoc);
	CHECK(ret >= 0, lblKO);

	ret = logLoad();
	CHECK(ret >= 0, lblKO);

#ifdef PREPAID
    if(jenisLog == BRIZZI_LOG)
    {
        if(isPrepaidTransaction(0)==0)
        {
            goto lblNoTran;
        }
    }
#endif

	if(ret == 0)
		goto lblEnd;

	ret = mapGetByte(logEntMod, entMod);    //Get Entry Mode
	CHECK(ret >= 0, lblKO);

	MAPGET(logCardType, CrdType, lblKO);
	MAPPUTSTR(traCardType, CrdType, lblKO);
    MAPGETWORD(logMnuItm, txnType, lblKO);  //Get Transaction Type
    key = 0;

	getBriTransTypeName(&transType, NULL, txnType);
	ret = briPrintData(transType, &key, &singleReceipt,1);
    if(ret < 0)
        goto lblKO;
    if(ret == 0)
    {
        ret = 1;
        goto lblEnd;
    }

	ret = usrInfo(infDuplicatePrinting);
	CHECK(ret >= 0, lblKO);

	trcS("Printing Receipt\n");
	MAPPUTSTR(rptDuplicate,"*DUPLICATE*",lblKO);
	ret = rptReceipt(key);      // Print Latest transaction receipt (Duplicate)

	rptReceipt(rloDuplicateLine); //BRI9
	rptReceipt(rloFeed); //BRI9

	ret = logLoadCurContext();
	CHECK(ret >= 0, lblKO);
	MAPPUTSTR(rptDuplicate,"",lblKO);

	goto lblEnd;

lblNoTran:
	usrInfo(infNoTran);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("logPrintTxn End\n");
}


/** Printing of duplicate receipt
 * \header log\\log.h
 * \source log\\log.c
 */
//void logDuplicata(void)
//{
//	int ret, logCount;
//	char rocRef[lenInvNum + 1];
//	char CrdType[lenCardTransType + 1];
//	char traName[prtW + 1];
//	char entMod;
//	word txnType, ptrTrx, trxNum, key, idx = 0;
//	card logRoc;
//	char amtS[lenAmt + 1],amtS1[lenAmt + 1]; //@agmr - BRI
////	card traAmt, tipAmt;
//	byte voidTxn;
//
////++ @agmr - BRI
//    word transType;
//    byte singleReceipt;
////    transType = key;
////-- @agmr - BRI
//
//	trcS("logDuplicata Beg\n");
//	memset(rocRef, 0x00, sizeof(rocRef));
//
//	MAPGETWORD(regTrxNo, trxNum, lblKO);
//	logCount = trxNum;
//	CHECK(logCount > 0, lblNoTran);
//
//	MAPGETWORD(regCurrIdxPtr, ptrTrx, lblKO);
//
//	if(ptrTrx > 0)
//	{
//		logCount = ptrTrx;
//		trxNum = ptrTrx;
//	}
//
//	mapMove(logBeg, trxNum - 1);
//
//	MAPGETCARD(logROC, logRoc, lblKO);
//	num2dec(rocRef, logRoc, 0);
//
//	MAPPUTSTR(traFindRoc, rocRef, lblKO);
//
//	ret = logLoad();
//	CHECK(ret >= 0, lblKO);
//
//	if(ret == 0)
//		goto lblEnd;
//
//	ret = mapGetByte(logEntMod, entMod);    //Get Entry Mode
//	CHECK(ret >= 0, lblKO);
//
//	MAPGET(logCardType, CrdType, lblKO);
//	MAPPUTSTR(traCardType, CrdType, lblKO);
//
//    	if(entMod == 'k')
//    		key = rloManualSaleDup;
//    	else
//    	{
//    		MAPGETWORD(logMnuItm, txnType, lblKO);  //Get Transaction Type
//            key = 0;
//
//    		getBriTransTypeName(&transType, NULL, txnType);
//    		ret = briPrintData(transType, &key, &singleReceipt);
//    		if(ret < 0)
//    		    goto lblKO;
//    		else if(ret > 0)
//    		    idx = transType;
//    		else
//    		{
//        		switch (txnType)
//        		{
//
//
//
//        			case mnuSale:
//        				key = rloTrxDup;
//        				idx = trtSale;
//        				break;
//        			case mnuPreaut:
//        				key = rloPreAuthDup;
//        				idx = trtPreAut;
//        				break;
//        //++ @agmr - BRI5
//        			case mnuAut:
//            		    {
//
//                        	char str[128];
//                            char buf[128];
//                            char amount[25];
//
//            		        memset(str,0,sizeof(str));
//            		        memset(buf,0,sizeof(buf));
//            		        memset(amount,0,sizeof(amount));
//            		        MAPPUTSTR(rptBuf1,"",lblKO);
//                		    mapGet(traTotAmt,amount,sizeof(amount));
//                            strcpy(buf,"Rp. ");
//                            strcat(buf,amount);
//        //                    fmtAmt(buf+4, amount, 0, ",.");
//                            sprintf(str,"TOTAL: %17s",buf);
//                		    MAPPUTSTR(rptBuf1,str,lblKO);
//            		    }
//        				key = rloPreAuthDup;
//        				idx = trtAut;
//        				break;
//        //-- @agmr - BRI5
//        			case mnuRefund:
//        				key = rloTrxDup; //rloRefundDup;
//        				idx = trtRefund;
//        				break;
//
//        			case mnuOffline:
//        				key = rloOfflineSaleDup;
//        				idx = trtOffline;
//        				break;
//        			default:
//        				break;
//        		}
//        	}
//    	}
//
//	VERIFY(key);
//
//	MAPGETBYTE(logVoidFlag, voidTxn, lblKO);
//
//	ret = usrInfo(infDuplicatePrinting);
//	CHECK(ret >= 0, lblKO);
//
//	if(voidTxn)
//		key = rloVoidDup;
//
//	mapMove(rqsBeg, idx - 1);
//
//	MAPGET(rqsRpt, traName, lblKO);
//	MAPPUTSTR(traTransType, traName, lblKO);
//
//	ret = strFormatDatTim(logDatTim);
//	CHECK(ret >= 0, lblKO);
//
//    memset(amtS,0,sizeof(amtS));
//    memset(amtS1,0,sizeof(amtS1));
//	MAPGET(logTraAmt, amtS, lblKO);
//	MAPGET(logTipAmt, amtS1, lblKO);
//    addStr(amtS,amtS,amtS1); //@agmr - BRI
//	MAPPUTSTR(traTotAmt, amtS, lblKO);
//
//    //++ @agmr
//    ret = fmtAmtReceipt2(traTotAmt, amtS);
//	CHECK(ret > 0, lblKO);
//	//-- @agmr
//
//	trcS("Printing Receipt\n");
//	ret = rptReceipt(key);      // Print Latest transaction receipt (Duplicate)
//
////	if(emvFullProcessCheck() == 1)
//	{
//    	int ret;
//    	char IccData[lenEmvICC + 1];
//    	char TrnCrt[lenIssTrnCrt + 1];
//
//        //TC
//	    ret = mapGet(traEmvICC, IccData, lenEmvICC);
//	    if(memcmp(IccData,"\x00\x00\x00\x00\x00\x00\x00\x00",8) != 0)
//	    {
//	        TrnCrt[0] = 8;
//	        memcpy(TrnCrt+1,IccData+7,8);
//	        ret = mapPut(emvIssTrnCrt,TrnCrt,lenIssTrnCrt+1);
//            CHECK(ret >= 0, lblKO);
//
//	        if(txnType != mnuInstallment)
//	            rptReceipt(rloSpace);
//
//		    ret = rptReceipt(rloEmvTags);
//		    CHECK(ret >= 0, lblKO);
//		}
//	}
//
////    if(txnType == mnuInstallment)
////    {
//	    rptReceipt(rloDuplicateLine); //BRI9
//	    rptReceipt(rloFeed); //BRI9
////	}
//
//	ret = logLoadCurContext();
//	CHECK(ret >= 0, lblKO);
//
//	goto lblEnd;
//
//lblNoTran:
//	usrInfo(infNoTran);
//	goto lblEnd;
//lblKO:
//	usrInfo(infProcessingError);
//	goto lblEnd;
//lblEnd:
//	trcS("logDuplicata End\n");
//}

void logDuplicata(byte jenisLog)
{
	int ret, logCount;
	char rocRef[lenInvNum + 1];
	char CrdType[lenCardTransType + 1];
	char entMod;
	word txnType, ptrTrx, trxNum, key;
	card logRoc;
    word transType;
    byte singleReceipt;

	trcS("logDuplicata Beg\n");
	memset(rocRef, 0x00, sizeof(rocRef));

//++ @agmr - print last trans
//	MAPGETWORD(regTrxNo, trxNum, lblKO);
    if(jenisLog == BRIZZI_LOG)
    {
        MAPGETWORD(appPrepaidPrintLastTransIdx, trxNum, lblKO);
    }
    else
    {
	    MAPGETWORD(appPrintLastTransIdx, trxNum, lblKO);
	}

	trxNum++;
//-- @agmr - print last trans

	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	MAPGETWORD(regCurrIdxPtr, ptrTrx, lblKO);

	if(ptrTrx > 0)
	{
		logCount = ptrTrx;
//		trxNum = ptrTrx; //@agmr - tutup
	}

	mapMove(logBeg, trxNum - 1);

	MAPGETCARD(logROC, logRoc, lblKO);
	num2dec(rocRef, logRoc, 0);

	MAPPUTSTR(traFindRoc, rocRef, lblKO);

	ret = logLoad();
	CHECK(ret >= 0, lblKO);

	if(ret == 0)
		goto lblEnd;

	ret = mapGetByte(logEntMod, entMod);    //Get Entry Mode
	CHECK(ret >= 0, lblKO);

	MAPGET(logCardType, CrdType, lblKO);
	MAPPUTSTR(traCardType, CrdType, lblKO);
    MAPGETWORD(logMnuItm, txnType, lblKO);  //Get Transaction Type
    key = 0;

	getBriTransTypeName(&transType, NULL, txnType);
	ret = briPrintData(transType, &key, &singleReceipt,1);
    if(ret < 0)
        goto lblKO;
    if(ret == 0)
    {
        ret = 1;
        goto lblEnd;
    }

	ret = usrInfo(infDuplicatePrinting);
	CHECK(ret >= 0, lblKO);

	trcS("Printing Receipt\n");
	MAPPUTSTR(rptDuplicate,"*DUPLICATE*",lblKO);
	ret = rptReceipt(key);      // Print Latest transaction receipt (Duplicate)

	rptReceipt(rloDuplicateLine); //BRI9
	rptReceipt(rloFeed); //BRI9

	ret = logLoadCurContext();
	CHECK(ret >= 0, lblKO);
	MAPPUTSTR(rptDuplicate,"",lblKO);

	goto lblEnd;

lblNoTran:
	usrInfo(infNoTran);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("logDuplicata End\n");
}

/** Printing of duplicate settlement receipt
 * \header log\\log.h
 * \source log\\log.c
 */
void logSetDup(void)
{

	trcS("logSetDup Beg\n");

//	setDuplicate = 1;
//	logSettlementRpt();
    printAllSetlleDuplicate();

	goto lblEnd;
lblEnd:
	trcS("logSetDup End\n");
}

/** This function deletes all transactions in the batch file.
 * \header log\\log.h
 * \source log\\log.c
 */

void logReset(void)
{
	int ret;

	ret = mapReset(logBeg);
	CHECK(ret >= 0, lblKO)
	MAPPUTWORD(regTrxNo, 0, lblKO);
	MAPPUTWORD(appPrintLastTransIdx, 0, lblKO); //@agmr
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	;
}

//++ @agmr - multi settle
int briLogReset(byte acqId)
{
	int ret;

//    MAPPUTBYTE(appNeedToBatchCopy,2,lblKO);
	ret = copyAllToTemporaryBatch();
	if(ret < 0)
	    goto lblKO;

    ret = markSettlementTrans(acqId);
    if(ret < 0)
        goto lblKO;

    MAPPUTBYTE(appNeedToBatchCopy,1,lblKO);
	logReset();

    ret = copyUnmarkToBatch();
    if(ret < 0)
       goto lblKO;

	MAPPUTBYTE(appNeedToBatchCopy,0,lblKO);

	CHECK(ret >= 0, lblKO)
//	MAPPUTWORD(regTrxNo, 0, lblKO);
	ret = 1;
	goto lblEnd;
lblKO:
//	usrInfo(infProcessingError);
	ret = -100;
lblEnd:
	return ret;
}

int markSettlementTrans(byte acqId)
{
	word trxNo,trxNoTemp=0;
	word idx;
	int ret;
	byte temp;

    MAPGETWORD(regTrxNo,trxNo,lblKO);

    for(idx = 0; idx<trxNo; idx++)
    {
        mapMove(logTempBeg, idx);
        MAPGETBYTE(logAcqIdxTemp, temp, lblKO);
        if(temp == acqId-1)
        {
            MAPPUTBYTE(logIsUnusedRecTemp,1,lblKO);
        }
        else
            trxNoTemp++;
    }
    MAPPUTWORD(regUnmarkTrxNo,trxNoTemp,lblKO);
    ret = 1;
    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
    return ret;
}

int copyUnmarkToBatch()
{
	word key, trxNo=0;
	word recIdxTemp;
	byte buf[256];
	int ret;
	word keyTemp, trxTempNo;
//	word recIdx = 0;
	byte temp/*, idx*/;

    MAPGETWORD(regTrxTempNo,trxTempNo,lblKO);
//    MAPGETWORD(regTrxNo,trxTempNo,lblKO);

    for(recIdxTemp = 0; recIdxTemp<trxTempNo; recIdxTemp++)
    {
        mapMove(logTempBeg, recIdxTemp);
        MAPGETBYTE(logIsUnusedRecTemp, temp, lblKO);
        if(temp == 1)
            continue;
        mapMove(logBeg, trxNo);
    	for (key = logBeg + 1,keyTemp = logTempBeg+1; key < logEnd; key++,keyTemp++)
    	{   // Loop around each field from the record of the "log" table
    		MAPGET(keyTemp, buf, lblKO);
    		ret = mapPut(key, buf, mapDatLen(key));
    		CHECK(ret >= 0, lblKO);
    	}
    	trxNo++;
    }
    MAPPUTWORD(regTrxNo,trxNo,lblKO);
    MAPPUTWORD(regCurrIdxPtr, trxNo, lblKO);

    ret = 1;
    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
    return ret;
}

int copyAllToBatch()
{
	word key, trxNo=0;
	word recIdxTemp;
	byte buf[256];
	int ret;
	word keyTemp, trxTempNo;
//	byte /*temp,*/ idx;

//    MAPGETWORD(regTrxNo,trxNo,lblKO);
//    if(trxNo != 0)
//    {
//        ret = 1;
//        goto lblEnd;
//    }

    MAPGETWORD(regTrxTempNo,trxTempNo,lblKO);

    for(recIdxTemp = 0; recIdxTemp<trxTempNo; recIdxTemp++)
    {
    	mapMove(logBeg, recIdxTemp);
    	mapMove(logTempBeg, recIdxTemp);

    	for (key = logBeg + 1,keyTemp = logTempBeg+1; key < logEnd; key++,keyTemp++)
    	{   // Loop around each field from the record of the "log" table
    		MAPGET(keyTemp, buf, lblKO);
    		if(keyTemp == logIsUnusedRecTemp)
    		{
    		    buf[0] = 0;
    		}
    		ret = mapPut(key, buf, mapDatLen(key));
    		CHECK(ret >= 0, lblKO);
    	}
    	trxNo++;
    }
    MAPPUTWORD(regTrxNo,trxNo,lblKO);
    MAPPUTWORD(regCurrIdxPtr, trxNo, lblKO);

    ret = 1;
    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
    return ret;
}

int copyAllToTemporaryBatch()
{
	word key, trxNo=0;
//	word recIdxTemp=0;
	byte buf[256];
	int ret;
	word keyTemp;/* trxTempNo=0;*/
	word recIdx = 0;
//	byte /*temp,*/ idx;

    ret = mapReset(logTempBeg);
    CHECK(ret >= 0, lblKO);

    MAPGETWORD(regTrxNo,trxNo,lblKO);

    for(recIdx = 0; recIdx<trxNo; recIdx++)
    {
        mapMove(logBeg, recIdx);
        mapMove(logTempBeg, recIdx);

    	for (key = logBeg + 1,keyTemp = logTempBeg+1; key < logEnd; key++,keyTemp++)
    	{   // Loop around each field from the record of the "log" table
    		MAPGET(key, buf, lblKO);
    		ret = mapPut(keyTemp, buf, mapDatLen(keyTemp));
    		CHECK(ret >= 0, lblKO);
    	}
//    	trxTempNo++;
    }
    MAPPUTWORD(regTrxTempNo,trxNo,lblKO);

    ret = 1;
    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
    return ret;
}

int batchSync()
{
    int ret;
    byte temp;

    MAPGETBYTE(appNeedToBatchCopy,temp,lblKO);
    if(temp == 1)
    {
        ret = copyAllToBatch();
        if(ret < 0)
            goto lblKO;
        MAPPUTBYTE(appNeedToBatchCopy,0,lblKO);
    }
    ret = 1;
    goto lblEnd;

lblKO:
    usrInfo(infBatchError);
    ret = -100;

lblEnd:
    return ret;

}
//-- @agmr - multi settle


/** This function checks whether the PAN on the card is listed on the exception file.
 * \header log\\log.h
 * \source log\\log.c
 */
int logIsInBlackList(void)
{
	// Process not implemented yet.
	return 0;
}

/** This function checks to see if there is a log entry with the same PAN, and
 *  optionally, the same PAN Sequence number. If there are multiple entries
 *  with he same PAN, the terminal selects the most revent entry and returns
 *  the correspondant amount by parameter.
 * \header log\\log.h
 * \source log\\log.c
 */
int logGetPreviousAmount(card * pulPrv)
{
	// Process not implemented yet.
	VERIFY(pulPrv);
	*pulPrv = 0;

	return 0;
}

/** Calculate all application totals in tot record based on the control character rqsDbCr:
 * - 'D': increment totAppDbAmt and totAppDbCnt
 * - 'C': increment totAppCrAmt and totAppCrCnt
 * - 'd': decrement totAppDbAmt and totAppDbCnt
 * - 'c': decrement totAppCrAmt and totAppCrCnt
 * - other: no impact on totals
 *
 * It is done in the following way:
 * - reset totals for all acquirers
 * - set internal total variables to zero
 * - For each record in log multirecord do:
 *   - move cursor in rqs multirecrod according to transaction type
 *   - get rqsDbCr
 *   - according to the value of rqsDbCr (D,d,C,c) modify total variables
 *   - retrieve logAcqIdx and move cursor in acq multirecord
 *   - modify acquirer totals directly in the data base
 *
 * Remarks:
 * - to modify string amounts use addStr and subStr functions, see tcab0049
 * - to be done: treatment of tips
 * - acquirer totals have less priority
 *
 * \header log\\log.h
 * \source log\\log.c
 */
int logCalcTot(void)
{
	// Application Totals Variable

	word
			totTipCnt = 0; // totManCnt = 0;

	// Transaction Type Totals Variable
	char 	totDbAmt[lenTotAmt + 1], totCrAmt[lenTotAmt + 1],
			totAmtSale[lenTotAmt + 1], totAmtDbSale[lenTotAmt + 1], totAmtCrSale[lenTotAmt + 1],
			totAmtPrecom[lenTotAmt + 1],	totAmtDbPrecom[lenTotAmt + 1], totAmtCrPrecom[lenTotAmt + 1],
			totAmtRefund[lenTotAmt + 1], totAmtDbRefund[lenTotAmt + 1], totAmtCrRefund[lenTotAmt + 1],
			totAmtVoid[lenTotAmt + 1], totAmtDbVoid[lenTotAmt + 1], totAmtCrVoid[lenTotAmt + 1],
			totAmtVoidPrecom[lenTotAmt + 1], totAmtDbVoidPrecom[lenTotAmt + 1], totAmtCrVoidPrecom[lenTotAmt + 1],
			totAmtVoidRefund[lenTotAmt + 1], totAmtDbVoidRefund[lenTotAmt + 1], totAmtCrVoidRefund[lenTotAmt + 1],

			totAmtOffline[lenTotAmt + 1], 	totAmtManual[lenTotAmt + 1], totAmtVoidManual[lenTotAmt + 1], totTipAmt[lenTotAmt + 1];

	word 	totDbCnt = 0, totCrCnt = 0,
			totCntSale =  0, totCntDbSale =  0, totCntCrSale =  0,
			totCntPrecom = 0, totCntDbPrecom = 0, totCntCrPrecom = 0,
			totCntRefund = 0, totCntDbRefund = 0, totCntCrRefund = 0,
			totCntVoid = 0, totCntDbVoid = 0, totCntCrVoid = 0,
			totCntVoidPrecom = 0, totCntDbVoidPrecom = 0, totCntCrVoidPrecom = 0,
			totCntVoidRefund = 0, totCntDbVoidRefund = 0, totCntCrVoidRefund = 0,

			totCntOffline = 0, totCntManual = 0,  totCntVoidManual = 0 ;



	// Operation variables
	char 	txnAmt[lenAmt + 1], tipAmt[lenAmt + 1];
	char 	prvAmt[lenTotAmt + 1];
	byte		txnType = ' ', voidFlag = 0, entMod;
	word 	curMnuItem, idx = 0, trxNum, transTypeOri;
	int 		ret = 0, ctr = 0;
	byte 	acqCurIdx, acqTxnIdx, issTxnIdx;
	byte 	isAnyTrans = 0; //@agmr - periksa ada transaksi atau tidak

	trcS("logCalcTot: Beg\n");

	//Initializations
	MAPPUTSTR(totAppCrAmt, "0", lblKO);
	MAPPUTSTR(totAppDbAmt, "0", lblKO);
	MAPPUTSTR(totAppSaleAmt, "0", lblKO);
	MAPPUTSTR(totAppSaleDbAmt, "0", lblKO);
	MAPPUTSTR(totAppSaleCrAmt, "0", lblKO);
	MAPPUTSTR(totAppPrecomAmt, "0", lblKO);
	MAPPUTSTR(totAppPrecomDbAmt, "0", lblKO);
	MAPPUTSTR(totAppPrecomCrAmt, "0", lblKO);
	MAPPUTSTR(totAppRefundAmt, "0", lblKO);
	MAPPUTSTR(totAppRefundDbAmt, "0", lblKO);
	MAPPUTSTR(totAppRefundCrAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidDbAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidCrAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidPrecomAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidPrecomDbAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidPrecomCrAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidRefundAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidRefundDbAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidRefundCrAmt, "0", lblKO);

	MAPPUTSTR(totAppOfflineAmt, "0", lblKO);
	MAPPUTSTR(totAppManualAmt, "0", lblKO);
	MAPPUTSTR(totAppVoidManualAmt, "0", lblKO);
	MAPPUTSTR(totAppTpAmt, "0", lblKO);



	MAPPUTWORD(totAppCrCnt, 0, lblKO);
	MAPPUTWORD(totAppDbCnt, 0, lblKO);
	MAPPUTWORD(totAppSaleCnt, 0, lblKO);
	MAPPUTWORD(totAppPrecomCnt, 0, lblKO);
	MAPPUTWORD(totAppPrecomDbCnt, 0, lblKO);
	MAPPUTWORD(totAppPrecomCrCnt, 0, lblKO);
	MAPPUTWORD(totAppRefundCnt, 0, lblKO);
	MAPPUTWORD(totAppRefundDbCnt, 0, lblKO);
	MAPPUTWORD(totAppRefundCrCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidDbCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidCrCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidPrecomCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidPrecomDbCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidPrecomCrCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidRefundCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidRefundDbCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidRefundCrCnt, 0, lblKO);

	MAPPUTWORD(totAppOfflineCnt, 0, lblKO);
	MAPPUTWORD(totAppManualCnt, 0, lblKO);
	MAPPUTWORD(totAppVoidManualCnt, 0, lblKO);
	MAPPUTWORD(totAppTpCnt, 0, lblKO);


	memset(totDbAmt, 0, sizeof(totDbAmt));
	memset(totCrAmt, 0, sizeof(totCrAmt));
	memset(totTipAmt, 0, sizeof(totTipAmt));
	memset(totAmtSale, 0, sizeof(totAmtSale));
	memset(totAmtPrecom, 0, sizeof(totAmtPrecom));
	memset(totAmtDbPrecom, 0, sizeof(totAmtDbPrecom));
	memset(totAmtCrPrecom, 0, sizeof(totAmtCrPrecom));
	memset(totAmtRefund, 0, sizeof(totAmtRefund));
	memset(totAmtDbRefund, 0, sizeof(totAmtDbRefund));
	memset(totAmtCrRefund, 0, sizeof(totAmtCrRefund));

	memset(totAmtVoid, 0, sizeof(totAmtVoid));
	memset(totAmtDbVoid, 0, sizeof(totAmtDbVoid));
	memset(totAmtCrVoid, 0, sizeof(totAmtCrVoid));
	memset(totAmtVoidPrecom, 0, sizeof(totAmtVoidPrecom));
	memset(totAmtDbVoidPrecom, 0, sizeof(totAmtDbVoidPrecom));
	memset(totAmtCrVoidPrecom, 0, sizeof(totAmtCrVoidPrecom));
	memset(totAmtVoidRefund, 0, sizeof(totAmtVoidRefund));
	memset(totAmtDbVoidRefund, 0, sizeof(totAmtVoidRefund));
	memset(totAmtCrVoidRefund, 0, sizeof(totAmtVoidRefund));

	memset(totAmtOffline, 0, sizeof(totAmtOffline));
	memset(totAmtManual, 0, sizeof(totAmtManual));
	memset(totAmtVoidManual, 0, sizeof(totAmtVoidManual));

	totDbAmt[0]       		= '0';
	totCrAmt[0]        		= '0';
	totAmtSale[0]     		= '0';
	totAmtPrecom[0]     	= '0';
	totAmtDbPrecom[0]     	= '0';
	totAmtCrPrecom[0]     	= '0';
	totAmtRefund[0] 		= '0';
	totAmtDbRefund[0] 	= '0';
	totAmtCrRefund[0] 	= '0';
	totAmtVoid[0]     		= '0';
	totAmtDbVoid[0]     	= '0';
	totAmtCrVoid[0]     	= '0';
	totAmtVoidPrecom[0]    = '0';
	totAmtDbVoidPrecom[0] = '0';
	totAmtCrVoidPrecom[0] = '0';
	totAmtVoidRefund[0]     = '0';
	totAmtDbVoidRefund[0] = '0';
	totAmtCrVoidRefund[0]  = '0';

	totTipAmt[0]       		= '0';
	totAmtOffline[0]  		= '0';
	totAmtManual[0] 		= '0';
	totAmtVoidManual[0]     = '0';

	//Get current transaction number
	MAPGETWORD(regTrxNo, trxNum, lblKO);
	MAPGETBYTE(acqID, acqCurIdx, lblKO);

	//Computation loop
	for (ctr = 0; ctr < trxNum; ctr++)
	{
		/// Get amount and transaction type from LOG
		mapMove(logBeg, ctr);
		MAPGETWORD(logMnuItm, curMnuItem, lblKO);
		MAPGET(logTraAmt, txnAmt, lblKO);
		MAPGET(logTipAmt, tipAmt, lblKO);

		MAPGETBYTE(logAcqIdx, acqTxnIdx, lblKO);
		MAPGETBYTE(logIssIdx, issTxnIdx, lblKO);

		//Check if current acquirer is the same for this record
		if(acqTxnIdx != acqCurIdx - 1)
			continue;

		isAnyTrans = 1; //@agmr;

		MAPGETBYTE(logEntMod, entMod, lblKO);
		MAPGETBYTE(logVoidFlag, voidFlag, lblKO);
		if((entMod == 'k' || entMod == 'K')&&(voidFlag != 1))
		{
			addStr(totAmtManual, totAmtManual, txnAmt);
			totCntManual++;
		}

		// Void Flag Check
		//MAPGETBYTE(logVoidFlag, voidFlag, lblKO);
		if(voidFlag == 1)
			curMnuItem = mnuVoid;

		//Determine index of equivalent menu item transaction
		switch (curMnuItem)
		{
			case mnuRefund:
				addStr(totAmtRefund, totAmtRefund, txnAmt);
				totCntRefund++;
				idx = 3;
				//if(issTxnIdx==1)
				if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
				{
					addStr(totAmtDbRefund, totAmtDbRefund, txnAmt);
					totCntDbRefund++;
				}
				//else if((issTxnIdx!=1)&&(curMnuItem != mnuVoid))
				else if((!checkDebitIssuer(issTxnIdx))&&(curMnuItem != mnuVoid))//@@SIMAS-CRDB - start
				{
					addStr(totAmtCrRefund, totAmtCrRefund, txnAmt);
					totCntCrRefund++;
				}
				break;

			case mnuOffline:
				addStr(totAmtOffline, totAmtOffline, txnAmt);
				totCntOffline++;
				idx = 16;
				break;
#ifdef PREPAID
			case mnuPrepaidPayment:
			case mnuPrepaidPaymentDisc:
#endif
			case mnuInstallment: //BRI7
			case mnuSaleRedeem: //@ar - BRI
			case mnuSale:
				if((entMod != 'k') && (entMod != 'K')){
					addStr(totAmtSale, totAmtSale, txnAmt);
					totCntSale++;
					idx = 0;

				}

				break;
			case mnuPreCom:
				addStr(totAmtPrecom, totAmtPrecom, txnAmt);
				totCntPrecom++;
				idx = 16;
				//if(issTxnIdx==1)
				if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
				{
					addStr(totAmtDbPrecom, totAmtDbPrecom, txnAmt);
					totCntDbPrecom++;
				}
				//else if((issTxnIdx!=1)&&(curMnuItem != mnuVoid))
				else if((!checkDebitIssuer(issTxnIdx))&&(curMnuItem != mnuVoid))//@@SIMAS-CRDB - start
				{
					addStr(totAmtCrPrecom, totAmtCrPrecom, txnAmt);
					totCntCrPrecom++;
				}
				break;

			case mnuVoid:
				MAPGETBYTE(logEntMod, entMod, lblKO);
				MAPGETWORD(logMnuItm, transTypeOri, lblKO);
				if(((transTypeOri == mnuSale)||(transTypeOri == mnuSaleRedeem))&& ((entMod != 'k') && (entMod != 'K'))){
					addStr(totAmtVoid, totAmtVoid, txnAmt);
					addStr(totAmtVoid, totAmtVoid, tipAmt);   //if sale is voided add corresponding tip with it
					totCntVoid++;
					idx = 7;
					//if(issTxnIdx==1)
					if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
					{
						addStr(totAmtDbVoid, totAmtDbVoid, txnAmt);
						totCntDbVoid++;
					}
					//else if(issTxnIdx!=1)
					else //@@SIMAS-CRDB - start
					{
						addStr(totAmtCrVoid, totAmtCrVoid, txnAmt);
						totCntCrVoid++;
					}
				}
				if(transTypeOri == mnuRefund){
					addStr(totAmtVoidRefund, totAmtVoidRefund, txnAmt);
					totCntVoidRefund++;
					idx = 7;
					//if(issTxnIdx==1)
					if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
					{
						addStr(totAmtDbVoidRefund, totAmtDbVoidRefund, txnAmt);
						totCntDbVoidRefund++;
					}
					//else if(issTxnIdx!=1)
					else //@@SIMAS-CRDB - start
					{
						addStr(totAmtCrVoidRefund, totAmtCrVoidRefund, txnAmt);
						totCntCrVoidRefund++;
					}
				}

				if(transTypeOri == mnuPreCom){
					addStr(totAmtVoidPrecom, totAmtVoidPrecom, txnAmt);
					totCntVoidPrecom++;
					idx = 7;
					//if(issTxnIdx==1)
					if (checkDebitIssuer(issTxnIdx)) //@@SIMAS-CRDB - start
					{
						addStr(totAmtDbVoidPrecom, totAmtDbVoidPrecom, txnAmt);
						totCntDbVoidPrecom++;
					}
					//else if(issTxnIdx!=1)
					else //@@SIMAS-CRDB - start
					{
						addStr(totAmtCrVoidPrecom, totAmtCrVoidPrecom, txnAmt);
						totCntCrVoidPrecom++;
					}
				}

				//MAPGETBYTE(logEntMod, entMod, lblKO);
				if((entMod == 'k' || entMod == 'K')&&(transTypeOri != mnuRefund))
				{
					addStr(totAmtVoidManual, totAmtVoidManual, txnAmt);
					totCntVoidManual++;
				}
				break;

			default:
				idx = 0;
				break;
			}

			//Get the transaction type
			mapMove(rqsBeg, idx);
			MAPGETBYTE(rqsDbCr, txnType, lblKO);

			//Calculate app totals
			if((entMod != 'k') && (entMod != 'K')){
				switch (txnType)
				{
					case 'D':
						addStr(totDbAmt, totDbAmt, txnAmt);
						totDbCnt++;
						//if((issTxnIdx==1)&&(curMnuItem != mnuVoid))
						if((checkDebitIssuer(issTxnIdx))&&(curMnuItem != mnuVoid))//@@SIMAS-CRDB - start
						{
							addStr(totAmtDbSale, totAmtDbSale, txnAmt);
							totCntDbSale++;
						}
						//else if((issTxnIdx!=1)&&(curMnuItem != mnuVoid))
						else if((!checkDebitIssuer(issTxnIdx))&&(curMnuItem != mnuVoid))
						{
							addStr(totAmtCrSale, totAmtCrSale, txnAmt);
							totCntCrSale++;
						}
						break;

					case 'C':
						addStr(totCrAmt, totCrAmt, txnAmt);
						totCrCnt++;
						//if((issTxnIdx==1)&&(curMnuItem != mnuVoid))
						if((checkDebitIssuer(issTxnIdx))&&(curMnuItem != mnuVoid))//@@SIMAS-CRDB - start
						{
							addStr(totAmtDbSale, totAmtDbSale, txnAmt);
							totCntDbSale++;
						}
						//else if((issTxnIdx!=1)&&(curMnuItem != mnuVoid))
						else if((!checkDebitIssuer(issTxnIdx))&&(curMnuItem != mnuVoid))
						{
							addStr(totAmtCrSale, totAmtCrSale, txnAmt);
							totCntCrSale++;
						}
						break;

					default:
						break;
				}
			}

			if(voidFlag != 1)
			{
				memset(prvAmt, 0, sizeof(prvAmt));
				memcpy(prvAmt, totTipAmt, sizeof(prvAmt));
				addStr(totTipAmt, totTipAmt, tipAmt);
				if(memcmp(prvAmt, totTipAmt, sizeof(totTipAmt) != 0))
					totTipCnt++;
			}
	}

	// Place totals in tot.xml
	MAPPUTSTR(totAppCrAmt, totCrAmt, lblKO);
	MAPPUTSTR(totAppDbAmt, totDbAmt, lblKO);
	MAPPUTSTR(totAppSaleAmt, totAmtSale, lblKO);
	MAPPUTSTR(totAppSaleDbAmt, totAmtDbSale, lblKO);
	MAPPUTSTR(totAppSaleCrAmt, totAmtCrSale, lblKO);
	MAPPUTSTR(totAppPrecomAmt, totAmtPrecom, lblKO);
	MAPPUTSTR(totAppPrecomDbAmt, totAmtDbPrecom, lblKO);
	MAPPUTSTR(totAppPrecomCrAmt, totAmtCrPrecom, lblKO);
	MAPPUTSTR(totAppRefundAmt, totAmtRefund, lblKO);
	MAPPUTSTR(totAppRefundDbAmt, totAmtDbRefund, lblKO);
	MAPPUTSTR(totAppRefundCrAmt, totAmtCrRefund, lblKO);
	MAPPUTSTR(totAppVoidAmt, totAmtVoid, lblKO);
	MAPPUTSTR(totAppVoidDbAmt, totAmtDbVoid, lblKO);
	MAPPUTSTR(totAppVoidCrAmt, totAmtCrVoid, lblKO);
	MAPPUTSTR(totAppVoidPrecomAmt, totAmtVoidPrecom, lblKO);
	MAPPUTSTR(totAppVoidPrecomDbAmt, totAmtDbVoidPrecom, lblKO);
	MAPPUTSTR(totAppVoidPrecomCrAmt, totAmtCrVoidPrecom, lblKO);
	MAPPUTSTR(totAppVoidRefundAmt, totAmtVoidRefund, lblKO);
	MAPPUTSTR(totAppVoidRefundDbAmt, totAmtDbVoidRefund, lblKO);
	MAPPUTSTR(totAppVoidRefundCrAmt, totAmtCrVoidRefund, lblKO);

	MAPPUTSTR(totAppTpAmt, totTipAmt, lblKO);
	MAPPUTSTR(totAppOfflineAmt, totAmtOffline, lblKO);
	MAPPUTSTR(totAppManualAmt, totAmtManual, lblKO);
	MAPPUTSTR(totAppVoidManualAmt, totAmtVoidManual, lblKO);

	MAPPUTWORD(totAppCrCnt, totCrCnt, lblKO);
	MAPPUTWORD(totAppDbCnt, totDbCnt, lblKO);
	MAPPUTWORD(totAppSaleCnt, totCntSale, lblKO);
	MAPPUTWORD(totAppSaleDbCnt, totCntDbSale, lblKO);
	MAPPUTWORD(totAppSaleCrCnt, totCntCrSale, lblKO);
	MAPPUTWORD(totAppPrecomCnt, totCntPrecom, lblKO);
	MAPPUTWORD(totAppPrecomDbCnt, totCntDbPrecom, lblKO);
	MAPPUTWORD(totAppPrecomCrCnt, totCntCrPrecom, lblKO);
	MAPPUTWORD(totAppRefundCnt, totCntRefund, lblKO);
	MAPPUTWORD(totAppRefundDbCnt, totCntDbRefund, lblKO);
	MAPPUTWORD(totAppRefundCrCnt, totCntCrRefund, lblKO);
	MAPPUTWORD(totAppVoidCnt, totCntVoid, lblKO);
	MAPPUTWORD(totAppVoidDbCnt, totCntDbVoid, lblKO);
	MAPPUTWORD(totAppVoidCrCnt, totCntCrVoid, lblKO);
	MAPPUTWORD(totAppVoidPrecomCnt, totCntVoidPrecom, lblKO);
	MAPPUTWORD(totAppVoidPrecomDbCnt, totCntDbVoidPrecom, lblKO);
	MAPPUTWORD(totAppVoidPrecomCrCnt, totCntCrVoidPrecom, lblKO);
	MAPPUTWORD(totAppVoidRefundCnt, totCntVoidRefund, lblKO);
	MAPPUTWORD(totAppVoidRefundDbCnt, totCntDbVoidRefund, lblKO);
	MAPPUTWORD(totAppVoidRefundCrCnt, totCntCrVoidRefund, lblKO);

	MAPPUTWORD(totAppTpCnt, totTipCnt, lblKO);
	MAPPUTWORD(totAppOfflineCnt, totCntOffline, lblKO);
	MAPPUTWORD(totAppManualCnt, totCntManual, lblKO);
	MAPPUTWORD(totAppVoidManualCnt, totCntVoidManual, lblKO);




	//	ret = 0;
	ret = 1;
	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;

lblEnd:
//++ @agmr
    if(isAnyTrans == 0)
        ret = 0;
//-- @agmr

	trcFN("logCalcTot: ret=%d \n", ret);
	return ret;
}

/** Displays all transaction type totals using function logCalcTot
 * \source sys\\log.c
 */
int logConfirmTotals(void)
{
	int ret;
	char buf[10];
	char bufCur[5];
	card count;
	card txnAmt, totCnt, totAmt;
	char amt[lenTotAmt + 1];
	char total[3 + 1];

	trcS("logConfirmTotals Beg\n");

	ret = logCalcTot();
	CHK;

//++ @agmr
	if(ret == 0)
	    goto lblNoTran;
//-- @agmr

	//SALE
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppSaleCnt, count, lblKO);
	MAPGET(totAppSaleAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Sale Amt: %d \n", txnAmt);
	trcFN("after Sale Cnt: %d \n", count);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "SALE");
	MAPGET(appCurrSign, bufCur, lblKO);
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;

	//VOID
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppVoidCnt, count, lblKO);
	MAPGET(totAppVoidAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Void Amt: %d \n", txnAmt);
	trcFN("after Void Cnt: %d \n", count);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "VOID");
	MAPGET(appCurrSign, bufCur, lblKO);
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	/*
	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;
	*/
	//OFFLINE
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppOfflineCnt, count, lblKO);
	MAPGET(totAppOfflineAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Offline Amt: %d \n", txnAmt);
	trcFN("after Offline Cnt: %d \n", count);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "OFFLINE");
	MAPGET(appCurrSign, bufCur, lblKO);
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	/*
	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;
	*/
	//MANUAL
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppManualCnt, count, lblKO);
	MAPGET(totAppManualAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Manual Amt: %d \n", txnAmt);
	trcFN("after Manual Cnt: %d \n", count);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "MANUAL");
	MAPGET(appCurrSign, bufCur, lblKO);
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);
/*
	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;

*/
	MAPGET(totAppTpAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt += txnAmt;
	totCnt += count;
	trcFN("after Tip Amt: %d \n", txnAmt);
	trcFN("after Tip Cnt: %d \n", count);

	trcFN("Sale Total Amt: %d \n", totAmt);
	trcFN("Sale Total Cnt: %d \n", totCnt);

	//REFUND
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppRefundCnt, count, lblKO);
	MAPGET(totAppRefundAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt += txnAmt;
	totCnt += count;
	trcFN("mnuRefund Amt: %d \n", txnAmt);
	trcFN("mnuRefund Cnt: %d \n", count);

	trcFN("Refund Total Amt: %d \n", totAmt);
	trcFN("Refund Total Cnt: %d \n", totCnt);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "REFUND");
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;


	//PRECOMP
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppPrecomCnt, count, lblKO);
	MAPGET(totAppPrecomAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Precom Amt: %d \n", txnAmt);
	trcFN("after Precom Cnt: %d \n", count);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "PRECOMP");
	MAPGET(appCurrSign, bufCur, lblKO);
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;

	//VOID PRECOMP
	memset(buf, 0x00, sizeof(buf));
	memset(bufCur, 0x00, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETCARD(totAppVoidPrecomCnt, count, lblKO);
	MAPGET(totAppVoidPrecomAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Void Precom Amt: %d \n", txnAmt);
	trcFN("after Void Precom Cnt: %d \n", count);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld", totAmt);
	strcpy(buf, "VOID PRECOMP");
	MAPGET(appCurrSign, bufCur, lblKO);
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);
/*
	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblCancel;
*/
	goto lblEnd;

lblNoTran:
    ret = 101;
    goto lblEnd;
lblCancel:
	trcS("User Cancelled\n");
//	ret = -1;
    ret = 0; //@agmr
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewTotals End\n");
	return ret;
}

static int logBatchIncrement(void)
{
	int ret;
	char currBat[lenBatNum + 1];
	char newBat[lenBatNum + 1];
	char BatNum[lenBatNum + 1];
	byte len, counter;

	trcS("logBatchIncrement Beg\n");

	//increment batch number
	MAPGET(acqCurBat, currBat, lblKO);
	trcFS("currBat: %s\n", currBat);
	memset(newBat, 0x00, sizeof(newBat));
	addStr(newBat, currBat, "000001");
	trcFS("newbat: %s\n", newBat);
	len = strlen(newBat);
	for (counter = 0; counter < (lenBatNum - len); counter++)
		BatNum[counter] = '0';
	memcpy(&BatNum[counter], newBat, len);
	trcFS("BatNum: %s\n", BatNum);
	MAPPUTSTR(acqCurBat, BatNum, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("logBatchIncrement: ret=%d \n", ret);
	return ret;
}

/** Settlement processing.
 * \header log\\log.h
 * \source log\\log.c
 */
void logSettlement(void)
{
	int ret;
//	card count;
	word mnuItem;
	byte acqCnt;
	char acqCurName[lenChdNam + 1];
	int counter;
	char currBat[lenBatNum + 10];
//	byte acqIDBefore;
	char datetime[lenDatTim + 1];
	word idx, trxNum;
	char Rrn[lenRrn + 1];
	char buf[lenMnu + 1];
	byte sendoffline;
	byte TxnTypebeforeOffline;
	word TxnMnuItmbeforeOffline;
	word countOffline;
	byte temp; //@agmr
	byte settlementAcq, currAcq; //@agmr
	int counterAwal,counterAkhir; //@agmr
	byte isMultiAcq = 0;
    byte revFlag, isEmpty=0;

	//AB: which transaction will be done? MOTO?
	trcS("logSettlement Beg\n");

    //@agmr - tutup
//	MAPGETBYTE(acqID, acqIDBefore, lblKO);  // save the current acqID before processing

	MAPGETWORD(traMnuItm, mnuItem, lblKO);
	ret = valAskPwd(appSetlPwd);
	CHECK((ret > 0) && (ret != kbdANN), lblKO);
	MAPGETBYTE(regAcqCnt, acqCnt, lblKO);

	txnAsli = 0; //@agmr

//++ @agmr - multi settle
	/*ret = selectAcquirer();
	if(ret < 0)
	    goto lblKO;
	if(ret == 0xff) //semua ACQ
	{*/
		isMultiAcq = 1;
		counterAwal = 0;
		counterAkhir = dimAcq;
	/*}
	else
	{
		isMultiAcq = 0;
		counterAwal = ret-1; //acquirer index = acq id -1
		counterAkhir = ret;
	}*/
//-- @agmr - multi settle
//++ @agmr - multi settle
	ret = batchSync();
	if(ret < 0)
		goto lblKO;
//-- @agmr - multi settle
//	for (counter = 0; counter < dimAcq; counter++)
	for (counter = counterAwal; counter < counterAkhir; counter++)
	{
		//mapMove(acqBeg, 0);
		mapMove(acqBeg, counter);
		MAPGET(acqCurBat, currBat, lblKO);
		trcFS("currBat: %s\n", currBat);
		MAPGET(acqName, acqCurName, lblKO);
		if(*acqCurName == 0)
			continue;

		//++ @agmr - BRI9
		if(memcmp(acqCurName,"MINI ATM",8) == 0 ||
			memcmp(acqCurName,"TUNAI",5) == 0    ||
			memcmp(acqCurName,"BRIZZI",5) == 0
		)
			continue;
		//-- @agmr - BRI9

		//++ @agmr - multi settle
		MAPGETBYTE(acqID, temp, lblKO);
		MAPPUTBYTE(appAcqSettlementIdx,temp-1,lblKO);
		//-- @agmr - multi settle

		MAPGET(acqCurBat, currBat, lblKO);
		trcFS("currBat: %s\n", currBat);
//++ @agmr
		ret = getBriReversalIdx();
		if(ret <=0)
			goto lblKO;
		MAPGETWORD(regTrxNo, trxNum, lblKO); //BRI9
		MAPGETBYTE(revReversalFlag, revFlag, lblKO);
		if(trxNum == 0)
			isEmpty = 2;
		else
			isEmpty = 0;

		if(isEmpty > 0)
		{
			if(revFlag == 0)
				goto lblBatchEmpty;
			else
				goto lblJump;
		}
//-- @agmr

//		CHECK(trxNum > 0, lblNoTran);		 //BRI9
//		MAPGETWORD(regTrxNo, count, lblKO);
//		CHECK(count > 0, lblNoTran);
/*
        for (ctr = 0; ctr < count; ctr++) {
            mapMove(logBeg, ctr);
            // Get amount and transaction type from LOG
            MAPGETBYTE(logAcqIdx, acqTxnIdx, lblKO);
            MAPGETBYTE(acqID, acqCurIdx, lblKO);

            //Check if current acquirer is the same for this record
            if(acqTxnIdx != acqCurIdx - 1)
                continue;
*/
		ret = logConfirmTotals();
		CHECK(ret > 0, lblKO);

//++ @agmr
		if(ret == 101)
			isEmpty = 1;
		else
			isEmpty = 0;

		if(isEmpty > 0) //tidak ada trans untuk acq ini
		{
			if(revFlag == 0)
			{
				if(isMultiAcq == 0)
					goto lblNoTran;
				else
					continue;
			}
			else
				goto lblJump;
		}
//-- @agmr
		//check first for offline transactions
		MAPGETWORD(regOfflineCnt, countOffline, lblKO);
		if(countOffline > 0)
		{
			sendoffline = 0;
			MAPGETBYTE(traTxnType, TxnTypebeforeOffline, lblKO);
			MAPPUTBYTE(traTxnTypeBeforeOffline, TxnTypebeforeOffline, lblKO);
			MAPGETWORD(traMnuItm, TxnMnuItmbeforeOffline, lblKO);
			MAPPUTBYTE(traMnuItmBeforeOffline, TxnMnuItmbeforeOffline, lblKO);

			MAPPUTBYTE(traTxnType, trtOffline, lblKO);
			MAPPUTWORD(traMnuItm, mnuOffline, lblKO);   //change first to offline for proper connection to host
			MAPGET(mnuOffline, buf, lblKO);
			MAPPUTSTR(traCtx, buf + 2, lblKO);

			//initialize log to point on first offline txn
			MAPPUTBYTE(regLocType, 'L', lblKO);
			MAPGETWORD(regTrxNo, trxNum, lblKO);
			idx = 0;
			while(idx < trxNum)
			{
				mapMove(logBeg, (word) idx);
				MAPGETWORD(logMnuItm, mnuItem, lblKO);
				if(mnuItem == mnuOffline)
//				  || mnuItem ==  mnuPrepaidPayment) //@agmr - brizzi
				{
                    //++ @agmr - multi settle
				    MAPGETBYTE(appAcqSettlementIdx,settlementAcq,lblKO);
				    MAPGETBYTE(logAcqIdx,currAcq,lblKO);
				    if(settlementAcq != currAcq)
				    {
				        idx++;
				        continue;
				    }
				    //-- @agmr - multi settle

					MAPGET(logRrn, Rrn, lblKO);
					if(strlen(Rrn) == 0)
					{
						sendoffline = 1;
						break;
					}
				}
				idx++;
			}
			if((idx <= trxNum) && (mnuItem == mnuOffline))
			{
				MAPPUTBYTE(traTxnType, trtOffline, lblKO);
				idx++;
				MAPPUTWORD(traIdx, idx, lblKO);
			}
			if(sendoffline == 1)
			{
				//Send offline Txns
				usrInfo(infOflSend);
				ret = onlSession();
				CHECK(ret >= 0, lblKO);
			}
			MAPPUTWORD(traMnuItm, mnuSettlement, lblKO);    //change back to settlement after completing offline sending
			MAPGET(mnuSettlement, buf, lblKO);
			MAPPUTSTR(traCtx, buf + 2, lblKO);
		}
lblJump: //@agmr
		MAPPUTSTR(traTransType, "SETTLEMENT", lblKO);
		MAPPUTBYTE(traTxnType, trtSettlement, lblKO);

        	MAPPUTBYTE(acqIsPendingSettlement, 1, lblKO); //@agmr
        	MAPPUTBYTE(regLocType, 'T', lblKO); //@agmr

//++ @agmr - supaya habis reversal tidak kirim msg 500
        	if(isEmpty>0)
        	{
           		MAPPUTWORD(traMnuItm, 0, lblKO);
           		MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
        	}
//-- @agmr - supaya habis reversal tidak kirim msg 500

		ret = onlSession();

		CHECK(ret >= 0, lblKO);
		MAPGET(acqCurBat, currBat, lblKO);
		MAPPUTSTR(totBatNum, currBat, lblKO);
		MAPGET(traDatTim, datetime, lblKO);
		MAPPUTSTR(totDatTim, datetime, lblKO);
		strFormatDatTim(traDatTim);

        	if(isEmpty == 2)
            		goto lblBatchEmpty;
        	else if (isEmpty == 1)
            		goto lblNoTran;

//		logPrintLog();
		logSettlementRpt();
		logBatchIncrement();
//++ @agmr - multi settle	//NANTI DIBUKA !!!
        	MAPGETBYTE(acqID, temp, lblKO);
		ret = briLogReset(temp);
		if(ret < 0)
		{
        		ret = batchSync();
        		if(ret == -100)
        	    		goto lblKO;
		    	goto lblBatchError;
		}
//		logSettlementConfirm(1);
//-- @agmr - multi settle
        	MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
	}
//	logReset(); //@agmr - tutup

    //@agmr - tutup
//	mapMove(acqBeg, (word) (acqIDBefore - 1));  // return the index to its original place in the acquirer

	goto lblEnd;
lblBatchError:
//    logSettlementConfirm(0);
    usrInfo(infBatchError);
    goto lblEnd;
lblBatchEmpty:
    MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
    usrInfo(infLogEmpty);
    goto lblEnd;
lblNoTran:
//    MAPPUTBYTE(appPendingSettlement, 0, lblKO); //@agmr
    MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
	usrInfo(infNoTran);
	goto lblEnd;
lblKO:
lblEnd:
	trcS("logSettlement End\n");
}

/** Settlement ReceiptPrinting
 * printing of settlement receipt
 * \header log\\log.h
 * \source log\\log.c
 */
void logSettlementRpt(void)
{
	int 		ret, idx, idy;
	char 	amountBuf[lenTotAmt + 1], cardName[lenChdNam + 1];
	word 	count, countChk1, countChk2, countChk3, countChk4,
			countChk5, countChk6, countChk7, countChk8, countChk9;   // count check buffers
	byte 	stDup = 0;
	byte 	binCnt;
	char 	curr[4+1];
	char 	mincurr[5+1];

	trcS("logSettlementRpt Beg\n");

	stDup = setDuplicate;
	setDuplicate = 0;
	strFormatDatTim(totDatTim);
	// Printing Headers
	ret = usrInfo(infConfigPrinting);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloHeader);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloSetHdr);
	CHECK(ret >= 0, lblKO);

	if(stDup)
		ret = rptReceipt(rloSetAcqDetLog);
	else
		ret = rptReceipt(rloSetAcqDet);

	CHECK(ret >= 0, lblKO);

	if(!stDup)
	{
		// Calculate Acquirer Totals
		ret = logCalculateTotalsByCardTypeAndCurAcq();
		CHECK(ret >= 0, lblKO);
	}

	//For receipt currency
    memset(curr,0,sizeof(curr));
	MAPGET(appCurrSign, curr, lblKO);
	MAPPUTSTR(rptPhpPlus, curr, lblKO);
	mincurr[0] = '-';
	memcpy(mincurr + 1, curr, 4);
//	MAPPUTSTR(rptPhpPlus, mincurr, lblKO);
    MAPPUTSTR(rptPhpMinus, mincurr, lblKO);

	// Print Header
	ret = rptReceipt(rloRptHeaderLog);
	CHECK(ret >= 0, lblKO);

	// Print Sale Totals
	MAPGET(acqTotAppDbAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(acqTotAppDbCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsSale);
	CHECK(ret >= 0, lblKO);

	// Print Precom Totals
	MAPGET(totAppPrecomAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(totAppPrecomCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsPrecom);
	CHECK(ret >= 0, lblKO);

	// Print Tip Totals
	/*
	MAPGET(acqTotAppTpAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(acqTotAppTpCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsTip);
	CHECK(ret >= 0, lblKO);
	*/

	/*
	// Print Manual Totals
	MAPGET(totAppManualAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(totAppManualCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsManual);
	CHECK(ret >= 0, lblKO);
	*/

	// Print Refund Totals
	MAPGET(totAppRefundAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(totAppRefundCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsRefund);
	CHECK(ret >= 0, lblKO);

	// Print Void Sale Totals
	MAPGET(totAppVoidAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(totAppVoidCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsVoid);
	CHECK(ret >= 0, lblKO);

	// Print Void Precom Totals
	MAPGET(totAppVoidPrecomAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(totAppVoidPrecomCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
	CHECK(ret >= 0, lblKO);

	// Print Void Refund Totals
	MAPGET(totAppVoidRefundAmt, amountBuf, lblKO);
	MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
	MAPGETWORD(totAppVoidRefundCnt, count, lblKO);
	MAPPUTWORD(traLogTotalCount, count, lblKO);
	ret = rptReceipt(rloTrxLogTotalsVoidRefund);
	CHECK(ret >= 0, lblKO);


	for(idy=0; idy<dimIss;idy++)
	{
		switch(idy)
		{
			case 1:
				ret = rptReceipt(rloSetIssDetDb);
				CHECK(ret >= 0, lblKO);

				// Print Header
				ret = rptReceipt(rloRptHeaderLog);
				CHECK(ret >= 0, lblKO);

				// Print Sale Totals
				MAPGET(issTotAppDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsSale);
				CHECK(ret >= 0, lblKO);

				// Print Precom Totals
				MAPGET(issTotAppPrecomDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppPrecomDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Refund Totals
				MAPGET(issTotAppRefundDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppRefundDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsRefund);
				CHECK(ret >= 0, lblKO);

				// Print Void Totals
				MAPGET(issTotAppVoidDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppVoidDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoid);
				CHECK(ret >= 0, lblKO);

				// Print Void Precom Totals
				MAPGET(issTotAppVoidPrecomDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppVoidPrecomDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Void Refund Totals
				MAPGET(issTotAppVoidRefundDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppVoidRefundDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidRefund);
				CHECK(ret >= 0, lblKO);

				break;

			case 0:
				ret = rptReceipt(rloSetIssDetCr);
				CHECK(ret >= 0, lblKO);

				// Print Header
				ret = rptReceipt(rloRptHeaderLog);
				CHECK(ret >= 0, lblKO);

				// Print Sale Totals
				MAPGET(issTotAppCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsSale);
				CHECK(ret >= 0, lblKO);

				// Print Precom Totals
				MAPGET(issTotAppPrecomCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppPrecomCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Refund Totals
				MAPGET(issTotAppRefundCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppRefundCrbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsRefund);
				CHECK(ret >= 0, lblKO);

				// Print Void Totals
				MAPGET(issTotAppVoidCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppVoidCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoid);
				CHECK(ret >= 0, lblKO);

				// Print Void Precom Totals
				MAPGET(issTotAppVoidPrecomCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppVoidPrecomCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Void Refund Totals
				MAPGET(issTotAppVoidRefundCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(issTotAppVoidRefundCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidRefund);
				CHECK(ret >= 0, lblKO);

				break;
		}
	}

	//Print Total Card Summary
	//MAPGETBYTE(regBinCnt, binCnt, lblKO);
	for (idx = 0; idx < dimBin; idx++)
	{
		ret = mapMove(binBeg, idx);
		MAPGETWORD(binTotAppDbCnt, count, lblKO);
		MAPGETWORD(binTotAppCrCnt, countChk1, lblKO);
		MAPGETWORD(binTotAppPrecomCnt, countChk2, lblKO);
		MAPGETWORD(binTotAppRefundCnt, countChk3, lblKO);
		MAPGETWORD(binTotAppVoidCnt, countChk4, lblKO);
		MAPGETWORD(binTotAppVoidPrecomCnt, countChk5, lblKO);
		MAPGETWORD(binTotAppVoidRefundCnt, countChk6, lblKO);
		if(count != 0 || countChk1 != 0 || countChk2 != 0 || countChk3 != 0 ||
			countChk4 != 0 || countChk5 != 0 || countChk6 != 0 )
			break;
	}

	if(count != 0 || countChk1 != 0 || countChk2 != 0 || countChk3 != 0 ||
		countChk4 != 0 || countChk5 != 0 || countChk6 != 0)
	{
		ret = rptReceipt(rloSetTotalsByCard);
		CHECK(ret >= 0, lblKO);

		for (idx = 0; idx < dimBin; idx++)
		{
			ret = mapMove(binBeg, idx);
			MAPGET(binName, cardName, lblKO);
			if(*cardName == 0)
				continue;


			MAPGETWORD(binTotAppDbCnt, count, lblKO);
			MAPGETWORD(binTotAppCrCnt, countChk1, lblKO);
			MAPGETWORD(binTotAppPrecomCnt, countChk2, lblKO);
			MAPGETWORD(binTotAppRefundCnt, countChk3, lblKO);
			MAPGETWORD(binTotAppVoidCnt, countChk4, lblKO);
			MAPGETWORD(binTotAppVoidPrecomCnt, countChk5, lblKO);
			MAPGETWORD(binTotAppVoidRefundCnt, countChk6, lblKO);

			if(count == 0 && countChk1 == 0 && countChk2 == 0 && countChk3 == 0 &&
				countChk4 == 0 && countChk5 == 0 && countChk6 == 0)
				continue;

			// Print Card Name
			ret = rptReceipt(rloSetCardName);
			CHECK(ret >= 0, lblKO);

			// Print Header
			ret = rptReceipt(rloRptHeaderLog);
			CHECK(ret >= 0, lblKO);

			// Print Sale Totals
			MAPGET(binTotAppDbAmt, amountBuf, lblKO);
			MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
			MAPPUTWORD(traLogTotalCount, count, lblKO);
			ret = rptReceipt(rloTrxLogTotalsSale);
			CHECK(ret >= 0, lblKO);

			// Print Precom Totals
			MAPGET(binTotAppPrecomAmt, amountBuf, lblKO);
			MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
			MAPPUTWORD(traLogTotalCount, countChk2, lblKO);
			ret = rptReceipt(rloTrxLogTotalsPrecom);
			CHECK(ret >= 0, lblKO);

			// Print Refund Card Totals
			MAPGET(binTotAppRefundAmt, amountBuf, lblKO);
			MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
			MAPPUTWORD(traLogTotalCount, countChk3, lblKO);
			ret = rptReceipt(rloTrxLogTotalsRefund);
			CHECK(ret >= 0, lblKO);

			// Print Void Totals
			MAPGET(binTotAppVoidAmt, amountBuf, lblKO);
			MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
			MAPPUTWORD(traLogTotalCount, countChk4, lblKO);
			ret = rptReceipt(rloTrxLogTotalsVoid);
			CHECK(ret >= 0, lblKO);

			// Print Void Precom Totals
			MAPGET(binTotAppVoidPrecomAmt, amountBuf, lblKO);
			MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
			MAPPUTWORD(traLogTotalCount, countChk5, lblKO);
			ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
			CHECK(ret >= 0, lblKO);

			// Print Void Refund Totals
			MAPGET(binTotAppVoidRefundAmt, amountBuf, lblKO);
			MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
			MAPPUTWORD(traLogTotalCount, countChk6, lblKO);
			ret = rptReceipt(rloTrxLogTotalsVoidRefund);
			CHECK(ret >= 0, lblKO);

		}
	}

	ret = rptReceipt(rloSep);
	CHECK(ret >= 0, lblKO);

	ret = rptReceipt(rloSetConfirm);
	CHECK(ret >= 0, lblKO);

    //++ @agmr - tutup
//	ret = rptReceipt(rloSetConfirm);
//	CHECK(ret >= 0, lblKO);
    //-- @agmr - tutup

	if(stDup)
		ret = rptReceipt(rloDuplicateLine);

    else
        rptReceipt(rloFeed);

	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("logSettlementRpt: ret=%d \n", ret);
}

/** Printing transaction log
 * \header log\\log.h
 * \source log\\log.c
 */
int logPrintLog(void)
{
	char amountBuf[lenTotAmt + 1];
	char traAmt[lenAmt + 1];
	char tipAmt[lenAmt + 1];
	card count;
	int ret;
	word idx, idy, dim;
	byte acqCurIdx, acqTxnIdx, issTxnIdx;

	trcS("logPrintLog Beg\n");

	// Get current number of transactions
	MAPGETWORD(regTrxNo, dim, lblKO);
	CHECK(dim > 0, lblLogEmpty);

	strFormatDatTim(traDatTim);

	// Printing Headers
	ret = usrInfo(infConfigPrinting);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloHeader);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloTrxLogHdr);
	CHECK(ret >= 0, lblKO);

	// Calculating Application Totals
	ret = logCalcTot();
	CHECK(ret >= 0, lblKO);

//++@agmr
    {
        char curr[4+1];
	    char mincurr[5+1];

        memset(curr,0,sizeof(curr));
    	MAPGET(appCurrSign, curr, lblKO);
    	MAPPUTSTR(rptPhpPlus, curr, lblKO);
    	mincurr[0] = '-';
    	memcpy(mincurr + 1, curr, 4);
        MAPPUTSTR(rptPhpMinus, mincurr, lblKO);
    }
//--@agmr

	for(idy=0; idy<dimIss;idy++)
	{
		switch(idy)
		{
			case 0:
				ret = rptReceipt(rloSetIssDetCr);
				CHECK(ret >= 0, lblKO);

				// Print Sale Totals
				MAPGET(totAppSaleCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppSaleCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsSale);
				CHECK(ret >= 0, lblKO);

				// Print Precom Totals
				MAPGET(totAppPrecomCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppPrecomCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Refund Totals
				MAPGET(totAppRefundCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppRefundCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsRefund);
				CHECK(ret >= 0, lblKO);

				// Print Void Totals
				MAPGET(totAppVoidCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppVoidCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoid);
				CHECK(ret >= 0, lblKO);

				// Print Void Precom Totals
				MAPGET(totAppVoidPrecomCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppVoidPrecomCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Void Refund Totals
				MAPGET(totAppVoidRefundCrAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppVoidRefundCrCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidRefund);
				CHECK(ret >= 0, lblKO);

				// Printing Transaction Totals
				MAPGETWORD(regTrxNo, dim, lblKO);
				MAPGETBYTE(acqID, acqCurIdx, lblKO);
				for (idx = 0; idx < dim; idx++)
				{
					mapMove(logBeg, idx);

					MAPGETBYTE(logAcqIdx, acqTxnIdx, lblKO);
					MAPGETBYTE(logIssIdx, issTxnIdx, lblKO);

					//Check if current acquirer is the same for this record
					if(acqTxnIdx != acqCurIdx - 1)
						continue;


					//if(issTxnIdx != idy)
					if (checkDebitIssuer(issTxnIdx))//@@SIMAS-CRDB
						continue;

					memset(amountBuf, 0x00, sizeof(amountBuf));
					MAPGET(logTraAmt, traAmt, lblKO);
					MAPGET(logTipAmt, tipAmt, lblKO);
					addStr(amountBuf, traAmt, tipAmt);
					MAPPUTSTR(traTotAmt, amountBuf, lblKO);

					ret = rptReceipt(rloTrxLogBody);
					CHECK(ret >= 0, lblKO);
				}

				ret = rptReceipt(rloFeed);
				CHECK(ret >= 0, lblKO);



				break;

			case 1:
				ret = rptReceipt(rloSetIssDetDb);
				CHECK(ret >= 0, lblKO);

				// Print Sale Totals
				MAPGET(totAppSaleDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppSaleDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsSale);
				CHECK(ret >= 0, lblKO);

				// Print Precom Totals
				MAPGET(totAppPrecomDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppPrecomDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Refund Totals
				MAPGET(totAppRefundDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppRefundDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsRefund);
				CHECK(ret >= 0, lblKO);

				// Print Void Totals
				MAPGET(totAppVoidDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppVoidDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoid);
				CHECK(ret >= 0, lblKO);

				// Print Void Precom Totals
				MAPGET(totAppVoidPrecomDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppVoidPrecomDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Void Refund Totals
				MAPGET(totAppVoidRefundDbAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPGETWORD(totAppVoidRefundDbCnt, count, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidRefund);
				CHECK(ret >= 0, lblKO);

				// Printing Transaction Totals
				MAPGETWORD(regTrxNo, dim, lblKO);
				MAPGETBYTE(acqID, acqCurIdx, lblKO);
				for (idx = 0; idx < dim; idx++)
				{
					mapMove(logBeg, idx);

					MAPGETBYTE(logAcqIdx, acqTxnIdx, lblKO);
					MAPGETBYTE(logIssIdx, issTxnIdx, lblKO);

					//Check if current acquirer is the same for this record
					if(acqTxnIdx != acqCurIdx - 1)
						continue;

					//if(issTxnIdx != idy)
					if (!checkDebitIssuer(issTxnIdx))//@@SIMAS-CRDB
						continue;

					memset(amountBuf, 0x00, sizeof(amountBuf));
					MAPGET(logTraAmt, traAmt, lblKO);
					MAPGET(logTipAmt, tipAmt, lblKO);
					addStr(amountBuf, traAmt, tipAmt);
					MAPPUTSTR(traTotAmt, amountBuf, lblKO);

					ret = rptReceipt(rloTrxLogBody);
					CHECK(ret >= 0, lblKO);
				}

				ret = rptReceipt(rloFeed);
				CHECK(ret >= 0, lblKO);

				break;
		}
	}


	ret = 1;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblLogEmpty:
	usrInfo(infLogEmpty);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("logPrintLog: ret=%d\n", ret);
	return ret;
}

/** Host totals processing
 * Prints host totals
 * \header log\\log.h
 * \source log\\log.c
 */
void logHostTotals(void)
{
	int ret, idx, counter;
	char amountBuf[lenTotAmt + 1], cardName[lenChdNam + 1],
	acqCurName[lenChdNam + 1];
	word count;//, countChk1, countChk2, countChk3, countChk4, countChk5, countChk6, countChk7;
	word dim;
//	byte acqBeforeCalc; @agmr - tutup

	trcS("logHostTotals Beg\n");

	MAPGETWORD(regTrxNo, dim, lblKO);
	CHECK(dim > 0, lblLogEmpty);

	strFormatDatTim(traDatTim);

	// Printing Headers
	ret = usrInfo(infConfigPrinting);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloHeader);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloHostTotHdr);
	CHECK(ret >= 0, lblKO);

	//get current acqID
//	MAPGETBYTE(acqID, acqBeforeCalc, lblKO); //@agmr - tutup - dapat menimbulkan processing error

	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, acqCurName, lblKO);
		if(*acqCurName == 0)
			continue;

//++BRI9
	    if(memcmp(acqCurName,"MINI ATM",8) == 0 ||
	    		memcmp(acqCurName,"TUNAI",5) == 0 ||
	    		memcmp(acqCurName,"DEBITSINAR",10) == 0
	       )
	       continue;
//--BRI9

		// Calculate Acquirer Totals
		ret = logCalculateTotalsByCardTypeAndCurAcq();
		CHECK(ret >= 0, lblKO);

		ret = rptReceipt(rloHostTotAcqDet);
		CHECK(ret >= 0, lblKO);


		for(idx=0; idx<dimIss;idx++)
		{
			switch(idx)
			{
				case 1:
					ret = rptReceipt(rloSetIssDetDb);
					CHECK(ret >= 0, lblKO);

					// Print Header
					ret = rptReceipt(rloRptHeaderLog);
					CHECK(ret >= 0, lblKO);

					// Print Sale Totals
					MAPGET(issTotAppDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsSale);
					CHECK(ret >= 0, lblKO);

					// Print Precom Totals
					MAPGET(issTotAppPrecomDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppPrecomDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Refund Totals
					MAPGET(issTotAppRefundDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppRefundDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsRefund);
					CHECK(ret >= 0, lblKO);

					// Print Void Totals
					MAPGET(issTotAppVoidDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoid);
					CHECK(ret >= 0, lblKO);

					// Print Void Precom Totals
					MAPGET(issTotAppVoidPrecomDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidPrecomDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Void Refund Totals
					MAPGET(issTotAppVoidRefundDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidRefundDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidRefund);
					CHECK(ret >= 0, lblKO);

					break;

				case 0:
					ret = rptReceipt(rloSetIssDetCr);
					CHECK(ret >= 0, lblKO);

					// Print Header
					ret = rptReceipt(rloRptHeaderLog);
					CHECK(ret >= 0, lblKO);

					// Print Sale Totals
					MAPGET(issTotAppCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsSale);
					CHECK(ret >= 0, lblKO);

					// Print Precom Totals
					MAPGET(issTotAppPrecomCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppPrecomCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Refund Totals
					MAPGET(issTotAppRefundCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppRefundCrbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsRefund);
					CHECK(ret >= 0, lblKO);

					// Print Void Totals
					MAPGET(issTotAppVoidCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoid);
					CHECK(ret >= 0, lblKO);

					// Print Void Precom Totals
					MAPGET(issTotAppVoidPrecomCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidPrecomCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Void Refund Totals
					MAPGET(issTotAppVoidRefundCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidRefundCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidRefund);
					CHECK(ret >= 0, lblKO);

					break;
			}
		}

		/*
		// Print Total Card Summary
		//idx = 0;
		ret = mapMove(binBeg, counter);
		MAPGETWORD(binTotAppDbCnt, count, lblKO);
		MAPGETWORD(binTotAppCrCnt, countChk1, lblKO);
		MAPGETWORD(binTotAppTpCnt, countChk2, lblKO);
		MAPGETWORD(binTotAppVoidCnt, countChk3, lblKO);
		MAPGETWORD(binTotAppManualCnt, countChk4, lblKO);
		MAPGETWORD(binTotAppRefundCnt, countChk5, lblKO);
		MAPGETWORD(binTotAppPrecomCnt, countChk6, lblKO);
		MAPGETWORD(binTotAppVoidPrecomCnt, countChk7, lblKO);

		if(count != 0 || countChk1 != 0 || countChk2 != 0 || countChk3 != 0 || countChk4 != 0  || countChk5 != 0
			 || countChk6 != 0  || countChk7 != 0)
		{
			ret = rptReceipt(rloSetTotalsByCard);
			CHECK(ret >= 0, lblKO);
			for (idx = 0; idx < dimBin; idx++)
			{
				ret = mapMove(binBeg, idx);
				MAPGET(binName, cardName, lblKO);
				if(*cardName == 0)
					continue;

				MAPGETWORD(binTotAppDbCnt, count, lblKO);
				MAPGETWORD(binTotAppCrCnt, countChk1, lblKO);
				MAPGETWORD(binTotAppTpCnt, countChk2, lblKO);
				MAPGETWORD(binTotAppVoidCnt, countChk3, lblKO);
				MAPGETWORD(binTotAppManualCnt, countChk4, lblKO);
				MAPGETWORD(binTotAppRefundCnt, countChk5, lblKO);
				MAPGETWORD(binTotAppPrecomCnt, countChk6, lblKO);
				MAPGETWORD(binTotAppVoidPrecomCnt, countChk7, lblKO);

				if(count == 0 && countChk1 == 0 && countChk2 == 0 && countChk3 == 0 && countChk4 == 0  && countChk5 != 0
					&& countChk6 != 0 && countChk7 != 0)
					continue;

				// Print Card Name
				ret = rptReceipt(rloSetCardName);
				CHECK(ret >= 0, lblKO);

				/*
				// Print Tip Card Totals
				MAPGET(binTotAppTpAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, countChk2, lblKO);
				ret = rptReceipt(rloTrxLogTotalsTip);
				CHECK(ret >= 0, lblKO);
				*/
				/*
				// Print Header
				ret = rptReceipt(rloRptHeaderLog);
				CHECK(ret >= 0, lblKO);

				// Print Precom Sale Totals
				MAPGET(binTotAppPrecomAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Refund Sale Totals
				MAPGET(binTotAppRefundAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsRefund);
				CHECK(ret >= 0, lblKO);

				// Print Manual Totals
				MAPGET(binTotAppManualAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				//ret = rptReceipt(rloTrxLogTotalsManual);
				//CHECK(ret >= 0, lblKO);


				// Print Void Sale Totals
				MAPGET(binTotAppVoidAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoid);
				CHECK(ret >= 0, lblKO);

				// Print Void Precom Totals
				MAPGET(binTotAppVoidPrecomAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
				CHECK(ret >= 0, lblKO);

				// Print Void Refund Totals
				MAPGET(binTotAppVoidRefundAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				ret = rptReceipt(rloTrxLogTotalsVoidRefund);
				CHECK(ret >= 0, lblKO);

				// Print Void Manual Totals
				MAPGET(binTotAppVoidManualAmt, amountBuf, lblKO);
				MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
				MAPPUTWORD(traLogTotalCount, count, lblKO);
				//ret = rptReceipt(rloTrxLogTotalsVoidManual);
				//CHECK(ret >= 0, lblKO);


			}
		}
		*/
		ret = rptReceipt(rloSep);
		CHECK(ret >= 0, lblKO);
	}
    rptReceipt(rloFeed);
	//return to recent acquirer
//	mapMove(acqBeg, (word) (acqBeforeCalc - 1));  @agmr - tutup - dapat menimbulkan processing error // minus 1 is inserted because mapmove starts at index 0 while acqId is at 1

	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblLogEmpty:
	usrInfo(infLogEmpty);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("logHostTotals: ret=%d \n", ret);
}

/** This function prints the daily totals regarding the transactions saved inside the batch after closing it.
 * The ticket shows the totals from the terminal and the totals from the host if the reconciliation failed.
 * Note that this logic is for single acquirer only.
 * \header log\\log.h
 * \source log\\log.c
 */
void logBatchTotals(void)
{
	int ret, counter, idy;
	char amountBuf[lenTotAmt + 1], acqCurName[lenChdNam + 1];
	card count;
	word dim;
//	byte acqBeforeCalc; @agmr - tutup

	trcS("logBatchTotals Beg\n");

	MAPGETWORD(regTrxNo, dim, lblKO);
	CHECK(dim > 0, lblLogEmpty);

	strFormatDatTim(traDatTim);

	// Printing Receipt Header
	ret = usrInfo(infConfigPrinting);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloHeader);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloBatTotHdr);
	CHECK(ret >= 0, lblKO);

	//get current acqID
//	MAPGETBYTE(acqID, acqBeforeCalc, lblKO); @agmr - tutup- dapat menimbulkan processing error

	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, acqCurName, lblKO);
		if(*acqCurName == 0)
			continue;
////++BRI9
	    if(memcmp(acqCurName,"MINI ATM",8) == 0 ||
	    		memcmp(acqCurName,"TUNAI",5) == 0 ||
	    		memcmp(acqCurName,"BRIZZI",6) == 0 ||
	    		memcmp(acqCurName,"DEBITSINAR",10) == 0
	       )
	       continue;
////--BRI9

		// Calculate Acquirer Totals
		ret = logCalculateTotalsByCardTypeAndCurAcq();
		CHECK(ret >= 0, lblKO);

		// Print Sale Totals
		MAPGET(acqTotAppDbAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppDbCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		//ret = rptReceipt(rloTrxLogTotalsSale);
		//CHECK(ret >= 0, lblKO);

		// Print Precom Totals
		MAPGET(acqTotAppPrecomAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppPrecomCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		//ret = rptReceipt(rloTrxLogTotalsPrecom);
		//CHECK(ret >= 0, lblKO);

		// Print Refund Totals
		MAPGET(acqTotAppRefundAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppRefundCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		//ret = rptReceipt(rloTrxLogTotalsRefund);
		//CHECK(ret >= 0, lblKO);

		/*
		// Print Tip Totals
		MAPGET(acqTotAppTpAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppTpCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsTip);
		CHECK(ret >= 0, lblKO);
		*/

		///* //FOR MULTI ACQUIRER LAYOUT - DO NOT DELETE
		// Print the Card Layout
		//ret = rptReceipt(rloTotalsByHost);
		//CHECK(ret >= 0, lblKO);
	}

	rptReceipt(rloSetTotalsByCardWoSep);

	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, acqCurName, lblKO);
		if(*acqCurName == 0)
			continue;

////++BRI9
	    if(memcmp(acqCurName,"MINI ATM",8) == 0 ||
	    		memcmp(acqCurName,"TUNAI",5) == 0 ||
	    		memcmp(acqCurName,"DEBITSINAR",10) == 0
	       )
	       continue;
////--BRI9

		ret = rptReceipt(rloHostTotAcqDet);
		CHECK(ret >= 0, lblKO);

		/*
		// Print Sale Totals
		MAPGET(acqTotAppDbAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppDbCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsSale);
		CHECK(ret >= 0, lblKO);

		// Print Precom Totals
		MAPGET(acqTotAppPrecomAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppPrecomCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsPrecom);
		CHECK(ret >= 0, lblKO);


		// Print Refund Totals
		MAPGET(acqTotAppRefundAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppRefundCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsRefund);
		CHECK(ret >= 0, lblKO);

		// Print Void Totals
		MAPGET(acqTotAppVoidAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppVoidCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsVoid);
		CHECK(ret >= 0, lblKO);

		// Print Void Precom Totals
		MAPGET(acqTotAppVoidPrecomAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppVoidPrecomCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
		CHECK(ret >= 0, lblKO);

		// Print Void Refund Totals
		MAPGET(acqTotAppVoidRefundAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppVoidRefundCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsVoidRefund);
		CHECK(ret >= 0, lblKO);



		/*
		// Print Tip Totals
		MAPGET(acqTotAppTpAmt, amountBuf, lblKO);
		MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
		MAPGETWORD(acqTotAppTpCnt, count, lblKO);
		MAPPUTWORD(traLogTotalCount, count, lblKO);
		ret = rptReceipt(rloTrxLogTotalsTip);
		CHECK(ret >= 0, lblKO);
		*/
		for(idy=0; idy<dimIss;idy++)
		{
			switch(idy)
			{
				case 1:
					ret = rptReceipt(rloSetIssDetDb);
					CHECK(ret >= 0, lblKO);

					// Print Header
					ret = rptReceipt(rloRptHeaderLog);
					CHECK(ret >= 0, lblKO);

					// Print Sale Totals
					MAPGET(issTotAppDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsSale);
					CHECK(ret >= 0, lblKO);

					// Print Precom Totals
					MAPGET(issTotAppPrecomDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppPrecomDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Refund Totals
					MAPGET(issTotAppRefundDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppRefundDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsRefund);
					CHECK(ret >= 0, lblKO);

					// Print Void Totals
					MAPGET(issTotAppVoidDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoid);
					CHECK(ret >= 0, lblKO);

					// Print Void Precom Totals
					MAPGET(issTotAppVoidPrecomDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidPrecomDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Void Refund Totals
					MAPGET(issTotAppVoidRefundDbAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidRefundDbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidRefund);
					CHECK(ret >= 0, lblKO);

					break;

				case 0:
					ret = rptReceipt(rloSetIssDetCr);
					CHECK(ret >= 0, lblKO);

					// Print Header
					ret = rptReceipt(rloRptHeaderLog);
					CHECK(ret >= 0, lblKO);

					// Print Sale Totals
					MAPGET(issTotAppCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsSale);
					CHECK(ret >= 0, lblKO);

					// Print Precom Totals
					MAPGET(issTotAppPrecomCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppPrecomCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Refund Totals
					MAPGET(issTotAppRefundCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppRefundCrbCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsRefund);
					CHECK(ret >= 0, lblKO);

					// Print Void Totals
					MAPGET(issTotAppVoidCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoid);
					CHECK(ret >= 0, lblKO);

					// Print Void Precom Totals
					MAPGET(issTotAppVoidPrecomCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidPrecomCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidPrecom);
					CHECK(ret >= 0, lblKO);

					// Print Void Refund Totals
					MAPGET(issTotAppVoidRefundCrAmt, amountBuf, lblKO);
					MAPPUTSTR(traLogTotalAmount, amountBuf, lblKO);
					MAPGETWORD(issTotAppVoidRefundCrCnt, count, lblKO);
					MAPPUTWORD(traLogTotalCount, count, lblKO);
					ret = rptReceipt(rloTrxLogTotalsVoidRefund);
					CHECK(ret >= 0, lblKO);

					break;
			}
		}

		ret = rptReceipt(rloSep);
		CHECK(ret >= 0, lblKO);
	}

	//*/

	//return to recent acquirer
//	mapMove(acqBeg, (word) (acqBeforeCalc - 1)); @agmr - tutup- dapat menimbulkan processing error   // minus 1 is inserted because mapmove starts at index 0 while acqId is at 1

	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblLogEmpty:
	usrInfo(infLogEmpty);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("logBatchTotals: ret=%d \n", ret);
}

//++ @agmr
#ifdef PREPAID
int pmtPrepaidSettlement()
{
	int ret;
	word mnuItem;
	byte acqCnt;
	char acqCurName[lenChdNam + 1];
	char currBat[lenBatNum + 10];
	char datetime[lenDatTim + 1];
	word idx, trxNum;
//	char buf[lenMnu + 1];
	byte temp; //@agmr
    byte revFlag, isEmpty=0;

	trcS("logSettlement Beg\n");

	MAPGETWORD(traMnuItm, mnuItem, lblKO);
	ret = valAskPwd(appSetlPwd);
	CHECK((ret > 0) && (ret != kbdANN), lblKO);
	MAPGETBYTE(regAcqCnt, acqCnt, lblKO);

	memset(uploadedPrepaidIdx,0,sizeof(uploadedPrepaidIdx));

#ifdef TEST_PREPAID_SETTLEMENT
{//hapus - test reversal
    char key;

    dspClear();
    dspLS(2,"RESET FLAG??");
	key = acceptable_key(kbdVAL_kbdANN, 1000);
	if(key == kbdVAL)
	{
	    resetPrepaidUploadedFlag();
	}
}
#endif


//++ @agmr - multi settle
	ret = batchSync();
	if(ret < 0)
	    goto lblKO;
//-- @agmr - multi settle

    for(idx=0;idx<dimAcq;idx++)
    {
        mapMove(acqBeg, idx);
        MAPGET(acqName, acqCurName, lblKO);
        if(memcmp(acqCurName,"BRIZZI",6) == 0)
            break;
    }
    CHECK(idx<dimAcq,lblNoTran);

    MAPGETBYTE(acqID, temp, lblKO);
    MAPPUTBYTE(appAcqSettlementIdx,temp-1,lblKO);

    ret = getBriReversalIdx();
    CHECK(ret>0,lblKO);

	MAPGETWORD(regTrxNo, trxNum, lblKO);
	MAPGETBYTE(revReversalFlag, revFlag, lblKO);

	if(trxNum == 0 && revFlag == 0)
        goto lblBatchEmpty;

//	ret = logConfirmTotals();
//	CHECK(ret > 0, lblKO);

	ret = logCalcTot();
	CHK;

	if(ret == 0)
        isEmpty = 1;
    else
        isEmpty = 0;

	if(isEmpty > 0 && revFlag==0) //tidak ada trans untuk acq ini
	{
	    goto lblNoTran;
	}
//-- @agmr
	MAPPUTSTR(traTransType, "PREPAID SETTLEMENT", lblKO);
	MAPPUTBYTE(traTxnType, trtPrepaidSettlement, lblKO);
    MAPPUTBYTE(acqIsPendingSettlement, 1, lblKO);
    MAPPUTBYTE(regLocType, 'T', lblEnd);

//++ @agmr - supaya habis reversal tidak kirim msg 500
    if(isEmpty>0)
    {
       MAPPUTWORD(traMnuItm, 0, lblKO);
       MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
    }
//-- @agmr - supaya habis reversal tidak kirim msg 500

    MAPPUTWORD(traIdx, 1, lblKO);
	ret = onlSession();
	CHECK(ret >= 0, lblKO);

	MAPGET(acqCurBat, currBat, lblKO);
	MAPPUTSTR(totBatNum, currBat, lblKO);
	MAPGET(traDatTim, datetime, lblKO);
	MAPPUTSTR(totDatTim, datetime, lblKO);
	strFormatDatTim(traDatTim);

    if(isEmpty == 2)
        goto lblBatchEmpty;
    else if (isEmpty == 1)
        goto lblNoTran;

//	logPrintLog();
	prepaidSettlementReceipt();
	prepaidBatchIncrement();

//++ @agmr - multi settle
#ifndef TEST_PREPAID_SETTLEMENT
    MAPGETBYTE(acqID, temp, lblKO); 	 //NANTI DIBUKA !!!
	ret = briLogReset(temp);
	if(ret < 0)
	{
    	ret = batchSync();
    	if(ret == -100)
    	    goto lblKO;
	    goto lblBatchError;
	}
#endif
//-- @agmr - multi settle

    MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
    memset(uploadedPrepaidIdx,0,sizeof(uploadedPrepaidIdx)); //@agmr

	goto lblEnd;
lblBatchError:
//    logSettlementConfirm(0);
    usrInfo(infBatchError);
    goto lblEnd;
lblBatchEmpty:
    MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
    usrInfo(infLogEmpty);
    goto lblEnd;
lblNoTran:
    MAPPUTBYTE(acqIsPendingSettlement, 0, lblKO); //@agmr
	usrInfo(infNoTran);
	goto lblEnd;
lblKO:
	return -1;
lblEnd:
	return 1;
	trcS("logSettlement End\n");
}
#endif

#ifdef PREPAID
int setLogIsUploaded()
{
	int ret;
	int i;

	for(i=0;i<uploadedPrepaidIdx[0];i++)
	{
	    if(i == MAX_PREPAID_SETTLEMENT)
	        break;

	    ret = mapMove(logBeg,uploadedPrepaidIdx[i+1]);
	    MAPPUTBYTE(logIsUploaded,1,lblKO);
	}
    memset(uploadedPrepaidIdx,0,sizeof(uploadedPrepaidIdx));
    return 1;
lblKO:
    return -1;
}
#endif
//-- @agmr

//++BRI9
void printAllLog()
{
	int ret;
	word dim;
	char acqCurName[lenChdNam + 1];
	int counter;

	MAPGETWORD(regTrxNo, dim, lblEnd);
	CHECK(dim > 0, lblLogEmpty);

	trcS("printAllLog Beg\n");
	for (counter = 0; counter < dimAcq; counter++)
	{
	    	mapMove(acqBeg, counter);
	    	memset(acqCurName,0,sizeof(acqCurName));
        	MAPGET(acqName, acqCurName,lblEnd);

    		if(*acqCurName == 0)
            		continue;

        	if(memcmp(acqCurName,"MINI ATM",8 ) == 0 ||
        		memcmp(acqCurName,"TUNAI",5) == 0 ||
        		memcmp(acqCurName,"DEBITSINAR",10) == 0
           	)
            		continue;

		logPrintLog();
	}
	goto lblEnd;

lblLogEmpty:
	usrInfo(infLogEmpty);
	ret = 0;
	goto lblEnd;

lblEnd:
	trcS("printAllLog End\n");
}


void printAllSetlleDuplicate()
{
	int ret;
//	word dim;
	char acqCurName[lenChdNam + 1];
	int counter;

//	MAPGETWORD(regTrxNo, dim, lblEnd);
//	CHECK(dim > 0, lblLogEmpty);

	trcS("printAllLog Beg\n");
	for (counter = 0; counter < dimAcq; counter++)
	{
	    mapMove(acqBeg, counter);
	    memset(acqCurName,0,sizeof(acqCurName));
        MAPGET(acqName, acqCurName,lblEnd);

    	if(*acqCurName == 0)
            continue;

        if(memcmp(acqCurName,"MINI ATM",8 ) == 0 ||
        		memcmp(acqCurName,"TUNAI",5) == 0 ||
        		memcmp(acqCurName,"DEBITSINAR",10) == 0
           )
            continue;

    	setDuplicate = 1;
    	logSettlementRpt();

	}
	rptReceipt(rloFeed);

lblEnd:
	trcS("printAllLog End\n");
}



//--BRI9

