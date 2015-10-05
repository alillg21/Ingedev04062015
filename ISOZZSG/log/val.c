//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/val.c $
//$Id: val.c 2077 2009-06-04 07:27:49Z ajloreto $

/** \file
 * Purpose :
 *  Validation treatment. The cardholder-related functionality such  as card, merchant, access rights,
 * financial and administrative validations.
 */

#include <string.h>
#include "log.h"
#include "bri.h"

/** Validate the access to the item selected from the menu.
 *  The first character of the menu item is used to hold the menu level.
 *  If it is a question mark, the menu item is disabled (will never appear in the menu).
 *  The second character is used to hold the access type.
 *  The value 'm' means that merchant password is needed; 'a' requires administrator password; 'space' means that the menu item is not
 *  password-protected.
 *
 * \header log\\log.h
 * \source log\\val.c
*/
int valOpr(void)
{
	word usMnuItm;
	char tcBuf[lenMnu + 1];
	word usKey;
	char tcPwd1[lenPwd + 1];
	char tcPwd2[lenPwd + 1];
	int ret;

	trcS("valOpr Beg\n");
	MAPGETWORD(traMnuItm, usMnuItm, lblKO);
	MAPGET(usMnuItm, tcBuf, lblKO);
	switch (tcBuf[1])
	{         // The second character of menu item string control the access type
		case 'm':
			usKey = appMrcPwd;
			break;                // Merchant password required
		case 'a':
			usKey = appAdmPwd;
			break;                // Administrator password required
		default:
			ret = 1;
			goto lblEnd;          // No password required
	}
	ret = dspClear();

	CHECK(ret >= 0, lblKO);     // Clear screen
	memset(tcPwd1, 0, lenPwd + 1);
	memset(tcPwd2, 0, lenPwd + 1);
	ret = usrInfo(infPwdInput);
	CHECK(ret >= 0, lblKO);     // Msg PwdInput on screen
	ret = enterPwd(2, tcPwd1, lenPwd + 1);
	CHECK(ret >= 0, lblKO);     // Enter the password
	CHECK(ret == kbdVAL, lblAbort);
	MAPGET(usKey, tcPwd2, lblKO);
	CHECK(strcmp(tcPwd1, tcPwd2) == 0, lblBadPwd);  // Compare the 2 passwords

	ret = 1;
	goto lblEnd;

lblBadPwd:
	usrInfo(infWrongPin);       // Msg WrongPin on screen
	ret = 0;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblAbort:                    // Abort by operator
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valOpr: ret=%d\n", ret);
	return ret;
}

/** Look for a BINLOGO containing for given PAN.
 * \n.
 * \return
 *  This function has return value of binLogo.
 * \header log\\log.h
 * \source log\\val.c
 */

byte valBinLogoAssign(const char *Pan)
{
	byte binLogo = 0;

	if(memcmp("4", &Pan[0], 1) == 0)
		binLogo = binLogoVisa;
	else if(memcmp("5", &Pan[0], 1) == 0)
		binLogo = binLogoMC;
	else if(memcmp("305", &Pan[0], 3) == 0)
		binLogo = binLogoJCB;
	else if(memcmp("36", &Pan[0], 2) == 0)
		binLogo = binLogoJCB;
	else if(memcmp("38", &Pan[0], 2) == 0)
		binLogo = binLogoJCB;
	else if(memcmp("37", &Pan[0], 2) == 0)
		binLogo = binLogoAmex;
	else if(memcmp("34", &Pan[0], 2) == 0)
		binLogo = binLogoAmex;

	return binLogo;

}

/** Look for a BIN range containing the given PAN. A loop is
 *  organized to move the cursor in BIN table.
 * \n If a BIN range is found, the BIN record is retrieved to treat parameters.
 * \return
 *  This function has return value.
 *    -  >0 : Validation done.
 *    - <=0 : Validation failed.
 * \header log\\log.h
 * \source log\\val.c
 */

int valBin(void)
{
	char Pan[lenPan + 1];
	byte cardBin[lenBinPrefix + 1];
	byte Bin[lenBinPrefix + 1];
	byte binId, IssId, AcqId;
	int ret;
	char localBinName[lenName + 1];
	byte binLogo;

	//++ @agmr
	byte debitIssId, debitAcqId;
	byte cardType=CREDIT_CARD;
//	byte kategoriKartu;
	char buf[50];
    //-- @agmr

	trcS("valBin Beg\n");
	MAPGET(traPan, Pan, lblDBA);
	trcFS("valBin: Pan=%s\n", Pan);
	ret = hex2bin(cardBin, Pan, lenBinPrefix);
	CHECK(ret >= 0, lblDBA);

	for (binId = 0; binId < dimBin; binId++)
	{  // Analyse BIN multi records
		ret = mapMove(binBeg, (word) binId);
		CHECK(ret >= 0, lblDBA);    // Move to the next record
		MAPGET(binLo, Bin, lblDBA);
		if(memcmp(cardBin, &Bin[1], lenBinPrefix) < 0)  // BIN >= low BIN?
			continue;           // No, move to the next record
		MAPGET(binHi, Bin, lblDBA);
		if(memcmp(cardBin, &Bin[1], lenBinPrefix) < 0)  // BIN <= high BIN?
			break;              // Yes, BIN found => exit
	}
	CHECK(binId < dimBin, lblBinNotFound);  // End of BIN table, BIN not found

	binLogo = valBinLogoAssign(Pan);

	MAPPUTBYTE(binLogoID, binLogo, lblDBA);

	MAPGET(binName, localBinName, lblDBA);
	MAPPUTSTR(traCardType, localBinName, lblDBA);
	MAPPUTBYTE(traBinIdx, binId, lblDBA);
	//Acquirer and Issuer from TMS uses the Credit Acquirer and Issuer
	MAPGETBYTE(binCrAcqID, AcqId, lblDBA);
	MAPGETBYTE(binCrIssID, IssId, lblDBA);

	MAPGETBYTE(binDbAcqID, debitAcqId, lblDBA);
	MAPGETBYTE(binDbIssID, debitIssId, lblDBA);

//++ @agmr
//	if(debitIssId != 0)
//		IssId = debitIssId;
	if(debitAcqId != 0)
		cardType = DEBIT_CARD;

    if(AcqId != 0 && debitAcqId != 0) //NON BRI CARD
    {
        memset(buf,0,sizeof(buf));
        mapMove(issBeg, (word) debitIssId-1);
        MAPGET(issRef,buf,lblDBA);
        if(strcmp(buf,"1000")!=0)
        {
            cardType = NON_BRI_DEBIT_CARD;
            IssId = debitIssId;
        }
        else
        {
            cardType = NON_BRI_CREDIT_CARD;
        }
    }
    else //BRI CARD
    {
        if(debitIssId != 0)
            IssId = debitIssId;
    }

    MAPPUTBYTE(traKategoriKartu,cardType,lblDBA);

    ret = briGetAcq(cardType,&AcqId);
    if(ret < 0)
        goto lblDBA;
    if(ret == 0)
        goto lblAcqNotFound;



//-- @agmr

	trcFN("AcqId: %d\n", AcqId);
	trcFN("IssId: %d\n", IssId);
	AcqId--;                    //Acquirer Id is 1 higher than the actual data base index for Acquirer
	IssId--;                    //Issuer Id is 1 higher than the actual data base index for Issuer
	CHECK(IssId < dimIss, lblDBA);
	CHECK(AcqId < dimAcq, lblDBA);
	MAPPUTBYTE(traIssIdx, IssId, lblDBA);
	MAPPUTBYTE(traAcqIdx, AcqId, lblDBA);
	mapMove(issBeg, (word) IssId);
	mapMove(acqBeg, (word) AcqId);

	ret = 1;
	goto lblEnd;

//@agmr - BRI3
lblAcqNotFound:
    usrInfo(infAcqNotFound);
    ret = 0;
    goto lblEnd;
//@agmr - BRI3

lblBinNotFound:
	usrInfo(infBinNotFound);
	ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valBin: ret=%d\n", ret);
	return ret;
}

/** This function checks the PAN (length and luhn) and the BIN (if exists a
 *  BIN range containing this card).
 * \return
 *  This function has return value.
 *    -  >0 : Validation done.
 *    - <=0 : Validation failed.
 * \header log\\log.h
 * \source log\\val.c
*/
static int valPan(void)
{
	char tcPan[lenPan + 1];
	int ret;
	byte txnType; //@agmr
	byte tmp;     //@agmr

	trcS("valPan Beg\n");
	MAPGET(traPan, tcPan, lblDBA);
	trcFS("valPan: Pan=%s\n", tcPan);
	CHECK(strlen(tcPan) >= 11, lblInvalidPan);  // Check Pan length
	CHECK(strlen(tcPan) <= 19, lblInvalidPan);

	ret = stdLuhnCheck(tcPan);
	CHECK(ret > 0, lblInvalidLuhn); // Check Pan Luhn
	ret = valBin();
	CHECK(ret > 0, lblStop);    // Check BIN from table

//++ @agmr
    MAPGETBYTE(traTxnType,txnType,lblDBA);
    MAPGETBYTE(traKategoriKartu,tmp,lblDBA)
    //installment hanya bisa kartu BRI CREDIT
    if(txnType == trtInstallment)
    {
        if(tmp != CREDIT_CARD)
        {
            usrInfo(infInvalidCard);
            ret = -100;
            goto lblEnd;
        }
    }
    //debit card tidak bisa card ver
    else if(txnType == trtAut)
    {
        if(tmp == DEBIT_CARD || tmp == NON_BRI_DEBIT_CARD)
        {
            usrInfo(infTransNotAllowed);
            ret = -100;
            goto lblEnd;
        }
    }

//-- @agmr

	ret = 1;
	goto lblEnd;

lblInvalidPan:
	usrInfo(infInvalidPan);
	ret = 0;
	goto lblEnd;
lblInvalidLuhn:
	usrInfo(infInvalidLuhn);
	ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblStop:                     // Execution interrupted
	if(!ret)                    // ret==0 means that the exception is already processed
		goto lblEnd;
	usrInfo(infProcessingError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valPan: ret=%d\n", ret);
	return ret;
}

/** This function validates whether the card is expired.
 * \return
 *  This function has return value.
 *    -  >0 : Validation done.
 *    - <=0 : Validation failed.
 * \header log\\log.h
 * \source log\\val.c
*/
static int valExpDat(void)
{
	char tcExpDat[lenExpDat + 1];
	char tcDatTim[lenDatTim + 1];
	int ret;

	trcS("valExpDat Beg\n");
	MAPGET(traExpDat, tcExpDat, lblDBA);
	MAPGET(traDatTim, tcDatTim, lblDBA);
	trcFS("valExpDat: ExpDat=%s\n", tcExpDat);
	CHECK(strlen(tcExpDat) == 4, lblInvalidInput);
	CHECK(memcmp(tcExpDat, &tcDatTim[2], 4) >= 0, lblCardExpired);
	ret = 1;
	goto lblEnd;

lblCardExpired:
	usrInfo(infCardExpired);
	ret = 0;
	goto lblEnd;
lblInvalidInput:
	usrInfo(infInvalidInput);
	ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valExpDat: ret=%d\n", ret);
	return ret;
}

/** Validates card entered; move cursors in bin, acq and iss tables to proper positions.
 * \header log\\log.h
 * \source log\\val.c
 */
int valCard(void)
{             //AB: to be implemented
	int ret;
	byte TxnType;

	trcS("valCard Beg\n");
	ret = valPan();
	CHECK(ret > 0, lblStop);    // Check Pan

	ret = valExpDat();
	//CHECK(ret > 0, lblStop);    // Check expiration date

	MAPGETBYTE(traTxnType, TxnType, lblBarred);
	if(TxnType == trtManual)
	{
		ret = valManualEntryAllowed();
		CHECK(ret >= 0, lblBarred); // manual entry is not allowed in MOTO or on issuer
	}

	ret = 1;
	goto lblEnd;

lblBarred:
	usrInfo(infManualBarred);
	ret = 0;
	goto lblEnd;
lblStop:                     // Execution interrupted
//++ @agmr
    if(ret == -100)
        goto lblEnd;
//-- @agmr
	if(!ret)                    // ret==0 means that the exception is already processed
		goto lblEnd;
	usrInfo(infProcessingError);    // Msg ProcessingError on screen
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valCard: ret=%d\n", ret);
	return ret;
}

/** Validate amount: check min and max value
 * \header log\\log.h
 * \source log\\val.c
 */
int valAmount(void)
{
	int ret;

	trcS("valAmount Beg\n");

// ++ @agmr - BRI- inquriy transaction
    {
    	byte tmp;

        MAPGETBYTE(traTxnType,tmp,lblKO);
        switch(tmp)
        {
            case trtInfoSaldo:
            case trtInfoSaldoBankLain:
	     	case trtMiniStatement:
	    	case trtMutRek:
		 	case trtPrevilege:
            case trtTransferSesamaBRIInquiry:
            case trtTransferSesamaBRI:
            case trtTransferAntarBankInquiry:
            case trtTransferAntarBank:
			/*Changed By Ali*/
			case trtPembayaranTVBerlangganan:
			case trtPembayaranTVBerlanggananInquiry:
			case trtPembayaranTiketPesawat:
			case trtPembayaranTiketPesawatInquiry:
			case trtPembayaranPdam:
			case trtPembayaranPdamInquiry:
			case trtPembayaranTelkom:
			case trtPembayaranTelkomInquiry:
			case trtPembayaranDPLKR:
			case trtPembayaranBrivaInquiry:
			case trtPembayaranBriva:
			case trtRegistrasiSmsBanking:
			case trtTrxPhoneBankingTransfer:
			case trtTrxPhoneBankingTransferInquiry:
			case trtTrxPhoneBankingPulsa:
			case trtPembayaranKKBRIInquiry:
			case trtPembayaranKKBRI:
			case trtPembayaranKKANZ:
			case trtPembayaranKKCITIKK:
			case trtPembayaranKKCITIKTA:
			case trtPembayaranKKHSBCKK:
			case trtPembayaranKKHSBCKTA:
			case trtPembayaranKKRBSKK:
			case trtPembayaranKKRBSKTA:
			case trtPembayaranKKStanCharKK:
			case trtPembayaranKKStanCharKTA:
			case trtPembayaranSPPInquiry:
			case trtPembayaranSPP:
			 case trtPembayaranPLNPascaInquiry:
            		case trtPembayaranPLNPasca:
			case trtPembayaranPLNPraInquiry:
            		case trtPembayaranPLNPra:
			case trtPembayaranPLNToken:
			#ifdef SSB
			case trtPembayaranSSBInquiry:
			case trtPembayaranSSB:
			#endif
			/*End of Changed By Ali*/
            case trtPembayaranCicilanInquiry:
            case trtPembayaranCicilan:
            case trtPembayaranPulsa:
            case trtPembayaranZakat:
	     case trtSetorPasti:
            case trtSetorSimpananInquiry:
            case trtSetorSimpanan:
            case trtSetorPinjamanInquiry:
            case trtSetorPinjaman:
            case trtTarikTunai:
            case trtVoidTarikTunai:
            case trtTCash:
            case trtTCashOut:
            case trtRegistrasiInternetBanking:
            case trtRegistrasiPhoneBanking:
            case trtAktivasiKartuOtorisasi:
            case trtAktivasiKartuInquiry:
            case trtAktivasiKartu:
            case trtRenewalKartuOtorisasi:
            case trtRenewalKartuInquiry:
            case trtRenewalKartu:
            case trtReissuePINOtorisasi:
            case trtReissuePIN:
            case trtGantiPasswordSPV:
            case trtInstallment:
            case trtInfoKodeBank:
            case trtPembayaranPascaBayarInquiry:
            case trtPembayaranPascaBayar:
                return 1;
            default:
                break;
        }
    }
// -- @agmr - BRI - inquriy transaction

	ret = valFloorLimit();
	CHECK(ret >= 0, lblKO);
	goto lblEnd;
lblKO:
	if(ret != -100) //@agmr
		usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcFN("valAmount ret=%d\n", ret);
	return ret;
}

/** Check whether transaction log is full
 *    - Data element : regTrxNo.
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valLogIsNotFull(void)
{
	int ret;
	word trxNo;

	trcS("valLogIsNotFull Beg\n");
	MAPGETWORD(regTrxNo, trxNo, lblKO);
	if(trxNo == dimLog)         //compare transaction no. to the current batch size
		goto lblLogFull;
	else
	{
		ret = 1;
		goto lblEnd;
	}

lblKO:
	usrInfo(infDataAccessError);
	goto lblEnd;
lblLogFull:
	ret = 0;
	usrInfo(infLogFull);
	goto lblEnd;
lblEnd:
	trcFN("valLogIsNotFull: ret=%d\n", ret);
	return ret;
}

/** Validates entered Transaction Connection Setting.
 *    - Data element : appConnType
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valConnSett(void)
{
	int ret;
	byte ConnType;

	trcS("valConnSett Beg\n");

	ret = mapDatLen(appConnType);
	VERIFY(ret == 1);           //One byte only

	MAPGETBYTE(appConnType, ConnType, lblKO);
	switch (ConnType)
	{         //connection type
		case 1:                  // Dial-up
		case 2:                  //TCPIP
		case 3:                  //RS232
		case 4:                  //GPRS
			ret = ConnType;
			break;
		default:
			goto lblInvalidInput;
	}

	goto lblEnd;
lblInvalidInput:
	usrInfo(infInvalidInput);
	ret = 0;
	goto lblEnd;
lblKO:
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valConnSett: ret=%d\n", ret);
	return ret;
}

/** Validates entered TMS dial option.
 *    - Data element : appDialOpt
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valDialOpt(void)
{
	int ret;
	byte dialOpt;

	trcS("valDialOpt Beg\n");

	ret = mapDatLen(appDialOpt);
	VERIFY(ret == 1);           //One byte only

	MAPGETBYTE(appDialOpt, dialOpt, lblKO);
	switch (dialOpt)
	{          //connection type
		case 1:                  // RS232
		case 2:                  //Dial-up
		case 3:                  //TCP/IP
		case 4:                  //GPRS
			ret = dialOpt;
			break;
		default:
			goto lblInvalidInput;
	}

	goto lblEnd;
lblInvalidInput:
	usrInfo(infInvalidInput);
	ret = 0;
	goto lblEnd;
lblKO:
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valDialOpt: ret=%d\n", ret);
	return ret;
}

/** Validates baud rate.
 *    - Data element : appBaudRate
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valBaudRate(void)
{
	int ret;
	byte baudRate;

	trcS("valBaudRate Beg\n");
	ret = mapDatLen(appBaudRate);
	VERIFY(ret == 1);           //One byte only

	MAPGETBYTE(appBaudRate, baudRate, lblKO);
	switch (baudRate)
	{
		case 1:                  //1200 bps
		case 2:                  //2400 bps
		case 3:                  //9600 bps
		case 4:                  //14400 bps
		case 5:                  //19200 bps
		case 6:                  //28800 bps
		case 7:                  //33600 bps
			ret = 1;
			break;
		default:
			goto lblKO;
	}

	goto lblEnd;
lblKO:
	return 0;
lblEnd:
	trcFN("valBaudRate: ret=%d\n", ret);
	return ret;
}

/** Validate if Application is initialized.
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valAppliIsInit()
{
	return 1;
}

/** Validate if there is a pending settlement.
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valNoPendingSettlement()
{
	return 1;
}

/** Check whether PIN is required.
 *    - Data element : issPinOpt
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valIsPinRequired()
{
	int ret;
	char PinReq;
	byte trnType;
	int tmp; //@agmr
	byte entryMode; //@@SIMAS-KEY_IN

	//@@AS0.0.26 : start
	MAPGETBYTE(traTxnType, trnType, lblKO);
	if (trnType==trtDebitLogon)
		return 0;
	//@@AS0.0.26 : end

	MAPGETBYTE(issPinOpt, PinReq, lblKO);

	switch (PinReq)
	{
		case '1':
			ret = 1;
			break;
		case '0':                // No, exit
		default:
			ret = 0;
			break;
	}

//++ @agmr
	tmp = briModifyPinEntry();
	switch(tmp)
	{
	    case -1:
	        goto lblKO;
	    case 0:
	        ret = 0;
	        break;
	    case 1:
	        ret = 1;
	        break;
	    case 100: //tidak ada perubahan
	    default:
	        break;
	}
//-- @agmr

	MAPGETBYTE(traEntMod,entryMode,lblKO); //@@SIMAS-KEY_IN
	if ((entryMode=='k') ||(entryMode=='K'))
		ret=0;

	goto lblEnd;
lblKO:
	usrInfo(infDataAccessError);
lblEnd:
	return ret;
}

/** Validate if tipping is allowed.
 *    - Data element : appTermOpt1
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valIsTipAllowed(void)
{
	int ret;
	byte tipOption;

	MAPGETBYTE(appTermOpt1, tipOption, lblKO);
	if(tipOption & 0x04)
		ret = 1;
	else
		ret = 0;
	goto lblEnd;

lblKO:
	usrInfo(infDataAccessError);
lblEnd:
	return ret;
}
/** Validate if trickle feed is enabled.
 *    - Data element : acqOpt2
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valIsTrickleFeedEnabled(void)
{
	int ret;
	byte trickleFeedOption;

	MAPGETBYTE(acqOpt2, trickleFeedOption, lblKO);
	if(trickleFeedOption & 0x10)
		ret = 1;
	else
		ret = -1;
	goto lblEnd;

lblKO:
	usrInfo(infDataAccessError);
lblEnd:
	return ret;
}

/** Validate if transaction is found.
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valTxnFound(void)
{
	return 1;
}

/** Validate if printing is required.
 *    - Data elements : appTermOpt2, issOpt2
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int valIsPrintRequired(void)
{
	int ret;
	byte printOption1, printOption2;
// ++ @agmr -- BRI
    if(isBRITransaction())
    {
        if(isBRIPrint() == 0)
        {
            ret = 0;
            goto lblEnd;
        }
    }
// -- @agmr -- BRI
	MAPGETBYTE(appTermOpt2, printOption1, lblKO);
	MAPGETBYTE(issOpt2, printOption2, lblKO);
	CHECK(ret >= 0, lblKO);
	if((printOption1 & 0x02) && (printOption2 & 0x04))
		ret = 1;
	else
		ret = 0;
	goto lblEnd;

lblKO:
	usrInfo(infDataAccessError);
lblEnd:
	return ret;
}

/** Validate if password is correct.
 * \header log\\log.h
 * \source log\\val.c
 */
int valPwd()
{
	return 1;
}

/** Validate if Manual entry mode is allowed.
 * \header log\\log.h
 * \source log\\val.c
 */
int valManualEntryAllowed()
{
	int ret;
	byte MOTO;
	byte issManual;

	ret = 1;
	MAPGETBYTE(appTermOpt1, MOTO, lblDBA);
	if(!(MOTO & 0x01))
	{
		ret = 0;
		goto lblEnd;
	}
	MAPGETBYTE(issOpt1, issManual, lblDBA);
	if(!(issManual & 0x04))
		ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("pmtManInput: ret=%d\n", ret);
	return ret;
}

int valEMVFallbackAllowed(void)//@@SIMAS_FALLBACK
{
	int ret;
	byte optVal;

	ret = 1;
	MAPGETBYTE(issOpt1, optVal, lblDBA);
	if(!(optVal & 0x02))
		ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valEMVFallbackAllowed: ret=%d\n", ret);
	return ret;
}

#ifdef SIMAS_TMS
int valIsDebitIssuer(void)
{
	int ret;
	byte optVal;

	ret = 1;
	MAPGETBYTE(issOpt4, optVal, lblDBA);
	if(!(optVal & 0x08))
		ret = 0;
	goto lblEnd;
lblDBA:
	usrInfo(infDataAccessError);
	ret = 0;
	goto lblEnd;
lblEnd:
	trcFN("valIsDebitIssuer: ret=%d\n", ret);
	return ret;
}
#endif //SIMAS_TMS

/** Validate if Magstripe entry mode is allowed.
 * \header log\\log.h
 * \source log\\val.c
 */
int valMagAllowed()
{
	return 1;
}

/** Validate if Chip entry mode is allowed.
 * \header log\\log.h
 * \source log\\val.c
 */
int valChipAllowed()
{
	return 1;
}

/** Validate if amount is below floor limit.
 * \header log\\log.h
 * \source log\\val.c
 */
int valFloorLimit()
{
	char amtTot[lenTotAmt + 1];
	char amtFlrLimit[lenAmt + 1];
	char amtS[lenAmt + 1];
	char amtTip[lenAmt + 1];
	char amt[lenTotAmt + 1];
	char traName[dspW + 1];
	card amtFlrLim = 0;
	int ret;
	word mnuItem;

	trcS("valFloorLimit Beg\n");
	memset(amt, 0x00, sizeof(amt));
	memset(amtFlrLimit, 0x00, sizeof(amtFlrLimit));
	memset(amtTot, 0x00, sizeof(amtTot));
	MAPGET(traAmt, amtS, lblKO);
	MAPGET(traTipAmt, amtTip, lblKO);
	MAPGETWORD(issFloorLimit, amtFlrLim, lblKO);
	amtFlrLim = amtFlrLim * 100;    // add centavos on floor limit
	num2dec(amtFlrLimit, amtFlrLim, sizeof(amtFlrLimit));
	addStr(amtTot, amtS, amtTip);
	trcFS("amtTot=%s\n", amtTot);
	if(amtTot[0] == '0')
	{
		usrInfo(infInvalidAmount);
//		ret = 0;
        ret = -100; //@agmr
		goto lblEnd;
	}
	MAPGETWORD(traMnuItm, mnuItem, lblKO);
	if((mnuItem != mnuSale) && (mnuItem != mnuSaleRedeem))
	{
		ret = 1;
		goto lblEnd;
	}
	trcFS("amtFlrLim=%s\n", amtFlrLimit);
	subStr(amt, amtTot, amtFlrLimit);
	trcFS("amt=%s\n", amt);
	if(amt[0] == '-')           // if below floor limit change trans type to offline
	{
		mapMove(rqsBeg, (word) (trtOffline - 1));
		MAPGET(rqsDisp, traName, lblKO);
		trcFS("traName: %s\n", traName);

		MAPPUTSTR(traTransType, traName, lblKO);
		MAPPUTBYTE(traTxnType, trtOffline, lblKO);
		MAPPUTWORD(traMnuItm, mnuOffline, lblKO);

	}
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcFN("valFloorLimit ret=%d\n", ret);
	return ret;
}

/** Ask for Password from user
 * \header sys\\log.h
 * \source sys\\adm.c
*/
int valAskPwd(word key)
{
	char pwd[lenPwd + 1];
	char Keypwd[lenPwd + 1];
	int ret;

	trcS("valAskPwd Beg\n");
	memset(pwd, 0, lenPwd + 1);
	memset(Keypwd, 0, lenPwd + 1);

	MAPGET(key, Keypwd, lblKO);

	ret = usrInfo(infEntPwd);
	CHECK(ret >= 0, lblKO);
	memset(pwd, 0, lenPwd + 1);

	ret = enterPwd(2, pwd, lenPwd + 1); //Enter Settlement password
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblEnd);

	CHECK(strcmp(pwd, Keypwd) == 0, lblBadPwd); //does password match?

	goto lblEnd;
lblBadPwd:
	usrInfo(infPwdInvalid);
	ret = 0;
	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcFN("valAskPwd ret=%d\n", ret);
	return ret;
}

/** Ask for PIN
 * \header sys\\log.h
 * \source sys\\adm.c
*/
int valAskPin(byte *outData) //@@SIMAS-SW_PIN
{
	char pin[13];
	int ret;

	trcS("valAskPin Beg\n");
	memset(pin, 0, sizeof(pin));

	ret = usrInfo(infEntPin);
	CHECK(ret >= 0, lblKO);

	ret = enterPwd(2, pin, sizeof(pin)); //Enter Settlement password
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblEnd); //If Not ENTER, go to lblEnd

	memcpy(outData,pin,sizeof(pin));

	goto lblEnd;
lblKO:
	usrInfo(infProcessingError);
	goto lblEnd;
lblEnd:
	trcFN("valAskPin ret=%d\n", ret);
	return ret;
}


/** \return
 * - 1 if current transaction is approved
 * - 0 if not
 * - 0 in case of fatal error
 */
int valRspCod(void)
{
	int ret;
	char RspCod[lenRspCod + 1];
	word trtType; //@agmr - BRI

	MAPGET(traRspCod, RspCod, lblKO);
	if(strcmp(RspCod, "00") == 0)
		return 1;

//+++ @agmr - BRI
    MAPGETBYTE(traTxnType,trtType,lblKO); //@agmr - BRI
    if((trtType == trtPembayaranPLNPra) ||(trtType == trtSetorPasti) )
    {
        if(strcmp(RspCod, "02") == 0)
            return 1;
    }

	if(trtType == trtSetorPasti)
    {
        if(strcmp(RspCod, "68") == 0)
            return 1;
    }

	if(trtType == trtPrevilege)
    {
        if(strcmp(RspCod, "01") == 0)
            return 1;
    }
	if(trtType == trtPembayaranPascaBayar)
	{

	}
    else if(trtType == trtRenewalKartu || trtType == trtAktivasiKartu)
    {
        if(strcmp(RspCod, "NC") == 0)
            return 1;
    }
//--- @agmr - BRI
	return 0;
lblKO:
	return 0;
}

/** Validates entered Blowfish Setting.
 *    - Data element : appBlowFish
 *
 * \header log\\log.h
 * \source log\\val.c
 */
int ValBfSett(void){
	int ret;
    byte BlowSet;

    trcS("ValBfSett Beg\n");

    ret = mapDatLen(appBlowFish);
    VERIFY(ret == 1);           //One byte only

    MAPGETBYTE(appBlowFish, BlowSet, lblKO);
    switch (BlowSet) {         //Set Blowfish
      case 1:                  // On Bf
      case 2:                  // Off Bf
          ret = BlowSet;
          break;
      default:
          goto lblInvalidInput;
    }

    goto lblEnd;
  lblInvalidInput:
    usrInfo(infInvalidInput);
    ret = 0;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("ValBfSett: ret=%d\n", ret);
    return ret;
}

