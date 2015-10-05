//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/adm.c $
//$Id: adm.c 1931 2009-04-27 09:49:34Z ajloreto $

#include "log.h"
#include "key.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bri.h"

static byte tmsLoad;            //Load/Configure TMS configuration?

#define MAXLEN 255

static byte _adm_dlgClick = 0;  //click after key press?

/** Enable/disable clicking while key pressing in dialogs
 * \param clk (I)
 *  - 0: disable clicking
 *  - 1: enable ckicking
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
*/
void admDlgClick(byte clk)
{
	_adm_dlgClick = clk;
}

static void dspLLSS2(byte loc, const char *str)
{
	byte idx;
	byte len;
	char tmp[255];

	VERIFY(str);

	idx = 0;
	len = strlen(str);
	while((loc & 0x0F) < dspWdt(0))
	{   //mask out attributes
		fmtSbs(tmp, str, idx, dspWdt(0));
		dspLS(loc, tmp);
		loc++;
		idx += dspWdt(0);
		if(idx > len || loc >= dspHgt(0))
			break;
	}
}

/** This function is similar to enterTxt.
 * The parameters of functions and the return values are the same as we can read in the program
 * references. 
 * In the usage are some differences between two functions and some bugs of the enterTxt function
 * are fixed. 
 * The str parameter of enterAPN function can be empty string.
 * In case of input of the numbers (0-9) the position will be automatically the next
 * until the length of string will be len  (this is the maximum of the length). 
 * After pressing a key (e.g. 2) then pressing key F1 (down) again and again the abc2ab...
 * or pressing key F2 (up) again and again the ABC2AB... are cyclical repeating.
 *
 * \param loc (I) Input line number of display
 * \param str (IO) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string (maximum length)
 *
 * \pre 
 *    - loc<dspH
 *    - strlen(str)<=(dspH-loc)*dspW+1)
 *    - len>0
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN).
 *    - 0 in case of timeout. 
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcbb0000.c
*/
int admEnterAPN(byte loc, char *str, byte len)
{
	int ret;                    //return value
	int idx;                    //index of str
	int idc = 0;                //index of the circuit
	int const tmo = 6000;       //time-out over 60 seconds
	char key = 0;               //the actual key
	char last = 0;              //the last pressed key (before the actual)
	int end = 0;                //input end
	int itr = 0;                //interrupt
	byte sln;                   //length of the str
	char buf[255];              //buffer for '*'

	const char *letterGrb[] =
	    {
	        "0 ",
	        "1.,!",
	        "2ABC",
	        "3DEF",
	        "4GHI",
	        "5JKL",
	        "6MNO",
	        "7PQRS",
	        "8TUV",
	        "9WXYZ",
	        "",                     //OK
	        "",                     //Esc
	        "<#-",                  //00
	        "",                     //Backspace
	        "",                     //
	        "?*;",                  //Paper Feed
	        "",
	        "",
	        "",
	        "?*;$@|",               //key * - it need for terminal I6280
	        "",
	        ".,!<#-",               //key # - it need for terminal I6280
	    };

	const char *letterLrb[] =
	    {
	        "0 ",
	        "1.,!",
	        "2abc",
	        "3def",
	        "4ghi",
	        "5jkl",
	        "6mno",
	        "7pqrs",
	        "8tuv",
	        "9wxyz",
	        "",                     //OK
	        "",                     //Esc
	        "<#-",                  //00
	        "",                     //Backspace
	        "",                     //
	        "?*;",                  //Paper Feed
	        "",
	        "",
	        "",
	        "?*;$@|",               //key * - it need for terminal I6280
	        "",
	        ".,!<#-",               //key # - it need for terminal I6280
	    };
	trcS("admEnterAPN Beg\n");

	sln = strlen(str);
	idx = sln;

	VERIFY(loc < dspHgt(0));    //line number should be on the screen
	VERIFY(strlen(str) <= (word) (len + 1));    //too long initial strings are not acepted
	VERIFY(len);

	if(sln == 0)
	{
		memset(str, 0, len + 1);
	}
	else
	{
		dspLLSS2(loc, str);
		dspLLSS2(loc + 1, str + 16);
	}

	do
	{
		ret = kbdStart(1);
		CHECK(ret >= 0, lblKO);
		ret = tmrStart(0, tmo);
		CHECK(ret >= 0, lblKO);
		key = 0;
		while(tmrGet(0) && !key)
		{
			key = kbdKey();
			CHECK(key != '\xFF', lblKO);
		}

		kbdStop();
		if(tmrGet(0))
			if(_adm_dlgClick)
				Click();

		if(!tmrGet(0))
		{
			goto lblTmo;
		}

		tmrStop(0);

		if(idx < len && key >= '0' && key <= '9')
		{ //numbers
			str[idx] = key;
			last = key;
			idx++;
			idc = 1;
		}
		else
		{
			switch (key)
			{
				case kbdVAL:     //OK
					end = 1;
					break;
				case kbdANN:     //Esc
					itr = 1;
					break;
				case kbdFWD:     //PaperFeed
				case kbd00:      //00
					if(idx < len)
					{
						str[idx] = letterLrb[key - 48][0];
						last = key;
						idx++;
						idc = 1;
					}
					break;
				case kbdCOR:     //Backspace
					if(idx > 0)
					{
						idx--;
						str[idx] = 0;
					}
					break;
				case kbdF1:      //F1 Down, now lower case
					if((last >= '0' && last <= '9') || last == '<' || last == '?')
					{
						idx--;
						str[idx] = letterLrb[last - 48][idc];
						buf[idx] = letterLrb[last - 48][idc];
						if(letterLrb[last - 48][idc + 1] == 0)
						{
							idc = 0;
						}
						else
						{
							idc++;
						}
						idx++;
					}
					break;
				case kbdF2:      //F2 Up, now capital letters
					if((last >= '0' && last <= '9') || last == '<' || last == '?')
					{
						idx--;
						str[idx] = letterGrb[last - 48][idc];
						buf[idx] = letterGrb[last - 48][idc];
						if(letterGrb[last - 48][idc + 1] == 0)
						{
							idc = 0;
						}
						else
						{
							idc++;
						}
						idx++;
					}
					break;
				case kbdF3:      //F3 Change between Latin1 and Cyrillic or Latin1 and Latin2 (hungarian) characters
					break;
			}
		}

		dspLLSS2(loc, str);
		dspLLSS2(loc + 1, str + 16);
		trcFS("str = %s", str);

	}
	while(!end && !itr);

	if(itr)
		goto lblEnd;
	goto lblEnd;

lblTmo:                      //Time-out
	tmrStop(0);
	return 0;
lblKO:                       //KO
	trcErr(ret);
	dspLS(0, "KO");
	tmrPause(3);
	key = -1;
	goto lblEnd;
lblEnd:                      //OK or Esc
	trcS("admEnterAPN End\n");
	return key;
}

static void admChangeConnSettings(byte ConnType)
{
	int ret;
	byte idx = 0, count = 0;
	char bufferSrv[lenSrv + 1];
	word priTxnKey, secTxnKey, priStlKey, seStlKey;

	trcS("admChangeConnSettings Beg\n");
	MAPPUTBYTE(appConnType, ConnType, lblKO);
	switch (ConnType)
	{
		case 1:
			priTxnKey = acqPriTxnPhnSrv;
			secTxnKey = acqSecTxnPhnSrv;
			priStlKey = acqPriStlPhnSrv;
			seStlKey = acqSecStlPhnSrv;
			MAPPUTSTR(appChn, "M8N2400", lblKO);
			break;
		case 2:
			priTxnKey = acqPriTxnIPSrv;
			secTxnKey = acqSecTxnIPSrv;
			priStlKey = acqPriStlIPSrv;
			seStlKey = acqSecStlIPSrv;
			MAPPUTSTR(appChn, "T", lblKO);
			break;
		case 3:
			priTxnKey = acqPriTxnPhnSrv;
			secTxnKey = acqSecTxnPhnSrv;
			priStlKey = acqPriStlPhnSrv;
			seStlKey = acqSecStlPhnSrv;
			MAPPUTSTR(appChn, "R8N19600", lblKO);
			break;
		case 4:
			priTxnKey = acqPriTxnIPSrv;
			secTxnKey = acqSecTxnIPSrv;
			priStlKey = acqPriStlIPSrv;
			seStlKey = acqSecStlIPSrv;
			MAPPUTSTR(appChn, "G", lblKO);
			break;
		default:
			ret = -1;
			goto lblKO;
	}

	MAPGETBYTE(regAcqCnt, count, lblKO);
	idx = 0;
	while(idx < count)
	{
		mapMove(acqBeg, idx);
		MAPGET(priTxnKey, bufferSrv, lblKO);
		MAPPUTSTR(acqPriTxnSrv, bufferSrv, lblKO);
		MAPGET(secTxnKey, bufferSrv, lblKO);
		MAPPUTSTR(acqSecTxnSrv, bufferSrv, lblKO);
		MAPGET(priStlKey, bufferSrv, lblKO);
		MAPPUTSTR(acqPriStlSrv, bufferSrv, lblKO);
		MAPGET(seStlKey, bufferSrv, lblKO);
		MAPPUTSTR(acqSecStlSrv, bufferSrv, lblKO);
		idx++;
	}
	goto lblEnd;

lblKO:
	if(ret != 0)
		usrInfo(infInvalidInput);
lblEnd:
	trcS("admChangeConnSettings End\n");
}

/** Parse a string for extracting a part of it
 * \param *dst (O) Pointer for destination. dim characters are reserved at dst
 * \param *src (I) Pointer to the source string to parse.
 * \param  dim (I) size of the destination
 * \return a pointer to the string following the extracted part of it
 */
static const char *admParseStr(char *dst, const char *src, int dim)
{
	VERIFY(dst);
	VERIFY(src);

	while(*src)
	{
		if(*src == '|')
		{
			src++;              // skip separator
			break;
		}

		VERIFY(dim > 1);        // dim includes ending zero
		*dst++ = *src++;
		dim--;
	}
	*dst = 0;
	return src;
}

/** Change Merchant password in appMrcPwd
 * \header sys\\log.h
 * \source sys\\adm.c
*/
void admChangePwd(void)
{
	char confPwd[lenPwd + 1];
	char newPwd[lenPwd + 1];
	int ret;

	trcS("admChangePwd Beg\n");
	memset(confPwd, 0, lenPwd + 1);
	memset(newPwd, 0, lenPwd + 1);

	ret = usrInfo(infNewPwdInput);
	CHECK(ret >= 0, lblKO);
	memset(newPwd, 0, lenPwd + 1);

	ret = enterPwd(2, newPwd, lenPwd + 1);  //Enter new password
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblEnd);

	ret = usrInfo(infConfPwdInput); //Confirm password
	CHECK(ret >= 0, lblKO);
	memset(confPwd, 0, lenPwd + 1);
	ret = enterPwd(2, confPwd, lenPwd + 1);
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblEnd);

	CHECK(strcmp(newPwd, confPwd) == 0, lblBadPwd); //New password match?

	MAPPUTSTR(appMrcPwd, newPwd, lblKO);
	goto lblEnd;
lblBadPwd:
	usrInfo(infPwdInvalid);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcFN("admChangePwd ret=%d\n", ret);
}

/** Resets all log data
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admReset(void)
{           //delete batch and reversal
	int ret;
	int i;
	char name[20];

	trcS("admReset Beg\n");
	ret = usrInfo(infConfirmReset);
	if(ret != kbdVAL)
		return;
	usrInfo(infTerminalReset);
	logReset();
	MAPPUTBYTE(appNeedToBatchCopy,0,lblKO);//@agmr

    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);

        memset(name,0,sizeof(name));
		MAPGET(acqName, name, lblKO);
		
		if(name[0] == 0)
		    continue;
		    
		MAPPUTBYTE(acqIsPendingSettlement,0,lblKO);//@agmr
	}

	ret = mapReset(revBeg);     //delete reversal
	CHECK(ret >= 0, lblKO);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcS("admReset End\n");
}

/** Resets all reversal data
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTCReset(void)
{        //delete Reversal
	int ret;

	trcS("admRevReset Beg\n");
	ret = usrInfo(infConfirmResetTC);
	if(ret != kbdVAL)
		return;

	ret = mapReset(tclogBeg);
	CHECK(ret >= 0, lblKO);
	usrInfo(infTerminalResetTC);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcS("admRevReset End\n");
}

void admTCResetWoKey(void)//@@SIMAS-INIT_DB
{        //delete Reversal
	mapReset(tclogBeg);
	goto lblEnd;
lblEnd:
	trcS("admRevReset End\n");
}


/** Resets all reversal data
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admRevReset(void)
{        //delete Reversal
	int ret;

	trcS("admRevReset Beg\n");
	ret = usrInfo(infConfirmResetRev);
	if(ret != kbdVAL)
		return;

	ret = mapReset(revBeg);
	CHECK(ret >= 0, lblKO);
	usrInfo(infTerminalResetRev);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcS("admRevReset End\n");
}

void admRevResetWoKey(void) //@@SIMAS-INIT_DB
{        //delete Reversal
	mapReset(revBeg);
	goto lblEnd;
lblEnd:
	trcS("admRevReset End\n");
}


/** Prints initialisation receipt
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admInitRpt(void)
{
	int ret;
	byte counter = 0;
	char AcqName[lenName + 1];
	char Aid[lenAID + 1];

	trcS("admInitRpt Beg\n");

	ret = usrInfo(infInitPrinting);
	CHECK(ret >= 0, lblKO);
	ret = rptReceipt(rloInitHeader);
	CHECK(ret >= 0, lblKO);
	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, AcqName, lblKO);
		if(AcqName[0] == 0)
			break;
		ret = rptReceipt(rloInitBody);
		CHECK(ret >= 0, lblKO);
	}
	ret = rptReceipt(rloInitEmvHeader);
	CHECK(ret >= 0, lblKO);
	for (counter = 0; counter < dimAID; counter++)
	{
		mapMove(aidBeg, counter);
		MAPGET(emvAid, Aid, lblKO);
		if(Aid[0] == 0)
			break;
		ret = rptReceipt(rloInitEmvBody);
		CHECK(ret >= 0, lblKO);
	}

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admInitRpt End\n");
}

/** Displays Software version
 * \header sys\\log.h
 * \source sys\\adm.c
*/
void admSWver(void)
{
	int ret;

	trcS("admSWver Beg\n");

	ret = usrInfo(infSoftwareVersion);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admSWver End\n");
}

/** Displays transaction details depends on the trace number entered
 * Entered trace number is searched and compared to logROC.
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewTxn(void)
{
	int ret, logCount, idxCnt, key = 0, mnu = 0;
	char txnTypeName[dspW + 1];
	char enterROC[lenInvNum + 1];
	char logFmtAmt[lenAmt + 1];
	char logAmt[lenAmt + 1];
	card retROC, crdenterROC;
	word txnType, trxNum, ifVoid;
	byte idx = 0, Exp;
	card traAmt, tipAmt;
	byte LocOpt, voidTxn;

	trcS("admViewTxn Beg\n");

	memset(logFmtAmt, 0x00, sizeof(logFmtAmt));

	MAPGETWORD(regTrxNo, trxNum, lblKO);
	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	MAPGETBYTE(appLocOpt, LocOpt, lblKO);
	if(LocOpt & 0x08)
	{
		ret = valAskPwd(appMrcPwd);
		CHECK(ret > 0, lblKO);
		CHECK(ret != kbdANN, lblEnd);
	}

	ret = scrDlg(infEnterTrace, traFindRoc);
	CHECK(ret >= 0, lblKO);
	CHECK(ret != kbdANN, lblEnd);

	MAPGET(traFindRoc, enterROC, lblKO);
	trcFS("Entered ROC: %s\n", enterROC);
	dec2num(&crdenterROC, enterROC, 0);

	for (idxCnt = 0; idxCnt < logCount; idxCnt++)
	{
		mapMove(logBeg, (word) idxCnt);
		MAPGETCARD(logROC, retROC, lblKO);
		if(crdenterROC == retROC)
			break;
	}
	CHECK(idxCnt < logCount, lblTraceNotFound); //Trace Not Found
	trcFN("Trace Found index: %d\n", idxCnt);

	//Format Amount -> 0.00
	MAPGETBYTE(appExp, Exp, lblKO);
	Exp %= 10;
	CHECK(Exp == 0 || Exp == 2 || Exp == 3, lblKO);
	MAPGET(logTraAmt, logAmt, lblKO);
	fmtAmt(logFmtAmt, logAmt, Exp, 0);
	MAPPUTSTR(traFmtAmt, logFmtAmt, lblKO);

	MAPGET(logTraAmt, logAmt, lblKO);
	ret = dec2num(&traAmt, logAmt, lenAmt);
	MAPGET(logTipAmt, logAmt, lblKO);
	ret = dec2num(&tipAmt, logAmt, lenAmt);
	ret = num2dec(logAmt, (traAmt + tipAmt), lenAmt);
	MAPPUTSTR(traTotAmt, logAmt, lblKO);

	MAPGETCARD(logMnuItm, txnType, lblKO);
	switch (txnType)
	{
		case mnuSale:
			idx = trtSale;
			break;
		case mnuSaleRedeem:
			idx = trtSaleRedeem;
			break; //@ar - BRI
		case mnuPreaut:
			idx = trtPreAut;
			break;
//++ @agmr - BRI5			
		case mnuAut:
			idx = trtAut;
			break;
//-- @agmr - BRI5						
		case mnuRefund:
			idx = trtRefund;
			break;
		case mnuOffline:
			idx = trtOffline;
			break;
		case mnuVoid:
			idx = trtVoid;
			break;    
        case mnuInstallment:        
            idx = trtInstallment;
            break;     
	case mnuPreCom:
            idx = trtPreCom;
            break;
#ifdef PREPAID
        case mnuInfoDeposit:
            idx = trtPrepaidInfoDeposit;
            break;      
        case mnuPrepaidPayment:
            idx = trtPrepaidPayment;
            break;
        case mnuTopUpDeposit:
            idx = trtPrepaidDeposit;
            break;
        case mnuTopUpOnline:
            idx = trtPrepaidTopUp;
            break;
	 case mnuPrepaidPaymentDisc:
            idx = trtPrepaidPaymentDisc;
            break;
	 
#endif             
// -- @agmr - BRI
	}

	MAPGETBYTE(logVoidFlag, voidTxn, lblKO);

	trcFN("idx: %d\n", idx);
	VERIFY(rqtBeg < idx);
	VERIFY(idx < rqtEnd);
	mapMove(rqsBeg, (word) (idx - 1));

	CHECK(idx > 0, lblTraceNotFound);

	if(voidTxn)
		ifVoid = rqsVDisp;
	else
		ifVoid = rqsDisp;

	MAPGET(ifVoid, txnTypeName, lblKO);
	trcFS("txnTypeName: %s\n", txnTypeName);
	MAPPUTSTR(traTypeName, txnTypeName, lblKO);
//	while(!key)
    key = 1;
    while(key)
	{
//		word inf = 0;

		switch (mnu)
		{
			case 0:
			    viewBatch1(NORMAL_LOG,0);
//				inf = infViewBatchTxn;
				break;
			case 1:
			    viewBatch2(NORMAL_LOG,0);
//				inf = infViewBatchTxn2;
				break;
//			case 2:
//				inf = infViewBatchTxn3;
//				break;
		}
//		ret = usrInfo(inf);
        ret = tmrStart(0, 30 * 100);
        CHECK(ret >= 0, lblKO);                   //one minute timeout
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);
        key = 0;
        while(tmrGet(0)) {
            key = kbdKey();     //capture a key
            if(key)
                break;
        }
        ret = kbdStop();
//		trcFN("ret %d\n", ret);
//		key = 0;
//		switch (ret)
        switch (key)
		{
			case kbdUP:          //MORE
				mnu++;
				break;
			case kbdDN:          //MORE
				mnu++;
				break;				
			case kbdANN:         //STOP
			case 0:              //TIMEOUT
				goto lblEnd;
		}
		CHECK(ret >= 0, lblKO);

//		if((mnu > 2))
		if((mnu > 1))
			mnu = 0;
		else if(mnu < 0)
//			mnu = 2;
            mnu = 1;
	}

	CHECK(ret >= 0, lblKO);
	goto lblEnd;

lblTraceNotFound:
	usrInfo(infTraceNotFound);
	goto lblEnd;
lblNoTran:
	usrInfo(infNoTran);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewTxn End\n");
}

/** Displays all transaction type totals using function logCalculateTotalsTransactionType
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewTotals(void)
{
	int ret;
	char buf[10];
	char bufCur[5];
	word count;
	card txnAmt, totCnt, totAmt;
	char amt[lenTotAmt + 1];
	char total[3 + 1];

	trcS("admViewTotals Beg\n");

	//ret = logConfirmTotals();
	ret = logCalcTot();
	CHECK(ret >= 0, lblKO);

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

	MAPGETCARD(totAppOfflineCnt, count, lblKO);
	MAPGET(totAppOfflineAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Offline Amt: %d \n", txnAmt);
	trcFN("after Offline Cnt: %d \n", count);

	MAPGETCARD(totAppManualCnt, count, lblKO);
	MAPGET(totAppManualAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt = totAmt + txnAmt;
	totCnt = totCnt + count;
	trcFN("after Manual Amt: %d \n", txnAmt);
	trcFN("after Manual Cnt: %d \n", count);

	trcFN("Sale Total Amt: %d \n", totAmt);
	trcFN("Sale Total Cnt: %d \n", totCnt);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%ld.%02lu", totAmt / 100, totAmt % 100);
	//num2dec(amt, totAmt, 0);
	strcpy(buf, "SALE");
	strcpy(bufCur, "PHP");
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblEnd;

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
	sprintf(amt, "%ld.%02lu", totAmt / 100, totAmt % 100);
	//num2dec(amt, totAmt, 0);
	strcpy(buf, "REFUND");
	strcpy(bufCur, "-PHP");
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	ret = usrInfo(infViewTotal);
	if((ret == 0) || (ret == kbdANN))
		goto lblEnd;

	//TIP
	memset(buf, 0, sizeof(buf));
	memset(bufCur, 0, sizeof(bufCur));
	totAmt = 0;
	totCnt = 0;

	MAPGETWORD(totAppTpCnt, count, lblKO);
	MAPGET(totAppTpAmt, amt, lblKO);
	dec2num(&txnAmt, amt, 0);
	totAmt += txnAmt;
	totCnt += count;
	trcFN("txnTypeTip Amt: %d \n", txnAmt);
	trcFN("txnTypeTip Cnt: %d \n", count);

	trcFN("TIP Total Amt: %d \n", totAmt);
	trcFN("TIP Total Cnt: %d \n", totCnt);

	sprintf(total, "%03ld", totCnt);
	sprintf(amt, "%lu.%02lu", totAmt / 100, totAmt % 100);
	strcpy(buf, "TIP");
	strcpy(bufCur, "PHP");
	MAPPUTSTR(traLogTxnTtlCnt, total, lblKO);
	MAPPUTSTR(traTypeName, buf, lblKO);
	MAPPUTSTR(traTypeCurName, bufCur, lblKO);
	MAPPUTSTR(traLogTotalAmount, amt, lblKO);

	ret = usrInfo(infViewTotal);
	goto lblEnd;

lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewTotals End\n");
}

/** Displays the current trace number
 *    - Data element : regInvNum.
 *  
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewRoc(void)
{
	int ret;

	trcS("admViewRoc Beg\n");

	ret = usrInfo(infViewCurrentTrace);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewRoc End\n");
}

/** Sets ROC
 *    - Data element : regInvNum.
 * 
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admSetRoc(void)
{
	int ret;

	trcS("admSetRoc Beg\n");

	ret = scrDlg(infSetTraceNum, regInvNum);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admSetRoc End\n");
}

/** View current batch number
 *    - Data element : acqCurBat.
 * 
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewBatchNum(void)
{
	int ret;

	trcS("admViewBatchNum Beg\n");
//	mapMove(acqBeg, 0);         // will remove this after implementing multi-acquirer
//	ret = usrInfo(infViewBatchNum);
//	CHECK(ret >= 0, lblKO);

	ret = selectAcquirer1();
	if(ret < 0)
	    goto lblKO;
	if(ret == 0)
	    goto lblEnd;
	    	
	mapMove(acqBeg,ret-1);
	ret = usrInfo(infViewBatchNum);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewBatchNum End\n");
}

void admSetBatchNum(void)
{
	int ret;

	trcS("admSetBatchNum Beg\n");

	ret = selectAcquirer1();
	if(ret < 0)
	    goto lblKO;
	if(ret == 0)
	    goto lblEnd;
	mapMove(acqBeg,ret-1);
	
	ret = scrDlg(infSetBatchNum, acqCurBat);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admSetBatchNum End\n");
}

/** View current stan
 *    - Data element : regSTAN.
 * 
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewStan(void)
{
	int ret;

	trcS("admViewStan Beg\n");

	ret = usrInfo(infViewStan);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewStan End\n");
}

/** Sets STAN
 *    - Data element : regSTAN.
 * 
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admSetStan(void)
{
	int ret;

	trcS("admSetStan Beg\n");

	ret = scrDlg(infSetStan, regSTAN);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admSetStan End\n");
}

/** Display terminal batch
 * Allows the user to navigate transactions records in the batch
 * The user has the option to view different transaction details such as amount, authorization code
 * pan and transaction date and time. 
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewBat(void)
{
	int ret, logCount, idxCnt = 0, key = 0, mnu = 0;
	char txnTypeName[dspW + 1];
	char logFmtAmt[lenAmt + 1];
	char logAmt[lenAmt + 1];
	word txnType = 0, /*inf = 0,*/ trxNum, ifVoid;
	byte idx = 0, Exp, voidTxn = 0;
	card traAmt, tipAmt;

	trcS("admViewBat Beg\n");
	memset(logFmtAmt, 0x00, sizeof(logFmtAmt));
	MAPGETWORD(regTrxNo, trxNum, lblKO);
	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	MAPGETBYTE(appExp, Exp, lblKO);
	Exp %= 10;
	CHECK(Exp == 0 || Exp == 2 || Exp == 3, lblKO);

//	while(!key)
	key = 1;
    while(key)
	{
		mapMove(logBeg, (word) idxCnt);

		//Format Amount -> 0.00
		MAPGET(logTraAmt, logAmt, lblKO);
		fmtAmt(logFmtAmt, logAmt, Exp, 0);
		MAPPUTSTR(traFmtAmt, logFmtAmt, lblKO);

		MAPGET(logTraAmt, logAmt, lblKO);
		ret = dec2num(&traAmt, logAmt, lenAmt);
		MAPGET(logTipAmt, logAmt, lblKO);
		ret = dec2num(&tipAmt, logAmt, lenAmt);
		ret = num2dec(logAmt, (traAmt + tipAmt), lenAmt);
		MAPPUTSTR(traTotAmt, logAmt, lblKO);

		MAPGETCARD(logMnuItm, txnType, lblKO);
		switch (txnType)
		{
			case mnuSale:
				idx = trtSale;
				break;
			case mnuSaleRedeem:
				idx = trtSaleRedeem;
				break; //@ar - BRI
			case mnuPreaut:
				idx = trtPreAut;
				break;
//++ @agmr - BRI5				
			case mnuAut:
				idx = trtAut;
				break;	
//-- @agmr - BRI5							
			case mnuRefund:
				idx = trtRefund;
				break;
			case mnuOffline:
				idx = trtOffline;
				break;
			case mnuVoid:
				idx = trtVoid;
				break;    
            case mnuInstallment:        
                idx = trtInstallment;
                break;
#ifdef PREPAID   
            case mnuInfoDeposit:
                idx = trtPrepaidInfoDeposit;
                break;             
            case mnuPrepaidPayment:
                idx = trtPrepaidPayment;
                break;
            case mnuTopUpDeposit:
                idx = trtPrepaidDeposit;
                break;
            case mnuTopUpOnline:
                idx = trtPrepaidTopUp;
                break;
            case mnuAktivasiDeposit:
                idx = trtPrepaidAktivasi;
                break;
            case mnuPrepaidRedeem:
                idx = trtPrepaidRedeem;
                break; 
	    //++@agmr - brizzi2
            case mnuPrepaidReaktivasi:
                idx = trtPrepaidReaktivasi;
                break; 
            case mnuPrepaidVoid:
                idx = trtPrepaidVoid;
                break;
	     //--@agmr - brizzi2 
	     case mnuPrepaidPaymentDisc:
		 idx = trtPrepaidPaymentDisc;
                break;
	    case mnuPreCom:
		 idx = trtPreCom;
                break;
            default:
                goto lblKO;
                break;                                      
#endif                
// -- @agmr - BRI        				
		}
		MAPGETBYTE(logVoidFlag, voidTxn, lblKO);

		trcFN("idx: %d\n", idx);
		VERIFY(rqtBeg < idx);
		VERIFY(idx < rqtEnd);
		mapMove(rqsBeg, (word) (idx - 1));

		if(voidTxn)
			ifVoid = rqsVDisp;
		else
			ifVoid = rqsDisp;

		MAPGET(ifVoid, txnTypeName, lblKO);
		MAPPUTSTR(traTypeName, txnTypeName, lblKO);

		switch (mnu)
		{	    
			case 0:
			    viewBatch1(NORMAL_LOG,1);
//				inf = infViewBatch;
				break;
			case 1:
			    viewBatch2(NORMAL_LOG,1);
//				inf = infViewBatch2;
				break;
//			case 2:
//			    viewBatch3(NORMAL_LOG);
//				inf = infViewBatch3;
//				break;
		}
//		ret = usrInfo(inf);

        ret = tmrStart(0, 30 * 100);
        CHECK(ret >= 0, lblKO);                   //one minute timeout
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);
        key = 0;
        while(tmrGet(0)) {
            key = kbdKey();     //capture a key
            if(key)
                break;
        }
        ret = kbdStop();

		trcFN("ret %d\n", ret);
//		key = 0;
//		switch (ret)
		switch(key)
		{
#ifdef __TELIUM__
			case kbdF1:
				idxCnt--;
				break;
			case kbdF4:
				idxCnt++;
				break;
			case kbdUP:
				mnu--;
				break;
			case kbdDN:
				mnu++;
				break;
#else

			case kbdDN:          //PREV
				idxCnt--;
				break;
			case kbdUP:          //MORE
				mnu++;
				break;
			case kbdF3:          //NEXT
				idxCnt++;
				break;
#endif

			case kbdANN:         //STOP
			case 0:              //TIMEOUT
				goto lblEnd;

		}
		CHECK(ret >= 0, lblKO);

//		if((mnu > 2))
//			mnu = 0;
//		else if(mnu < 0)
//			mnu = 2;

		if((mnu > 1))
			mnu = 0;
		else if(mnu < 0)
			mnu = 1;

		if(idxCnt < 0)
			idxCnt = logCount - 1;
		else if(idxCnt >= logCount)
			idxCnt = 0;
	}

	CHECK(ret >= 0, lblKO);
	goto lblEnd;

lblNoTran:
	usrInfo(infNoTran);
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewBat End\n");
}

/** Initiates TMS parameter download
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTMSLoadParam(void)
{

	trcS("admTMSLoadParam Beg\n");

	tmsLoad = 1;                //set Load TMS to TRUE
	admTMSConfig();
	goto lblEnd;

lblEnd:
	trcS("admTMSLoadParam End\n");
}

/** Format IP address
 * \header sys\\log.h
 * \source sys\\adm.c
 */
static int admFmtIp(word key)
{
	int ret, cnt, ctr, lenSrvIP;
	char srvrIP[lenSrv + 1];
	char fmtIP[lenSrv + 1];

	memset(fmtIP, 0x00, sizeof(fmtIP));
	MAPGET(key, srvrIP, lblKO);

	lenSrvIP = strlen(srvrIP);

	if((srvrIP[0] != '|') && (srvrIP[3] != '.') && (lenSrvIP != 0)
	        && (srvrIP[7] != '.') && (srvrIP[11] != '.')
	        && (srvrIP[lenSrvIP - 1] != ' '))
	{
		ctr = 0;
		for (cnt = 0; cnt < lenSrvIP; cnt++)
		{

			if(srvrIP[cnt] != ' ')
				fmtIP[cnt + ctr] = srvrIP[cnt];
			else
				break;

			if(((cnt + 1) % 3 == 0) && (ctr < 3))
			{
				strcat(fmtIP, ".");
				ctr++;
			}
		}

		MAPPUTSTR(key, fmtIP, lblKO);
	}

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admFmtIp End\n");
	return ret;

}

static int admSaveHostIp(word key)
{
	int ret;
	char buf[64 + 1];
	char newHostIp[64 + 1];
	char adr[lenSrv + 1];
	int len;
	int ctr = 0;

	len = mapDatLen(key);
	CHECK(len > 0, lblKO);

	MAPGET(traSrv, adr, lblKO);
	MAPGET(key, buf, lblKO);

	//check if new value is the same with old
	if(memcmp(buf, adr, sizeof(adr)) != 0)
	{
		len = strlen(adr);
		memset(newHostIp, 0, sizeof(newHostIp));
		memcpy(newHostIp, adr, len);

		while(buf[ctr])
		{
			if(buf[ctr] == '|')
			{
				break;
			}
			ctr++;
		}

		memcpy(&newHostIp[len], &buf[ctr], strlen(buf) - ctr);
	}
	//save new ip
	MAPPUTSTR(key, newHostIp, lblKO);
	ret = 1;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("admSaveHostIp ret= %d\n", ret);
	return ret;
}

static int admSaveHostPort(word key, word portkey)
{
	int ret;
	char buf[64 + 1];
	char newHostPort[64 + 1];
	char port[lenSrv + 1];
	int len;
	int ctr = 0;

	len = mapDatLen(key);
	CHECK(len > 0, lblKO);

	MAPGET(traSrv, port, lblKO);
	MAPGET(key, buf, lblKO);

	//check if new value is the same with old
	if(memcmp(buf, port, sizeof(port)) != 0)
	{
		len = strlen(port);
		memset(newHostPort, 0, sizeof(newHostPort));
		//memcpy(newHostPort, port, len);

		while(buf[ctr])
		{
			if(buf[ctr] == '|')
			{
				break;
			}
			memcpy(&newHostPort[ctr], &buf[ctr], 1);
			ctr++;
		}
		memcpy(&newHostPort[ctr++], "|", 1);
		memcpy(&newHostPort[ctr], port, len);
	}

	if((portkey == infTMSGPRSPriPort) || (portkey == infTMSGPRSSecPort))
		strcat(newHostPort, "|");
	//save new port
	MAPPUTSTR(key, newHostPort, lblKO);
	ret = 1;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("admSaveHostPort ret=%d\n", ret);
	return ret;
}

static int admGetHostIpPort(word key, word infIpKey, word infPortKey,
                            const char *src)
{
	int ret;
	char adr[20];
	char port[10];

	//get primary ip add
	memset(adr, 0, sizeof(adr));
	src = admParseStr(adr, src, sizeof(adr));   //get ip
	//VERIFY(*src);

	//save to traSrv
	MAPPUTSTR(traSrv, adr, lblKO);
	ret = scrDlg(infIpKey, traSrv);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = admSaveHostIp(key);
	CHECK(ret >= 0, lblKO);

	//get primary port
	memset(port, 0, sizeof(port));
	src = admParseStr(port, src, sizeof(port)); //get port

	//save to traSrv
	MAPPUTSTR(traSrv, port, lblKO);
	ret = scrDlg(infPortKey, traSrv);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = admSaveHostPort(key, infPortKey);
	CHECK(ret >= 0, lblKO);

	ret = kbdVAL;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("admGetHostIpPort ret= %d\n", ret);
	return ret;
}

static int admTcpipGprsPrm(byte type)
{
	int ret, cnt;
	int len;
	char buf[64 + 1];
	word key1 = 0, key2 = 0;

	len = mapDatLen(appPriSrv);
	CHECK(len > 0, lblKO);

	MAPGET(appPriSrv, buf, lblKO);
	switch (type)
	{
		case 3:
			MAPPUTSTR(appChn, "T", lblKO);
			admChangeConnSettings(2); // re-assign all to tcp ip mode
			break;
		case 4:
			MAPPUTSTR(appChn, "G", lblKO);
			admChangeConnSettings(4); // re-assign all to gprs mode
			break;
	}

	//clear appPriSrv if data retrieved was a telephone number
	for (cnt = 0; cnt < strlen(buf); cnt++)
	{
		if(buf[cnt] == '|')
			break;
	}
	if(cnt == strlen(buf))
	{
		strcpy(buf, "|");
	}

	switch (type)
	{
		case 3:
			key1 = infTMSPriIP;
			key2 = infTMSPriPort;
			break;
		case 4:
			key1 = infTMSGPRSPriIP;
			key2 = infTMSGPRSPriPort;
			break;
	}

	ret = admGetHostIpPort(appPriSrv, key1, key2, buf); //pri ip/port
	if(ret != kbdVAL)
	{         // Cancel, timeout => exit
		ret = 0;
		goto lblEnd;
	}

	ret = admFmtIp(appPriSrv);  //format ip add
	CHECK(ret > 0, lblKO);

	len = mapDatLen(appSecSrv);
	CHECK(ret > 0, lblKO);

	MAPGET(appSecSrv, buf, lblKO);

	//clear appSecSrv if data retreived was a telephone number
	for (cnt = 0; cnt < strlen(buf); cnt++)
	{
		if(buf[cnt] == '|')
			break;
	}
	if(cnt == strlen(buf))
	{
		strcpy(buf, "|");
	}

	switch (type)
	{
		case 3:
			key1 = infTMSSecIP;
			key2 = infTMSSecPort;
			break;
		case 4:
			key1 = infTMSGPRSSecIP;
			key2 = infTMSGPRSSecPort;
			break;
	}

	ret = admGetHostIpPort(appSecSrv, key1, key2, buf); //sec ip/port
	if(ret != kbdVAL)
	{         // Cancel, timeout => exit
		ret = 0;
		goto lblEnd;
	}

	ret = admFmtIp(appSecSrv);  //format ip add
	CHECK(len > 0, lblKO);

	MAPGET(appPriSrv, buf, lblKO);
	MAPPUTSTR(appIpAdd, buf, lblKO);
	MAPGET(appSecSrv, buf, lblKO);
	MAPPUTSTR(appSecIpAdd, buf, lblKO);

	ret = kbdVAL;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("admTcpipPrm End ret=%d\n", ret);
	return ret;
}

/** View current TMS configuration and connection settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTMSViewConfig(void)
{
	int ret;
	byte dialOpt;

	trcS("admTMSViewConfig Beg\n");

	ret = dspClear();
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infTMSRefNum);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infTMSNii);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infTMSHostLink);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appDialOpt, dialOpt, lblKO);
	switch (dialOpt)
	{
		case 2:
			admViewDialup();
			break;
		case 3:
		case 4:
			admViewTCPIPGprs(dialOpt);
			break;
		case 1:
			admViewRs232();
			break;
		default:
			ret = -1;
			goto lblKO;
			break;
	}

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admTMSViewConfig End\n");
}

/*
static int admTMSGetHostIpPort(const char *primaryIP, const char *secondaryIP) {
    int ret;
    char adr[20];
    char port[10];
 
    //get primary ip add
    memset(adr, 0, sizeof(adr));
    primaryIP = admParseStr(adr, primaryIP, sizeof(adr));   //get ip
    MAPPUTSTR(traPriIpBuf, adr, lblKO);
 
    //get primary port
    memset(port, 0, sizeof(port));
    primaryIP = admParseStr(port, primaryIP, sizeof(port)); //get port
    MAPPUTSTR(traPriPortBuf, port, lblKO);
 
    //get secondary ip add
    memset(adr, 0, sizeof(adr));
    secondaryIP = admParseStr(adr, secondaryIP, sizeof(adr));   //get ip
    MAPPUTSTR(traSecIpBuf, adr, lblKO);
 
    //get secondary port
    memset(port, 0, sizeof(port));
    secondaryIP = admParseStr(port, secondaryIP, sizeof(port)); //get port
    MAPPUTSTR(traSecPortBuf, port, lblKO);
 
    goto lblEnd;
  lblKO:
    usrInfo(infProcessingError);
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("admGetHostIpPort ret= %d\n", ret);
    return ret;
}
*/

/** Prints current TMS configuration
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTMSPrintConfig(void)
{
	int ret;

	trcS("admTMSPrintConfig Beg\n");

	ret = rptReceipt(rloCommsConfig);   //(rloTMSCfgHdr);
	CHECK(ret >= 0, lblKO);

	/*    MAPGETBYTE(appDialOpt, dialOpt, lblKO);
	    switch (dialOpt) {
	      case 2:
	          ret = rptReceipt(rloTMSCfgDialUp);
	          CHECK(ret >= 0, lblKO);
	          break;
	 
	      case 3:
	          MAPGET(appPriSrv, pri, lblKO);
	          MAPGET(appSecSrv, sec, lblKO);
	          ret = admTMSGetHostIpPort(pri, sec);
	          CHECK(ret >= 0, lblKO);
	 
	          ret = rptReceipt(rloTMSCfgTcpIp);
	          CHECK(ret >= 0, lblKO);
	 
	          break;
	 
	      case 1:
	      case 4:
	          break;
	      default:
	          ret = -1;
	          goto lblKO;
	          break;
	    }
	*/
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admTMSPrintConfig End\n");
}

/** Displays TCPIP configuration
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewTCPIPGprs(byte type)
{
	int ret, len;
	char adr[20];
	char port[10];
	char buf[64 + 1];
	const char *src;
	word key = 0;

	trcS("admViewTCPIP Beg\n");

	len = mapDatLen(appPriSrv);
	CHECK(len > 0, lblKO);
	MAPGET(appPriSrv, buf, lblKO);
	src = buf;

	//get primary ip add
	memset(adr, 0, sizeof(adr));
	src = admParseStr(adr, src, sizeof(adr));   //primary get ip
	VERIFY(*src);

	//save to traSrv
	MAPPUTSTR(traSrv, adr, lblKO);

	switch (type)
	{
		case 3:
			key = infVTMSPriIP;
			break;
		case 4:
			key = infVTMSGprsPriIP;
			break;
	}

	ret = usrInfo(key);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	//get primary port
	memset(port, 0, sizeof(port));
	src = admParseStr(port, src, sizeof(port)); //primary get port

	//save to traSrv
	MAPPUTSTR(traSrv, port, lblKO);

	switch (type)
	{
		case 3:
			key = infVTMSPriPort;
			break;
		case 4:
			key = infVTMSGprsPriPort;
			break;
	}

	ret = usrInfo(key);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	len = mapDatLen(appSecSrv);
	CHECK(len > 0, lblKO);
	MAPGET(appSecSrv, buf, lblKO);
	src = buf;

	//get secondary ip add
	memset(adr, 0, sizeof(adr));
	src = admParseStr(adr, src, sizeof(adr));   //secondary get ip
	VERIFY(*src);

	//save to traSrv
	MAPPUTSTR(traSrv, adr, lblKO);

	switch (type)
	{
		case 3:
			key = infVTMSSecIP;
			break;
		case 4:
			key = infVTMSGprsSecIP;
			break;
	}

	ret = usrInfo(key);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	//get secondary port
	memset(port, 0, sizeof(port));
	src = admParseStr(port, src, sizeof(port)); //secondary get port

	//save to traSrv
	MAPPUTSTR(traSrv, port, lblKO);

	switch (type)
	{
		case 3:
			key = infVTMSSecPort;
			break;
		case 4:
			key = infVTMSGprsSecPort;
			break;
	}

	ret = usrInfo(key);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewTCPIP End\n");
}

/** Displays dialup configuration
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewDialup(void)
{
	int ret;

	trcS("admViewDialup Beg\n");

	ret = usrInfo(infViewTMSPrimaryPhone);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infViewTMSSecondaryPhone);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infTMSBaudRate);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewDialup End\n");
}

/** Displays RS232 configuration
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admViewRs232(void)
{
	int ret;

	trcS("admViewRs232 Beg\n");

	ret = usrInfo(infPortSetting);
	CHECK(ret != kbdANN, lblEnd)
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admViewRs232 End\n");
}

/** Configure dialup settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
static int admDialup(void)
{
	int ret, cnt;
	char BRate[15];
	char bufBRate[lenChn + 1];
	char bufPri[lenSrv + 1];
	char bufSec[lenSrv + 1];
	word baudRate;

	MAPGET(appPriSrv, bufPri, lblKO);
	MAPGET(appSecSrv, bufSec, lblKO);
	MAPPUTSTR(appChn, "M8N2400", lblKO);

	admChangeConnSettings(1);   // re-assign all to dial up mode

	//clear appPriSrv if data retreived was an IP address
	for (cnt = 0; cnt < strlen(bufPri); cnt++)
	{
		if(bufPri[cnt] == '|')
		{
			MAPPUTSTR(appPriSrv, "", lblKO);
			break;
		}
	}

	ret = scrDlg(infTMSPrimaryPhone, appPriSrv);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	//clear appSecSrv if data retreived was an IP address
	for (cnt = 0; cnt < strlen(bufSec); cnt++)
	{
		if(bufSec[cnt] == '|')
		{
			MAPPUTSTR(appSecSrv, "", lblKO);
			break;
		}
	}

	ret = scrDlg(infTMSSecondaryPhone, appSecSrv);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	//modem paramter
	ret = scrDlg(infTMSBaudRate, appBaudRate);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	//check baudrate value
	ret = valBaudRate();
	CHECK(ret >= 0, lblKO);

	MAPGET(cvtBaudRate, BRate, lblKO);

	switch (strlen(BRate) / 2)
	{
		case 1:
			baudRate = 1200;
			break;
		case 2:
			baudRate = 2400;
			break;
		case 3:
			baudRate = 9600;
			break;
		case 4:
			baudRate = 14400;
			break;
		case 5:
			baudRate = 19200;
			break;
		case 6:
			baudRate = 28800;
			break;
		case 7:
			baudRate = 33600;
			break;
		default:
			baudRate = 12400;
			break;
	}

	MAPGET(appChn, bufBRate, lblKO);
	trcFS("saved appChn: %s\n", bufBRate);

	memset(bufBRate, 0x00, sizeof(bufBRate));
	memcpy(bufBRate, "M8N", sizeof(bufBRate));
	num2dec(&bufBRate[3], baudRate, 0);

	trcFS("selected appChn: %s\n", bufBRate);
	MAPPUTSTR(appChn, bufBRate, lblKO);
	ret = kbdVAL;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("admDialup ret= %d\n", ret);
	return ret;
}

/** Configure rs232 settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
static int admRs232(void)
{
	int ret;
	char BRate[7];
	char bufBRate[lenChn + 1];
	char bufPri[lenSrv + 1];
	char bufSec[lenSrv + 1];
	byte baudRate;

	trcS("admRs232 Beg\n");
	MAPGET(appPriSrv, bufPri, lblKO);
	MAPGET(appSecSrv, bufSec, lblKO);
	MAPPUTSTR(appChn, "R8N19600", lblKO);

	admChangeConnSettings(3);   // re-assign all to rs232 mode

	ret = scrDlg(infPortSetting, appComType);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	//RS 232 Baud Rate
	ret = scrDlg(infRs232BaudRate, appBaudRate);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	MAPGETBYTE(appBaudRate, baudRate, lblKO);

	memset(BRate, 0x00, sizeof(BRate));

	switch (baudRate)
	{
		case 1:
			memcpy(BRate, "115200", 6);
			break;
		case 2:
			memcpy(BRate, "57600", 5);
			break;
		case 3:
			memcpy(BRate, "38400", 5);
			break;
		case 4:
			memcpy(BRate, "19200", 5);
			break;

		case 5:
		default:
			memcpy(BRate, "9600", 4);
			break;
	}

	MAPGET(appChn, bufBRate, lblKO);
	trcFS("saved appChn: %s\n", bufBRate);

	memset(bufBRate, 0x00, sizeof(bufBRate));
	memcpy(bufBRate, "R8N1", sizeof(bufBRate));
	memcpy(&bufBRate[4], BRate, sizeof(BRate));

	trcFS("selected appChn: %s\n", bufBRate);
	MAPPUTSTR(appChn, bufBRate, lblKO);
	ret = kbdVAL;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = -1;
	goto lblEnd;
lblEnd:
	trcFN("admDialup ret= %d\n", ret);
	trcS("admRs232 End\n");
	return ret;
}

/** Configure TMS connection settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTMSConfig(void)
{
	int ret;
	byte dialOpt = 0, counter = 0;
	char Tid[lenTid + 1];
	char refNum[lenTmsRefNum + 1];
	char AcqName[lenName + 1];
	char Aid[lenAID + 1];
	char RespCode[lenRspCod + 1];
	word count = 0; //BRI9
//	card count = 0; SALAH TIPE DATA !!!!!!!

	trcS("admTMSConfig Beg\n");

	if(!tmsLoad)
	{
		ret = scrDlg(infTMSRefNum, regTmsRefNum);
		if(ret != kbdVAL)
		{
			ret = 0;
			goto lblEnd;
		}                       // Cancel, timeout => exit
		ret = scrDlg(infTMSNii, appNII);
		if(ret != kbdVAL)
		{
			ret = 0;
			goto lblEnd;
		}                       // Cancel, timeout => exit
	}

	ret = scrDlg(infTMSHostLink, appDialOpt);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit
	ret = valDialOpt();
	CHECK(ret > 0, lblKO);

	if(!tmsLoad)
	{
		dialOpt = ret;

		switch (dialOpt)
		{
			case 2:
				ret = admDialup();
				if(ret != kbdVAL)
				{
					ret = 0;
					goto lblEnd;
				}                 // Cancel, timeout => exit
				break;
			case 4:
			case 3:
				ret = admTcpipGprsPrm(dialOpt);
				if(ret != kbdVAL)
				{
					ret = 0;
					goto lblEnd;
				}                 // Cancel, timeout => exit
				break;
			case 1:
				ret = admRs232();
				if(ret != kbdVAL)
				{
					ret = 0;
					goto lblEnd;
				}                 // Cancel, timeout => exit
				break;
			default:
				ret = -1;
				goto lblKO;
				break;
		}

		ret = usrInfo(infTMSLoadParams);
		CHECK(ret >= 0, lblKO)

		if((ret == 0) || (ret == kbdANN))
			goto lblEnd;
	}
	tmsLoad = 0;                //return TMSLoad to default value

	mapMove(acqBeg, 0);
	MAPGETWORD(regTrxNo, count, lblKO);
	CHECK(count == 0, lblSettle);   //settle first before tms download

	//move this on per received of file for better processing
	//ret = mapReset(acqBeg);
	//CHECK(ret > 0, lblEnd);
	//ret = mapReset(issBeg);
	//CHECK(ret > 0, lblEnd);
	//ret = mapReset(binBeg);
	//CHECK(ret > 0, lblEnd);
	//ret = mapReset(aidBeg);
	//CHECK(ret > 0, lblEnd);
	//ret = mapReset(pubBeg);
	//CHECK(ret > 0, lblEnd);
	MAPPUTWORD(traMnuItm, mnuTMSLP, lblKO);
	MAPPUTBYTE(traTxnType, trtTMSLOGON, lblKO);
	MAPPUTBYTE(regBinCnt, 0, lblKO);
	MAPPUTBYTE(regIssCnt, 0, lblKO);
	MAPPUTBYTE(regAcqCnt, 0, lblKO);
	MAPPUTBYTE(regAidCnt, 0, lblKO);
	MAPPUTBYTE(regPubCnt, 0, lblKO);

	MAPGET(appTid, Tid, lblKO);
	trcFS("appTid: %s.\n", Tid);
	MAPGET(regTmsRefNum, refNum, lblKO);
	trcFS("regTmsRefNum: %s.\n", refNum);

	ret = onlSession();
	CHECK(ret > 0, lblEnd);

	//ret = prtImg(logo, 32, 16, "c2");    //centered image x4
	//CHECK(ret >= 0, lblKO);
	MAPGET(traRspCod, RespCode, lblKO);
	CHECK(memcmp(RespCode, "00", lenRspCod) == 0, lblEnd);

#ifdef SIMAS_TMS
	storeKeyFromTMS();
#endif

	ret = rptReceipt(rloInitHeader);
	CHECK(ret >= 0, lblKO);
	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, AcqName, lblKO);
		if(AcqName[0] == 0)
			break;
		ret = rptReceipt(rloInitBody);
		CHECK(ret >= 0, lblKO);
	}
	ret = rptReceipt(rloInitEmvHeader);
	CHECK(ret >= 0, lblKO);
	for (counter = 0; counter < dimAID; counter++)
	{
		mapMove(aidBeg, counter);
		MAPGET(emvAid, Aid, lblKO);
		if(Aid[0] == 0)
			break;
		ret = rptReceipt(rloInitEmvBody);
		CHECK(ret >= 0, lblKO);
	}
    
    rptReceipt(rloFeed);
    
	if(dialOpt == 1)            // RS232
		MAPPUTSTR(appChn, "R8N19600", lblKO);

	modifyMnuLayout(); //@@AS0.0.29
	
	goto lblEnd;

lblSettle:
	usrInfo(infPerformStl);
	ret = 0;
	goto lblEnd;
lblKO:
	if(ret != 0)
		usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	tmsLoad = 0;
	trcS("admTMSConfig End\n");
}

/** Sets system date & time
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTDT(void)
{
	int ret;
	char buf[dspW + 1];
	char newdatetime[lenDatTim + 1];

	usrInfo(infEntDat);         // enter date
	memset(newdatetime, 0, sizeof(newdatetime));
	memset(buf, 0, dspW + 1);
	ret = enterDate(1, buf, 6);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit
	MAPPUTSTR(msgVar, buf, lblKO);
	usrInfo(infVarLin2);

	memcpy(&newdatetime[2], &buf[0], 4);    //CC+YYMMDDhhmmss
	memcpy(&newdatetime[0], &buf[4], 2);

	usrInfo(infEntTim);         // enter time
	memset(buf, 0, dspW + 1);
	ret = enterTime(1, buf, 6);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit
	memcpy(&newdatetime[6], buf, 6);

	ret = setDateTime(newdatetime);
	CHECK(ret >= 0, lblKO);
	MAPPUTSTR(msgVar, buf, lblKO);
	usrInfo(infVarLin2);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	;
}

/** Print Terminal Application list
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admTermAppli(void)
{
	stub("APPLICATION");
}

/** Displays communication settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmViewSetting(void)
{
	int ret;
	byte val;
	char stat[dspW + 1];
	word msgKey;

	trcS("admCmmViewSetting Beg\n");

	ret = usrInfo(infScrTonePulse);
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infPabx);
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appToneDetect, val, lblKO);
	msgKey = ((val == 1) ? msgOn : msgOff);
	MAPGET(msgKey, stat, lblKO);
	MAPPUTSTR(regToneDetect, stat, lblKO);
	ret = usrInfo(infScrToneDetect);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admCmmViewSetting End\n");
}

/** Print communication settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmPrintSetting(void)
{
	int ret;
	byte val;
	char stat[dspW + 1];
	word msgKey;

	trcS("admCmmPrintSetting Beg\n");

	ret = usrInfo(infConfigPrinting);
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appToneDetect, val, lblKO);
	msgKey = ((val == 1) ? msgOn : msgOff);
	MAPGET(msgKey, stat, lblKO);
	MAPPUTSTR(regToneDetect, stat, lblKO);

	MAPGETBYTE(appTonePulse, val, lblKO);
	msgKey = ((val == 1) ? msgTone : msgPulse);
	MAPGET(msgKey, stat, lblKO);
	MAPPUTSTR(regTonePulse, stat, lblKO);

	MAPGETBYTE(appTmsDhcp, val, lblKO);
	msgKey = ((val == 1) ? msgOn : msgOff);
	MAPGET(msgKey, stat, lblKO);
	MAPPUTSTR(regDHCP, stat, lblKO);

	ret = rptReceipt(rloCommsConfig);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admCmmPrintSetting End\n");
}

/** Configure dial up mode
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmLineSetup(void)
{
	int ret, key = 0;
	byte val;
	char stat[dspW + 1];
	word msgKey;

	trcS("admCmmLineSetup Beg\n");

	ret = scrDlg(infScrTonePulse, appTonePulse);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = scrDlg(infPabx, appPabx);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	MAPGETBYTE(appToneDetect, val, lblKO);

	msgKey = ((val == 1) ? msgOn : msgOff);
	MAPGET(msgKey, stat, lblKO);

	while(!key)
	{

		MAPPUTSTR(regToneDetect, stat, lblKO);

		ret = usrInfo(infScrToneDetect);
		CHECK(ret >= 0, lblKO);

		trcFN("ret %d\n", ret);
		key = 0;
		switch (ret)
		{
			case kbdDN:          //PREV
			case kbdUP:          //NEXT
				if(val)
				{
					msgKey = msgOff;
					val = 0;
				}
				else
				{
					msgKey = msgOn;
					val = 1;
				}
				break;
			case kbdVAL:         //ENTER
				key = 1;
				break;
			case kbdCOR:         //CANCEL
			case kbdANN:         //STOP
			case 0:              //TIMEOUT
				goto lblEnd;
		}
		MAPGET(msgKey, stat, lblKO);
		CHECK(ret >= 0, lblKO);
	}

	MAPPUTBYTE(appToneDetect, val, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcS("admCmmLineSetup End\n");
}

/** Configure IP settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmIPSetup(void)
{
	int ret;

	trcS("admCmmIPSetup Beg\n");

	ret = scrDlg(infScrEnableDhcp, appTmsDhcp);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = scrDlg(infScrIpAdd, appIpAdd);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = admFmtIp(appIpAdd);   //format ip add
	CHECK(ret > 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = scrDlg(infScrSubMask, appSubMask);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = admFmtIp(appSubMask); //format ip add
	CHECK(ret > 0, lblKO);

	ret = scrDlg(infScrGateway, appDefGateway);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit

	ret = admFmtIp(appDefGateway);  //format ip add
	CHECK(ret > 0, lblKO);

	goto lblEnd;

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcS("admCmmIPSetup End\n");
}

/** Configure EDH settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmEDHSetup(void)
{
	int ret;

	trcS("admCmmEDHSetup Beg\n");

	ret = usrInfo(infScrEnableEdh);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcS("admCmmEDHSetup End\n");
}

/** Set country code for modem setup
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmCountryCode(void)
{
	stub("COUNTRY CODE");
}

/** Set DTMF speed for modem setup
 * \header sys\\log.h
 * \source sys\\adm.c
*/
void admCmmDTMF(void)
{
	stub("DTMF SPEED");
}


/** Set GPRS configuration
 * \header sys\\log.h
 * \source sys\\adm.c
*/
void admCmmGConfigure(void) {
    int ret;
    char GprsApn[lenApn + 1];
	char apn_cetak[lenApn + 1];
    char buf[lenApn - 2], buf2[lenApn - 2];//, buf3[lenApn - 2];
    char key;
	
	char temp[64+1];
	char ip1[32+1];
	char ip2[32+1];
	char ip3[32+1];
	char ip[48+1];

	memset(buf, 0, sizeof(buf));
	// reset it first
	memset(GprsApn, 0, sizeof(GprsApn));
	memset(temp, 0, sizeof(temp));
	memset(ip1, 0, sizeof(ip1)); 
	memset(ip2, 0, sizeof(ip2)); 
	memset(ip3, 0, sizeof(ip3));
	memset(ip, 0, sizeof(ip));
	
    MAPGET(acqPriTxnIPSrv, ip1, lblEnd);
	MAPGET(acqSecTxnIPSrv, ip2, lblEnd);
	MAPGET(acqSecTxnIPSrv, ip3, lblEnd);
	
	memset(GprsApn, 0, sizeof(GprsApn));
	memset(apn_cetak, 0, sizeof(apn_cetak));

	MAPGET(appGprsApn, GprsApn, lblKO);
		
	dspClear();
	display_text(1, 0, "IP & PORT:", 		  	dsp_ALIGN_LEFT);
	display_text(2, 0, ip1, 		 		  	dsp_ALIGN_LEFT);
	display_text(4, 0, "apn|user name|pass:", 	dsp_ALIGN_LEFT);
	display_text(5, 0, GprsApn, 			  	dsp_ALIGN_LEFT);
	display_text(7, 0, "F1-No          F4-Yes", dsp_ALIGN_LEFT);
	key = acceptable_key(kbdF1_kbdF4, 60);
	CHECK(key != kbdF4, lblTestGPRS);
	CHECK(key == kbdF1, lblEnd);
	
	// APN baru
	dspClear();
    dspLS(0|0x40,"      APN");        
    memset(buf,0,sizeof(buf));
    dspLS(1,"New APN :");
    ret = enterTxt(2,buf,32, 0xFF);
    if(ret < 0)
        goto lblKO;
	if(ret != kbdVAL)
        goto lblEnd;
    
    memset(buf2, 0, sizeof(buf2));
    strcat(buf2, "|");
    strcat(buf2, buf);
    strcat(buf2, "|");
    
    // User Name Baru
	dspClear();
    dspLS(0|0x40,"  USER NAME");        
    memset(buf,0,sizeof(buf));
    dspLS(1,"New User Name :");
    ret = enterTxt(2,buf,16, 0xFF);
    if(ret < 0)
        goto lblKO;
	if(ret != kbdVAL)
        goto lblEnd;
    
	strcat(buf2, buf);
    strcat(buf2, "|");
    
	// Password Baru
	dspClear();
    dspLS(0|0x40,"   PASSWORD ");        
    memset(buf,0,sizeof(buf));
    dspLS(1,"New Password :");
    ret = enterTxt(2,buf,16, 0xFF);
    if(ret < 0)
        goto lblKO;
	if(ret != kbdVAL)
        goto lblEnd;
    
	strcat(buf2, buf);
    strcat(buf2, "|");
    
    MAPPUTSTR(appGprsApn, buf2, lblKO);
    reboot();
    goto lblEnd;
    
lblTestGPRS:
/*	dspClear(); dspLS(0, "Starting GPRS...");
    if(comStart(chnGprs) < 0){
		
		usrInfo(infComStartKO);
		//goto lblEnd;
		goto lblKO;
	}
    
	dspLS(1, "COMSTART OK");

	// set configuration
    if(comSet(GprsApn) < 0){

		usrInfo(infComSetKO);
		//goto lblEnd;
		goto lblKO;
	}

	dspLS(2, "COMSET OK");	

	strcpy(ip,ip1); strcat(ip,"|");
	strcpy(ip,ip2); strcat(ip,"|");
	strcpy(ip,ip3); strcat(ip,"|");

	// set ip/port
	if(comDial(ip) < 0){

		usrInfo(infComDialKO);
		//goto lblEnd;
		goto lblKO;
	}

	dspLS(3, "COMDIAL OK");	
	
	ret = comHangStart();
	CHECK(ret >= 0, lblKO);
	
	ret = comHangWait();
	CHECK(ret >= 0, lblKO);

	//ret = comStop();
       //CHECK(ret >= 0, lblKO);

	usrInfo(infSuccConfig);
*/
	goto lblEnd;
	
	lblKO:
		//comStop();
		ret = -1;
		usrInfo(infProcessingError);
	lblEnd:
		;

}
/*void admCmmGConfigure(void) {
    int ret;
    char GprsApn[lenApn + 1];
	char apn_cetak[lenApn + 1];
    char buf[lenApn - 2];
    char key;
	
	char temp[64+1];
	char ip1[32+1];
	char ip2[32+1];
	char ip3[32+1];
	char ip[48+1];

	memset(buf, 0, sizeof(buf));
	// reset it first
	memset(GprsApn, 0, sizeof(GprsApn));
	memset(temp, 0, sizeof(temp));
	memset(ip1, 0, sizeof(ip1)); 
	memset(ip2, 0, sizeof(ip2)); 
	memset(ip3, 0, sizeof(ip3));
	memset(ip, 0, sizeof(ip));
	
    MAPGET(acqPriTxnIPSrv, ip1, lblEnd);
	MAPGET(acqSecTxnIPSrv, ip2, lblEnd);
	MAPGET(acqSecTxnIPSrv, ip3, lblEnd);
	
	memset(GprsApn, 0, sizeof(GprsApn));
	memset(apn_cetak, 0, sizeof(apn_cetak));

	MAPGET(appGprsApn, GprsApn, lblKO);
		
	dspClear();
	display_text(1, 0, "IP & PORT:", 		  	dsp_ALIGN_LEFT);
	display_text(2, 0, ip1, 		 		  	dsp_ALIGN_LEFT);
	display_text(4, 0, "apn|user name|pass:", 	dsp_ALIGN_LEFT);
	display_text(5, 0, GprsApn, 			  	dsp_ALIGN_LEFT);
	display_text(7, 0, "F1-No          F4-Yes", dsp_ALIGN_LEFT);
	key = acceptable_key(kbdF1_kbdF4, 60);
	CHECK(key == kbdF4, lblEnd);
	
	dspClear(); dspLS(0, "Starting GPRS...");
    if(comStart(chnGprs) < 0){
		
		usrInfo(infComStartKO);
		//goto lblEnd;
		goto lblKO;
	}
    
	dspLS(1, "COMSTART OK");

	// set configuration
    if(comSet(GprsApn) < 0){

		usrInfo(infComSetKO);
		//goto lblEnd;
		goto lblKO;
	}

	dspLS(2, "COMSET OK");	

	strcpy(ip,ip1); strcat(ip,"|");
	strcpy(ip,ip2); strcat(ip,"|");
	strcpy(ip,ip3); strcat(ip,"|");

	// set ip/port
	if(comDial(ip) < 0){

		usrInfo(infComDialKO);
		//goto lblEnd;
		goto lblKO;
	}

	dspLS(3, "COMDIAL OK");	
	
	ret = comHangStart();
	CHECK(ret >= 0, lblKO);
	
	ret = comHangWait();
	CHECK(ret >= 0, lblKO);

	//ret = comStop();
       //CHECK(ret >= 0, lblKO);

	usrInfo(infSuccConfig);

	goto lblEnd;
	
	lblKO:
		//comStop();
		ret = -1;
		usrInfo(infProcessingError);
	lblEnd:
		;

}*/

/** Toggle GPRS encryption mode
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmGEncryption(void)
{
	int ret;

	trcS("admCmmGEncryption Beg\n");

	ret = usrInfo(infScrGprsEncryption);
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("admCmmGEncryption: ret=%d\n", ret);
	return;
}

/** Toggle GPRS auto enable option
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admCmmGAutoEnable(void)
{
	stub("AUTO ENABLE");
}

/** Loading of test key
 * \header sys\\log.h
 * \source sys\\adm.c
 */
#if 0
void admMasterKey(void)
{
	byte ucLoc;
	byte tucCrt[4];
	char tcTmp[prtW + 1];
	byte tucKey[16];
	const byte *MKey = (byte *) "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD"; // 0123456789ABCDEF encrypted by 1111111111111111
	const byte *WKey = (byte *) "\xBD\xA8\x39\x20\x6C\x3A\xE2\x46"; //WK, working key is 6B218F24DE7DC66C
	int ret;
	byte CrtLen;

#ifdef __APR__

	CrtLen = 3;
#else

	CrtLen = 4;
#endif

	memset(tcTmp, 0, prtW + 1);
	memset(tucCrt, 0, sizeof(tucCrt));
	trcS("admMasterKey Beg\n");

	ret = dspClear();
	CHECK(ret >= 0, lblKO);
	ret = cryStart('m');
	CHECK(ret >= 0, lblKO);

	ret = usrInfo(infTestKeyLoad);
	CHECK(ret >= 0, lblKO);
	ucLoc = __RootKeyLoc__;
	ret = cryLoadTestKey(ucLoc);
	CHECK(ret >= 0, lblKO);
	ret = cryVerify(ucLoc, tucCrt);
	CHECK(ret >= 0, lblKO);

	strcpy(tcTmp, "TK ");
	ret = bin2hex(&tcTmp[3], &ucLoc, 1);
	CHECK(ret == 2, lblKO);
	strcat(tcTmp, ": ");
	VERIFY(strlen(tcTmp) == 7);
	ret = bin2hex(&tcTmp[7], tucCrt, 4);
	CHECK(ret == 8, lblKO);
	//TODO rptReceipt();
	ret = prtS(tcTmp);
	CHECK(ret >= 0, lblKO);

	MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
	CHECK(ret >= 0, lblKO);
	usrInfo(infVarLin1);
	//CHECK(memcmp(tucCrt, "\x82\xE1\x36\x65", CrtLen) == 0, lblKO);

	// Download the master key \x01\x23\x45\x67\x89\xAB\xCD\xEF into array 2 slot 4 using transport key array 1 slot 0
	usrInfo(infLodMstKey);
	memcpy(tucKey, MKey, 8);
	memcpy(&tucKey[8], MKey, 8);

	//MAPGETBYTE(appMKeyLoc, ucLoc, lblDBA);
	ucLoc = __appMKeyLoc__;
	ret = cryLoadDKuDK(ucLoc, __RootKeyLoc__, tucKey, 'g');
	CHECK(ret >= 0, lblKO);
	ret = cryVerify(ucLoc, tucCrt);
	CHECK(ret >= 0, lblKO);

	strcpy(tcTmp, "MK ");
	ret = bin2hex(&tcTmp[3], &ucLoc, 1);
	CHECK(ret == 2, lblKO);
	strcat(tcTmp, ": ");
	VERIFY(strlen(tcTmp) == 7);
	ret = bin2hex(&tcTmp[7], tucCrt, 4);
	CHECK(ret == 8, lblKO);
	//TODO rptReceipt();
	ret = prtS(tcTmp);
	CHECK(ret >= 0, lblKO);
	tmrPause(1);

	MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
	usrInfo(infVarLin3);
	//CHECK(memcmp(tucCrt, "\xD5\xD4\x4F\xF7", CrtLen) == 0, lblKO);  // It is the certificate of the key 0123456789ABCDEF

	//TODO usrInfo();
	//ret = dspLS(2, "Load Working Key ");
	//CHECK(ret >= 0, lblKO);
	usrInfo(infLodWrkKey);
	memcpy(tucKey, WKey, 8);
	memcpy(&tucKey[8], WKey, 8);
	ucLoc = __appPinKeyLoc__;

	ret = cryLoadDKuDK(ucLoc, __appMKeyLoc__, tucKey, 'p');
	CHECK(ret >= 0, lblKO);
	ret = cryVerify(ucLoc, tucCrt);
	CHECK(ret >= 0, lblKO);

	strcpy(tcTmp, "WK ");
	ret = bin2hex(&tcTmp[3], &ucLoc, 1);
	CHECK(ret == 2, lblKO);
	strcat(tcTmp, ": ");
	VERIFY(strlen(tcTmp) == 7);
	ret = bin2hex(&tcTmp[7], tucCrt, 4);
	CHECK(ret == 8, lblKO);
	//TODO rptReceipt();
	ret = prtS(tcTmp);
	CHECK(ret >= 0, lblKO);
	tmrPause(1);

	MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
	usrInfo(infVarLin3);
	//CHECK(memcmp(tucCrt, "\xF7\x53\xC2\x98", CrtLen) == 0, lblKO);  //it is the certificate of the key 6B218F24DE7DC66C

	//TODO rptReceipt();
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	cryStop('m');
	trcS("admMasterKey End\n");
}
#endif

 void admMasterKey(void) {
    byte ucLoc;
    byte tucCrt[4];
    char tcTmp[prtW + 1];
    byte tucKey[16];
//    const byte *MKey = (byte *) "\x11\x11\x11\x11\x11\x11\x1\x11"; // 0123456789ABCDEF encrypted by 1111111111111111
//    const byte *WKey = (byte *) "\xF4\x03\x79\xAB\x9E\x0E\xC5\x33"; //WK, working key is 6B218F24DE7DC66C
     

//    byte MdefKey[8] = "\x11\x11\x11\x11\x11\x11\x11\x11";
//    byte WdefKey[8] = "\xF4\x03\x79\xAB\x9E\x0E\xC5\x33";
    
//    byte MdefKey[8] = "\xF4\x03\x79\xAB\x9E\x0E\xC5\x33";
//    byte WdefKey[8] = "\x11\x11\x11\x11\x11\x11\x11\x11";  
/*
    byte MdefKey[8] = "\x2B\x2B\x2B\x2B\x2B\x2B\x2B\x2B";  
    byte WdefKey[8] = "\xF4\x03\x79\xAB\x9E\x0E\xC5\x33";    
*/
#ifdef SAT_PROD
    byte MdefKey[16] = "\x49\x2B\x27\x4F\x0A\xB4\xDB\xCE\x49\x2B\x27\x4F\x0A\xB4\xDB\xCE";  //Encrypted under Transport key
    byte WdefKey[16] = "\x17\x66\x8D\xFC\x72\x92\x53\x2D\x17\x66\x8D\xFC\x72\x92\x53\x2D"; //Encrypted under MK
#else //SAT_PROD
    byte MdefKey[16] = "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD";  //Encrypted under Transport key
    byte WdefKey[16] = "\x17\x66\x8D\xFC\x72\x92\x53\x2D\x17\x66\x8D\xFC\x72\x92\x53\x2D"; //Encrypted under MK
#endif //SAT_PROD
    int ret;
    byte CrtLen;
    byte defaultKey = 0;
	
	byte keyD[50]; //download key

#ifdef __APR__
    CrtLen = 3;
#else
    CrtLen = 4;
#endif

    memset(tcTmp, 0, prtW + 1);
    memset(tucCrt, 0, sizeof(tucCrt));
    trcS("admMasterKey Beg\n");
    
    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');

    CHECK(ret >= 0, lblKO);
//    ret = usrInfo(infTestKeyLoad);
    CHECK(ret >= 0, lblKO);  
    ucLoc = __RootKeyLoc__;
    ret = cryLoadTestKey(ucLoc);
    CHECK(ret >= 0, lblKO);
    ret = cryVerify(ucLoc, tucCrt);
    CHECK(ret >= 0, lblKO);
    strcpy(tcTmp, "TK ");
    ret = bin2hex(&tcTmp[3], &ucLoc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tcTmp, ": ");
    VERIFY(strlen(tcTmp) == 7);
    ret = bin2hex(&tcTmp[7], tucCrt, 4);
    CHECK(ret == 8, lblKO);
    //TODO rptReceipt();
//    ret = prtS(tcTmp);
//    CHECK(ret >= 0, lblKO);

    MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
    CHECK(ret >= 0, lblKO);
//    usrInfo(infVarLin1);
    //CHECK(memcmp(tucCrt, "\x82\xE1\x36\x65", CrtLen) == 0, lblKO);

// Download the master key \x01\x23\x45\x67\x89\xAB\xCD\xEF into array 2 slot 4 using transport key array 1 slot 0
    usrInfo(infLodMstKey);
    
//#ifndef DEFAULT_TMK
//    ret = downloadKey(keyD,sizeof(keyD));
//    if(ret >= 0)
//    {
//        memcpy(tucKey,keyD,16);
//        defaultKey = 0;
//    }
//#else
//    memcpy(tucKey, MdefKey, 8);
//    memcpy(&tucKey[8], MdefKey, 8);
//    defaultKey = 1;
//#endif

#if 0
// awal download key    
    ret = downloadKey(keyD,sizeof(keyD));
    if(ret >= 0)
//    	goto lblKO;
    {
        memcpy(tucKey,keyD,16);
        defaultKey = 0;
//ShowData(keyD,32,0,1,8);  //hapus
    }
    else
//akhir download key

//tutup untuk download key    
    {
        memcpy(tucKey, MdefKey, 8);
        memcpy(&tucKey[8], MdefKey, 8);
        defaultKey = 1;
    }
#endif 

        memcpy(tucKey, MdefKey, 8);
        memcpy(&tucKey[8], MdefKey, 8);
        defaultKey = 1;
       
    //MAPGETBYTE(appMKeyLoc, ucLoc, lblDBA);
    ucLoc = __appMKeyLoc__;
//prtS("key 1:");    
//ShowData(tucKey,16,0,1,8); //hapus  
//encrypt(tucKey,3DES,trKey, tucKeyEcnry)
  
    ret = cryLoadDKuDK(ucLoc, __RootKeyLoc__, tucKey, 'g');
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(ucLoc, tucCrt);
    CHECK(ret >= 0, lblKO);
    strcpy(tcTmp, "MK ");
    ret = bin2hex(&tcTmp[3], &ucLoc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tcTmp, ": ");
    VERIFY(strlen(tcTmp) == 7);
    ret = bin2hex(&tcTmp[7], tucCrt, 4);
    CHECK(ret == 8, lblKO);
    //TODO rptReceipt();
//    ret = prtS(tcTmp);
//    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
//    usrInfo(infVarLin3);
    //CHECK(memcmp(tucCrt, "\xD5\xD4\x4F\xF7", CrtLen) == 0, lblKO);  // It is the certificate of the key 0123456789ABCDEF

    //TODO usrInfo();
    //ret = dspLS(2, "Load Working Key ");
    //CHECK(ret >= 0, lblKO);
    usrInfo(infLodWrkKey);
// awal download key  
    if(defaultKey == 0)
        memcpy(tucKey, keyD+16, 16);
// akhir  download key      

//tutup untuk download key
    else
    {        
        memcpy(tucKey, WdefKey, 8);
        memcpy(&tucKey[8], WdefKey, 8);
    }
    ucLoc = __appPinKeyLoc__;
//prtS("key 2:");    
//ShowData(tucKey,16,0,1,8); //hapus  
    ret = cryLoadDKuDK(ucLoc, __appMKeyLoc__, tucKey, 'p');
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(ucLoc, tucCrt);
    CHECK(ret >= 0, lblKO);

    strcpy(tcTmp, "WK ");
    ret = bin2hex(&tcTmp[3], &ucLoc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tcTmp, ": ");
    VERIFY(strlen(tcTmp) == 7);
    ret = bin2hex(&tcTmp[7], tucCrt, 4);
    CHECK(ret == 8, lblKO);
    //TODO rptReceipt();
//    ret = prtS(tcTmp);
//    CHECK(ret >= 0, lblKO);
    tmrPause(1);
	 dspClear();
	 dspLS(2,"LOAD KEY SUCCESS");
	 tmrPause(3);
    MAPPUTSTR(msgVar, &tcTmp[3], lblKO);
//    usrInfo(infVarLin3);
    //CHECK(memcmp(tucCrt, "\xF7\x53\xC2\x98", CrtLen) == 0, lblKO);  //it is the certificate of the key 6B218F24DE7DC66C

    //TODO rptReceipt();
    goto lblEnd;
  lblKO:
    usrInfo(infProcessingError);
    goto lblEnd;
  lblEnd:
    cryStop('m');
    trcS("admMasterKey End\n");
}
/** Configure Transaction connection settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admConnSettings(void)
{
	int ret;

	trcS("admConnSettings Beg\n");

	ret = scrDlg(infConnMethod, appConnType);
	if(ret != kbdVAL)
	{
		ret = 0;
		goto lblEnd;
	}                           // Cancel, timeout => exit
	ret = valConnSett();
	CHECK(ret > 0, lblKO);

	admChangeConnSettings(ret);

	if(ret == 3)                // rs232 connection
	{
		ret = scrDlg(infPortSetting, appComType);
		if(ret != kbdVAL)
		{
			ret = 0;
			goto lblEnd;
		}                       // Cancel, timeout => exit
	}

	goto lblEnd;

lblKO:
	if(ret != 0)
		usrInfo(infInvalidInput);
lblEnd:
	trcS("admConnSettings End\n");
}

// Additional Function
// BlowFish Admin
static void admChangeBfSetting(byte BlowSet){

    int ret;
	
    trcS("admChangeConnSettings Beg\n");
    MAPPUTBYTE(appBlowFish, BlowSet, lblKO);
    switch (BlowSet) {
      case 1:
          MAPPUTSTR(appBf, "0", lblKO);
          break;
      case 2:
          MAPPUTSTR(appBf, "1", lblKO);
          break;
      default:
          ret = -1;
          goto lblKO;
    }

    goto lblEnd;

  lblKO:
    if(ret != 0)
        usrInfo(infInvalidInput);
  lblEnd:
    trcS("admChangeConnSettings End\n");
}

/** Configure Blowfish settings
 * \header sys\\log.h
 * \source sys\\adm.c
 */
void admBlowfish(void){
	int ret;
	char buf[16 + 1];

    trcS("admBlowfish Beg\n");

    ret = scrDlg(infBlowfishMethod, appBlowFish);
    if(ret != kbdVAL) {
        ret = 0;
        goto lblEnd;
    }                           // Cancel, timeout => exit

	ret = ValBfSett();
    CHECK(ret > 0, lblKO);

    admChangeBfSetting(ret);

	if(ret == 2)                // BlowFish On
    {

		dspClear();
		dspLS(1, "Enter The Key");
		
		memset(buf, 0, 16 + 1);
	    ret = enterStr(2, buf, 16+1);
	    CHECK(ret >= 0, lblKO);

		mapPutStr(appBfKey, buf);

		if(ret != kbdVAL) {
	        ret = 0;
	        goto lblEnd;
	    }
    }


    goto lblEnd;

  lblKO:
    if(ret != 0)
        usrInfo(infInvalidInput);
  lblEnd:
    trcS("admBlowfish End\n");
}

void admEDCFunction(void)
{
	int ret;
	int nbr_func;
	char buff[4];
	dspClear();
	dspLS(2, "EDC FUNCTION ?");
	memset(buff,0,sizeof buff);
	ret = enterStr(3, buff, 4);
	CHECK(ret, lblEnd);
	CHECK(ret == kbdVAL, lblEnd);
	nbr_func = atoi(buff);
	call_function(nbr_func);
	lblEnd:
		trcS("admEDCF End\n");
}


