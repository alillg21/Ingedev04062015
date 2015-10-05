//Subversion reference
//$HeadURL:  $
//$Id:  $

#include <string.h>
#include "log.h"
#include "bri.h" //@agmr - BRI

static word revKey[revEnd - revBeg - 1] = {
            revReversalFlag,            ///< revReversalFlag , just get existing reversalFlag
            rqsPrcCod,                  ///< revPrcCod , processing code
            regSTAN,                    ///< revSTAN , system trace audit number
            regInvNum,                  ///< revInvNum , processing code
            traDatTim,                  ///< revDatTim , transaction date time
            traAmt,                     ///< revAmt , transaction amount
            traEntMod,                  ///< revEntMod , entry mode
            traConCode,                 ///< revConCod , condition code
            traPinBlk,                  ///< revPinBlk , PIN block
            traAcqIdx,                  ///< revTID , transaction id
            traTrk2,                    ///< revTrk2 , track 2
            traTipAmt,                  ///< revTipAmt , tip amount
            traEmvICC,		            //revICCData, bit 55
            acqCurBat,                  ///< revBatNum , batch number
            traBinIdx,                  ///< revBinIdx , index in bin table
            traIssIdx,                   ///< revIssIdx , index in iss table
            traField48,                 //@agmr - BRI
            traField48Len,
            traField63,                  //@agmr - BRI
            traField63Len,
            traField57,                  //@agmr - BRI
            traField57Len,            
            traRevNII,
            
// ++@agmr - BRI10          
            traTxnType,
            traPan,
            traExpDat
// --@agmr - BRI10              
        };


/** Save reversal data into database
 * \header log\\log.h
 * \source log\\rev.c
 */
int revSave(void)
{
	word key;
	byte idx;
	byte buf[512];
	int ret;

	// Save transaction from tra record into reversal buffer
	trcS("revSave Beg\n");

//++ @agmr - BRI3
#ifdef REVERSAL_TERPISAH
    MAPGETBYTE(traAcqIdx,idx,lblDBA); 
    mapMove(revBeg, idx);
#else
    ret = getBriReversalIdx();
    if(ret <=0)
        goto lblDBA;
#endif        
//-- @agmr - BRI3
    
//	ret = mapMove(revBeg, traAcqIdx);
//	for (key = revBeg + 1; key < (revEnd - 1); key++)
	for (key = revBeg + 1; key < revEnd; key++)  //@agmr - BRI
	{ // Loop around each field from the record of the "log" table
		idx = (byte) (key - revBeg - 1);    //index within logKey array
		VERIFY(isSorted(traBeg, revKey[idx], traEnd)
		       || isSorted(regBeg, revKey[idx], regEnd) //all the keys in logKey array can only be from tra or reg records
		       || isSorted(rqsBeg, revKey[idx], rqsEnd)
		       || isSorted(revBeg, revKey[idx], revEnd)
		       || isSorted(acqBeg, revKey[idx], acqEnd));
		MAPGET(revKey[idx], buf, lblDBA);
		ret = mapPut(key, buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}

	ret = 1;
	goto lblEnd;

lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("revSave: ret=%d \n", ret);
	return ret;
}

/** Contains the reversal transaction flow. For manual reversal.
 * \header log\\log.h
 * \source log\\rev.c
 */
int reversalTransaction(void)
{
	byte flag;
	int ret;

	mapMove(revBeg, traAcqIdx);
	MAPGETBYTE(revReversalFlag, flag, lblNoReversal);
	if(flag == 1)
	{
		MAPPUTWORD(traMnuItm, mnuReversal, lblKO);
		ret = onlSession();
		CHECK(ret >= 0, lblKO);
		MAPPUTBYTE(revReversalFlag, flag, lblKO)
	}
	ret = 1;

lblNoReversal:
	ret = 0;
	goto lblEnd;
lblKO:
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("End saveReversalData ret=%d\n", ret);
	return ret;
}

static word tcKey[tclogEnd - tclogBeg - 1] = {
            TCFlag,            ///< revReversalFlag , just get existing reversalFlag
            traMti,
            traPan,
            regSTAN,                    ///< revSTAN , system trace audit number
            regInvNum,                  ///< revInvNum , processing code
            traDatTim,                  ///< revDatTim , transaction date time
            traAmt,                     ///< revAmt , transaction amount
            traRspBit13,
            traRspBit12,
            traExpDat,
            traEntMod,                  ///< revEntMod , entry mode
            traPanSeq,
            traRrn,
            traAutCod,
            traRspCod,
            traTrk2,                    ///< revTrk2 , track 2
            traTipAmt,                  ///< revTipAmt , tip amount
            traEmvICC,
            TCEmvLen
       };


int tcSave(void)
{
	word key;
	byte idx;
	byte buf[256];
	int ret;

	// Save transaction from tra record into tc buffer
	trcS("tcSave Beg\n");

	for (key = tclogBeg + 1; key < (tclogEnd - 1); key++)
	{ // Loop around each field from the record of the "log" table
		idx = (byte) (key - tclogBeg - 1);    //index within logKey array
		VERIFY(isSorted(traBeg, tcKey[idx], traEnd)
		       || isSorted(regBeg, tcKey[idx], regEnd) //all the keys in logKey array can only be from tra or reg records
		       || isSorted(rqsBeg, tcKey[idx], rqsEnd)
		       || isSorted(tclogBeg, tcKey[idx], tclogEnd)
		       || isSorted(acqBeg, tcKey[idx], acqEnd));
		MAPGET(tcKey[idx], buf, lblDBA);
		ret = mapPut(key, buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}

	ret = 1;
	goto lblEnd;

lblDBA:                      // Data base access error encountered
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("tcSave: ret=%d \n", ret);
	return ret;
}

