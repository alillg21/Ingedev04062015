#include "log.h"
#include <string.h>
#include <stdlib.h>
#include "stdio.h"
#include <bri.h>
#include "prepaid.h"
#include "logon.h"

static 	byte UseChip = 0;
extern 	byte txnAsli;

int pmtForceOnline(void)
{
	trcS("pmtForceOnline Beg\n");
	return 0;
}

int pmtManInput(void)
{
	char 	Pan[lenPan + 1];
	char 	Trk2[lenTrk2 + 1];
	char 	ExpDat[lenExpDat + 1];
	char 	Buf[128 * 3];
	char 	tmp[dspW * 2 + 1];
	int 		ret;

	trcS("pmtManInput Beg\n");
	memset(Pan, 0, lenPan + 1);
	memset(Trk2, 0, lenTrk2 + 1);
	memset(ExpDat, 0, lenExpDat + 1);
	memset(Buf, 0, sizeof(Buf));

	ret = usrInfo(infPanInput);
	CHECK(ret >= 0, lblKO);
	ret = enterStr(2, Buf, lenPan + 1);
	CHECK(ret >= 0, lblKO);

	if(ret != kbdVAL)
		goto lblEnd;
	strcpy(Pan, Buf);

	memset(Buf, 0, sizeof(Buf));
	ret = usrInfo(infExpDatInput);
	CHECK(ret >= 0, lblKO);
	ret = enterStr(2, Buf, lenExpDat + 1);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL)
		goto lblEnd;
	ExpDat[0] = Buf[2];
	ExpDat[1] = Buf[3];
	ExpDat[2] = Buf[0];
	ExpDat[3] = Buf[1];

	MAPPUTSTR(traPan, Pan, lblDBA);
	MAPPUTSTR(traExpDat, ExpDat, lblDBA);
	strcpy(tmp, Pan);
	strcat(tmp, "=");
	strcat(tmp, ExpDat);
	ret = mapPut(traTrk21, tmp, dspW);
	CHECK(ret >= 0, lblDBA);
	MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

	ret = kbdVAL;
	goto lblEnd;
	lblKO:
		usrInfo(infProcessingError);
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

int pmtCardInput(void)
{
	char 	EntMod;
	char 	CrdInpCtl[lenCrdInpCtl + 1];
	char 	Pan[lenPan + 1];
	char 	Trk2[lenTrk2 + 1];
	char 	Trk1[128 + 1];
	char 	CardHolderName[40 + 1];
	char 	ExpDat[lenExpDat + 1];
	char 	Buf[128 * 3];
	char 	*pTrk2;
	char 	FirstKey;
	char 	tmp[dspW * 2 + 1];
	int 		ret;
	byte 	Fallback;
	char 	Srvcod[3+1];
	char 	BlockedPan[lenPan + 1];
	int 		Fallbackret = 0;
	word 	inf;
	byte 	txnType;


	trcS("pmtCardInput Beg\n");
	memset(Pan, 0, lenPan + 1);
	memset(Trk2, 0, lenTrk2 + 1);
	memset(ExpDat, 0, lenExpDat + 1);
	memset(Buf, 0, sizeof(Buf));
	memset(Trk1, 0, sizeof(Trk1));
	memset(CardHolderName, ' ', sizeof(CardHolderName));
	memset(BlockedPan, '*', sizeof BlockedPan);

	MAPGETBYTE(traTxnType,txnType,lblKO);

	MAPGETBYTE(traFallback,Fallback,lblKO);
	if(Fallback == 1)
		inf = infCardInputFB;
	else
	{
		if(txnType == trtAktivasiKartuOtorisasi || txnType == trtRenewalKartuOtorisasi || txnType == trtReissuePINOtorisasi)
			inf = infCardPengawasInput;
		else if (txnType == trtAktivasiKartuInquiry || txnType == trtReissuePIN)
			inf = infCardNasabahInput;
		else if(txnType == trtRenewalKartuInquiry)
			inf = infCardNasabahLamaInput;
		else if(txnType == trtDummy)
			inf = infDummy;
		else if(txnType == trtRenewalKartu)
			inf = infCardNasabahBaruInput;
		else if((txnType == trtPreAuth) || (txnType == trtPreCancel) || (txnType == trtPreCom))
			inf = infCardInputPre;
		else
			inf = infCardInput;
	}

	ret = usrInfo(inf);
	CHECK(ret >= 0, lblKO);
	MAPGET(rqsEntMod, CrdInpCtl, lblKO);
	trcFS("CrdInpCtl: %s\n", CrdInpCtl);

	if((emvGetFallBack() != 0)||(Fallback == 1))
	{
		memset(CrdInpCtl,0,sizeof(CrdInpCtl));
		Fallbackret = 1;
		CrdInpCtl[0] = 'm';
		CrdInpCtl[1] = 'k';
		emvSetFallBack(0);
		MAPPUTBYTE(traFallback, 0,lblKO);
	}
	ret = getCard((byte *) Buf, CrdInpCtl);
	CHECK(ret >= 0, lblKO);
	CHECK(ret > 0, lblExit);

	EntMod = (char) ret;
	switch (EntMod)
	{
		case 'm':
			if(*Buf != 0)
			{
				memcpy(Trk1, Buf, 128);
				ret = fmtTok(0, Trk1, "^");
				CHECK(ret <= sizeof(Trk1), lblInvalidTrk);
				memset(Trk1, 0, sizeof(Trk1));
				memcpy(Trk1, &Buf[ret + 1], 128 - ret);
				ret = fmtTok(CardHolderName, Trk1, "^");
			}

			MAPPUTSTR(traTrk1, CardHolderName, lblKO);

			pTrk2 = &Buf[128];
			MAPPUTSTR(traTrk2, pTrk2, lblKO);
			ret = fmtTok(0, pTrk2, "=");
			CHECK(ret <= lenPan, lblInvalidTrk);
			ret = fmtTok(Pan, pTrk2, "=");
			VERIFY(ret <= lenPan);

			pTrk2 += ret;
			CHECK(*pTrk2 == '=', lblInvalidTrk);
			pTrk2++;
			ret = fmtSbs(ExpDat, pTrk2, 0, lenExpDat);
			VERIFY(ret == 4);


			pTrk2 += ret;
			ret = fmtSbs(Srvcod, pTrk2, 0, 3);
			VERIFY(ret == 3);
			MAPPUTSTR(traSrvCod, Srvcod, lblDBA);
			if((Srvcod[0] == '2')||(Srvcod[0] == '6'))
			{
				if (emvFullProcessCheck() == 1)
				{
					if(Fallback == 0)
						UseChip = 1;
					else
						UseChip = 0;
				}
			}
			else
				Fallbackret = 0;

			break;

		case 'c':
			if((isBRITransaction() == 1 && txnType != trtInstallment)
#ifdef PREPAID
				|| txnType == trtPrepaidTopUp
				|| txnType == trtPrepaidDeposit
#endif
			)
				goto lblChipBarred;
			break;

		default:
			//if((txnType == trtSale)||(txnType == trtPreAuth)){
			if((txnType == trtPreAuth)||(txnType == trtPreCancel)||(txnType == trtPreCom)){
				VERIFY(ret == 'k');
				FirstKey = Buf[0];
				if(FirstKey == kbdANN) {
					ret = 0;
					goto lblEnd;
				}

				if(('0' > FirstKey || FirstKey > '9') &&( FirstKey != kbdVAL)){
					ret = 0;
					goto lblEnd;
				}

				memset(Buf, 0, sizeof(Buf));
				Buf[0] = FirstKey;


					ret = pmtManInput();
					CHECK(ret >= 0, lblExit);


				if(ret != kbdANN){
					ret = valAskPwd(appRefPwd);
					CHECK((ret > 0) && (ret != kbdANN), lblKO);
				}
				else
					goto lblExit;
			}
			break;
	}

	if(EntMod != 'k')
	{
		MAPPUTSTR(traPan, Pan, lblDBA);
		memcpy(BlockedPan + 12, Pan + 12, lenPan-12);
		BlockedPan[sizeof(BlockedPan)-1]=0;
		MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);
		MAPPUTSTR(traExpDat, ExpDat, lblDBA);
		strcpy(tmp, Pan);
		strcat(tmp, "=");
		strcat(tmp, ExpDat);
		ret = mapPut(traTrk21, tmp, dspW);
		CHECK(ret >= 0, lblDBA);
		MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);
	}

	if(Fallbackret == 1)
	EntMod = 'f';

	ret = EntMod;


	if(txnType == trtRenewalKartuInquiry)
	{
		char buf[20];
		memset(buf,0,sizeof(buf));
		mapGet(traPan,buf,sizeof(buf));
		mapPut(traNomorKartuLama,buf,lenNomorKartuLama);
	}

	if(txnType == trtAktivasiKartuOtorisasi || 	txnType == trtRenewalKartuOtorisasi || txnType == trtReissuePINOtorisasi )
	{
		char buf[20];
		memset(buf,0,sizeof(buf));
		mapGet(traPan,buf,sizeof(buf));
		mapPut(traNomorKartuPetugas,buf,lenNomorKartuPetugas);
	}

	goto lblEnd;

	lblChipBarred:
		usrInfo(infPlsSwipeCard);
		ret = -100;
		goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblExit:
		ret = 0;
		goto lblEnd;

	lblInvalidTrk:
		usrInfo(infInvalidTrack);
		ret = 0;
		goto lblEnd;

	lblDBA:
		usrInfo(infDataAccessError);
		ret = 0;
		goto lblEnd;

	lblEnd:
		trcFN("pmtCardInput: ret=%d\n", ret);
		return ret;
}

int pmtOldPinInput(void)
{
	char Pan[lenPan + 1];
	byte Acc[8];
	byte Blk[lenPinBlk];
	char Msg1[dspW + 1];
	char Msg2[dspW + 1];
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;
	int 	ret;

	trcS("pmtOldPinInput Beg\n");

	memset(Blk, 0, sizeof(Blk));
	ret = cryStart('m');
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appPinKeyLoc, LocPK, lblKO);


	MAPGET(traPan, Pan, lblKO);
	ret = stdGetAcc(Acc, Pan);

	dspClear();
	dspLS(1,"   UBAH PIN");
	strcpy(Msg,"\nOld PIN ?\n\n");
	ret = cryGetPin(Msg, Acc, LocPK, Blk);
	CHECK(ret >= 0, lblKO);
	if(ret == 0)
		goto lblEnd;
	ret = dspStart();
	CHECK(ret >= 0, lblKO);
	ret = mapPut(traPinBlk, Blk, lenPinBlk);
	CHECK(ret >= 0, lblKO);

	ret = 1;
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;
	lblEnd:
		cryStop('m');
		trcFN("pmtOldPinInput: ret=%d\n", ret);
		return ret;
}

int pmtNewPinInput(void)
{
	char Pan[lenPan + 1];
	byte Acc[8];
	byte Blk[lenPinBlk];
	char Msg1[dspW + 1];
	char Msg2[dspW + 1];
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;
	int 	ret;
	char pin1[20],pin2[20];
	int 	i;

	trcS("pmtNewPinInput Beg\n");

	for(i = 0; i < 2; i++)
	{
		memset(Blk, 0, sizeof(Blk));
		ret = cryStart('m');
		CHECK(ret >= 0, lblKO);

		MAPGETBYTE(appPinKeyLoc, LocPK, lblKO);
		MAPGET(traPan, Pan, lblKO);
		ret = stdGetAcc(Acc, Pan);

		dspClear();
		dspLS(1,"   UBAH PIN");

		if(i == 0)
			strcpy(Msg,"\nENTER NEW PIN :");
		else
			strcpy(Msg,"\nRE-ENTER NEW PIN:");
		ret = cryGetPin(Msg, Acc, LocPK, Blk);
		CHECK(ret >= 0, lblKO);

		if(ret == 0)
			goto lblEnd;
		ret = dspStart();
		CHECK(ret >= 0, lblKO);
		ret = mapPut(traNewPinBlk, Blk, lenPinBlk);
		CHECK(ret >= 0, lblKO);
		if(i==0)
			memcpy(pin1,Blk,lenPinBlk);
		else
			memcpy(pin2,Blk,lenPinBlk);

		if(i == 1)
		{
			if(memcmp(pin1,pin2,lenPinBlk) == 0)
				break;
			else
			{
				usrInfo(infPINNotMatch);
				i=-1;
			}
		}
	}

	ret = 1;
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblEnd:
		cryStop('m');
		trcFN("pmtNewPinInput: ret=%d\n", ret);
		return ret;
}

#ifdef USE_SEC_AREA
int pmtPinInput(void)
{
	char Pan[lenPan + 1];
	byte Acc[8];
	byte Blk[lenPinBlk];
	char Msg1[dspW + 1];
	char Msg2[dspW + 1];
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;
	int 	ret;

	trcS("pmtPinInput Beg\n");
	memset(Blk, 0, sizeof(Blk));
	ret = cryStart('m');
	CHECK(ret >= 0, lblKO);

	MAPGETBYTE(appPinKeyLoc, LocPK, lblKO);
	MAPGET(traPan, Pan, lblKO);
	ret = stdGetAcc(Acc, Pan);
	MAPGET(msgPin, Msg2, lblKO);
	dspStop();

	strcpy(Msg, " ");
	strcat(Msg, "\n");
	strcat(Msg, Msg2);
	strcat(Msg, "\n");
	strcat(Msg, "\n");
	ret = cryGetPin(Msg, Acc, LocPK, Blk);
	CHECK(ret >= 0, lblKO);
	if(ret == 0)
		goto lblEnd;
	ret = dspStart();
	CHECK(ret >= 0, lblKO);
	ret = mapPut(traPinBlk, Blk, lenPinBlk);
	CHECK(ret >= 0, lblKO);

	MAPPUTBYTE(traPinReq, 1, lblKO);

	ret = 1;
	goto lblEnd;

	lblKO:
		if (ret==-2) {//@@SIMAS-PIN_BYPASS
			usrInfo(infPinBypassed);//@@SIMAS-PIN_BYPASS
			MAPPUTBYTE(traPinReq, 0, lblKO);
			ret = 1;
		}
		else{ //@@SIMAS-PIN_BYPASS
			usrInfo(infProcessingError);
			ret = 0;
		}
		goto lblEnd;

	lblEnd:
		cryStop('m');
		trcFN("pmtPinInput: ret=%d\n", ret);
		return ret;
}
#else //USE_SEC_AREA
int pmtPinInput(void) //@@SIMAS-SW_PIN
{
	byte pin[20],pinbcd[20],MK[lenKEY],WK[lenKEY],ePin[20];
	char Pan[lenPan + 1];
	byte Acc[10];
	byte Blk[lenPinBlk];
	char Msg1[dspW + 1];
	char Msg2[dspW + 1];
	char Msg[(sizeof(Msg1) + sizeof(Msg2) + dspW) + 1];
	byte LocPK;
	int 	ret,pinlen,panLen;

	trcS("pmtPinInput Beg\n");
	memset(Blk, 0, sizeof(Blk));

	MAPGET(traPan, Pan, lblKO);
	if ((strlen(Pan) % 2) !=0)
		strcat(Pan,"F");
	panLen=strlen(Pan)/2;
	ret=hex2bin(Acc,Pan,panLen);

	//ret = stdGetAcc(Acc, Pan);
	//MAPGET(msgPin, Msg2, lblKO);

	ret=mapGet(acqTMK,MK,lenKEY);
	CHECK(ret > 0, lblKO);

	ret=mapGet(acqTWK,WK,lenKEY);
	CHECK(ret > 0, lblKO);


	//=========================
	ret=valAskPin(pin);
	if (ret == kbdVAL) {
		pinlen=strlen(pin);
		if ((pinlen % 2) != 0)
			strcat(pin,"F");
		hex2bin(pinbcd,pin,strlen(pin)/2);
		ret=GetEncryptedPinBlock(pinbcd,pinlen,MK,WK,lenKEY,Acc,8,ePin);
	}
	else if (ret==kbdANN)
		ret=-2;
	CHECK(ret >= 0, lblKO);

	ret = mapPut(traPinBlk, ePin, lenPinBlk);
	CHECK(ret >= 0, lblKO);

	MAPPUTBYTE(traPinReq, 1, lblKO);
	ret = 1;
	goto lblEnd;
	//==========================


	lblKO:
		if (ret==-2) {//@@SIMAS-PIN_BYPASS
			usrInfo(infPinBypassed);//@@SIMAS-PIN_BYPASS
			MAPPUTBYTE(traPinReq, 0, lblKO);
			ret = 1;
		}
		else{ //@@SIMAS-PIN_BYPASS
			usrInfo(infProcessingError);
			ret = 0;
		}
		goto lblEnd;

	lblEnd:
		trcFN("pmtPinInput: ret=%d\n", ret);
		return ret;
}
#endif //USE_SEC_AREA

int pmtAmountInput(void) {
	char Amt[lenAmt + 1];
	byte Exp;
	int 	ret;
#ifdef BRIZI_DISCOUNT
	char buf[20],buf1[20];
	byte res[5];
#endif
	byte tmp;


	trcS("pmtAmountInput Beg\n");
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
		case trtPembayaranTVBerlanggananInquiry:
		case trtPembayaranTVBerlangganan:
		case trtPembayaranTiketPesawatInquiry:
		case trtPembayaranTiketPesawat:
		case trtPembayaranPdamInquiry:
		case trtPembayaranPdam:
		case trtPembayaranTelkomInquiry:
		case trtPembayaranTelkom:
#ifdef SSB
		case trtPembayaranSSBInquiry:
		case trtPembayaranSSB:
#endif
		case trtPembayaranDPLKR:
		case trtPembayaranBrivaInquiry:
		case trtPembayaranBriva:
		case trtTrxPhoneBankingTransfer:
		case trtTrxPhoneBankingTransferInquiry:
		case trtTrxPhoneBankingPulsa:
		case trtRegistrasiSmsBanking:
		case trtPembayaranKKBRIInquiry:
		case trtPembayaranKKBRI:
		case trtPembayaranKKANZ:
		case trtPembayaranKKCITIKTA:
		case trtPembayaranKKCITIKK:
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

	usrInfo(infAmtInput);
	MAPGETBYTE(appExp, Exp, lblKO);
	memset(Amt, 0, sizeof(Amt));
	ret = enterAmt(3, Amt, Exp);
	CHECK(ret >= 0, lblKO);

	if(ret == kbdANN || ret == kbdF1 || ret == 0)
	{
		ret = -100;
		goto lblEnd;
	}

	if(ret != kbdVAL) {
		ret = 0;
		goto lblEnd;
	}
#ifdef BRIZI_DISCOUNT
	if(tmp == trtPrepaidPaymentDisc){
		memset(buf1,0,sizeof(buf1));
		memset(buf,0,sizeof(buf));
		MAPGETBYTE(appBriziDiscFlag,res,lblKO);
		MAPGET(appBriziDiscValue,buf1,lblKO);
		if(res[0] == 2){
			sprintf(buf,"%d",((atoi(Amt)) - ((atoi(Amt) * atoi(buf1))/100)));
			if((atoi(buf) < 0) ){
				usrInfo(infTransacError);
				goto lblKO;
			}
			else
				MAPPUTSTR(traAmt, buf, lblKO);
			}
		else{
			sprintf(buf,"%d",((atoi(Amt) - atoi(buf1))));
			if((atoi(buf) < 0)){
				usrInfo(infTransacError);
				goto lblKO;
			}
			else
				MAPPUTSTR(traAmt, buf, lblKO);
		}
	memset(buf1,0,sizeof(buf1));
	mapPut(traTotalAmount,Amt,lenTotalAmount);
	mapGet(traTotalAmount,buf1,sizeof(buf1));
	}
	else
#endif
	MAPPUTSTR(traAmt, Amt, lblKO);

	ret = valAmount();
	CHECK(ret > 0, lblKO);
	ret = 1;
	goto lblEnd;
	lblKO:
		if(ret == -100)
			goto lblEnd;
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblEnd:
		trcFN("pmtAmountInput: ret=%d\n", ret);
		return ret;
}


int pmtTipInput(void) {
	char TipAmt[lenAmt + 1];
	int 	ret;
	byte idx;
	byte Exp;

	trcS("pmtTipInput Beg\n");
	MAPGETBYTE(traTxnType, idx, lblKO);
	switch (idx) {
		case trtSale:
		case trtOffline:
			break;

		default:
			goto lblEnd;
	}

	ret = valIsTipAllowed();
	CHECK(ret >= 0, lblEnd);
	if(ret == 0) {
		ret = 1;
		goto lblEnd;
	}

	ret = dspClear();
	CHECK(ret >= 0, lblKO);
	usrInfo(infTipInput);
	MAPGETBYTE(appExp, Exp, lblKO);
	memset(TipAmt, 0, sizeof(TipAmt));
	ret = enterAmt(3, TipAmt, Exp);
	CHECK(ret >= 0, lblKO);
	if(ret != kbdVAL) {
		ret = 0;
		goto lblEnd;
	}
	MAPPUTSTR(traTipAmt, TipAmt, lblKO);
	if(atoi(TipAmt) > 0)
		MAPPUTBYTE(traTipFlag, 1, lblKO);
	ret = 1;
	goto lblEnd;
	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;
	lblEnd:
		trcFN("pmtTipInput: ret=%d\n", ret);
		return ret;
}

static int pmtComputeTotAmt(void)
{
	int 	ret;
	char amtS[lenAmt + 1];
	char amtTip[lenAmt + 1];
	char amtTot[lenTotAmt + 1];
	byte tmp;

	MAPGETBYTE(traTxnType,tmp,lblKO);
	switch(tmp)
	{
		case trtInfoSaldo:
			return 1;

		default:
			break;
	}

	MAPGET(traAmt, amtS, lblKO);
	MAPGET(traTipAmt, amtTip, lblKO);

	ret = sizeof(amtTot);
	VERIFY(ret >= 16 + 1);
	memset(amtTot, 0, sizeof(amtTot));
	addStr(amtTot, amtS, amtTip);
	trcFS("amtS=%s\n", amtS);
	trcFS("amtTip=%s\n", amtTip);
	trcFS("amtTot=%s\n", amtTot);
	CHECK(strlen(amtTot) <= lenAmt, lblKO);
	MAPPUTSTR(traTotAmt, amtTot, lblKO);
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;
	lblEnd:
		trcFN("pmtComputeTotAmt: ret=%d\n", ret);
		return ret;
}


static int pmtTxnOthers(byte txnType)
{
	int 		ret;
	word 	key;
	byte 	flag;
	word 	menuItem = 0;
	byte 	IssId,AcqId;

	trcS("pmtTxnOthers Beg\n");

	txnAsli = 0;
	switch (txnType)
	{
		case trtVoid:
			key = appVoidPwd;
			menuItem = mnuVoid;
			break;

		case trtVoidTarikTunai:
			key = appVoidPwd;
			menuItem = mnuVoidTarikTunai;
			break;

		default:
			key = appVoidPwd;
			break;
	}

	ret = scrDlg(infEnterTrace, traFindRoc);
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblKO);

	if(txnType == trtVoidTarikTunai)
		ret = briLogLoadByRoc(TUNAI_LOG);
	else
		ret = logLoad();
	CHECK(ret > 0, lblEnd);

	MAPPUTWORD(traMnuItm,menuItem,lblKO);
	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPGETBYTE(traIssIdx, IssId, lblKO);
	MAPGETBYTE(traAcqIdx, AcqId, lblKO);
	mapMove(issBeg, (word) IssId);
	mapMove(acqBeg, (word) AcqId);


	if(txnType == trtVoid || txnType == trtVoidTarikTunai)
	{
		MAPGETBYTE(traVoidFlag, flag, lblKO);
		CHECK(flag != 1, lblAlreadyVoided);
	}

	ret = pmtComputeTotAmt();
	CHECK(ret > 0, lblKO);

	switch (txnType)
	{
		case trtVoid:
			key = infVoidTotal;
			break;

		default:
			key = infVoidTotal;
			break;
	}

	{
		char buf[30],buf1[30];

		memset(buf,0,sizeof(buf));
		mapGet(traTotAmt,buf,sizeof(buf));
		fmtAmt(buf1,buf,0,",.");
		fmtPad(buf1,-16,' ');
		MAPPUTSTR(msgBuffer,buf1,lblKO);
		ret = usrInfo(infVoidTotal2);
		CHECK(ret > 0, lblKO);
		CHECK(ret == kbdVAL, lblKO);
	}

	ret = pmtBRIKonfirmasiTanpaInquiry(txnType);
	if(ret <= 0)
		goto lblKO;

	ret = onlSession();
	CHECK(ret > 0, lblKO);

	ret = valRspCod();
	CHECK(ret > 0, lblDeclined);

	MAPPUTWORD(traMnuItm, menuItem, lblKO);
	ret = logSave();
	CHECK(ret > 0, lblKO);

	ret = 1;
	goto lblEnd;

	lblDeclined:
		ret = -3;
		goto lblEnd;

	lblAlreadyVoided:
		usrInfo(infAlreadyVoided);
		ret = 0;
		goto lblEnd;

	lblKO:
		ret = 0;
		trcS("pmtTxnOthers aborted\n");

	lblEnd:
		trcFN("pmtTxnOthers: ret=%d\n", ret);
		return ret;
}


static int pmtAskAppCod(void)
{
	int ret;

	trcS("pmtAskAppCod Beg\n");

	ret = scrDlg(infEnterAppCod, traAutCod);
	CHECK(ret >= 0, lblKO);
	if(ret == kbdANN)
	{
		ret = 0;
		goto lblEnd;
	}

	ret = 1;
	goto lblEnd;

	lblKO:
		trcS("pmtAskAppCod aborted\n");

	lblEnd:
		trcFN("pmtAskAppCod: ret=%d\n", ret);
		return ret;
}

static void pmtPrepareOfflineIfAvailable(void)
{
	card 	idx = 0;
	char 	buf[lenMnu + 1], Rrn[lenRrn + 1];
	word 	mnuItem, trxNum;
	byte 	TxnTypebeforeOffline;
	word 	TxnMnuItmbeforeOffline, count;
	int 		ret;

	trcS("pmtPrepareOfflineIfAvailable Beg\n");
	ret = valIsTrickleFeedEnabled();
	CHECK(ret > 0, lblEnd);

	MAPGETWORD(regOfflineCnt, count, lblKO);
	if(count > 0)
	{
		MAPGETBYTE(traTxnType, TxnTypebeforeOffline, lblKO);
		MAPPUTBYTE(traTxnTypeBeforeOffline, TxnTypebeforeOffline, lblKO);
		MAPGETWORD(traMnuItm, TxnMnuItmbeforeOffline, lblKO);
		MAPPUTBYTE(traMnuItmBeforeOffline, TxnMnuItmbeforeOffline, lblKO);

		MAPPUTBYTE(traTxnType, trtOffline, lblKO);
		MAPPUTWORD(traMnuItm, mnuOffline, lblKO);
		MAPGET(mnuOffline, buf, lblKO);
		MAPPUTSTR(traCtx, buf + 2, lblKO);

		MAPPUTBYTE(regLocType, 'L', lblKO);
		MAPGETWORD(regTrxNo, trxNum, lblKO);
		while(idx < trxNum)
		{
			mapMove(logBeg, (word) idx);
			MAPGETWORD(logMnuItm, mnuItem, lblKO);
			if(mnuItem == mnuOffline)
			{
				MAPGET(logRrn, Rrn, lblKO);
				if(strlen(Rrn) == 0)
				{
					trcS("offline txn present\n");
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
	}
	lblKO:
	lblEnd:
		trcS("pmtPrepareOfflineIfAvailable End\n");
}


static int pmtMagStripe(void)
{
	int 		ret;
	word 	key;
	byte 	PanDisplay;
	char 	Amt[lenAmt + 1], Amt2[lenAmt + 1];
	byte 	txnType;
	byte		res;
	char 	EntMod;
#ifdef _USE_ECR
	char 	buf[25];
	byte 	appIsECRbyte;
#endif

	trcS("pmtMagStripe Beg\n");
	MAPPUTBYTE(traEmvFullFlag, 0, lblKO);
	MAPGETBYTE(traTxnType, txnType, lblKO);
	res = 0;

	if(txnType == trtPembayaranPLNToken)
		goto lblJump;

#ifdef PREPAID
	if(txnType == trtPrepaidAktivasi || txnType == trtPrepaidRedeem || txnType == trtPrepaidRedeemInquiry|| txnType == trtPrepaidReaktivasiInquiry
		|| txnType == trtPrepaidReaktivasi || txnType == trtPrepaidVoidInquiry || txnType == trtPrepaidVoid )
		goto lblJump;
#endif

	if(UseChip == 1){
		dspClear();
		usrInfo(infEMVChipDetect);
		UseChip = 0;
		Beep();
		ret = 0;
		goto lblEnd;
	}

	ret = valCard();
	CHECK(ret > 0, lblKO);

	if(checkPendingSettlement()!=0)
	goto lblExit;

	MAPGETBYTE(appTermOpt1, PanDisplay, lblKO);
	//@@SIMAS_FALLBACK - start
	 MAPGETBYTE(traEntMod, EntMod, lblKO);
    	 if (EntMod=='f') {
        	if ((!valEMVFallbackAllowed()) && ((txnType!=trtPreAuth)&&(txnType!=trtPreCom)&&(txnType!=trtPreCancel)))
            		goto lblFallbackNotAllowed;
    	}

	//@@SIMAS_FALLBACK - end
	if(PanDisplay & 0x02)
	{
		while(1)
		{
			ret = usrInfo(infConfirmPan);
			CHECK(ret >= 0, lblKO);
			if(ret == kbdVAL)
				break;
			if(ret == kbdANN)
				goto lblExit;
			Beep();
		}
	}

	if (valIsPinRequired())
	if (debitLogon(txnType,0) < 0)
		goto lblKO;

	if(txnType == trtGantiPIN || txnType == trtAktivasiKartuInquiry || txnType == trtRenewalKartu)
	{
		ret = valIsPinRequired();
		CHECK(ret >= 0, lblKO);
		if(ret)
		{
			if(txnType == trtGantiPIN)
			{
				ret = pmtOldPinInput();
				CHECK(ret > 0, lblKO);
			}

			ret = pmtNewPinInput();
			CHECK(ret > 0, lblKO);
		}
		else if(ret == 0)
		{
			usrInfo(infCardPinNotRequired);
			goto lblExit;
		}
	}
	else
	{
#ifdef _USE_ECR
		MAPGETBYTE(appIsECR,appIsECRbyte,lblKO);
		if(appIsECRbyte == 1){
			memset(buf,0,sizeof(buf));
			mapGet(appAmtECR,buf,sizeof(buf));
			MAPPUTSTR(traAmt, buf, lblKO);
		}
#endif
		MAPGET(traAmt, Amt, lblKO);
		if(Amt[0] == 0) {
			ret = pmtAmountInput();
			CHECK(ret > 0, lblKO);

			ret = pmtTipInput();
			CHECK(ret > 0, lblKO);

			ret = valAmount();
			CHECK(ret > 0, lblKO);

			MAPGETBYTE(traTxnType,txnType,lblKO);
			switch(txnType)
			{
				case trtInfoSaldo:
				case trtInfoSaldoBankLain:
				case trtMutRek:
				case trtPrevilege:
				case trtMiniStatement:
				case trtTransferSesamaBRIInquiry:
				case trtTransferSesamaBRI:
				case trtTransferAntarBankInquiry:
				case trtTransferAntarBank:
				case trtPembayaranTVBerlangganan:
				case trtPembayaranTVBerlanggananInquiry:
				case trtPembayaranTiketPesawatInquiry:
				case trtPembayaranTiketPesawat:
				case trtPembayaranPdamInquiry:
				case trtPembayaranPdam:
				case trtPembayaranTelkomInquiry:
				case trtPembayaranTelkom:
#ifdef SSB
				case trtPembayaranSSBInquiry:
				case trtPembayaranSSB:
#endif
				case trtPembayaranDPLKR:
				case trtPembayaranBrivaInquiry:
				case trtPembayaranBriva:
				case trtTrxPhoneBankingTransfer:
				case trtTrxPhoneBankingTransferInquiry:
				case trtTrxPhoneBankingPulsa:
				case trtRegistrasiSmsBanking:
				case trtPembayaranKKBRIInquiry:
				case trtPembayaranKKBRI:
				case trtPembayaranKKANZ:
				case trtPembayaranKKCITIKTA:
				case trtPembayaranKKCITIKK:
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

#ifdef PREPAID
				case trtPrepaidDeposit:
				case trtPrepaidTopUp:
				case trtPrepaidRedeemInquiry:
				case trtPrepaidRedeem:
				case trtPrepaidAktivasi:
				case trtPrepaidReaktivasiInquiry:
				case trtPrepaidReaktivasi:
				case trtPrepaidVoidInquiry:
				case trtPrepaidVoid:
#endif
				goto lblJump;
				default:
				break;
			}


			ret = pmtComputeTotAmt();
			CHECK(ret > 0, lblKO);
			{
				char buf[30],buf1[30];

				memset(buf,0,sizeof(buf));
				mapGet(traTotAmt,buf,sizeof(buf));
				fmtAmt(buf1,buf,0,",.");
				fmtPad(buf1,-16,' ');
				MAPPUTSTR(msgBuffer,buf1,lblKO);
				ret = usrInfo(infTotal2);
				CHECK(ret > 0, lblKO);
				CHECK(ret == kbdVAL, lblExit);
			}

		}

		lblJump:
		if(txnType != trtReissuePIN
#ifdef PREPAID
			&& txnType != trtPrepaidAktivasi
			&& txnType != trtPrepaidRedeem
			&& txnType != trtPrepaidRedeemInquiry
			&& txnType != trtPrepaidReaktivasi
			&& txnType != trtPrepaidReaktivasiInquiry
			&& txnType != trtPrepaidVoid
			&& txnType != trtPrepaidVoidInquiry
#endif
			&& txnType != trtPembayaranPLNToken
		)
		{
			MAPGET(traAmt, Amt2, lblKO);
			MAPGETBYTE(appPINDebitFlag,res,lblKO);
			/*
			if((txnType == trtSale) && ((atoi(Amt2)) < 200000) && (res == 2))
				memset(Amt2, 0x00, sizeof(Amt2));
			else
			{
			*/ret = valIsPinRequired();
				CHECK(ret >= 0, lblKO);
				if(ret > 0)
				{
					ret = pmtPinInput();
					CHECK(ret > 0, lblKO);
				}
			//}
		}
	}

	ret = pmtBRICustomInput();
	CHECK(ret > 0, lblKO);

	ret = pmtBRIKonfirmasiTanpaInquiry(txnType);
	if(ret <= 0)
		goto lblEnd;

	MAPGETWORD(traMnuItm, key, lblKO);
	switch (key)
	{
		case mnuOffline:
			ret = pmtAskAppCod();
			break;

		default:
			pmtPrepareOfflineIfAvailable();
			ret = onlSession();
			break;
	}
	CHECK(ret > 0, lblKO);

	ret = 1;
	goto lblEnd;

	lblExit:
		ret = 0;
		goto lblEnd;

	lblKO:
		goto lblEnd;

	lblEnd:
		trcFN("pmtMagStripe: ret=%d\n", ret);
		return ret;

	//@@SIMAS_FALLBACK - start
	lblFallbackNotAllowed:
		usrInfo(infFallbackNotAllowed);
		goto lblEnd;	//@@SIMAS_FALLBACK - end
}


#ifdef __EMV__
static int pmtSmartCard(void)
{
	int 		ret;
	byte 	TrnTyp[1 + lenTrnTyp];
	char 	DatTim[lenDatTim + 1];
	byte 	TrnDat[1 + lenTrnDat];
	byte 	TrnTim[1 + lenTrnTim];
	card 	InvNum;
	byte 	TrnSeqCnt[1 + lenTrnSeqCnt];
	char 	buf[2 * lenTrnSeqCnt + 1];

	trcS("pmtSmartCard Beg\n");

	usrInfo(infProcessing);
	TrnTyp[0] = lenTrnTyp;
	VERIFY(TrnTyp[0] == 1);
	TrnTyp[1] = 0;
	ret = mapPut(emvTrnTyp, TrnTyp, 1 + lenTrnTyp);
	CHECK(ret >= 0, lblDBA);

	MAPGET(traDatTim, DatTim, lblDBA);
	TrnDat[0] = lenTrnDat;
	VERIFY(TrnDat[0] == 3);
	ret = hex2bin(&TrnDat[1], &DatTim[2], lenTrnDat);
	VERIFY(ret == 3);
	ret = mapPut(emvTrnDat, TrnDat, 1 + lenTrnDat);
	CHECK(ret >= 0, lblDBA);

	TrnTim[0] = lenTrnTim;
	VERIFY(TrnTim[0] == 3);
	ret = hex2bin(&TrnTim[1], &DatTim[2 + 6], lenTrnTim);
	VERIFY(ret == 3);
	ret = mapPut(emvTrnTim, TrnTim, 1 + lenTrnTim);
	CHECK(ret >= 0, lblDBA);

	MAPGETCARD(regInvNum, InvNum, lblDBA);
	TrnSeqCnt[0] = lenTrnSeqCnt;
	VERIFY(TrnSeqCnt[0] == 4);
	num2dec(buf, InvNum, 2 * lenTrnSeqCnt);
	hex2bin(&TrnSeqCnt[1], buf, lenTrnSeqCnt);
	ret = mapPut(emvTrnSeqCnt, TrnSeqCnt, 1 + lenTrnSeqCnt);
	CHECK(ret >= 0, lblDBA);

	ret = emvProcessing();
	CHECK(ret >= 0, lblKO);

	goto lblEnd;
	lblDBA:
		usrInfo(infDataAccessError);
		ret = -1;
		goto lblEnd;

	lblKO:
		goto lblEnd;

	lblEnd:
		trcFN("pmtSmartCard ret=%d\n", ret);
		return ret;
}
#endif

int dmyPmtSmartCard(void)
{
	int ret;

	MAPPUTSTR(traChipSwipe, " (Chip)", lblKO);
	MAPPUTBYTE(traEntMod, 'C', lblKO);
	MAPPUTBYTE(traTxnType, trtDummy, lblKO);
	emvFullProcessSet();
	ret = pmtSmartCard();

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		trcFN("pmtReceipt ret=%d\n", ret);
		return ret;
}

static int pmtLogSave(void)
{
	char RspCod[lenRspCod + 1];
	byte type;
	int 	ret;

	trcS("pmtLogSave Beg\n");

#ifdef PREPAID
	MAPGETBYTE(traTxnType,type,lblKO)
	if((type == trtPrepaidPayment) || (type == trtPrepaidPaymentDisc))
		goto saveBatch;

	if(type == trtPrepaidRedeemInquiry || type == trtPrepaidReaktivasiInquiry || type == trtPrepaidVoidInquiry)
	{
		ret = 0;
		goto lblEnd;
	}
#endif

	if(isBRITransaction() == 1)
	{
		if(isBRISaveToBatch() == 1)
			goto saveBatch;
		else
		{
			ret = 1;
			goto lblEnd;
		}
	}

	MAPGET(traRspCod, RspCod, lblKO);
	if(strcmp(RspCod, "00") != 0)
	{
		ret = 1;
		goto lblEnd;
	}

	saveBatch:
		ret = logSave();
		CHECK(ret >= 0, lblKO);
		goto lblEnd;

	lblKO:
		goto lblEnd;

	lblEnd:
		trcFN("pmtLogSave ret=%d\n", ret);
		return ret;
}

static int fmtAmtReceipt(word key, char *amt)
{
	int 	ret;
	char amtFmt[20+1];
	byte exp;

	MEM_ZERO(amtFmt);
	MAPGETBYTE(appExp, exp, lblKO);
	fmtAmt(amtFmt, amt, exp, ",.");
	ret = mapPut(key, amtFmt, strlen(amtFmt));
	CHECK(ret > 0, lblKO);

	return ret;
	lblKO:
		return -1;
}

static int pmtPrintEmvDebug(void) {
	char EntMod;
	int 	ret;

	MAPGETBYTE(traEntMod, EntMod, lblKO);
	if(debugEmvFlag == 1){
		if(EntMod == 'C' ||EntMod == 'c')
			ret = rptReceipt(rloEmvDebug);
		tmrSleep(3);
	}
	return 0;
	lblKO:
	return -1;
}

static int pmtReceipt(word key)
{
	int 		ret;
	char 	EntMod;
	word 	mnuItem;
	byte 	pinVer;
	byte 	singleReceipt=0;
	word 	txnType;
	byte		res;
	byte	dat[50]; //@@SIMAS-CVM_COMBI

	res = 0;
	txnType = key;
	ret = briPrintData(txnType, &key, &singleReceipt,0);
	if(ret < 0)
		goto lblKO;

	if(ret == 0)
	{
		ret = 1;
		goto lblEnd;
	}

	ret = rptReceipt(key);
	CHECK(ret >= 0, lblKO);

	if(singleReceipt)
	{
		rptReceipt(rloReceiptFeed);
		goto lblEnd;
	}
	if((isBRITransaction() == 1 && txnType != trtInstallment))
		ret = rptReceipt(rloReceiptTypeBankBRITrans);
#ifdef PREPAID
	else if(isPrepaidTransaction(1) == 1)
	{
		rptReceipt(rloReceiptTypeMerchantBRITrans);
	}
#endif
	else
	{
		mapGet(emvCVMRes, dat, sizeof(dat)); //@@SIMAS-CVM_COMBI
		MAPGETBYTE(traPinVer, pinVer, lblKO);

		//if(pinVer == 1)
		if ((pinVer == 1) &&  (dat[0]!=0x03)&&  (dat[0]!=0x05))//@@SIMAS-CVM_COMBI
			ret = rptReceipt(rloPinVerTypeMerchant);
		else
		{
			if((txnType == trtSale)||(txnType == trtSaleRedeem))
			{
				MAPGETBYTE(traEntMod, EntMod, lblKO);
				if((EntMod == 'M') || (EntMod == 'm'))
				{
					MAPGETBYTE(traPinReq,res,lblKO);

					if(res == 1)
						ret = rptReceipt(rloReceiptTypeJustMerchantBRITrans);
					else
						ret = rptReceipt(rloReceiptTypeJustMerchantBRITransSign);

				}
				else
				{
					/*MAPGETBYTE(appMQPSVEPSFlag,res,lblKO);
					MAPGET(appMQPSVEPSValue,buff,lblKO);
					mapGet(traAmt,amount,sizeof(amount));*/
					MAPGETBYTE(traPinReq,res,lblKO);

					if(res == 1)
						ret = rptReceipt(rloReceiptTypeMerchantNoSignature);
					else
						ret = rptReceipt(rloReceiptTypeMerchant);
				}
			}
			else
			{
				/*MAPGETBYTE(appMQPSVEPSFlag,res,lblKO);
				MAPGET(appMQPSVEPSValue,buff,lblKO);
				mapGet(traAmt,amount,sizeof(amount));
				*/
				MAPGETBYTE(traPinReq,res,lblKO);

				if(res == 1)
					ret = rptReceipt(rloReceiptTypeMerchantNoSignature);
				else
					ret = rptReceipt(rloReceiptTypeMerchant);
			}

		}
	}

	if( (((txnType == trtTarikTunai) || (txnType == trtVoidTarikTunai)) && (singleReceipt == 0))
#ifdef PREPAID
	|| ((isPrepaidTransaction(1)== 1) && (txnType != trtPrepaidPayment)) || ((isPrepaidTransaction(1)== 1) && (txnType != trtPrepaidPaymentDisc))
#endif
	)
		goto lblCustomerCopy;
	ret = usrInfo(infBankCopyConfirm);
	if(ret != kbdANN)
	{
		ret = usrInfo(infBankCopyPrinting);
		CHECK(ret >= 0, lblKO);
		ret = rptReceipt(key);

		if((EntMod == 'C') || (EntMod == 'c'))
		{
			if(mnuItem != mnuInstallment)
			rptReceipt(rloSpace);
		}

		CHECK(ret >= 0, lblKO);

		if(singleReceipt)
		{
			rptReceipt(rloReceiptFeed);
			goto lblEnd;
		}
		if(isBRITransaction() == 1&& txnType != trtInstallment)
			ret = rptReceipt(rloReceiptTypeBankBRITrans);
#ifdef PREPAID
		else if(isPrepaidTransaction(1) == 1)
		{
			rptReceipt(rloReceiptTypeBankBRITrans);
		}
#endif
		else
		{
			if(pinVer == 1)
				ret = rptReceipt(rloPinVerTypeBank);
			else
				if((txnType == trtSale) || (txnType == trtSaleRedeem))
				{
					MAPGETBYTE(traEntMod, EntMod, lblKO);
					if((EntMod == 'M') || (EntMod == 'm'))
						ret = rptReceipt(rloReceiptTypeJustBankBRITrans);
					else
					{
						/*MAPGETBYTE(appMQPSVEPSFlag,res,lblKO);
						MAPGET(appMQPSVEPSValue,buff,lblKO);
						mapGet(traAmt,amount,sizeof(amount));
						*/
						MAPGETBYTE(traPinReq,res,lblKO);

						if(res == 1)
							ret = rptReceipt(rloReceiptTypeBankNoSignature);
						else
							ret = rptReceipt(rloReceiptTypeBank);
					}
				}
				else
				{

					/*MAPGETBYTE(appMQPSVEPSFlag,res,lblKO);
					MAPGET(appMQPSVEPSValue,buff,lblKO);
					mapGet(traAmt,amount,sizeof(amount));

					if((res == 2)&&(atoi(amount) < atoi( buff)))
						ret = rptReceipt(rloReceiptTypeBankNoSignature);
					else
						*/

					MAPGETBYTE(traPinReq,res,lblKO);

					if(res == 1)
						ret = rptReceipt(rloReceiptTypeBankNoSignature);
					else
						ret = rptReceipt(rloReceiptTypeBank);
				}
		}
	}

	lblCustomerCopy:
	ret = usrInfo(infCustCopyConfirm);
	if(ret != kbdANN)
	{
		ret = usrInfo(infCustCopyPrinting);
		CHECK(ret >= 0, lblKO);

		MAPPUTBYTE(traCustFlag, 1, lblKO);
		ret = rptReceipt(key);
		if((EntMod == 'C') || (EntMod == 'c'))
		{
			if(mnuItem != mnuInstallment)
				rptReceipt(rloSpace);
		}

		CHECK(ret >= 0, lblKO);

		if(isBRITransaction() == 1 && txnType != trtInstallment)
			ret = rptReceipt(rloReceiptTypeCustomerBRITrans);
#ifdef PREPAID
		else if(isPrepaidTransaction(1) == 1)
			rptReceipt(rloReceiptTypeCustomerBRITrans);
#endif
		else
		{
			if(pinVer == 1)
				ret = rptReceipt(rloPinVerTypeCustomer);
			else
				if((txnType == trtSale) || (txnType == trtSaleRedeem))
				{
					MAPGETBYTE(traEntMod, EntMod, lblKO);
					if((EntMod == 'M') || (EntMod == 'm'))
						ret = rptReceipt(rloReceiptTypeJustCustomerBRITrans);
					else
						ret = rptReceipt(rloReceiptTypeCustomer);
				}
				else
					ret = rptReceipt(rloReceiptTypeCustomer);
		}
	}

	MAPPUTBYTE(traCustFlag, 0, lblKO);
	goto lblEnd;

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		trcFN("pmtReceipt ret=%d\n", ret);
		return ret;
}


int pmtIncrementInv(void)
{
	int 	ret;
	card invNum;

	MAPGETCARD(regInvNum, invNum, lblKO);
	if(invNum >= 999999)
	invNum = 0;
	invNum++;
	MAPPUTCARD(regInvNum, invNum, lblKO);
	goto lblEnd;

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		trcFN("pmtIncrementInv ret=%d\n", ret);
		return ret;
}


int pmtPayment(word mnuItem)
{
	int 		ret;
	word 	inf = 0;
	char 	EntMod;
	char 	Trk2[lenTrk2 + 1];
	char 	Pan[lenPan + 1];
	char 	ExpDat[lenExpDat + 1];
	char 	tmp[dspW * 2 + 1];
	char 	*ptr;
	byte 	TxnType;
	char 	BlockedPan[lenPan+1];
	char 	Srvcod[3+1];
	char 	CID[lenCID + 1];
	byte 	ServiceCode;
	byte 	EmvTrnType;
	byte 	FallBack;
	byte 	acquirerId;
	byte 	tempCardFlag=0;
	card 	dupROC;

	trcS("pmtPayment Beg\n");
	memset(BlockedPan, '*', sizeof BlockedPan);
	MAPGETBYTE(traTxnType, TxnType, lblDBA);
	ret = batchSync();
	if(ret < 0)
		goto lblStop;



	MAPGETBYTE(appTempStanFlag, tempCardFlag, lblStop);
	if(tempCardFlag==1)
	{
		MAPPUTBYTE(appTempStanFlag, 0, lblStop);
		MAPGETCARD(regInvNumTemp, dupROC, lblStop);
		MAPPUTCARD(regInvNum, dupROC, lblStop);
	}

	ret = valLogIsNotFull();
	CHECK(ret > 0, lblStop);

	txnAsli = 0;
	{
		card cur, dup;

		MAPGETCARD(regInvNum, cur, lblDBA);
		MAPGETCARD(regDupInvNum, dup, lblDBA);
		if(dup > cur)
		{
			if(dup >= 1000000)
				MAPPUTCARD(regInvNum, dup-1000000, lblDBA);
		}
		MAPPUTCARD(regDupInvNum, 0, lblDBA);
	}
	MAPGETBYTE(traEntMod, EntMod, lblDBA);

#ifdef PREPAID
	switch(TxnType)
	{
		case trtPrepaidInfoDeposit:
			ret = pmtPrepaidInfoDeposit();
			CHECK(ret >= 0, lblStop);
			goto lblJump;

		case trtPrepaidPaymentDisc:
			ret = briGetAcq(DEBIT_CARD, &acquirerId);
			if(ret == FAIL)
				goto lblStop;
			acquirerId--;

			if(checkPendingSettlement()!=0)
			{
				ret = -100;
				goto lblStop;
			}

			ret = pmtPrepaidPaymentDisc();
			CHECK(ret >= 0, lblStop);
			goto lblJump;

		case trtPrepaidPayment:
			ret = briGetAcq(DEBIT_CARD, &acquirerId);
			if(ret == FAIL)
				goto lblStop;
			acquirerId--;

			if(checkPendingSettlement()!=0)
			{
				ret = -100;
				goto lblStop;
			}

			ret = pmtPrepaidPayment();
			CHECK(ret >= 0, lblStop);
			goto lblJump;

		case trtPrepaidRedeemInquiry:
		case trtPrepaidRedeem:
		case trtPrepaidAktivasi:
		case trtPrepaidReaktivasiInquiry:
		case trtPrepaidReaktivasi:
		case trtPrepaidVoidInquiry:
		case trtPrepaidVoid:
			ret = briGetAcq(DEBIT_CARD, &acquirerId);
			if(ret == FAIL)
			goto lblStop;
			acquirerId--;

			if(checkPendingSettlement()!=0)
			{
				ret = -100;
				goto lblStop;
			}
			MAPPUTBYTE(traAcqIdx, acquirerId, lblStop);
			mapMove(acqBeg, (word) acquirerId);
			break;

		default:
			break;
	}
#endif

	MAPGETBYTE(appServiceCode, ServiceCode, lblDBA);
	MAPGETBYTE(traFallback, FallBack, lblEnd);
	switch (EntMod)
	{
		case 'M':
			MAPGET(traTrk2, Trk2, lblDBA);

			ptr = Trk2;
			ret = fmtTok(0, ptr, "=");
			CHECK(ret <= lenPan, lblInvalidTrk);
			ret = fmtTok(Pan, ptr, "=");
			VERIFY(ret <= lenPan);

			ptr += ret;
			VERIFY(*ptr == '=');
			ptr++;
			ret = fmtSbs(ExpDat, ptr, 0, lenExpDat);
			VERIFY(ret == 4);
			MAPPUTSTR(traExpDat, ExpDat, lblDBA);

			ptr += 4;
			ret = fmtSbs(Srvcod, ptr, 0, 3);
			VERIFY(ret == 3);
			if(ServiceCode == 1){
				MAPPUTSTR(traSrvCod, Srvcod, lblDBA);
				if((Srvcod[0] == '2')||(Srvcod[0] == '6'))
					UseChip = 1;
				else
					UseChip = 0;
			}
			strcpy(tmp, Pan);
			strcat(tmp, "=");
			strcat(tmp, ExpDat);
			ret = mapPut(traTrk21, tmp, dspW);
			CHECK(ret >= 0, lblDBA);
			MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

			MAPPUTSTR(traPan, Pan, lblDBA);
			memcpy(BlockedPan + 12, Pan + 12, lenPan-12);
			BlockedPan[sizeof(BlockedPan)-1]=0;
			MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);
			break;

		case 'C':
			break;

		default:
			switch (TxnType)
			{
				case trtVoidTarikTunai:
				case trtVoid:
					ret = pmtTxnOthers(TxnType);
					if(ret == -3)
						goto lblDeclined;
					CHECK(ret > 0, lblDBA);
					break;

				case trtManual:
					ret = pmtManInput();
					if(ret != kbdVAL)
					{
						ret = 0;
						goto lblEnd;
					}
					EntMod = 'k';
					MAPPUTBYTE(traEntMod, (byte) EntMod, lblDBA);
					break;

				case trtPembayaranPLNToken:
					MAPPUTSTR(traTrk1, " ", lblDBA);
					break;
#ifdef PREPAID
				case trtPrepaidRedeemInquiry:
				case trtPrepaidRedeem:
				case trtPrepaidAktivasi:
				case trtPrepaidReaktivasiInquiry:
				case trtPrepaidReaktivasi:
					MAPPUTSTR(traTrk1, " ", lblDBA);
					break;

				case trtPrepaidVoidInquiry:
					ret = getPrepaidVoidTrans();
					if(ret == -3)
						goto lblDeclined;
					CHECK(ret > 0, lblEnd);
					break;
#endif
				default:
					ret = pmtCardInput();
					CHECK(ret > 0, lblStop);
					EntMod = (char) ret;
					MAPPUTBYTE(traEntMod, (byte) EntMod, lblDBA);
					break;
			}
			break;
		}

		switch (EntMod)
		{
			case 'f':
				if((TxnType != trtVoid) || (TxnType != trtManual || TxnType != trtVoidTarikTunai) || (TxnType != trtSaleRedeem))
				{
					MAPPUTSTR(traChipSwipe, "-FALLBACK",lblStop);
					ret = pmtMagStripe();
					CHECK(ret >=0, lblStop);
				}
				break;

			case 'm':
			case 'M':
				if((TxnType != trtVoid) || (TxnType != trtManual || TxnType != trtVoidTarikTunai) || (TxnType != trtSaleRedeem))
				{
					MAPPUTSTR(traChipSwipe, "-SWIPE", lblStop);
					ret = pmtMagStripe();
					CHECK(ret >=0, lblStop);
				}
				break;

			case 'c':
			case 'C':
#ifdef __EMV__
				MAPPUTSTR(traChipSwipe, " (Chip)", lblStop);
				emvFullProcessSet();
				ret = pmtSmartCard();
				if(ret < 0)
					goto lblEnd;
#endif
				if(CheckFallBack(ret) == 1)
				{
					dspClear();
					usrInfo(infUnknownAID);
					removeCard();

					MAPPUTBYTE(traEntMod, 'f', lblDBA);
					MAPPUTSTR(traChipSwipe, "-FALLBACK", lblStop);
					pmtSale();
					goto lblFinish;
				}

				if (emvFullProcessCheck() == 0 )
				{
					byte emvTrack2Bcd[lenEmvTrk2+ 1];
					card track2Len;
					char emvTrack2Hex[lenTrk2+ 1];

					MEM_ZERO(emvTrack2Hex);
					MEM_ZERO(emvTrack2Bcd);
					mapGet(emvTrk2, emvTrack2Bcd, sizeof(emvTrack2Bcd));
					bin2num(&track2Len, &emvTrack2Bcd[0], 1);
					bin2hex(emvTrack2Hex, emvTrack2Bcd+1,track2Len);
					mapPut(traTrk2, emvTrack2Hex, (track2Len*2)-1);

					MAPGET(traTrk2, Trk2, lblDBA);

					ptr = Trk2;
					ret = fmtTok(0, ptr, "D");
					CHECK(ret <= lenPan, lblInvalidTrk);
					ret = fmtTok(Pan, ptr, "D");
					VERIFY(ret <= lenPan);

					ptr += ret;
					VERIFY(*ptr == 'D');
					ptr++;
					ret = fmtSbs(ExpDat, ptr, 0, lenExpDat);
					VERIFY(ret == 4);
					MAPPUTSTR(traExpDat, ExpDat, lblDBA);

					strcpy(tmp, Pan);
					strcat(tmp, "=");
					strcat(tmp, ExpDat);
					ret = mapPut(traTrk21, tmp, dspW);
					CHECK(ret >= 0, lblDBA);
					MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

					MAPPUTSTR(traPan, Pan, lblDBA);
					memcpy(BlockedPan + 12, Pan + 12, lenPan-12);
					BlockedPan[sizeof(BlockedPan)-1]=0;
					MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);

					MAPPUTSTR(traChipSwipe, "-DIP", lblStop);
					ret = pmtMagStripe();
					CHECK(ret >=0, lblStop);
				}
				break;


			default:
#ifdef PREPAID
				if((TxnType == trtPrepaidAktivasi) || (TxnType == trtPrepaidRedeem) || (TxnType == trtPrepaidRedeemInquiry)
					|| (TxnType == trtPrepaidReaktivasiInquiry) || (TxnType == trtPrepaidReaktivasi) || (TxnType == trtPrepaidVoidInquiry)
					|| (TxnType == trtPrepaidVoid)
				)
				{
					MAPPUTSTR(traCardType, "BRIZZI", lblDBA);
					ret = pmtMagStripe();
					CHECK(ret >=0, lblStop);
					break;
				}
#endif
				if((TxnType != trtVoid) && (TxnType != trtVoidTarikTunai)) {
					CHECK(EntMod == 'k', lblInvalidTrk);
					ret = pmtMagStripe();
					CHECK(ret >= 0, lblStop);
				}
				break;
		}

#ifdef PREPAID
	MAPGETBYTE(traTxnType, TxnType, lblDBA);
	if(TxnType == trtPrepaidTopUp)
	{
		ret = valRspCod();
		CHECK(ret > 0, lblReversalAndDeclined);
		ret = pmtPrepaidTopUp();
		CHECK(ret > 0, lblSendReversal);
		ret = clearPrepaidReversalFlag();
		CHECK(ret > 0, lblDBA);
	}

	if(TxnType == trtPrepaidAktivasi)
	{
		ret = valRspCod();
		CHECK(ret > 0, lblReversalAndDeclined);
		ret = pmtPrepaidAktivasiDeposit();
		CHECK(ret > 0, lblSendReversal);
		ret = clearPrepaidReversalFlag();
		CHECK(ret > 0, lblDBA);
	}

	if(TxnType == trtPrepaidRedeem)
	{
		ret = valRspCod();
		CHECK(ret > 0, lblReversalAndDeclined);

		{
			PREPAID_T dat;
			char nomorKartu1[30];
			char nomorKartu2[30];

			ret = getPrepaidCardInfo(&dat);
			CHECK(ret > 0, lblSendReversal);

			memset(nomorKartu1,0,sizeof(nomorKartu1));
			memset(nomorKartu2,0,sizeof(nomorKartu2));
			bin2hex(nomorKartu1,dat.cardNumber,CARD_NUMBER_LEN);

			ret = mapGet(traPan,nomorKartu2,sizeof(nomorKartu2));
			CHECK(ret > 0, lblSendReversal);

			if(strcmp(nomorKartu1,nomorKartu2)!=0)
				goto lblSendReversal;
		}

		ret = pmtPrepaidRedeem();
		CHECK(ret > 0, lblSendReversal);
		ret = clearPrepaidReversalFlag();
		CHECK(ret > 0, lblDBA);
	}

	if(TxnType == trtPrepaidReaktivasi)
	{
		ret = valRspCod();
		CHECK(ret > 0, lblReversalAndDeclined);
		ret = pmtPrepaidReaktivasi();
		CHECK(ret > 0, lblSendReversal);
		ret = clearPrepaidReversalFlag();
		CHECK(ret > 0, lblDBA);
	}

	if(TxnType == trtPrepaidVoid)
	{
		ret = valRspCod();
		CHECK(ret > 0, lblReversalAndDeclined);
		ret = pmtPrepaidVoid();
		CHECK(ret > 0, lblSendReversal);
		ret = clearPrepaidReversalFlag();
		CHECK(ret > 0, lblDBA);
	}

	if(TxnType == trtPrepaidRedeemInquiry || TxnType == trtPrepaidVoidInquiry || TxnType == trtPrepaidReaktivasiInquiry)
		goto lblReversalAndDeclined;

	lblJump:
#endif
		CHECK(ret >= 0, lblStop);
		MAPGETBYTE(traTxnType, TxnType, lblDBA);
		if((TxnType != trtOffline && TxnType != trtPrepaidPayment) && (TxnType != trtOffline && TxnType != trtPrepaidPaymentDisc)) {
#if 1
			if((emvFullProcessCheck() == 1) && (mnuItem != mnuVoid) && (mnuItem != mnuVoidTarikTunai))
			{
				MAPGET(emvCID, CID, lblDBA);
				MAPGETBYTE(traEmvTrnType, EmvTrnType, lblDBA);
				switch(CID[1] & 0xC0)
				{
					case 0x40:
						if(EmvTrnType == CID_GEN1){
							TxnType = trtOffline;
							ret = incCard(regAutCodOff);
							CHECK(ret >= 0, lblStop);
						}
						else if(EmvTrnType == CID_GEN2)
							;
						break;

					case 0x80:
						ret = valRspCod();
						CHECK(ret > 0, lblDeclined);
						break;

					case 0xC0:
						ret = valRspCod();
						CHECK(ret > 0, lblDeclined);
						break;

					case 0x00:
						if((mnuItem != mnuVoid) && (mnuItem != mnuVoidTarikTunai)&&(mnuItem != mnuPreCom)&&(mnuItem != mnuPreCancel))
						{
							ret = incCard(regInvNum);
							CHECK(ret > 0, lblStop);
						}
						doBRIReversal();
						goto lblDeclined;
				}
			}
			else
#endif
			{
				ret = valRspCod();
				CHECK(ret > 0, lblDeclined);
			}

		}
		else {
			if(TxnType != trtPreCancel&& TxnType != trtPreCom)
			{
				ret = incCard(regSTAN);
				CHECK(ret >= 0, lblStop);
			}
		}

		ret = pmtLogSave();
		CHECK(ret > 0, lblStop);
		ret = pmtReceipt(TxnType);
		CHECK(ret > 0, lblStop);
		if((mnuItem != mnuVoid) && (mnuItem != mnuVoidTarikTunai)&&(mnuItem != mnuPreCom)&&(mnuItem != mnuPreCancel))
		{
			ret = incCard(regInvNum);
			CHECK(ret > 0, lblStop);
		}

		if(mnuItem == mnuVoid)
			logLoadCurContext();

		if(mnuItem == mnuVoidTarikTunai)
			briLogLoadCurContext(TUNAI_LOG);

		if((mnuItem == mnuVoid) || (mnuItem == mnuVoidTarikTunai))
		{
			card dup;

			MAPGETCARD(regDupSTAN,dup,lblDBA);
			MAPPUTCARD(regSTAN,dup,lblDBA);
			incCard(regSTAN);
		}

		if(EntMod == 'c' || EntMod == 'C') {

		}

		ret = 1;
		goto lblEnd;

	lblReversalAndDeclined:
		ret = clearPrepaidReversalFlag();
		CHECK(ret > 0, lblDBA);
		goto lblDeclined;

	lblInvalidTrk:
		inf = infInvalidTrack;
		ret = -1;
		goto lblEnd;

	lblDBA:
		if(!ret)
			goto lblEnd;
		ret = -2;
		inf = infDataAccessError;
		goto lblEnd;

	lblDeclined:
		inf = inf;
		ret = -3;
		goto lblEnd;

	lblStop:
		if(!ret)
			goto lblEnd;
		if(ret == -100)
			goto lblEnd;
		inf = infProcessingError;
		ret = 0;
		goto lblEnd;

	lblSendReversal:
		ret = doPrepaidReversal();
		goto lblEnd;

	lblEnd:
		comHangStart();
		comHangWait();
		DisconnectFromHost();
		pmtPrintEmvDebug();
		if(inf)
			usrInfo(inf);
		if(EntMod == 'c' || EntMod == 'C') {
			dspClear();
			usrInfo(infRemoveCard);
			removeCard();
		}

	lblFinish:
		return ret;
		trcS("pmtPayment End\n");
}


void pmtSale(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtSale Beg\n");

	mapMove(rqsBeg, (word) (trtSale - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtSale, lblKO);

	MAPPUTBYTE(regLocType, 'T', lblKO);

	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);

	pmtPayment(mnuSale);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtSale End\n");
}


void pmtSaleRedeem(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtSaleRedeem Beg\n");

	mapMove(rqsBeg, (word) (trtSaleRedeem - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtSaleRedeem, lblKO);

	MAPPUTBYTE(regLocType, 'T', lblKO);

	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);

	pmtPayment(mnuSaleRedeem);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtSale End\n");
}


void pmtPreaut(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtPreaut Beg\n");

	mapMove(rqsBeg, (word) (trtPreAut - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtPreAut, lblKO);

	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
	pmtPayment(mnuPreaut);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtPreaut End\n");
}

void pmtPreAut(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtPreAut Beg\n");

	mapMove(rqsBeg, (word) (trtPreAut - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtPreAut, lblKO);

	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
	pmtPayment(mnuPreAuth);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtPreAut End\n");
}


void pmtAut(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtAut Beg\n");

	mapMove(rqsBeg, (word) (trtAut - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtAut, lblKO);
	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
	pmtPayment(mnuAut);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtAut End\n");
}

void pmtPreAuth(void)
{
	int 	ret;
	char traName[dspW + 1];
	byte tempCardFlag;

	trcS("pmtPreAuth Beg\n");

	mapMove(rqsBeg, (word) (trtPreAuth - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtPreAuth, lblKO);
	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
	MAPGETBYTE(appTempStanVoidFlag, tempCardFlag, lblKO);
	if(tempCardFlag==1)
	{
		 incCard(regInvNumTemp);
		 MAPPUTBYTE(appTempStanVoidFlag, 0, lblKO);
	}
	pmtPayment(mnuPreAuth);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtPreAuth End\n");
}

void pmtPreCancel(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtPreCancel Beg\n");

	mapMove(rqsBeg, (word) (trtPreCancel - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtPreCancel, lblKO);
	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
	pmtPayment(mnuPreCancel);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtPreCancel End\n");
}

void pmtPreCom(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtPreCom Beg\n");

	mapMove(rqsBeg, (word) (trtPreCom - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtPreCom, lblKO);
	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
	pmtPayment(mnuPreCom);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtPreCom End\n");
}

void pmtRefund(void)
{
	int ret;
	char traName[dspW + 1];
	byte LocOpt;

	trcS("pmtRefund Beg\n");

	MAPGETBYTE(appLocOpt, LocOpt, lblKO);
	if(LocOpt & 0x04)
	{
		ret = valAskPwd(appRefPwd);
		CHECK((ret > 0) && (ret != kbdANN), lblKO);
	}

	mapMove(rqsBeg, (word) (trtRefund - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtRefund, lblKO);
	pmtPayment(mnuRefund);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtRefund End\n");
}


void pmtOffline(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtOffline Beg\n");

	mapMove(rqsBeg, (word) (trtOffline - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtOffline, lblKO);
	pmtPayment(mnuOffline);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtOffline End\n");
}


void pmtVoid(void)
{
	int	ret;
	char traName[dspW + 1];
	byte LocOpt;
	byte firstPowerOn; //@@SIMAS-INIT_DB
	byte revFlag;//@@SIMAS-INIT_DB

	trcS("pmtVoid Beg\n");

	MAPGETBYTE(appLocOpt, LocOpt, lblKO);
	if(LocOpt & 0x02)
	{
		ret = valAskPwd(appVoidPwd);
		CHECK((ret > 0) && (ret != kbdANN), lblKO);
	}

	mapMove(rqsBeg, (word) (trtVoid - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtVoid, lblKO);
	//@@SIMAS-INIT_DB - start
	MAPGETBYTE(appIsFirstPowerOnTemp,firstPowerOn,lblKO);
	MAPGETBYTE(revReversalFlag, revFlag, lblKO);
	if((firstPowerOn== 1)&&(revFlag==0))
	{
		admRevResetWoKey();
		admTCResetWoKey();
		setFirstPowerOnTempFlag(0);
	}
	//@@SIMAS-INIT_DB - end

	pmtPayment(mnuVoid);
	goto lblEnd;

	lblKO:
	lblEnd:
	trcS("pmtVoid End\n");
}


void pmtAdjust(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtAdjust Beg\n");

	mapMove(rqsBeg, (word) (trtAdjust - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtAdjust End\n");
}


void pmtManual(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtManual Beg\n");

	mapMove(rqsBeg, (word) (trtManual - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtManual, lblKO);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtManual End\n");
}


void BlockPan(char *origPan,char *blockedPan)
{
	int numBeginBlocked=0,numEndBlocked=0,i;
	int len=0,nn=0;

	while(origPan[len]!=0)
		len++;

	mapGet(appPANBlockBegin,&numBeginBlocked,sizeof(int));
	mapGet(appPANBlockEnd,&numEndBlocked,sizeof(int));

	memcpy(blockedPan,origPan,len);
	blockedPan[len] = 0;

	for(i=0;i<len;i++)
	{
		if(i<numBeginBlocked)
		blockedPan[i] = 'X';
	}

	for(i=len-1;i>=0;i--)
	{
		if(nn<numEndBlocked)
		{
			blockedPan[i] = 'X';
			nn++;
		}
	}
}

int pmtFallback()
{
	int 	ret;

	dspClear();
	removeCard();
	mapPutWord(traMnuItm, mnuSale);
	MAPPUTBYTE(traFallback, 1, lblKO);
	mapPutByte(traEntMod, 'f');
	mapPutStr(traChipSwipe, "-FALLBACK");
	pmtSale();
	return 1;

	lblKO:
		return 0;
}


void pmtBalance(void)
{
	int 	ret;
	char traName[dspW + 1];

	trcS("pmtBalance Beg\n");

	mapMove(rqsBeg, (word) (trtBalance - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtBalance, lblKO);
	pmtPayment(mnuBalance);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtBalance End\n");
}


void pmtVoidTarikTunai(void) {
	int 	ret;
	char traName[dspW + 1];
	byte LocOpt;

	trcS("pmtVoid Beg\n");

#ifdef TEST_VOID
	voidFlag = 1;
#endif

	MAPGETBYTE(appLocOpt, LocOpt, lblKO);
	if(LocOpt & 0x02) {
		ret = valAskPwd(appVoidPwd);
		CHECK((ret > 0) && (ret != kbdANN), lblKO);
	}

	mapMove(rqsBeg, (word) (trtVoidTarikTunai - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtVoidTarikTunai, lblKO);

	pmtPayment(mnuVoidTarikTunai);
	goto lblEnd;

	lblKO:
	lblEnd:
		trcS("pmtVoid End\n");
}

void pmtBRI(void)
{
	int 		ret;
	word 	key, txnType, subType = 0, nextTxn = 0;
	char 	traName[dspW + 1];
	byte 	isInquiry = 0;

	trcS("pmtBRI Beg\n");

	ret = mapGetWord(traMnuItm, key);
	CHECK(ret >= 0, lblKO);
	VERIFY(key);

	switch(key)
	{
		case mnuInfoSaldo:
			txnType = trtInfoSaldo;
			strcpy(traName,"INFO SALDO");
			break;

		case mnuInfoSaldoBankLain:
			strcpy(traName,"INFO BANK LAIN");
			txnType = trtInfoSaldoBankLain;
			break;

		case mnuMiniStatement:
			strcpy(traName,"MINI STATEMENT");
			txnType = trtMiniStatement;
			break;

		case mnuMutRek:
			strcpy(traName,"MUTASI REKENING");
			txnType = trtMutRek;
			break;

		 case mnuPrevilege:
			strcpy(traName,"PREVILEGE");
			txnType = trtPrevilege;
			break;

		case mnuTransferSesamaBRI:
			isInquiry = 1;
			strcpy(traName,"TRNF SESAMA BRI");
			txnType = trtTransferSesamaBRIInquiry;
			nextTxn = trtTransferSesamaBRI;
			break;

		case mnuTransferAntarBank:
			isInquiry = 1;
			strcpy(traName,"TRNF ANTAR BANK");
			txnType = trtTransferAntarBankInquiry;
			nextTxn = trtTransferAntarBank;
			break;

		case mnuInfoKodeBank:
			strcpy(traName,"INFO KODE BANK");
			txnType = trtInfoKodeBank;
			break;

		case mnuPembayaranTvInd:
			isInquiry = 1;
			strcpy(traName,"TV INDOVISION");
			subType = TV_INDOVISION;
			txnType = trtPembayaranTVBerlanggananInquiry;
			nextTxn = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTvTopTv:
			isInquiry = 1;
			strcpy(traName,"TV TOPTV");
			subType = TV_TOPTV;
			txnType = trtPembayaranTVBerlanggananInquiry;
			nextTxn = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTvOkTv:
			isInquiry = 1;
			strcpy(traName,"TV OKTV");
			subType = TV_OKTV;
			txnType = trtPembayaranTVBerlanggananInquiry;
			nextTxn = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTiketGaruda:
			isInquiry = 1;
			strcpy(traName,"GARUDA");
			subType = TIKET_GARUDA;
			txnType = trtPembayaranTiketPesawatInquiry;
			nextTxn = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketLionAir:
			isInquiry = 1;
			strcpy(traName,"LION AIR");
			subType = TIKET_LION_AIR;
			txnType = trtPembayaranTiketPesawatInquiry;
			nextTxn = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketSriwijaya:
			isInquiry = 1;
			strcpy(traName,"SRIWIJAYA");
			subType = TIKET_SRIWIJAYA_AIR;
			txnType = trtPembayaranTiketPesawatInquiry;
			nextTxn = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketMandala:
			isInquiry = 1;
			strcpy(traName,"MANDALA");
			subType = TIKET_MANDALA;
			txnType = trtPembayaranTiketPesawatInquiry;
			nextTxn = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranPdamSby:
			isInquiry = 1;
			strcpy(traName,"PDAM SBY");
			subType = PEMB_PDAMSBY;
			txnType = trtPembayaranPdamInquiry;
			nextTxn = trtPembayaranPdam;
			break;

		case mnuPembayaranTelkom:
			isInquiry = 1;
			strcpy(traName,"PEMB. TELKOM");
			txnType = trtPembayaranTelkomInquiry;
			nextTxn = trtPembayaranTelkom;
			break;

		case mnuPembayaranDPLKR:
			isInquiry = 1;
			strcpy(traName,"PEMB. DPLK");
			txnType = trtPembayaranDPLKR;
			break;

		case mnuPembayaranBriva:
			isInquiry = 1;
			strcpy(traName,"PEMB. BRIVA");
			txnType = trtPembayaranBrivaInquiry;
			nextTxn = trtPembayaranBriva;
			break;

		case mnuRegistrasiSmsBanking:
			strcpy(traName,"REG SMS BANKING");
			txnType = trtRegistrasiSmsBanking;
			break;

		case mnuRegistrasiTrxTransfer:
			isInquiry = 1;
			strcpy(traName,"TRX PHONE TRANSFER");
			txnType = trtTrxPhoneBankingTransferInquiry;
			nextTxn = trtTrxPhoneBankingTransfer;
			break;

		case mnuRegistrasiTrxPulsa:
			strcpy(traName,"TRX PHONE PULSA");
			txnType = trtTrxPhoneBankingPulsa;
			break;

		case mnuPembayaranKKBRI:
			strcpy(traName,"PEMB. KK BRI");
			txnType = trtPembayaranKKBRIInquiry;
			nextTxn = trtPembayaranKKBRI;
			break;

		case mnuPembayaranKKCITIKK:
			strcpy(traName,"PEMB. KK CITIBANK");
			txnType = trtPembayaranKKCITIKK;
			break;

		case mnuPembayaranKKCITIKTA:
			strcpy(traName,"PEMB. KTA CITIBANK");
			txnType = trtPembayaranKKCITIKTA;
			break;

		case mnuPembayaranKKStanCharKK:
			strcpy(traName,"PEMB. KK SCB");
			txnType = trtPembayaranKKStanCharKK;
			break;

		case mnuPembayaranKKStanCharKTA:
			strcpy(traName,"PEMB. KTA SCB");
			txnType = trtPembayaranKKStanCharKTA;
			break;

		case mnuPembayaranKKHSBCKK:
			strcpy(traName,"PEMB. KK HSBC");
			txnType = trtPembayaranKKHSBCKK;
			break;

		case mnuPembayaranKKHSBCKTA:
			strcpy(traName,"PEMB. KTA HSBC");
			txnType = trtPembayaranKKHSBCKTA;
			break;

		case mnuPembayaranKKRBSKK:
			strcpy(traName,"PEMB. KK RBS");
			txnType = trtPembayaranKKRBSKK;
			break;

		case mnuPembayaranKKRBSKTA:
			strcpy(traName,"PEMB. KTA RBS");
			txnType = trtPembayaranKKRBSKTA;
			break;

		case mnuPembayaranKKANZ:
			strcpy(traName,"PEMB. KK ANZ");
			txnType = trtPembayaranKKANZ;
			break;

		case mnuPembayaranSPP:
			strcpy(traName,"PEMB. SPP");
			txnType = trtPembayaranSPPInquiry;
			nextTxn = trtPembayaranSPP;
			break;

		case mnuPembayaranPLNPasca:
			isInquiry = 1;
			strcpy(traName,"PEMBAYARAN PLN PASCA");
			txnType = trtPembayaranPLNPascaInquiry;
			nextTxn = trtPembayaranPLNPasca;
			break;

		case mnuPembayaranPLNPra:
			isInquiry = 1;
			strcpy(traName,"PEMBAYARAN PLN PRA");
			txnType = trtPembayaranPLNPraInquiry;
			nextTxn = trtPembayaranPLNPra;
			break;

		case mnuPembayaranPLNToken:
			isInquiry = 1;
			strcpy(traName,"PEMBAYARAN PLN TOKEN");
			txnType = trtPembayaranPLNToken;
			MAPPUTBYTE(traEntMod,'k',lblEnd);
			break;

		case mnuPembayaranCicilanFIF:
			isInquiry = 1;
			strcpy(traName,"CICILAN FIF");
			subType = CICILAN_FIF;
			txnType = trtPembayaranCicilanInquiry;
			nextTxn = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanBAF:
			isInquiry = 1;
			strcpy(traName,"CICILAN BAF");
			subType = CICILAN_BAF;
			txnType = trtPembayaranCicilanInquiry;
			nextTxn = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanOTO:
			isInquiry = 1;
			strcpy(traName,"CICILAN OTO");
			subType = CICILAN_OTO;
			txnType = trtPembayaranCicilanInquiry;
			nextTxn = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanFinansia:
			isInquiry = 1;
			strcpy(traName,"CICILAN FINANSIA");
			subType = CICILAN_FNS;
			txnType = trtPembayaranCicilanInquiry;
			nextTxn = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanVerena:
			isInquiry = 1;
			strcpy(traName,"CICILAN VERENA");
			subType = CICILAN_VER;
			txnType = trtPembayaranCicilanInquiry;
			nextTxn = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanWOM:
			isInquiry = 1;
			strcpy(traName,"CICILAN WOM");
			subType = CICILAN_WOM;
			txnType = trtPembayaranCicilanInquiry;
			nextTxn = trtPembayaranCicilan;
			break;

		case mnuPembayaranSimpati50:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranSimpati100:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

/*		case mnuPembayaranSimpati150:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;
*/
		case mnuPembayaranSimpati200:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"200000",lblKO);
			break;

		case mnuPembayaranSimpati300:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_300;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"300000",lblKO);
			break;

		case mnuPembayaranSimpati500:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;

		case mnuPembayaranSimpati1000:
			strcpy(traName,"PEMB. SIMPATI");
			subType = SIMPATI_1000;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"1000000",lblKO);
			break;

		case mnuPembayaranMentari25:
			strcpy(traName,"PEMB. MENTARI");
			subType = MENTARI_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"25000",lblKO);
			break;

		case mnuPembayaranMentari50:
			strcpy(traName,"PEMB. MENTARI");
			subType = MENTARI_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranMentari100:
			strcpy(traName,"PEMB. MENTARI");
			subType = MENTARI_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranMentari250:
			strcpy(traName,"PEMB. MENTARI");
			subType = MENTARI_250;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"250000",lblKO);
			break;

		case mnuPembayaranMentari500:
			strcpy(traName,"PEMB. MENTARI");
			subType = MENTARI_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;

		case mnuPembayaranMentari1000:
			strcpy(traName,"PEMB. MENTARI");
			subType = MENTARI_1000;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"1000000",lblKO);
			break;

		case mnuPembayaranIm325:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"25000",lblKO);
			break;

		case mnuPembayaranIm350:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranIm375:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_75;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"75000",lblKO);
			break;

		case mnuPembayaranIm3100:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranIm3150:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;

		case mnuPembayaranIm3200:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"200000",lblKO);
			break;

		case mnuPembayaranIm3500:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;

		case mnuPembayaranIm31000:
			strcpy(traName,"PEMBELIAN IM3");
			subType = IM3_1000;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"1000000",lblKO);
			break;

		case mnuPembayaranXl25:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"20000",lblKO);
			break;

		case mnuPembayaranXl50:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranXl75:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_75;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"75000",lblKO);
			break;

		case mnuPembayaranXl100:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranXl150:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;

		case mnuPembayaranXl200:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"200000",lblKO);
			break;

		case mnuPembayaranXl300:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_300;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"300000",lblKO);
			break;

		case mnuPembayaranXl500:
			strcpy(traName,"PEMBELIAN XL");
			subType = XL_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;

		case mnuPembayaranEsia25:
			strcpy(traName,"PEMBELIAN ESIA");
			subType = ESIA_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"25000",lblKO);
			break;

		case mnuPembayaranEsia50:
			strcpy(traName,"PEMBELIAN ESIA");
			subType = ESIA_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranEsia75:
			strcpy(traName,"PEMBELIAN ESIA");
			subType = ESIA_75;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"75000",lblKO);
			break;

		case mnuPembayaranEsia100:
			strcpy(traName,"PEMBELIAN ESIA");
			subType = ESIA_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranEsia150:
			strcpy(traName,"PEMBELIAN ESIA");
			subType = ESIA_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;

		case mnuPembayaranEsia200:
			strcpy(traName,"PEMBELIAN ESIA");
			subType = ESIA_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"250000",lblKO);
			break;

		case mnuPembayaranSmart20:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_20;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"20000",lblKO);
			break;
		case mnuPembayaranSmart25:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"25000",lblKO);
			break;
		case mnuPembayaranSmart50:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;
		case mnuPembayaranSmart100:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;
		case mnuPembayaranSmart150:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;
		case mnuPembayaranSmart200:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"200000",lblKO);
			break;
		case mnuPembayaranSmart300:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_300;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"300000",lblKO);
			break;
		case mnuPembayaranSmart500:
			strcpy(traName,"PEMBELIAN SMART");
			subType = SMART_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;



		case mnuPembayaranFren20:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_20;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"20000",lblKO);
			break;

		case mnuPembayaranFren25:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"25000",lblKO);
			break;

		case mnuPembayaranFren50:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranFren100:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranFren150:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;

		case mnuPembayaranFren200:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"200000",lblKO);
			break;

		case mnuPembayaranFren300:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_300;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"300000",lblKO);
			break;

		case mnuPembayaranFren500:
			strcpy(traName,"PEMBELIAN FREN");
			subType = FREN_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;

		case mnuPembayaranThree20:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_20;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"20000",lblKO);
			break;

		case mnuPembayaranThree50:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranThree75:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_75;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"75000",lblKO);
			break;

		case mnuPembayaranThree100:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranThree150:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;

		case mnuPembayaranThree300:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_300;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"300000",lblKO);
			break;

		case mnuPembayaranThree500:
			strcpy(traName,"PEMBELIAN THREE");
			subType = THREE_500;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"500000",lblKO);
			break;

		case mnuPembayaranAxis25:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_25;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"25000",lblKO);
			break;

		case mnuPembayaranAxis50:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_50;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"50000",lblKO);
			break;

		case mnuPembayaranAxis75:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_75;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"75000",lblKO);
			break;

		case mnuPembayaranAxis100:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_100;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"100000",lblKO);
			break;

		case mnuPembayaranAxis150:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_150;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"150000",lblKO);
			break;

		case mnuPembayaranAxis200:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_200;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"200000",lblKO);
			break;

		case mnuPembayaranAxis300:
			strcpy(traName,"PEMBELIAN AXIS");
			subType = AXIS_300;
			txnType = trtPembayaranPulsa;
			MAPPUTSTR(traAmt,"300000",lblKO);
			break;
#ifdef SSB
		case mnuSIMBaruA :
			strcpy(traName,"SIM A BARU");
			isInquiry = 1;
			subType = SIMABARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruAU :
			strcpy(traName,"SIM A UMUM BARU");
			isInquiry = 1;
			subType = SIMAUBARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruB1 :
			strcpy(traName,"SIM B1 BARU");
			isInquiry = 1;
			subType = SIMB1BARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruB1U :
			strcpy(traName,"SIM B1 UMUM BARU");
			isInquiry = 1;
			subType = SIMB1UBARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruB2 :
			strcpy(traName,"SIM B2 BARU");
			isInquiry = 1;
			subType = SIMB2BARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruB2U :
			strcpy(traName,"SIM B2 UMUM BARU");
			isInquiry = 1;
			subType = SIMB2UBARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruC :
			strcpy(traName,"SIM C BARU");
			isInquiry = 1;
			subType = SIMCBARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMBaruD :
			strcpy(traName,"SIM D BARU");
			isInquiry = 1;
			subType = SIMDBARU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangA :
			strcpy(traName,"SIM A PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPA;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangAU :
			strcpy(traName,"SIM A UMUM PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPAU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB1 :
			strcpy(traName,"SIM B1 PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPB1;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB1U :
			strcpy(traName,"SIM B1 UMUM PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPB1U;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB2 :
			strcpy(traName,"SIM B2 PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPB2;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB2U :
			strcpy(traName,"SIM B2 UMUM PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPB2U;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangC :
			strcpy(traName,"SIM C PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPC;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPanjangD :
			strcpy(traName,"SIM C PERPANJANGAN");
			isInquiry = 1;
			subType = SIMPD;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanAU :
			strcpy(traName,"SIM A UMUM PENINGKATAN");
			isInquiry = 1;
			subType = SIMPNAU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB1 :
			strcpy(traName,"SIM B1 PENINGKATAN");
			isInquiry = 1;
			subType = SIMPNB1;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB1U :
			strcpy(traName,"SIM B1 UMUM PENINGKATAN");
			isInquiry = 1;
			subType = SIMPNB1U;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB2 :
			strcpy(traName,"SIM B2 PENINGKATAN");
			isInquiry = 1;
			subType = SIMPNB2;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB2U :
			strcpy(traName,"SIM B2 UMUM PENINGKATAN");
			isInquiry = 1;
			subType = SIMPNB2U;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanA :
			strcpy(traName,"SIM A PENURUNAN");
			isInquiry = 1;
			subType = SIMPUA;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanAU :
			strcpy(traName,"SIM A UMUM PENURUNAN");
			isInquiry = 1;
			subType = SIMPUAU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB1 :
			strcpy(traName,"SIM B1 PENURUNAN");
			isInquiry = 1;
			subType = SIMPUB1;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB1U :
			strcpy(traName,"SIM B1 UMUM PENURUNAN");
			isInquiry = 1;
			subType = SIMPUB1U;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB2 :
			strcpy(traName,"SIM B2 PENURUNAN");
			isInquiry = 1;
			subType = SIMPUB2;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB2U :
			strcpy(traName,"SIM B2 UMUM PENURUNAN");
			isInquiry = 1;
			subType = SIMPUB2U;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamA :
			strcpy(traName,"SIM A MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMAGOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamAU :
			strcpy(traName,"SIM A UMUM MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMAUGOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB1 :
			strcpy(traName,"SIM B1 MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMB1GOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB1U :
			strcpy(traName,"SIM B1 UMUM MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMB1UGOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB2 :
			strcpy(traName,"SIM B2 MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMB2GOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB2U :
			strcpy(traName,"SIM B2 UMUM MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMB2UGOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamC :
			strcpy(traName,"SIM C MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMCGOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMGolSamD :
			strcpy(traName,"SIM D MUTASI GOLONGAN SAMA");
			isInquiry = 1;
			subType = SIMDGOLSAM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanAU :
			strcpy(traName,"SIM A UMUM MUTASI PENINGKATAN");
			isInquiry = 1;
			subType = SIMAUMP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB1 :
			strcpy(traName,"SIM B1 MUTASI PENINGKATAN");
			isInquiry = 1;
			subType = SIMB1MP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB1U :
			strcpy(traName,"SIM B1 UMUM MUTASI PENINGKATAN");
			isInquiry = 1;
			subType = SIMB1UMP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB2 :
			strcpy(traName,"SIM B2 MUTASI PENINGKATAN");
			isInquiry = 1;
			subType = SIMB2MP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB2U :
			strcpy(traName,"SIM B2 UMUM MUTASI PENINGKATAN");
			isInquiry = 1;
			subType = SIMB2UMP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanA :
			strcpy(traName,"SIM A MUTASI PENURUNAN");
			isInquiry = 1;
			subType = SIMAMPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanAU :
			strcpy(traName,"SIM A UMUM MUTASI PENURUNAN");
			isInquiry = 1;
			subType = SIMAUMPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB1 :
			strcpy(traName,"SIM B1 MUTASI PENURUNAN");
			isInquiry = 1;
			subType = SIMB1MPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB1U :
			strcpy(traName,"SIM B1 UMUM MUTASI PENURUNAN");
			isInquiry = 1;
			subType = SIMB1UMPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB2 :
			strcpy(traName,"SIM B2 MUTASI PENURUNAN");
			isInquiry = 1;
			subType = SIMB2MPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB2U :
			strcpy(traName,"SIM B2 UMUM MUTASI PENURUNAN");
			isInquiry = 1;
			subType = SIMB2UMPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangA :
			strcpy(traName,"SIM A HILANG");
			isInquiry = 1;
			subType = SIMAH;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangAU :
			strcpy(traName,"SIM A UMUM HILANG");
			isInquiry = 1;
			subType = SIMAUH;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangB1 :
			strcpy(traName,"SIM B1 HILANG");
			isInquiry = 1;
			subType = SIMB1H;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangB1U :
			strcpy(traName,"SIM B1 UMUM HILANG");
			isInquiry = 1;
			subType = SIMB1UH;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangB2 :
			strcpy(traName,"SIM B2 HILANG");
			isInquiry = 1;
			subType = SIMB2H;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangB2U :
		strcpy(traName,"SIM B2 UMUM HILANG");
			isInquiry = 1;
			subType = SIMB2UH;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangC :
			strcpy(traName,"SIM C HILANG");
			isInquiry = 1;
			subType = SIMCH;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangD :
			strcpy(traName,"SIM D HILANG");
			isInquiry = 1;
			subType = SIMDH;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanAU :
			strcpy(traName,"SIM A UMUM HILANG PENINGKATAN");
			isInquiry = 1;
			subType = SIMAUHP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB1 :
			strcpy(traName,"SIM B1 HILANG PENINGKATAN");
			isInquiry = 1;
			subType = SIMB1HP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB1U :
			strcpy(traName,"SIM B1 UMUM HILANG PENINGKATAN");
			isInquiry = 1;
			subType = SIMB1UHP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB2 :
			strcpy(traName,"SIM B2 HILANG PENINGKATAN");
			isInquiry = 1;
			subType = SIMB2HP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB2U :
			strcpy(traName,"SIM B2 UMUM HILANG PENINGKATAN");
			isInquiry = 1;
			subType = SIMB2UHP;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanA :
			strcpy(traName,"SIM A HILANG PENURUNAN");
			isInquiry = 1;
			subType = SIMAHPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanAU :
			strcpy(traName,"SIM A UMUM HILANG PENURUNAN");
			isInquiry = 1;
			subType = SIMAUHPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanB1 :
			strcpy(traName,"SIM B1 HILANG PENURUNAN");
			isInquiry = 1;
			subType = SIMB1HPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanB1U :
			strcpy(traName,"SIM B1 UMUM HILANG PENURUNAN");
			isInquiry = 1;
			subType = SIMB1UHPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanB2 :
			strcpy(traName,"SIM B2 HILANG PENURUNAN");
			isInquiry = 1;
			subType = SIMB2HPU;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuUjiSim :
			strcpy(traName,"UJI SIMULATOR");
			isInquiry = 1;
			subType = UJISIM;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuBPKBBaruRoda2 :
			strcpy(traName,"BPKB RODA 2 BARU");
			isInquiry = 1;
			subType = BPKBR2B;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuBPKBBaruRoda4 :
			strcpy(traName,"BPKB RODA 4 BARU");
			isInquiry = 1;
			subType = BPKBR4B;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuBPKBGantiRoda2 :
			strcpy(traName,"BPKB RODA 2 GANTI PEMILIK");
			isInquiry = 1;
			subType = BPKBR2G;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuBPKBGantiRoda4 :
			strcpy(traName,"BPKB RODA 4 GANTI PEMILIK");
			isInquiry = 1;
			subType = BPKBR4G;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuSTCK :
			strcpy(traName,"STCK");
			isInquiry = 1;
			subType = STCK;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuMutasiRanRoda2 :
			strcpy(traName,"MUTASI RANMOR RODA 2");
			isInquiry = 1;
			subType = MRR2;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;

		case mnuMutasiRanRoda4 :
			strcpy(traName,"MUTASI RANMOR RODA 4");
			isInquiry = 1;
			subType = MRR4;
			txnType = trtPembayaranSSBInquiry;
			nextTxn = trtPembayaranSSB;
			break;
#endif


		case mnuSetorPasti:
			txnType = trtSetorPasti;
			strcpy(traName,"SETOR PASTI");
			break;


		case mnuPembayaranZakatDhuafa:
			strcpy(traName,"PEMB. ZAKAT");
			subType = ZAKAT_DHUAFA;
			txnType = trtPembayaranZakat;
			break;

		case mnuPembayaranZakatYBM:
			strcpy(traName,"PEMB. ZAKAT");
			subType = ZAKAT_YBM;
			txnType = trtPembayaranZakat;
			break;

		case mnuPembayaranInfaqDhuafa:
			strcpy(traName,"PEMB. INFAQ");
			subType = INFAQ_DHUAFA;
			txnType = trtPembayaranZakat;
			break;

		case mnuPembayaranInfaqYBM:
			strcpy(traName,"PEMB. INFAQ");
			subType = INFAQ_YBM;
			txnType = trtPembayaranZakat;
			break;

		case mnuPembayaranDPLKDhuafa:
			strcpy(traName,"PEMB. DPLK");
			subType = DPLK_DHUAFA;
			txnType = trtPembayaranZakat;
			break;

		case mnuPembayaranDPLKYBM:
			strcpy(traName,"PEMB. DPLK");
			subType = DPLK_YBM;
			txnType = trtPembayaranZakat;
			break;

		case mnuSetorSimpanan:
			strcpy(traName,"SETOR SIMPANAN");
			isInquiry = 1;
			txnType = trtSetorSimpananInquiry;
			nextTxn = trtSetorSimpanan;
			break;

		case mnuSetorPinjaman:
			strcpy(traName,"SETOR PINJAMAN");
			isInquiry = 1;
			txnType = trtSetorPinjamanInquiry;
			nextTxn = trtSetorPinjaman;
			break;

		case mnuTarikTunai:
			strcpy(traName,"TARIK TUNAI");
			txnType = trtTarikTunai;
			break;

		case mnuVoidTarikTunai:
			strcpy(traName,"VOID TRK TUNAI");
			txnType = trtVoidTarikTunai;
			break;

		case mnuRegistrasiInternetBanking:
			strcpy(traName,"REG INT BANKING");
			txnType = trtRegistrasiInternetBanking;
			break;

#ifdef T_BANK
		case mnuCashIn:
			strcpy(traName,"CASH IN");
			txnType = trtTCash;
			break;

		case mnuCashOut:
			strcpy(traName,"CASH OUT");
			txnType = trtTCashOut;
			break;
#endif
		case mnuRegistrasiPhoneBanking:
			strcpy(traName,"REG PHN BANKING");
			txnType = trtRegistrasiPhoneBanking;
			break;

		case mnuPIN:
		case mnuGantiPIN:
			strcpy(traName,"UBAH PIN");
			txnType = trtGantiPIN;
			break;

		case mnuRenewalKartu:
			strcpy(traName,"RENEWAL KARTU");
			txnType = trtRenewalKartuOtorisasi;
			break;

		case mnuAktivasiKartu:
			strcpy(traName,"AKTIVASI KARTU");
			txnType = trtAktivasiKartuOtorisasi;
			break;

		case mnuReissuePIN:
			strcpy(traName,"REISSUE PIN");
			txnType = trtReissuePINOtorisasi;
			break;

		case mnuGantiPasswordSPV:
			strcpy(traName,"UBAH PASS SPV");
			txnType = trtGantiPasswordSPV;
			break;

		case mnuInstallment:
			strcpy(traName,"CICILAN BRING");
			MAPPUTBYTE(traEmvFullFlag, 1, lblKO);
			txnType = trtInstallment;
			break;

		case mnuPembayaranHalo:
			strcpy(traName,"HALO");
			isInquiry = 1;
			subType = PASCA_BAYAR_HALO;
			txnType = trtPembayaranPascaBayarInquiry;
			nextTxn = trtPembayaranPascaBayar;
			break;

		case mnuPembayaranMatrix:
			strcpy(traName,"MATRIX");
			isInquiry = 1;
			subType = PASCA_BAYAR_MATRIX;
			txnType = trtPembayaranPascaBayarInquiry;
			nextTxn = trtPembayaranPascaBayar;
			break;
	}

	while(1)
	{
		txnAsli = 0;
		mapPut(traRspCod, "", 2);
		MAPPUTBYTE(regLocType,'T',lblKO);
		MAPPUTBYTE(traSubType,subType,lblKO);
		MAPPUTBYTE(traInquiryFlag,isInquiry,lblKO);
		MAPPUTBYTE(traNextTxn,nextTxn,lblKO);
		mapMove(rqsBeg, (word) (txnType - 1));
		trcFS("traName: %s\n", traName);

		MAPPUTSTR(traTransType, traName, lblKO);
		MAPPUTSTR(traTypeName, traName, lblKO);
		MAPPUTBYTE(traTxnType, txnType, lblKO);
		if( pmtPayment(key) != 1)
			goto lblEnd;

		nextTxn = 0;
		isInquiry = 0;
		switch(txnType)
		{
			case trtRenewalKartuOtorisasi:
				strcpy(traName,"RENEWAL KARTU");
				txnType = trtRenewalKartuInquiry;
				break;

			case trtRenewalKartuInquiry:
				strcpy(traName,"RENEWAL KARTU");
				txnType = trtRenewalKartu;
				break;

			case trtAktivasiKartuOtorisasi:
				strcpy(traName,"AKTIVASI");
				isInquiry = 1;
				txnType = trtAktivasiKartuInquiry;
				nextTxn = trtAktivasiKartu;
				break;

			case trtReissuePINOtorisasi:
				strcpy(traName,"REISSUE PIN");
				txnType = trtReissuePIN;
				break;

			default:
				goto lblEnd;
		}
	}

	goto lblEnd;
	lblKO:
	lblEnd:
		trcS("pmtBRI End\n");
}


int disableSubmenu()
{
	int 		ret;
	char 	buf[20];
	word 	mnuItem;
	int           i = 0, x = 0;
	byte 	tempBuf[25];

	dspClear();
	dspLS(0|0x40,"DISABLE SUBMENU");

	MAPGETWORD(traMnuItm, mnuItem, lblKO);
	switch(mnuItem)
	{
		case mnuAktivasiKartu:
			dspLS(1,"AKTIVASI KARTU");
			break;

		case mnuRenewalKartu:
			dspLS(1,"RENEWAL KARTU");
			break;

		case mnuReissuePIN:
			dspLS(1,"REISSUE PIN");
			break;

		case mnuPIN:
		case mnuGantiPIN:
			dspLS(1,"GANTI PIN");
			break;

		case mnuGantiPasswordSPV:
			dspLS(1,"GANTI PASS SPV");
			break;

		case mnuCReprint:
			dspLS(1,"REPRINT");
			break;

		case mnuCLastTrx:
			dspLS(1,"LAST TRX");
			break;

		case mnuCAnyTrx:
			dspLS(1,"ANY KARTU");
			break;

		case mnuCReport:
			dspLS(1,"REPORT");
			break;

		case mnuCTodaySummary:
			dspLS(1,"TODAY SUMMARY");
			break;

		case mnuCTodayDetail:
			dspLS(1,"TODAY DETAIL");
			break;

		case mnuCPastSummary:
			dspLS(1,"PAST SUMMARY");
			break;

		case mnuCPastDetail:
			dspLS(1,"PAST DETAIL");
			break;

		case mnuSetorSimpanan:
			dspLS(1,"SETOR SIMPANAN");
			break;

		case mnuSetorPinjaman:
			dspLS(1,"SETOR PINJAMAN");
			break;

		case mnuSetorPasti:
			dspLS(1,"SETOR PASTI");
			break;



		case mnuTarikTunai:
			dspLS(1,"TARIK TUNAI");
			break;

		case mnuVoidTarikTunai:
			dspLS(1,"VOID TARIK TUNAI");
			break;

		case mnuTReprint:
			dspLS(1,"REPRINT");
			break;

		case mnuTLastTrx:
			dspLS(1,"LAST TRX");
			break;

		case mnuTReport:
			dspLS(1,"REPORT");
			break;

		case mnuTAnyTrx:
			dspLS(1,"ANY TRX");
			break;

		case mnuTTodaySummary:
			dspLS(1,"TODAY SUMMARY");
			break;

		case mnuTTodayDetail:
			dspLS(1,"TODAY DETAIL");
			break;

		case mnuTPastSummary:
			dspLS(1,"PAST SUMMARY");
			break;

		case mnuTPastDetail:
			dspLS(1,"PAST DETAIL");
			break;

		case mnuInfoSaldo:
			dspLS(1,"INFO SALDO");
			break;

		case mnuInfoSaldoBankLain:
			dspLS(1,"SALDO BANK LAIN");
			break;

		case mnuMiniStatement:
			dspLS(1,"MINI STATEMENT");
			break;
		case mnuMutRek:
			dspLS(1,"MUTASI REKENING");
			break;
		case mnuPrevilege:
			dspLS(1,"PREVILEGE");
			break;
		case mnuTransferSesamaBRI:
			dspLS(1,"TRANS SESAMA BRI");
			break;

		case mnuTransferAntarBank:
			dspLS(1,"TRANS ANTAR BANK");
			break;

		case mnuInfoKodeBank1:
			dspLS(1,"INFO KODE BANK");
			break;

		case mnuInfoKodeBank:
			dspLS(1,"INFO KODE BANK");
			break;

		case mnuPembayaranBriva:
			dspLS(1,"PEMB. BRIVA");
			break;

		case mnuPembayaranTelkom:
			dspLS(1,"PEMB. TELKOM");
			break;

		case mnuPembayaranDPLKR:
			dspLS(1,"PEMB. DPLKR");
			break;

		case mnuPembayaranHalo:
			dspLS(1,"PEMB. KARTU HALO");
			break;

		case mnuPembayaranMatrix:
			dspLS(1,"PEMB KARTU MATRIX");
			break;

		case mnuPembayaranPLNPasca:
			dspLS(1,"PEMB. PLN PASCA");
			break;

		case mnuPembayaranPLNPra:
			dspLS(1,"PEMB. PLN PRA");
			break;

		case mnuPembayaranPLNToken:
			dspLS(1,"CETAK TOKEN");
			break;

		case mnuPembayaranCicilanFIF:
			dspLS(1,"PEMB. CICILAN FIF");
			break;

		case mnuPembayaranCicilanBAF:
			dspLS(1,"PEMB. CICILAN BAF");
			break;

		case mnuPembayaranCicilanOTO:
			dspLS(1,"PEMB. CICILAN OTO");
			break;

		case mnuPembayaranCicilanFinansia:
			dspLS(1,"PEMB. CICILAN FIN");
			break;

		case mnuPembayaranCicilanVerena:
			dspLS(1,"PEMB. CICILAN VER");
			break;

		case mnuPembayaranCicilanWOM:
			dspLS(1,"PEMB. CICILAN WOM");
			break;

		case mnuPembayaranTvInd:
			dspLS(1,"PEMB. INDOVISION");
			break;

		case mnuPembayaranTvOkTv:
			dspLS(1,"PEMB. OK TV");
			break;

		case mnuPembayaranTvTopTv:
			dspLS(1,"PEMB. TOP TV");
			break;

		case mnuPembayaranTiketGaruda:
			dspLS(1,"PEMB TIKET GARUDA");
			break;

		case mnuPembayaranTiketLionAir:
			dspLS(1,"PEMB TIKET LION");
			break;

		case mnuPembayaranTiketSriwijaya:
			dspLS(1,"PEMB TIKET SRIWI");
			break;

		case mnuPembayaranTiketMandala:
			dspLS(1,"PEMB TIKET MANDALA");
			break;

		case mnuPembayaranPdamSby:
			dspLS(1,"PEMB PDAM SBY");
			break;

		case mnuRegistrasiTrxTransfer:
			dspLS(1,"REG. TRX TRANSFER");
			break;

		case mnuRegistrasiTrxPulsa:
			dspLS(1,"REG. TRX PULSA");
			break;

		case mnuPembayaranSPP:
			dspLS(1,"PEMB SPP");
			break;

		case mnuPembayaranKodeUniv:
			dspLS(1,"KODE UNIVERSITAS");
			break;

		case mnuPembayaranSimpati50:
			dspLS(1,"PEMB. SIMPATI 50");
			break;

		case mnuPembayaranSimpati100:
			dspLS(1,"PEMB. SIMPATI 50");
			break;

//		case mnuPembayaranSimpati150:
//			dspLS(1,"PEMB. SIMPATI 150");
//			break;

		case mnuPembayaranSimpati200:
			dspLS(1,"PEMB. SIMPATI 200");
			break;

		case mnuPembayaranSimpati300:
			dspLS(1,"PEMB. SIMPATI 300");
			break;

		case mnuPembayaranSimpati500:
			dspLS(1,"PEMB. SIMPATI 500");
			break;

		case mnuPembayaranSimpati1000:
			dspLS(1,"PEMB. SIMPATI 1000");
			break;

		case mnuPembayaranMentari25:
			dspLS(1,"PEMB. MENTARI 25");
			break;

		case mnuPembayaranMentari50:
			dspLS(1,"PEMB. MENTARI 50");
			break;

		case mnuPembayaranMentari100:
			dspLS(1,"PEMB. MENTARI 100");
			break;

		case mnuPembayaranMentari250:
			dspLS(1,"PEMB. MENTARI 250");
			break;

		case mnuPembayaranMentari500:
			dspLS(1,"PEMB. MENTARI 500");
			break;

		case mnuPembayaranMentari1000:
			dspLS(1,"PEMB. MENTARI 1000");
			break;

		case mnuPembayaranIm325:
			dspLS(1,"PEMB. IM3 25");
			break;

		case mnuPembayaranIm350:
			dspLS(1,"PEMB. IM3 50");
			break;

		case mnuPembayaranIm375:
			dspLS(1,"PEMB. IM3 75");
			break;

		case mnuPembayaranIm3100:
			dspLS(1,"PEMB. IM3 100");
			break;

		case mnuPembayaranIm3150:
			dspLS(1,"PEMB. IM3 150");
			break;

		case mnuPembayaranIm3200:
			dspLS(1,"PEMB. IM3 200");
			break;

		case mnuPembayaranIm3500:
			dspLS(1,"PEMB. IM3 500");
			break;

		case mnuPembayaranIm31000:
			dspLS(1,"PEMB. IM3 1000");
			break;

		case mnuPembayaranXl25:
			dspLS(1,"PEMB. XL 20");
			break;

		case mnuPembayaranXl50:
			dspLS(1,"PEMB. XL 50");
			break;

		case mnuPembayaranXl75:
			dspLS(1,"PEMB. XL 75");
			break;

		case mnuPembayaranXl100:
			dspLS(1,"PEMB. XL 100");
			break;

		case mnuPembayaranXl150:
			dspLS(1,"PEMB. XL 150");
			break;

		case mnuPembayaranXl200:
			dspLS(1,"PEMB. XL 200");
			break;

		case mnuPembayaranXl300:
			dspLS(1,"PEMB. XL 300");
			break;

		case mnuPembayaranXl500:
			dspLS(1,"PEMB. XL 500");
			break;

		case mnuPembayaranEsia25:
			dspLS(1,"PEMB. ESIA 25");
			break;

		case mnuPembayaranEsia50:
			dspLS(1,"PEMB. ESIA 50");
			break;

		case mnuPembayaranEsia75:
			dspLS(1,"PEMB. ESIA 75");
			break;

		case mnuPembayaranEsia100:
			dspLS(1,"PEMB. ESIA 100");
			break;

		case mnuPembayaranEsia150:
			dspLS(1,"PEMB. ESIA 150");
			break;

		case mnuPembayaranEsia200:
			dspLS(1,"PEMB. ESIA 200");
			break;


		case mnuPembayaranSmart20:
			dspLS(1,"PEMB. SMART 20");
			break;
		case mnuPembayaranSmart25:
			dspLS(1,"PEMB. SMART 25");
			break;
		case mnuPembayaranSmart50:
			dspLS(1,"PEMB. SMART 50");
			break;

		case mnuPembayaranSmart100:
			dspLS(1,"PEMB. SMART 100");
			break;
		case mnuPembayaranSmart150:
			dspLS(1,"PEMB. SMART 150");
			break;

		case mnuPembayaranSmart200:
			dspLS(1,"PEMB. SMART 200");
			break;
		case mnuPembayaranSmart300:
			dspLS(1,"PEMB. SMART 300");
			break;
		case mnuPembayaranSmart500:
			dspLS(1,"PEMB. SMART 500");
			break;


		case mnuPembayaranFren20:
			dspLS(1,"PEMB. FREN 20");
			break;

		case mnuPembayaranFren25:
			dspLS(1,"PEMB. FREN 25");
			break;

		case mnuPembayaranFren50:
			dspLS(1,"PEMB. FREN 50)");
			break;

		case mnuPembayaranFren100:
			dspLS(1,"PEMB. FREN 100");
			break;

		case mnuPembayaranFren150:
			dspLS(1,"PEMB. FREN 150");
			break;

		case mnuPembayaranFren200:
			dspLS(1,"PEMB. FREN 200");
			break;

		case mnuPembayaranFren300:
			dspLS(1,"PEMB. FREN 300");
			break;

		case mnuPembayaranFren500:
			dspLS(1,"PEMB. FREN 500");
			break;

		case mnuPembayaranThree20:
			dspLS(1,"PEMB. THREE 20");
			break;

		case mnuPembayaranThree50:
			dspLS(1,"PEMB. THREE 50");
			break;

		case mnuPembayaranThree75:
			dspLS(1,"PEMB. THREE 75");
			break;

		case mnuPembayaranThree100:
			dspLS(1,"PEMB. THREE 100");
			break;

		case mnuPembayaranThree150:
			dspLS(1,"PEMB. THREE 150");
			break;

		case mnuPembayaranThree300:
			dspLS(1,"PEMB. THREE 300");
			break;

		case mnuPembayaranThree500:
			dspLS(1,"PEMB. THREE 500");
			break;

		case mnuPembayaranAxis25:
			dspLS(1,"PEMB. AXIS 25");
			break;

		case mnuPembayaranAxis50:
			dspLS(1,"PEMB. AXIS 50");
			break;

		case mnuPembayaranAxis75:
			dspLS(1,"PEMB. AXIS 75");
			break;

		case mnuPembayaranAxis100:
			dspLS(1,"PEMB. AXIS 100");
			break;

		case mnuPembayaranAxis150:
			dspLS(1,"PEMB. AXIS 150");
			break;

		case mnuPembayaranAxis200:
			dspLS(1,"PEMB. AXIS 200");
			break;

		case mnuPembayaranAxis300:
			dspLS(1,"PEMB. AXIS 300");
			break;

		case mnuPembayaranKKBRI:
			dspLS(1,"PEMB. KK BRI");
			break;

		case mnuPembayaranKKCITIKTA:
			dspLS(1,"PEMB. KTA CITIBANK");
			break;

		case mnuPembayaranKKCITIKK:
			dspLS(1,"PEMB. KK CITIBANK");
			break;

		case mnuPembayaranKKStanCharKK:
			dspLS(1,"PEMB. KK STANCHAR");
			break;

		case mnuPembayaranKKStanCharKTA:
			dspLS(1,"PEMB. KTA STANCHAR");
			break;

		case mnuPembayaranKKANZ:
			dspLS(1,"PEMB. KK ANZ");
			break;

		case mnuPembayaranKKHSBCKTA:
			dspLS(1,"PEMB. KTA HSBC");
			break;

		case mnuPembayaranKKHSBCKK:
			dspLS(1,"PEMB. KK HSBC");
			break;

		case mnuPembayaranKKRBSKK:
			dspLS(1,"PEMB. KK RBS");
			break;

		case mnuPembayaranKKRBSKTA:
			dspLS(1,"PEMB. KTA RBS");
			break;

		case mnuPembayaranZakatDhuafa:
			dspLS(1,"PEMB. ZAKAT DHUAFA");
			break;

		case mnuPembayaranZakatYBM:
			dspLS(1,"PEMB. ZAKAT YBM");
			break;

		case mnuPembayaranInfaqDhuafa:
			dspLS(1,"PEMB. INFAQ DHUAFA");
			break;

		case mnuPembayaranInfaqYBM:
			dspLS(1,"PEMB. INFAQ YBM");
			break;

		case mnuPembayaranDPLKDhuafa:
			dspLS(1,"PEMB. DPLK DHUAFA");
			break;

		case mnuPembayaranDPLKYBM:
			dspLS(1,"PEMB. INFAQ YBM");
			break;

#ifdef T_BANK
		case mnuCashIn:
			dspLS(1,"CASH IN");
			break;

		case mnuCashOut:
			dspLS(1,"CASH OUT");
			break;
#endif
		case mnuRegistrasiInternetBanking:
			dspLS(1,"REG. INTERNET BANK");
			break;

		case mnuRegistrasiPhoneBanking:
			dspLS(1,"REG. PHONE BANK");
			break;

		case mnuRegistrasiSmsBanking:
			dspLS(1,"REG. SMS BANK");
			break;

		case mnuMLastTrx:
			dspLS(1,"M LAST TRX");
			break;

		case mnuMAnyTrx:
			dspLS(1,"M ANY TRX");
			break;

		case mnuMVLastTrx:
			dspLS(1,"MV LAST TRX");
			break;

		case mnuMVAnyTrx:
			dspLS(1,"MV ANY TRX");
			break;

		case mnuMTodaySummary:
			dspLS(1,"M PAST SUMMARY");
			break;

		case mnuMTodayDetail:
			dspLS(1,"M TODAY DETAIL");
			break;

		case mnuMPastSummary:
			dspLS(1,"M PAST SUMMARY");
			break;

		case mnuMPastDetail:
			dspLS(1,"M PAST DETAIL");
			break;

		case mnuSale:
			dspLS(1,"SALE");
			break;

		case mnuSaleRedeem:
			dspLS(1,"SALE REDEEM");
			break;

		case mnuInstallment:
			dspLS(1,"INSTALLMENT");
			break;

		case mnuRefund:
			dspLS(1,"REFUND");
			break;

		case mnuPreaut:
			dspLS(1,"PREAUTH");
			break;

		case mnuAut:
			dspLS(1,"AUTH");
			break;

		case mnuVoid:
			dspLS(1,"VOID");
			break;

		case mnuOffline:
			dspLS(1,"OFFLINE");
			break;

		case mnuSettlement:
			dspLS(1,"SETTLEMENT");
			break;

		case mnuMrcChgPwd:
			dspLS(1,"MERCH CHANGE PASS");
			break;

		case mnuMrcReset:
			dspLS(1,"MERCH RESET");
			break;

		case mnuFn99MrcReset:
			dspLS(1,"99 MERCH RESET");
			break;

		case mnuMrcResetRev:
			dspLS(1,"MERCH RESET REV");
			break;

		case mnuFn99MrcResetRev:
			dspLS(1,"99 RESET");
			break;

		case mnuDeleteCLog:
			dspLS(1,"VIEW C TUNAI");
			break;

		case mnuDeleteMLog:
			dspLS(1,"VIEW M TUNAI");
			break;

		case mnuDeleteTLog:
			dspLS(1,"DELETE T LOG");
			break;

		case mnuViewLog:
			dspLS(1,"VIEW LOG");
			break;

		case mnuViewCardService:
			dspLS(1,"VIEW CARD SERV");
			break;

		case mnuViewMiniAtm:
			dspLS(1,"VIEW MINI ATM");
			break;

		case mnuViewTunai:
			dspLS(1,"VIEW TUNAI");
			break;

		case mnuPrepaidReprint:
			dspLS(1,"PREPAID REPRINT");
			break;

		case mnuPrepaidReport:
			dspLS(1,"PREPAID REPORT");
			break;

		case mnuPDateDetail:
			dspLS(1,"DATE DETAIL");
			break;

		case mnuPAllDetail:
			dspLS(1,"ALL DETAIL");
			break;

		case mnuInitialize:
			dspLS(1,"INITIALIZE");
			break;

		case mnuSaldo:
			dspLS(1,"MIN SALDO");
			break;

		case mnuPrepaidInfoSaldo:
			dspLS(1,"INFO SALDO");
			break;

		case mnuInfoDeposit:
			dspLS(1,"INFO DEPO");
			break;

		case mnuPrepaidPembayaran:
			dspLS(1,"PEMBAYARAN");
			break;

		case mnuPrepaidPayment:
			dspLS(1,"PREPAID PAYM");
			break;

		case mnuTopUpOnline:
			dspLS(1,"TOP UP  ONLINE");
			break;

		case mnuTopUpDeposit:
			dspLS(1,"TOP UP  DEPO");
			break;

		case mnuPrepaidRedeem:
			dspLS(1,"PREPAID  REDEEM");
			break;

		case mnuPrepaidPaymentDisc:
			dspLS(1,"PREPAID  PAYM DISC");
			break;

		case mnuPrepaidReaktivasi:
			dspLS(1,"PREPAID  REAKTIV");
			break;

		case mnuPrepaidVoid:
			dspLS(1,"PREPAID  VOID");
			break;

		case mnuAktivasiDeposit:
			dspLS(1,"AKTIVASI DEPOSIT");
			break;

		case mnuPrepaidInfoKartu:
			dspLS(1,"PREPAID  INFO KARTU");
			break;

		case mnuPrepaidPrintLog:
			dspLS(1,"PREPAID  PRINT LOG");
			break;

		case mnuPrepaidSettlement:
			dspLS(1,"PREPAID  SETT");
			break;

		case mnuPRLastTrx:
			dspLS(1,"LAST TRX");
			break;

		case mnuPRAnyTrx:
			dspLS(1,"ANY TRX");
			break;

		case mnuPDateSummary:
			dspLS(1,"DATE SUMMARY");
			break;

		case mnuPAllSummary:
			dspLS(1,"ALL SUMMARY");
			break;

		case mnuPDateDetailAll:
			dspLS(1,"DATE DETAIL");
			break;

		case mnuPAllDetailAll:
			dspLS(1,"ALL DETAIL");
			break;

		case mnuPDateDetailPayment:
			dspLS(1,"DATE DET. PAYM");
			break;

		case mnuPAllDetailPayment:
			dspLS(1,"ALL DET. PAYM");
			break;

		case mnuDatang:
			dspLS(1,"DATANG");
			break;

		case mnuPulangs:
			dspLS(1,"PULANG");
			break;

		case mnuIstirahatA:
			dspLS(1,"ISTIRAHAT B");
			break;

		case mnuIstirahatB:
			dspLS(1,"ISTIRAHAT B");
			break;

		case mnuLemburA:
			dspLS(1,"LEMBUR A");
			break;

		case mnuLemburB:
			dspLS(1,"LEMBUR B");
			break;

		case mnuShiftsIA:
			dspLS(1,"SHIFT I A");
			break;

		case mnuShiftsIB:
			dspLS(1,"SHIFT I B");
			break;

		case mnuShiftsIIA:
			dspLS(1,"SHIFT II A");
			break;

		case mnuShiftsIIB:
			dspLS(1,"SHIFT II B");
			break;

		case mnuShiftsIIIA:
			dspLS(1,"SHIFT III A");
			break;

		case mnuShiftsIIIB:
			dspLS(1,"SHIFT III B");
			break;

		case mnuSIMBaruA :
			dspLS(1,"SIM BARU A");
			break;

		case mnuSIMBaruAU :
			dspLS(1,"SIM BARU AU");
			break;

		case mnuSIMBaruB1 :
			dspLS(1,"SIM BARU B1");
			break;

		case mnuSIMBaruB1U :
			dspLS(1,"SIM BARU B1U");
			break;

		case mnuSIMBaruB2 :
			dspLS(1,"SIM BARU B2");
			break;

		case mnuSIMBaruB2U :
			dspLS(1,"SIM BARU B2U");
			break;

		case mnuSIMBaruC :
			dspLS(1,"SIM BARU C");
			break;

		case mnuSIMBaruD :
			dspLS(1,"SIM BARU D");
			break;

		case mnuSIMPanjangA :
			dspLS(1,"SIM PANJ A");
			break;

		case mnuSIMPanjangAU :
			dspLS(1,"SIM PANJ AU");
			break;

		case mnuSIMPanjangB1 :
			dspLS(1,"SIM PANJ B1");
			break;

		case mnuSIMPanjangB1U :
			dspLS(1,"SIM PANJ B1U");
			break;

		case mnuSIMPanjangB2 :
			dspLS(1,"SIM PANJ B2");
			break;

		case mnuSIMPanjangB2U :
			dspLS(1,"SIM PANJ B2U");
			break;

		case mnuSIMPanjangC :
			dspLS(1,"SIM PANJ C");
			break;

		case mnuSIMPanjangD :
			dspLS(1,"SIM PANJ D");
			break;

		case mnuSIMPeningkatanAU :
			dspLS(1,"SIM PEN AU");
			break;

		case mnuSIMPeningkatanB1 :
			dspLS(1,"SIM PEN B1");
			break;

		case mnuSIMPeningkatanB1U :
			dspLS(1,"SIM PEN B1U");
			break;

		case mnuSIMPeningkatanB2 :
			dspLS(1,"SIM PEN B2U");
			break;

		case mnuSIMPeningkatanB2U :
			dspLS(1,"SIM PEN B2U");
			break;

		case mnuSIMPenurunanA :
			dspLS(1,"SIM PEN A");
			break;

		case mnuSIMPenurunanAU :
			dspLS(1,"SIM PEN AU");
			break;

		case mnuSIMPenurunanB1 :
			dspLS(1,"SIM PEN B1");
			break;

		case mnuSIMPenurunanB1U :
			dspLS(1,"SIM PEN B1U");
			break;

		case mnuSIMPenurunanB2 :
			dspLS(1,"SIM PEN B2");
			break;

		case mnuSIMPenurunanB2U :
			dspLS(1,"SIM PEN B2U");
			break;

		case mnuSIMGolSamA :
			dspLS(1,"SIM GOL SAMA A");
			break;

		case mnuSIMGolSamAU :
			dspLS(1,"SIM GOL SAMA AU");
			break;

		case mnuSIMGolSamB1 :
			dspLS(1,"SIM GOL SAMA B1");
			break;

		case mnuSIMGolSamB1U :
			dspLS(1,"SIM GOL SAMA B1U");
			break;

		case mnuSIMGolSamB2 :
			dspLS(1,"SIM GOL SAMA B2");
			break;

		case mnuSIMGolSamB2U :
			dspLS(1,"SIM GOL SAMA B2U");
			break;

		case mnuSIMGolSamC :
			dspLS(1,"SIM GOL SAMA C");
			break;

		case mnuSIMGolSamD :
			dspLS(1,"SIM GOL SAMA D");
			break;

		case mnuSIMMutasiPeningkatanAU :
			dspLS(1,"SIM MUT PEN AU");
			break;

		case mnuSIMMutasiPeningkatanB1 :
			dspLS(1,"SIM MUT PEN B1");
			break;

		case mnuSIMMutasiPeningkatanB1U :
			dspLS(1,"SIM MUT PEN B1U");
			break;

		case mnuSIMMutasiPeningkatanB2 :
			dspLS(1,"SIM MUT PEN B2");
			break;

		case mnuSIMMutasiPeningkatanB2U :
			dspLS(1,"SIM MUT PEN B2U");
			break;

		case mnuSIMMutasiPenurunanA :
			dspLS(1,"SIM MUT PEN A");
			break;

		case mnuSIMMutasiPenurunanAU :
			dspLS(1,"SIM MUT PEN AU");
			break;

		case mnuSIMMutasiPenurunanB1 :
			dspLS(1,"SIM MUT PEN B1");
			break;

		case mnuSIMMutasiPenurunanB1U :
			dspLS(1,"SIM MUT PEN B1U");
			break;

		case mnuSIMMutasiPenurunanB2 :
			dspLS(1,"SIM MUT PEN B2");
			break;

		case mnuSIMMutasiPenurunanB2U :
			dspLS(1,"SIM MUT PEN B2U");
			break;

		case mnuSIMHilangA :
			dspLS(1,"SIM HILANG A");
			break;

		case mnuSIMHilangAU :
			dspLS(1,"SIM HILANG AU");
			break;

		case mnuSIMHilangB1 :
			dspLS(1,"SIM HILANG B1");
			break;

		case mnuSIMHilangB1U :
			dspLS(1,"SIM HILANG B1U");
			break;

		case mnuSIMHilangB2 :
			dspLS(1,"SIM HILANG B2");
			break;

		case mnuSIMHilangB2U :
			dspLS(1,"SIM HILANG B2U");
			break;

		case mnuSIMHilangC :
			dspLS(1,"SIM HILANG C");
			break;

		case mnuSIMHilangD :
			dspLS(1,"SIM HILANG D");
			break;

		case mnuSIMHilangPeningkatanAU :
			dspLS(1,"SIM HILANG PEN AU");
			break;

		case mnuSIMHilangPeningkatanB1 :
			dspLS(1,"SIM HILANG PEN B1");
			break;

		case mnuSIMHilangPeningkatanB1U :
			dspLS(1,"SIM HILANG PEN B1U");
			break;

		case mnuSIMHilangPeningkatanB2 :
			dspLS(1,"SIM HILANG PEN B2");
			break;

		case mnuSIMHilangPeningkatanB2U :
			dspLS(1,"SIM HILANG PEN B2U");
			break;

		case mnuSIMHilangPenurunanA :
			dspLS(1,"SIM HILANG PEN A");
			break;

		case mnuSIMHilangPenurunanAU :
			dspLS(1,"SIM HILANG PEN AU");
			break;

		case mnuSIMHilangPenurunanB1 :
			dspLS(1,"SIM HILANG PEN B1");
			break;

		case mnuSIMHilangPenurunanB1U :
			dspLS(1,"SIM HILANG PEN B1U");
			break;

		case mnuSIMHilangPenurunanB2 :
			dspLS(1,"SIM HILANG PEN B2");
			break;

		case mnuUjiSim :
			dspLS(1,"UJI SIMULATOR");
			break;

		case mnuBPKBBaruRoda2 :
			dspLS(1,"BPKB RODA 2");
			break;

		case mnuBPKBBaruRoda4 :
			dspLS(1,"BPKB RODA 4");
			break;

		case mnuBPKBGantiRoda2 :
			dspLS(1,"BPKB RODA 2");
			break;

		case mnuBPKBGantiRoda4 :
			dspLS(1,"BPKB RODA 4");
			break;

		case mnuSTCK :
			dspLS(1,"STCK");
			break;

		case mnuMutasiRanRoda2 :
			dspLS(1,"MUTASI RODA 2");
			break;

		case mnuMutasiRanRoda4 :
			dspLS(1,"MUTASI RODA 4");
			break;

		case mnuMrcInitRpt:
			dspLS(1,"MERCH INIT REPORT");
			break;

		case mnuMrcViewTot:
			dspLS(1,"MERCH VIEW TOTAL");
			break;

		case mnuMrcSWver:
			dspLS(1,"MERCH SW VERSION");
			break;

		case mnuMrcViewRoc:
			dspLS(1,"MERCH VIEW ROC");
			break;

		case mnuMrcSetRoc:
			dspLS(1,"MERCH SET ROC");
			break;

		case mnuMrcViewBatNum:
			dspLS(1,"MERCH VIEW BATCH NUM");
			break;

		case mnuMrcSetBatNum:
			dspLS(1,"MERCH SET BATCH NUM");
			break;

		case mnuMrcViewSTAN:
			dspLS(1,"MERCH VIEW STAN");
			break;

		case mnuMrcSetSTAN:
			dspLS(1,"MERCH SET STAN");
			break;

		case mnuMrcViewBat:
			dspLS(1,"MERCH VIEW BAT");
			break;

		case mnuMrcViewTxn:
			dspLS(1,"MERCH VIEW TRX");
			break;

		case mnuMrcPrnTra:
			dspLS(1,"MERCH PRINT TRX");
			break;

		case mnuMrcReprint:
			dspLS(1,"MERCH REPRINT");
			break;

		case mnuMrcSetDup:
			dspLS(1,"MERCH SET DUP");
			break;

		case mnuMrcHostTot:
			dspLS(1,"MERCH HOST TOTAL");
			break;

		case mnuMrcDetails:
			dspLS(1,"MERCH DETAIL");
			break;

		case mnuMrcBatTot:
			dspLS(1,"BATCH TOTAL");
			break;

		case mnuTMSLP:
			dspLS(1,"TMS LP");
			break;

		case mnuTMSVC:
			dspLS(1,"TMS VC");
			break;

		case mnuTMSPC:
			dspLS(1,"TMS PC");
			break;

		case mnuTMSConfig:
			dspLS(1,"CONFIG TMS");
			break;

		case mnuTDT:
			dspLS(1,"TDT");
			break;

		case mnuTermAppli:
			dspLS(1,"TERM APP");
			break;

		case mnuTMKey:
			dspLS(1,"TM KEY");
			break;

		case mnuFn99TMKey:
			dspLS(1,"FUNC 99 KEY");
			break;

		case mnuEDCFunction:
			dspLS(1,"EDC FUNCT");
			break;

		case mnuCmmVS:
			dspLS(1,"CMM VS");
			break;

		case mnuCmmPS:
			dspLS(1,"CMM PS");
			break;

		case mnuCmmLS:
			dspLS(1,"CMM LS");
			break;

		case mnuCmmGS:
			dspLS(1,"CMM GS");
			break;

		case mnuCmmCS:
			dspLS(1,"CMM CS");
			break;

		case mnuBalance:
			dspLS(1,"BALANCE");
			break;

		case mnuLogon:
			dspLS(1,"LOGON");
			break;

	}

	dspLS(2,"F1-ENA   F4-DIS");

	while(1)
	{
		memset(buf,0,sizeof(buf));
		ret = enterStr(3, buf, 2);
		if((ret == kbdF1) || (ret == kbdF4))
			break;
		if(ret < 0)
			goto lblKO;
		if(ret == kbdANN)
			goto lblBatal;
		if(ret >= 0)
			continue;
		Beep();
	}
	memset(buf,0,sizeof(buf));
	if(ret == kbdF1)
		strcpy(buf, "0");
	else
		strcpy(buf, "1");
	MAPPUTSTR(traPowerConsuming,buf,lblKO);

	if(atoi(buf) == 0)
	{
		for(i=mnuBeg; i <= mnuEnd; i++)
		{
			x = i - mnuBeg;
			mapGet(mnuBeg + x, tempBuf, sizeof(tempBuf));
			if(i == (int)mnuItem)
			{
				MAPPUTBYTE(appmnuBeg + x, 0, lblKO);
				if(
					(i == mnuCardService) ||
					(i == mnuTunai) ||
					(i == mnuPembelian) ||
					(i == mnuMiniATM) ||
					#ifdef T_BANK
					(i == mnuTCash) ||
					#endif
					(i == mnuBrizzi) ||
					(i == mnuAbsen) ||
					(i == mnuSSB) ||
					(i == mnuLogon)
				)
				{
					tempBuf[0] = '1';
					mapPut(mnuBeg + x, tempBuf, 0);
				}
				else if( ((i > mnuCardService) && (i <= mnuCReprint)) ||
					((i >= mnuTarikTunai) && (i <= mnuTReprint)) ||
					((i >= mnuTarikTunai) && (i <= mnuTReprint)) ||
					((i >= mnuSale) && (i <= mnuPReprint)) ||
					((i == mnuInitialize) && (i <= mnuPrepaidPembayaran)) ||
					((i == mnuPrepaidPembayaran) && (i <= mnuPrepaidReprint)) ||
					(i == mnuTReport) ||
					(i == mnuMrcLog) ||
					(i == mnuInformasi) ||
					(i == mnuTransfer) ||
					(i == mnuPembayaran) ||
					(i == mnuIsiUlang) ||
					(i == mnuRegistrasi) ||
					(i == mnuMReprintReview) ||
					(i == mnuPrepaidReport) ||
					((i == mnuDatang) && (i <= mnuShifts)) ||
					(i == mnuSIM) ||
					(i == mnuUjiSim) ||
					(i == mnuBPKB) ||
					(i == mnuSTCK) ||
					(i == mnuMutasiRan)
				)
				{
					tempBuf[0] = '2';
					mapPut(mnuBeg + x, tempBuf, 0);
				}
				else if(
					((i >= mnuCLastTrx) && (i <= mnuCAnyTrx))||
					((i >= mnuCTodaySummary) && (i <= mnuCPastDetail)) ||
					((i >= mnuTLastTrx) && (i <= mnuTAnyTrx)) ||
					((i >= mnuTTodaySummary) && (i <= mnuTPastDetail)) ||
					((i >= mnuMrcReprint) && (i <= mnuMrcPrnTra)) ||
					((i >= mnuMrcViewTxn) && (i <= mnuMrcBatTot)) ||
					((i >= mnuInfoSaldo) && (i <= mnuInfoKodeBank1)) ||
					((i >= mnuTransferSesamaBRI) && (i <= mnuInfoKodeBank)) ||
					((i >= mnuPembayaranBriva) && (i <= mnuPembayaranPostPaid)) ||
					(i == mnuPembayaranDPLKR) ||
					(i == mnuPembayaranTiketPesawat) ||
					(i == mnuPembayaranPdam) ||
					(i == mnuPembayaranPLN) ||
					(i == mnuPembayaranKK) ||
					(i == mnuPembayaranCicilan) ||
					(i == mnuPembayaranZakat) ||
					(i == mnuPembayaranInfaq) ||
					(i == mnuPembayaranSimpati) ||
					(i == mnuPembayaranMentari) ||
					(i == mnuPembayaranIm3) ||
					(i == mnuPembayaranEsia) ||
					(i == mnuPembayaranSmart) ||
//					(i == mnuPembayaranFren) ||
					(i == mnuPembayaranThree) ||
					(i == mnuPembayaranAxis) ||
					(i == mnuPembayaranXl) ||
					(i == mnuPembayaranXl) ||
					((i >= mnuRegistrasiInternetBanking) && (i <= mnuRegistrasiTrxBanking)) ||
					(i == mnuRegistrasiSmsBanking) ||
					(i == mnuMReprint) ||
					(i == mnuMReview) ||
					(i == mnuMTodaySummary) ||
					(i == mnuMTodayDetail) ||
					(i == mnuMPastSummary) ||
					(i == mnuMPastDetail) ||
					(i == mnuPrepaidPembayaran) ||
					(i == mnuPrepaidPayment) ||
					(i == mnuPrepaidPaymentDisc) ||
					(i == mnuPRLastTrx) ||

					(i == mnuPRAnyTrx) ||
					((i >= mnuPDateSummary) && (i <= mnuPDateDetail)) ||
					(i == mnuShiftsI) ||
					(i == mnuShiftsII) ||
					(i == mnuShiftsIII) ||
					(i == mnuSIMBaru) ||
					(i == mnuSIMPanjang) ||
					(i == mnuSIMPeningkatan) ||
					(i == mnuSIMPenurunan) ||
					(i == mnuSIMMutasi) ||
					(i == mnuSIMHilang) ||
					(i == mnuBPKBBaru) ||
					(i == mnuBPKBGanti) ||
					((i >= mnuMutasiRanRoda2) && (i <= mnuMutasiRanRoda4))
				)
				{
					tempBuf[0] = '3';
					mapPut(mnuBeg + x, tempBuf, 0);
				}
				else if( ((i >= mnuPembayaranHalo) && (i <= mnuPembayaranMatrix)) ||
					((i >= mnuPembayaranTiketGaruda) && (i <= mnuPembayaranTiketSriwijaya)) ||
					((i >= mnuPembayaranPLNPasca) && (i <= mnuPembayaranPLNToken)) ||
					((i >= mnuPembayaranKKBRI) && (i <= mnuPembayaranKKCITI)) ||
					((i >= mnuPembayaranSPP) && (i <= mnuPembayaranKodeUniv)) ||
					(i == mnuPembayaranPdamSby) ||
					(i == mnuPembayaranKKStanChar) ||
					(i == mnuPembayaranKKHSBC) ||
					(i == mnuPembayaranKKRBS) ||
					(i == mnuPembayaranKKANZ) ||
					(i == mnuPembayaranZakatDhuafa) ||
					(i == mnuPembayaranZakatYBM) ||
					(i == mnuPembayaranInfaqDhuafa) ||
					(i == mnuPembayaranInfaqYBM) ||
					((i == mnuPembayaranCicilanFIF) && (i <= mnuPembayaranCicilanWOM)) ||
					((i >= mnuPembayaranSimpati50) &&(i <= mnuPembayaranSimpati1000)) ||
					((i >= mnuPembayaranMentari25) &&(i <= mnuPembayaranMentari1000)) ||
					((i >= mnuPembayaranIm325) &&(i <= mnuPembayaranIm31000)) ||
					((i >= mnuPembayaranEsia25) &&(i <= mnuPembayaranEsia200)) ||
					((i >= mnuPembayaranSmart20) &&(i <= mnuPembayaranSmart500)) ||
					((i >= mnuPembayaranFren20) &&(i <= mnuPembayaranFren500)) ||
					((i >= mnuPembayaranThree20) &&(i <= mnuPembayaranThree500)) ||
					((i >= mnuPembayaranAxis25) &&(i <= mnuPembayaranAxis300)) ||
					((i >= mnuPembayaranXl25) &&(i <= mnuPembayaranXl500)) ||
					((i >= mnuRegistrasiTrxTransfer) &&(i <= mnuRegistrasiTrxPulsa)) ||
					((i >= mnuMLastTrx) &&(i <= mnuMAnyTrx)) ||
					((i >= mnuMVLastTrx) &&(i <= mnuMVAnyTrx)) ||
					((i >= mnuPAllDetailPayment) &&(i <= mnuPAllDetailAll)) ||
					((i >= mnuShiftsIA) &&(i <= mnuShiftsIB)) ||
					((i >= mnuShiftsIIA) &&(i <= mnuShiftsIIB)) ||
					((i >= mnuShiftsIIIA) &&(i <= mnuShiftsIIIB)) ||
					(i == mnuSIMBaruA) ||
					((i >= mnuSIMBaruC) &&(i <= mnuSIMBaruD)) ||
					((i >= mnuSIMPanjangA) &&(i <= mnuSIMPanjangD)) ||
					((i >= mnuSIMPeningkatanAU) &&(i <= mnuSIMPeningkatanB2U)) ||
					((i >= mnuSIMPenurunanA) &&(i <= mnuSIMPenurunanB2)) ||
					(i == mnuSIMMutasiGolSam) ||
					(i == mnuSIMMutasiPeningkatan) ||
					(i == mnuSIMMutasiPenurunan) ||
					(i == mnuSIMHilangPeningkatan) ||
					(i == mnuSIMHilangPenurunan) ||
					((i >= mnuBPKBBaruRoda2) &&(i <= mnuBPKBBaruRoda4)) ||
					((i >= mnuBPKBGantiRoda2) &&(i <= mnuBPKBGantiRoda4))

				)
				{
					tempBuf[0] = '4';
					mapPut(mnuBeg + x, tempBuf, 0);
				}
				else if(
					((i >= mnuPembayaranKKCITIKK) && (i <= mnuPembayaranKKCITIKTA)) ||
					((i >= mnuPembayaranKKStanCharKK) && (i <= mnuPembayaranKKStanCharKTA)) ||
					((i >= mnuPembayaranKKHSBCKK) && (i <= mnuPembayaranKKHSBCKTA)) ||
					((i >= mnuPembayaranKKRBSKK) && (i <= mnuPembayaranKKRBSKTA)) ||
					((i >= mnuSIMGolSamA) && (i <= mnuSIMGolSamD)) ||
					((i >= mnuSIMMutasiPeningkatanAU) && (i <= mnuSIMMutasiPeningkatanB2U)) ||
					((i >= mnuSIMMutasiPenurunanA) && (i <= mnuSIMMutasiPenurunanB2)) ||
					((i >= mnuSIMHilangA) && (i <= mnuSIMHilangD)) ||
					((i >= mnuSIMHilangPeningkatanAU) && (i <= mnuSIMHilangPeningkatanB2U)) ||
					((i >= mnuSIMHilangPenurunanA) && (i <= mnuSIMHilangPenurunanB2))

				)
				{
					tempBuf[0] = '5';
					mapPut(mnuBeg + x, tempBuf, 0);
				}
				break;
			}
		}
	}
	else if(atoi(buf) == 1)
	{
		for(i=mnuBeg; i <= mnuEnd; i++)
		{
			x = i - mnuBeg;
			if(i == (int)mnuItem)
			{
				MAPPUTBYTE(appmnuBeg + x, 1, lblKO);
				mapGet(mnuBeg + x, tempBuf, sizeof(tempBuf));
				tempBuf[0] = '?';
				mapPut(mnuBeg + x, tempBuf, 0);
				break;
			}
		}
	}

	return 1;
	lblBatal:
		//checkSubMnuLayout();
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}
#ifdef ABSENSI
void pmtAbsen(void)
{
	int 		ret;
	word 	key, txnType, subType = 0, nextTxn = 0;
	char 	traName[dspW + 1];
	byte 	isInquiry = 0;

	trcS("pmtBRI Beg\n");
	ret = mapGetWord(traMnuItm, key);
	CHECK(ret >= 0, lblKO);
	VERIFY(key);

	switch(key)
	{
		case mnuDatang:
			isInquiry = 1;
			strcpy(traName,"ABSEN DATANG");
			subType=DATANG;
			txnType = trtAbsensi;
			break;

		case mnuPulangs:
			isInquiry = 1;
			strcpy(traName,"ABSEN PULANG");
			subType=PULANG;
			txnType = trtAbsensi;
			break;

		case mnuIstirahatA:
			isInquiry = 1;
			strcpy(traName,"ISTIRAHAT MULAI");
			subType=ISTIMUL;
			txnType = trtAbsensi;
			break;

		case mnuIstirahatB:
			isInquiry = 1;
			strcpy(traName,"ISTIRAHAT SELESAI");
			subType=ISTIAKH;
			txnType = trtAbsensi;
			break;

		case mnuLemburA:
			isInquiry = 1;
			strcpy(traName,"LEMBUR MULAI");
			subType=LEMTANG;
			txnType = trtAbsensi;
			break;

		case mnuLemburB:
			isInquiry = 1;
			strcpy(traName,"LEMBUR SELESAI");
			subType=LEMLANG;
			txnType = trtAbsensi;
			break;

		case mnuShifts:
			isInquiry = 1;
			strcpy(traName,"SHIFT");
			txnType = trtAbsensi;
			break;

		case mnuShiftsI:
			isInquiry = 1;
			strcpy(traName,"SHIFT 1");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIA:
			isInquiry = 1;
			subType=SHIFTIA;
			strcpy(traName,"DATANG");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIB:
			isInquiry = 1;
			subType=SHIFTIB;
			strcpy(traName,"PULANG");
			txnType = trtAbsensi;
			break;

		case mnuShiftsII:
			isInquiry = 1;
			strcpy(traName,"SHIFT 2");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIIA:
			isInquiry = 1;
			subType=SHIFTIIA;
			strcpy(traName,"DATANG");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIIB:
			isInquiry = 1;
			subType=SHIFTIIB;
			strcpy(traName,"PULANG");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIII:
			isInquiry = 1;
			strcpy(traName,"SHIFT 3");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIIIA:
			isInquiry = 1;
			subType=SHIFTIIIA;
			strcpy(traName,"DATANG");
			txnType = trtAbsensi;
			break;

		case mnuShiftsIIIB:
			isInquiry = 1;
			subType=SHIFTIIIB;
			strcpy(traName,"PULANG");
			txnType = trtAbsensi;
			break;
	}

	txnAsli = 0;
	mapPut(traRspCod, "", 2);
	MAPPUTBYTE(regLocType,'T',lblKO);
	MAPPUTBYTE(traSubType,subType,lblKO);
	MAPPUTBYTE(traInquiryFlag,isInquiry,lblKO);
	MAPPUTBYTE(traNextTxn,nextTxn,lblKO);
	MAPPUTSTR(traAmt,"1000",lblKO);

	mapMove(rqsBeg, (word) (txnType - 1));
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, txnType, lblKO);
	if( pmtPayment(key) != 1)
		goto lblEnd;

	nextTxn   = 0;
	isInquiry = 0;
	goto lblEnd;

	lblKO:
	lblEnd:
		return;
}
#endif

#if 0
	int magStripeInput(void)
	{
	int ret;
	word key;
	byte PanDisplay;
	char Amt[lenAmt + 1];
	byte txnType;

	trcS("magStripeInput Beg\n");
	//Bao add for EMV
	if(UseChip == 1){
	dspClear();
	usrInfo(infEMVChipDetect);
	Beep();
	ret = 0;
	goto lblEnd;
	}
	ret = valCard();            // Track2 analysis
	CHECK(ret > 0, lblKO);

	MAPGETBYTE(appTermOpt1, PanDisplay, lblKO);
	if(PanDisplay & 0x02)
	{
	ret = usrInfo(infConfirmPan);
	CHECK(ret >= 0, lblKO);
	CHECK(ret == kbdVAL, lblExit);
	}
	ret = 1;
	goto lblEnd;

	lblExit:                     //smooth exit
	ret = 0;
	goto lblEnd;
	lblKO:
	goto lblEnd;
	lblEnd:
	trcFN("pmtMagStripe: ret=%d\n", ret);
	return ret;
	}


	int smartCardInput()
	{
	int ret;

#ifdef __EMV__
	MAPPUTSTR(traChipSwipe, "-DIP", lblStop);
	emvFullProcessSet();

	ret = pmtSmartCard();
#endif
	if(CheckFallBack(ret) == 1)
	{
	dspClear();
	usrInfo(infCardErr);
	//usrInfo(infUseMagStripe);
	removeCard();

	MAPPUTBYTE(traEntMod, 'f', lblDBA);
	MAPPUTSTR(traChipSwipe, "-FALLBACK", lblStop);
	//		pmtSale();

	goto lblFinish;
	}
	//End
	if (emvFullProcessCheck() == 0)
	{
	byte emvTrack2Bcd[lenEmvTrk2+ 1];
	card track2Len;
	char emvTrack2Hex[lenTrk2+ 1];
	MEM_ZERO(emvTrack2Hex);
	MEM_ZERO(emvTrack2Bcd);
	mapGet(emvTrk2, emvTrack2Bcd, sizeof(emvTrack2Bcd));
	bin2num(&track2Len, &emvTrack2Bcd[0], 1);
	bin2hex(emvTrack2Hex, emvTrack2Bcd+1,track2Len);
	mapPut(traTrk2, emvTrack2Hex, (track2Len*2)-1);

	MAPGET(traTrk2, Trk2, lblDBA);

	//get PAN from Track2
	ptr = Trk2;
	ret = fmtTok(0, ptr, "D");    // Search seperator '='
	CHECK(ret <= lenPan, lblInvalidTrk);
	ret = fmtTok(Pan, ptr, "D");  // Retrieve Pan from track 2
	VERIFY(ret <= lenPan);

	//get Expiry Date from Track2
	ptr += ret;
	VERIFY(*ptr == 'D');
	ptr++;
	ret = fmtSbs(ExpDat, ptr, 0, lenExpDat);  // Retrieve expiration date from track 2
	VERIFY(ret == 4);
	MAPPUTSTR(traExpDat, ExpDat, lblDBA);

	strcpy(tmp, Pan);
	strcat(tmp, "=");
	strcat(tmp, ExpDat);
	ret = mapPut(traTrk21, tmp, dspW);
	CHECK(ret >= 0, lblDBA);
	MAPPUTSTR(traTrk22, tmp + dspW, lblDBA);

	MAPPUTSTR(traPan, Pan, lblDBA);
	memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
	BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr
	MAPPUTSTR(traBlockPan, BlockedPan,lblDBA);

	MAPPUTSTR(traChipSwipe, "-DIP", lblStop);
	pmtMagStripe();
	}
	}

#endif

int fmtAmtReceipt2(word key, char *amt)
{
	return(fmtAmtReceipt(key, amt));
}

