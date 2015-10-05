//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/emv.c $
//$Id: emv.c 1917 2009-04-22 13:24:29Z abarantsev $

/** \file
 * Financial (payment) operations transaction flow is implemented here
 *
 * All of them follow the return code convention:
 *    - >0 : operation OK, continue
 *    -  0 : user cancel or timeout: smooth exit
 *    - <0 : irrecoverable error, stop processing anyway
 */

#include <string.h>
#include "log.h"
#include "emv.h"
#include "bri.h"


byte goOnline = 0;

//#ifdef __EMV__
#if 1
/** Transfer to EMV Kernel Selection module the list of AIDs accepted by the application.
 * Retrieve all records from "aid" table, then fills a queue by AID values to build the argument of emvInit() wrapper.
 * \header log\\log.h
 * \source log\\emv.c
 */
int emvInitAid(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	word idx;
	byte AID[1 + lenAID];       //reserve space for single AID
	byte dAID[(lenAID + 4) * 16];   //reserve space for up to 16 AIDs
	tBuffer bAID;               //bufer for up to 16 AIDs
	tQueue qAID;                //queue for up to 16 AIDs
	int dim;                    //number of AIDs in the queue que

	trcS("emvInitAid Beg\n");
	bufInit(&bAID, dAID, sizeof(dAID));
	queInit(&qAID, &bAID);
	for (idx = 0; idx < dimAID; idx++)
	{    //fill the queue by application AIDs
		mapMove(aidBeg, idx);
		MAPGET(emvAid, AID, lblDBA);
		if(AID[0] == 0)
		{       // AID disabled
			continue;
		}
		ret = quePut(&qAID, AID + 1, *AID);
		CHECK(ret == AID[0], lblRAM);
	}

	dim = queLen(&qAID);        //number of AIDs announced by the terminal application
	CHECK(dim > 0, lblNotInitialized);

	ret = emvInit(&qAID);       // Send the list of AIDs to EMV Kernel
	CHECK(ret == queLen(&qAID), lblKO);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblRAM:
	inf = infMemoryError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblNotInitialized:
	inf = infNotInitialized;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvInitAid: ret=%d\n", ret);
	return ret;
}

/** Check if the AID selected is part of the AID(s) list accepted by the application.
 * If the AID is accepted, the cursor points to the right record from the aid table.
 * \param    aid (I)  EMV AID selected.
 * \header log\\log.h
 * \source log\\emv.c
 */
int emvMapAidMove(const byte * aid)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	word idx;                   // The index of application selected within the list
	char tmp[1 + lenAID];

	VERIFY(aid);
	trcS("emvMapAidMove Beg\n");

	for (idx = 0; idx < dimAID; idx++)
	{    // Check the AID selected from the AID(s) list
		mapMove(aidBeg, idx);
		MAPGET(emvAid, tmp, lblDBA);
		if(memcmp(&tmp[1], aid + 1, tmp[0]) == 0)   // Compare with the AID selected
			break;
	}
	CHECK(idx < dimAID, lblAidNotFound);    //if at the end of AID table, AID not found

	ret = idx;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblAidNotFound:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvMapAidMove: ret=%d\n", ret);
	return ret;
}

/** Show various candidates (AIDs) present in the list.
 * Cardholder selects the candidate (AID) from the menu to continue the transaction.
 * If the cardholder does not select any AID, the terminal terminates the transaction.
 * \param que (I)  The list of candidates (AIDs).
 * \source log\\emv.c
 */
static int emvAppSelMnu(tQueue * que)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	char mnu[MNUMAX][dspW + 1]; // The final menu array prepared to mnuSelect
	char *itm[MNUMAX];          // Array of pointers to mnu items
	byte idx;                   // Index within ptr array
	byte aid[1 + lenAID];       // Application Identifier
	byte AppNam[lenAppNam + 1]; // Application Preferred Name
	byte pri;                   // Application Priority Indicator

	VERIFY(que);
	trcS("emvAppSelMnu Beg\n");
	idx = 1;
	memset(itm, 0, sizeof(itm));    // Reset array of pointers to mnu items
	MAPGET(msgApplicationSelect, mnu[0], lblDBA);   // Menu title
	itm[0] = mnu[0];
	while(queLen(que))
	{        // Retrieve from queue the candidate application names
		ret = queNxt(que);
		VERIFY(ret <= lenAID);
		aid[0] = (byte) ret;    // Length of AID
		ret = queGet(que, &aid[1]); // Retrieve Application Identifier (AID)
		VERIFY(ret >= 0);

		VERIFY(queNxt(que) <= lenAppNam + 1);
		memset(AppNam, 0, lenAppNam + 1);
		ret = queGet(que, AppNam);  // Retrieve Application Preferred Name
		VERIFY(ret >= 0);

		VERIFY(queNxt(que) == 1);
		ret = queGet(que, &pri);    // Retrieve Application Priority Indicator
		VERIFY(ret >= 0);

		VERIFY(strlen((char *) AppNam) <= dspW);
		VERIFY(idx < MNUMAX);
		strcpy(mnu[idx], (char *) AppNam);  // Put Application Preferred Name into menu array
		itm[idx] = mnu[idx];
		idx++;
	}

	ret = mnuSelect((Pchar *) itm, 0, 60);  // Perform user dialog, to display menu items on screen with the array of pointer
	CHECK(ret >= 0, lblKO);

	if(!ret)
	{                  // Timeout or aborted - nothing to do
		ret = idx - 1;          // Index out of range means that nothing is selected
		goto lblEnd;
	}

	ret %= MNUMAX;              // Extract the item number selected from menu state
	ret--;                      // Decrement it since zero item is menu title
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	queRewind(que);             // rewind to to the beginning of the queue
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppSelMnu: ret=%d\n", ret);
	return ret;
}

/** Process the Application Selection to determine which one of the applications that are supported by both the card
 * and terminal will be used to conduct the transaction.
 * This process takes place in 2 steps:
 *  - The terminal builds a candidate list of mutually supported  applications (AIDs).
 *  - A single application (AID) from this list is identified and selected to process the transaction.
 * \header log\\log.h
 * \source log\\emv.c
 */
int emvApplicationSelect(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte AID[1 + lenAID];       // Application Identifier
	char AppNam[lenAppNam + 1];
	byte AppPrfNam[1 + lenAppPrfNam];
	byte pri;                   // Application Priority Indicator
	byte dat[256];
	tBuffer buf;
	tQueue que;
	byte idx, dim;
	word pbm;
	byte cardholderConfirm = 0;

	trcS("emvApplicationSelect Beg\n");
	ret = usrInfo(infApplicationSelect);
	CHECK(ret >= 0, lblKO);
	bufInit(&buf, dat, sizeof(dat));
	queInit(&que, &buf);

	ret = emvSelect(&que);      // Ask EMV Kernel to prepare a candidate list
	switch (ret)
	{
		case (-eEmvErrCardBlock):
			goto lblCardBlock;
		default:
			break;
	}
	CHECK(ret >= 0, lblKO);
	VERIFY(queLen(&que) % 3 == 0);
	VERIFY(ret < 256);
	dim = (byte) ret;
	switch (dim)
{
		case 0:
			emvSelectStatus(&pbm);
          if(pbm == 0  || pbm == 0x6A82) {
              emvSetFallBack(1);    // Fallback Set
              goto lblFallBack;
          } else if(pbm == 0x6283) {    // Application Blocked
              goto lblAppBlock;
          } else if(pbm == 0x6A81) {    // Card Blocked
              goto lblCardBlock;
          } else {              // No candidate into the list?
              goto lblListEmpty;
          }
			break;
		case 1:                  // Only one candidate into the list
			idx = 0;
			cardholderConfirm = 1;
			break;
		default:                 // Several candidates into the list: ADVTK 22
			ret = emvAppSelMnu(&que); // Build the menu and ask Cardholder to select
			CHECK(ret >= 0, lblKO);
			VERIFY(ret <= dim);
			if(ret == dim)
			{      // Nothing is selected or Timeout
				ret = 0;
				goto lblEnd;
			}
			idx = (byte) ret;
			break;
	}

	do
	{                        // Identifying the candidate application (AID): Retrieve the related AID, PreferedName and Priority from queue
		ret = queNxt(&que);
		VERIFY(ret <= lenAID);
		AID[0] = (byte) ret;    // Length of AID
		ret = queGet(&que, &AID[1]);    // Retrieve Application Identifier (AID)
		VERIFY(ret >= 0);

		VERIFY(queNxt(&que) <= lenAppNam + 1);
		memset(AppNam, 0, lenAppNam + 1);
		ret = queGet(&que, (byte *) AppNam);    // Retrieve candidate Application Name
		VERIFY(ret >= 0);

		VERIFY(queNxt(&que) == 1);
		ret = queGet(&que, &pri);   // Retrieve Application Priority Indicator
		VERIFY(ret >= 0);
	}
	while(idx--);

	if((pri & 0x80) && (cardholderConfirm))
	{   //Cardholder Confirmation (Confirmation required: ADVTK 11)
		ret = dspClear();
		CHECK(ret >= 0, lblKO);
		MAPPUTSTR(msgVar, AppNam, lblKO);
		usrInfo(infVarLin0);    // Display Application Preferred Name
		ret = usrInfo(infApplicationConfirm);
		CHECK(ret >= 0, lblKO); // Ask for user to confirm
		if(ret != kbdVAL)
		{     // If not, the tansaction is aborted
			ret = 0;
			goto lblEnd;
		}
	}

	trcFS("AppNam=%s\n", AppNam);
	*AppPrfNam = strlen(AppNam);    //EMV Application Preferred Name (Tag 9F12 => Length + Binary data)
	VERIFY(*AppPrfNam <= lenAppPrfNam);
	memcpy(AppPrfNam + 1, AppNam, *AppPrfNam);
	ret = mapPut(emvAppPrfNam, AppPrfNam + 1, (byte) (1 + AppPrfNam[0]));
	CHECK(ret >= 0, lblKO);

	//ADDED, To get AID
	//ret = mapPut(emvDFNam, AID, (byte) (1 + AID[0]));
	ret = mapPut(emvDFNam, AID, 1+lenDFNam);
       CHECK(ret >= 0, lblKO);

	ret = emvFinalSelect(AID);  // Initiate the FinalSelect from the candidate application chosen
	switch (ret)
	{
		case (-eEmvErrReselect):
			goto lblFallBack;
		default:
			break;
	}
	CHECK(ret >= 0, lblKO);
	ret = emvMapAidMove(AID);   // Check if the AID is part of the AID(s) application's table
	CHECK(ret >= 0, lblKO);

	ret = 1;
	goto lblEnd;

lblFallBack:
	inf = infFallBack;
	ret = 0;
	goto lblEnd;
lblAppBlock:
	inf = infEmvAppBlock;
	ret = -1;
	goto lblEnd;
lblCardBlock:
	inf = infEmvCrdBlock;
	ret = -1;
	goto lblEnd;
lblListEmpty:
	inf = infAidListEmpty;
	ret = 0;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvApplicationSelect: ret=%d\n", ret);
	return ret;
}

/** Perform the amount input, and ensure that the EMV amount tags are filled.
 * \source log\\pmt.c
 */
static int emvAmountInput(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte AmtNum[1 + lenAmtNum];
	char Amt[lenAmt + 1];
	card AmtNat;
	byte AmtBin[1 + lenAmtBin];
	char TipAmt[lenAmt + 1];
       char AmtTot[lenAmt + 1];
	byte AmtExp;
	//char AmtTot[lenAmt + 1];
#ifdef _USE_ECR
	char buf[25];
	byte appIsECRbyte;
#endif


	trcS("emvAmountInput: Beg\n");

	mapPut(emvAmtOthNum, (byte *) "\x06\x00\x00\x00\x00\x00\x00", 1 + lenAmtNum);
    mapPut(emvAmtOthBin, (byte*) "\x04\x00\x00\x00\x00", 1+4);
    memset(AmtTot, 0, sizeof AmtTot);
	#ifdef _USE_ECR
		MAPGETBYTE(appIsECR,appIsECRbyte,lblKO);
		if(appIsECRbyte == 1){
			memset(buf,0,sizeof(buf));
			mapGet(appAmtECR,buf,sizeof(buf));
			MAPPUTSTR(traAmt, buf, lblKO);
		}
	#endif
		MAPGET(traAmt, Amt, lblDBA);
	if(Amt[0] == 0) {
		ret = pmtAmountInput();
		CHECK(ret > 0, lblKO);
	}

	ret = pmtTipInput();        // Enter Tip amount
	CHECK(ret > 0, lblKO);

    usrInfo(infProcessing); //@agmr

	MAPGET(traAmt, Amt, lblDBA);
	MAPGET(traTipAmt, TipAmt, lblKO);
	MAPPUTSTR(emvAppTraAmt, Amt, lblKO);
	addStr(AmtTot, Amt, TipAmt);
	MAPPUTSTR(traTotAmt, AmtTot, lblKO);
	MAPGETBYTE(appExp, AmtExp, lblKO);
       if(AmtExp == 0){
	     fmtPad(AmtTot, -10, '0');
	     memcpy(AmtTot + 10, "00", 2);
       }
	else
            fmtPad(AmtTot, -lenAmt, '0');

	AmtNum[0] = lenAmtNum;
	hex2bin(&AmtNum[1], AmtTot, lenAmtNum);
	ret = mapPut(emvAmtNum, AmtNum, 1 + lenAmtNum);
	CHECK(ret >= 0, lblDBA);

	AmtBin[0] = lenAmtBin;
	dec2num(&AmtNat, AmtTot, 0);
	num2bin(&AmtBin[1], AmtNat, lenAmtBin);
	ret = mapPut(emvAmtBin, AmtBin, 1 + lenAmtBin);
	CHECK(ret >= 0, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
//++@agmr
    if(ret == -100)
        goto lblEnd;
//--@agmr
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAmountInput: ret=%d\n", ret);
	return ret;
}

/** Insert the tag inside the queue.
 * \param    qT (O)    Queue filled with tags.
 * \param    lTag (I)  Tag to insert.
 * \source log\\emv.c
 */
static void putTagQue(tQueue * qT, long lTag)
{
	int ret;
	word dim;
	card tmp;

	trcS("putTagQue: Beg\n");
	VERIFY(qT);

	if(lTag < 0)
	{
		lTag = -lTag;
		queRewind(qT);
		dim = queLen(qT);
		while(dim--)
		{
			ret = queGet(qT, (byte *) & tmp);
			VERIFY(ret == sizeof(card));
			if(tmp == (card) lTag)
			{
				lTag = 0;
			}
		}
	}

	if(!lTag)
	{
		return;
	}
	ret = quePut(qT, (byte *) & lTag, sizeof(lTag));
	VERIFY(ret >= 0);
}

/** Retrieve all tag values qT from data base and puts them into qV.
 * \param    qV (O)  Queue containing TLVs built from data base.
 * \param    qT (I)  Queue containing tags used to build the correspondant TLVs.
 * \source log\\emv.c
 */
static int getEmvQue(tQueue * qV, tQueue * qT)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	card tag;
	word key;
	byte Buf[1 + 256];

	trcS("getEmvQue: Beg\n");
	VERIFY(qV);
	VERIFY(qT);

	trcS("getEmvQue qT: ");
	trcQueDol(qT);
	trcS("\n");

	queReset(qV);
	queRewind(qT);
	while(queLen(qT))
	{
		VERIFY(queNxt(qT) == sizeof(card));
		ret = queGet(qT, (byte *) & tag);
		VERIFY(ret == sizeof(card));
		key = mapKeyTag(tag);
		if(!key)
		{
			continue;
		}
		MAPGET(key, Buf, lblDBA);
		if(!Buf[0])
		{
			continue;
		}
		ret = quePut(qV, (byte *) & tag, sizeof(card));
		VERIFY(ret == sizeof(card));
		ret = quePut(qV, &Buf[1], Buf[0]);
		VERIFY(ret == Buf[0]);
	}
	trcS("queEmv qV:\n");
	trcQueTlv(qV);

	ret = queLen(qV);
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = -1;
	}
	trcFN("getEmvQue: ret=%d\n", ret);
	return ret;
}

/** Save all tag values qV into data base.
 * \param    qV (I)  Queue containing TLVs to be saved into data base.
 * \source log\\emv.c
 */
static int putEmvQue(tQueue * qV)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	card tag;
	byte len;
	word key;
	byte Buf[1 + 256];

	// Save TLVs into data base
	trcS("putEmvQue: Beg\n");
	VERIFY(qV);

	trcS("putEmv que:\n");
	trcQueTlv(qV);

	queRewind(qV);
	while(queLen(qV))
	{
		VERIFY(queNxt(qV) == sizeof(card));
		ret = queGet(qV, (byte *) & tag);
		VERIFY(ret == sizeof(card));
		VERIFY(queNxt(qV) <= 256);
		ret = queGet(qV, &Buf[1]);
		VERIFY(ret >= 0);
		VERIFY(ret < 256);
		len = (byte) ret;
		Buf[0] = len;
		key = mapKeyTag(tag);
		if(!key)
		{
			continue;
		}
		ret = mapPut(key, Buf, (byte) (1 + len));
		CHECK(ret == 1 + len, lblDBA);

//++ @agmr
        if(key == emvTVR)
        {
            mapPut(traEmvTVR, Buf, 1 + len);
//            ShowData(Buf,1+len,0,0,15);
        }
        if(key == emvIssTrnCrt)
        {
            mapPut(traEmvIssTrnCrt, Buf, 1 + len);
//            ShowData(Buf,1+len,0,0,15);
        }
//-- @agmr
	}

	queRewind(qV);
	ret = queLen(qV);
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = -1;
	}
	trcFN("putEmvQue: ret=%d\n", ret);
	return ret;
}

/** Initialize the context before starting an EMV transaction.
 * A number of tags described in the EMV Kernel documentation should be
 *  transmitted to EMV Kernel transaction module.
 *  If Kernel did not receive a tag which is necessary for the next step, it
 *  can optionally send a list of tags wanted.
 * \param qVAK (I) Queue containing TLVs transmitted from Application to Kernel.
 * \param qTKA (O) Queue containing tags that Kernel wants to know when the next Kernel function will be called.
 * \source log\\emv.c
 */
static int emvAppContext(tQueue * qVAK, tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	long tka[] = {              //array of tags to build qTKA
	                 tagTrnCurCod,
	                 tagAmtBin,
	                 tagTrnTyp,
	                 tagAmtNum,
	                 tagAmtOthNum,
	                 tagAmtOthBin,
	                 tagTrnCurExp,
	                 tagTrnDat,
	                 tagAcqId,
	                 tagTrmAvn,
	                 tagMrcCatCod,
	                 tagAccCntCod,
	                 tagTrmFlrLim,
	                 tagTrmId,
	                 tagTrnTim,
	                 tagTrmCap,
	                 tagTrmTyp,
	                 tagAddTrmCap,
	                 tagTrnSeqCnt,
	                 tagVlpSupInd,
	                 tagVlpTrmLim,
	                 //tagTCC,
	                 tagTACDft,
	                 tagTACDen,
	                 tagTACOnl,
	                 tagThrVal,
	                 tagTarPer,
	                 tagMaxTarPer,
	                 tagLstRspCod,
	                 tagTRMOverpassAIP,
	                 0
	             };
	byte idx;

	VERIFY(qVAK);
	VERIFY(qTKA);
	trcS("emvAppContext Beg\n");

	ret = emvAmountInput();
	CHECK(ret > 0, lblKO);

	queReset(qVAK);
	queReset(qTKA);

	idx = 0;
	while(tka[idx])
	{
		putTagQue(qTKA, tka[idx]);
		idx++;
	}

	ret = getEmvQue(qVAK, qTKA);
	CHECK(ret >= 0, lblDBA);

	queReset(qTKA);

	ret = emvContext(qVAK, qTKA);
	//CHECK(ret >= 0, lblKO);

	//ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	ret = -1;
	goto lblEnd;
lblKO:
//++@agmr
	if(ret == -100)
		goto lblEnd;
//--@agmr
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = -1;
	}
	trcFN("emvAppContext: ret=%d\n", ret);
	return ret;
}

/** This function should be called before any EMV Kernel function.
 * This function retrieves all tag values qTKA from data base requested by Kernel
 * and puts them into qVAK.
 * \param qVAK (0)  Queue containing TLVs built from data base.
 * \param qTKA (I)  Queue containing tags used to build the correspondant TLVs that Kernel wants to know.
 * \source log\\emv.c
 */
static int emvBefore(tQueue * qVAK, tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end

	// Build TLVs requested by Kernel
	VERIFY(qVAK);
	VERIFY(qTKA);
	trcS("emvBefore: Beg\n");

	queRewind(qTKA);
	queReset(qVAK);
	ret = getEmvQue(qVAK, qTKA);
	CHECK(ret >= 0, lblDBA);    // Build TLVs from tags requested by Kernel
	queReset(qTKA);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvBefore: ret=%d\n", ret);
	return ret;
}

/** This function should be called during AppPrepare.
 * \n This function checks for inconsistency between PAN # and Track 2 equivalent data contained in ICC
 * \return
 *  This function has return value.
 *    -  >0 : Pan Check OK.
 *    - <=0 : Pan Check failed.
 * \header log\\log.h
 * \source log\\emv.c
*/
static int emvCheckPan(void)
{
	int ret;
	byte idx;
	byte bcdPan[1 + lenEmvPAN];
	byte bcdTrk2[1 + lenEmvTrk2];
	char ascPan[1 + lenPan + 1];
	char ascTrk2[1 + lenTrk2 + 1];
	card panLen, track2Len;
	char chdName[lenChdNam + 1];

	ret = mapGet(emvPAN, bcdPan, lenEmvPAN + 1);
	CHECK(ret >= 0, lblKO);
	VERIFY(bcdPan[0] * 2 <= 1 + lenPan);
	bin2hex(ascPan, bcdPan + 1, *bcdPan);
	trcFS("ascPan=%s\n", ascPan);
	for (idx = 0; idx < lenPan; idx++)
	{
		if(isSorted('0' - 1, ascPan[idx], '9' + 1))
			continue;
		ascPan[idx] = 0;
	}

	ret = mapGet(emvTrk2, bcdTrk2, lenEmvTrk2 + 1);
	CHECK(ret >= 0, lblKO);
	VERIFY(bcdTrk2[0] * 2 <= 1 + lenTrk2);
	bin2hex(ascTrk2, bcdTrk2 + 1, *bcdTrk2);
	trcFS("ascTrk2=%s\n", ascTrk2);
	for (idx = 0; idx < lenTrk2; idx++)
	{
		if(isSorted('0' - 1, ascTrk2[idx], '9' + 1))
			continue;
		ascTrk2[idx] = 0;
	}

	panLen = 0;
	trcFS("ascPan=%s\n", ascPan);
	trcFS("ascTrk2Pan=%s\n", ascTrk2);
	bin2num(&panLen, &bcdPan[0], 1);
	bin2num(&track2Len, &bcdTrk2[0], 1);
	trcS("bcdPan = ");
	trcBN(bcdPan, panLen);
	trcS("\n");
	trcS("bcdTrk2 = ");
	trcBN(bcdTrk2, track2Len);
	trcS("\n");
#if 0
	if(bcdPan[0] && bcdTrk2[0])
	{
		if(strcmp(ascPan, ascTrk2) != 0)
		{
			CHECK((memcmp(&bcdPan[1], &bcdTrk2[1], panLen - 1) == 0), lblKO);
		}
	}
#endif
	if(bcdPan[0] && bcdTrk2[0]) {
            if(strcmp(ascPan, ascTrk2) != 0) {
                CHECK((memcmp(&bcdPan[1], &bcdTrk2[1], panLen - 1) == 0), lblDatErr);
            }
        }

	ret = mapGet(emvChdNam, chdName, 1 + lenChdNam);
	CHECK(ret > 0, lblKO);
	MAPPUTSTR(traTrk1, chdName+1, lblKO);
	//---------------------------

	goto lblEnd;

lblDatErr:
    usrInfo(infEMVCardDatErr);
    goto lblKO;
lblKO:
	return -1;
lblEnd:
	return 0;
}

/** Retrieve the necessary EMV Transaction data from the card,
 * before the EMV transaction can be processed.
 * It performs the Get Processing Option command to the card. The card answer
 * contains AFL (Application File Locator).
 * It is the list of records in card files to be red. After that the Kernel
 * reads all the information according to this AFL.
 * If Kernel did not receive a tag which is necessary for the next step, it
 * can optionally send a list of tags wanted.
 * \param    qVAK (I)  Queue containing TLVs transmitted from Application to Kernel.
 * \param    qTAK (I)  Queue containing tags that Application wants to obtain from Kernel
 * \param    qVKA (O)  Queue containing TLVs transmitted from Kernel to the Application.
 * \param    qTKA (O)  Queue containing tags that Kernel wants to know when the next Kernel function will be called.
 * \source log\\emv.c
 */
static int emvAppPrepare(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA,
                         tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte idx;
	long tak[] = {
	                 tagPAN,
	                 tagPANSeq,
	                 tagAIP,
	                 tagCapkIdx,
	                 tagTrk2,
	                 tagExpDat,
	                 tagCVM,
	                 tagCrdAvn,
	                 tagChdNam,
	                 tagTSI,
	                 tagIACDft,
	                 tagIACDnl,
	                 tagIACOnl,
	                 tagAppLbl,
	                 tagAppPrfNam,
	                 tagTrnPINDta,
	                 tagIssCntCod, //@@REPAIR
	                 0
	             };

	VERIFY(qVAK);
	VERIFY(qTAK);
	VERIFY(qVKA);
	VERIFY(qTKA);
	trcS("emvAppPrepare: Beg\n");

	ret = emvBefore(qVAK, qTKA);
	CHECK(ret > 0, lblKO);

	queReset(qVKA);
	queReset(qTAK);
	idx = 0;
	while(tak[idx])
	{
		putTagQue(qTAK, tak[idx++]);
	}

	ret = emvPrepare(qVAK, qTAK, qVKA, qTKA);
	CHECK(ret >= 0, lblDeny);

	ret = putEmvQue(qVKA);
	CHECK(ret >= 0, lblDBA);

	ret = emvCheckPan();
	CHECK(ret >= 0, lblDeny);

	ret = 1;
	goto lblEnd;

lblDeny:
	inf = infNotAccepted;
	goto lblEnd;
lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppPrepare: ret=%d\n", ret);
	return ret;
}

/** Retrieve from data base PAN and Expiration Date in EMV format.
 * \n Then save it into data base in Ascii format.
 * \source log\\emv.c
 */
static int emvData(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	char buf[(2 * lenEmvPAN) + 1];
	byte EmvPAN[1 + lenEmvPAN]; // PAN in Emv format
	char Pan[lenPan + 1];       // PAN in Ascii format
	byte EmvExpDat[1 + lenEmvExpDat];   // Exp Date in Emv format
	char ExpDat[lenExpDat + 1]; // Exp Date in Ascii format
	char BlockedPan[lenPan+1];

	trcS("emvData: Beg\n");

	memset(BlockedPan, '*', sizeof BlockedPan);

	MAPGET(emvPAN, EmvPAN, lblDBA);
	CHECK(EmvPAN[0], lblDBA);
	bin2hex(buf, &EmvPAN[1], EmvPAN[0]);

	ret = fmtTok(0, buf, "F");  //PAN parsing
	CHECK(ret <= lenPan, lblKO);

	ret = fmtTok(Pan, buf, "F");
	VERIFY(ret <= lenPan);

	MAPPUTSTR(traPan, Pan, lblDBA);
    memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
    BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr
    MAPPUTSTR(traBlockPan,BlockedPan,lblDBA);
	MAPGET(emvExpDat, EmvExpDat, lblDBA);
	CHECK(EmvExpDat[0], lblDBA);
	bin2hex(ExpDat, &EmvExpDat[1], 2);  // Convert and parse Expiration Date (get YYMM only and truncate DD)

	MAPPUTSTR(traExpDat, ExpDat, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvData: ret=%d\n", ret);
	return ret;
}

/** Retrieve from the application table CAPK the Public Key Exponent
 * (Tag DF7F) and the Public RSA key (Tag DF1F), according to AID selected
 * and Public Key Index retrieved from "emvPrepare" call.
 * \source log\\emv.c
 */
static int loadCapk(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	word idx;
	byte CapkIdx[1 + lenCapkIdx];
	byte AID[1 + lenAID];
	byte RID[1 + lenRid];
	byte ridCapkIdx;
	card exp;
	byte RSAKey[1 + lenRSAKey];
	byte RSAKeyExp[1 + lenRSAKeyExp];

	trcS("loadCapk: Beg\n");

	MAPGET(emvCapkIdx, CapkIdx, lblDBA);
	if(CapkIdx[0] != 1)         // There is no CAPK index (Authentication not supported by the card)
		goto lblDBA;

	MAPGET(emvAid, AID, lblDBA);
	for (idx = 0; idx < dimPub - 1; idx++)
	{    // Check if the AID and the CAPK index from the card is part of the CAPK multi records table of the application
		mapMove(pubBeg, idx);
		MAPGET(pubRid, RID, lblDBA);
		CHECK(RID[0], lblDBA);  // CAPK table empty, RID not found
		if(memcmp(&RID[1], &AID[1], lenRid) != 0)   // RID from table == AID from card (5 first digits)?
			continue;           // No, move to the next record
		MAPGETBYTE(pubIdx, ridCapkIdx, lblDBA); // Retrieve RID Capk index from DBA
		if(ridCapkIdx == CapkIdx[1])    // RID Capk index from table == Capk index from card ?
			break;              // Yes, Capk index found => exit
	}

	//CHECK(idx<pubtEnd-1, lblStop);

	memset(RSAKeyExp, 0, 1 + lenRSAKeyExp);
	MAPGETCARD(pubExp, exp, lblDBA);
	// Prepare TAG DF7F EMV Public Key Exponent
	// Associated to the RSA Public Key, coming from
	// the Certification Authority.
	// Format b, Length 1 to 3, "\x01\x03"
	if(exp < 0x100)
	{           // Exponent should be saved in direct byte order
		RSAKeyExp[0] = 1;       // Length=1 (255) => 1 byte
	}
	else if(exp < 0x10000)
	{
		RSAKeyExp[0] = 2;       // Length=2 (65535) => 2 bytes
	}
	else
	{
		VERIFY(exp <= 0x10001);
		RSAKeyExp[0] = 3;       // Length=3 (>65535) => 3 bytes
	}

	num2bin(&RSAKeyExp[1], exp, RSAKeyExp[0]);  // ulong -> "\xNN\xNN\xNN"
	ret = mapPut(emvRSAKeyExp, RSAKeyExp, 1 + lenRSAKeyExp);
	CHECK(ret >= 0, lblDBA);

	MAPGET(pubkeyData, RSAKey, lblDBA);

	// Prepare TAG DF1F EMV Public RSA Key
	// RSA key transmitted to the EMV module, coming
	// from the Certification Authority.
	// Format b, Length Up to 248, "
	ret = mapPut(emvRSAKey, RSAKey, 1 + lenRSAKey);
	CHECK(ret >= 0, lblDBA);

	ret = idx;
	goto lblEnd;

lblDBA:
//	inf = infDataAccessError; //closed SW
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("loadCapk: ret=%d\n", ret);
	return ret;
}

/** Perform the Data Authentication regarding to the terminal and the smart card capabilities.
 * The EMV Kernel performs the Data Authentication and sets the corresponding
 * bits TVR and TSI.
 * The queue qVAK should be prepared according the qTKA got from the previous
 * "emvPrepare" call. Usually it contains the tags DF1F (RSA Key) and DF7F
 * (Public Key Exponent).
 * They should be retrieved from the application table of CAPK public keys
 * according to application AID and public key index retrieved from emvPrepare call.
 * If Kernel did not receive a tag which is necessary for the next step, it
 * can optionally send a list of tags wanted.
 * \param    qVAK (I)  Queue containing TLVs transmitted from Application to Kernel.
 * \param    qTAK (I)  Queue containing tags that Application wants to obtain from Kernel at the end of the execution of this function.
 * \param    qVKA (O)  Queue containing TLVs transmitted from Kernel to the Application.
 * \param    qTKA (O)  Queue containing tags that Kernel wants to know when the next Kernel function will be called.
 * \source log\\emv.c
 */
static int emvAppAuthenticate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA,
                              tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte idx;
	long tak[] = {
	                 tagTVR,
	                 tagTSI,
	                 0
	             };

	VERIFY(qVAK);
	VERIFY(qTAK);
	VERIFY(qVKA);
	VERIFY(qTKA);
	trcS("emvAppAuthenticate: Beg\n");

	ret = loadCapk();
	ret = emvBefore(qVAK, qTKA);
	CHECK(ret > 0, lblKO);

	queReset(qVKA);
	queReset(qTAK);
	idx = 0;
	while(tak[idx])
	{
		putTagQue(qTAK, tak[idx++]);
	}

	ret = emvAuthenticate(qVAK, qTAK, qVKA, qTKA);
	CHECK(ret >= 0, lblKO);

	ret = putEmvQue(qVKA);
	CHECK(ret >= 0, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppAuthenticate: ret=%d\n", ret);
	return ret;
}

/** Ask for an offline pin for an EMV card.
 * \param ucLastTry (I-)  Flag to prevent the last try on pin entry
 * \source log\\emv.c
 */
static int OffCVM(byte ucLastTry)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	char Msg1[dspW + 1];        // "           12.00"
	char Msg2[dspW + 1];        // "PIN:"
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	word prompt;
	int key;

	trcS("OffCVM: Beg\n");
	ret = cryStart('m');
	CHECK(ret >= 0, lblCry);

	//MAPGET(cmbDlgAmtPad, Msg1, lblDBA); // Get message to display on line 1 (amount)
	prompt = ucLastTry ? msgLastPin : msgPinPrompt;
	MAPGET(prompt, Msg2, lblDBA);   // Get message to display on line 2 (Pin prompt)

	dspStop();                  // Close channel to give cryptomodule access to HMI
	strcpy(Msg1," ");
	strcpy(Msg, Msg1);
	strcat(Msg, "\n");
	strcat(Msg, Msg2);
	strcat(Msg, "\n");
	strcat(Msg, "\n");
	ret = cryEmvPin(Msg);       // Ask for an off-line pin for an EMV card
	CHECK(ret >= 0, lblCry);

	key = ret;
	ret = dspStart();           // Now we can open HMI again
	CHECK(ret >= 0, lblKO);

	ret = key;
	goto lblEnd;

lblKO:                       // Non-classified low-level error
	inf = infProcessingError;   // Message ProcessingError on screen
	goto lblEnd;
lblCry:                      // Crypto peripheral error encountered
	dspStart();                 // Restart in case if it was stopped; the return code is not checked intentionnally
	inf = infCryptoPeriphError; // Message CryptoPeriphError on screen
	ret = kbdANN;
	goto lblEnd;
lblDBA:                      // Data access error encountered
	inf = infDataAccessError;   // Message DataAccessError on screen
	goto lblEnd;
lblEnd:
	cryStop('m');               // Stop in case if it was started; the return code is not checked intentionnally
	if(inf)
	{
		usrInfo(inf);
		if(inf != infCryptoPeriphError)
			ret = 0;
	}
	trcFN("OffCVM: ret=%d\n", ret);
	return ret;
}

/** Ensure that the person presenting the smart card is the cardholder to whom
 * the application in the ICC was issued.
 * The Cardholder Verification Methods (CVMs) are the different possibilities
 * to do this processing, and are defined by the card issuer.
 * The EMV Kernel must use the relevant data from the ICC to determine which
 * CVMs shall be executed. Some cards require multiple verification methods.
 * The possible CVMs are: Offline PIN processing (encrypted or not), Online PIN
 * Processing, Signature. The Signature can be associated to the Offline PIN
 * (multiple CVMs).
 * For this complete process, 4 functions are necessary:
 * - emvCVMstart: Wrappers around the function "amgEmvCardHolderVerification"
 *   with the first parameter CV_INITIAL. Should be called before CVM processsing
 * - emvCVMoff: Wrappers around the function "amgEmvCardHolderVerification". This
 *   function is called after offline pin processing using "cryEmvPin" function
 *   to inform EMV Kernel about the result of this information.
 * - emvCVMnext: Wrappers around the function "amgEmvCardHolderVerification".
 *
 * Usually this function is called inside a loop processing CVMs. The decision
 * to quit ot to continue processing is based on the value of the tag DF38
 * (CVMOUT Result).
 * - CV_BYPASSED: The merchant has bypassed the pin-entry. The next CVM must be performed.
 * - CV_KO: The PIN was not retrieved correctly. The next CVM must be performed.
 * - CV_OK: Online PIN or Display "Wrong pin" was processed correctly. The next CVM must be performed.
 *
 * - cryEmvPin: The Offline Pin entry proccess.
 * \param    qVAK (I)  Queue containing TLVs transmitted from Application toKernel.
 * \param    qTAK (I)  Queue containing tags that Application wants to obtain fromKernel at the end of the execution of this function.
 * \param    qVKA (O)  Queue containing TLVs transmitted from Kernel to the Application.
 * \param    qTKA (O)  Queue containing tags that Kernel wants to know when the next Kernel function will be called.
 * \source log\\emv.c
 */
static int emvAppCvm(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte CVMOutRes[1 + lenCVMOutRes];
	byte lastTry;               // Is it a last try?
	int opt;                    // Entry option;
	byte idx, ct;               // Cvm number
	byte off;                   // Offline pin?
	byte pinPas = 0;            // Pin bypass allowed?
	byte res;
	byte reqSign=0; //@@REPAIR
	long tak[] = {
	                 tagCVMOutRes,
	                 tagCVMRes,
	                 tagTVR,
	                 tagTSI,
	                 0
	             };

	// Cardholder Verification Method in progress
	VERIFY(qVAK);
	VERIFY(qTAK);
	VERIFY(qVKA);
	VERIFY(qTKA);
	trcS("emvAppCvm: Beg\n");

	ret = emvBefore(qVAK, qTKA);
	CHECK(ret > 0, lblKO);

	lastTry = 0;
	opt = 0;
	off = 0;
	idx = 0;
	while(++idx)
	{
		queReset(qVKA);
		queReset(qTAK);
		ct = 0;
		while(tak[ct])
		{
			putTagQue(qTAK, tak[ct++]);
		}

		if(idx == 1)
		{
			VERIFY(!off);
			ret = emvCVMstart(qVAK, qTAK, qVKA);    // Call before starting CVM processing
		}
		else if(off)
		{
			ret = emvCVMoff(qTAK, qVKA);    // Call after off-line processing
		}
		else
		{
			ret = emvCVMnext(opt, qTAK, qVKA);  // The descision to quit or to continue processing based on the value of tag DF38 (CVMOUT Result)
		}
		CHECK(ret >= 0, lblKO);

		ret = putEmvQue(qVKA);
		CHECK(ret >= 0, lblKO);

		MAPGET(emvCVMOutRes, CVMOutRes, lblDBA);
		if(bitTest(CVMOutRes + 1, cvmWrongPin))
		{
			opt = 1;
			dspClear();
			ret = usrInfo(infWrongPin);
			CHECK(ret >= 0, lblKO);
		}
		else if(off)
		{
			dspClear();
			ret = usrInfo(infValidPin);
			CHECK(ret >= 0, lblKO);
			MAPPUTBYTE(traPinVer, 1, lblKO);
		}
		off = 1;
		if(bitTest(CVMOutRes + 1, cvmSignature))
		{
			opt = 1;
			MAPPUTBYTE(traSgnReq, 1, lblDBA);
			reqSign=1; //@@REPAIR
		}


		//MAPGETBYTE(appPINCreditFlag, res, lblKO);
		//taroh di sini buat check TVR code negara, kalo negaranya indonesia harus minta PIN
		if(bitTest(CVMOutRes + 1, cvmOnlinePin))
		{
			ret = pmtPinInput();    // OnLine pin entry process
			/*CHECK(ret >= 0, lblPinBypassed);
			MAPPUTBYTE(traPinReq, 1, lblDBA);  // Check if OnLine Pin allowed from DBA (record "tra" transaction stuff)
			opt = 1;*/
			if(ret==0)
			{
				MAPGETBYTE(issPinOpt, pinPas, lblKO);
				CHECK((pinPas == '1'), lblPinBypassed);  // No, exit
				opt = 0;        // Yes, call "emvCVMnext" with option CV_BYPASSED
				MAPPUTBYTE(traPinReq, 0, lblDBA);
				off = 0;
			}
			else
			{
				MAPPUTBYTE(traPinReq, 1, lblDBA);  // Check if OnLine Pin allowed from DBA (record "tra" transaction stuff)
				opt = 1;
				break; //@@SIMAS-PIN_BYPASS
			}
		}

		if(bitTest(CVMOutRes + 1, cvmEnding))
		{
			//MAPGETBYTE(issPinOpt, pinPas, lblKO);
			//if(pinPas==1)
			//{

				//@@REPAIR - start
				word key,tag;
				byte dat[50];
				byte reqPIN=0;
				char Aid[132];

				tag=tagIssCntCod;
				key = mapKeyTag(tag);
				ret = mapGet(key, dat, sizeof(dat));

				MAPGET(emvAid, Aid, lblKO);
				if (dat[0]) {
					if (reqSign && (dat[1]==0x03) &&(dat[2]==0x60))
						reqPIN=1;
					/*else if(memcmp(&Aid[1], "\xA0\x00\x00\x03\x33", 5)==0)
						reqPIN=1;*/
				}
				else if(memcmp(&Aid[1], "\xA0\x00\x00\x03\x33", 5)==0)
						reqPIN=1;

				if (reqPIN) {
				//@@REPAIR - end
					dspClear();

					ret = scrollDisplay8("PRESS YES IF NO PIN", 1, 6, "", "NO      YES");
					if(ret == kbdVAL)
					{
						MAPPUTBYTE(traPinReq, 0, lblDBA);
					}
					else
					{
						MAPPUTBYTE(traPinReq, 1, lblDBA);   // Check if OnLine Pin allowed from DBA (record "tra" transaction stuff)
						ret = pmtPinInput();    // OnLine pin entry process
						CHECK(ret >= 0, lblKO);
						opt = 1;
					}
				}//@@REPAIR
				break;
			//}
		}

		if(bitTest(CVMOutRes + 1, cvmOfflinePin))
		{
			opt = 1;
			off = 1;
			if(bitTest(CVMOutRes + 1, cvmLastAttempt))
			{
				lastTry = 1;
				dspClear();
				ret = usrInfo(infLastPinTry);
				CHECK(ret >= 0, lblKO);
			}

			while(1)
			{
				ret = OffCVM(lastTry);  // OffLine pin entry process
				if  (   (ret == kbdVAL)
					&&	((0 < getPinLen()) && (getPinLen() < 4))
					)
				{
					continue;
				}
				else
					break;
			}
			CHECK(ret >= 0, lblKO);

			if(ret == kbdVAL && getPinLen() == 0)
			{ // OffLine pin entry bypassed
				//MAPGETBYTE(appPinPas, pinPas, lblDBA); // Check if Bypassed allowed from DBA (record "tra" transaction stuff)
				MAPGETBYTE(issPinOpt, pinPas, lblKO);
				CHECK((pinPas == '1'), lblPinBypassed);  // No, exit
				opt = 0;        // Yes, call "emvCVMnext" with option CV_BYPASSED
				off = 0;
			}
		}
	}

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;   // Msg DataAccessError on screen
	goto lblEnd;
lblKO:
	inf = infProcessingError;   // Msg ProcessingError on screen
	goto lblEnd;
lblPinBypassed:              //Nothing to do
	ret = 0;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		dspClear();
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppCvm: ret=%d\n", ret);
	return ret;
}

/** Perform the Processing Restrictions and the Terminal Risk Management.
 * The Processing Restrictions function determines the degree of compatibility
 * of the payment application in the terminal with the application in the ICC,
 * and makes any necessary adjustments, including possible rejection of the transaction.
 * Terminal Risk Management is performed by Kernel to protect the acquirer,
 * issuer and the payment system from fraud.
 * It provides positive issuer authorisation for high-value transactions and
 * Ensures that transactions initiated from ICCs go online periodically
 * to protect against threats that might be undetectable in an offline environment.
 * The Terminal Risk Management is usually performed only if the AIP of the card
 * supports it (Tag DF0B).
 * The queue qVAK should be prepared according the qTKA go from the previous EMV
 * Kernel function call "emvCVM...". Usually it contains the tag 81 (Amount, Authorized (Binary).
 * Only the EMV Kernel can modify TVR and TSI. However, there are verifications that
 * cannot be performed by Kernel such as Opposition list checking and Transaction forced online.
 * In this case the application must perform them itself and transmit about the results
 * using special tags DF26 and DF1C.
 * \param    qVAK (I)  Queue containing TLVs transmitted from Application to Kernel.
 * \param    qTAK (I)  Queue containing tags that Application wants to obtain from Kernel at the end of the execution of this function.
 * \param    qVKA (O)  Queue containing TLVs transmitted from Kernel to the Application.
 * \param    qTKA (O)  Queue containing tags that Kernel wants to know when the next Kernel function will be called.
 * \source log\\emv.c
 */
static int emvAppValidate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA,
                          tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	card prv;
	byte FrcOnl[1 + lenFrcOnl];
	byte HotLst[1 + lenHotLst];
	byte AmtPrv[1 + lenAmtBin];
	byte idx;
	long tak[] = {
	                 tagTVR,
	                 tagTSI,
	                 0
	             };
	long tka[] = {
	                 -tagAmtBin,
	                 -tagAmtNum,
	                 -tagFrcOnl,
	                 -tagHotLst,
	                 -tagAmtPrv,
	                 0
	             };

	VERIFY(qVAK);
	VERIFY(qTAK);
	VERIFY(qVKA);
	VERIFY(qTKA);
	trcS("emvAppValidate: Beg\n");

	ret = pmtForceOnline();
	CHECK(ret >= 0, lblKO);

	VERIFY(ret <= 1);
	FrcOnl[0] = lenFrcOnl;
	VERIFY(FrcOnl[0] == 1);
	FrcOnl[1] = (byte) ret;
	ret = mapPut(emvFrcOnl, FrcOnl, 1 + lenFrcOnl);
	CHECK(ret >= 0, lblDBA);

	ret = logIsInBlackList();
	CHECK(ret >= 0, lblKO);

	VERIFY(ret <= 1);
	HotLst[0] = lenHotLst;
	VERIFY(HotLst[0] == 1);
	HotLst[1] = (byte) ret;
	ret = mapPut(emvHotLst, HotLst, 1 + lenHotLst);
	CHECK(ret >= 0, lblDBA);

	prv = 0;
	ret = logGetPreviousAmount(&prv);
	CHECK(ret >= 0, lblKO);

	AmtPrv[0] = lenAmtBin;
	VERIFY(AmtPrv[0] == 4);
	num2bin(&AmtPrv[1], prv, lenAmtBin);
	ret = mapPut(emvAmtPrv, AmtPrv, 1 + lenAmtBin);
	CHECK(ret >= 0, lblKO);

	idx = 0;
	while(tka[idx])
	{
		putTagQue(qTKA, tka[idx++]);
	}

	ret = emvBefore(qVAK, qTKA);
	CHECK(ret > 0, lblKO);

	queReset(qVKA);
	queReset(qTAK);
	idx = 0;
	while(tak[idx])
	{
		putTagQue(qTAK, tak[idx++]);
	}

	ret = emvValidate(qVAK, qTAK, qVKA, qTKA);
	CHECK(ret >= 0, lblKO);

	ret = putEmvQue(qVKA);
	CHECK(ret >= 0, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:
	usrInfo(infDataAccessError);    // Msg DataAccessError on screen
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);    // Msg ProcessingError on screen
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppValidate: ret=%d\n", ret);
	return ret;
}

/** Perform the Terminal Action Analysis and the Card Action Analysis.
 * The Terminal Action Analysis applies rules set by the issuer in the card
 * and by the acquirer in the terminal to the results of offline processing
 * to determine whether the transaction should be approved offline, declined offline,
 * or sent online for an authorisation.
 * - Review Offline Processing Results
 * The terminal reviews the results of offline processing recorded in he TVR to
 * determine whether the transaction should go online, be approved offline, or be declined offline.
 * This process considers issuer-defined criteria from the card called Issuer Action Codes
 *  and acquirer-defined criteria in the terminal called Terminal Action Code.
 * - Request Cryptogram Processing
 * The terminal requests a cryptogram from the card.
 * The Card Action Analysis allows issuers to perform velocity checking and other risk management,
 * which is internal to the card. The card may override the terminalís decision.
 * This function returns the type of cryptogram generated by the card (tag 9F27), and maybe,
 * the final terminal decision in the tag DF31 (0=Declined, 1=Approved).
 * \param qVAK (I)  Queue containing TLVs transmitted from Application to Kernel.
 * \param qTAK (I)  Queue containing tags that Application wants to obtain from Kernel at the end of the execution of this function.
 * \param qVKA (O)  Queue containing TLVs transmitted from Kernel to the Application.
 * \param qTKA (O)  Queue containing tags that Kernel wants to know when the next Kernel function will be called.
 * \source log\\emv.c
 */
static int emvAppAnalyse(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA,
                         tQueue * qTKA)
{
	int ret;
	//byte FallbackSet;
	word inf = 0;               //info screen to be displayed at the end
	byte idx;
	long tak[] = {
	                 tagTVR,
	                 tagTSI,
	                 tagRspCod,
	                 tagUnpNum,
	                 tagTrnTyp,
	                 tagIAD,
	                 tagIssTrnCrt,
	                 tagCID,
	                 tagAIP,
	                 tagATC,
	                 tagPANSeq,
	                 tagTrmDc1,
	                 tagTrmDc2,
	                 tagTaaResult,
	                 tagLstCmdRsp,
	                 0
	             };

	VERIFY(qVAK);
	VERIFY(qTAK);
	VERIFY(qVKA);
	VERIFY(qTKA);
	trcS("emvAppAnalyse: Beg");

	ret = emvBefore(qVAK, qTKA);
	CHECK(ret > 0, lblKO);

	queReset(qVKA);
	queReset(qTAK);
	idx = 0;
	while(tak[idx])
	{
		putTagQue(qTAK, tak[idx++]);
	}

	ret = emvAnalyse(qVAK, qTAK, qVKA, qTKA);
	CHECK(ret >= 0, lblKO);
	if(ret == 17)
		return ret;

	ret = putEmvQue(qVKA);
	CHECK(ret >= 0, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppAnalyse: ret=%d\n", ret);
	return ret;
}

static int emvAskReferral(void)
{
	// Process not implemented yet.
	return 1;
}

static int emvOnlAut(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte OnlSta[1 + lenOnlSta];
	char RspCod[1 + lenRspCod];
	char AuthRes[1 + lenAuthRes];

	trcS("emvOnlAut: Beg\n");

	OnlSta[0] = lenOnlSta;
	VERIFY(OnlSta[0] == 1);
	ret = onlSession();
	CHECK(ret >= 0, lblEnd);

	MAPGET(traRspCod, RspCod, lblDBA);
	if((ret == 0) || (RspCod[1] == 0))
	{    // Communication NOK
		OnlSta[1] = 2;
		ret = mapPut(emvCommOk, "\x01\x00", 1 + lenCommOk);
		CHECK(ret >= 0, lblDBA);
		ret = mapPut(emvAuthRes, "\x01\x00", 1 + lenAuthRes);
		CHECK(ret >= 0, lblDBA);
	}
	else
	{                    // Communication OK
		OnlSta[1] = 1;
		AuthRes[0] = 1;
		AuthRes[1] = 0;
		if(0 == memcmp(RspCod, "00", 2))
		      AuthRes[1] = 1;
		ret = mapPut(emvCommOk, "\x01\x01", 1 + lenCommOk);
		CHECK(ret >= 0, lblDBA);
		ret = mapPut(emvAuthRes, AuthRes, 1 + lenAuthRes);  // Yes, Online process status : 2 = Online failed
		CHECK(ret >= 0, lblDBA);
	}

	ret = mapPut(emvOnlSta, OnlSta, 1 + lenOnlSta);
	CHECK(ret >= 0, lblDBA);
	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvOnlAut: ret=%d\n", ret);
	return ret;
}
/*
int CekTVR()
{
	char TVR[1 + lenTVR];
	int ret = 1;

	MAPGET(emvTVR, TVR, lblKO);//for check tvr, is expired application ..?
	if(bitTest(TVR + 1, tvrExpApl)){
		usrInfo(infCardExpired);
		ret = 0;
	}

	if(bitTest(TVR+1, tvrPINTryLim)){//for check tvr, is pin limit  ..?
		usrInfo(infPinBlocked);
		ret = 0;
	}

	return ret;
	lblKO:
		return -1;
}
*/

static int emvUpdate()
{
	int ret;
	char IccData[lenEmvICC + 1];
	word lenEmv;
	char TrnCrt[lenIssTrnCrt + 1];
	char CID[lenCID + 1];
	char TVR[1+lenTVR];

	MAPGETWORD(TCEmvLen, lenEmv, lblKO);

	ret = mapGet(traEmvICC, IccData, lenEmvICC);
	CHECK(ret > 0, lblKO);

	MAPGET(emvIssTrnCrt, TrnCrt, lblKO);
	MAPGET(emvCID, CID, lblKO);
	MAPGET(emvTVR, TVR, lblKO);

	memcpy(IccData + 3, CID + 1, 1);
	memcpy(IccData + 7, TrnCrt + 1, 8);
	memcpy(IccData + 18, TVR + 1 , 5);

	ret = mapPut(traEmvICC, IccData, lenEmv);
	CHECK(ret > 0, lblKO);

	return ret;
	lblKO:
		return -1;

}

/** Analyse the Cryptogram Info Data type to start the following transaction process :
 *  - AAC = Offline Transaction Declined => Message Declined on screen
 *  - TC  = Offline Transaction Approved => Message Approved on screen
 *  - ARQC = Online Authorisation requested => Online process
 *  - AAR = Referral requested by the card => Referral process
 * \param    ucCIDtype (I)  Cryptogram Info Data type
 * \param    ucAC (I)  1 = First Generate Application Cryptogram (AC) 2 = Second Generate Application Cryptogram (AC)
 * \header log\\log.h
 * \source log\\emv.c
 */
static int emvResult(byte ucCIDtype, byte ucAC)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte OnlSta[1 + lenOnlSta];
	byte TxnType;

	// Cryptogram Info Data type analysis
	trcS("emvResult: Beg\n");

	trcFN("emvResult: CID type=%02X\n", ucCIDtype);
	VERIFY((ucAC == 1) || (ucAC == 2));

	switch (ucCIDtype)
	{
		case 0x00:               // *** AAC: offline/online declined ***
			if(ucAC == 1)
			{       // First Generate Application Cryptogram
				trcS("First generate AC\n");
				trcS("Card answers AAC\n");
				trcS("Offline declined Z1\n");
				usrInfo(infDeclinedEMV);
			}
			else
			{   // Second Generate Application Cryptogram

//++ @agmr - untuk Installment Master emv di by pass, sudah disetujui oleh BRI
//#ifdef BYPASS_MASTER_EMV
//                {
//                    byte txnType;
//                    char buf[50];
//
//                    MAPGETBYTE(traTxnType,txnType,lblKO);
//                    MAPGET(binHi,buf,lblKO);
//
//                    if(txnType == trtInstallment &&
//                       ( (buf[1] & 0x50) == 0x50)
//                       )
//                    {
//                        //CID[1] = 0x40;
//                        mapPut(emvCID, "\x01\x40",2);
//
//        				MAPGET(emvOnlSta, OnlSta, lblKO);
//        				if(OnlSta[1] == 1)
//        				{  // Online completed
//        					trcS("Second generate AC\n");
//        					trcS("Card answers TC\n");
//        					trcS("Online approved 00\n");
//        					MAPPUTBYTE(traEmvTrnType, CID_GEN2, lblKO);
//
//        					emvUpdate();
//        //					MAPPUTBYTE(TCFlag, 1, lblKO); //@agmr - TC upload tutup sementara TC upload
//        					tcSave();
//
//        					MAPPUTBYTE(revReversalFlag, 0, lblKO);
//        					mapReset(revBeg);
//
//        					usrInfo(infApprove);
//        				}
//        				MAPPUTBYTE(traEmvFullFlag,0,lblKO);
//                        memset(buf,0x00,sizeof(buf));
//                        mapPut(emvTVR,buf,lenTVR);
//                        mapPut(emvIssTrnCrt,buf,lenIssTrnCrt);
//                        break;
//                    }
//                }
//#endif //BYPASS_MASTER_EMV
//                doBRIReversal();
//-- @agmr -

				MAPGET(emvOnlSta, OnlSta, lblKO);
				if(OnlSta[1] == 1)
				{  // Online completed
					trcS("Second generate AC\n");
					trcS("Card answers AAC\n");
					trcS("Online declined 05\n");
					usrInfo(infDeclinedEMV);
				}
				else
				{          // Online failed
					trcS("Second generate AC\n");
					trcS("Card answers AAC\n");
					trcS("UnableOnline declined Z3\n");
					usrInfo(infDeclinedEMV);
				}
			}
			break;
		case 0x40:               //TC: offline/online approved
			if(ucAC == 1)
			{       // First Generate Application Cryptogram ?
				trcS("First generate AC\n");
				trcS("Card answers TC\n");
				trcS("Offline approved Y1\n");
				mapPut(traRspCod, "00",2); //@@SIMAS-EMV_OFFLINE
				MAPPUTBYTE(traEmvTrnType, CID_GEN1, lblKO);
			}
			else
			{
				MAPGET(emvOnlSta, OnlSta, lblKO);
				if(OnlSta[1] == 1)
				{  // Online completed
					trcS("Second generate AC\n");
					trcS("Card answers TC\n");
					trcS("Online approved 00\n");
					MAPPUTBYTE(traEmvTrnType, CID_GEN2, lblKO);

					emvUpdate();
					//MAPPUTBYTE(TCFlag, 1, lblKO); //@agmr - TC upload tutup sementara TC upload
					tcSave();

					MAPPUTBYTE(revReversalFlag, 0, lblKO);
					mapReset(revBeg);

					usrInfo(infApprove);
				}
				else
				{          // Online failed
					trcS("Second generate AC\n");
					trcS("Card answers TC\n");
					trcS("UnableOnline approved Y3\n");
					usrInfo(infDeclinedEMV);
				}
			}
			break;
		case 0x80:               //ARQC: go online
			MAPGETBYTE(traTxnType, TxnType, lblKO);
			if (TxnType == trtOffline)
			{
				MAPPUTBYTE(traTxnType, trtSale, lblKO);
				MAPPUTBYTE(traMnuItm, mnuSale, lblKO);
			}
			ret = emvOnlAut();
			CHECK(ret > 0, lblKO);
			break;
		default:                 //AAR: referral required
			VERIFY(ucCIDtype == 0xC0);
			trcS("First generate AC\n");
			trcS("Card answers AAR\n");
			trcS("Go Referral\n");
			ret = emvAskReferral();
			CHECK(ret > 0, lblKO);
			break;
	}

    ret = 1;
    goto lblEnd;

  lblKO:
    if(ret != -100) //@agmr
    	inf = infProcessingError;
    goto lblEnd;
  lblEnd:
    if(inf) {
        usrInfo(inf);
//        ret = 0;
    }
    trcFN("emvResult: ret=%d\n", ret);
    return ret;
}

/** Determine whether the transation is finally approved offline or declined
 * offline and also processes the issuer scripts.
 * A second Generate AC command is sent to the card with the type of cryptogram
 * depending of the Online process performed by the payment application.
 * The card will return the type of cryptogram TC or AAC representing the final
 * decision for the transaction.
 * \n AAC = Offline Transaction Declined => Message Declined on screen
 * \n TC  = Offline Transaction Approved => Message Approved on screen
 * \param    qVAK (I)  Queue containing TLVs transmitted from Application to Kernel.
 * \param    qTAK (I)  Queue containing tags that Application wants to obtain from Kernel at the end of the execution of this function.
 * \param    qVKA (O)  Queue containing TLVs transmitted from Kernel to the Application.
 * \param    qTKA (O)  Queue containing tags that Kernel wants to know when the  next Kernel function will be called.
 * \header log\\log.h
 * \source log\\emv.c
 */
static int emvAppComplete(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA,
                          tQueue * qTKA)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	byte cry;
	byte idx;
	long tak[] = {
	                 tagTVR,
	                 tagTSI,
	                 tagRspCod,
	                 tagIAD,
	                 tagIssTrnCrt,
	                 tagCID,
	                 tagOnlSta,
	                 tagScrRes,
	                 tagApduResp,
	                 tagLstCmdRsp,
	                 0
	             };
	long tka[] = {
	                 -tagAutCod,
	                 -tagRspCod,
	                 -tagIssAutDta,
	                 -tagIssSc1,
	                 -tagIssSc2,
	                 -tagOnlSta,
	                 0
	             };

	VERIFY(qVAK);
	VERIFY(qTAK);
	VERIFY(qVKA);
	VERIFY(qTKA);
	trcS("emvAppComplete: Beg\n");

	idx = 0;
	while(tka[idx])
	{
		putTagQue(qTKA, tka[idx++]);
	}

	ret = emvBefore(qVAK, qTKA);
	CHECK(ret > 0, lblKO);

	queReset(qVKA);
	queReset(qTAK);
	idx = 0;
	while(tak[idx])
	{
		putTagQue(qTAK, tak[idx++]);
	}

	cry = 0x00;
	ret = emvIsApproved();
	if(ret)
	{
		cry = 0x40;
	}

	ret = emvComplete(cry, qVAK, qTAK, qVKA);
	CHECK(ret >= 0, lblKO);
	CHECK(ret == 1, lblEnd);

	ret = putEmvQue(qVKA);
	CHECK(ret >= 0, lblDBA);

	ret = 1;
	goto lblEnd;

lblDBA:
	inf = infDataAccessError;
	goto lblEnd;
lblKO:
	inf = infProcessingError;
	goto lblEnd;
lblEnd:
	if(inf)
	{
		usrInfo(inf);
		ret = 0;
	}
	trcFN("emvAppComplete: ret=%d\n", ret);
	return ret;
}

static int emvGetLabel()
{
	int ret;
	char label[1+lenAppPrfNam];
	char PrfNam[1 + lenAppPrfNam];
	byte txnType; //BRI7
	char tra1Label[30];
	char tra2Label[30];


	MAPGET(emvAppLbl, label,lblKO);
	MAPGET(emvAppPrfNam, PrfNam, lblKO);
	MAPGET(emvChdNam, tra1Label, lblKO);
	memset(tra2Label, 0x00, sizeof(tra2Label));
	strncpy(tra2Label, tra1Label+1, strlen(tra1Label)-1);
	mapPut(appAppPrfNam, PrfNam + 1, lenAppPrfNam);
	mapPut(emvChdNam, tra2Label, 30);

	if(PrfNam[0] == 0){
		ret = mapPut(emvAppPrfNam, label + 1, lenAppPrfNam);
		CHECK(ret >= 0, lblKO);
		mapPut(appAppPrfNam, label + 1, lenAppPrfNam);
	} else {
		int i;
		for (i = 0; i < PrfNam[0]; i++)
		{
			if (PrfNam[i+1] > 0x7F) // NON ASCII
			{
				ret = mapPut(emvAppPrfNam, label + 1, lenAppPrfNam);
				CHECK(ret >= 0, lblKO);
				mapPut(appAppPrfNam, label + 1, lenAppPrfNam);
				break;
			}
		}
	}
//++BRI7
	ret = mapGetByte(traTxnType, txnType);
	if(txnType != trtInstallment)
//--BRI7
		//ret = usrInfo(infTotalAmt);
//		ret = usrInfo(infTotal);
//	CHECK(ret > 0, lblKO);
//	CHECK(ret != kbdANN, lblKO);
{
            char buf[30],buf1[30];

            memset(buf,0,sizeof(buf));
            mapGet(traTotAmt,buf,sizeof(buf));
            fmtAmt(buf1,buf,0,",.");
            fmtPad(buf1,-16,' ');
            MAPPUTSTR(msgBuffer,buf1,lblKO);
            while(1)
            {
        	    ret = usrInfo(infTotal2);
        	    CHECK(ret >= 0, lblKO);
        	    if(ret == kbdVAL)
        	        break;

        	    if(ret == kbdANN || ret == 0)
        	        return -100;
        	}
}
	return ret;
	lblKO:
		return -1;

}

/** Perform transaction flow of complete EMV transaction.
 * \header log\\log.h
 * \source log\\emv.c
 */
static int emvTransactionFlow(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	enum { tSize = 256, vSize = 1024 };
	tQueue qTAK;
	tBuffer bTAK;
	byte dTAK[tSize];           // Tags Application -> Kernel
	tQueue qTKA;
	tBuffer bTKA;
	byte dTKA[tSize];           // Tags Kernel -> Application
	tQueue qVAK;
	tBuffer bVAK;
	byte dVAK[vSize];           // TLVs Application -> Kernel
	tQueue qVKA;
	tBuffer bVKA;
	byte dVKA[vSize];           // TLVs Kernel -> Application
	byte CID[1 + lenCID];
	byte cry;
	char ACReq2hex[8+1];
	byte ACReq2nd[3];
	char LstCmd[7+1], LstCmdhex[14+1];
	char LstCmdtra[4+1];
    char ACReq2[261+1]; //@agmr - BRI - aslinya [57+1]
    byte txnType; //BRI7

	trcS("emvTransactionFlow: Beg\n");
	bufInit(&bTAK, dTAK, tSize);
	queInit(&qTAK, &bTAK);      //Input queue = Tags to send from Application -> Kernel
	bufInit(&bTKA, dTKA, tSize);
	queInit(&qTKA, &bTKA);      //Output queue = Tags to receive from Kernel -> Application
	bufInit(&bVAK, dVAK, vSize);
	queInit(&qVAK, &bVAK);      //Input queue = Values to send from Application -> Kernel
	bufInit(&bVKA, dVKA, vSize);
	queInit(&qVKA, &bVKA);      //Output queue = Values to receive from Kernel -> Application

	// Normally, before calling a Kernel function the application:
	// - prepares the queue qVAK according to qTKA from the previous call
	// - prepares the queue qTAK containing the tag numbers wanted by the application
	// - performs a call
	// - retrieves the values from qVKA filled by Kernel according to qTAK request
	// - uses qTKA to provide to the Kernel the information wanted in form of qVAK at the next call

//++BRI7
    ret = pmtBRICustomInput();
	CHECK(ret >= 0, lblKO);
	if(ret == 0)
		goto lblEnd;
//--BRI7

// ++ @agmr - BRI7
    MAPGETBYTE(traTxnType,txnType,lblKO);
    ret = pmtBRIKonfirmasiTanpaInquiry(txnType);
    if(ret <= 0)
        goto lblEnd;
// ++ @agmr - BRI7

	ret = emvAppContext(&qVAK, &qTKA);
	CHECK(ret >= 0, lblKO); // Bao add '='
	CHECK(ret == 0, lblEnd); //Bao add

	ret = emvAppPrepare(&qVAK, &qTAK, &qVKA, &qTKA);
	CHECK(ret > 0, lblKO);

	if(txnType != trtDummy){
		ret = emvGetLabel();
		CHECK(ret > 0, lblKO);
	}

	ret = emvData();            //convert some data elements from EMV binary format to ASCII
	CHECK(ret > 0, lblKO);

	if(txnType == trtDummy){
		ret = 1;
		goto lblEnd;
	}

	if (emvFullProcessCheck() != 1)
//	if (emvFullProcessCheck() == 0)
	{
		ret = 0;
	       goto lblEnd;
	}


	ret = valBin();
	CHECK(ret > 0, lblKO);

//++@agmr
    if(txnType == trtInstallment)
    {
    	byte tmp;

        MAPGETBYTE(traKategoriKartu,tmp,lblDBA)
        if(tmp != CREDIT_CARD)
        {
            usrInfo(infInvalidCard);
            ret = -100;
            goto lblEnd;
        }
    }

    if(txnType == trtSaleRedeem)
		inputRedeem();

    if(checkPendingSettlement()!=0)
    {
        ret = -100;
        goto lblEnd;
    }
//--@agmr

	ret = emvAppAuthenticate(&qVAK, &qTAK, &qVKA, &qTKA);
	CHECK(ret > 0, lblKO);

	ret = emvAppCvm(&qVAK, &qTAK, &qVKA, &qTKA);
	CHECK(ret > 0, lblKO);

	ret = emvAppValidate(&qVAK, &qTAK, &qVKA, &qTKA);
	CHECK(ret > 0, lblKO);

	ret = emvAppAnalyse(&qVAK, &qTAK, &qVKA, &qTKA);
	CHECK(ret > 0, lblKO);
	CHECK(ret == 1, lblEnd);

	MAPGET(emvCID, CID, lblDBA);
	MAPPUTSTR(emvACRsp1, CID, lblDBA);
	CHECK(CID[0] == lenCID, lblDBA);
	VERIFY(CID[0] == 1);
	ret = emvResult(CID[1] & 0xC0, 1);  // Start the right process (AAC, TC, ARQC, AAR)
	CHECK(ret > 0, lblKO);

	switch (CID[1] & 0xC0)
	{
		case 0x80:               //ARQC: go online
		case 0xC0:               //AAR: referral required
			ret = emvAppComplete(&qVAK, &qTAK, &qVKA, &qTKA);
			CHECK(ret > 0, lblKO);
			if (ret==27) {					  	//@@AS0.0.34 - Kartu di cabut
				ret=1; 							//@@AS0.0.34
				mapPut(emvCID, "\x01\x00",2); 	//@@AS0.0.34
			} 									//@@AS0.0.34
			CHECK(ret == 1, lblEnd); //@agmr - Periksa emv, bila kartu dicabut pada saat recv. tidak masuk ke kondisi ini

			cry = 0x00;
			ret = emvIsApproved();    // Terminal decision before generate AC
			if(ret)
			{
				cry = 0x40;
			}

			MEM_ZERO(ACReq2nd);
			MAPGET(emvACReq2, ACReq2, lblDBA);
			bin2hex(ACReq2hex, ACReq2, 4);
			memcpy(ACReq2nd, ACReq2hex + 6, 2);
			MAPPUTSTR(traACReq2, ACReq2nd, lblKO);

			MAPGET(emvCID, CID, lblDBA);
			MAPPUTSTR(emvACRsp2, CID, lblDBA);
			CHECK(CID[0] == lenCID, lblDBA);
			VERIFY(CID[0] == 1);
			ret = emvResult(CID[1] & 0xC0, 2);
			CHECK(ret > 0, lblKO);
			break;
		case 0x00:               //AAC: offline declined
		case 0x40:               //TC : offline approved
			break;
		default:
			VERIFY((ret == 0x00) || (ret == 0x40));
			break;
	}

	MEM_ZERO(LstCmdtra);
	MAPGET(emvLstCmdRsp, LstCmd, lblDBA);
	bin2hex(LstCmdhex, LstCmd, 7);
	memcpy(LstCmdtra, LstCmdhex + 10, 4);
	ret = mapPut(traLstCmdRsp, LstCmdtra, 4);
	ret = 0; //Bao add //1
    goto lblEnd;

  lblDBA:
    inf = infDataAccessError;
    ret = -1; //Bao add //0
    goto lblEnd;
  lblKO:
    if(ret == 0)
        goto lblEnd;
    if(ret != -100)
	   inf = infProcessingError;
    goto lblEnd;
  lblEnd:

	if(ret == 24)
		inf = infCardNotAccepted;
	if(ret == 17)
		inf = infEmvError;
    if(inf) {
	 dspClear();
        usrInfo(inf);
        //ret = -1; //Bao add // 0
    }
    trcFN("emv nsactionFlow: ret=%d\n", ret);
    return ret;
}
void myMapPut(word key,const void *ptr,word len){
	char * tmpPtr;
	tmpPtr = (char *)ptr;
	tmpPtr[0] = len - 1;
	mapPut(key, ptr, len);
}
void emvReset()
{
	char zero[256];
	char space[256];

	MEM_ZERO(zero);
	MEM_0x20(space);

	myMapPut(emvPAN, 			zero, 	1);
	//myMapPut(emvTrnDat, 		zero, 	1+lenTrnDat);
	//myMapPut(emvTrnTyp, 		FF, 	1+lenTrnTyp);
	myMapPut(emvAmtNum, 		zero, 	1+lenAmtNum);
	myMapPut(emvAmtOthNum, 		zero, 	1+lenAmtNum);
	mapPut(emvPANSeq, "\x00\x00", 1+lenPANSeq);
	myMapPut(emvIACDft, 		zero, 	1+lenTVR);
	myMapPut(emvIACDnl, 		zero, 	1+lenTVR);
	myMapPut(emvIACOnl, 		zero, 	1+lenTVR);
	//myMapPut(emvAIP, 			zero, 	1+lenAIP);
	myMapPut(emvTVR, 			zero, 	1+lenTVR);
	myMapPut(emvCVMRes, 		zero, 	1+lenCVMRes);
	myMapPut(emvTSI, 			zero, 	1+lenTSI);
	myMapPut(emvIAD, 			zero, 	1);
	myMapPut(emvRspCod, 		space, 	1+lenRspCod);
	mapPut(emvACReq1,			zero, 	1);
	mapPut(traACReq2,			"FF", 	2);
	mapPut(emvACRsp1,			zero, 	1);
	mapPut(emvACRsp2,   		"\x01\xFF\x00", 	3);
	myMapPut(emvIssTrnCrt,		zero, 	1+lenIssTrnCrt);
	myMapPut(emvATC,			zero,	1+lenATC);
	myMapPut(emvUnpNum,			zero,	1+lenUnpNum);
	myMapPut(traLstCmdRsp,		zero,	5);
	myMapPut(emvAmtNum,			zero,	1+lenAmtNum);
	myMapPut(emvAmtPrv,			zero,	1+lenAmtBin);
	myMapPut(emvAppTraAmt,		zero,	1+lenAmt);
	myMapPut(emvIssCountryCode, zero, 	1+lenTrnCurCod); //@@REPAIR
	myMapPut(emvScriptResult, 	zero, 	1+lenEMVScriptRes); //@@REPAIR
}

/** Entry point of the complete EMV process.
 * \header log\\log.h
 * \source log\\emv.c
 */
int emvProcessing(void)
{
	int ret;
	word inf = 0;               //info screen to be displayed at the end
	char EntMod;                //card entry mode
	byte fsBuf[__FSBUFLEN__];   //final select response buffer
	char aid[1+lenAID];
//	byte txnType; //BRI8


	trcS("emvProcessing Beg\n");

	emvReset();

	MAPGETBYTE(traEntMod, EntMod, lblDBA);
	if(EntMod == 'c')
	{         // Operation is initiated from menu, application select is required
		ret = emvStart();
		CHECK(ret >= 0, lblKO);
		emvSetFSbuf(fsBuf);     // Set the buffer to hold the last APPLICATION select command
		ret = emvApplicationSelect();
		CHECK(ret > 0, lblStop);
	}
	else
	{                    // Otherwise, the transaction is initiated by card insertion, the application is already selected
		VERIFY(EntMod == 'C');
	}

	//ret = getTrmFlrLim(); //AB: get floor limit from configuration downloaded from TMS
	//CHECK(ret >= 0, lblDBA);

	/*
	MAPGET(emvAid, aid, lblKO);
	if(memcmp(aid + 1, "\xA0\x00\x00\x00\x04\x10\x10", 7) == 0){
		ret = mapPut(emvTrmCap, "\x03\xE0\xB0\xC8", 4);
		CHECK(ret >= 0, lblKO);
	} else {
		ret = mapPut(emvTrmCap, "\x03\xE0\xB8\xC8", 4);
		CHECK(ret >= 0, lblKO);
	}
	*/
	ret = emvTransactionFlow();             // EMV transaction flow process
	CHECK(ret >= 0, lblStop);
	CHECK(ret == 0, lblEnd);
	goto lblEnd;

lblDBA:
    usrInfo(infDataAccessError);
    ret = -1;
    goto lblEnd;
  lblKO:
    if(ret != -100)
    {
    	usrInfo(infProcessingError);
    	ret = -1;
    }
    goto lblEnd;
  lblStop:
    if(!ret)
        goto lblEnd;
    if(ret != -100)
    {
    	inf = infProcessingError;
    	ret = -1;
    }
    goto lblEnd;
lblEnd:
	if(inf)
		usrInfo(inf);
	trcFN("emvProcessing: ret=%d \n", ret);
	emvStop();
	return ret;
}

int emvFullProcessSet(void)
{
	word inf = 0;               //info screen to be displayed at the end
	int ret;
	byte TxnType;
	word mnuItm;
	byte EntMod;

	MAPGETWORD(traMnuItm, mnuItm, lblKO);
	MAPGETBYTE(traTxnType, TxnType, lblKO);
	MAPGETBYTE(traEntMod, EntMod, lblKO);
	if(EntMod == 'C' || EntMod == 'c') {
		switch(TxnType)
		{
			case trtSale:
			case trtSaleRedeem: //@ar - BRI
			case trtPreAut:
			case trtAut: //@agmr - BRI5
			case trtInstallment: //BRI7
#ifdef PREPAID
//            case trtPrepaidTopUp:
//            case trtPrepaidDeposit:
#endif
  			       MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
				break;
			default:
				//MAPPUTBYTE(traEmvFullFlag, 0, lblKO);
				break;
		}
	}
	goto lblEnd;
lblKO:
	goto lblEnd;
lblEnd:
    if(inf)
        usrInfo(inf);
	return ret;
}

int emvFullProcessCheck(void)
{
	int ret;
	byte fullEmv;
	byte txnType;
	word mnuItm;

//++ @agmr
	MAPGETBYTE(traTxnType,txnType,lblKO);
	if(txnType == trtBatchUpload)
	{
	    MAPGETBYTE(logTraEmvFullFlag, fullEmv, lblKO);
	}
	else
//-- @agmr
	{
	    MAPGETBYTE(traEmvFullFlag, fullEmv, lblKO);
	}

    MAPGETWORD(traMnuItm,mnuItm,lblKO);
    if(fullEmv && (mnuItm == mnuVoid || mnuItm == mnuVoidTarikTunai))
        fullEmv = 2;

	return fullEmv;
	lblKO:
		return -1;
}
// END SW 20100824
#endif
