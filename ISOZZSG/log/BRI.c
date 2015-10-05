#include "SDK30.h"
#include "OEM_Public.h"
#include "sys.h"
#undef __USE_DEL_INTERFACE__
#include "TlvTree.h"
#include "LinkLayer.h"
#include "oem_dgprs.h"
#ifdef __CHN_GPRS__
#include "gprs_lib.h"
#endif

#ifdef __CHN_SSL__
#include <string.h>
#include <SSL_.h>
#include <X509_.h>
#endif

#ifdef __CHN_FTP__
#include <FTP_.h>
#endif

#ifdef __CHN_TCP__
#include<IP_.h>
#endif

#ifdef __CHN_SCK__
#include<IP_.h>
#endif

#include "stdlib.h"
#include "BRI.h"
#include "prepaid.h"
#include "lineEncryption.h"
#include "prepaid.h"

typedef struct sComChn {
    byte chn;                   // communication channel used
    byte chnRole;
    struct {
        LL_HANDLE hdl;
        char separator;
        unsigned long int conn_delay;   // 4 Second delay
        unsigned long int ll_conn_delay;
        unsigned int send_timeout;
        int ifconn;
#ifdef __CHN_COM__
        byte BperB;             //BITS_PER_BYTE 7 or not
        byte Parity;            //PARITY
#endif
#ifdef __CHN_SCK__
        int hdlSck;
        struct sockaddr_in peeraddr_in; // For peer socket address.
#endif
#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs
            unsigned int cTimeout;
            unsigned int cValue;
        } gprs;
#endif
#ifdef __CHN_SSL__
        SSL_HANDLE hdlSsl;
        SSL_PROFILE_HANDLE hdlProfile;
        char SslProfName[PROFILE_NAME_SIZE + 1];
#endif
    } prm;
    TLV_TREE_NODE hCfg;
    TLV_TREE_NODE hPhyCfg;
    TLV_TREE_NODE hDLinkCfg;
    TLV_TREE_NODE hTransCfg;
} tComChn;





extern tComChn prepaidCom;

//++ @ BRI report


typedef struct
{
    char amt[lenAmt+1];
    char pan[lenPan+1];
    char txnDateTime[lenDatTim+1];
    char refNum[lenRrn+1];
    char expDat[lenExpDat+1];
    char appCode[lenAutCod+1];
    char transName[21];
    card stan;
    card roc;

    //card service log
    char nomorKartuPetugas[lenNomorKartuPetugas+1];
    char nomorKartuLama[lenNomorKartuLama+1];
}DETAIL_TRANS_T;

static word mLogKey[mLogEnd - mLogBeg - 1] = {
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
    traExpDatFmt,			//<logExpDatFmt
    traTrk2,                    ///<logTrk2: rack 2 of the card
    traAutCod,                  ///<logAutCod: Authorization (approval) code
    regAutCodOff,
    acqCurBat,                  ///<logBatNum: batch number
    traAmt,                     ///<logTraAmt: Transaction Amount
    traFmtAmt,		  //amount formatted
    traTipAmt,                  ///<logTipAmt:  Transaction Tip Amount
    traFmtTip,
    traEmvICC,		//revICCData, bit 55
    regSTAN,                    ///<logSTAN: System Trace Audit Number 1..999999
    traRrn,                     ///<logRrn: Retrieval Reference Number
    traRspCod,		//response code
    traPinBlk,
    regInvNum,                  ///<logROC: invoice number / R.O.C.
    traEntMod,                  ///<logEntMod: POS Entry Mode
    traConCode,                 ///<logConCod: POS Condition Code
    traCardType,                 ///<logCardType: Card Type

    traSubType,
    traInquiryAmt,

    traNomorRekeningTujuan,
    traNomorRekeningTujuanBRI,
    traNamaPenerima,
    traNomorRekeningAsal,
    traNomorRekeningAsalBRI,
    traNamaPengirim,
    traCustomerRefNumber,
    traNamaBankAsal,
    traNamaBankTujuan,
    traJenisRekening,
    traJenisTransfer,

    traKodeWilayah,
    traNamaPelangganPLN,
    traPelangganId,
    traLWBP,
    traMPLO,
    traMeter,
    traBillPeriod1,
    traBillPeriod2,
    traBillPeriod3,
    traBillPeriod4,
    traCustomerSegment,
    traPowerConsuming,

    traTotalOutstanding,
    traBillStatus,
    traPaymentStatus ,
    traCustomerName,
    traContractNumber,
    traCustomerType,
    traAdminCharge,
    traPaymentNumber,
    traDueDate,
    traTotalAmount,
    traPokokHutang,
    traBunga,
    traPenaltyFee,

    traNoTelpon,
    traNominalVoucher,
    traPeriode,
    traNoVoucher,
    traKodeMRP,

    traSisaSaldo,
    traFee,
    traJenisKartu
};

static word cLogKey[cLogEnd - cLogBeg - 1] = {
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
    traExpDatFmt,			//<logExpDatFmt
    traTrk2,                    ///<logTrk2: rack 2 of the card
    traAutCod,                  ///<logAutCod: Authorization (approval) code
    regAutCodOff,
    acqCurBat,                  ///<logBatNum: batch number
    traAmt,                     ///<logTraAmt: Transaction Amount
    traFmtAmt,		  //amount formatted
    traTipAmt,                  ///<logTipAmt:  Transaction Tip Amount
    traFmtTip,
    traEmvICC,		//revICCData, bit 55
    regSTAN,                    ///<logSTAN: System Trace Audit Number 1..999999
    traRrn,                     ///<logRrn: Retrieval Reference Number
    traRspCod,		//response code
    traPinBlk,
    regInvNum,                  ///<logROC: invoice number / R.O.C.
    traEntMod,                  ///<logEntMod: POS Entry Mode
    traConCode,                 ///<logConCod: POS Condition Code
    traCardType,                 ///<logCardType: Card Type

    traNomorKartuPetugas,
    traNomorRekeningAsalBRI,
    traNamaPenerima,
    traTanggalLahir,
    traNomorKartuLama,
    traKodeUkerLama,
    traKodeUkerBaru,
    traStatusKartu
};

static word tLogKey[tLogEnd - tLogBeg - 1] = {
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
    traExpDatFmt,			//<logExpDatFmt
    traTrk2,                    ///<logTrk2: rack 2 of the card
    traAutCod,                  ///<logAutCod: Authorization (approval) code
    regAutCodOff,
    acqCurBat,                  ///<logBatNum: batch number
    traAmt,                     ///<logTraAmt: Transaction Amount
    traFmtAmt,		  //amount formatted
    traTipAmt,                  ///<logTipAmt:  Transaction Tip Amount
    traFmtTip,
    traEmvICC,		//revICCData, bit 55
    regSTAN,                    ///<logSTAN: System Trace Audit Number 1..999999
    traRrn,                     ///<logRrn: Retrieval Reference Number
    traRspCod,		//response code
    traPinBlk,
    regInvNum,                  ///<logROC: invoice number / R.O.C.
    traEntMod,                  ///<logEntMod: POS Entry Mode
    traConCode,                 ///<logConCod: POS Condition Code
    traCardType,                 ///<logCardType: Card Type
    traTrk1,

    traNomorRekeningAsalBRI,
    traNamaPenerima,
    traSisaSaldo,
    traFee
};

static word tCashLogKey[tCashLogEnd - tCashLogBeg - 1] = {
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
    traExpDatFmt,			//<logExpDatFmt
    traTrk2,                    ///<logTrk2: rack 2 of the card
    traAutCod,                  ///<logAutCod: Authorization (approval) code
    regAutCodOff,
    acqCurBat,                  ///<logBatNum: batch number
    traAmt,                     ///<logTraAmt: Transaction Amount
    traFmtAmt,		  //amount formatted
    traTipAmt,                  ///<logTipAmt:  Transaction Tip Amount
    traFmtTip,
    traEmvICC,		//revICCData, bit 55
    regSTAN,                    ///<logSTAN: System Trace Audit Number 1..999999
    traRrn,                     ///<logRrn: Retrieval Reference Number
    traRspCod,		//response code
    traPinBlk,
    regInvNum,                  ///<logROC: invoice number / R.O.C.
    traEntMod,                  ///<logEntMod: POS Entry Mode
    traConCode,                 ///<logConCod: POS Condition Code
    traCardType,                 ///<logCardType: Card Type
    traTrk1,

    traNomorRekeningAsalBRI,
    traNamaPenerima,
    traSisaSaldo,
    traFee
};

//--  BRI1 report

static word cLogKeyContext[cLogEnd - cLogBeg - 1] = {  //for some unknown reason , logEnd - logBeg doesnt work... for now hardcoded...
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
            traCardTypeContext        ///<logCardType: Card Type
        };

static word mLogKeyContext[mLogEnd - mLogBeg - 1] = {  //for some unknown reason , logEnd - logBeg doesnt work... for now hardcoded...
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
            traCardTypeContext         ///<logCardType: Card Type
        };

static word tLogKeyContext[tLogEnd - tLogBeg - 1] = {  //for some unknown reason , logEnd - logBeg doesnt work... for now hardcoded...
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
            traCardTypeContext         ///<logCardType: Card Type
        };

static word tCashLogKeyContext[tCashLogEnd - tCashLogBeg - 1] = {  //for some unknown reason , logEnd - logBeg doesnt work... for now hardcoded...
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
            traCardTypeContext         ///<logCardType: Card Type
        };


//static tComChn prepaidCom;
//byte prepaidRespCode[2];
static char tempTiket[25][10];   //@ar - v0.45

#define CHK if(ret<0) return ret;

void hapusAwalan(char* str,char c)
{
	/*********************************************************************
	Diberi akhiran NULL
	**********************************************************************/
	card len=0;
	card i=0;
	card lenStr = strlen(str);

	for(i=0; i < lenStr; i++)
	{
		len = lenStr-i;
		if(str[i] == c) continue;
		memmove(str,str+i,len);
		str[len]=0;
		return;
	}
	if(i == lenStr)
	{
		str[0] = 0;
	}
}

void trimRight(char* buf, byte chr)
{
	char* ptr;

	ptr = buf+strlen(buf)-1;
	while(1)
	{
		if(*ptr == chr)
		{
			*ptr = 0;
			ptr--;
			continue;
		}
		break;
	}
}

void trimLeft(char* buf, byte chr)
{
	byte i;

	i=0;
	while (i<strlen(buf)) {
		if (buf[i] == chr)
			i++;
		else
			break;
	}

	memmove(buf,&buf[i],strlen(buf)-i);
	buf[strlen(buf)-i] = 0x0;
}


int isBRIPrint(void)
{
	byte 	type;
	int 		ret;

	MAPGETBYTE(traTxnType,type,lblKO);

	switch(type)
	{
		case trtInfoSaldo:
			ret = KonfirmasiInfoSaldo8();
			return ret;

		case trtInfoSaldoBankLain:
			ret = KonfirmasiInfoSaldoLain8();
			return ret;

		case trtMiniStatement:
			return -1;

		case trtMutRek:
			return -1;

		case trtPrevilege:
			return -1;

		case trtTransferSesamaBRIInquiry:
		case trtTransferAntarBankInquiry:
		case trtPembayaranTVBerlanggananInquiry:
		case trtPembayaranTiketPesawatInquiry:
		case trtPembayaranPdamInquiry:
		case trtPembayaranTelkomInquiry:
		case trtPembayaranBrivaInquiry:
		case trtTrxPhoneBankingTransferInquiry:
		case trtPembayaranKKBRIInquiry:
		case trtPembayaranSPPInquiry:
#ifdef SSB
		case trtPembayaranSSBInquiry:
#endif
		case trtPembayaranPLNPascaInquiry:
		case trtPembayaranPLNPraInquiry:
		case trtPembayaranCicilanInquiry:
		case trtSetorSimpananInquiry:
		case trtSetorPinjamanInquiry:
		case trtAktivasiKartuOtorisasi:
		case trtAktivasiKartuInquiry:
		case trtRenewalKartuOtorisasi:
		case trtRenewalKartuInquiry:
		case trtReissuePINOtorisasi:
		case trtPembayaranPascaBayarInquiry:
			return 0;

		default:
			return 1;
	}

	lblKO:
		return -1;
}

char* convertBulan(byte jenis, char* bulanHuruf, byte bulanAngka)
{
	char *blnPendek[]={
		"-: ", //monSpr
		"Jan", //monJan
		"Feb", //monFeb
		"Mar", //monMar
		"Apr", //monApr
		"May", //monMay
		"Jun", //monJun
		"Jul", //monJul
		"Aug", //monAug
		"Sep", //monSep
		"Oct", //monOct
		"Nov", //monNov
		"Dec", //monDec
	};

	char *blnPanjang[]={
		"-: ", //monSpr
		"JANUARI", //monJan
		"FEBRUARI", //monFeb
		"MARET", //monMar
		"APRIL", //monApr
		"MEI", //monMay
		"JUNI", //monJun
		"JULI", //monJul
		"AGUSTUS", //monAug
		"SEPTEMBER", //monSep
		"OKTOBER", //monOct
		"NOVEMBER", //monNov
		"DESEMBER", //monDec
	};

	if(bulanAngka <= 12)
	{
		if(jenis == 0)
			strcpy(bulanHuruf,blnPendek[bulanAngka]);
		else
			strcpy(bulanHuruf,blnPanjang[bulanAngka]);
	}
	else
		bulanHuruf[0] = 0;

	return bulanHuruf;
}

int isBRISaveToBatch(void)
{
	byte 	type;
	int 		ret;
	char 	RspCod[lenRspCod + 1];

	MAPGETBYTE(traTxnType,type,lblKO);
	MAPGET(traRspCod, RspCod, lblKO);
	switch(type)
	{
		case trtInfoSaldo:
		case trtTransferSesamaBRIInquiry:
		case trtTransferAntarBankInquiry:
		case trtMiniStatement:
		case trtMutRek:
		case trtPrevilege:
		case trtPembayaranTVBerlanggananInquiry:
		case trtPembayaranTiketPesawatInquiry:
		case trtPembayaranPdamInquiry:
		case trtPembayaranTelkomInquiry:
#ifdef SSB
		case trtPembayaranSSBInquiry:
#endif
		case trtPembayaranBrivaInquiry:
		case trtTrxPhoneBankingTransferInquiry:
		case trtRegistrasiSmsBanking:
		case trtPembayaranKKBRIInquiry:
		case trtPembayaranSPPInquiry:
		case trtPembayaranPLNPascaInquiry:
		case trtPembayaranPLNPraInquiry:
		case trtPembayaranPLNToken:
		case trtPembayaranCicilanInquiry:
		case trtSetorSimpananInquiry:
		case trtSetorPinjamanInquiry:
		case trtAktivasiKartuOtorisasi:
		case trtAktivasiKartuInquiry:
		case trtRenewalKartuOtorisasi:
		case trtRenewalKartuInquiry:
		case trtReissuePINOtorisasi:
		//case trtTCash:
		//case trtTCashOut:
		case trtRegistrasiInternetBanking:
		case trtRegistrasiPhoneBanking:
		case trtGantiPIN:
		case trtGantiPasswordSPV:
		case trtPembayaranPascaBayarInquiry:
#ifdef ABSENSI
		case trtAbsensi:
#endif
			return 0;
#ifdef PREPAID
		case trtPrepaidInfoDeposit:
			return 0;

		case trtPrepaidPayment:
		case trtPrepaidPaymentDisc:
			return 1;
#endif
		default:
			if(memcmp(RspCod,"00",2)==0)
				return 1;
			switch(type)
			{
				case trtAktivasiKartu:
				case trtRenewalKartu:
					if(memcmp(RspCod,"NC",2)==0)
					return 1;
				default:
					return 0;
			}
			return 0;
	}

	lblKO:
		return -1;
}

int isBRITransaction (void)
{
	byte type;
	int 	ret;

	MAPGETBYTE(traTxnType,type,lblKO)
	if( (type >= trtInfoSaldo && type <= trtPembayaranPascaBayar)
#ifdef ABSENSI
	|| (type == trtAbsensi)
#endif
#ifdef SSB
	|| (type == trtPembayaranSSB) || (type == trtPembayaranSSBInquiry)
#endif
	)
		return 1;
	else
		return 0;

	lblKO:
		return -1;
}

int isPembelianTransaction (void)
{
	int 		ret;
	word 	mnuItem;

	MAPGETWORD(traMnuItm,mnuItem,lblKO);
	if(mnuItem > mnuPembelian && mnuItem <= mnuSettlement)
		return 1;
	else
		return 0;

	lblKO:
		return -1;
}

//++ @agmr - BRI1
int whichBRITransaction (void)
{
	byte 	type;
	int 		ret;

	MAPGETBYTE(traTxnType,type,lblKO)
	switch(type)
	{
		case trtInfoSaldo :
		case trtInfoSaldoBankLain:
		case trtMiniStatement:
		case trtMutRek:
		case trtPrevilege:
		case trtTransferSesamaBRI:
		case trtTransferAntarBank:
		case trtPembayaranPLNPasca:
		case trtPembayaranPLNPra:
		case trtPembayaranCicilan:
		case trtPembayaranPulsa:
		case trtPembayaranZakat:
		case trtSetorPasti:
		case trtPembayaranPascaBayar:
		case trtPembayaranTVBerlangganan:
		case trtPembayaranPdam:
		case trtPembayaranTelkom:
#ifdef SSB
		case trtPembayaranSSB:
#endif
		case trtPembayaranDPLKR:
		case trtPembayaranBriva:
		case trtTrxPhoneBankingTransfer:
		case trtTrxPhoneBankingPulsa:
		case trtRegistrasiSmsBanking:
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
		case trtPembayaranSPP:
			return MINI_ATM_LOG;

		case trtSetorPinjaman:
		case trtSetorSimpanan:
		case trtTarikTunai:
		case trtVoidTarikTunai:
			return TUNAI_LOG;

		//case trtPreCom:
		case trtAktivasiKartu:
		case trtRenewalKartu:
		case trtReissuePIN:
			return CARD_SERVICE_LOG;

		case trtTCash:
		case trtTCashOut:
			return TCASH_LOG;

#ifdef PREPAID
		case trtPrepaidInfoDeposit:
		case trtPrepaidPayment:
		case trtPrepaidPaymentDisc:
		case trtPrepaidDeposit:
		case trtPrepaidTopUp:
		case trtPrepaidRedeem:
		case trtPrepaidAktivasi:
		//++ @agmr - brizzi2
		case trtPrepaidReaktivasi:
		case trtPrepaidVoid:
		//-- @agmr - brizzi2
#endif
		case trtInstallment:
		case trtRegistrasiInternetBanking:
		case trtRegistrasiPhoneBanking:
		case trtGantiPIN:
		case trtGantiPasswordSPV:
		case trtReissuePINOtorisasi:
		case trtRenewalKartuOtorisasi:
		case trtAktivasiKartuOtorisasi:
		case trtTransferSesamaBRIInquiry:
		case trtTransferAntarBankInquiry:
		case trtPembayaranPLNPascaInquiry:
		case trtPembayaranPLNPraInquiry:
		case trtPembayaranPLNToken:
		case trtPembayaranCicilanInquiry:
		case trtSetorPinjamanInquiry:
		case trtSetorSimpananInquiry:
		case trtAktivasiKartuInquiry:
		case trtRenewalKartuInquiry:
		case trtPembayaranPascaBayarInquiry:
		default:
			return 0;
	}

	lblKO:
		return 0;
}
//-- @agmr - BRI1

int pmtBRICustomInput()
{
	int 	ret;
	byte txnType;

	MAPGETBYTE(traTxnType, txnType, lblKO);
	switch(txnType)
	{
		case trtRegistrasiInternetBanking:
		case trtRegistrasiPhoneBanking:
		case trtInfoSaldo:
		case trtMiniStatement:
		case trtMutRek:
		case trtPrevilege:
		case trtRenewalKartuOtorisasi:
		case trtAktivasiKartuOtorisasi:
		case trtReissuePINOtorisasi:
			return 1;

		case trtTCash:
			return(inputTCash());

		case trtTCashOut:
			return 1; //(inputTCash());

		case trtPreAuth:
			return 1;
		case trtPreCancel:
		case trtPreCom:
			return (inputPreAuth());

		case trtInfoSaldoBankLain:
			return (inputInfoSaldoBankLain());

		case trtTransferSesamaBRIInquiry:
			return (inputTransferSesamaBRI());

		case trtTransferAntarBankInquiry:
			return (inputTransferAntarBank());

		case trtPembayaranPLNPascaInquiry:
			return (inputPembayaranPLN());

		case trtPembayaranPLNPraInquiry:
			return (inputPembayaranPLNPra());

		case trtPembayaranPLNToken:
			return (inputPembayaranPLNToken());

		case trtPembayaranPulsa:
			return (inputPembayaranPulsa());

		case trtPembayaranCicilanInquiry:
			return (inputPembayaranCicilan());

		case trtSetorSimpananInquiry:
		case trtSetorPinjamanInquiry:
			return (inputSetoran(txnType));

		case trtTarikTunai:
			return (inputTarikTunai());

		case trtPembayaranZakat:
			return (inputPembayaranZakat());

		case trtSetorPasti:
			return (inputSetorPasti());

		case trtInstallment:
			return (inputInstallment());

		case trtPembayaranPascaBayarInquiry:
			return (inputPembayaranPascaBayar());

		case trtGantiPasswordSPV:
			return (inputGantiPasswordSPV());

		case trtReissuePIN:
			return (inputReissuePIN());

		case trtRegistrasiSmsBanking:
			return (inputSmsBanking());

		case trtPembayaranTVBerlanggananInquiry:
			return (inputPembayaranTV());

		case  trtPembayaranTiketPesawatInquiry:
			return (inputTiketPesawat());

		case trtPembayaranPdamInquiry:
			return (inputPembayaranPdam());

		case trtPembayaranTelkomInquiry:
			return (inputPembayaranTelkom());
#ifdef SSB
		case trtPembayaranSSBInquiry:
			return (inputPembayaranSSB());
#endif
		case trtPembayaranDPLKR:
			return (inputPembayaranDPLKR());

		case trtPembayaranBrivaInquiry:
			return (inputPembayaranBriva());

		case trtTrxPhoneBankingTransferInquiry:
			return (inputTrxTransfer());

		case trtTrxPhoneBankingPulsa:
			return (inputTrxPulsa());

		case trtPembayaranKKBRIInquiry:
			return (inputPembayaranKKBRI());

		case trtPembayaranKKANZ:
			return (inputPembayaranKKANZ());

		case trtPembayaranKKCITIKK:
		case trtPembayaranKKCITIKTA:
			return (inputPembayaranKKCITI());

		case trtPembayaranKKHSBCKK:
		case trtPembayaranKKHSBCKTA:
			return (inputPembayaranKKHSBC());

		case trtPembayaranKKRBSKK:
		case trtPembayaranKKRBSKTA:
			return (inputPembayaranKKRBS());

		case trtPembayaranKKStanCharKTA:
		case trtPembayaranKKStanCharKK:
			return (inputPembayaranKKStanChar());

		case trtPembayaranSPPInquiry:
			return (inputPembayaranSPP());
#ifdef PREPAID
		case trtPrepaidDeposit:
			return (inputPrepaidDeposit());

		case trtPrepaidTopUp:
		case trtPrepaidAktivasi:
			ret = pmtPrepaidInquiry();
			if(ret < 0)
				goto lblKO;
			break;

		case trtPrepaidRedeemInquiry:
			ret = pmtPrepaidRedeemInquiry();
			if(ret < 0)
				goto lblKO;
			break;
		//++@agmr - brizzi2
		case trtPrepaidReaktivasiInquiry:
			ret = pmtPrepaidReaktivasiInquiry();
			if(ret < 0)
				goto lblKO;
			break;
		//--@agmr - brizzi2
#endif
		case trtDummy:
			return 1;

		default:
			break;
	}

	return 1;

	lblKO:
		if(ret == -100)
		return ret;

	return -1;
}

int infoKodeUniv()
{
	char msg[61][22] =
	{
		"001: Universitas",
		"     Indonesia",
		"002: Universitas",
		"     Padjadjaran",
		"003: Universitas",
		"     Negeri ",
		"     Malang",
		"004: Universitas",
		"     Jember",
		"005: UIN Bandung",
		"006: Universitas",
		"     Terbuka",

		"007: UIN Jakarta",
		"008: Universitas",
		"     Nasional",
		"     Jakarta",
		"009: ITN Malang",
		"010: Universitas",
		"     Airlangga",
		"011: Insitut ",
		"     Teknologi",
		"     Bandung",
		"012: Universitas",
		"     Atma Jaya",
		"     Yogyakarta",

		"013: Universitas",
		"     Islam",
		"     Indonesia",
		"014: Universitas",
		"     Sam Ratulangi",
		"015: UPI Padang",
		"016: Universitas",
		"     Diponegoro",
		"017: Universitas",
		"     Trunojoyo",
		"     Madura",
		"018: Universitas",
		"     Kristen",
		"     Satya Wacana",

		"019: Universitas",
		"     Negeri Semarang",
		"020: Universitas",
		"     Nusa Cendana ",
		"     Kupang",
		"021: Universitas",
		"     Ahmad Dahlan",
		"022: Universitas",
		"     Haluleo",
		"023: Universitas",
		"     Janabadra",
		"024: UPN Veteran",
		"     Yogyakarta",

		"025: Sekolah Tinggi",
		"     Pariwisata",
		"     Bandung",
		"026: Universitas",
		"     Jendral",
		"     Soedirman",
		"027: UIN Sumatra",
		"     Utara",
		{NULL}
	};
	int ret;

	ret = scrollDisplay8(msg,60,6,"    KODE UNIV"," ");
	return ret;
}

int infoKodeBank()
{
	char msg[151][22] =
	{
		"002:B R I",
               "008:MANDIRI",
               "009:B N I",
               "200:BTN",
               "014:BCA",


               "016:B I I",
               "110:B J B",
               "451:B S M",
               "052:R B S",
               "441:BUKOPIN",
               "031:CITI BANK",


               "111:D K I",
               "426:M E G A",
               "022:CIMB NIAGA",
               "422:BRI SYARIAH",
               "011:DANAMON",
               "113:JATENG",


               "114:JATIM",
               "147:MUAMALAT",
               "028:OCBC NISP",
               "097:MAYAPADA",
               "013:PERMATA",
               "089:RABO BANK",

               "023:BANK UOB",
               "119:RIAU",
               "120:SUMSEL",
               "124:KALTIM",
               "132:PAPUA",
               "153:SINAR MAS",

               "145:BNP",
               "558:PUNDI",
               "494:BRI AGRO",
               "036:WINDU",
               "041:HSBC",
               "050:STANCHRT",
               "037:ARTA GRAHA",

               "506:BSMI",
               "950:COMMBANK",
               "122:KALSEL",
               "123:KALBAR",
               "125:KALTENG",
               "042:TOKYO",

               "213:BTPN",
               "133:BENGKULU",
               "553:MAYORA",
               "019:PANIN",
               "061:ANZ INDONESIA",
               "054:CAPITAL",

               "087:EKONOMI",
               "112:D I Y",
               "115:JAMBI",
               "116:ACEH",
               "117:SUMUT",
               "118:NAGARI",

               "121:LAMPUNG",
               "126:SULSEL",
               "127:SULUT",
               "134:SULTENG",
               "135:SULTRA",
               "425:JABARSYA",

               "076:BUMI ARTHA",
               "513:INA PERDANA",
               "088:ANTAR DAERAH",
               "095:MUTIARA",
               "128:N T B",
               "129:BALI",

               "130:N T T",
               "131:MALUKU",
               "146:SWADESI",
               "151:MESTIKA",
               "157:MASPION",
               "161:GANESHA",

               "167:KESAWAN",
               "212:B SAUDARA",
               "501:ROYAL",
               "535:BKE",
               "542:ARTOS",
               "555:INDEX",

               "566:VICTORIA",
               "485:BUMI PUTERA",
               "498:INDOMONEX",
               "688:BPR KS",
               "472:JASA JAKARTA",
               "503:NOBOU BANK",

               "046:BANK DBS",
               "068:BANK WORI",
               "    INDONESIA",
               "517:PANIN BANK",
               "            SYARIAH",

               "484:BANK HANA",
               "523:SAHABAT",
               "699:BANK EKA",
               "600:BPR/LSB",
               "987:LAINNYA",
		{NULL}
	};
	int ret;

	ret = scrollDisplay8(msg,151,6,"    KODE BANK"," ");
	return ret;
}

int pmtBRIKonfirmasiTanpaInquiry(byte txnType)
{
	int ret;

	switch(txnType)
	{
		case trtPembayaranPulsa:
			ret = KonfirmasiPembayaranPulsa();
			break;

		case trtPembayaranDPLKR:
			ret = KonfirmasiPembayaranDPLKR();
			break;

		case trtPembayaranZakat:
			ret = KonfirmasiPembayaranZakat();
			break;

		case trtSetorPasti:
			ret = KonfirmasiSetorPasti();
			break;

		case trtTCash:
			ret = KonfirmasiTCash();
			break;

		/*case trtTCashOut:
			ret = KonfirmasiTCash();
			break; */

		case trtRegistrasiInternetBanking:
			ret = KonfirmasiRegistrasiInternetBanking();
			break;

		case trtRegistrasiPhoneBanking:
			ret = KonfirmasiRegistrasiPhoneBanking();
			break;

		case trtRegistrasiSmsBanking:
			ret = KonfirmasiRegistrasiSmsBanking();
			break;

		case trtPembayaranKKBRI:
			ret = KonfirmasiPembayaranKKBRIBayar();
			break;

		case trtPembayaranKKANZ:
			ret = KonfirmasiPembayaranKKANZ();
			break;

		case trtPembayaranKKCITIKK:
		case trtPembayaranKKCITIKTA:
			ret = KonfirmasiPembayaranKKCITI();
			break;

		case trtPembayaranKKHSBCKK:
		case trtPembayaranKKHSBCKTA:
			ret = KonfirmasiPembayaranKKHSBC();
			break;

		case trtPembayaranKKRBSKK:
		case trtPembayaranKKRBSKTA:
			ret = KonfirmasiPembayaranKKRBS();
			break;

		case trtPembayaranKKStanCharKK:
		case trtPembayaranKKStanCharKTA:
			ret = KonfirmasiPembayaranKKStanChar();
			break;

		case trtReissuePINOtorisasi:
		case trtAktivasiKartuOtorisasi:
			ret = KonfirmasiKartuOtorisasi();
			break;

		case trtInstallment:
			ret = KonfirmasiInstallment();
			break;

		case trtVoidTarikTunai:
			ret = KonfirmasiVoidTarikTunai();
			break;

		case trtTrxPhoneBankingPulsa:
			ret = KonfirmasiTrxPulsa();
			break;

		default:
			ret = 1;
			break;

	}

	return ret;
}


int pmtBRIKonfirmasiSesudahInquiry()
{
	int 	ret;
	byte isInquiry = 0;
	byte nextTxn = 0;
	byte txnType;
	char buf[25];

	MAPGETBYTE(traTxnType, txnType, lblKO);
	MAPGETBYTE(traInquiryFlag, isInquiry, lblKO);

	MAPPUTBYTE(traInquiryFlag,0,lblKO);
	switch(txnType)
	{
		case trtTransferSesamaBRIInquiry:
			ret = KonfirmasiTransferSesamaBRI();
			break;

		case trtTransferAntarBankInquiry:
			ret = KonfirmasiTransferAntarBank();
			break;

		case trtPembayaranCicilanInquiry:
			ret = KonfirmasiPembayaranCicilan8();
			break;

		case trtSetorPinjamanInquiry:
		case trtSetorSimpananInquiry:
			ret = KonfirmasiSetoran(txnType);
			break;

		case trtPembayaranTVBerlanggananInquiry:
			ret = KonfirmasiPembayaranTV();
			break;

		case trtPembayaranTiketPesawatInquiry:
			ret = KonfirmasiPembayaranTiketPesawat();
			break;

		case trtPembayaranPdamInquiry:
			ret = KonfirmasiPembayaranPdam();
			break;

		case trtPembayaranTelkomInquiry:
			ret = KonfirmasiPembayaranTelkom();
			break;
#ifdef SSB
		case trtPembayaranSSBInquiry:
			ret = KonfirmasiPembayaranSSB();
			break;
#endif

		case trtPembayaranBrivaInquiry:
			memset(buf, 0, sizeof(buf));
			ret = KonfirmasiPembayaranBriva();
			mapGet(traBillStatus, buf,lenBillStatus);
			if(strcmp(buf, "Y") == 0)
			{
				if(ret>0)
				ret = KonfirmasiPembayaranBriva2();
			}
			break;

		case trtTrxPhoneBankingTransferInquiry:
			ret = KonfirmasiTrxTransfer();
			break;

		case trtPembayaranKKBRIInquiry:
			ret = KonfirmasiPembayaranKKBRIBayar();
			break;

		case trtPembayaranSPPInquiry:
			ret = KonfirmasiPembayaranSPP();
			break;
#ifdef ABSENSI
		case trtAbsensi:
			ret = KonfirmasiAbsensi();
			break;
#endif
		case trtPembayaranPLNPascaInquiry:
			ret = KonfirmasiPembayaranPLN();
			break;

		case trtPembayaranPLNPraInquiry:
			ret = KonfirmasiPembayaranPLNPra();
			break;

		case trtReissuePINOtorisasi:
		case trtAktivasiKartuOtorisasi:
		case trtRenewalKartuOtorisasi:
			ret = 9999;
			break;

		case trtAktivasiKartuInquiry:
		case trtRenewalKartuInquiry:
			ret = KonfirmasiAktivasiKartu();
			break;

		case trtPembayaranPascaBayarInquiry:
			ret = KonfirmasiPembayaranPascaBayar8();
			break;

#ifdef PREPAID
		case trtPrepaidRedeemInquiry:
			ret = KonfirmasiPrepaidRedeem();
			break;

		//++ @agmr - brizzi2
		case trtPrepaidReaktivasiInquiry:
			ret = KonfirmasiPrepaidReaktivasi();
			break;

		case trtPrepaidVoidInquiry:
			ret = KonfirmasiPrepaidVoid();
			break;

		//-- @agmr - brizzi2
#endif
		default:
			ret = 0;
			break;

	}

	if(ret == 0)
		goto lblEnd;

	MAPGETBYTE(traNextTxn, nextTxn, lblKO);
	if(nextTxn == 0)
	{
		ret = 0;
		goto lblEnd;
	}
	MAPPUTBYTE(traTxnType, nextTxn, lblKO);
	ret = 1;
	goto lblEnd;

	lblKO:
		ret =  -1;
	lblEnd:
		return ret;
}

int isBRIInquiryTransaction()
{
	byte 	txnType;
	int 		ret;

	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch(txnType)
	{
		case trtTransferSesamaBRIInquiry:
		case trtTransferAntarBankInquiry:
		case trtPembayaranPLNPascaInquiry:
		case trtPembayaranPLNPraInquiry:
		case trtPembayaranCicilanInquiry:
		case trtSetorPinjamanInquiry:
		case trtSetorSimpananInquiry:
		case trtPembayaranPascaBayarInquiry:
		case trtPembayaranTVBerlanggananInquiry:
		case trtPembayaranTiketPesawatInquiry:
		case trtPembayaranPdamInquiry:
		case trtPembayaranTelkomInquiry:
#ifdef SSB
		case trtPembayaranSSBInquiry:
#endif
		case trtPembayaranBrivaInquiry:
		case trtTrxPhoneBankingTransferInquiry:
		case trtPembayaranKKBRIInquiry:
		case trtPembayaranSPPInquiry:
#ifdef PREPAID
		case trtPrepaidRedeemInquiry:
		//++ @agmr - brizzi2
		case trtPrepaidReaktivasiInquiry:
		case trtPrepaidVoidInquiry:
		//-- @agmr - brizzi2
#endif
			return 1;

		default:
			return 0;
	}

	lblKO:
		return 0;
}

int inputTCash()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"T-BANK");
	dspLS(1,"TOKEN :");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaPengirim,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"T-BANK");
	dspLS(1,"NO TELP:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaPenerima, buf, lblKO);


	dspClear();
	dspLS(0|0x40,"T-BANK");
	dspLS(1,"Input Nominal:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenAmt+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);


	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPreAuth()
{
	int 	ret;
	char buf[20];
	byte txnType;
	card invNum; //@@REPAIR
	card retROC, crdenterROC;
	card dupROC;
	word logCount=0;
	int	 idxCnt, key = 0, mnu = 0;
	char enterROC[lenInvNum + 1];
	byte tempCardFlag;

	dspClear();
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch(txnType)
	{
		case trtPreCom:
			dspLS(0|0x40,"PRE COM");
			break;

		case trtPreCancel:
			dspLS(0|0x40,"PRE CANCEL");
			break;
	}
	dspLS(1,"Approval Code:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenTim+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traPasswordPetugas, buf, lblKO); //@@REPAIR
	MAPPUTSTR(traAutCod, buf, lblKO);
	MAPPUTBYTE(appBriziDiscFlag,1,lblKO);
	MAPPUTSTR(appBriziDiscValue, buf, lblKO);

	dspLS(1,"Trace Number:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenTim+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traPINInternetBanking, buf, lblKO); //@@REPAIR
	MAPGETCARD(regInvNum, dupROC, lblKO);
	MAPPUTCARD(regInvNumTemp, dupROC, lblKO);
	invNum = atoi(buf); //@@REPAIR
	MAPPUTCARD(regInvNum, invNum, lblKO); //@@REPAIR
	MAPGETWORD(regTrxNo, logCount, lblKO);

	dec2num(&crdenterROC, buf, 0);

	for (idxCnt = 0; idxCnt < logCount; idxCnt++)
	{
		mapMove(logBeg, (word) idxCnt);
		MAPGETCARD(logROC, retROC, lblKO);
		if(crdenterROC == retROC)
			goto lblDup;
	}

	MAPPUTBYTE(appTempStanFlag,1,lblKO);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblDup:
		ret = -1;
		usrInfo(infDuplicatePre);
		goto lblEnd;
	lblKO:
		return ret = -1;
	lblEnd:
		return ret;
}


int inputInfoSaldoBankLain()
{
	int ret;
	int key;

	while(1)
	{
		key = usrInfo(infInfoSaldoBankLain);
		if(key == '1')
		{
			MAPPUTBYTE(traJenisRekening,REK_TABUNGAN,lblKO);
			break;
		}
		else if(key == '2')
		{
			MAPPUTBYTE(traJenisRekening,REK_GIRO,lblKO);
			break;
		}
		else if(key == 0xff)
			goto lblKO;

		else if(key == 0)
			goto lblBatal;
	}

	return 1;

	lblBatal:
		ret = 0;
		goto lblEnd;

	lblKO:
		ret = -1;

	lblEnd:
		return ret;
}

int inputTransferSesamaBRI()
{
	int 	ret;
	char buf[20];
	byte Exp;

	dspClear();
	dspLS(0|0x40,"  Transfer BRI");
	dspLS(1,"Nomor Rekening");
	dspLS(2,"Tujuan:");

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(3, buf, lenNomorRekeningBRI+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= lenNomorRekeningBRI)
			break;
		Beep();
	}
	MAPPUTSTR(traNomorRekeningTujuanBRI,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"  Transfer BRI");
	dspLS(1,"Jumlah:      Rp.");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);

	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
	    return ret;
}

int inputTransferAntarBank()
{
	int 	ret;
	int 	key;
	char buf[20];
	byte Exp;
	tInfo inf;

	inf.cmd = 's';
	strcpy(inf.msg[0],"  Antar Bank");
	strcpy(inf.msg[1],"Rekening Asal");
	strcpy(inf.msg[2],"1. Tabungan");
	strcpy(inf.msg[3],"2. Giro");
	inf.dly = 30;

	while(1)
	{
		key = info(&inf);
		if(key == '1')
		{
			MAPPUTBYTE(traJenisRekening,REK_TABUNGAN,lblKO);
			break;
		}
		else if(key == '2')
		{
			MAPPUTBYTE(traJenisRekening,REK_GIRO,lblKO);
			break;
		}
		else if(key == 0xff)
			goto lblKO;
		else if(key == 0)
		{
			ret = 0;
			goto lblEnd;
		}
	}

	dspClear();
	dspLS(0,"Kode Bank dan Nomor");
	dspLS(1,"Rekening Tujuan:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, sizeof(buf));
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 10)
			break;
		Beep();
	}
	mapPut(traBankId,buf,3);
	MAPPUTSTR(traNomorRekeningTujuan,buf+3,lblKO);

	dspClear();
	dspLS(0|0x40,"  Antar Bank");
	dspLS(1,"Jumlah:      Rp.");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	dspClear();
	dspLS(0,"Enter Cust Ref Number");
	dspLS(1,"atau Lanjut:");
	dspLS(3,"BATAL     LANJUT");
	memset(buf,0,sizeof(buf));

	ret = enterPhr(2, buf, sizeof(buf));
	if(ret == kbdANN || ret == kbdF1)
		goto lblBatal;
	if(ret < 0)
		goto lblKO;

	MAPPUTSTR(traCustomerRefNumber,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranTV()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"TV BERLANGGANAN");
	dspLS(1,"No ID Pelanggan:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaPenerima,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputTiketPesawat()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"TIKET PESAWAT");
	dspLS(1,"ENTER KODE BAYAR:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaPenerima,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranPdam()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"PEMB PDAM");
	dspLS(1,"No ID Pelanggan:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaPenerima,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}


int inputPembayaranTelkom()
{
	int 	ret;
	char buf[20];
	char buf1[20];
	char buf2[20];

	dspClear();
	dspLS(0|0x40,"PEMB. TELKOM");
	dspLS(1,"KODE AREA + TELP:");

	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf2));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	if(strlen(buf) < 13){
		strncpy(buf1,buf, 3);
		if((strcmp(buf1,"021")) || (strcmp(buf1,"061")) || (strcmp(buf1,"022")) || (strcmp(buf1,"031"))){
			sprintf(buf2,"0%s",buf);
			MAPPUTSTR(traNamaPenerima,buf2,lblKO);
		}

	}
	else
   		MAPPUTSTR(traNamaPenerima,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranSSB()
{
	int 	ret;
	char buf[20];
	char buf1[20];
	char buf2[20];

	memset(buf,0,sizeof(buf));
	mapGet(traTotalOutstanding,buf,lenTotalOutstanding);
	if(
		(strcmp(buf, "81") == 0) ||
		(strcmp(buf, "82") == 0) ||
		(strcmp(buf, "91") == 0) ||
		(strcmp(buf, "92") == 0)

	)
	{
		dspClear();
		dspLS(0|0x40,"PEMB. SSB");
		dspLS(1,"JUMLAH:");

		memset(buf,0,sizeof(buf));
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf2));

		while(1)
		{
			ret = enterPhr(2, buf, lenBillStatus+1);
			if(ret == kbdANN || ret == 0)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(buf) >= 1)
				break;
			Beep();
		}

		MAPPUTSTR(traBillStatus,buf,lblKO);
	}

	dspClear();
	dspLS(0|0x40,"PEMB. SSB");
	dspLS(1,"KODE SATPAS:");

	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf2));

	while(1)
	{
		ret = enterPhr(2, buf, lenPaymentNumber+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traPaymentNumber,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranDPLKR()
{
	int 	ret;
	char buf[20], buf2[20],  buf3[20];
	byte Exp;

	dspClear();
	dspLS(0|0x40,"  DPLK");
	dspLS(1,"Enter NO DPLK:");
	memset(buf,0,sizeof(buf));
	memset(buf2,0,sizeof(buf2));
	memset(buf3,0,sizeof(buf3));

	while(1)
	{
		ret = enterPhr(2, buf, lenAdminCharge-1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAdminCharge,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"    DPLK");
	dspLS(1,"PEMBAYARAN:  Rp.");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);

	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if((atoi(buf)) < 20000){
			KonfirmasiMinInput();
			goto lblBatal;
		}
		if(buf[0] != '0' && buf[0] != 0)
			break;
		if(strlen(buf) >= 5)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
	ret = -1;
	lblEnd:
		return ret;
}

int inputRedeem()
{
	int 	ret;
	char buf[20];
	char buf1[20];
	char buf2[20];

	dspClear();
	dspLS(0|0x40,"SALE REDEMPTION");
	dspLS(1,"POINT REDEM:");

	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf2));

	strcpy(buf, "0");

	while(1)
	{
		ret = enterAmt(3, buf, 0);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 1)
			break;
		Beep();
	}

	MAPPUTSTR(traPowerConsuming,buf,lblKO);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranBriva(){
	int 	ret;
	char buf[20];
	char buf1[20];
	char buf2[20];

	dspClear();
	dspLS(0|0x40,"PEMB. BRIVA");
	dspLS(1,"NO BRIVA:");

	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf2));

	while(1)
	{
		ret = enterPhr(2, buf, lenNomorRekening+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 6)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaBankAsal,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputSmsBanking()
{

	int 	ret;
	char buf[20], buf_2[20];
	char key;

	dspClear();
	dspLS(0|0x40,"  SMS BANKING");
	dspLS(1,"No Telepon:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNoTelpon,buf,lblKO);


	dspClear();
	dspLS(0|0x40,"  SMS BANKING");
	dspLS(1,"PIN  :");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPwd(3,buf,7);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traPINInternetBanking,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"  SMS BANKING");
	dspLS(1,"Konfirmasi PIN  :");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPwd(3,buf,7);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPGET(traPINInternetBanking,buf_2,lblKO);
	if(strcmp(buf_2, buf) == 0){
		MAPPUTSTR(traPINInternetBanking,buf,lblKO);
	}else{
		dspClear();
		dspLS(0|0x40,"  SMS BANKING");
		dspLS(1,"  PIN  Salah");
		key = acceptable_key(kbdVAL_kbdANN,60);
		goto lblBatal;
	}

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;

}

int inputTrxTransfer()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"  REG TRANSFER");
	dspLS(1,"Input No Rek:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenLWBP+4);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traLWBP,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputTrxPulsa()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"  REG PULSA");
	dspLS(1,"Input No HP:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenPelangganId+4);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traNoTelpon,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranKKBRI()
{
	int ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"  PEMB. KK BRI");
	dspLS(1,"Input No KK:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenMeter+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 16)
			break;
		Beep();
	}

	MAPPUTSTR(traMeter,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranKKANZ()
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"PEMB. KK ANZ");
	dspLS(1,"Input No KK:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenMeter+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traMeter,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"PEMB. KK ANZ");
	dspLS(1,"Input Jumlah:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenAmt+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranKKCITI()
{
	int 	ret;
	char buf[20];
	byte txnType;

	MAPGETBYTE(traTxnType, txnType, lblKO);
	dspClear();
	if(txnType == trtPembayaranKKCITIKK){
		dspLS(0|0x40,"PEMB. KK CITI");
		dspLS(1,"Input No KK:");
	}else{
		dspLS(0|0x40,"PEMB. KTA CITI");
		dspLS(1,"Input No KTA:");
	}

	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenMeter+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traMeter,buf,lblKO);

	dspClear();
	if(txnType == trtPembayaranKKCITIKK){
		dspLS(0|0x40,"PEMB. KK CITI");
		dspLS(1,"Input Jumlah:");
	}else{
		dspLS(0|0x40,"PEMB. KTA CITI");
		dspLS(1,"Input Jumlah:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenAmt+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranKKHSBC()
{
	int 		ret;
	char 	buf[20];
	byte 	txnType;

	MAPGETBYTE(traTxnType, txnType, lblKO);
	dspClear();
	if(txnType == trtPembayaranKKHSBCKK){
		dspLS(0|0x40,"PEMB. KK HSBC");
		dspLS(1,"Input No KK:");
	}else{
		dspLS(0|0x40,"PEMB. KTA HSBC");
		dspLS(1,"Input No KTA:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenMeter+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traMeter,buf,lblKO);

	dspClear();
	if(txnType == trtPembayaranKKHSBCKK){
		dspLS(0|0x40,"PEMB. KK HSBC");
		dspLS(1,"Input Jumlah:");
	}else{
		dspLS(0|0x40,"PEMB. KTA HSBC");
		dspLS(1,"Input Jumlah:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenAmt+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranKKRBS()
{
	int 		ret;
	char 	buf[20];
	byte 	txnType;

	MAPGETBYTE(traTxnType, txnType, lblKO);
	dspClear();
	if(txnType == trtPembayaranKKRBSKK){
		dspLS(0|0x40,"PEMB. KK RBS");
		dspLS(1,"Input No KK:");
	}else{
		dspLS(0|0x40,"PEMB. KTA RBS");
		dspLS(1,"Input No KTA:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenPhone+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traMeter,buf,lblKO);

	dspClear();
	if(txnType == trtPembayaranKKRBSKK){
		dspLS(0|0x40,"PEMB. KK RBS");
		dspLS(1,"Input Jumlah:");
	}else{
		dspLS(0|0x40,"PEMB. KTA RBS");
		dspLS(1,"Input Jumlah:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenAmt+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranKKStanChar()
{
	int 		ret;
	char 	buf[20];
	byte 	txnType;

	MAPGETBYTE(traTxnType, txnType, lblKO);
	dspClear();
	if(txnType == trtPembayaranKKStanCharKK){
		dspLS(0|0x40,"PEMB. KK SCB");
		dspLS(1,"Input No KK:");
	}else{
		dspLS(0|0x40,"PEMB. KTA SCB");
		dspLS(1,"Input No KTA:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenMeter+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traMeter,buf,lblKO);

	dspClear();
	if(txnType == trtPembayaranKKStanCharKK){
		dspLS(0|0x40,"PEMB. KK SCB");
		dspLS(1,"Input Jumlah:");
	}else{
		dspLS(0|0x40,"PEMB. KTA SCB");
		dspLS(1,"Input Jumlah:");
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenAmt+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranSPP()
{
	int 		ret;
	char 	buf[20];

	dspClear();
	dspLS(0|0x40,"BAYAR SPP");
	dspLS(1,"Kode Univ:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenKodeWilayah+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 3)
			break;
		Beep();
	}

	MAPPUTSTR(traKodeWilayah,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"BAYAR SPP");
	dspLS(1,"No MHS/Billing:");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenNamaPelangganPLN+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 20)
			break;
		Beep();
	}

	MAPPUTSTR(traNamaPelangganPLN,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranPLN()
{
	int 		ret;
	char 	buf[20];

	dspClear();
	dspLS(0|0x40,"  PLN");
	dspLS(1,"No ID Pelanggan:");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenPelangganId+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= (lenPelangganId))
			break;
		Beep();
	}

	MAPPUTSTR(traPelangganId,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranPLNToken()
{
	int 		ret;
	char 	buf[20];

	dspClear();
	dspLS(0|0x40,"  PLN");
	dspLS(1,"NO METERAN : ");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenPelangganId);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}
	MAPPUTSTR(traPelangganId,buf,lblKO);


	dspClear();
	dspLS(0|0x40,"     PLN");
	dspLS(1,"NO REF : ");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenTotalAmount+1);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		if(strlen(buf) >= 12)
			break;
		Beep();
	}
	MAPPUTSTR(traTotalAmount,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranPLNPra()
{
	int 		ret;
	char 	buf[20], buf2[20],  buf3[20];
	byte 	Exp;

	dspClear();
	dspLS(0|0x40,"  PLN");
	dspLS(1,"No Meter/ID Pel:");
	memset(buf,0,sizeof(buf));
	memset(buf2,0,sizeof(buf2));
	memset(buf3,0,sizeof(buf3));
	while(1)
	{
		ret = enterPhr(2, buf, lenPelangganId+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;
		Beep();
	}
	if(strlen(buf) < 12){
		fmtPad(buf3, lenPelangganId -strlen(buf) , '0');
		sprintf(buf2, "%s%s",buf3, buf);
		MAPPUTSTR(traPelangganId,buf2,lblKO);
	}else
		MAPPUTSTR(traPelangganId,buf,lblKO);

	dspClear();
	dspLS(0|0x40,"     PLN");
	dspLS(1,"PEMBAYARAN:  Rp.");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if((atoi(buf)) < 20000){
			KonfirmasiMinInput();
			goto lblBatal;
		}
		if(buf[0] != '0' && buf[0] != 0)
			break;
		if(strlen(buf) >= 5)
			break;
		Beep();
	}
	MAPPUTSTR(traTotAmt,buf,lblKO);
	MAPPUTSTR(traAmt,buf,lblKO);
	MAPPUTSTR(traBuffer1,buf,lblKO);

	mapGet(traBuffer1,buf,lenBuffer1);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int KonfirmasiMinInput()
{

	int 		ret;
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PLN PREPAID");

	strcpy(msg[i++]," ");

	strcpy(msg[i++],"  Min Nominal Pemb.");
	strcpy(msg[i++],"     Rp. 20.000      ");

	ret = scrollDisplay8(msg,i,6,judul,"                 OK");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}


int inputPembayaranCicilan()
{
	int 		ret, minBufLen =4, maxBufLen =16 ;
	char 	buf[20];
	char 	buf1[30];
	byte 	subType;

	MAPGETBYTE(traSubType,subType,lblKO);
	switch(subType)
	{
		case CICILAN_FIF:
			strcpy(buf1,"Pembayaran FIF");
			break;
		case CICILAN_BAF:
			strcpy(buf1,"Pembayaran BAF");
			break;
		case CICILAN_OTO:
			strcpy(buf1,"Pembayaran OTO");
			break;
		case CICILAN_FNS:
			strcpy(buf1,"Pemb Finansia");
			break;
		case CICILAN_VER:
			strcpy(buf1,"Pemb Verena");
			break;
		case CICILAN_WOM:
			strcpy(buf1,"Pembayaran WOM");
			break;
	}


	dspClear();
	dspLS(0|0x40,buf1);
	dspLS(1,"No Kontrak:");
	memset(buf,0,sizeof(buf));
	while(strlen(buf) <minBufLen)
	{
		ret = enterPhr(2, buf, maxBufLen+1);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret == kbdVAL && strlen(buf) < minBufLen)
			Beep();
	}
	if(ret < 0)
		goto lblKO;
	MAPPUTSTR(traContractNumber,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranPulsa()
{
	int 		ret;
	char 	buf[20];
	char 	buf1[30];
	byte 	subType;

	dspClear();

	memset(buf1,0,sizeof(buf1));
	MAPGETBYTE(traSubType,subType,lblKO);
	switch(subType/10)
	{
		case 1:
			strcpy(buf1,"Nomor SIMPATI:");
			break;
		case 2:
			strcpy(buf1,"Nomor MENTARI:");
			break;
		case 3:
			strcpy(buf1,"Nomor IM3:");
			break;
		case 4:
			strcpy(buf1,"Nomor XL:");
			break;
		case 5:
			strcpy(buf1, "Nomor Esia");
			dspLS(0|0x40,"  Kode Daerah");
			dspLS(1,buf1);
			memset(buf,0,sizeof(buf));
			while (1)
			{
				ret = enterPhr(2, buf, lenBillPeriod+1);
				if(ret == kbdANN)
					goto lblBatal;
				if(ret < 0)
					goto lblKO;
				if(strlen(buf) >= 3)
					break;
				Beep();
			}
			MAPPUTSTR(traBillPeriod1,buf,lblKO);
			break;
		case 6:
			strcpy(buf1, "Nomor SMART:");
			break;
		case 7:
			strcpy(buf1, "Nomor FREN:");
			break;
		case 8:
			strcpy(buf1, "Nomor THREE:");
			break;
		case 9:
			strcpy(buf1, "Nomor AXIS:");
			break;
	}

	dspLS(0|0x40,"  Isi Ulang");
	dspLS(1,buf1);
	memset(buf,0,sizeof(buf));
	if((subType/10) == 2)
	{
		while (1)
		{
			ret = enterPhr(2, buf, lenNoTelpon);
			if(ret == kbdANN)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(buf) >= 8)
				break;
			Beep();
		}
	}
	else
	{
		while (1)
		{
			ret = enterPhr(2, buf, lenNoTelpon +1);
			if(ret == kbdANN)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(buf) >= 8)
				break;
			Beep();
		}
	}
	MAPPUTBYTE(traJenisKartu, strlen(buf), lblKO);
	MAPPUTSTR(traNoTelpon,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputSetoran(byte txnType)
{
	int 		ret;
	char 	buf[20];
	byte 	Exp;

	dspClear();
	if(txnType == trtSetorPinjamanInquiry)
		dspLS(0|0x40,"Setoran Pinjaman");
	else
		dspLS(0|0x40,"Setoran Simpanan");

	dspLS(1,"No. Rekening:");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenNomorRekeningBRI+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= lenNomorRekeningBRI)
			break;
		Beep();
	}
	MAPPUTSTR(traNomorRekeningAsalBRI,buf,lblKO);

	dspClear();
	if(txnType == trtSetorPinjamanInquiry)
		dspLS(0|0x40,"Setoran Pinjaman");
	else
		dspLS(0|0x40,"Setoran Simpanan");
	dspLS(1,"Amount:");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputTarikTunai()
{
	int 		ret;
	char 	buf[20];
	byte 	Exp;

	dspClear();

	dspLS(0|0x40," Tarik Tunai");
	dspLS(1,"Jumlah:      Rp.");
	dspLS(3,"BATAL     LANJUT");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN || ret == kbdF1)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

    MAPPUTSTR(traAmt,buf,lblKO);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranZakat()
{
	int 		ret;
	char 	buf[20];
	byte 	Exp;
	byte 	subType;

	MAPGETBYTE(traSubType,subType,lblKO);

	dspClear();
	if(subType == ZAKAT_DHUAFA || subType == ZAKAT_YBM)
		dspLS(0|0x40," ZAKAT");
	else if(subType == INFAQ_DHUAFA || subType == INFAQ_YBM)
		dspLS(0|0x40," INFAQ");
	else if(subType == DPLK_DHUAFA || subType == DPLK_YBM)
		dspLS(0|0x40," DPLK");

	dspClear();
	dspLS(1,"Jumlah:      Rp.");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputSetorPasti()
{
	int 		ret;
	char 	buf[20];
	byte 	Exp;

	dspClear();

	dspLS(0|0x40," SETOR PASTI");
	dspLS(1,"AMOUNT:      Rp.");
	dspLS(3,"BATAL     LANJUT");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN || ret == kbdF1)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

    MAPPUTSTR(traAmt,buf,lblKO);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputInstallment()
{
	int 		ret;
	char 	buf[20];
	byte 	Exp;
	byte 	len;
	char 	tmp[5];
	card 	term;

	dspClear();
	dspLS(0|0x40,"CICILAN BRING");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		strcpy(buf,"Term (3-24):");
		len = strlen(buf);
		ret = enterPhr(3, buf, 3+len);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;

		strcpy(tmp,&buf[len]);
		dec2num(&term,tmp,2);
		if( 0 == term % 3 && term >=3 && term <= 24)
			break;

		memset(buf,0,sizeof(buf));
		Beep();
	}

	MAPPUTSTR(traInstallmentTerm,tmp,lblKO);

	dspClear();
	dspLS(0|0x40,"CICILAN BRING");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		strcpy(buf,"Plan (1-3):");
		len = strlen(buf);
		ret = enterPhr(3, buf, 2+len);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;

		strcpy(tmp,&buf[len]);
		dec2num(&term,tmp,2);
		if(term < 1 || term > 3)
		{
			memset(buf,0,sizeof(buf));
			Beep();
			continue;
		}else
			break;
	}

    MAPPUTSTR(traInstallmentPlan,tmp,lblKO);

	dspClear();
	dspLS(0|0x40,"CICILAN BRING");
	dspLS(2,"Amount:      Rp.");

	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(3,buf,Exp);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(buf[0] != '0' && buf[0] != 0)
			break;
		Beep();
	}

	MAPPUTSTR(traAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputPembayaranPascaBayar()
{
	int 		ret;
	char 	buf[20];
	word 	MnuItm;

	MAPGETWORD(traMnuItm, MnuItm, lblKO);
	dspClear();
	switch(MnuItm)
	{
		case mnuPembayaranHalo:
			dspLS(0|0x40,"  HALO");
			dspLS(1,"No Kartu HALO:");
			break;
		case mnuPembayaranMatrix:
			dspLS(0|0x40,"  MATRIX");
			dspLS(1,"No Matrix:");
			break;
	}

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenNoTelpon+1);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 8)
			break;
		Beep();
	}
	MAPPUTSTR(traNoTelpon,buf,lblKO);
	ret = 1;
	goto lblEnd;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

#ifdef PREPAID
int inputPrepaidDeposit()
{
	int 		ret;
	char 	buf[25];

	dspClear();

	dspLS(0|0x40,"TOPUP DEPOSIT");
	dspLS(1,"No Kartu Brizzi :");
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, NOMOR_KARTU_BRIZZI_LEN+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= NOMOR_KARTU_BRIZZI_LEN)
			break;
		Beep();
	}
	MAPPUTSTR(TRA_NOMOR_KARTU_BRIZZI,buf,lblKO);
	if(ret < 0 )
		goto lblKO;
	goto lblEnd;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

#endif


int KonfirmasiTransferSesamaBRI()
{
	int 		ret;
	char 	namaPenerima[31];
	char 	rekeningTujuan[20];
	char 	jumlah[20];
	byte 	key;
	char 	amount[20];

	dspClear();

	memset(rekeningTujuan,0,sizeof(rekeningTujuan));
	mapGet(traNomorRekeningTujuanBRI,rekeningTujuan,sizeof(rekeningTujuan));

	memset(namaPenerima,0,sizeof(namaPenerima));
	mapGet(traNamaPenerima,namaPenerima,sizeof(namaPenerima));

	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(jumlah,0,sizeof(jumlah));
	strcpy(jumlah,"Rp."); //@@OA
	fmtAmt(jumlah+3, amount, 0, ",.");
	dspLS(0,rekeningTujuan);
	dspLS(1,namaPenerima);
	dspLS(2,jumlah);
	dspLS(3,"CONFIRM YES / NO");

	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdANN:
				goto lblBatal;
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}


int KonfirmasiTransferAntarBank()
{
	int 		ret;
	char 	namaPenerima[31];
	char 	namaBankTujuan[20];
	char 	rekeningTujuan[20];
	char 	namaBankAsal[20];
	char 	refNumber[20];
	char 	jumlah[20];
	char 	msg[12][21];
	char 	amount[20];

	dspClear();

	memset(namaBankTujuan,0,sizeof(namaBankTujuan));
	mapGet(traNamaBankTujuan,namaBankTujuan,sizeof(namaBankTujuan));

	memset(namaBankAsal,0,sizeof(namaBankAsal));
	mapGet(traNamaBankAsal,namaBankAsal,sizeof(namaBankAsal));

	memset(refNumber,0,sizeof(refNumber));
	mapGet(traCustomerRefNumber,refNumber,sizeof(refNumber));

	memset(rekeningTujuan,0,sizeof(rekeningTujuan));
	mapGet(traNomorRekeningTujuan,rekeningTujuan,sizeof(rekeningTujuan));

	memset(namaPenerima,0,sizeof(namaPenerima));
	mapGet(traNamaPenerima,namaPenerima,sizeof(namaPenerima));
	if(strlen(namaPenerima) > 16)
		namaPenerima[16] = 0;

	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(jumlah,0,sizeof(jumlah));
	fmtAmt(jumlah, amount, 0, ",.");
	memset(msg,0,sizeof(msg));

	strcpy(msg[0],"Bank Asal :");
	memcpy(msg[1],namaBankAsal,20);
	strcpy(msg[2],"Bank Tujuan :");
	memcpy(msg[3],namaBankTujuan,20);
	strcpy(msg[4],"No. Rekening :");
	memcpy(msg[5],rekeningTujuan,20);
	strcpy(msg[6],"Nama Nasabah :");
	memcpy(msg[7],namaPenerima,20);
	strcpy(msg[8],"Cust. Ref :");
	memcpy(msg[9],refNumber,20);
	strcpy(msg[10],"Jumlah :");
	memcpy(msg[11],jumlah,20);


	ret = scrollDisplay(msg,12,2,"   ANTAR BANK","BATAL     LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblEnd:
		return ret;

}

int KonfirmasiPembayaranPulsa()
{
	int 		ret;
	char 	buf1[30];
	char 	nomor[20], kode_area[20];
	char 	jumlah[20], amount[20];
	byte 	key;
	int		count = 0;
	byte 	subType;

	dspClear();

	MAPGETBYTE(traSubType,subType,lblKO);
	switch(subType/10)
	{
		case 1:
			strcpy(buf1,"Nomor SIMPATI:");
			break;
		case 2:
			strcpy(buf1,"Nomor MENTARI:");
			break;
		case 3:
			strcpy(buf1,"Nomor IM3:");
			break;
		case 4:
			strcpy(buf1,"Nomor XL:");
			break;
		case 5:
			strcpy(buf1, "Nomor ESIA");
			mapGet(traBillPeriod1, kode_area, sizeof(kode_area));
			count =1;
			break;
		case 6:
			strcpy(buf1, "Nomor SMART:");
			break;
		case 7:
			strcpy(buf1, "Nomor FREN:");
			break;
		case 8:
			strcpy(buf1, "Nomor THREE:");
			break;
		case 9:
			strcpy(buf1, "Nomor AXIS:");
			break;
    }

	memset(nomor,0,sizeof(nomor));
	mapGet(traNoTelpon,nomor,sizeof(nomor));
	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(jumlah,0,sizeof(jumlah));
	strcpy(jumlah,"Rp.");
	fmtAmt(jumlah+3, amount, 0, ",.");

	dspLS(0,buf1);
	dspLS(1,nomor);
	if(count)
		dspLS(1,strcat(kode_area, nomor));
	else
		dspLS(1,nomor);
	dspLS(2,jumlah);
	dspLS(3,"CONFIRM YES / NO");

	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdANN:
				goto lblBatal;
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}

	return 1;
	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblKO:
		ret = -1;

	lblEnd:
		kbdStop();
		return ret;

}


int KonfirmasiPembayaranCicilan8()
{
    int 		ret;
    char 		str[128];
    char 		buf[128];
    char 		judul[30];
    char 		msg[10][22];
    int 		i=0;

	strcpy(judul,"CICILAN");

	//No kontrak
	strcpy(msg[i++],"No Kontrak :");

	memset(msg,0,sizeof(msg));
	memset(buf,0,sizeof(buf));
	mapGet(traContractNumber,buf,sizeof(buf));
	sprintf(str,"%-16s",buf);
	strcpy(msg[i++],str);

	//Nama
	strcpy(msg[i++],"Nama :");

	memset(buf,0,sizeof(buf));
	mapGet(traCustomerName,buf,sizeof(buf));
	sprintf(str,"%-16s",buf);
	strcpy(msg[i++],str);

	//Jatuh tempo
	strcpy(msg[i++],"Jatuh Tempo :");

	memset(buf,0,sizeof(buf));
	memset(str,0,sizeof(str));
	mapGet(traDueDate,buf,sizeof(buf));
	memcpy(str,buf,2);
	strcat(str,"-");
	memcpy(str+3,buf+2,2);
	strcat(str,"-");
	strcat(str,buf+4);
	sprintf(buf,"%-16s",str);
	strcpy(msg[i++],buf);

	//Angsuran
	strcpy(msg[i++],"Angsuran ke :");

	memset(buf,0,sizeof(buf));
	mapGet(traPaymentNumber,buf,sizeof(buf));
	strcat(msg[i++],buf);

	//Tagihan
	strcpy(msg[i++],"Tagihan :");

	memset(buf,0,sizeof(buf));
	mapGet(traTotalAmount,buf,sizeof(buf));
	buf[strlen(buf) - 2] = 0;
	mapPut(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcpy(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiInfoSaldo8()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[10][22];
	int 		i=0;

	strcpy(judul,"INFORMASI SALDO");

	memset(msg,0,sizeof(msg));
	//Nama
	strcpy(msg[i++],"Nama :");

	memset(buf,0,sizeof(buf));
	mapGet(traNamaPengirim,buf,sizeof(buf));
	sprintf(str,"%-16s",buf);
	strcpy(msg[i++],str);

	//Tagihan
	strcpy(msg[i++],"Saldo :");

	memset(buf,0,sizeof(buf));
	mapGet(traSisaSaldo,buf,sizeof(buf));
	buf[strlen(buf) - 2] = 0;
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcpy(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"PRINT?      YES/NO");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiInfoSaldoLain8()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[10][22];
	byte 	jenis;
	int 		i=0;

	strcpy(judul,"INFORMASI SALDO");

	memset(msg,0,sizeof(msg));
	//Nama
	strcpy(msg[i++],"Nama/Bank :");

	memset(buf,0,sizeof(buf));
	mapGet(traNamaBankAsal,buf,sizeof(buf));
	sprintf(str,"%-16s",buf);
	strcpy(msg[i++],str);

	// Jenis Rekening
	strcpy(msg[i++],"Jenis Rek :");
	MAPGETBYTE(traJenisRekening,jenis,lblKO);
	if(jenis == REK_TABUNGAN)
		strcpy(msg[i++],"TABUNGAN");
	else
		strcpy(msg[i++],"GIRO");

	//Tagihan
	strcpy(msg[i++],"Saldo :");

	memset(buf,0,sizeof(buf));
	mapGet(traSisaSaldo,buf,sizeof(buf));
	buf[strlen(buf) - 2] = 0;
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcpy(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"PRINT?      YES/NO");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		goto lblEnd;

	lblKO:
		ret = -1;

	lblEnd:
		kbdStop();

	return ret;
}


int KonfirmasiSetoran(byte txnType)
{
	int 		ret;
	char 	buf[30];
	char 	amount[20];
	char 	msg[10][22];
	char 	judul[25];
	int 		i = 0;

	memset(msg,0,sizeof(msg));

	dspClear();

	if(txnType == trtSetorPinjamanInquiry)
		strcpy(judul,"Setoran Pinjaman");
	else
		strcpy(judul,"Setoran Simpanan");
	memset(msg,0,sizeof(msg));

	//Nama
	strcpy(msg[i++], "Nama:");
	memset(buf,0,sizeof(buf));
	mapGet(traNamaPenerima,buf,sizeof(buf));
	strcpy(msg[i++],buf);

	strcpy(msg[i++], "No Rekening:");
	memset(buf,0,sizeof(buf));
	mapGet(traNomorRekeningAsalBRI,buf,sizeof(buf));
	strcpy(msg[i++],buf);

	strcpy(msg[i++], "Jumlah Setor:");
	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(msg[i++],buf);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");

	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;

	lblEnd:
		return ret;

}

int KonfirmasiTarikTunai()
{
	int 		ret;
	char 	key;
	char 	buf[30];
	char 	amount[20];
	card 	Amt;

	dspClear();

	dspLS(0|0x40,"  Tarik Tunai");
	dspLS(1,"Konfirmasi");

	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	dec2num(&Amt, amount, strlen(amount));
	memset(amount,0,sizeof(amount));
	Amt = Amt/100;
	num2dec(amount,Amt,0);
	memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");

	dspLS(2,buf);
	dspLS(3,"BATAL   LANJUT");

	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;

	lblEnd:
		return ret;

}

int KonfirmasiVoidTarikTunai()
{
	int 		ret;
	char 	buf[50];
	char 	amount[20];
	char 	msg[4][22];
	char 	judul[25];

	memset(msg,0,sizeof(msg));

	dspClear();

	strcpy(judul,"Void Tarik Tunai");

	//No Acc.
	memset(buf,0,sizeof(buf));
	mapGet(traPan,buf,sizeof(buf));
	strcpy(msg[1],buf);

	//Nama
	memset(buf,0,sizeof(buf));
	mapGet(traTrk1,buf,sizeof(buf));
	memcpy(msg[2],buf,20);

	//Trace number
	memset(buf,0,sizeof(buf));
	memset(amount,0,sizeof(buf));
	strcpy(buf,"Trc : ");
	mapGet(traFindRoc,amount,sizeof(amount));
	fmtPad(amount,-6,'0');
	sprintf(buf,"Trc: %s",amount);
	strcpy(msg[3],buf);

	//Amount
	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");
	strcpy(msg[4],buf);

	ret = scrollDisplay8(msg,5,6,judul,"BATAL     LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		goto lblEnd;

	lblEnd:
		return ret;

}

int KonfirmasiPembayaranZakat()
{
	int 		ret;
	char 	key;
	char 	buf[30];
	char 	amount[20];
	char 	jenis[10];
	char 	pengelola[20];
	byte 	subType = 0;

	MAPGETBYTE(traSubType,subType,lblKO);

	dspClear();
	switch(subType)
	{
		case ZAKAT_DHUAFA:
			strcpy(jenis,"ZAKAT");
			strcpy(pengelola,"Dompet Dhuafa");
			break;
		case ZAKAT_YBM:
			strcpy(jenis,"ZAKAT");
			strcpy(pengelola,"YBM");
			break;
		case INFAQ_DHUAFA:
			strcpy(jenis,"INFAQ");
			strcpy(pengelola,"Dompet Dhuafa");
			break;
		case INFAQ_YBM:
			strcpy(jenis,"INFAQ");
			strcpy(pengelola,"YBM");
			break;
		case DPLK_DHUAFA:
			strcpy(jenis,"DPLK");
			strcpy(pengelola,"Dompet Dhuafa");
			break;
		case DPLK_YBM:
			strcpy(jenis,"DPLK");
			strcpy(pengelola,"YBM");
			break;
	}


	dspLS(0,jenis);
	dspLS(1,pengelola);
	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");

	dspLS(2,buf);
	dspLS(3,"DATA BENAR?");

	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
	kbdStop();
	return ret;
}

int KonfirmasiSetorPasti()
{
	int 		ret;
	char 	key;
	char 	buf[30];
	char 	amount[20];

	dspLS(0,"SETOR PASTI");
	memset(amount,0,sizeof(amount));
	mapGet(traAmt,amount,sizeof(amount));
	memset(buf,0,sizeof(buf));
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4, amount, 0, ",.");

	dspLS(1,buf);
	dspLS(2,"DATA BENAR?");

	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblEnd:
	kbdStop();
	return ret;
}

int KonfirmasiPembayaranSPP()
{

	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB. SPP");

	//NO. MHS
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NO. MHS: ");
	mapGet(traNomorRekeningTujuan,buf,lenNomorRekening);
	trimRight(buf,0x20);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);

	//NAMA MHS
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NAMA MHS:");
	mapGet(traCustomerName,buf,lenNomorRekening);
	sprintf(str,"%18s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//JENIS PEMB
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"JENIS PEMB:");
	mapGet(traNoVoucherMitra,buf,lenNomorRekening);
	sprintf(str,"%18s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//JUMLAH
	strcpy(msg[i++],"JUMLAH:");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL		   LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranTV()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"TV BERLANGGANAN");

	//NAMA
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NAMA:");
	mapGet(traNamaPengirim,buf,lenNamaPemilik);
	trimRight(buf,0x20);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);

	//PERIODE
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"PERIODE:");
	mapGet(traLWBP,buf,lenLWBP);
	sprintf(str,"%18s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");


	//AMOUNT
	strcpy(msg[i++],"TAGIHAN :");
	memset(buf,0,sizeof(buf));
	mapGet(traTotalAmount,buf,lenInquiryAmt);
	//buf[strlen(buf) - 2] = 0;
	//mapPut(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranTiketPesawat()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0, tempJml=0, x=0;
	char 	buf23[150];

	memset(msg,0,sizeof(msg));
	strcpy(judul,"TIKET PESAWAT");

	//Kode Bayar
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Kode Bayar:");
	mapGet(traNoTelpon,buf,lenNoTelpon);
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//kode booking
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Kode Booking:");
	mapGet(traNamaBankAsal,buf,lenNamaBank);
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//nama
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Nama     :");
	mapGet(traBuffer3,buf,lenBuffer3);
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);

	//Jml Flighet
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Jml Flight:");
	mapGet(traBillStatus,buf,lenBillStatus);
	tempJml = atoi(buf);
	sprintf(str,"%s",buf);
	strcat(msg[i++],str);

	//jml Bayar
	strcpy(msg[i++],"Jml Bayar :");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenInquiryAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);

	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"No Penerbangan  :");
	strcpy(msg[i++],"");

	for(x=0; x<tempJml; x++)
	{
		memset(buf23, 0, sizeof(buf23));
		memset(buf,0,sizeof(buf));
		strcpy(buf, tempTiket[x]);

		strncpy(buf23,      buf+2, 2);
		strncpy(buf23+2,  " ", 1);
		strncpy(buf23+3,  buf+11, 4);
		strncpy(buf23+7,  " ", 1);
		strncpy(buf23+8,  buf+4, 1);
		strncpy(buf23+9,  " ", 1);
		strncpy(buf23+10, buf+5 , 3);
		strncpy(buf23+13, "-", 1);
		strncpy(buf23+14, buf+8, 3);
		strncpy(buf23+17, " ", 1);
		strncpy(buf23+18, buf+15,2);
		strncpy(buf23+20, "/", 1);
		strncpy(buf23+21, buf+17,2);
		strncpy(buf23+23, " ", 1);
		strncpy(buf23+24, buf+19,2);
		strncpy(buf23+26, ":", 1);
		strncpy(buf23+27, buf+20,2);
		strncpy(buf23+29, ":", 1);
		strncpy(buf23+30, buf+1,1);

		sprintf(str,"%s", buf23);
		strcpy(msg[i++], str);
	}

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}


int KonfirmasiPembayaranPdam()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB PDAM");

	//NO Pelanggan
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"No Plgn  :");
	mapGet(traNamaPelangganPLN,buf,lenNamaPelangganPLN);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//Nama
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Nama Plgn:");
	mapGet(traCustomerName,buf,lenCustomerName);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//Jumlah Pakai
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Jumlah Pakai   :");
	mapGet(traBuffer4,buf,lenBuffer4);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//Priode bayar
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Periode Bayar :");
	mapGet(traJenisTransfer,buf,lenJenisTransfer);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//Kode Askoper
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Kode Askoper :");
	mapGet(traPokokHutang,buf,lenPokokHutang);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//no reff
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"No Ref      :");
	mapGet(traRrn,buf,lenRrn);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");


	//AMOUNT
	memset(buf,0,sizeof(buf));
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"TAGIHAN :Rp.");
	fmtAmt(str+12,buf,0,",.");
	strcat(msg[i++],str);


	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}


int KonfirmasiPembayaranTelkom()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	buf1[20];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB. TELKOM");

	//NO TELP
	memset(buf,0,sizeof(buf));
	memset(buf,0,sizeof(buf1));
	MAPGET(traPaymentNumber, buf1, lblKO);
	MAPGET(traAdminCharge, buf, lblKO);
	sprintf(str,"NO      : 0%d%d",atoi(buf1),atoi(buf));
	strcpy(msg[i++],str);

	//NAMA
	memset(buf,0,sizeof(buf));
	mapGet(traNamaPengirim,buf,lenNamaPemilik);
	sprintf(str,"NAMA    : %s",buf);
	strcpy(msg[i++],str);
	sprintf(str,"     %s",buf+11);
	strcpy(msg[i++],str);

	//TAGIHAN
	strcpy(msg[i++],"");
	memset(buf,0,sizeof(buf));
	mapGet(traTotalAmount,buf,lenInquiryAmt);
	strcpy(str,"TAGIHAN : Rp. ");
	fmtAmt(str+14,buf,0,",.");
	strcat(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblKO:
		return -1;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranSSB()
{
       int                 ret;
       char         str[128];
       char         buf[128];
       char         buf1[128];
       char         judul[30];
       char         msg[20][22];
       int                 i=0;

       memset(msg,0,sizeof(msg));
       strcpy(judul,"PEMB. SSB");

       //KODE SATPAS
       memset(buf,0,sizeof(buf));
       memset(str,0,sizeof(str));
       MAPGET(traPaymentNumber, buf, lblKO);
       sprintf(str,"SATPAS: %d",atoi(buf));
       strcpy(msg[i++],str);

       //NO REKENING
       memset(buf,0,sizeof(buf));
       memset(buf1,0,sizeof(buf1));
       memset(str,0,sizeof(str));
       mapGet(traNomorKartuLama,buf,lenNomorKartuLama);
       strncpy(buf1,buf,11);
       sprintf(str,"REK   : %s",buf1);
       strcpy(msg[i++],str);
       memset(buf1,0,sizeof(buf1));
       strncpy(buf1,buf+11,strlen(buf)-11);
       sprintf(str,"        %s",buf1);
       strcpy(msg[i++],str);

       //NAMA
       memset(buf,0,sizeof(buf));
       memset(str,0,sizeof(str));
       mapGet(traCustomerName,buf,lenCustomerName);
       sprintf(str,"NAMA  : %s", buf);
       strcpy(msg[i++],str);
       memset(buf1,0,sizeof(buf1));
       strncpy(buf1,buf+11,strlen(buf)-11);
       sprintf(str,"        %s",buf1);
       strcpy(msg[i++],str);

       //AMOUNT
       memset(buf,0,sizeof(buf));
       mapGet(traAmt,buf,lenAmt);
	buf[strlen(buf)-2] = 0;
       strcpy(str,"TAGIHAN:Rp.");
       fmtAmt(str+11,buf,0,",.");
       strcat(msg[i++],str);


       ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
       if(ret == kbdVAL || ret == kbdF4)
       {
               ret = 1;
               goto lblEnd;
       }
       else
               goto lblBatal;

       lblBatal:
               ret = 0;
               usrInfo(infUserCancel);
               goto lblEnd;

       lblKO:
               return -1;

       lblEnd:
               kbdStop();
               return ret;
}
int KonfirmasiPembayaranDPLKR()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB. DPLK");

	//NO DPLK
	memset(buf,0,sizeof(buf));
	mapGet(traAdminCharge,buf,lenAdminCharge);
	strcat(msg[i++],"NO DPLK : ");
	sprintf(str,"%s",buf);
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//TAGIHAN
	strcpy(msg[i++],"");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"TAGIHAN : Rp. ");
	fmtAmt(str+14,buf,0,",.");
	strcat(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
	{
		goto lblBatal;
	}

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}


int KonfirmasiPembayaranBriva(){
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB. BRIVA");

	//NO BRIVA
	memset(buf,0,sizeof(buf));
	mapGet(traNoVoucherMitra,buf,lenNoVoucherMitra);
	strcat(msg[i++],"NO BRIVA : ");
	sprintf(str,"%s",buf);
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//NAMA INSTITUSI
	memset(buf,0,sizeof(buf));
	mapGet(traCustomerName,buf,lenCustomerName);
	strcat(msg[i++],"NAMA INS : ");
	sprintf(str,"%s",buf);
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//NAMA PELANGGAN
	memset(buf,0,sizeof(buf));
	mapGet(traBuffer3,buf,lenBuffer3);
	strcat(msg[i++],"NAMA PEL : ");
	sprintf(str,"%s",buf);
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//TAGIHAN
	memset(buf,0,sizeof(buf));
	mapGet(traPaymentStatus, buf, lenPaymentStatus);
	if(strcmp(buf, "Y") == 0)
	{
		//KETERANGAN

		memset(buf,0,sizeof(buf));
		mapGet(traCustomerName,buf,lenCustomerName);
		strcat(msg[i++],"KETERANGAN : ");
		sprintf(str,"%s",buf);
		strcat(msg[i++],str);
		strcpy(msg[i++],"");

		memset(buf,0,sizeof(buf));
		mapGet(traAmt,buf,lenAmt);;
		strcat(msg[i++],"TAGIHAN :");
		strcpy(str,"Rp. ");
		fmtAmt(str+4,buf,0,",.");
		strcat(msg[i++],str);
	}
	else
	{
		//KETERANGAN
		memset(buf,0,sizeof(buf));
		mapGet(traCustomerName,buf,lenCustomerName);
		strcat(msg[i++],"KETERANGAN : ");
		sprintf(str,"%s",buf);
		strcat(msg[i++],str);
		strcpy(msg[i++],"");

		//TAGIHAN
		memset(buf,0,sizeof(buf));
		mapGet(traAmt,buf,lenAmt);
		strcat(msg[i++],"TAGIHAN :");
		strcpy(str,"Rp. ");
		fmtAmt(str+4,buf,0,",.");
		strcat(msg[i++],str);

		//JML PEMBYARAN
		strcpy(msg[i++],"");
		memset(buf,0,sizeof(buf));
		mapGet(traAmt,buf,lenAmt);
		strcat(msg[i++],"JML PEMBYARAN:");
		strcpy(str,"Rp. ");
		fmtAmt(str+4,buf,0,",.");
		strcat(msg[i++],str);
	}

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;

}

int KonfirmasiPembayaranBriva2(){
	int 		ret;
	char 	buf[128];
	byte 	Exp;

	//NO BRIVA
	dspClear();
	dspLS(0|0x40,"     BRIVA");
	dspLS(1,"PEMBAYARAN:  Rp.");
	memset(buf,0,sizeof(buf));
	MAPGETBYTE(appExp, Exp, lblKO);
	while(1)
	{
		ret = enterAmt(2,buf,Exp);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 5)
			break;
		Beep();
	}

	MAPPUTSTR(traTotAmt,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;

	lblKO:
		ret = -1;

	lblEnd:
		return ret;
}

int KonfirmasiPembayaranKKBRI()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB. KK BRI");

	//NO KK
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NO KK :");
	mapGet(traMeter,buf,lenMeter);
	trimRight(buf,0x20);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//NAMA
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NAMA :");
	mapGet(traNamaPengirim,buf,lenNamaPemilik);
	trimRight(buf,0x20);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//TAGIHAN
	strcpy(msg[i++],"TAGIHAN :");
	memset(buf,0,sizeof(buf));
	mapGet(traPelangganId,buf,lenPelangganId);
	buf[strlen(buf)-2] = 0;
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//MINIMUM PAYMENT
	strcpy(msg[i++],"PEMB. MINIMAL :");
	memset(buf,0,sizeof(buf));
	mapGet(traTotalAmount,buf,lenInquiryAmt);
	buf[strlen(buf)-2] = 0;
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//PAY DUE DATE
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"PAY DUE DATE      :");
	mapGet(traDueDate,buf,lenDueDate);
	trimRight(buf,0x20);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//AVAILABLE CREDIT
	strcpy(msg[i++],"SISA KREDIT :");
	memset(buf,0,sizeof(buf));
	mapGet(traBunga,buf,lenBunga);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//AVAILABLE CASH
	strcpy(msg[i++],"SISA TUNAI :");
	memset(buf,0,sizeof(buf));
	mapGet(traNominalVoucher,buf,lenBunga);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);


	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranKKBRIBayar()
{
	int 		ret;
	char 	buf[20];

	if(KonfirmasiPembayaranKKBRI())
	{
		dspClear();
		dspLS(0|0x40,"  PEMB. KK BRI");
		dspLS(1,"Input Bayar:");
		memset(buf,0,sizeof(buf));
		while(1)
		{
			ret = enterPhr(2, buf, lenNoVoucherMitra+1);
			if(ret == kbdANN || ret == 0)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(buf) >= 3)
				break;
			Beep();
		}

		MAPPUTSTR(traAmt,buf,lblKO);

		return 1;
		lblBatal:
			ret = 0;
			goto lblEnd;
		lblKO:
			ret = -1;
		lblEnd:
			return ret;
		}
	else
		return 0;
}

int KonfirmasiPembayaranKKANZ()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMB. KK ANZ");

	//NO KK
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NO KK :");
	mapGet(traMeter,buf,lenMeter);
	trimRight(buf,0x20);
	sprintf(str,"%21s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//JUMLAH
	strcpy(msg[i++],"JUMLAH :");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranKKCITI()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;
	byte 	txnType;

	memset(msg,0,sizeof(msg));
	MAPGETBYTE(traTxnType, txnType, lblKO);
	if(txnType == trtPembayaranKKCITIKK){
		strcpy(judul,"PEMB. KK CITI");

		//NO KK
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KK :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}else{
		strcpy(judul,"PEMB. KTA CITI");

		//NO KTA
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KTA :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}

	//JUMLAH
	strcpy(msg[i++],"JUMLAH :");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblKO:
		return -1;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranKKHSBC()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;
	byte 	txnType;

	memset(msg,0,sizeof(msg));
	MAPGETBYTE(traTxnType, txnType, lblKO);
	if(txnType == trtPembayaranKKHSBCKK){
		strcpy(judul,"PEMB. KK HSBC");

		//NO KK
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KK :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}else{
		strcpy(judul,"PEMB. KTA HSBC");

		//NO KTA
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KTA :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}

	//JUMLAH
	strcpy(msg[i++],"JUMLAH :");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblKO:
		return -1;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranKKRBS()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;
	byte 	txnType;

	memset(msg,0,sizeof(msg));
	MAPGETBYTE(traTxnType, txnType, lblKO);
	if(txnType == trtPembayaranKKRBSKK){
		strcpy(judul,"PEMB. KK RBS");

		//NO KK
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KK :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}else{
		strcpy(judul,"PEMB. KTA RBS");

		//NO KTA
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KTA :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}

	//NAMA
	strcpy(msg[i++],"JUMLAH :");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblKO:
		return -1;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranKKStanChar()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;
	byte 	txnType;

	memset(msg,0,sizeof(msg));
	MAPGETBYTE(traTxnType, txnType, lblKO);
	if(txnType == trtPembayaranKKStanCharKK){
		strcpy(judul,"PEMB. KK SCB");

		//NO KK
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KK :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}else{
		strcpy(judul,"PEMB. KTA SCB");

		//NO KTA
		memset(buf,0,sizeof(buf));
		strcpy(msg[i++],"NO KTA :");
		mapGet(traMeter,buf,lenMeter);
		trimRight(buf,0x20);
		sprintf(str,"%21s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}

	//NAMA
	strcpy(msg[i++],"JUMLAH :");
	memset(buf,0,sizeof(buf));
	mapGet(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
	ret = 0;
	usrInfo(infUserCancel);
	goto lblEnd;

	lblKO:
	return -1;

	lblEnd:
	kbdStop();
	return ret;
}

int KonfirmasiTrxTransfer()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"REG. TRX TRANSFER");
	strcpy(msg[i++],"");

	//NAMA
	memset(buf,0,sizeof(buf));
	mapGet(traNamaPengirim,buf,lenNamaPemilik);
	sprintf(str,"NAMA    : %s",buf);
	strcpy(msg[i++],str);

	//NAMA
	memset(buf,0,sizeof(buf));
	mapGet(traLWBP,buf,lenLWBP);
	sprintf(str,"NO. REK : %s",buf);
	strcpy(msg[i++],str);
	sprintf(str,"          %s",buf+11);
	strcpy(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiTrxPulsa()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"REG. TRX PULSA");

	//NAMA
	memset(buf,0,sizeof(buf));
	mapGet(traNoTelpon,buf,lenNoTelpon);
	sprintf(str,"NO HP : %s",buf);
	strcpy(msg[i++],str);
	sprintf(str,"        %s",buf+13);
	strcpy(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}
/*End of Changed By Ali*/

#ifdef ABSENSI
int KonfirmasiAbsensi()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0, cek=0, count=0, setIsti = 0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"ABSENSI");
	strcpy(msg[i++],"");


	//fitur
	memset(buf,0,sizeof(buf));
	mapGet(traJenisRekening, buf, 1);
	trimRight(buf,0x20);
	sprintf(str,"%1s",buf);
	if(strcmp(str, "A") == 0)
		sprintf(str, "ABSEN  : DATANG");
	else if(strcmp(str, "B") == 0)
		sprintf(str, "ABSEN  : PULANG");
	else if(strcmp(str, "C") == 0){
		sprintf(str, "ABSEN  : ISTIRAHAT MULAI");
		setIsti = 1;
	}
	else if(strcmp(str, "D") == 0)
		sprintf(str, "ABSEN  : ISTIRAHAT SELESAI");
	else if(strcmp(str, "E") == 0)
		sprintf(str, "ABSEN  : LEMBUR DATANG");
	else if(strcmp(str, "F") == 0)
		sprintf(str, "ABSEN  : LEMBUR PULANG");
	else if(strcmp(str, "G") == 0)
		sprintf(str, "ABSEN  : SHIFT I DATANG");
	else if(strcmp(str, "H") == 0)
		sprintf(str, "ABSEN  : SHIFT I PULANG");
	else if(strcmp(str, "I") == 0)
		sprintf(str, "ABSEN  : SHIFT II DATANG");
	else if(strcmp(str, "J") == 0)
		sprintf(str, "ABSEN  : SHIFT II PULANG");
	else if(strcmp(str, "K") == 0)
		sprintf(str, "ABSEN  : SHIFT III DATANG");
	else if(strcmp(str, "L") == 0)
		sprintf(str, "ABSEN  : SHIFT III PULANG");
	strcpy(msg[i++],str);

	//informasi
	memset(buf,0,sizeof(buf));
	mapGet(traStatusKartu,buf,lenStatusKartu);
	sprintf(str,"%2s",buf);
	if(strcmp(str,"00") == 0)
		strcpy(str,"STATUS : OK");
	else if(strcmp(str,"TL") == 0){
		strcpy(str,"STATUS : TERLAMBAT");
		cek = 1;
	}
	else if((strcmp(str,"CP") == 0) && (setIsti == 1)){
		strcpy(str,"STATUS : IST. CEPAT");
		count = 1;
	}
	else if((strcmp(str,"CP") == 0) && (setIsti == 0)){
		strcpy(str,"STATUS : PULANG CEPAT");
		count = 1;
	}

	strcpy(msg[i++],str);


	//jam
	memset(buf,0,sizeof(buf));
	mapGet(traTanggalLahir,buf,lenTanggalLahir);
	sprintf(str,"JAM    : %8s",buf);
	strcpy(msg[i++],str);

	if(cek){
		//terlambat
		memset(buf,0,sizeof(buf));
		mapGet(traFee, buf, lenFee);
		sprintf(str,"TRLMBT : %8s",buf);
		strcpy(msg[i++],str);
	}

	if(count){
		//cepat
		memset(buf,0,sizeof(buf));
		mapGet(traFee, buf, lenFee);
		sprintf(str,"PLG CPT: %8s",buf);
		strcpy(msg[i++],str);
	}

	ret = scrollDisplay8(msg,i,12,judul,"OK");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 0;
		goto lblEnd;
	}

	lblEnd:
		kbdStop();
		return ret;

}
#endif

int konfirmasiSettingTIDMID()
{
	int 		ret;
	char 	buf[20];
	char		tid[9];
	char 	MID[lenMid + 1];
	byte 	counter = 0;
	char 	AcqName[lenName + 1];

	dspClear();
	dspLS(0|0x40,"SETTING TID");
	dspLS(1,"TID :");
	MAPGET(acqTID,tid,lblKO);
	dspLS(2,tid);
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(2, buf, lenTid+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 3)
			break;
		Beep();
	}

	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, AcqName, lblKO);
		ret = mapPut(acqTID,buf,8);
		if(AcqName[0] == 0)
			break;
	}

	dspClear();
	dspLS(0|0x40,"SETTING MID");
	dspLS(1,"MID :");
	MAPGET(acqMID, MID, lblKO);
	dspLS(2,MID);
	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenMid+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 13)
			break;
		Beep();
	}
	for (counter = 0; counter < dimAcq; counter++)
	{
		mapMove(acqBeg, counter);
		MAPGET(acqName, AcqName, lblKO);
		ret = mapPut(acqMID,buf,15);
		if(AcqName[0] == 0)
			break;
	}


	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}


int printVersion()
{
	SEGMENT 	Infos;

	info_seg(4, &Infos);
	//pprintf("\x1b" "@\x1b" "@" "Sinarmas ICT220 " "%04s\n" "(Secured)", Infos.libelle);
	pprintf("\x1b" "@\x1b" "@" "Sinarmas ICT220 " "%04s\n" "(Secured)", VERSI_APP); //@@SIMAS-APP_VERSION
	ttestall(PRINTER, 0);
	return 1;
}


#ifdef _USE_ECR
int KonfirmasiPembayaranECR()
{
	int 		ret;
	char 	buf[30];
	char 	msg[20][22];
	char 	judul[30];
	char 	str[128];
	int  		i=0;

	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMBAYARAN SALE");

	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"");
	strcpy(msg[i++],"AMOUNT ? ");
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);

	//NOMINAL
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NOMINAL    :");
	mapGet(appAmtECR,buf,lenAmt);
	memset(str,0,sizeof(str));
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL|| ret == kbdF4)
	{
	ret = 1;
	goto lblEnd;
	}
    	else
    		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;

}

int KonfirmasiShowOption(void)
{
	int 		ret;
	char 	bufs[20];

	dspClear();
	dspLS(0,"1. DEBIT");
	dspLS(1,"2. KREDIT");
	dspLS(2,"3. BRIZZI");

	memset(bufs,0,sizeof(bufs));
	MAPPUTBYTE(traTxnType, trtDummy, lblKO);

	while(1)
	{
		ret = enterPhr(3, bufs, lenBankId);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(bufs) >= 1)
			break;
		Beep();
	}


	if(strcmp(bufs, "1") == 0)
		magStripeInput();
	else if(strcmp(bufs, "2") == 0)
		smartCardInput();
	else if(strcmp(bufs, "3") == 0)
		prepaidInput();

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;


}

int KonfirmasiSetPIN(void)
{
	char 	txt[20];
	int 		ret;
	char		buf[25], buff[25];
	byte		res;


	memset(buf, 0x00, sizeof(buf));
	memset(buff, 0x00, sizeof(buff));

	dspClear();
	dspLS(0,"Set PIN");
	dspLS(1,"1. DEBIT");
	dspLS(2,"2. CREDIT");

	while(1)
	{
		memset(txt,0,sizeof(txt));
		ret = enterStr(3, txt, 2);
		if(ret == kbdANN || ret == 0)
			goto lblKO;

		if(txt[0] == 0x31)
		{
			memset(buf,0,sizeof(buf));
			dspClear();
			dspLS(0,"Set PIN DEBIT");
			dspLS(1,"1. DISABLE");
			dspLS(2,"2. ENABLE");

			while(1)
			{
				ret = enterStr(3, buf, 2);
				if(ret == kbdANN || ret == 0)
					goto lblBatal;
				if(ret < 0)
					goto lblKO;
				if(strlen(buf) >= 1)
					break;
				Beep();
			}
			if(buf[0] == 0x31)
			{
				MAPPUTBYTE(appPINDebitFlag, 1, lblKO);
				break;
			}
			else if(buf[0] == 0x32)
			{
				MAPPUTBYTE(appPINDebitFlag, 2,lblKO);
				break;
			}
			MAPGETBYTE(appPINDebitFlag, res, lblKO);
			break;
		}
		else if(txt[0] == 0x32)
		{
			memset(buf,0,sizeof(buf));
			dspClear();
			dspLS(0,"Set PIN CREDIT");
			dspLS(1,"1. DISABLE");
			dspLS(2,"2. ENABLE");

			while(1)
			{
				ret = enterStr(3, buf, 2);
				if(ret == kbdANN || ret == 0)
					goto lblBatal;
				if(ret < 0)
					goto lblKO;
				if(strlen(buf) >= 1)
					break;
				Beep();
			}
			if(buf[0] == 0x31)
			{
				//MAPPUTBYTE(appPINCreditFlag, 1, lblKO);
				break;
			}
			else if(buf[0] == 0x32)
			{
				//MAPPUTBYTE(appPINCreditFlag, 2,lblKO);
				break;
			}
			//MAPGETBYTE(appPINCreditFlag, res, lblKO);
			break;
		}
	}

	lblBatal:
	lblKO:
		return 0;
}


int KonfirmasiSetMQPSVEPS(void)
{
	char 	txt[20];
	int 		ret;
	char		buf[25], buff[25];
	byte 	Exp;
	byte		res;


	memset(buf, 0x00, sizeof(buf));
	memset(buff, 0x00, sizeof(buff));

	dspClear();
	dspLS(0,"Set MQPS VEPS");
	dspLS(1,"1. DISABLE");
	dspLS(2,"2. ENABLE");

	while(1)
	{
		memset(txt,0,sizeof(txt));
		ret = enterStr(3, txt, 2);
		if(ret == kbdANN || ret == 0)
			goto lblKO;

		if(txt[0] == 0x31)
		{
			MAPPUTBYTE(appMQPSVEPSFlag,1,lblKO);
			break;
		}
		else if(txt[0] == 0x32)
		{
			MAPPUTBYTE(appMQPSVEPSFlag,2,lblKO);
			break;
		}
	}

	MAPGETBYTE(appMQPSVEPSFlag, res, lblKO);
	if(res == 2)
	{
		dspClear();
		dspLS(0|0x40,"INPUT AMOUNT");
		dspLS(1,"Amount: Rp.");
		memset(buf,0,sizeof(buf));

		MAPGETBYTE(appExp, Exp, lblKO);

		while(1)
		{
			ret = enterAmt(2,buf,Exp);
			if(ret == kbdANN || ret == 0)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(buf) >= 3)
				break;
			Beep();
		}
		MAPPUTSTR(appMQPSVEPSValue, buf, lblKO);
	}
	else
		MAPPUTSTR(appMQPSVEPSValue, "0", lblKO);

	MAPGET(appMQPSVEPSValue,buff,lblKO);

	lblBatal:
	lblKO:
		return 0;
}

int magStripeInput(void)
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
	int 		ret;
	char 	BlockedPan[lenPan + 1];
	byte 	Fallback = 1;
	int 		Fallbackret = 0;

	trcS("magStripeInput Beg\n");

	memset(Pan, 0, lenPan + 1);
	memset(Trk2, 0, lenTrk2 + 1);
	memset(ExpDat, 0, lenExpDat + 1);
	memset(Buf, 0, sizeof(Buf));
	memset(Trk1, 0, sizeof(Trk1));
	memset(CardHolderName, ' ', sizeof(CardHolderName));
	memset(BlockedPan, '*', sizeof BlockedPan);

	MAPGETBYTE(traEntMod, EntMod, lblKO);
	MAPPUTBYTE(traTxnType, trtDummy, lblKO);

	ret = usrInfo(infCardInput);
	CHECK(ret >= 0, lblKO);

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


	if(*Buf != 0)
	{
		memcpy(Trk1, Buf, 128);
		ret = fmtTok(0, Trk1, "^");
		CHECK(ret <= sizeof(Trk1), lblExit);
		memset(Trk1, 0, sizeof(Trk1));
		memcpy(Trk1, &Buf[ret + 1], 128 - ret);
		ret = fmtTok(CardHolderName, Trk1, "^");
	}

	MAPPUTSTR(traTrk1, CardHolderName, lblKO);

	pTrk2 = &Buf[128];
	MAPPUTSTR(traTrk2, pTrk2, lblKO);
	ret = fmtTok(Pan, pTrk2, "=");
	CHECK(ret <= lenPan, lblExit);
	VERIFY(ret <= lenPan);
	MAPPUTSTR(traPan, Pan, lblKO);


	lblExit:
		ret = 0;
		goto lblEnd;
	lblKO:
		goto lblEnd;
	lblEnd:
		trcFN("pmtMagStripe: ret=%d\n", ret);
		return ret;
}

int prepaidInput(void)
{
	char 		buf[50];
	int 			ret;
	PREPAID_T 	pData;

	ret = infoKartu(&pData);
	if(ret == FAIL)
	{
		usrInfo(infProcessToReaderFailWait);
		ret = -100;
		goto lblEnd;
	}

	//simpan card Number
	memset(buf,0,sizeof(buf));
	bin2hex(buf,pData.cardNumber,CARD_NUMBER_LEN);
	MAPPUTSTR(traPan,buf,lblKO);
	MAPPUTSTR(traBlockPan,buf,lblKO);

	//No. Kartu
	memset(buf,0,sizeof(buf));
	ret = mapGet(traPan,buf,sizeof(buf));
	CHECK(ret>=0,lblKO);

	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int smartCardInput(void)
{
	int 		ret;
	char 	traName[dspW + 1];

	trcS("pmtSale Beg\n");

	mapMove(rqsBeg, (word) (trtSale - 1));
	MAPGET(rqsDisp, traName, lblKO);
	trcFS("traName: %s\n", traName);

	MAPPUTSTR(traTransType, traName, lblKO);
	MAPPUTSTR(traTypeName, traName, lblKO);
	MAPPUTBYTE(traTxnType, trtDummy, lblKO);
	MAPPUTBYTE(regLocType, 'T', lblKO);
	MAPPUTBYTE(traEmvFullFlag, 1, lblKO);

	pmtPayment(mnuSale);
	goto lblEnd;

	lblKO:
		ret = -1;

	lblEnd:
		ret = 1;

	return ret;
	trcS("pmtSale End\n");
}

#endif


int KonfirmasiPembayaranPLNPra()
{
	int 		ret;
	char 	str[128];
	char 	buf[128], buf2[128];
	char 	judul[30];
	char 	msg[20][22];
	int  		i=0;


	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMBAYARAN PLN");

	//NO METERAN
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NO METERAN : ");
	mapGet(traTotalAmount,buf, lenTotalAmount);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//PELANGGAN ID
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"ID PEL.    : ");
	mapGet(traInquiryAmt,buf,lenInquiryAmt);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//NAMA PELANGGAN
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NAMA PEL.  : ");
	mapGet(traNamaPenerima,buf,lenNamaPemilik);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");


	//TARIF / DAYA
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"TARIF/DAYA  :");
	mapGet(traPaymentNumber,buf,lenPaymentNumber);
	memset(buf2,0,sizeof(buf2));
	mapGet(traAdminCharge,buf2,lenAdminCharge);
	memset(str,0,sizeof(str));
	sprintf(str,"%s / %d", buf, atoi(buf2));
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");


	//NOMINAL
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NOMINAL    :");
	mapGet(traBuffer1,buf,lenBuffer1);
	memset(str,0,sizeof(str));
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");


	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
	    kbdStop();
	    return ret;

}

int KonfirmasiPembayaranPLN()
{
	int 		ret;
	char 	str[128], str1[128];
	char 	buf[128];
	char 	buf1[128];
	char 	buf2[128];
	char 	judul[30];
	char 	msg[20][22];
	int 		i=0;


	memset(msg,0,sizeof(msg));
	strcpy(judul,"PEMBAYARAN PLN");

	//PELANGGAN ID
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"IPEL       : ");
	mapGet(traInquiryAmt,buf, lenInquiryAmt);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//NAMA
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"NAMA       : ");
	mapGet(traNamaPenerima,buf,lenNamaPemilik);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//Tunggakan
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"TUNGGAKAN  : ");
	mapGet(traTotalOutstanding,buf,lenTotalOutstanding);
	memset(str,0,sizeof(str));
	sprintf(str,"%s",buf);
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//BL / TH
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"BL / TH    : ");
	mapGet(traBuffer1,buf,lenBuffer1);
	if(strlen(buf)>18){
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf2));
		strncpy(buf1,buf,18);
		strncpy(buf2,buf+18,strlen(buf)-18);
		sprintf(str,"%s",buf1);
		strcpy(msg[i++],str);
		sprintf(str,"%s",buf2);
		strcpy(msg[i++],str);
	}else{
		memset(str,0,sizeof(str));
		sprintf(str,"%s",buf);
		strcpy(msg[i++],str);
		strcpy(msg[i++],"");
	}
	//TAGIHAN
	memset(str1,0,sizeof(str1));
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"TAGIHAN    : ");
	mapGet(traBunga,buf,lenBunga);
	memset(str,0,sizeof(str));
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//ADMIN BANK
	strcpy(msg[i++],"ADMIN BANK : ");
	memset(str,0,sizeof(str));
	sprintf(str,"%s","Rp. 2.500");
	strcpy(msg[i++],str);
	strcpy(msg[i++],"");

	//TOTAL BAYAR
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	strcpy(msg[i++],"TOTAL BAYAR: ");
	mapGet(traAmt,buf,lenAmt);
	sprintf(buf1,"%d",(atoi(buf))+2500);
	memset(str,0,sizeof(str));
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf1,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int inputInternetBankingPin()
{
	char 	pwd1[10], pwd2[10];
	int 		ret;

	while(1)
	{
		dspClear();
		dspLS(0|0x40,"REG INTERNET");
		dspLS(1|0x40,"BANKING");
		dspLS(3,"");
		memset(pwd1,0,sizeof(pwd1));
		dspLS(2,"Enter PIN :");
		while(1)
		{
			ret = enterPwd(3,pwd1,7);
			if(ret == 0 || ret == kbdANN)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(pwd1) == 6)
				break;
			Beep();
		}

		memset(pwd2,0,sizeof(pwd2));
		dspLS(2,"Re-Enter PIN:");
		while(1)
		{
			ret = enterPwd(3,pwd2,7);
			if(ret == 0 || ret == kbdANN)
				goto lblBatal;
			if(ret < 0)
				goto lblKO;
			if(strlen(pwd2) == 6)
				break;
			Beep();
		}

		if(strcmp(pwd1,pwd2) !=0)
		{
			usrInfo(infPINNotMatch);
			continue;
		}
		else
			break;
	}
	MAPPUTSTR(traPINInternetBanking,pwd1,lblKO);
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}


int inputGantiPasswordSPV()
{
	char 	pwd1[10], pwd2[10];
	int 		ret;

	dspClear();
	dspLS(0|0x40,"UBAH PASS SPV");
	dspLS(3,"");
	memset(pwd1,0,sizeof(pwd1));
	dspLS(2,"Old Password :");
	while(1)
	{
		ret = enterPwd(3,pwd1,7);
		if(ret == 0 ||ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(pwd1) == 6)
			break;
		Beep();
	}
	MAPPUTSTR(traPasswordPetugas,pwd1,lblKO);
	while(1)
	{
		dspClear();
		dspLS(0|0x40,"UBAH PASS SPV");
		dspLS(3,"");
		memset(pwd1,0,sizeof(pwd1));

		dspLS(2,"New Password :");
		ret = enterPwd(3,pwd1,7);
		if(ret == kbdANN)
			goto lblBatal;
		if(ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(pwd1) != 6)
		{
			usrInfo(infPassword6Digit);
			continue;
		}

		memset(pwd2,0,sizeof(pwd2));
		dspLS(2,"Re-Enter Password:");
		ret = enterPwd(3,pwd2,7);

		if(ret == kbdANN)
			goto lblBatal;
		if(ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;

		if(strcmp(pwd1,pwd2) !=0)
		{
			usrInfo(infPasswordNotMatch);
			continue;
		}
		else
			break;
	}
	MAPPUTSTR(traPINInternetBanking,pwd1,lblKO);
	return 1;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int inputReissuePIN()
{
	int 		ret;
	char 	buf[20];

	dspClear();
	dspLS(0|0x40,"  REISSUE PIN");
	dspLS(2,"Nomor Rekening");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPhr(3, buf, lenNomorRekeningBRI+1);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) == lenNomorRekeningBRI)
			break;
		Beep();
	}
	MAPPUTSTR(traNomorRekeningAsalBRI,buf,lblKO);

	ret = pmtNewPinInput();
	CHECK(ret > 0, lblKO);


	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
	    return ret;
}


int KonfirmasiAktivasiKartu()
{
	int 			ret;
	char 		str[128];
	char 		buf[128];
	char 		judul[30];
	char 		baris1[30], baris2[30];
	char 		msg[10][22];
	int 			i=0;
	byte 		txnType;

	MAPGETBYTE(traTxnType, txnType, lblBatal);
	memset(msg,0,sizeof(msg));

	if(txnType == trtAktivasiKartuInquiry)
		strcpy(judul,"AKTIVASI KARTU");
	else
		strcpy(judul,"RENEWAL KARTU");

	//nomor kartu
	memset(buf,0,sizeof(buf));
	mapGet(traPan,buf,sizeof(buf));
	trimRight(buf,0x20);
	strcpy(msg[i++],"Nomor Kartu:");
	strcpy(msg[i++],buf);

	//nomor rekening
	memset(buf,0,sizeof(buf));
	mapGet(traNomorRekeningAsalBRI,buf,sizeof(buf));
	trimRight(buf,0x20);
	strcpy(msg[i++],"Nomor Rekening:");
	strcpy(msg[i++],buf);

	//Nama nasabah
	memset(buf,0,sizeof(buf));
	mapGet(traNamaPenerima,buf,30);
	trimRight(buf,0x20);
	ret = displayLongText(buf, strlen(buf), baris1, baris2, 20);
	strcpy(msg[i++],"Nama:");
	strcpy(msg[i++],baris1);
	if(ret == 2)
	{
		trimLeft(baris2,0x20);
		strcpy(msg[i++],baris2);
	}

	//Tanggal lahir
	strcpy(msg[i++],"Tanggal Lahir:");
	memset(buf,0,sizeof(buf));
	mapGet(traTanggalLahir,buf,lenTanggalLahir);
	sprintf(str,"%c%c-%c%c-%c%c%c%c",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
	strcpy(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiTCash()
{
	int 			ret;
	char 		str[128];
	char 		buf[128];
	char 		judul[30];
	char 		msg[10][22];
	int 			i=0;

	//nominal
	memset(buf,0,sizeof(buf));
	strcpy(msg[i++],"Nominal     : ");
	mapGet(traAmt,buf,lenAmt);
	memset(str,0,sizeof(str));
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);
	strcpy(msg[i++],"");

	//kode akses
	memset(buf,0,sizeof(buf));
	mapGet(traNamaPengirim,buf,lenNamaPemilik);
	trimRight(buf,0x20);
	strcpy(msg[i++],"No Telpon   :");
	strcpy(msg[i++],buf);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL      LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblEnd:
		kbdStop();
		return ret;

}

int KonfirmasiRegistrasiInternetBanking()
{
	int 		ret;
	char 	key;

	dspClear();

	display_text(0,0,"REG INTERNET BANKING",dsp_ALIGN_CENTER);
	display_text(2,0,"Jaga kerahasiaan",dsp_ALIGN_CENTER);
	display_text(3,0,"user id dan password",dsp_ALIGN_CENTER);
	display_text(4,0,"anda",dsp_ALIGN_CENTER);
	display_text(7,0,"BATAL        LANJUT",dsp_ALIGN_CENTER);
	kbdStart(1);

	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = inputInternetBankingPin();
				if(ret == 0)
					goto lblBatal;
				if(ret <0 )
					goto lblKO;
				ret = 1;
				goto lblEnd;
		}
	}

	kbdStop();
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiRegistrasiSmsBanking()
{
	int ret;
	char key;

	dspClear();

	dspLS(0,"Reg SMS Banking");
	dspLS(1,"Jaga kerhsiaan");
	dspLS(2,"PIN Anda");
	dspLS(3,"BATAL     LANJUT");

	MAPPUTBYTE(traJenisKartu,1,lblKO);
	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}
	kbdStop();
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiRegistrasiPhoneBanking()
{
	int 		ret;
	char 	key;

	dspClear();

	dspLS(0,"Reg Phn Banking");
	dspLS(1,"Jaga kerhsiaan");
	dspLS(2,"PIN Anda");
	dspLS(3,"BATAL     LANJUT");

	MAPPUTBYTE(traJenisKartu,1,lblKO); //@agmr - BRI - dianggap kartu kredit
	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}
	kbdStop();
	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiKartuOtorisasi()
{
	int 		ret;
	char 	buf[15];

	dspClear();
	dspLS(0,"   OTORISASI");
	dspLS(1,"Password SPV:");
	memset(buf,0,sizeof(buf));

	while(1)
	{
		ret = enterPwd(2, buf, lenPasswordPetugas+1);
		if(ret == 0 || ret == kbdANN)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) == 6)
			break;
		Beep();
	}

	MAPPUTSTR(traPasswordPetugas,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int KonfirmasiPembayaranPascaBayar()
{
	int 		ret;
	char 	buf[31];
	char 	str[50];
	char 	msg[6][21];
	char 	baris1[21];
	char 	baris2[21];
	int 		i=0;

	dspClear();

	//Nama Pemilik
	memset(str,0,sizeof(str));
	memset(buf,0,sizeof(buf));
	ret = mapGet(traNamaPenerima,buf,sizeof(buf));
	CHK;
	strcpy(str,"Nama:");
	trimRight(buf,0x20);
	strcat(str,buf);
	ret = displayLongText(str,strlen(str),baris1,baris2,17);
	strcpy(msg[i], baris1);
	i++;
	if(ret == 2)
	{
		strcpy(msg[i],baris2);
		i++;
	}

	// NO TELPON
	strcpy(msg[i],"No:");
	memset(buf,0,sizeof(buf));
	ret = mapGet(traNoTelpon,buf,sizeof(buf));
	CHK;
	trimRight(buf,0x20);
	strcat(msg[i],buf);
	i++;

	//JUMLAH TAGIHAN
	strcpy(msg[i],"Tagihan:");
	i++;
	ret = mapGet(traTotalAmount,buf,sizeof(buf));
	CHK;
	mapPut(traAmt,buf,lenAmt);
	trimLeft(buf,'0');
	if(strlen(buf) >=2)
	buf[strlen(buf)-2] = 0;
	memset(baris1,0,sizeof(baris1));
	strcpy(baris1,"Rp. ");
	fmtAmt(baris1+4,buf,0,",.");
	sprintf(str,"%16s",baris1);
	strcpy(msg[i],str);
	i++;

	ret = scrollDisplay(msg,i,2,"KONFIRMASI","BATAL     LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		goto lblEnd;
	lblEnd:
		kbdStop();
		return ret;
}

int KonfirmasiPembayaranPascaBayar8()
{
	int 		ret;
	char 	str[128];
	char 	buf[128];
	char 	judul[30];
	char 	msg[10][22];
	int 		i=0;

	strcpy(judul,"KONFIRMASI");

	memset(msg,0,sizeof(msg));

	//Nama Pemilik
	strcpy(msg[i++],"Nama:");
	memset(buf,0,sizeof(buf));
	ret = mapGet(traNamaPenerima,buf,sizeof(buf));
	CHK;
	strcpy(msg[i++],buf);

	// NO TELPON
	strcpy(msg[i++],"No Telpon:");
	memset(buf,0,sizeof(buf));
	ret = mapGet(traNoTelpon,buf,sizeof(buf));
	CHK;
	trimRight(buf,0x20);
	strcat(msg[i++],buf);

	//JUMLAH TAGIHAN
	strcpy(msg[i++],"Tagihan :");
	memset(buf,0,sizeof(buf));
	mapGet(traTotalAmount,buf,sizeof(buf));
	buf[strlen(buf) - 2] = 0;
	mapPut(traAmt,buf,lenAmt);
	strcpy(str,"Rp. ");
	fmtAmt(str+4,buf,0,",.");
	strcat(msg[i++],str);

	ret = scrollDisplay8(msg,i,6,judul,"BATAL          LANJUT");
	if(ret == kbdVAL || ret == kbdF4)
	{
		ret = 1;
		goto lblEnd;
	}
	else
		goto lblBatal;

	lblBatal:
		ret = 0;
		usrInfo(infUserCancel);
		goto lblEnd;
	lblEnd:
		kbdStop();
		return ret;
}


int KonfirmasiInstallment()
{
	char 	key;
	int 		ret;
	char 	plan[10], term[10], amount[20];
	char 	buf[50];

	dspClear();
	display_text(0, 0, "CICILAN BRING", dsp_ALIGN_CENTER);

	ret = mapGet(traInstallmentTerm,term,sizeof(term));
	CHK;
	sprintf(buf,"Term : %14s",term );
	display_text(3, 0, buf, dsp_ALIGN_LEFT);

	ret = mapGet(traInstallmentPlan,plan,sizeof(plan));
	CHK;
	sprintf(buf,"Plan : %14s",plan );
	display_text(4, 0, buf, dsp_ALIGN_LEFT);

	ret = mapGet(traAmt,amount,sizeof(amount));
	CHK;
	strcpy(buf,"Rp.");
	fmtAmt(buf+3,amount,0,",.");
	display_text(5, 0, buf, dsp_ALIGN_RIGHT);

	display_text(7, 0, "BATAL      LANJUT", dsp_ALIGN_CENTER);
	kbdStart(1);
	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdF1:
			case kbdANN:
				goto lblBatal;
			case kbdF4:
			case kbdVAL:
				ret = 1;
				goto lblEnd;
		}
	}

	lblBatal:
		ret = 0;
		goto lblEnd;

	lblEnd:
		kbdStop();
		return ret;
}

int getField48(tBuffer *req)
{
	int 		ret = 0;
	byte 	buf[1024];
	byte 	data[1024];
	byte 	*ptr;
	byte 	txnType,subType;
	card 	i48len;
	word	menuItm;

	ptr = buf;
	memset(buf,0,sizeof(buf));
	memset(data,0,sizeof(data));

	ret = mapGetWord(traMnuItm, menuItm);
	CHECK(ret >= 0, lblKO);

	if(menuItm == mnuReversal)
	{
		//++ @agmr - BRI - reversal field
		mapGetCard(revField48len,i48len);
		mapGet(revField48,buf,lenField48);
		//-- @agmr - BRI - reversal field

		ret = bufApp(req, (byte *) buf,i48len);
		CHK;
		return bufLen(req);
	}


	MAPGETBYTE(traTxnType,txnType,lblKO);
	MAPGETBYTE(traSubType,subType,lblKO);

	switch(txnType)
	{
		case trtInfoSaldo:
			break;

		case trtInfoSaldoBankLain:
			break;

		case trtMiniStatement:
			break;

		case trtMutRek:
			break;
		case trtPrevilege:
			break;

		case trtTransferSesamaBRIInquiry:
			memcpy(ptr,"000",3);
			ptr+=3;
			ret = mapGet(traNomorRekeningTujuanBRI,data,lenNomorRekeningBRI);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,ret);
			ptr+=ret;
			fmtPad(buf,23,' ');
			ptr = buf+23;
			break;

		case trtTransferSesamaBRI:
			memcpy(ptr,"000",3);
			ptr+=3;
			ret = mapGet(traNomorRekeningTujuanBRI,data,lenNomorRekeningBRI);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNomorRekeningBRI,' ');
			memcpy(ptr,data,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			fmtPad(buf,23,' ');
			ptr = buf+23;
			break;

		case trtTransferAntarBankInquiry:
			ret = mapGet(traBankId,data,lenBankId);
			CHECK(ret > 0,lblKO);
			fmtPad(data,-3,'0');
			memcpy(ptr,data,lenBankId);
			ptr+=lenBankId;

			memset(data,0,sizeof(data));
			ret = mapGet(traNomorRekeningTujuan,data,lenNomorRekening);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNomorRekening,' ');
			memcpy(ptr,data,lenNomorRekening);
			ptr+=lenNomorRekening;

			memset(data,0,sizeof(data));
			ret = mapGet(traCustomerRefNumber,data,lenCustomerRefNumber);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenCustomerRefNumber,' ');
			memcpy(ptr,data,lenCustomerRefNumber);
			ptr+=lenCustomerRefNumber;
			fmtPad(buf,60,' ');
			ptr = buf+60;
			break;

		case trtTransferAntarBank:
			ret = mapGet(traBankId,data,lenBankId);
			CHECK(ret > 0,lblKO);
			fmtPad(data,-3,'0');
			memcpy(ptr,data,lenBankId);
			ptr+=lenBankId;

			memset(data,0,sizeof(data));
			ret = mapGet(traNomorRekeningTujuan,data,lenNomorRekening);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNomorRekening,' ');
			memcpy(ptr,data,lenNomorRekening);
			ptr+=lenNomorRekening;

			memset(data,0,sizeof(data));
			ret = mapGet(traCustomerName,data,lenCustomerName);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenCustomerName,' ');
			memcpy(ptr,data,lenCustomerName);
			ptr+=lenCustomerName;

			memset(ptr,' ',7);
			ptr+=7;

			memset(data,0,sizeof(data));
			ret = mapGet(traCustomerRefNumber,data,lenCustomerRefNumber);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenCustomerRefNumber,' ');
			memcpy(ptr,data,lenCustomerRefNumber);
			ptr+=lenCustomerRefNumber;
			fmtPad(buf,120,' ');
			ptr = buf+ 120;
			break;

		case trtPembayaranPLNPascaInquiry:
			ret = mapGet(traPelangganId,data,lenPelangganId);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenPelangganId,' ');
			memcpy(ptr,data,lenPelangganId);
			ptr+=lenPelangganId;
			break;

		case trtPembayaranPLNPasca:
			mapGet(traField48,data,sizeof(data));
			mapGetCard(traField48Len,i48len);
			memcpy(ptr,data,lenKodeWilayah);
			ptr+=lenKodeWilayah;

			memcpy(ptr,"4",1);
			ptr++;

			memcpy(ptr,data+lenKodeWilayah,i48len-lenKodeWilayah);
			ptr+=(i48len-lenKodeWilayah);
			break;

		case trtPembayaranPLNPraInquiry:
			ret = mapGet(traPelangganId,data,lenPelangganId);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,lenPelangganId);
			ptr+=lenPelangganId;
			break;

		case trtPembayaranPLNPra:
			mapGet(traField48,data,sizeof(data));
			mapGetCard(traField48Len,i48len);

			memcpy(ptr,data,lenKodeWilayah);
			ptr+=lenKodeWilayah;

			memcpy(ptr,"4",1);
			ptr+=1;

			memcpy(ptr,data+3, i48len-3);
			ptr+=(i48len-3);
			break;

		case trtPembayaranPLNToken:
			memcpy(ptr,"0001",4);
			ptr+=4;

			ret = mapGet(traPelangganId,data,lenPelangganId);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,lenPelangganId);
			ptr+=11;

			memcpy(ptr," ",1);
			ptr+=1;

			ret = mapGet(traTotalAmount,data,lenTotalAmount);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,lenTotalAmount);
			ptr+=lenTotalAmount;
			break;

		case trtPembayaranCicilanInquiry:
			switch(subType)
			{
				case CICILAN_FIF:
					memcpy(ptr,"00",2);
					break;
				case CICILAN_BAF:
					memcpy(ptr,"01",2);
					break;
				case CICILAN_OTO:
					memcpy(ptr,"02",2);
					break;
				case CICILAN_FNS:
					memcpy(ptr,"03",2);
					break;
				case CICILAN_VER:
					memcpy(ptr,"04",2);
					break;
				case CICILAN_WOM:
					memcpy(ptr,"05",2);
					break;
			}
			ptr+=2;
			ret = mapGet(traContractNumber,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenContractNumber,' ');
			memcpy(ptr,data,lenContractNumber);
			ptr+=lenContractNumber;
			break;

		case trtPembayaranCicilan:
			ret = mapGet(traField48,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			ret = mapGetCard(traField48Len,i48len);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,i48len);
			ptr+=i48len;
			break;

		case trtPembayaranPulsa:
			switch(subType/10)
			{
				case 1: //simpati
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,lenNoTelpon,' ');
					memcpy(ptr,data,lenNoTelpon);
					ptr+=lenNoTelpon;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					strcat(data,"00");
					fmtPad(data,lenAmt,' ');
					memcpy(ptr,data,lenAmt);
					ptr+=lenAmt;
					break;

				case 2: //mentari
				case 3: //im3
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,lenNoTelpon,' ');
					memcpy(ptr,data,lenNoTelpon);
					ptr+=lenNoTelpon;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					strcat(data,"00");
					fmtPad(data,-lenAmt,'0');
					memcpy(ptr,data,lenAmt);
					ptr+=lenAmt;
					break;

				case 4: //xl
					memset(data,0,sizeof(data));
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,13,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					strcat(data,"00");
					fmtPad(data,-12,'0');
					memcpy(ptr,data,12);
					ptr+=12;
					break;

				case 5://Esia
					memset(data,0,sizeof(data));
					ret = mapGet(traBillPeriod1, data, sizeof(data));
					CHECK(ret > 0, lblKO);
					if(strlen(data) < 4)
					fmtPad(data,4,' ');
					memcpy(ptr,data,4);
					ptr+=4;

					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,9,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					fmtPad(data,-12,'0');
					memcpy(ptr,data,12);
					ptr+=12;
					break;

				case 6://smart
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,13,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					fmtPad(data,-12,'0');
					memcpy(ptr,data,12);
					ptr+=12;
					break;

				case 7://frean
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,13,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					fmtPad(data,-12,'0');
					memcpy(ptr,data,12);
					ptr+=12;
					break;

				case 8://Three
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,13,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					fmtPad(data,-12,'0');
					memcpy(ptr,data,12);
					ptr+=12;
					break;

				case 9://Three
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0, lblKO);
					fmtPad(data,13,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					memset(data,0,sizeof(data));
					ret = mapGet(traAmt,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					fmtPad(data,-12,'0');
					memcpy(ptr,data,12);
					ptr+=12;
					break;
			}
			break;

		case trtPembayaranTVBerlanggananInquiry:
			ret = mapGet(traNamaPenerima,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;


		case trtPembayaranTVBerlangganan:
			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			ret = mapGet(traTLEData,data,lenTLEData);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,strlen(data));
			ptr+=strlen(data);
			break;

		case trtPembayaranTiketPesawatInquiry:
			ret = mapGet(traNamaPenerima,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtPembayaranTiketPesawat:
			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			ret = mapGet(traField48,data,traField48);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,strlen(data));
			ptr+=strlen(data);
			break;

		case trtPembayaranPdamInquiry:
			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			memcpy(ptr,"01",2);
			ptr+=2;
			ret = mapGet(traNamaPenerima,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtPembayaranPdam:
			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			mapGet(traField48,data,sizeof(data));
			mapGetCard(traField48Len,i48len);

			memcpy(ptr,data,lenKodeWilayah);
			ptr+=lenKodeWilayah;

			memcpy(ptr,"2",1);
			ptr+=1;

			memcpy(ptr,data+3, 112);
			ptr+=112;

			memcpy(ptr,"01",2);
			ptr+=2;

			memcpy(ptr,data+115, 43);
			ptr+=43;

			memcpy(ptr,data+160, i48len-160);
			ptr+=(i48len-160);

			break;

		case trtPembayaranTelkomInquiry:
			ret = mapGet(traNamaPenerima,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtPembayaranTelkom:
			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			ret = mapGet(traTLEData,data,lenTLEData);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,strlen(data));
			ptr+=strlen(data);
			break;
#ifdef SSB
		case trtPembayaranSSBInquiry:
		case trtPembayaranSSB:
			memset(data,0,sizeof(data));
			memset(buf,0,sizeof(buf));
			strcpy(data,"000POLRI");
			ret = mapGet(traPaymentNumber,buf,sizeof(buf));
			strcat(data,buf);
			memset(buf,0,sizeof(buf));
			ret = mapGet(traTotalOutstanding,buf,sizeof(buf));
			strcat(data,buf);
			fmtPad(buf,23,' ');
			strcat(data,buf);
			memcpy(ptr,data, 23);
			ptr+=23;
			break;
#endif
		case trtPembayaranDPLKR:
			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			memset(buf,0,sizeof(buf));
			ret = mapGet(traAdminCharge,buf,lenAdminCharge);
			CHECK(ret > 0,lblKO);
			fmtPad(buf,23,' ');
			strcpy(data,"000");
			strcat(data,buf);
			memcpy(ptr,data,strlen(data));
			ptr+=strlen(data);
			break;

	     case trtPembayaranBrivaInquiry:
		 	memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			ret = mapGet(traNamaBankAsal,data,lenNamaBank);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,strlen(data));
			ptr+=strlen(data);
		 	break;

		case trtPembayaranBriva:
			memset(buf, 0, sizeof(buf));
			mapGet(traBillStatus, buf,lenBillStatus);
			if(strcmp(buf, "Y") != 0)
			{
				memset(buf, 0, sizeof(buf));
				mapGet(traAmt,buf,lenAmt);
				strcat(buf,"00");
				mapPut(traAmt,buf,lenAmt);
			}
			else
			{
				memset(buf, 0, sizeof(buf));
				mapGet(traTotAmt,buf,lenTotAmt);
				strcat(buf,"00");
				mapPut(traAmt,buf,lenAmt);
			}

			memset(ptr, 0, sizeof(ptr));
			memset(data,0,sizeof(data));
			mapGet(traField48,data,sizeof(data));
			mapGetCard(traField48Len,i48len);

			memcpy(ptr,data, i48len);
			ptr+=i48len;
			break;

		case trtTrxPhoneBankingTransfer:
			memcpy(ptr, "1", 1);
			ptr+=1;
			ret = mapGet(traLWBP, data,lenLWBP);
			CHECK(ret > 0, lblKO);
			fmtPad(data,35,' ');
			memcpy(ptr, data, 35);
			ptr += 34;
			break;

		case trtTrxPhoneBankingTransferInquiry:
			memcpy(ptr,"000", 3);
			ptr+=3;
			ret = mapGet(traLWBP, data, lenLWBP);
			CHECK(ret > 0,lblKO);
			fmtPad(data,20,' ');
			memcpy(ptr, data, 20);
			ptr+=20;
			break;

		case trtTrxPhoneBankingPulsa:
			memcpy(ptr, "2", 1);
			ptr+=1;
			ret = mapGet(traNoTelpon, data, lenNoTelpon);
			CHECK(ret > 0, lblKO);
			fmtPad(data,35,' ');
			memcpy(ptr, data, 34);
			ptr += 34;
			break;

		case trtPembayaranKKBRIInquiry:
		case trtPembayaranKKBRI:
			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKANZ:
			memcpy(ptr, "3", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKCITIKTA:
		case trtPembayaranKKCITIKK:
			memcpy(ptr, "0000", 4);
			ptr += 4;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKHSBCKK:
			memcpy(ptr, "4", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKHSBCKTA:
			memcpy(ptr, "5", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKRBSKK:
			memcpy(ptr, "6", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKRBSKTA:
			memcpy(ptr, "7", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKStanCharKK:
			memcpy(ptr, "1", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranKKStanCharKTA:
			memcpy(ptr, "2", 1);
			ptr += 1;

			ret = mapGet(traMeter, data, lenMeter);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenMeter);
			fmtPad(data,16,' ');
			ptr += lenMeter;
			break;

		case trtPembayaranSPPInquiry:
			memcpy(ptr, "00", 2);
			ptr += 2;

			ret = mapGet(traKodeWilayah, data, lenKodeWilayah);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenKodeWilayah);
			ptr += lenKodeWilayah;
			ret = mapGet(traNamaPelangganPLN, data, lenNamaPelangganPLN);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenNamaPelangganPLN);
			ptr += lenNamaPelangganPLN;
			break;

		case trtPembayaranSPP:
			ret = mapGet(traKodeWilayah, data, lenKodeWilayah);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenKodeWilayah);
			ptr += lenKodeWilayah;

			ret = mapGet(traNomorRekeningAsal, data, lenNamaPelangganPLN);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenNamaPelangganPLN);
			ptr += lenNamaPelangganPLN;

			ret = mapGet(traNamaPelangganPLN, data, lenNamaPelangganPLN);
			CHECK(ret > 0, lblKO);
			memcpy(ptr, data, lenNamaPelangganPLN);
			ptr += lenNamaPelangganPLN;
			break;

#ifdef ABSENSI
		case trtAbsensi:
			switch(subType)
			{
				case SHIFTIA:
					memcpy(ptr,"G", 1);
					break;
				case SHIFTIB:
					memcpy(ptr,"H", 1);
					break;
				case SHIFTIIA:
					memcpy(ptr,"I", 1);
					break;
				case SHIFTIIB:
					memcpy(ptr,"J", 1);
					break;
				case SHIFTIIIA:
					memcpy(ptr,"K", 1);
					break;
				case SHIFTIIIB:
					memcpy(ptr,"L", 1);
					break;
				case LEMLANG:
					memcpy(ptr,"F", 1);
					break;
				case LEMTANG:
					memcpy(ptr,"E", 1);
					break;
				case PULANG:
					memcpy(ptr,"B", 1);
					break;
				case DATANG:
					memcpy(ptr,"A", 1);
					break;
				case ISTIMUL:
					memcpy(ptr,"C", 1);
					break;
				case ISTIAKH:
					memcpy(ptr,"D", 1);
					break;
			}
			ptr += 1;
			fmtPad(ptr, 23, ' ');
			ptr += 23;
			break;
#endif
		case trtPembayaranZakat:
			switch(subType%10)
			{
				case 1: // dhuafa
					memcpy(ptr,"001",3);
					break;
				case 2: // YBM
					memcpy(ptr,"002",3);
					break;
			}
			fmtPad(ptr,23,' ');
			ptr+=23;
			break;

		case trtSetorPasti:
			break;

		case trtSetorSimpananInquiry:
		case trtSetorSimpanan:
		case trtSetorPinjamanInquiry:
		case trtSetorPinjaman:
			memcpy(ptr,"002",3);
			ptr+=3;
			ret = mapGet(traNomorRekeningAsalBRI,data,sizeof(data));
			CHECK(ret > 0, lblKO);
			fmtPad(data,lenNomorRekeningBRI,' ');
			memcpy(ptr,data,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			fmtPad(buf,23,' ');
			ptr = buf+23;
			break;

		case trtTarikTunai:
		case trtVoidTarikTunai:
			memset(ptr,' ',23);
			ptr+=23;
			break;

		case trtTCash:
			ret = mapGet(traNamaPengirim,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenLogo,' ');
			memcpy(ptr,data,lenLogo);
			ptr+=lenLogo;

			ret = mapGet(traNamaPenerima,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtTCashOut:
			ret = mapGet(traNamaPenerima,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenLogo,' ');
			memcpy(ptr,data,lenLogo);
			ptr+=lenLogo;

			ret = mapGet(traNamaPengirim,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtRegistrasiInternetBanking:
			ret = mapGet(traPan,data,sizeof(data));
			CHECK(ret > 0, lblKO);
			fmtPad(data,16,' ');
			memcpy(ptr,data,16);
			ptr+=16;

			memset(data,0,sizeof(data));
			ret = mapGet(traPINInternetBanking,data,lenPINInternetBanking);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenPINInternetBanking,' ');
			memcpy(ptr,data,lenPINInternetBanking);
			ptr+=lenPINInternetBanking;
			memset(ptr,' ',13);
			ptr+=13;
			break;

		case trtRegistrasiPhoneBanking:
			memset(data,0,sizeof(data));
			mapGet(traJenisKartu,data,1);
			if(data[0] == 1)
				memcpy(ptr,"1109",4);
			else
				memcpy(ptr,"1103",4);
			fmtPad(ptr,35,' ');
			ptr+=35;
			break;

		case trtRegistrasiSmsBanking:
			ret = mapGet(traPan,data,sizeof(data));
			CHECK(ret > 0, lblKO);
			fmtPad(data,16,' ');
			memcpy(ptr,data,16);
			ptr+=16;

			memset(data,0,sizeof(data));
			ret = mapGet(traNoTelpon,data,lenNoTelpon);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;

			memset(data,0,sizeof(data));
			ret = mapGet(traPINInternetBanking,data,lenPINInternetBanking);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenPINInternetBanking,' ');
			memcpy(ptr,data,lenPINInternetBanking);
			ptr+=lenPINInternetBanking;
			break;

		case trtRenewalKartuOtorisasi:
		case trtReissuePINOtorisasi:
		case trtAktivasiKartuOtorisasi:
			if(txnType == trtRenewalKartuOtorisasi)
				memcpy(ptr,"000",3);
			else
				memcpy(ptr,"001",3);
			ptr+=3;
			ret = mapGet(traPasswordPetugas,data,lenPasswordPetugas);
			CHECK(ret > 0, lblKO);
			fmtPad(data,lenPasswordPetugas,' ');
			memcpy(ptr,data,lenPasswordPetugas);
			ptr+=lenPasswordPetugas;
			fmtPad(buf,23,' ');
			ptr = buf+23;
			break;

		case trtAktivasiKartu:
			memset(ptr,0x20,60);
			ret = mapGet(traNomorKartuPetugas,data,lenNomorKartuPetugas);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNomorKartuPetugas,' ');
			memcpy(ptr,data,lenNomorKartuPetugas);
			ptr+=lenNomorKartuPetugas;

			memset(data,0,sizeof(data));
			ret = mapGet(traPasswordPetugas,data,lenPasswordPetugas);
			CHECK(ret>0,lblKO);
			fmtPad(data,lenPasswordPetugas,' ');
			memcpy(ptr,data,lenPasswordPetugas);
			ptr+=lenPasswordPetugas;

			memset(data,0,sizeof(data));
			ret = mapGet(traNewPinBlk,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			sprintf(ptr,"%02X%02X%02X%02X%02X%02X%02X%02X",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
			ptr+=16;
			memcpy(ptr,"  ",2);
			ptr+=2;
			memset(data,0,sizeof(data));
			ret = mapGet(traNomorRekeningAsalBRI,data,lenNomorRekeningBRI);
			CHECK(ret > 0, lblKO);
			fmtPad(data,15,' ');
			memcpy(ptr,data,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			ptr = buf+60;
			break;

		case trtRenewalKartu:
			memset(ptr,0x20,60);
			ret = mapGet(traNomorKartuPetugas,data,lenNomorKartuPetugas);
			CHECK(ret > 0,lblKO);
			fmtPad(data,lenNomorKartuPetugas,' ');
			memcpy(ptr,data,lenNomorKartuPetugas);
			ptr+=lenNomorKartuPetugas;

			memset(data,0,sizeof(data));
			ret = mapGet(traNomorKartuLama,data,lenNomorKartuLama);
			CHECK(ret>0,lblKO);
			memcpy(ptr,data,lenNomorKartuLama);
			ptr+=lenNomorKartuLama;

			memset(data,0x20,8);
			memcpy(ptr,data,8);
			ptr+=8;

			memset(data,0,sizeof(data));
			ret = mapGet(traNomorRekeningAsalBRI,data,lenNomorRekeningBRI);
			CHECK(ret > 0, lblKO);
			fmtPad(data,15,' ');
			memcpy(ptr,data,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			ptr = buf+60;

			break;

		case trtReissuePIN:
			memset(ptr,0x20,60);
			ret = mapGet(traNomorKartuPetugas,data,lenNomorKartuPetugas);
			CHECK(ret > 0, lblKO);
			fmtPad(data,lenNomorKartuPetugas,' ');
			memcpy(ptr,data,lenNomorKartuPetugas);
			ptr+=lenNomorKartuPetugas;

			memset(data,0,sizeof(data));
			ret = mapGet(traNomorRekeningAsalBRI,data,lenNomorRekeningBRI);
			CHECK(ret > 0, lblKO);
			fmtPad(data,lenNomorRekeningBRI,' ');
			memcpy(ptr,data,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			ptr = buf+60;

			break;

		case trtInstallment:
			break;

		case trtGantiPIN:
			memset(data,0,sizeof(data));
			ret = mapGet(traNewPinBlk,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			sprintf(ptr,"%02X%02X%02X%02X%02X%02X%02X%02X",data[0],data[1],data[2],data[3],	data[4],data[5],data[6],data[7]);
			ptr+=16;
			break;

		case trtGantiPasswordSPV:
			strcpy(data,"001");
			memcpy(ptr,data,3);
			ptr+=3;

			memset(data,0,sizeof(data));
			ret = mapGet(traPasswordPetugas,data,lenPasswordPetugas);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,6);
			ptr+=6;

			memset(data,0,sizeof(data));
			ret = mapGet(traPINInternetBanking,data,lenPINInternetBanking);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,6);
			ptr+=6;
			memset(ptr,' ',8);
			ptr+=8;
			break;

		case trtPembayaranPascaBayarInquiry:
			ret = mapGet(traNoTelpon,data,lenNoTelpon);
			CHECK(ret > 0, lblKO);
			fmtPad(data,lenNoTelpon,' ');
			memcpy(ptr,data,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtPembayaranPascaBayar:
			switch(subType)
			{
				case PASCA_BAYAR_HALO:
					ret = mapGet(traField48,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					memmove(data+24,data+24+30,68-30-24);
					memcpy(ptr,data,68-30);
					ptr+=38;
					break;

				case PASCA_BAYAR_MATRIX:
					memset(ptr,0x20,59);
					ret = mapGet(traNoTelpon,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					fmtPad(data,13,' ');
					memcpy(ptr,data,13);
					ptr+=13;

					ret = mapGet(traField48,data,sizeof(data));
					CHECK(ret > 0,lblKO);
					memcpy(ptr,data+1,46);
					ptr+=46;
					break;
			}
			break;

#ifdef PREPAID
		case trtPrepaidInfoDeposit:
			memset(data,0,sizeof(data));
			ret = mapGet(traPan,data,lenPan);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,16);
			ptr+=16;
			break;

		case trtPrepaidDeposit:
			memset(data,0,sizeof(data));
			ret = mapGet(TRA_NOMOR_KARTU_BRIZZI,data,LEN_TRA_NOMOR_KARTU_BRIZZI);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;
			break;

		case trtPrepaidTopUp:
			{
				byte buffer[50];

				//random dari SAM
				memset(data,0,sizeof(data));
				memset(buffer,0,sizeof(buffer));
				ret = mapGet(TRA_KEY_HOST,buffer,LEN_TRA_KEY_HOST);
				CHECK(ret > 0,lblKO);
				bin2hex(data,buffer,KEY_HOST_LEN*2);
				memcpy(ptr,data,KEY_HOST_LEN*2);
				ptr+=(KEY_HOST_LEN*2);

				//current balance
				memset(data,0,sizeof(data));
				memset(buffer,0,sizeof(buffer));
				ret = mapGet(TRA_SISA_SALDO,buffer,LEN_TRA_SISA_SALDO);
				memcpy(data,buffer,AMOUNT_LEN_L);
				trimLeft(data,'0');
				strcat(data,"00");
				fmtPad(data, -AMOUNT_LEN_L, '0');
				memcpy(ptr,data,AMOUNT_LEN_L);
				ptr+= AMOUNT_LEN_L;

				//byte spasi
				memset(ptr,' ',2);
				ptr+=2;
			}
			break;

		case trtPrepaidAktivasi:
		{
			byte buffer[50];

			//random
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_KEY_HOST,buffer,LEN_TRA_KEY_HOST);
			bin2hex(data,buffer,KEY_HOST_LEN);
			memcpy(ptr,data,2*KEY_HOST_LEN);
			ptr+= (2*KEY_HOST_LEN);

			//saldo
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_SALDO_AWAL,buffer,LEN_TRA_SALDO_AWAL);
			memcpy(data,buffer,AMOUNT_LEN_L);
			trimLeft(data,'0');
			strcat(data,"00");
			fmtPad(data, -AMOUNT_LEN_L, '0');
			memcpy(ptr,data,AMOUNT_LEN_L);
			ptr+= AMOUNT_LEN_L;
		}
			break;

		case trtPrepaidRedeemInquiry:
		case trtPrepaidRedeem:
		{
			byte buffer[50];

			//Nomor kartu Brizzi
			memset(data,0,sizeof(data));
			ret = mapGet(TRA_NOMOR_KARTU_BRIZZI,data,LEN_TRA_NOMOR_KARTU_BRIZZI);
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;

			//current balance
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_SISA_SALDO,buffer,LEN_TRA_SISA_SALDO);
			memcpy(data,buffer,AMOUNT_LEN_L);
			trimLeft(data,'0');
			strcat(data,"00");
			fmtPad(data, -AMOUNT_LEN_L, '0');
			memcpy(ptr,data,AMOUNT_LEN_L);
			ptr+= AMOUNT_LEN_L;
		}
			break;

	//++ @agmr - brizzi2
		case trtPrepaidReaktivasiInquiry:
		{
			byte buffer[50];

			//Nomor kartu Brizzi
			memset(data,0,sizeof(data));
			ret = mapGet(TRA_NOMOR_KARTU_BRIZZI,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;

			//current balance
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_SISA_SALDO,buffer,sizeof(buffer));
			CHECK(ret > 0,lblKO);
			memcpy(data,buffer,AMOUNT_LEN_L);
			trimLeft(data,'0');
			fmtPad(data, -AMOUNT_LEN_L, '0');
			memcpy(ptr,data,AMOUNT_LEN_L);
			ptr+= AMOUNT_LEN_L;

			//last transaction date
			memset(data,0,sizeof(data));
			ret = mapGet(TRA_LAST_TRANS_DATE,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,LAST_TRANS_DATE_LEN);
			ptr+= LAST_TRANS_DATE_LEN;
		}
			break;

		case trtPrepaidReaktivasi:
		{
			byte buffer[50];

			//Nomor kartu Brizzi
			memset(data,0,sizeof(data));
			ret = mapGet(TRA_NOMOR_KARTU_BRIZZI,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;

			//current balance
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_SISA_SALDO,buffer,sizeof(buffer));
			CHECK(ret > 0,lblKO);
			memcpy(data,buffer,AMOUNT_LEN_L);
			trimLeft(data,'0');
			fmtPad(data, -AMOUNT_LEN_L, '0');
			memcpy(ptr,data,AMOUNT_LEN_L);
			ptr+= AMOUNT_LEN_L;

			//last transaction date
			memset(data,0,sizeof(data));
			ret = mapGet(TRA_LAST_TRANS_DATE,data,sizeof(data));
			CHECK(ret > 0,lblKO);
			memcpy(ptr,data,LAST_TRANS_DATE_LEN);
			ptr+= LAST_TRANS_DATE_LEN;

			memset(data, 0, sizeof(data));
			mapGet(traField48,data,sizeof(data));
			mapGetCard(traField48Len,i48len);
			memcpy(ptr,data,lenKodeWilayah);
			ptr+=lenField48;
		}
			break;

		case trtPrepaidVoidInquiry:
		{
			byte buffer[50];

			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			MAPGET(logDatTim,buffer,lblKO);

			memcpy(ptr,buffer+2,2);//year
			ptr+=2;

			memcpy(ptr,buffer+4,2);//month
			ptr+=2;

			memcpy(ptr,buffer+6,2); //day
			ptr+=2;

			memcpy(ptr,buffer+8,6); //time
			ptr+=6;

			//procode
			memset(buffer,0,sizeof(buffer));
			MAPGET(LOG_PROCODE, buffer, lblKO);
			memcpy(ptr,buffer,PROC_LEN);
			ptr+=PROC_LEN;

			//amount
			memset(buffer,0,sizeof(buffer));
			MAPGET(logTraAmt, buffer, lblKO);
			strcat(buffer,"00");
			fmtPad(buffer, -AMOUNT_LEN_L, '0');
			memcpy(ptr,buffer,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//ref number
			memset(buffer,0,sizeof(buffer));
			MAPGET(LOG_REF_NUMBER,buffer,lblKO);
			memcpy(ptr,buffer,REF_NUMBER_LEN);
			ptr+=REF_NUMBER_LEN;

			//batch number
			memset(buffer,0,sizeof(buffer));
			MAPGET(LOG_PREPAID_BATCH_NUM,buffer,lblKO);
			memcpy(ptr,buffer,BATCH_NUMBER_LEN);
			ptr+=BATCH_NUMBER_LEN;

			//nomor kartu
			memset(buffer,0,sizeof(buffer));
			MAPGET(LOG_NOMOR_KARTU_BRIZZI,buffer,lblKO);
			memcpy(ptr,buffer,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;

			//hash
			memset(buffer,0,sizeof(buffer));
			MAPGET(LOG_HASH,buffer,lblKO);
			bin2hex(ptr,buffer,HASH_LEN);
			ptr+= 2*HASH_LEN;
		}
			break;

		case trtPrepaidVoid:
		{
			byte buffer[50];

			//random dari SAM
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_KEY_HOST,buffer,LEN_TRA_KEY_HOST);
			CHECK(ret > 0,lblKO);
			bin2hex(data,buffer,KEY_HOST_LEN*2);
			memcpy(ptr,data,KEY_HOST_LEN*2);
			ptr+=(KEY_HOST_LEN*2);

			//current balance
			memset(data,0,sizeof(data));
			memset(buffer,0,sizeof(buffer));
			ret = mapGet(TRA_SISA_SALDO,buffer,LEN_TRA_SISA_SALDO);
			memcpy(data,buffer,AMOUNT_LEN_L);
			trimLeft(data,'0');
			strcat(data,"00");
			fmtPad(data, -AMOUNT_LEN_L, '0');
			memcpy(ptr,data,AMOUNT_LEN_L);
			ptr+= AMOUNT_LEN_L;

			//byte spasi
			memset(ptr,' ',2);
			ptr+=2;
		}
			break;
//-- @agmr - brizzi2


		case trtPrepaidSettlement:
		{
			char buffer[50];
			int 	i;
			int 	adaTrx = 0;

			for(i=0;i<MAX_PREPAID_SETTLEMENT;i++)
			{
				ret = procPrepaidUpload();
				if(ret < 0)
					goto lblKO;
				if(ret == 0)
					break;

				adaTrx = 1;
				uploadedPrepaidIdx[i+1] = ret-1;

				memset(data,0,sizeof(data));
				memset(buffer,0,sizeof(buffer));
				MAPGET(logDatTim,buffer,lblKO);

				memcpy(ptr,buffer+2,2);//year
				ptr+=2;

				memcpy(ptr,buffer+4,2);//month
				ptr+=2;

				memcpy(ptr,buffer+6,2); //day
				ptr+=2;

				memcpy(ptr,buffer+8,6); //time
				ptr+=6;

	                    //procode
	                    memset(buffer,0,sizeof(buffer));
	                    MAPGET(LOG_PROCODE, buffer, lblKO);
	                    memcpy(ptr,buffer,PROC_LEN);
	                    ptr+=PROC_LEN;

	                    //amount
	                    memset(buffer,0,sizeof(buffer));
	                    MAPGET(logTraAmt, buffer, lblKO);
	    	            strcat(buffer,"00");
	    	            fmtPad(buffer, -AMOUNT_LEN_L, '0');
	    	            memcpy(ptr,buffer,AMOUNT_LEN_L);
	    	            ptr+=AMOUNT_LEN_L;

	    	            //ref number
	    	            memset(buffer,0,sizeof(buffer));
	    	            MAPGET(LOG_REF_NUMBER,buffer,lblKO);
	    	            memcpy(ptr,buffer,REF_NUMBER_LEN);
	    	            ptr+=REF_NUMBER_LEN;

	    	            //batch number
	    	            memset(buffer,0,sizeof(buffer));
	    	            MAPGET(LOG_PREPAID_BATCH_NUM,buffer,lblKO);
	    	            memcpy(ptr,buffer,BATCH_NUMBER_LEN);
	    	            ptr+=BATCH_NUMBER_LEN;

	    	            //nomor kartu
	    	            memset(buffer,0,sizeof(buffer));
	    	            MAPGET(LOG_NOMOR_KARTU_BRIZZI,buffer,lblKO);
	    	            memcpy(ptr,buffer,NOMOR_KARTU_BRIZZI_LEN);
	    	            ptr+=NOMOR_KARTU_BRIZZI_LEN;

	    	            //hash
	    	            memset(buffer,0,sizeof(buffer));
	    	            MAPGET(LOG_HASH,buffer,lblKO);
	    	            bin2hex(ptr,buffer,HASH_LEN);
	    	            ptr+= 2*HASH_LEN;
    	        	}

			if(adaTrx == 0)
			{
				mapPut(traRspCod, "00", 2);
				return -101;
			}
				uploadedPrepaidIdx[0] = i;
		}

			break;
#endif

#ifdef LINE_ENCRYPTION
		case trtTLEKeyDownload:
		{
			char serialNum[20 + 1];
			char sap[16 + 1];
			char dvc;

			memset(data,0,sizeof(data));
			mapGet(traTLEUserId,data,lenTLEUserId);
			memcpy(ptr,data,lenTLEUserId);
			ptr+=lenTLEUserId;

			*ptr++ = '|';

			memset(data,0,sizeof(data));
			shaData(data);

			memcpy(ptr,data,40);
			ptr+=40;

			*ptr++ = '|';

			memset(serialNum, 0, sizeof(serialNum));
			memset(sap, 0, sizeof(sap));
			memset(&dvc, 0, sizeof(dvc));
			getSapSer(sap, serialNum, dvc);

			memcpy(ptr,serialNum+2,8);
			ptr+=8;
		}
			break;
#endif
	}

	//++ @agmr - BRI - reversal field
	if(txnType != trtPrepaidSettlement)
	{
		mapPutCard(traField48Len,ptr-buf);
		mapPut(traField48,buf,ptr-buf);
	}
	//-- @agmr - BRI - reversal field

	ret = bufApp(req, (byte *) buf,ptr-buf);
	CHK;
	return bufLen(req);

	lblKO:
		return -1;
}

int getIso57(tBuffer * req)
{
	int 		ret = 0;
	char 	buf[20];
	word 	menuItm;
	card 	len;

	ret = mapGetWord(traMnuItm, menuItm);
	if(ret < 0)
		return ret;

	if(menuItm == mnuReversal)
	{
		memset(buf,0,sizeof(buf));
		//++ @agmr - BRI - reversal field
		mapGetCard(revField57len,len);
		mapGet(revField57,buf,lenField57);
		//-- @agmr - BRI - reversal field

		ret = bufApp(req, (byte *) buf,len);
		return ret;
	}
	memset(buf,0,sizeof(buf));
	strcpy(buf,"@POSBRI#");

	//++ @agmr - BRI - reversal field
	mapPutCard(traField57Len,8);
	mapPut(traField57,buf,8);
	//-- @agmr - BRI - reversal field

	ret = bufApp(req, (byte *) buf,strlen(buf));
	CHK;
	return ret;
}

int getField63(tBuffer *req)
{
	byte 	txnType,subType,jenis;
	char 	buf[70];
	char 	buf1[105];
	int 		ret;
	word 	mnuItm;

	MAPGETWORD(traMnuItm, mnuItm, lblKO);
	if(mnuItm == mnuReversal)
	{
		card i63len;

		//++ @agmr - BRI - reversal field
		mapGetCard(revField63len,i63len);
		mapGet(revField63,buf,lenField63);
		//-- @agmr - BRI - reversal field

		ret = bufApp(req, (byte *) buf,i63len);
		CHK;
		return bufLen(req);
	}

	MAPGETBYTE(traTxnType,txnType,lblKO);
	MAPGETBYTE(traSubType,subType,lblKO);
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	switch(txnType)
	{
		case  trtDebitLogon:
		case  trtLogOn:
		case trtEchoTest:
		{
				char 	serialNum[20 + 1];
				char 	sap[16 + 1];
				char 	dvc;
				SEGMENT Infos;
				char		bufTemp[50], bufTemp1[50];

				info_seg(4, &Infos);

				memset(bufTemp, 0x00, strlen(bufTemp));
				memcpy(bufTemp,"v2013.1.0.0.4",strlen("v2013.1.0.0.4"));
				fmtPad(bufTemp,-lenNamaBank,'0');
				memcpy(buf, bufTemp,strlen(bufTemp));
				memset(serialNum, 0, sizeof(serialNum));
				memset(sap, 0, sizeof(sap));
				memset(&dvc, 0, sizeof(dvc));
				getSapSer(sap, serialNum, dvc);
				memset(bufTemp1, 0x00, strlen(bufTemp1));
				memcpy(bufTemp1,serialNum,strlen(serialNum));
				fmtPad(bufTemp1,-lenNamaBank,'0');
				memcpy(buf+strlen(bufTemp), bufTemp1,strlen(bufTemp1));
		}
			break;

		case trtSaleRedeem:
		{
			char bufferx[105];
			char amount[20];
			char ProdCode[20], ProdCode1[20];

			memset(bufferx, 0, sizeof(bufferx));
			memset(amount, 0, sizeof(amount));
			memset(ProdCode, 0, sizeof(ProdCode));
			memset(ProdCode1, 0, sizeof(ProdCode1));
			mapGet(rptHdrSiteAddr3, ProdCode1, 0);
			strncpy(ProdCode, ProdCode1, 2);
			mapGet(traPowerConsuming,amount,lenPowerConsuming);
			fmtPad(amount,lenPowerConsuming,'0');
			memcpy(bufferx, "000000000000000000000000000000000000", 36);
			memcpy(bufferx+36, amount, lenPowerConsuming);
			memcpy(bufferx+45, "000000000000000000000000", 24);
			memcpy(bufferx+69, ProdCode, 12);
			memcpy(buf1,bufferx, strlen(bufferx));
		}
			break;

		case trtInfoSaldo:
			memcpy(buf,"000BRI",6);
			break;

		case trtTCash:
			memcpy(buf,"003BRI",6);
			break;

		case trtTCashOut:
			memcpy(buf,"003BRI",6);
			break;

		case trtInfoSaldoBankLain:
			memcpy(buf,"000INQBLAIN",11);
			break;

		case trtMiniStatement:
			memcpy(buf,"004MINISTAT",11);
			break;

		case trtMutRek:
			memcpy(buf,"500MUTASI",9);
			break;

        case trtPrevilege:
			memcpy(buf,"003PRIVIL",9);
			break;


		case trtTransferSesamaBRIInquiry:
			memcpy(buf,"001BRI",6);
			break;

		case trtTransferSesamaBRI:
			memcpy(buf,"001BRI",6);
			break;

		case trtTransferAntarBankInquiry:
			memcpy(buf,"003ANTARBANK",12);
			break;

		case trtTransferAntarBank:
			memcpy(buf,"006ANTARBANK",12);
			break;

		case trtPembayaranPLNPascaInquiry:
			memcpy(buf,"201PLNPOST",10);
			break;

		case trtPembayaranPLNPasca:
			memcpy(buf,"201PLNPOST",10);
			break;

		case trtPembayaranPLNPraInquiry:
			memcpy(buf,"201PLNPREPAID",13);
			break;

		case trtPembayaranPLNPra:
			memcpy(buf,"201PLNPREPAID",13);
			break;

		case trtPembayaranPLNToken:
			memcpy(buf,"201PLNTOKEN",11);
			break;

		case trtPembayaranCicilanInquiry:
			memcpy(buf,"205SYB",6);
			break;

		case trtPembayaranCicilan:
			switch(subType)
			{
				case CICILAN_FIF:
					memcpy(buf,"206SYBFIF",9);
					break;

				case CICILAN_BAF:
					memcpy(buf,"206SYBBAF",9);
					break;

				case CICILAN_OTO:
					memcpy(buf,"206SYBOTO",9);
					break;

				case CICILAN_FNS:
					memcpy(buf,"206SYBFNS",9);
					break;

				case CICILAN_VER:
					memcpy(buf,"206SYBVER",9);
					break;

				case CICILAN_WOM:
					memcpy(buf,"206SYBWOM",9);
					break;
			}
			break;

		case trtPembayaranPulsa:
			switch(subType/10)
			{
				case 1:
					memcpy(buf,"300SIMPATI",10);
					break;

				case 2:
					memcpy(buf,"300MENTARI",10);
					break;

				case 3:
					memcpy(buf,"300IM3SMART",11);
					break;

				case 4:
					memcpy(buf,"300XL",5);
					break;

				case 5:
					memcpy(buf, "300ESIA", 7);
					break;

				case 6:
					memcpy(buf, "300SMART", 8);
					break;

				case 7:
					memcpy(buf, "300FREN", 7);
					break;

				case 8:
					memcpy(buf, "300THREE", 8);
					break;

				case 9:
					memcpy(buf, "300AXIS", 8);
					break;

			}
			break;

		case trtPembayaranZakat:
			switch(subType/10)
			{
				case 5:
					memcpy(buf,"001ZAKAT",8);
					break;
				case 6:
					memcpy(buf,"001INFAQ",8);
					break;
				case 7:
					memcpy(buf,"001DPLK",7);
					break;
			}
			break;

		case trtSetorPasti:
			 MAPGETBYTE(traKategoriKartu,jenis,lblKO);
             if((jenis == 1) || (jenis == 2))
             memcpy(buf,"001BRI",6);
             else
             memcpy(buf,"003ANTARBANK",12);
			 break;

		case trtPembayaranTVBerlanggananInquiry:
		case trtPembayaranTVBerlangganan:
			switch(subType)
			{
				case TV_INDOVISION:
					memcpy(buf, "200TVID", 7);
					break;

				case TV_OKTV:
					memcpy(buf, "200TVOK", 7);
					break;

				case TV_TOPTV:
					memcpy(buf, "200TVTO", 7);
					break;

				default:
					break;
			}
			break;

		case trtPembayaranTiketPesawatInquiry:
			switch(subType)
			{
				case TIKET_GARUDA:
					memcpy(buf, "200GARUDA", 9);
					break;

				case TIKET_LION_AIR:
					memcpy(buf, "200LION", 7);
					break;

				case TIKET_SRIWIJAYA_AIR:
					memcpy(buf, "200SRIWJY", 9);
					break;

				case TIKET_MANDALA:
					memcpy(buf, "200MANDALA", 10);
					break;

				default:
					break;
			}
			break;

		case trtPembayaranTiketPesawat:
			memcpy(buf, "200TICKET", 9);
			break;

			case trtPembayaranPdamInquiry:
			memcpy(buf, "003PDAM", 7);
			break;

		case trtPembayaranPdam:
			memcpy(buf, "201PDAM", 7);
			break;

		case trtPembayaranTelkomInquiry:
		case trtPembayaranTelkom:
			memcpy(buf, "200TELMD", 8);
			break;
#ifdef SSB
		case trtPembayaranSSBInquiry:
		case trtPembayaranSSB:
			memcpy(buf, "001POLRI", 8);
			break;
#endif
		case trtPembayaranDPLKR:
			memcpy(buf, "001DPLK", 7);
			break;

		case trtPembayaranBrivaInquiry:
			memcpy(buf, "500BRIVA", 8);
			break;

		case trtPembayaranBriva:
			memcpy(buf, "500BRIVA", 8);
			break;

		case trtTrxPhoneBankingTransfer:
			memcpy(buf, "012PBTRXREG", 11);
			break;

		case trtTrxPhoneBankingTransferInquiry:
			memcpy(buf, "001BRI", 6);
			break;

		case trtTrxPhoneBankingPulsa:
			memcpy(buf, "012PBTRXREG", 11);
			break;

		case trtRegistrasiSmsBanking:
			memcpy(buf,"012SMSREG", 9);
			break;

		case trtPembayaranKKBRIInquiry:
		case trtPembayaranKKBRI:
			memcpy(buf,"204BRICC", 8);
			break;

		case trtPembayaranKKANZ:
			memcpy(buf,"204ANZCC", 8);
			break;

		case trtPembayaranKKCITIKTA:
			memcpy(buf,"204CTPL", 7);
			break;

		case trtPembayaranKKCITIKK:
			memcpy(buf,"204CTCC", 7);
			break;

		case trtPembayaranKKHSBCKK:
		case trtPembayaranKKHSBCKTA:
			memcpy(buf,"204HSBCCC", 9);
			break;

		case trtPembayaranKKRBSKK:
		case trtPembayaranKKRBSKTA:
			memcpy(buf,"204RBSCC", 8);
			break;

		case trtPembayaranKKStanCharKK:
		case trtPembayaranKKStanCharKTA:
			memcpy(buf,"204SCBCC", 8);
			break;

		case trtPembayaranSPPInquiry:
		case trtPembayaranSPP:
			memcpy(buf,"204SPPONLINE", 12);
			break;

		case trtSetorSimpananInquiry:
			memcpy(buf,"001BRI",6);
			break;

		case trtSetorSimpanan:
			memcpy(buf,"001BRI",6);
			break;

		case trtSetorPinjamanInquiry:
			memcpy(buf,"001BRI",6);
			break;

		case trtSetorPinjaman:
			memcpy(buf,"001BRI",6);
			break;

		case trtTarikTunai:
			memcpy(buf,"000BRI",6);
			break;

		case trtVoidTarikTunai:
			memcpy(buf,"000BRI",6);
			break;

		case trtRegistrasiInternetBanking:
			memcpy(buf,"012IBANKREG",11);
			break;

		case trtRegistrasiPhoneBanking:
			memcpy(buf,"012PBREG",8);
			break;

		case trtRenewalKartuOtorisasi:
		case trtAktivasiKartuOtorisasi:
		case trtReissuePINOtorisasi:
			memcpy(buf,"001OTO",6);
			break;

		case trtAktivasiKartuInquiry:
			memcpy(buf,"204AKTIVASI",11);
			break;

		case trtAktivasiKartu:
			memcpy(buf,"003AKTIVASI",11);
			break;

		case trtRenewalKartuInquiry:
			memcpy(buf,"204RENEWAL",10);
			break;

		case trtRenewalKartu:
			memcpy(buf,"003RENEWAL",10);
			break;

		case trtReissuePIN:
			memcpy(buf,"003REISSUE",10);
			break;

		case trtGantiPIN:
			memcpy(buf,"011PINCHG",9);
			break;

		case trtGantiPasswordSPV:
			memcpy(buf,"001OTO",6);
			break;

		case trtInstallment:
			memcpy(buf,"001BRI",6);
			break;

		case trtPembayaranPascaBayarInquiry:
		case trtPembayaranPascaBayar:
			switch(subType)
			{
				case PASCA_BAYAR_HALO:
					memcpy(buf,"203TELKOMSEL",12);
					break;

				case PASCA_BAYAR_MATRIX:
					memcpy(buf,"202INDOSAT",10);
					break;
			}
			break;
#ifdef ABSENSI
		case trtAbsensi:
			memcpy(buf,"002BRI",6);
			break;
#endif
#ifdef PREPAID
		//++@agmr - brizzi2
		case trtPrepaidReaktivasiInquiry:
		case trtPrepaidReaktivasi:
		case trtPrepaidVoidInquiry:
		case trtPrepaidVoid:
		//--@agmr - brizzi2
		case trtPrepaidAktivasi:
		case trtPrepaidTopUp:
			memcpy(buf,"003EMONEY",9);
			break;

		case trtPrepaidRedeem:
		case trtPrepaidDeposit:
		case trtPrepaidRedeemInquiry:
		case trtPrepaidInfoDeposit:
			memcpy(buf,"204EMONEY",9);
			break;

		case trtPrepaidSettlement:
			memcpy(buf,"500BRIZZI",9);
			break;

#endif
		default:
			return 0;
	}

	if((txnType!=trtSaleRedeem) && (txnType!=  trtDebitLogon) &&(txnType!=  trtLogOn) && (txnType!=  trtEchoTest))
	{
		fmtPad(buf,20,' ');
		//++ @agmr - BRI - reversal field
		mapPutCard(traField63Len,20);
		mapPut(traField63,buf,20);
		//-- @agmr - BRI - reversal field
		ret = bufApp(req, (byte *) buf, 20);
		CHK;
	}
	else if((txnType==trtSaleRedeem) && (txnType!=  trtDebitLogon) &&(txnType!=  trtLogOn) && (txnType!=  trtEchoTest))
	{
		fmtPad(buf1,105,' ');

		//++ @agmr - BRI - reversal field
		mapPutCard(traField63Len,105);
		mapPut(traField63,buf1,105);
		//-- @agmr - BRI - reversal field

		ret = bufApp(req, (byte *) buf1, 105);
		CHK;
	}
	else if((txnType!=trtSaleRedeem) || (txnType==  trtDebitLogon) || (txnType==  trtLogOn) || (txnType==  trtEchoTest))
	{
		fmtPad(buf,35,' ');

		//++ @agmr - BRI - reversal field
		mapPutCard(traField63Len,35);
		mapPut(traField63,buf,35);
		//-- @agmr - BRI - reversal field

		ret = bufApp(req, (byte *) buf, 35);
		CHK;
	}
	return bufLen(req);

	lblKO:
		return -1;
}

int getField61(tBuffer *req)
{
	byte 	txnType,subType;
	char 	buf[20], *ptr;
	char 	data[20];
	int 		ret;
	byte 	exp;

	MAPGETBYTE(traTxnType,txnType,lblKO);
	MAPGETBYTE(traSubType,subType,lblKO);
	memset(buf,0,sizeof(buf));
	ptr = buf;

	switch(txnType)
	{
		case trtInstallment:
			memset(data,0,sizeof(data));
			ret = mapGet(traInstallmentPlan,data,lenInstallmentPlan);
			CHECK(ret > 0, lblKO);
			fmtPad(data,-3,'0');
			memcpy(ptr,data,3);
			ptr+=3;

			memset(data,0,sizeof(data));
			ret = mapGet(traInstallmentTerm,data,lenInstallmentTerm);
			CHECK(ret > 0, lblKO);
			fmtPad(data,-2,'0');
			memcpy(ptr,data,2);
			ptr+=2;

			memset(data,0,sizeof(data));
			ret = mapGet(traAmt,data,lenAmt);
			CHECK(ret > 0, lblKO);
			MAPGETBYTE(appExp, exp, lblKO);
			if(exp == 0)
			{
				fmtPad(data,-7,'0');
				memcpy(data + 7, "00", 2);
			}
			else
				fmtPad(data,-9,'0');

			memcpy(ptr,data,9);
			ptr+=9;
			break;

		default:
			return 0;
	}

	ret = bufApp(req, (byte *)buf,ptr-buf);
	CHK;
	return bufLen(req);

	lblKO:
		return -1;
}

int parseField60(const byte* val, word len)
{
	int 			ret;
	byte 		txnType;
	const byte 	*ptr;

	ptr = val;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch (txnType)
	{
#ifdef LINE_ENCRYPTION
		case trtTLEKeyDownload:
#ifdef _DEBUG_TLE
			prtS("VAL:");
			ShowData(val,128,0,0,8);
#endif // _DEBUG_TLE
			if(len == 129 && *val == 0)
				mapPut(traEncryptedUMK,val+1,128);
			else
				mapPut(traEncryptedUMK,val,128);
			break;
#endif
		default:
			break;
	}

	return 0;
	lblKO:
		return -1;
}

int parseField61(const byte* val, word len)
{
	int 			ret;
	byte 		txnType;
	const byte 	*ptr;

	ptr = val;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch (txnType)
	{
		case trtInstallment:
			ptr += 16;

			ret = mapPut(traInstallmentCicilan,ptr,lenInstallmentCicilan);
			CHK;
			break;

		default:
			break;
	}

	return 0;
	lblKO:
		return -1;
}

int parseField63(const byte* val, word len)
{
	int 			ret;
	byte 		txnType;
	const byte 	*ptr;

	ptr = val;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch (txnType)
	{
		case trtSaleRedeem:
			//	SALES-AMT   		- (12)
			mapPut(traInquiryAmt, ptr, lenInquiryAmt);
			ptr+=lenInquiryAmt;

			//	REDEEMED-AMT   	- (12)
			mapPut(traPelangganId, ptr, lenInquiryAmt);
			ptr+=lenInquiryAmt;

			//	NET-SALES-AMT	- (12)
			mapPut(traBunga, ptr, lenInquiryAmt);
			ptr+=lenInquiryAmt;

			//	REDEEMED-PT	- (11)
			mapPut(traNomorRekeningAsalBRI, ptr, 11);
			ptr+=11;

			//	AWARD-PT		- (11)
			mapPut(traNomorRekeningTujuanBRI, ptr, 11);
			ptr+=11;

			//	BAL-PT			- (11)
			mapPut(traNamaBankAsal, ptr, 11);
			ptr+=11;

			//	PROD-CD		- (2)
			mapPut(traTotalOutstanding, ptr, lenTotalOutstanding);
			ptr+=lenTotalOutstanding;
			break;

		default:
			break;
	}

	return 0;
	lblKO:
		return -1;
}

int parseField48(const byte* val, word len)
{
	int 			ret;
	byte 		txnType,subType;
	const byte 	*ptr;
	byte 		data[256], jenis;
	char 		buf[128], buf1[128];
	int 			i=0;

	ptr = val;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	MAPGETBYTE(traSubType,subType,lblKO);

	switch (txnType)
	{
#ifdef LINE_ENCRYPTION
		case trtTLEKeyDownload:
			mapPut(traTLEKCV,val+11,3);
			break;
#endif
		case trtInfoSaldo:
			mapPut(traSisaSaldo,ptr,lenSisaSaldo);
			ptr+=lenSisaSaldo;

			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			break;

		case trtInfoSaldoBankLain:
			mapPut(traSisaSaldo,ptr,lenSisaSaldo);
			ptr+=lenSisaSaldo;

			mapPut(traNamaBankAsal,ptr,lenNamaBank);
			ptr+=lenNamaBank;
			break;

		case trtMiniStatement:
			mapPut(traBuffer6,ptr,lenBuffer6-23);
			ptr+=105;

			mapPut(traNomorRekeningTujuanBRI,ptr,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			break;

		case trtMutRek:


			mapPut(traNomorRekeningTujuanBRI,ptr,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;

			mapPut(traBuffer7,ptr,lenBuffer7-36);
			ptr+=(lenBuffer7)-36;


			break;

		case trtPrevilege:


			mapPut(traNomorRekeningTujuanBRI,ptr,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;

			mapPut(traBuffer7,ptr,lenBuffer7-36);
			ptr+=(lenBuffer7)-36;


			break;




		case trtTransferSesamaBRIInquiry:
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			break;

		case trtTransferAntarBankInquiry:
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;

			mapPut(traNamaBankAsal,ptr,lenNamaBank);
			ptr+=lenNamaBank;

			mapPut(traNamaBankTujuan,ptr,lenNamaBank);
			ptr+=lenNamaBank;
			break;

		case trtTransferAntarBank:
			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			ptr+=(lenNamaPemilik+30);

			mapPut(traJenisTransfer,ptr,lenJenisTransfer);
			ptr+=lenJenisTransfer;

			mapPut(traNomorRekeningAsal,ptr,lenNomorRekening);
			break;

		case trtPembayaranTVBerlanggananInquiry:
			//reply
			mapPut(traTLEData, ptr, lenTLEData-12);

			//ID Pelanggan
			mapPut(traNamaPenerima,ptr,lenNoTelpon);
			ptr+=lenNoTelpon;

			//Nama Pelanggan
			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;

			//Periode
			mapPut(traLWBP,ptr,lenLWBP);
			ptr+=lenLWBP;
			break;

		case trtPembayaranTVBerlangganan:
			//ID Pelanggan
			mapPut(traNamaPenerima,ptr,lenNoTelpon);
			ptr+=lenNoTelpon;

			//Nama Pelanggan
			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;

			//Periode
			mapPut(traLWBP,ptr,lenLWBP);
			ptr+=lenLWBP;
			break;

		case trtPembayaranTiketPesawatInquiry:
		case trtPembayaranTiketPesawat:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			//kode bayars 13
			mapPut(traNoTelpon,ptr,lenNoTelpon);
			ptr+=lenNoTelpon;

			//kode drive 2
			mapPut(traBuffer2,ptr,lenBuffer2);
			ptr+=lenBuffer2;

			//kode datel 4
			mapPut(traTanggalLahir,ptr,lenTanggalLahir-4);
			ptr+=4;


			//jumlah flight 1
			mapPut(traBillStatus,ptr,lenBillStatus);
			ptr++;

			memset(buf,0,sizeof(buf));
			mapGet(traBillStatus,buf,lenBillStatus);
			for(i=0;i<= atoi(buf);i++)
			{
				strcpy(tempTiket[i], ptr);
				ptr+=lenBuffer4;

			}

			//nama Penumpang 30
			mapPut(traBuffer3,ptr,lenBuffer3-10);
			ptr+=30;

			//kode penerbangan
			mapPut(traNamaBankAsal,ptr,lenNamaBank);
			ptr+=lenNamaBank;
			break;


		case trtPembayaranPdamInquiry:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			//	- Jumlah tagihan		10-29	(20)
			ptr+=9;
			mapPut(traNoVoucherMitra, ptr, lenNoVoucherMitra);
			ptr+=lenNoVoucherMitra;


			//	- Priode bayar				48	(1)
			ptr+=18;
			mapPut(traJenisTransfer,ptr,lenJenisTransfer-2);
			ptr++;


			//	- kode askoper			49-58	(10)
			mapPut(traPokokHutang,ptr,lenPokokHutang-2);
			ptr+=10;
			ptr+=57;

			//	- Nomor perlanggan		116-135 (20)
			mapPut(traBuffer1,ptr,lenBuffer1-5);
			ptr+=20;

			//	- Nama pelanggan		136-155 (20)
			mapPut(traCustomerName,ptr,lenCustomerName);
			ptr+=lenCustomerName;
			ptr+=42;

			//	- No Pelanggan			198-217 (20)
			mapPut(traNamaPelangganPLN,ptr,lenNamaPelangganPLN);
			ptr+=lenNamaPelangganPLN;

			//   - Jumlah Pakai
			mapPut(traBuffer4,ptr,lenBuffer4-3);
			ptr+=lenBuffer4;

			break;

		case trtPembayaranPdam:
			//	- Jumlah tagihan		10-29	(20)
			ptr+=9;
			mapPut(traNoVoucherMitra, ptr, lenNoVoucherMitra);
			ptr+=lenNoVoucherMitra;


			//	- Priode bayar				48	(1)
			ptr+=19;
			mapPut(traJenisTransfer,ptr,lenJenisTransfer-2);
			ptr++;

			//	- kode askoper			49-58	(10)
			mapPut(traPokokHutang,ptr,lenPokokHutang-2);
			ptr+=10;
			ptr+=57;

			//	- Nomor perlanggan		116-135 (20)
			mapPut(traBuffer1,ptr,lenBuffer1-5);
			ptr+=20;

			//	- Nama pelanggan		136-155 (20)
			mapPut(traCustomerName,ptr,lenCustomerName);
			ptr+=lenCustomerName;
			ptr+=42;

			//	- No Pelanggan	baru		198-217 (20)
			mapPut(traNamaPelangganPLN,ptr,lenNamaPelangganPLN);
			ptr+=lenNamaPelangganPLN;

			//   - Jumlah Pakai
			mapPut(traBuffer4,ptr,lenBuffer4-3);
			ptr+=lenBuffer4;

			break;

		case trtPembayaranTelkomInquiry:
			//reply
			mapPut(traTLEData, ptr, lenTLEData-12);

			//Kode Area
			mapPut(traPaymentNumber,ptr,lenPaymentNumber);
			ptr += lenPaymentNumber;

			//No Telp
			mapPut(traAdminCharge,ptr,lenAdminCharge);
			ptr+= lenAdminCharge;

			ptr+= 18;
			//Amount
			mapPut(traAmt, ptr, lenInquiryAmt);
			mapPut(traTotalAmount, ptr, lenInquiryAmt);
			ptr+= lenInquiryAmt;

			//Nama Pelanggan
			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			break;

		case trtPembayaranTelkom:
			//Kode Area
			mapPut(traPaymentNumber,ptr,lenPaymentNumber);
			ptr+= lenPaymentNumber;

			//No Telp
			mapPut(traAdminCharge,ptr,lenAdminCharge);
			ptr+= lenAdminCharge;

			ptr+= 18;
			//Amount
			mapPut(traTotalAmount, ptr, lenInquiryAmt);
			ptr+= lenInquiryAmt;

			//Nama Pelanggan
			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			break;

#ifdef SSB
		case trtPembayaranSSBInquiry:
			//No Rekening
			mapPut(traNomorKartuLama,ptr,15);
			ptr+= 15;

			//Nama
			mapPut(traCustomerName,ptr,lenCustomerName);
			break;

		case trtPembayaranSSB:
			break;

#endif
		case trtPembayaranDPLKR:
			break;

		case trtPembayaranBrivaInquiry:
		case trtPembayaranBriva:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			// - nomor BRIVA 			1-20		(20)
			mapPut(traNoVoucherMitra,ptr,lenNoVoucherMitra);
			ptr+=lenNoVoucherMitra;

			//- keterangan 			21 - 40		(20)
			mapPut(traCustomerName, ptr, lenNoVoucherMitra);
			ptr+=lenNoVoucherMitra;

			//- Coorporate fee		41-48		(8)
			mapPut(traFee,ptr,lenFee);
			ptr+=lenFee;

			//- Customer Fee			49 - 56		(8)
			mapPut(traPeriode,ptr,lenPeriode);
			ptr+=lenPeriode;

			// - Nama Pelanggan 		57 - 91		(35)
			mapPut(traBuffer3, ptr, 35);
			ptr+=35;

			// - Amount Tagihan 		92 - 103		(12)
			mapPut(traPelangganId, ptr, lenPelangganId);
			mapPut(traAmt, ptr, lenAmt);
			ptr+=lenPelangganId;

			//- Rekening Giro Institusi	104 - 118	(15)
			mapPut(traNamaBankAsal, ptr, lenNamaBank);
			ptr+=lenNamaBank;

			// - Nama Institusi 		119 - 138	(20)
			mapPut(traBuffer1, ptr, 20);
			ptr+=20;

			// - Payment Status 		139			(1)
			mapPut(traPaymentStatus, ptr, lenPaymentStatus);
			ptr++;

			// - Flag Billing			140			(1)
			mapPut(traBillStatus, ptr, lenPaymentStatus);
			ptr++;

			// - Status Bayar 			141			(1)
			mapPut(traTotalOutstanding, ptr, lenPaymentStatus);
			ptr++;

			break;

		case trtRegistrasiSmsBanking:
			//no telp
			mapPut(traNoTelpon, ptr, lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtTrxPhoneBankingTransferInquiry:
			//Nama Rekening
			mapPut(traNamaPengirim,ptr,lenNamaPemilik);
			break;

		case trtTrxPhoneBankingPulsa:
			//status
			mapPut(traBillStatus, ptr, lenBillStatus);
			ptr+=lenBillStatus;

			//no telpon
			mapPut(traNoTelpon, ptr, lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtPembayaranKKBRIInquiry:
			//Nama
			mapPut(traNamaPengirim, ptr, lenNamaPemilik);
			ptr+=lenNamaPemilik;

			//tagihan
			mapPut(traPelangganId, ptr, lenPelangganId);
			ptr+=lenPelangganId;

			//minimum payment
			mapPut(traTotalAmount, ptr, lenTotalAmount);
			ptr+=lenTotalAmount;

			//pay due date
			mapPut(traDueDate, ptr, lenDueDate);
			ptr+=lenDueDate;

			//available credit
			mapPut(traBunga, ptr, lenBunga);
			ptr+=lenBunga;

			//available cash
			mapPut(traNominalVoucher, ptr, lenNominalVoucher-1);
			ptr+=(lenNominalVoucher-1);
			break;

		case trtPembayaranKKBRI:
			break;

		case trtPembayaranKKANZ:
		case trtPembayaranKKCITIKK:
		case trtPembayaranKKCITIKTA:
		case trtPembayaranKKHSBCKK:
		case trtPembayaranKKHSBCKTA:
		case trtPembayaranKKRBSKK:
		case trtPembayaranKKRBSKTA:
		case trtPembayaranKKStanCharKTA:
		case trtPembayaranKKStanCharKK:
			break;

		case trtPembayaranSPPInquiry:
			//Nomor billing					char(20)
			mapPut(traNomorRekeningAsal, ptr, lenNomorRekening);
			ptr+=lenNomorRekening;

			//Nomor Mahasiswa 				char(20)
			mapPut(traNomorRekeningTujuan, ptr, lenNomorRekening);
			ptr+=lenNomorRekening;

			//Institution Reff					char(20)
			mapPut(traNamaPelangganPLN, ptr, lenNomorRekening);
			ptr+=lenNomorRekening;

			//Nama Mahasiswa				char(20)
			mapPut(traCustomerName, ptr, lenNomorRekening);
			ptr+=lenNomorRekening;

			//Program Studi					char(20)
			mapPut(traNoVoucherMitra, ptr, lenNomorRekening);
			ptr+=lenNomorRekening;

			//Periode 						char(11)
			mapPut(traNominalVoucher, ptr, 11);
			ptr+=11;

			//Jumlah tagihan (padded with space)	num(9)
			mapPut(traPowerConsuming, ptr, lenPowerConsuming);
			ptr+=lenPowerConsuming;
			break;

		case trtPembayaranSPP:
			break;

#ifdef ABSENSI
		case trtAbsensi:
			//fitur
			mapPut(traJenisRekening, ptr, 1);
			ptr+=1;

			//informasi
			mapPut(traStatusKartu, ptr, lenStatusKartu);
			ptr+=lenStatusKartu;

			//jam sekarang
			mapPut(traTanggalLahir, ptr, lenTanggalLahir);
			ptr+=lenTanggalLahir+1;

			//jam terlambat
			memset(buf, 0, sizeof(buf));
			mapGet(traStatusKartu,buf,lenStatusKartu);
			sprintf(str,"%s",buf);
			if(strcmp(str, "TL") == 0){
				mapPut(traFee, ptr, lenTanggalLahir);
			}
			if(strcmp(str, "CP") == 0){
				mapPut(traFee, ptr, lenTanggalLahir);
			}

			break;
#endif
		case trtPembayaranPLNPascaInquiry:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			ptr+=3;

			//ID Pelanggan
			mapPut(traInquiryAmt,ptr,lenInquiryAmt);
			ptr+=lenRrn;

			ptr+=33;
			//Tunggakan
			mapPut(traTotalOutstanding,ptr,lenTotalOutstanding);
			ptr+=lenTotalOutstanding;

			ptr+=35;
			//Tagihan
			mapPut(traBunga,ptr,11);
			ptr+=11;

			memset(buf, 0, sizeof(buf));
			memset(buf1, 0, sizeof(buf1));
			mapGet(traBunga,buf,lenBunga);
			sprintf(buf1,"%d",(atoi(buf)));
			mapPut(traAmt,buf1,lenAmt);
			ptr+=19;

			//Nama Pelanggan
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			ptr+=134;

			//BL / TH
			mapPut(traBuffer1,ptr,23);
			ptr+=23;

			break;

		case trtPembayaranPLNPasca:
			ptr+=4;

			//ID Pelanggan
			mapPut(traInquiryAmt,ptr,lenInquiryAmt);
			ptr+=lenRrn;


			ptr+=20;
			//Tipe Tarif
			mapPut(traCustomerSegment,ptr,lenCustomerSegment);
			ptr+=lenCustomerSegment;

			//Daya
			mapPut(traPenaltyFee,ptr,lenAdminCharge);
			ptr+=lenAdminCharge;

			//Tunggakan
			mapPut(traTotalOutstanding,ptr,lenTotalOutstanding);
			ptr+=lenTotalOutstanding;
			ptr+=22;

			//Admin Bank
			mapPut(traAdminCharge,ptr,lenAdminCharge);
			ptr+=lenAdminCharge;
			ptr+=4;

			//tagihan
			mapPut(traPokokHutang,ptr,11);
			ptr+=11;

			//RP TAG PLN
			mapPut(traNominalVoucher,ptr,10);
			ptr+=10;
			ptr+=9;

			//Nama Pelanggan
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			ptr+=52;

			//MLPO REF
			mapPut(traUMK,ptr,lenUMK);
			ptr+=lenUMK;
			ptr+=50;

			//BL / TH
			mapPut(traBuffer1,ptr,23);
			ptr+=23;
			ptr+=59;

			//NO Telp UPJ
			mapPut(traNoKartuInternetBanking,ptr,15);
			ptr+=15;

			//Stand Meter
			mapPut(traLWBP,ptr,lenLWBP);
			ptr+=lenLWBP;

			break;

		case trtPembayaranPLNToken:
		case trtPembayaranPLNPra:
			ptr+=4;
			//ID Pelanggan              5-16       char(12)
			mapPut(traPokokHutang,ptr, lenPokokHutang);
			ptr+=lenPokokHutang;
			ptr+=20;

			//- Tipe Tarif                37-40     char(4)
			mapPut(traBillPeriod1,ptr, lenBillPeriod);
			ptr+=lenBillPeriod;

			//- Daya                        41-49     char(9)
			mapPut(traPenaltyFee,ptr, lenPenaltyFee);
			ptr+=lenPenaltyFee;

			//- Nomor Meteran           50-60     char(11)
			mapPut(traInstallmentPokok,ptr, 11);
			ptr+=11;
			ptr+=13;

			//- Admin CA                74-82     number(9)
			mapPut(traAdminCharge,ptr, lenPenaltyFee);
			ptr+=lenPenaltyFee;
			ptr+=4;

			//- RP BAYAR                87-97     number(11)
			mapPut(traNoKartuInternetBanking,ptr, 11);
			ptr+=11;
			ptr+=10;

			//- Materai                 108-116   number(9)
			mapPut(traPowerConsuming,ptr,lenPenaltyFee);
			ptr+=lenPenaltyFee;

			//- Nama Pelanggan          117-146   char(30)
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			ptr+=22;

			//- Rp Token                169-179   number(11)
			mapPut(traNominalVoucher,ptr,11);
			ptr+=11;

			//- Jumlah KWH              180-189   char(10)
			mapPut(traSisaSaldo,ptr,10);
			ptr+=10;

			//- PPN                     190-198   number(9)
			mapPut(traInstallmentCicilan,ptr,lenPenaltyFee);
			ptr+=lenPenaltyFee;

			//- MLPO Referensi          199-230   char(32)
			mapPut(traUMK,ptr,lenUMK);
			ptr+=lenUMK;
			ptr+=41;

			//- PPJ                     272-280   number(9)
			mapPut(traBunga,ptr,lenPenaltyFee);
			ptr+=lenPenaltyFee;

			//- Token                   281-300   char(20)
			mapPut(traNoVoucherMitra,ptr,lenNoVoucherMitra);
			ptr+=lenNoVoucherMitra;
			ptr+=53;

			//- Angsuran                354-362   number(9)
			mapPut(traBuffer1,ptr,lenPenaltyFee);
			ptr+=lenPenaltyFee;

			//- No Telepon              363-373   char(12)
			mapPut(traNoTelpon,ptr,lenBunga);
			ptr+=lenBunga;

			break;

		case trtPembayaranPLNPraInquiry:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			ptr+=3;

			//ID Pelanggan
			mapPut(traInquiryAmt,ptr,lenInquiryAmt);
			ptr+=lenInquiryAmt;
			ptr+=20;

			//Tipe Tarif
			mapPut(traPaymentNumber,ptr,lenPaymentNumber);
			ptr+=lenPaymentNumber;

			//Daya
			mapPut(traAdminCharge,ptr,lenAdminCharge);
			ptr+=lenAdminCharge;

			//Nomor Meteran
			mapPut(traTotalAmount,ptr,11);
			ptr+=11;
			ptr+=56;

			//Nama Pelanggan
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			ptr+=53;

			//MLPO Ref
			mapPut(traUMK,ptr,lenUMK);
			ptr+=lenUMK;

			break;

		case trtPembayaranCicilanInquiry:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			mapPut(traTotalOutstanding,ptr,lenTotalOutstanding);
			ptr+=lenTotalOutstanding;

			mapPut(traBillStatus,ptr,lenBillStatus);
			ptr+=lenBillStatus;

			mapPut(traPaymentStatus,ptr,lenPaymentStatus);
			ptr+=lenPaymentStatus;

			mapPut(traCustomerName,ptr,lenCustomerName);
			ptr+=lenCustomerName;

			mapPut(traContractNumber,ptr,lenContractNumber);
			ptr+=lenContractNumber;

			mapPut(traCustomerType,ptr,lenCustomerType);
			ptr+=lenCustomerType;

			mapPut(traMPLO,ptr,lenMPLO);
			ptr+=lenMPLO;

			mapPut(traAdminCharge,ptr,lenAdminCharge);
			ptr+=lenAdminCharge;

			mapPut(traPaymentNumber,ptr,lenPaymentNumber);
			ptr+=lenPaymentNumber;

			mapPut(traDueDate,ptr,lenDueDate);
			ptr+=lenDueDate;

			mapPut(traTotalAmount,ptr,lenTotalAmount);
			ptr+=lenTotalAmount;

			mapPut(traPokokHutang,ptr,lenPokokHutang);
			ptr+=lenPokokHutang;

			mapPut(traBunga,ptr,lenBunga);
			ptr+=lenBunga;

			mapPut(traPenaltyFee,ptr,lenPenaltyFee);
			ptr+=lenPenaltyFee;
			break;

		case trtPembayaranZakat:
			mapPut(traNomorRekeningTujuanBRI,ptr,lenNomorRekeningBRI);
			ptr+=(lenNomorRekeningBRI+1);

			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			ptr = val+60;
			break;

		case trtSetorPasti:
			MAPGETBYTE(traKategoriKartu,jenis,lblKO);
     			if(jenis > 2)
     			{
				memset(buf, 0, sizeof(buf));
				ptr+=60;
				mapPut(traTotalOutstanding,ptr,lenTotalOutstanding);
				mapGet(traTotalOutstanding,buf,lenTotalOutstanding);
				if(
					(strcmp(buf, "00") == 0)||
					(strcmp(buf, "01") == 0)||
					(strcmp(buf, "02") == 0)||
					(strcmp(buf, "03") == 0)
				)
				{
					ptr -=60;
					mapPut(traNomorRekeningTujuanBRI,ptr,14);
					ptr+=(14);
					ptr+=3;

					mapPut(traNomorRekeningAsalBRI,ptr,lenNomorRekeningBRI);
					ptr+=(lenNomorRekeningBRI);
					ptr+=9;

					mapPut(traNamaBankAsal,ptr,lenNamaBank);
					ptr+=(lenNamaBank);

					ptr+=4;

					mapPut(traTotalOutstanding,ptr,lenTotalOutstanding);
					ptr+=(lenTotalOutstanding);

					ptr+=1;

					mapPut(traNamaBankTujuan,ptr,lenNamaBank);
					ptr+=(lenNamaBank);

					mapPut(traNomorRekeningTujuan,ptr,lenNomorRekeningBRI);
					ptr+=(lenNomorRekeningBRI);
				}
     			}
			else
			{
				mapPut(traNomorRekeningTujuanBRI,ptr,lenNomorRekeningBRI);
				ptr+=(lenNomorRekeningBRI);

				mapPut(traNomorRekeningAsalBRI,ptr,lenNomorRekeningBRI);
				ptr+=(lenNomorRekeningBRI);

				ptr+=30;

				mapPut(traNamaBankAsal,ptr,lenNamaBank);
				ptr+=(lenNamaBank);

				mapPut(traNomorRekeningAsal,ptr,lenNomorRekening);
				ptr+=(lenNomorRekening);
			}
			break;

		case trtSetorSimpananInquiry:
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			break;

		case trtSetorPinjamanInquiry:
			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr+=lenNamaPemilik;
			break;

		case trtTarikTunai:
		case trtVoidTarikTunai:
			mapPut(traSisaSaldo,ptr,lenSisaSaldo);
			ptr+=lenSisaSaldo;

			mapPut(traFee,ptr,lenFee);
			ptr= val+46;
			break;

		case trtTCash:
			break;

		case trtTCashOut:
			break;

		case trtRegistrasiInternetBanking:
			mapPut(traNoKartuInternetBanking,ptr,lenNoKartuInternetBanking);
			ptr+=lenNoKartuInternetBanking;

			mapPut(traUserId,ptr,lenUserId);
			ptr=val+35;
			break;

		case trtAktivasiKartuInquiry:
		case trtRenewalKartuInquiry:
			mapPut(traNomorRekeningAsalBRI,ptr,lenNomorRekeningBRI);
			ptr+=lenNomorRekeningBRI;
			ptr++;

			mapPut(traTanggalLahir,ptr,lenTanggalLahir);
			ptr+=(lenTanggalLahir+36);

			mapPut(traNamaPenerima,ptr,lenNamaPemilik);
			ptr = val+120;
			break;
			case trtRenewalKartu:
			ptr+=3;
			mapPut(traNomorKartuLama,ptr,lenNomorKartuLama);
			ptr+=lenNomorKartuLama;

			mapPut(traKodeUkerLama,ptr,lenKodeUker);
			ptr+=lenKodeUker;

			mapPut(traKodeUkerBaru,ptr,lenKodeUker);
			ptr+=lenKodeUker;

			mapPut(traNomorRekeningAsal,ptr,lenNomorRekening);
			ptr+=lenNomorRekeningBRI;

			mapPut(traStatusKartu,ptr,lenStatusKartu);
			ptr+=lenStatusKartu;

			mapPut(traNoTelpon,ptr,lenNoTelpon);
			ptr+=lenNoTelpon;
			break;

		case trtPembayaranPulsa:
			memset(data,0,sizeof(data));
			mapGet(traSubType,data,1);
			switch(data[0]/10)
			{
				case 1:
				case 2:
				case 3:
					ret = mapPut(traNoTelpon, ptr, lenNoTelpon);
					ptr+= lenNoTelpon;
					mapPut(traNominalVoucher, ptr, lenNominalVoucher);
					ptr+=lenNominalVoucher;
					mapPut(traPeriode, ptr, lenPeriode);
					ptr+=lenPeriode;
					mapPut(traNoVoucher, ptr, lenNoVoucher);
					ptr+=lenNoVoucher;
					break;
				case 4:
					ptr+=20;
					memset(data,0,sizeof(data));
					memcpy(data,ptr,16);
					trimLeft(data,'0');
					if(memcmp(data,"62",2)==0)
					{
						memmove(data,&data[1],14);
						data[0] = '0';
					}

					ret = mapPut(traNoTelpon, data, lenNoTelpon);
					ptr+= 16;
					mapPut(traNominalVoucher, ptr, 8);
					ptr+=8;
					mapPut(traNoVoucherMitra, ptr, lenNoVoucherMitra);///<Nomor voucher
					ptr+=lenNoVoucherMitra;
					break;
			}
			break;

		case trtPembayaranPascaBayarInquiry:
			mapPut(traField48,ptr,len);

			switch(subType)
			{
				case PASCA_BAYAR_HALO:
					mapPut(traNoTelpon,ptr,lenNoTelpon);
					ptr+=13;
					ptr+=11;

					mapPut(traNamaPenerima,ptr,lenNamaPemilik);
					ptr+=lenNamaPemilik;

					mapPut(traTotalAmount,ptr,lenTotalAmount);
					ptr+=lenTotalAmount;
					break;

				case PASCA_BAYAR_MATRIX:
					ptr+=12;

					mapPut(traTotalAmount,ptr,lenTotalAmount);
					ptr+=lenTotalAmount;

					ptr+=11;
					ptr+=12;

					mapPut(traNamaPenerima,ptr,lenNamaPemilik);
					ptr+=lenNamaPemilik;
					break;
			}
			break;

		case trtReissuePIN:
			ptr+=16; // no kartu petugas
			mapPut(traNomorRekeningAsalBRI,ptr,lenNomorRekeningBRI);
			break;
#ifdef PREPAID
		case trtPrepaidInfoDeposit:
			ptr+=6;
			mapPut(TRA_SALDO_DEPOSIT,ptr,AMOUNT_LEN_L);
			break;

		case trtPrepaidTopUp:
			memset(data,0,sizeof(data));
			hex2bin(data,ptr,KEY_HOST_LEN);
			mapPut(TRA_KEY_HOST,data,LEN_TRA_KEY_HOST);
			break;

		case trtPrepaidAktivasi:
			memset(data,0,sizeof(data));
			hex2bin(data,ptr,KEY_HOST_LEN);
			mapPut(TRA_KEY_HOST,data,LEN_TRA_KEY_HOST);
			ptr+=48;

			memset(data,0,sizeof(data));
			memcpy(data,ptr,SISA_SALDO_LEN);
			data[SISA_SALDO_LEN-2]=0;
			fmtPad(data,-SISA_SALDO_LEN,'0');
			mapPut(TRA_SISA_SALDO_DEPOSIT,data,LEN_TRA_SISA_SALDO_DEPOSIT);
			break;

		case trtPrepaidRedeemInquiry:
			//saldo kartu
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			mapPut(TRA_SISA_SALDO,buf,AMOUNT_LEN_L);

			fmtPad(buf,-12,'0');
			MAPPUTSTR(traAmt,buf,lblKO);
			ptr+=AMOUNT_LEN_L;

			//deposit balance
			mapPut(TRA_SALDO_DEPOSIT,ptr,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			break ;

		case trtPrepaidRedeem:
			//curr balance
			mapPut(TRA_SISA_SALDO,ptr,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//deposit balance
			mapPut(TRA_SALDO_DEPOSIT,ptr,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//fee
			mapPut(TRA_FEE,ptr,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//total nominal redeem
			mapPut(TRA_TOTAL_REDEEM,ptr,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;
			break ;

			//++ @agmr - brizzi2
		case trtPrepaidReaktivasiInquiry:
			mapPut(traField48,ptr,len);
			mapPutCard(traField48Len, len);

			//nomor kartu
			mapPut(TRA_NOMOR_KARTU_DARI_HOST,ptr,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;

			//saldo kartu
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			ret = mapPut(TRA_SISA_SALDO,buf,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//biaya admin
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			ret = mapPut(TRA_BIAYA_ADMIN,buf,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//saldo deposit
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			ret = mapPut(TRA_SALDO_DEPOSIT,buf,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//lama pasif
			ret = mapPut(TRA_LAMA_PASIF,ptr,LAMA_PASIF_LEN);
			ptr+=LAMA_PASIF_LEN;

			//status kartu seteleh reaktivasi
			ret = mapPut(TRA_STATUS_KARTU,ptr,STATUS_KARTU_LEN);
			ptr+=STATUS_KARTU_LEN;
			break;


		case trtPrepaidReaktivasi:
			//nomor kartu
			mapPut(TRA_NOMOR_KARTU_DARI_HOST,ptr,NOMOR_KARTU_BRIZZI_LEN);
			ptr+=NOMOR_KARTU_BRIZZI_LEN;

			//saldo kartu
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			ret = mapPut(TRA_SISA_SALDO,buf,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//biaya admin
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			ret = mapPut(TRA_BIAYA_ADMIN,buf,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//saldo deposit
			memset(buf,0,sizeof(buf));
			memcpy(buf,ptr,AMOUNT_LEN_L);
			buf[AMOUNT_LEN_L-2]=0;
			fmtPad(buf,-AMOUNT_LEN_L,'0');
			mapPut(TRA_SALDO_DEPOSIT,buf,AMOUNT_LEN_L);
			ptr+=AMOUNT_LEN_L;

			//status kartu seteleh reaktivasi
			mapPut(TRA_STATUS_KARTU,ptr,STATUS_KARTU_LEN);
			ptr+=STATUS_KARTU_LEN;

			//lama pasif
			mapPut(TRA_LAMA_PASIF,ptr,LAMA_PASIF_LEN);
			ptr+=LAMA_PASIF_LEN;

			break;

		case trtPrepaidVoidInquiry:
			break;

		case trtPrepaidVoid:
			memset(data,0,sizeof(data));
			hex2bin(data,ptr,KEY_HOST_LEN);
			mapPut(TRA_KEY_HOST,data,LEN_TRA_KEY_HOST);
			break;
			//-- @agmr - brizzi2

#endif
		default:
			break;
	}

	return 0;

	lblKO:
		return -1;
}

char scrollDisplay8(char (*msg)[22],byte jumlahMsg, byte jumlahBaris,char* atas, char* bawah)
{
	typedef struct t_hal
	{
		char* brs1;
		char* brs2;
		char* brs3;
		char* brs4;
		char* brs5;
		char* brs6;
		char* brs7;
		char* brs8;

	}  t_hal;

	t_hal 	halaman[30];
	byte 	idxHal=0;
	byte 	nHalaman;
	char 	key;
	int 		baris=0;
	byte 	i;
	char 	ret;

	nHalaman = jumlahMsg/jumlahBaris;
	if(jumlahMsg%jumlahBaris)
		nHalaman++;

	memset(halaman,0,sizeof(halaman));

	for(i = 0; i < nHalaman; i++)
	{
		if(baris == jumlahMsg)
			break;
		if(atas)
			halaman[i].brs1 = NULL;
		else
			halaman[i].brs1 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs2 = "";
			baris++;
			halaman[i].brs3 = "";
			baris++;
			halaman[i].brs4 = "";
			baris++;
			halaman[i].brs5 = "";
			baris++;
			halaman[i].brs6 = "";
			baris++;
			halaman[i].brs7 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs8 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs2 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs3 = "";
			baris++;
			halaman[i].brs4 = "";
			baris++;
			halaman[i].brs5 = "";
			baris++;
			halaman[i].brs6 = "";
			baris++;
			halaman[i].brs7 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs8 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs3 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs4 = "";
			baris++;
			halaman[i].brs5 = "";
			baris++;
			halaman[i].brs6 = "";
			baris++;
			halaman[i].brs7 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs8 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs4 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs5 = "";
			baris++;
			halaman[i].brs6 = "";
			baris++;
			halaman[i].brs7 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs8 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs5 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs6 = "";
			baris++;
			halaman[i].brs7 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs8 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs6 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs7 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs8 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs7 = msg[baris++];

		if(bawah)
			halaman[i].brs8 = NULL;
		else
		{
			if(baris == jumlahMsg)
			{
				halaman[i].brs8 = "";
				baris++;
				break;
			}
			else
				halaman[i].brs8 = msg[baris++];
		}
	}
	dspClear();

	if(atas)
		display_text(0,0,atas,dsp_ALIGN_CENTER);
	if(bawah)
		display_text(7,0,bawah,dsp_ALIGN_CENTER);

	idxHal = 0;
	if(halaman[idxHal].brs1 != NULL)
		display_text(0,0,halaman[idxHal].brs1,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs2 != NULL)
		display_text(1,0,halaman[idxHal].brs2,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs3 != NULL)
		display_text(2,0,halaman[idxHal].brs3,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs4 != NULL)
		display_text(3,0,halaman[idxHal].brs4,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs5 != NULL)
		display_text(4,0,halaman[idxHal].brs5,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs6 != NULL)
		display_text(5,0,halaman[idxHal].brs6,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs7 != NULL)
		display_text(6,0,halaman[idxHal].brs7,dsp_ALIGN_LEFT);
	if(halaman[idxHal].brs8 != NULL)
		display_text(7,0,halaman[idxHal].brs8,dsp_ALIGN_LEFT);

	kbdStart(1);
	key = 0;

	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdUP:
				if(idxHal == 0)
					continue;
				idxHal--;
				break;

			case kbdDN:
				if(idxHal == nHalaman-1)
					continue;
				idxHal++;
				break;

			case kbdF4:
			case kbdVAL:
				ret = kbdVAL;
				goto lblEnd;
				break;

			case kbdF1:
			case kbdANN:
			ret = kbdANN;
			goto lblEnd;
			break;

		}
		dspClear();
		if(atas)
			display_text(0,0,atas,dsp_ALIGN_CENTER);
		if(bawah)
			display_text(7,0,bawah,dsp_ALIGN_CENTER);
		if(halaman[idxHal].brs1 != NULL)
			display_text(0,0,halaman[idxHal].brs1,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs2 != NULL)
			display_text(1,0,halaman[idxHal].brs2,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs3 != NULL)
			display_text(2,0,halaman[idxHal].brs3,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs4 != NULL)
			display_text(3,0,halaman[idxHal].brs4,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs5 != NULL)
			display_text(4,0,halaman[idxHal].brs5,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs6 != NULL)
			display_text(5,0,halaman[idxHal].brs6,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs7 != NULL)
			display_text(6,0,halaman[idxHal].brs7,dsp_ALIGN_LEFT);
		if(halaman[idxHal].brs8 != NULL)
			display_text(7,0,halaman[idxHal].brs8,dsp_ALIGN_LEFT);
	}
	return 0;

	lblEnd:
		kbdStop();
		return ret;
}



char scrollDisplay(char (*msg)[21],byte jumlahMsg, byte jumlahBaris,char* atas, char* bawah)
{
	typedef struct t_hal
	{
		char* brs1;
		char* brs2;
		char* brs3;
		char* brs4;
	}  t_hal;

	t_hal 	halaman[10];
	byte 	idxHal=0;
	byte 	nHalaman;
	char 	key;
	int 		baris=0;
	byte 	a,b,i/*,idx*/;
	char 	ret;

	nHalaman = jumlahMsg/jumlahBaris;
	if(jumlahMsg%jumlahBaris)
		nHalaman++;

	if(atas)
		a = 2;
	else
		a = 1;

	if(bawah)
		b = 3;
	else
		b = 4;

	memset(halaman,0,sizeof(halaman));

	for(i = 0; i < nHalaman; i++)
	{
		if(baris == jumlahMsg)
			break;
		if(atas)
			halaman[i].brs1 = NULL;
		else
			halaman[i].brs1 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs2 = "";
			baris++;
			halaman[i].brs3 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs4 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs2 = msg[baris++];

		if(baris == jumlahMsg)
		{
			halaman[i].brs3 = "";
			baris++;
			if(!bawah)
			{
				halaman[i].brs4 = "";
				baris++;
			}
			break;
		}
		else
			halaman[i].brs3 = msg[baris++];

		if(bawah)
			halaman[i].brs4 = NULL;
		else
		{
			if(baris == jumlahMsg)
			{
				halaman[i].brs4 = "";
				baris++;
				break;
			}
			else
				halaman[i].brs4 = msg[baris++];
		}
	}
	dspClear();

	if(atas)
		dspLS(0|0x40,atas);
	if(bawah)
		dspLS(3,bawah);

	idxHal = 0;

	if(halaman[idxHal].brs1 != NULL)
		dspLS(0,halaman[idxHal].brs1);
	if(halaman[idxHal].brs2 != NULL)
		dspLS(1,halaman[idxHal].brs2);
	if(halaman[idxHal].brs3 != NULL)
		dspLS(2,halaman[idxHal].brs3);
	if(halaman[idxHal].brs4 != NULL)
		dspLS(3,halaman[idxHal].brs4);

	kbdStart(1);
	key = 0;


	while(1)
	{
		key = kbdKey();
		switch(key)
		{
			case kbdUP:
				if(idxHal == 0)
					continue;
				idxHal--;
				if(halaman[idxHal].brs1 != NULL)
					dspLS(0,halaman[idxHal].brs1);
				if(halaman[idxHal].brs2 != NULL)
					dspLS(1,halaman[idxHal].brs2);
				if(halaman[idxHal].brs3 != NULL)
					dspLS(2,halaman[idxHal].brs3);
				if(halaman[idxHal].brs4 != NULL)
					dspLS(3,halaman[idxHal].brs4);
				break;

			case kbdDN:
				if(idxHal == nHalaman-1)
					continue;
				idxHal++;
				if(halaman[idxHal].brs1 != NULL)
					dspLS(0,halaman[idxHal].brs1);
				if(halaman[idxHal].brs2 != NULL)
					dspLS(1,halaman[idxHal].brs2);
				if(halaman[idxHal].brs3 != NULL)
					dspLS(2,halaman[idxHal].brs3);
				if(halaman[idxHal].brs4 != NULL)
					dspLS(3,halaman[idxHal].brs4);
				break;

			case kbdF4:
			case kbdVAL:
				ret = kbdVAL;
				goto lblEnd;
				break;

			case kbdF1:
			case kbdANN:
				ret = kbdANN;
				goto lblEnd;
				break;

		}
	}
	return 0;

	lblEnd:
		kbdStop();
		return ret;
}

int downloadKey(byte* msg, int msgLen)
{

	int 	ret;

	if(openCom()==-1)
		return -1;

	ret = LL_Receive(prepaidCom.prm.hdl, msgLen, msg, 5*100);
	ttestall(0, 100);

	if(ret <=0)
	{
		closeCom();
		return -1;
	}

	closeCom();
	return ret;

}

int displayLongText(char *txtInput, int txtLen, char* baris1, char* baris2, int maxKolom)
{
	memset(baris1,0,maxKolom+1);
	memset(baris2,0,maxKolom+1);
	if(txtLen > maxKolom)
	{
		memcpy(baris1,txtInput,maxKolom);
		if(txtLen > 2*maxKolom)
			memcpy(baris2,txtInput+maxKolom,maxKolom);
		else
			memcpy(baris2,txtInput+maxKolom,txtLen-maxKolom);
		return 2;
	}
	else
	{
		memcpy(baris1,txtInput,maxKolom);
		return 1;
	}
}

int briLogSave(byte jenisLog)
{
	word 	key;
	word 	idx;
	byte 	buf[256];
	int 	       ret;
	word 	regBriLogIdx,regBriLogSize,briLogBeg, briLogEnd;
	word 	dimBriLog;
	word 	mnuItem;

	trcS("briLogSave Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogIdx  = regCLogIdx;
			regBriLogSize = regCLogSize;
			briLogBeg = cLogBeg;
			briLogEnd = cLogEnd;
			dimBriLog = dimCLog;
			break;

		case MINI_ATM_LOG:
			regBriLogIdx  = regMLogIdx;
			regBriLogSize = regMLogSize;
			briLogBeg = mLogBeg;
			briLogEnd = mLogEnd;
			dimBriLog = dimMLog;
			break;

		case TUNAI_LOG:
			regBriLogIdx  = regTLogIdx;
			regBriLogSize = regTLogSize;
			briLogBeg = tLogBeg;
			briLogEnd = tLogEnd;
			dimBriLog = dimTLog;
			break;

		case TCASH_LOG:
			regBriLogIdx  = regTCashLogIdx;
			regBriLogSize = regTCashLogSize;
			briLogBeg = tCashLogBeg;
			briLogEnd = tCashLogEnd;
			dimBriLog = dimTCashLog;
			break;

		default:
			return -1;
			break;

	}

	MAPGETWORD(regBriLogIdx,idx,lblDBA);
	mapMove(briLogBeg, idx);

	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);

		if(jenisLog == CARD_SERVICE_LOG)
		{
			MAPGET(cLogKey[idx], buf, lblDBA);
		}
		else if(jenisLog == MINI_ATM_LOG)
		{
			MAPGET(mLogKey[idx], buf, lblDBA);
		}
		else if(jenisLog == TCASH_LOG)
		{
			MAPGET(tCashLogKey[idx], buf, lblDBA);;
		}
		else
		{
			MAPGET(tLogKey[idx], buf, lblDBA);
		}

		ret = mapPut(key, buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}

	MAPGETWORD(traMnuItm, mnuItem, lblDBA);

	ret = incCard(regBriLogIdx);
	CHK;


	MAPGETWORD(regBriLogIdx,idx,lblDBA);
	if(idx >= dimBriLog-1)
	{
		idx = 0;
		MAPPUTBYTE(regBriLogIdx,idx,lblDBA);
	}

	MAPGETWORD(regBriLogSize,idx,lblDBA);
	if(idx <= dimBriLog-1)
	{
		ret = incCard(regBriLogSize);
		CHK;
	}

	ret = 1;
	goto lblEnd;

	lblDBA:
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("revSave: ret=%d \n", ret);
		return ret;
}


int briLogUpdate(byte jenisLog)
{
	word 	key, trxNo;
	byte 	idx;
	byte 	logIdx;
	byte 	buf[256];
	char 	rocRef[lenInvNum + 1];
	card 	rocNum, logRoc;
	int 		ret;
	word 	regBriLogSize,briLogBeg, briLogEnd, brilogROC;

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize = regCLogSize;
			briLogBeg = cLogBeg;
			briLogEnd = cLogEnd;
			brilogROC = cLogROC;
			break;

		case MINI_ATM_LOG:
			regBriLogSize = regMLogSize;
			briLogBeg = mLogBeg;
			briLogEnd = mLogEnd;
			brilogROC = mLogROC;
			break;

		case TUNAI_LOG:
			regBriLogSize = regTLogSize;
			briLogBeg = tLogBeg;
			briLogEnd = tLogEnd;
			brilogROC = tLogROC;
			break;

		case TCASH_LOG:
			regBriLogSize = regTCashLogSize;
			briLogBeg = tCashLogBeg;
			briLogEnd = tCashLogEnd;
			brilogROC = tCashLogROC;
			break;

		default:
			return -1;
			break;

	}

	MAPGETWORD(regBriLogSize, trxNo, lblDBA);
	MAPGET(traFindRoc, rocRef, lblDBA);
	dec2num(&rocNum, rocRef, sizeof(rocRef));

	for (logIdx = 0; logIdx < trxNo; logIdx++)
	{
		mapMove(briLogBeg, logIdx);
		switch(jenisLog)
		{
			case CARD_SERVICE_LOG:
				MAPGETCARD(cLogROC, logRoc, lblDBA);
				break;
			case MINI_ATM_LOG:
				MAPGETCARD(mLogROC, logRoc, lblDBA);
				break;
			case TUNAI_LOG:
				MAPGETCARD(tLogROC, logRoc, lblDBA);
				break;
			case TCASH_LOG:
				MAPGETCARD(tCashLogROC, logRoc, lblDBA);
				break;
		}

		if(logRoc == rocNum)
			break;
	}
	CHECK(logIdx <= trxNo, lblDBA);

	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);
		switch(jenisLog)
		{
			case CARD_SERVICE_LOG:
				MAPGET(cLogKey[idx], buf, lblDBA);
				break;

			case MINI_ATM_LOG:
				MAPGET(mLogKey[idx], buf, lblDBA);
				break;

			case TUNAI_LOG:
				MAPGET(tLogKey[idx], buf, lblDBA);
				break;

			case TCASH_LOG:
				MAPGET(tCashLogKey[idx], buf, lblDBA);
				break;
		}

		ret = mapPut(key, buf, mapDatLen(key));
		CHECK(ret >= 0, lblDBA);
	}

	ret = 1;
	goto lblEnd;

	lblDBA:
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("logUpdate: ret=%d #\n", ret);
		return ret;
}

int briLogLoadByRoc(byte jenisLog)
{
	word 	key, trxNo;
	word 	idx;
	word 	logIdx;
	byte 	buf[256];
	char 	rocRef[lenInvNum + 1];
	card 	rocNum, logRoc;
	int 		ret;
	word 	briLogBeg, briLogEnd;
	card 	brilogROC;
	word 	regBriLogSize;
	card 	dupStan, dupROC;

	trcS("briLogLoad Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize = regCLogSize;
			briLogBeg = cLogBeg;
			briLogEnd = cLogEnd;
			brilogROC = cLogROC;
			break;

		case MINI_ATM_LOG:
			regBriLogSize = regMLogSize;
			briLogBeg = mLogBeg;
			briLogEnd = mLogEnd;
			brilogROC = mLogROC;
			break;

		case TUNAI_LOG:
			regBriLogSize = regTLogSize;
			briLogBeg = tLogBeg;
			briLogEnd = tLogEnd;
			brilogROC = tLogROC;
			break;

		case TCASH_LOG:
			regBriLogSize = regTCashLogSize;
			briLogBeg = tCashLogBeg;
			briLogEnd = tCashLogEnd;
			brilogROC = tCashLogROC;
			break;

		default:
			return -1;
			break;

	}

	MAPGETWORD(regBriLogSize, trxNo, lblDBA);
	MAPGET(traFindRoc, rocRef, lblDBA);
	dec2num(&rocNum, rocRef, sizeof(rocRef));

	for (logIdx = 0; logIdx < trxNo; logIdx++)
	{
		mapMove(briLogBeg, logIdx);
		MAPGETCARD(brilogROC, logRoc, lblDBA);
		if(logRoc == rocNum)
			break;
	}
	CHECK(logIdx < trxNo, lblTraceNotFound);

	MAPGETCARD(regSTAN, dupStan, lblDBA);
	MAPPUTCARD(regDupSTAN, dupStan, lblDBA);
	MAPGETCARD(regInvNum, dupROC, lblDBA);
	MAPPUTCARD(regDupInvNum, dupROC+1000000, lblDBA);

	ret = briLogSaveCurContext(jenisLog);
	CHECK(ret >= 0, lblDBA);
	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);
		ret = mapGet(key, buf, sizeof(buf));
		CHECK(ret >= 0, lblDBA);

		if(jenisLog == CARD_SERVICE_LOG)
			ret = mapPut(cLogKey[idx], buf, sizeof(buf));
		else if(jenisLog == MINI_ATM_LOG)
			ret = mapPut(mLogKey[idx], buf, sizeof(buf));
		else if(jenisLog == TCASH_LOG)
			ret = mapPut(tCashLogKey[idx], buf, sizeof(buf));
		else
			ret = mapPut(tLogKey[idx], buf, sizeof(buf));

		CHECK(ret >= 0, lblDBA);
	}

	ret = 1;
	goto lblEnd;

	lblTraceNotFound:
		ret = 0;
		usrInfo(infTraceNotFound);
		goto lblEnd;
	lblDBA:
		trcErr(ret);
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("logLoad: ret=%d \n", ret);
		return ret;
}

int briLogLoadByIndex(byte jenisLog,card index)
{
	word	key;
	word 	idx;
	byte 	buf[256];
	int 		ret;
	word 	briLogBeg, briLogEnd, briLogDatTim;
	word 	regBriLogSize;

	trcS("briLogLoad Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize = regCLogSize;
			briLogBeg = cLogBeg;
			briLogEnd = cLogEnd;
			briLogDatTim = cLogDatTim;
			break;

		case MINI_ATM_LOG:
			regBriLogSize = regMLogSize;
			briLogBeg = mLogBeg;
			briLogEnd = mLogEnd;
			briLogDatTim = mLogDatTim;
			break;

		case TUNAI_LOG:
			regBriLogSize = regTLogSize;
			briLogBeg = tLogBeg;
			briLogEnd = tLogEnd;
			briLogDatTim = tLogDatTim;
			break;

		case TCASH_LOG:
			regBriLogSize = regTCashLogSize;
			briLogBeg = tCashLogBeg;
			briLogEnd = tCashLogEnd;
			briLogDatTim = tCashLogDatTim;
			break;

		default:
			return -1;
			break;
	}

	ret = mapMove(briLogBeg, index);
	if(ret < 0)
		goto lblTraceNotFound;

	CHECK(ret >= 0, lblDBA);
	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);
		ret = mapGet(key, buf, sizeof(buf));
		CHECK(ret >= 0, lblDBA);

		if(jenisLog == CARD_SERVICE_LOG)
			ret = mapPut(cLogKey[idx], buf, sizeof(buf));
		else if(jenisLog == MINI_ATM_LOG)
			ret = mapPut(mLogKey[idx], buf, sizeof(buf));
		else if(jenisLog == TCASH_LOG)
			ret = mapPut(tCashLogKey[idx], buf, sizeof(buf));
		else
			ret = mapPut(tLogKey[idx], buf, sizeof(buf));

		CHECK(ret >= 0, lblDBA);
	}

	{
		word tmp,tmp1;

		memset(buf,0,sizeof(buf));
		mapGet(traMnuItm,&tmp,sizeof(word));
		mapGet(regInvNum,&tmp1,sizeof(word));
		sprintf(buf,"mnu=%i %i",tmp,tmp1);
		prtS(buf);
	}
	ret = 1;
	goto lblEnd;

	lblTraceNotFound:
		ret = 0;
		usrInfo(infTraceNotFound);
		goto lblEnd;
	lblDBA:
		trcErr(ret);
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("logLoad: ret=%d \n", ret);
		return ret;
}

int briLogLoadByDate(byte jenisLog, char *dateTime)
{
	word 	key, trxNo;
	word 	idx;
	word 	logIdx;
	byte 	buf[256];
	int 		ret;
	word 	briLogBeg, briLogEnd, briLogDatTim;

	trcS("briLogLoad Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			MAPGETWORD(regCLogSize, trxNo, lblDBA);
			briLogBeg = cLogBeg;
			briLogEnd = cLogEnd;
			briLogDatTim = cLogDatTim;
			break;

		case MINI_ATM_LOG:
			MAPGETWORD(regMLogSize, trxNo, lblDBA);
			briLogBeg = mLogBeg;
			briLogEnd = mLogEnd;
			briLogDatTim = mLogDatTim;
			break;

		case TUNAI_LOG:
			MAPGETWORD(regTLogSize, trxNo, lblDBA);
			briLogBeg = tLogBeg;
			briLogEnd = tLogEnd;
			briLogDatTim = tLogDatTim;
			break;

		case TCASH_LOG:
			MAPGETWORD(regTCashLogSize, trxNo, lblDBA);
			briLogBeg = tCashLogBeg;
			briLogEnd = tCashLogEnd;
			briLogDatTim = tCashLogDatTim;
			break;

		default:
			return -1;
			break;
	}

	for (logIdx = 0; logIdx < trxNo; logIdx++)
	{
		mapMove(briLogBeg, logIdx);
		mapGet(briLogDatTim, buf, sizeof(buf));
		if(memcmp(buf,dateTime,8) == 0)
			break;
	}
	CHECK(logIdx < trxNo, lblTraceNotFound);

	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);
		ret = mapGet(key, buf, sizeof(buf));
		CHECK(ret >= 0, lblDBA);

		if(jenisLog == CARD_SERVICE_LOG)
			ret = mapPut(cLogKey[idx], buf, sizeof(buf));
		else if(jenisLog == MINI_ATM_LOG)
			ret = mapPut(mLogKey[idx], buf, sizeof(buf));
		else if(jenisLog == TCASH_LOG)
			ret = mapPut(tCashLogKey[idx], buf, sizeof(buf));
		else
			ret = mapPut(tLogKey[idx], buf, sizeof(buf));

		CHECK(ret >= 0, lblDBA);
	}

	ret = 1;
	goto lblEnd;

	lblTraceNotFound:
		ret = 0;
		usrInfo(infTraceNotFound);
		goto lblEnd;
	lblDBA:
		trcErr(ret);
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("logLoad: ret=%d \n", ret);
		return ret;
}


void briViewBat(byte jenisLog, byte posisiLog)
{
	int 		ret, logCount,  key = 1, mnu = 0;
	char 	txnTypeName[20 + 1];
	word 	txnType = 0, trxNum, ifVoid,mnuItm;
	byte 	Exp, voidTxn = 0;
	word 	briLogBeg, briLogEnd;
	word 	regBriLogSize, brilogMnuItm, brilogVoidFlag,regBriLogIdx;
	word 	ptrTrx;
	card 	idxCnt = 0;
	card 	logRoc;
	char 	rocRef[lenInvNum + 1];
	word 	logIdx, idx;
	card 	brilogROC,rocNum;

	trcS("briViewBat Beg\n");
	memset(rocRef, 0x00, sizeof(rocRef));
	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize = regCLogSize;
			regBriLogIdx    = regCLogIdx;
			brilogMnuItm = cLogMnuItm;
			brilogVoidFlag = cLogVoidFlag;
			brilogROC = cLogROC;
			briLogBeg = cLogBeg;
			briLogEnd = cLogEnd;
			break;

		case MINI_ATM_LOG:
			regBriLogSize = regMLogSize;
			regBriLogIdx    = regMLogIdx;
			brilogMnuItm = mLogMnuItm;
			brilogVoidFlag = mLogVoidFlag;
			brilogROC = mLogROC;
			briLogBeg = mLogBeg;
			briLogEnd = mLogEnd;
			break;

		case TUNAI_LOG:
			regBriLogSize = regTLogSize;
			regBriLogIdx    = regTLogIdx;
			brilogMnuItm = tLogMnuItm;
			brilogVoidFlag = tLogVoidFlag;
			brilogROC = tLogROC;
			briLogBeg = tLogBeg;
			briLogEnd = tLogEnd;
			break;

		case TCASH_LOG:
			regBriLogSize = regTCashLogSize;
			regBriLogIdx    = regTCashLogIdx;
			brilogMnuItm = tCashLogMnuItm;
			brilogVoidFlag = tCashLogVoidFlag;
			brilogROC = tCashLogROC;
			briLogBeg = tCashLogBeg;
			briLogEnd = tCashLogEnd;
			break;

		default:
			return;
	}

	MAPGETWORD(regBriLogSize, trxNum, lblKO);
	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	MAPGETBYTE(appExp, Exp, lblKO);
	Exp %= 10;
	CHECK(Exp == 0 || Exp == 2 || Exp == 3, lblKO);

	switch(posisiLog)
	{
		case 0:
			idxCnt = 0;
			break;

		case 1:
			MAPGETWORD(regBriLogIdx, ptrTrx, lblKO);

			if(ptrTrx > 0)
				trxNum = ptrTrx;

			idxCnt = trxNum -1;
			break;

		case 2:
			ret = scrDlg(infEnterTrace, traFindRoc);
			CHECK(ret >= 0, lblKO);
			MAPGET(traFindRoc, rocRef, lblKO);
			dec2num(&rocNum, rocRef, sizeof(rocRef));

			for (logIdx = 0; logIdx < logCount; logIdx++)
			{
				mapMove(briLogBeg, logIdx);
				MAPGETCARD(brilogROC, logRoc, lblKO);
				if(logRoc == rocNum)
					break;
			}
			CHECK(logIdx < trxNum, lblTraceNotFound);
			idxCnt = logIdx;
			break;

		default:
			goto lblKO;
	}

	while(key)
	{
		mapMove(briLogBeg,  idxCnt);

		MAPGETCARD(brilogMnuItm, mnuItm, lblKO);
		getBriTransTypeName(&txnType,NULL,mnuItm);
		MAPGETBYTE(brilogVoidFlag, voidTxn, lblKO);

		trcFN("idx: %d\n", idx);
		VERIFY(rqtBeg < idx);
		VERIFY(idx < rqtEnd);
		mapMove(rqsBeg, (word) (idx - 1));

		if(voidTxn)
			ifVoid = rqsVDisp;
		else
			ifVoid = rqsDisp;

		MAPGET(ifVoid, txnTypeName, lblKO);

		switch (mnu)
		{
			case 0:
				viewBatch1(jenisLog,1);
				break;

			case 1:
				viewBatch2(jenisLog,1);
				break;
		}


		ret = tmrStart(0, 30 * 100);
		CHECK(ret >= 0, lblKO);
		ret = kbdStart(1);
		CHECK(ret >= 0, lblKO);
		key = 0;
		while(tmrGet(0)) {
			key = kbdKey();
			if(key)
				break;
		}
		ret = kbdStop();

		switch (key)
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

			case kbdDN:
				idxCnt--;
				break;

			case kbdUP:
				mnu++;
				break;

			case kbdF3:
				idxCnt++;
				break;
#endif

			case kbdANN:
			case 0:
				goto lblEnd;

		}
		CHECK(key >= 0, lblKO);

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
	lblTraceNotFound:
		usrInfo(infTraceNotFound);
		goto lblEnd;
	lblKO:
		usrInfo(infProcessingError);
		goto lblEnd;
	lblEnd:
		trcS("admViewBat End\n");
}

int viewBatch1(byte jenisLog,byte flag)
{
	char 	buf[50], buf1[50];
	card 	invNum, briLogROC, briLogAmt;
	int 		ret;
	word 	mnuItem;

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			strcpy(buf,"CARD SERVICE");
			briLogROC = cLogROC;
			briLogAmt = cLogTraAmt;
			MAPGETCARD(cLogMnuItm, mnuItem, lblDBA);
			break;

		case MINI_ATM_LOG:
			strcpy(buf,"MINI ATM");
			briLogROC = mLogROC;
			briLogAmt = mLogTraAmt;
			MAPGETCARD(mLogMnuItm, mnuItem, lblDBA);
			break;

		case TUNAI_LOG:
			strcpy(buf,"TUNAI");
			briLogROC = tLogROC;
			briLogAmt = tLogTraAmt;
			MAPGETCARD(tLogMnuItm, mnuItem, lblDBA);
			break;

		case TCASH_LOG:
			strcpy(buf,"T-CASH");
			briLogROC = tCashLogROC;
			briLogAmt = tCashLogTraAmt;
			MAPGETCARD(tCashLogMnuItm, mnuItem, lblDBA);
			break;

		case NORMAL_LOG:
			if(flag == 0)
				strcpy(buf,"VIEW TXN");
			else
				strcpy(buf,"VIEW BATCH");
			briLogROC = logROC;
			briLogAmt = logTraAmt;
			MAPGETCARD(logMnuItm, mnuItem, lblDBA);
			break;
	}
	dspClear();
	display_text(0,0,buf,dsp_ALIGN_CENTER);

	//invoice number / ROC
	MAPGETCARD(briLogROC,invNum,lblDBA);
	memset(buf,0,sizeof(buf));
	sprintf(buf,"LOG:       REF %06lu",invNum);
	display_text(3,0,buf,dsp_ALIGN_LEFT);

	//Jenis transaksi
	memset(buf,0,sizeof(buf));
	getBriTransTypeName(NULL, buf, mnuItem); //@agmr BRI2
	display_text(4,0,buf,dsp_ALIGN_LEFT);

	//Amount
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	ret = mapGet(briLogAmt,buf1,sizeof(buf1));
	if(ret <0)
		goto lblDBA;
	strcpy(buf,"Rp. ");
	fmtAmt(buf+4,buf1,0,",.");
	display_text(5,0,buf,dsp_ALIGN_RIGHT);

	//PREV NEXT
	if(flag == 0)
		display_text(7,0,"MORE",dsp_ALIGN_CENTER);
	else
		display_text(7,0,"PREV     MORE    NEXT",dsp_ALIGN_CENTER);
	goto lblEnd;

	lblDBA:
		return -1;
	lblEnd:
		return 1;

}

int viewBatch2(byte jenisLog, byte flag)
{
	char 	buf[50], buf1[50];
	word 	briLogAutCode, briLogPan, briLogDatTim,briLogExpDat;
	int 		ret;

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			strcpy(buf,"CARD SERVICE");
			briLogAutCode = cLogAutCod;
			briLogPan = cLogPan;
			briLogDatTim = cLogDatTim;
			briLogExpDat = cLogExpDat;
			break;

		case MINI_ATM_LOG:
			strcpy(buf,"MINI ATM");
			briLogAutCode = mLogAutCod;
			briLogPan = mLogPan;
			briLogDatTim = mLogDatTim;
			briLogExpDat = mLogExpDat;
			break;

		case TCASH_LOG:
			strcpy(buf,"T-CASH");
			briLogAutCode = tCashLogAutCod;
			briLogPan = tCashLogPan;
			briLogDatTim = tCashLogDatTim;
			briLogExpDat = tCashLogExpDat;
			break;

		case TUNAI_LOG:
			strcpy(buf,"TUNAI");
			briLogAutCode = tLogAutCod;
			briLogPan = tLogPan;
			briLogDatTim = tLogDatTim;
			briLogExpDat = tLogExpDat;
			break;

		case NORMAL_LOG:
			if(flag == 0)
				strcpy(buf,"VIEW TXN");
			else
				strcpy(buf,"VIEW BATCH");
			briLogAutCode = logAutCod;
			briLogPan = logPan;
			briLogDatTim = logDatTim;
			briLogExpDat = logExpDat;
			break;

	}
	dspClear();
	display_text(0,0,buf,dsp_ALIGN_CENTER);

	//APPROVAL CODE
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	ret = mapGet(briLogAutCode,buf1,sizeof(buf1));
	if(ret < 0)
		goto lblDBA;

	sprintf(buf,"APPROVAL CODE: %s",buf1);
	display_text(2,0,buf,dsp_ALIGN_LEFT);

	//PAN
	memset(buf,0,sizeof(buf));
	ret = mapGet(briLogPan,buf,sizeof(buf));
	if(ret < 0)
		goto lblDBA;
	display_text(3,0,buf,dsp_ALIGN_LEFT);

	//EXP DATE
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	ret = mapGet(briLogExpDat,buf1,sizeof(buf1));
	if(ret < 0)
		goto lblDBA;
	sprintf(buf,"EXP. DATE:      %c%c/%c%c",buf1[2],buf1[3],buf1[0],buf1[1]);
	display_text(4,0,buf,dsp_ALIGN_LEFT);

	//DATE TIME
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	ret = mapGet(briLogDatTim,buf1,sizeof(buf1));
	if(ret <0)
	goto lblDBA;

	sprintf(buf,"%c%c/%c%c/%c%c%c%c   %c%c:%c%c:%c%c",buf1[6],buf1[7],buf1[4],buf1[5],buf1[0],buf1[1],
		buf1[2],buf1[3],buf1[8],buf1[9],buf1[10],buf1[11],
		buf1[12],buf1[13]);
	display_text(5,0,buf,dsp_ALIGN_LEFT);

	//PREV NEXT
	if(flag == 0)
		display_text(7,0,"MORE",dsp_ALIGN_CENTER);
	else
		display_text(7,0,"PREV     MORE    NEXT",dsp_ALIGN_CENTER);
	goto lblEnd;

	lblDBA:
		return -1;
	lblEnd:
		return 1;
}

int briLogCalcTotal(byte jenisLog, TOTAL_T *total, char* dateTime)
{
	char 	txnAmt[lenAmt + 1];
	word 	curMnuItem, trxNum;
	int 		ret = 0, ctr = 0;
	word 	regBriLogSize;
	word 	briLogBeg, briLogMnuItm, briLogTraAmt, briLogDatTim;
	byte 	tIndex, tanda=0;
	char 	txnDateTime[lenDatTim+1];

	trcS("briLogCalcTot: Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize = regCLogSize;
			briLogBeg = cLogBeg;
			briLogMnuItm = cLogMnuItm;
			briLogTraAmt = cLogTraAmt;
			briLogDatTim = cLogDatTim;
			break;

		case MINI_ATM_LOG:
			regBriLogSize = regMLogSize;
			briLogBeg = mLogBeg;
			briLogMnuItm = mLogMnuItm;
			briLogTraAmt = mLogTraAmt;
			briLogDatTim = mLogDatTim;
			break;

		case TUNAI_LOG:
			regBriLogSize = regTLogSize;
			briLogBeg = tLogBeg;
			briLogMnuItm = tLogMnuItm;
			briLogTraAmt = tLogTraAmt;
			briLogDatTim = tLogDatTim;
			break;

		case TCASH_LOG:
			regBriLogSize = regTCashLogSize;
			briLogBeg = tCashLogBeg;
			briLogMnuItm = tCashLogMnuItm;
			briLogTraAmt = tCashLogTraAmt;
			briLogDatTim = tCashLogDatTim;
			break;

		case BRIZZI_LOG:
			regBriLogSize = regTrxNo;
			briLogBeg = logBeg;
			briLogMnuItm = logMnuItm;
			briLogTraAmt = logTraAmt;
			briLogDatTim = logDatTim;
			break;
	}

	MAPGETWORD(regBriLogSize, trxNum, lblKO);

	for (ctr = 0; ctr < trxNum; ctr++)
	{
		mapMove(briLogBeg, ctr);
		MAPGETWORD(briLogMnuItm, curMnuItem, lblKO);
		MAPGET(briLogTraAmt, txnAmt, lblKO);
		MAPGET(briLogDatTim, txnDateTime,lblKO);
		if(dateTime != NULL)
		{
			if(memcmp(dateTime,txnDateTime,8) != 0)
			continue;
		}
		tanda=1;

		switch (curMnuItem)
		{
			case mnuPIN:
			case mnuGantiPIN:
				tIndex = GANTI_PIN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"UBAH PIN");
				break;

			case mnuAktivasiKartu:
				tIndex = AKTIVASI_KARTU;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"AKTIVASI KARTU");
				break;

			case mnuRenewalKartu:
				tIndex = RENEWAL_KARTU;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"RENEWAL KARTU");
				break;

			case mnuGantiPasswordSPV:
				tIndex = GANTI_PASSWORD_SPV;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"UBAH PASSWORD SPV");
				break;

			case mnuReissuePIN:
				tIndex = REISSUE_PIN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"REISSUE PIN");
				break;

			case mnuInfoSaldo:
				tIndex = INFO_SALDO;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"INFO SALDO");
				break;

			case mnuInfoSaldoBankLain:
				tIndex = INFO_SALDO_BANK_LAIN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"INFO SALDO BANK LAIN");
				break;

			case mnuMiniStatement:
				tIndex = MINI_STATEMENT;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"MINI STATEMENT");
				break;

			case mnuMutRek:
				tIndex = MUTASI_REKENING;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"MUTASI REKENING");
				break;

			case mnuPrevilege:
				tIndex = PREVILEGE;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PREVILEGE");
				break;


			case mnuTransferSesamaBRI:
				tIndex = TRANSFER_SESAMA_BRI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TRANSFER SESAMA BRI");
				break;

			case mnuTransferAntarBank:
				tIndex = TRANSFER_ANTAR_BANK;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TRANSFER ANTAR BANK");
				break;

			case mnuPembayaranPLNPasca:
				tIndex = PEMBAYARAN_PLN_PASCA;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN PLN PASCA");
				break;

			case mnuPembayaranPLNPra:
				tIndex = PEMBAYARAN_PLN_PRA;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN PLN PRA");
				break;

			case mnuPembayaranPLNToken:
				tIndex = PEMBAYARAN_PLN_TOKEN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"CETAK TOKEN PLN");
				break;

			case mnuPembayaranCicilanFIF:
			case mnuPembayaranCicilanBAF:
			case mnuPembayaranCicilanOTO:
			case mnuPembayaranCicilanFinansia:
			case mnuPembayaranCicilanVerena:
			case mnuPembayaranCicilanWOM:
				tIndex = PEMBAYARAN_CICILAN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"CICILAN");
				break;

			case mnuPembayaranSimpati50:
			case mnuPembayaranSimpati100:
	//		case mnuPembayaranSimpati150:
			case mnuPembayaranSimpati200:
			case mnuPembayaranSimpati300:
			case mnuPembayaranSimpati500:
			case mnuPembayaranSimpati1000:
				tIndex = PEMBAYARAN_SIMPATI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG SIMPATI/AS");
				break;

			case mnuPembayaranMentari25:
			case mnuPembayaranMentari50:
			case mnuPembayaranMentari100:
			case mnuPembayaranMentari250:
			case mnuPembayaranMentari500:
			case mnuPembayaranMentari1000:
				tIndex = PEMBAYARAN_MENTARI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG MENTARI");
				break;

			case mnuPembayaranIm325:
			case mnuPembayaranIm350:
			case mnuPembayaranIm375:
			case mnuPembayaranIm3100:
			case mnuPembayaranIm3150:
			case mnuPembayaranIm3200:
			case mnuPembayaranIm3500:
			case mnuPembayaranIm31000:
				tIndex = PEMBAYARAN_IM3;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG IM3");
				break;

			case mnuPembayaranXl25:
			case mnuPembayaranXl50:
			case mnuPembayaranXl75:
			case mnuPembayaranXl100:
			case mnuPembayaranXl150:
			case mnuPembayaranXl200:
			case mnuPembayaranXl300:
			case mnuPembayaranXl500:
				tIndex = PEMBAYARAN_XL;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG PULSA XL");
				break;

			case mnuPembayaranEsia25:
			case mnuPembayaranEsia50:
			case mnuPembayaranEsia75:
			case mnuPembayaranEsia100:
			case mnuPembayaranEsia150:
			case mnuPembayaranEsia200:
				tIndex = PEMBAYARAN_ESIA;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG PULSA ESIA");
				break;

			case mnuPembayaranSmart20:
			case mnuPembayaranSmart25:
			case mnuPembayaranSmart50:
			case mnuPembayaranSmart100:
			case mnuPembayaranSmart150:
			case mnuPembayaranSmart200:
			case mnuPembayaranSmart300:
			case mnuPembayaranSmart500:
				tIndex = PEMBAYARAN_SMART;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG PULSA SMART");
				break;

			case mnuPembayaranFren20:
			case mnuPembayaranFren25:
			case mnuPembayaranFren50:
			case mnuPembayaranFren100:
			case mnuPembayaranFren150:
			case mnuPembayaranFren200:
			case mnuPembayaranFren300:
			case mnuPembayaranFren500:
				tIndex = PEMBAYARAN_FREN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG PULSA FREN");
				break;

			case mnuPembayaranThree20:
			case mnuPembayaranThree50:
			case mnuPembayaranThree75:
			case mnuPembayaranThree100:
			case mnuPembayaranThree150:
			case mnuPembayaranThree300:
			case mnuPembayaranThree500:
				tIndex = PEMBAYARAN_FREN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG PULSA THREE");
				break;

			case mnuPembayaranAxis25:
			case mnuPembayaranAxis50:
			case mnuPembayaranAxis75:
			case mnuPembayaranAxis100:
			case mnuPembayaranAxis150:
			case mnuPembayaranAxis200:
			case mnuPembayaranAxis300:
				tIndex = PEMBAYARAN_AXIS;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"ISI ULANG PULSA AXIS");
				break;

			case mnuPembayaranKKBRI:
				tIndex = PEMBAYARAN_KK_BRI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KK BRI");
				break;

			case mnuPembayaranKKCITIKK:
				tIndex = PEMBAYARAN_KK_CITI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KK CITIBANK");
				break;

			case mnuPembayaranKKCITIKTA:
				tIndex = PEMBAYARAN_KK_CITI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KTA CITIBANK");
				break;

			case mnuPembayaranKKStanCharKK:
				tIndex = PEMBAYARAN_KK_STANCHAR;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KK STANDCHAR");
				break;

			case mnuPembayaranKKStanCharKTA:
				tIndex = PEMBAYARAN_KK_STANCHAR;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KTA STANDCHAR");
				break;

			case mnuPembayaranKKHSBCKK:
				tIndex = PEMBAYARAN_KK_HSBC;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KK HSBC");
				break;

			case mnuPembayaranKKHSBCKTA:
				tIndex = PEMBAYARAN_KK_HSBC;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KTA HSBC");
				break;

			case mnuPembayaranKKRBSKK:
				tIndex = PEMBAYARAN_KK_RBS;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KK RBS");
				break;

			case mnuPembayaranKKRBSKTA:
				tIndex = PEMBAYARAN_KK_RBS;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KTA RBS");
				break;

			case mnuPembayaranKKANZ:
				tIndex = PEMBAYARAN_KK_ANZ;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN KK/KTA ANZ");
				break;

			case mnuPembayaranHalo:
				tIndex = PEMBAYARAN_HALO;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"BAYAR KARTU HALO");
				break;

			case mnuPembayaranMatrix:
				tIndex = PEMBAYARAN_MATRIX;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"MATRIX");
				break;

			case mnuPembayaranZakatYBM:
			case mnuPembayaranZakatDhuafa:
				tIndex = PEMBAYARAN_ZAKAT;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN ZAKAT");
				break;

			case mnuSetorPasti:
				tIndex = TRX_SETOR_PASTI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"SETOR PASTI");
				break;


			case mnuPembayaranInfaqYBM:
			case mnuPembayaranInfaqDhuafa:
				tIndex = PEMBAYARAN_INFAQ;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN INFAQ");
				break;

			case mnuPembayaranTvInd:
			case mnuPembayaranTvOkTv:
			case mnuPembayaranTvTopTv:
				tIndex = PEMBAYARAN_TV;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN TV BERLANGGANAN");
				break;

			case mnuPembayaranTiketGaruda:
			case mnuPembayaranTiketLionAir:
			case mnuPembayaranTiketSriwijaya:
			case mnuPembayaranTiketMandala:
				tIndex = PEMBAYARAN_TIKET_PESAWAT;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN TIKET PESAWAT");
				break;

			case mnuPembayaranPdamSby:
				tIndex = PEMBAYARAN_PDAM;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN PDAM");
				break;

			case mnuPembayaranTelkom:
				tIndex = PEMBAYARAN_TELKOM;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN TELKOM");
				break;

			case mnuPembayaranDPLKR:
				tIndex = PEMBAYARAN_DPLKR;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN DPLK");
				break;

			case mnuPembayaranBriva:
				tIndex = PEMBAYARAN_BRIVA;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN BRIVA");
				break;

			case mnuRegistrasiTrxPulsa:
				tIndex = TRX_PULSA;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TRX PHONE PULSA");
				break;

			case mnuRegistrasiTrxTransfer:
				tIndex = TRX_TRANSFER;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TRX PHONE TRANSFER");
				break;

			case mnuPembayaranSPP:
				tIndex = PEMBAYARAN_SPP;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN SPP");
				break;

			//tunai
			case mnuTarikTunai:
				tIndex = TARIK_TUNAI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TARIK TUNAI");
				break;

			case mnuVoidTarikTunai:
				tIndex = VOID_TARIK_TUNAI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"VOID TARIK TUNAI");
				break;

			case mnuSetorPinjaman:
				tIndex = SETOR_PINJAMAN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"SETORAN PINJAMAN");
				break;

			case mnuSetorSimpanan:
				tIndex = SETOR_SIMPANAN;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"SETORAN SIMPANAN");
				break;
#ifdef T_BANK
			case mnuCashOut:
			case mnuCashIn:
				tIndex = TCASH;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TCASH");
				break;
#endif
#ifdef PREPAID
			case mnuPrepaidPayment:
				tIndex = PREPAID_PAYMENT;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN");
				break;

			case mnuAktivasiDeposit:
				tIndex = AKTIVASI_DEPOSIT;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"AKTIVASI DEPOSIT");
			break;

			case mnuPrepaidRedeem:
				tIndex = PREPAID_REDEEM;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"REDEEM");
				break;

			//++@agmr - brizzi2
			case mnuPrepaidReaktivasi:
				tIndex = PREPAID_REAKTIVASI;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"REAKTIVASI");
				break;

			case mnuPrepaidVoid:
				tIndex = PREPAID_VOID;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"VOID PEMBAYARAN");
				if(total[PREPAID_PAYMENT].transName[0] == 0)
					strcpy(total[PREPAID_PAYMENT].transName,"PEMBAYARAN");
				addStr(total[PREPAID_PAYMENT].totAmt,total[PREPAID_PAYMENT].totAmt,txnAmt);
				total[PREPAID_PAYMENT].count++;
				break;
			//--@agmr - brizzi2

			case mnuPrepaidPaymentDisc:
				tIndex = PREPAID_PAYMENT_DISC;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"PEMBAYARAN DISCOUNT");
				break;

			case mnuTopUpDeposit:
				tIndex = TOPUP_DEPOSIT;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TOP UP DEPOSIT");
				break;

			case mnuTopUpOnline:
				tIndex = TOPUP_ONLINE;
				if(total[tIndex].transName[0] == 0)
					strcpy(total[tIndex].transName,"TOP UP ONLINE");
				break;
#endif

			default:
				continue;
		}
		addStr(total[tIndex].totAmt,total[tIndex].totAmt,txnAmt);
		total[tIndex].count++;
	}

	if(tanda == 0)
		goto lblNoTransaction;

	ret = 1;
	goto lblEnd;

	lblNoTransaction:
		ret = 0;
		usrInfo(infNoTran);
		goto lblEnd;
	lblKO:
		ret = -1;
		usrInfo(infProcessingError);
		goto lblEnd;

	lblEnd:
		trcFN("logCalcTot: ret=%d \n", ret);
		return ret;
}

int strFormatDatTimBRI(char* datTim, char* fmtDatTim) {
	int 			ret;
	card 		month;
	char 		datetime[lenDatTim + 1];
	char 		newdatetime[lenDatTim + 1];
	char 		fmtdatetime[lenDatTim + 1];
	const char 	*bgd = " ";
	const char 	*ctl = "ghabcdijkl";

	memcpy(datetime, datTim, lenDatTim);
	memset(newdatetime, 0, sizeof(newdatetime));
	memset(fmtdatetime, 0, sizeof(fmtdatetime));
	trcFS("datetime: %s\n", datetime);

	dec2num(&month, &datetime[4], 2);
	ret = fmtMut(newdatetime, (const char *) datetime, bgd, ctl);

	CHECK(month <= 12, lblKO);
	CHECK(month > 0, lblKO);

	MAPGET(monSpr + month, fmtdatetime, lblKO);
	memcpy(&fmtdatetime[3], newdatetime, strlen(newdatetime));
	trcFS("fmtdatetime: %s\n", fmtdatetime);
	memcpy(fmtDatTim, fmtdatetime, strlen(fmtdatetime));
	CHECK(ret > 0, lblKO);
	goto lblEnd;

	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int strFormatDatTimBRI2(char* datTim, char* fmtDatTim) {
	int 		ret;
	card 	month;
	char 	*ptr;

	trcFS("datetime: %s\n", datetime);

	ptr = fmtDatTim;
	memcpy(ptr,datTim+6,2);
	ptr+=2;
	*ptr++ = 0x20;

	dec2num(&month, datTim+4, 2);
	CHECK(month <= 12, lblKO);
	CHECK(month > 0, lblKO);

	MAPGET(monSpr + month, ptr, lblKO);
	ptr+=3;

	*ptr++ = 0x20;
	memcpy(ptr,datTim,4);
	ptr+=4;

	*ptr++ = 0x20;

	memcpy(ptr,datTim+8,2);
	ptr+=2;
	*ptr++ = ':';

	memcpy(ptr,datTim+10,2);
	ptr+=2;
	*ptr++ = ':';

	memcpy(ptr,datTim+12,2);
	ptr+=2;

	goto lblEnd;

	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

int briLogPrintTotal(byte jenisLog, char* dateTime)
{
	TOTAL_T 	total[tTotalEnd];
	int 		ret;
	int 		i;
	char 	buf[50], buf2[30], buf3[30], buf4[30];
	char 	dt[20];
	card 	tmp;
	char 	waktu[20];

	memset(waktu,0,sizeof(waktu));
	memset(total,0,sizeof(total));
	memset(dt,0,sizeof(dt));

	getDateTime(dt+2);
	dec2num(&tmp,dt+2,2);
	if(tmp < 70)
		memcpy(dt,"20",2);
	else
		memcpy(dt,"19",2);

	for(i=0;i<tTotalEnd;i++)
		memset((total[i]).totAmt,'0',lenTotAmt);

	if(*dateTime == 0)
		memcpy(waktu,dt,14);
	else if(dateTime != NULL)
		memcpy(waktu,dateTime,14);

	ret = briLogCalcTotal(jenisLog, total, waktu);
	if(ret == -1)
		goto lblKO;

	if(ret == 0)
	{
		ret = 0;
		goto lblEnd;
	}

	memset(buf,0,sizeof(buf));
	sprintf(buf,"REPORT DATE: %c%c/%c%c/%c%c%c%c     %c%c:%c%c:%c%c", dt[6],dt[7],dt[4],dt[5],dt[0],dt[1],dt[2],dt[3],dt[8],dt[9],dt[10],dt[11],dt[12],dt[13]);
	MAPPUTSTR(rptBuf1,buf,lblKO);

	strcpy(buf,"SUMMARY REPORT");
	MAPPUTSTR(rptJudul,buf,lblKO);
	ret = rptReceipt(rloTotalSummaryReport);
	CHECK(ret >= 0, lblKO);

	memset(buf,0,sizeof(buf));
	sprintf(buf,"TRANSACTION DATE: %c%c/%c%c/%c%c%c%c", dt[6],dt[7],dt[4],dt[5],dt[0],dt[1],dt[2],dt[3]);
	prtES("\x1B\x20",buf);

	prtES("\x1B\x20","------------------------------------------------");
	prtES("\x1B\x20","                     COUNT                 TOTAL");
	prtES("\x1B\x20","------------------------------------------------");
	for(i=0;i<tTotalEnd;i++)
	{
		if(total[i].count <= 0)
			continue;

		memset(buf3,0, sizeof(buf3));
		memset(buf3,' ', 20);
		memcpy(buf3,total[i].transName, strlen(total[i].transName));
		memset(buf2,0,sizeof(buf2));
		fmtAmt(buf2, total[i].totAmt, 0, ",.");
		memset(buf4,0, sizeof(buf4));
		memset(buf4,' ', 17);
		memcpy(buf4+17-strlen(buf2),buf2, strlen(buf2));
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%s %04i  Rp. %s",buf3, total[i].count, buf4);
		prtES("\x1B\x20",buf);
	}
	prtES("\x1B\x20","------------------------------------------------");
	ret = rptReceipt(rloReceiptFeed);
	CHECK(ret >= 0, lblKO);

	ret = 1;
	goto lblEnd;

	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}


void printDetail(byte jenisLog,byte tanda,DETAIL_TRANS_T *detail)
{
	char 	buf[48+1];
	char 	buf2[48+1];
	int 		ret;

	if(tanda == 0)
	{
		char datTim [lenDatTim+1];
		char fmtDatTim [lenDatTim+1];

		getDateTime(datTim+2);
		memcpy(datTim, "20", 2);
		strFormatDatTimBRI(datTim, fmtDatTim);
		MAPPUTSTR(rptBuf1,fmtDatTim,lblKO);
		memset(fmtDatTim,0, sizeof(fmtDatTim));
		strFormatDatTimBRI(detail->txnDateTime, fmtDatTim);
		MAPPUTSTR(rptBuf2,fmtDatTim,lblKO);

		MAPPUTSTR(rptJudul,"DETAIL REPORT",lblKO);
		ret = rptReceipt(rloDetailReportHeader);
		CHECK(ret >= 0, lblKO);
	}

	MAPPUTSTR(rptBuf1,detail->txnDateTime,lblKO);
	MAPPUTSTR(rptBuf2,detail->appCode,lblKO);
	MAPPUTSTR(rptBuf3,detail->refNum,lblKO);
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%06i", (int)detail->roc);
	MAPPUTSTR(rptBuf4,buf,lblKO);
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%06i", (int)detail->stan);
	MAPPUTSTR(rptBuf5,buf,lblKO);
	MAPPUTSTR(rptBuf8,detail->transName,lblKO);

	if(jenisLog == CARD_SERVICE_LOG)
	{
		MAPPUTSTR(rptBuf9," ",lblKO);
		memset(buf, 0, sizeof(buf));
		MAPPUTSTR(rptBuf6,buf,lblKO);
		MAPPUTSTR(rptBuf7,buf,lblKO);
		memset(buf, ' ', 19);
		memcpy(buf+ 19-strlen(detail->nomorKartuPetugas) ,detail->nomorKartuPetugas, strlen(detail->nomorKartuPetugas));
		memset(buf2, 0, sizeof(buf2));
		memcpy(buf2,"KARTU CS                     ", 29);
		memcpy(buf2+29,buf,19);
		MAPPUTSTR(rptBuf10,buf2,lblKO);
		memset(buf, 0, sizeof(buf));
		memset(buf, ' ', 19);
		memcpy(buf+ 19-strlen(detail->pan) ,detail->pan, strlen(detail->pan));
		memset(buf2, 0, sizeof(buf2));
		memcpy(buf2,"KARTU NASABAH                ", 29);
		memcpy(buf2+29,buf,19);
		MAPPUTSTR(rptBuf11,buf2,lblKO);
	}else
	{
		memset(buf, 0, sizeof(buf));
		MAPPUTSTR(rptBuf10,buf,lblKO);
		MAPPUTSTR(rptBuf11,buf,lblKO);
		MAPPUTSTR(rptBuf6,detail->pan,lblKO);
		memset(buf,0, sizeof(buf));
		sprintf(buf,"EXP   :            %c%c/%c%c", detail->expDat[0], detail->expDat[1],detail->expDat[2],detail->expDat[3]);
		MAPPUTSTR(rptBuf7,buf,lblKO);
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, detail->amt, 0, ",.");
		MAPPUTSTR(rptBuf9,buf,lblKO);
	}
	rptReceipt(rloDetailData);

	lblKO:
	return;
}

int briLogPrintDetail(byte jenisLog, char* dateTime)
{
	word 			curMnuItem, trxNum;
	int 				ret = 0, ctr = 0;
	word 			regBriLogSize, briLogBeg, briLogMnuItm, briLogTraAmt, briLogDatTim;
	word 			briLogPan,briLogROC,briLogExpDat,briLogAppCode,briLogSTAN;
	word 			briLogRrn;
	DETAIL_TRANS_T 	detail;
	byte 			tanda = 0;
	char 			dt[20];
	card 			tmp;
	char 			waktu[20];

	trcS("briLogPrintDetail: Beg\n");

	memset(waktu,0,sizeof(waktu));
	memset(dt,0,sizeof(dt));
	getDateTime(dt+2);
	dec2num(&tmp,dt+2,2);
	if(tmp < 70)
		memcpy(dt,"20",2);
	else
		memcpy(dt,"19",2);


	if(dateTime[0] == 0)
	{
		memcpy(waktu,dt,14);
	}
	else
	{
		memcpy(waktu,dateTime,14);
	}

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize   	= regCLogSize;
			briLogBeg       	= cLogBeg;
			briLogMnuItm    	= cLogMnuItm;
			briLogDatTim    	= cLogDatTim;
			briLogPan       	= cLogPan;
			briLogTraAmt    	= cLogTraAmt;
			briLogRrn       		= cLogRrn;
			briLogSTAN      	= cLogSTAN;
			briLogROC       	= cLogROC;
			briLogAppCode   	= cLogAutCod;
			briLogExpDat    	= cLogExpDat;
			break;

		case MINI_ATM_LOG:
			regBriLogSize   	= regMLogSize;
			briLogBeg       	= mLogBeg;
			briLogMnuItm    	= mLogMnuItm;
			briLogDatTim    	= mLogDatTim;
			briLogPan       	= mLogPan;
			briLogTraAmt    	= mLogTraAmt;
			briLogRrn       		= mLogRrn;
			briLogSTAN      	= mLogSTAN;
			briLogROC       	= mLogROC;
			briLogAppCode   	= mLogAutCod;
			briLogExpDat    	= mLogExpDat;
			break;

		case TCASH_LOG:
			regBriLogSize   		= regTCashLogSize;
			briLogBeg       		= tCashLogBeg;
			briLogMnuItm    		= tCashLogMnuItm;
			briLogDatTim    		= tCashLogDatTim;
			briLogPan       		= tCashLogPan;
			briLogTraAmt    		= tCashLogTraAmt;
			briLogRrn       			= tCashLogRrn;
			briLogSTAN      		= tCashLogSTAN;
			briLogROC       		= tCashLogROC;
			briLogAppCode   		= tCashLogAutCod;
			briLogExpDat    		= tCashLogExpDat;
			break;

		case TUNAI_LOG:
			regBriLogSize   		= regTLogSize;
			briLogBeg       		= tLogBeg;
			briLogMnuItm    		= tLogMnuItm;
			briLogDatTim    		= tLogDatTim;
			briLogPan       		= tLogPan;
			briLogTraAmt    		= tLogTraAmt;
			briLogRrn       			= tLogRrn;
			briLogSTAN      		= tLogSTAN;
			briLogROC       		= tLogROC;
			briLogAppCode   		= tLogAutCod;
			briLogExpDat    		= tLogExpDat;
			break;

		default:
			goto lblKO;
	}

	MAPGETWORD(regBriLogSize, trxNum, lblKO);

	for (ctr = 0; ctr < trxNum; ctr++)
	{
		memset(&detail,0,sizeof(detail));
		mapMove(briLogBeg, ctr);
		MAPGETWORD(briLogMnuItm, curMnuItem, lblKO);
		MAPGET(briLogDatTim, detail.txnDateTime,lblKO);
		MAPGET(briLogPan,detail.pan,lblKO);
		MAPGET(briLogTraAmt,detail.amt,lblKO);
		MAPGET(briLogRrn,detail.refNum,lblKO);
		MAPGETCARD(briLogSTAN,detail.stan,lblKO);
		MAPGETCARD(briLogROC,detail.roc,lblKO);
		MAPGET(briLogAppCode,detail.appCode,lblKO);
		MAPGET(briLogExpDat,detail.expDat,lblKO);

		switch(jenisLog)
		{
			case CARD_SERVICE_LOG:
				MAPGET(cLogNomorKartuPetugas, detail.nomorKartuPetugas, lblKO);
				break;
		}

		if(dateTime != NULL)
		{
			if(memcmp(waktu,detail.txnDateTime,8) != 0)
				continue;
		}
		getBriTransTypeName(NULL, detail.transName, curMnuItem); //@agmr BRI2
		printDetail(jenisLog,tanda,&detail);

		tanda=1;
	}

	if(tanda == 0)
		goto lblNoTransaction;

	prtES("\x1B\x20","------------------------------------------------");
	ret = rptReceipt(rloReceiptFeed);
	CHECK(ret >= 0, lblKO);

	ret = 1;
	goto lblEnd;

	lblNoTransaction:
		ret = 0;
		usrInfo(infNoTran);
		goto lblEnd;
	lblKO:
		ret = -1;
		usrInfo(infProcessingError);
		goto lblEnd;

	lblEnd:
		trcFN("logCalcTot: ret=%d \n", ret);
		return ret;
}

void getBriTransTypeName(word *txnType, char *transName, word key)
{
	word 	type;
	char 	name[30];
	byte 	voidTxn;

	memset(name,0,sizeof(name));
	switch (key)
	{
		case mnuInfoSaldo:
			strcpy(name,"INFO SALDO");
			type = trtInfoSaldo;
			break;

		case mnuInfoSaldoBankLain:
			strcpy(name,"INFO SALDO BANK LAIN");
			type = trtInfoSaldoBankLain;
			break;

		case mnuMiniStatement:
			strcpy(name,"MINI STATEMENT");
			type = trtMiniStatement;
			break;

		case mnuMutRek:
			strcpy(name,"MUTASI REKENING");
			type = trtMutRek;
			break;

		case mnuPrevilege:
			strcpy(name,"PREVILEGE");
			type = trtPrevilege;
			break;


		case mnuTransferSesamaBRI:
			strcpy(name,"TRANSFER SESAMA BRI");
			type = trtTransferSesamaBRI;
			break;

		case mnuTransferAntarBank:
			strcpy(name,"TRANSFER ANTAR BANK");
			type = trtTransferAntarBank;
			break;

		case mnuPembayaranPLNPasca:
			strcpy(name,"PEMBAYARAN PLN PASCA");
			type = trtPembayaranPLNPasca;
			break;

		case mnuPembayaranPLNPra:
			strcpy(name,"PEMBAYARAN PLN PRA");
			type = trtPembayaranPLNPra;
			break;

		case mnuPembayaranPLNToken:
			strcpy(name,"CETAK TOKEN PLN");
			type = trtPembayaranPLNToken;
			break;

		case mnuPembayaranCicilanFIF:
			strcpy(name,"PEMBAYARAN FIF");
			type = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanBAF:
			strcpy(name,"PEMBAYARAN BAF");
			type = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanOTO:
			strcpy(name,"PEMBAYARAN OTO");
			type = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanFinansia:
			strcpy(name,"PEMBAYARAN FINANSIA");
			type = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanVerena:
			strcpy(name,"PEMBAYARAN VERENA");
			type = trtPembayaranCicilan;
			break;

		case mnuPembayaranCicilanWOM:
			strcpy(name,"PEMBAYARAN WOM");
			type = trtPembayaranCicilan;
			break;

		case mnuPembayaranSimpati50:
		case mnuPembayaranSimpati100:
//		case mnuPembayaranSimpati150:
		case mnuPembayaranSimpati200:
		case mnuPembayaranSimpati300:
		case mnuPembayaranSimpati500:
		case mnuPembayaranSimpati1000:
			strcpy(name,"ISI ULANG SIMPATI/AS");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranMentari25:
		case mnuPembayaranMentari50:
		case mnuPembayaranMentari100:
		case mnuPembayaranMentari250:
		case mnuPembayaranMentari500:
		case mnuPembayaranMentari1000:
			strcpy(name,"ISI ULANG MENTARI");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranIm325:
		case mnuPembayaranIm350:
		case mnuPembayaranIm375:
		case mnuPembayaranIm3100:
		case mnuPembayaranIm3150:
		case mnuPembayaranIm3200:
		case mnuPembayaranIm3500:
		case mnuPembayaranIm31000:
			strcpy(name,"ISI ULANG IM3");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranXl25:
		case mnuPembayaranXl50:
		case mnuPembayaranXl75:
		case mnuPembayaranXl100:
		case mnuPembayaranXl150:
		case mnuPembayaranXl200:
		case mnuPembayaranXl300:
		case mnuPembayaranXl500:
			strcpy(name,"ISI ULANG XL");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranEsia25:
		case mnuPembayaranEsia50:
		case mnuPembayaranEsia75:
		case mnuPembayaranEsia100:
		case mnuPembayaranEsia150:
		case mnuPembayaranEsia200:
			strcpy(name,"ISI ULANG ESIA");
			type = trtPembayaranPulsa;
			break;

			case mnuPembayaranSmart20:
			case mnuPembayaranSmart25:
			case mnuPembayaranSmart50:
			case mnuPembayaranSmart100:
			case mnuPembayaranSmart150:
			case mnuPembayaranSmart200:
			case mnuPembayaranSmart300:
			case mnuPembayaranSmart500:

			strcpy(name,"ISI ULANG SMART");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranFren20:
		case mnuPembayaranFren25:
		case mnuPembayaranFren50:
		case mnuPembayaranFren100:
		case mnuPembayaranFren150:
		case mnuPembayaranFren200:
		case mnuPembayaranFren300:
		case mnuPembayaranFren500:
			strcpy(name,"ISI ULANG FREN");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranThree20:
		case mnuPembayaranThree50:
		case mnuPembayaranThree75:
		case mnuPembayaranThree100:
		case mnuPembayaranThree150:
		case mnuPembayaranThree300:
		case mnuPembayaranThree500:
			strcpy(name,"ISI ULANG THREE");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranAxis25:
		case mnuPembayaranAxis50:
		case mnuPembayaranAxis75:
		case mnuPembayaranAxis100:
		case mnuPembayaranAxis150:
		case mnuPembayaranAxis200:
		case mnuPembayaranAxis300:
			strcpy(name,"ISI ULANG AXIS");
			type = trtPembayaranPulsa;
			break;

		case mnuPembayaranKKBRI:
			strcpy(name,"PEMB. KK BRI");
			type = trtPembayaranKKBRI;
			break;

		case mnuPembayaranKKANZ:
			strcpy(name,"PEMB. KK ANZ");
			type = trtPembayaranKKANZ;
			break;

		case mnuPembayaranKKCITIKK:
			strcpy(name,"PEMB. KK CITI");
			type = trtPembayaranKKCITIKK;
			break;

		case mnuPembayaranKKCITIKTA:
			strcpy(name,"PEMB. KTA CITI");
			type = trtPembayaranKKCITIKTA;
			break;

		case mnuPembayaranKKHSBCKK:
			strcpy(name,"PEMB. KK HSBC");
			type = trtPembayaranKKHSBCKK;
			break;

		case mnuPembayaranKKHSBCKTA:
			strcpy(name,"PEMB. KTA HSBC");
			type = trtPembayaranKKHSBCKTA;
			break;

		case mnuPembayaranKKRBSKK:
			strcpy(name,"PEMB. KK RBS");
			type = trtPembayaranKKRBSKK;
			break;

		case mnuPembayaranKKRBSKTA:
			strcpy(name,"PEMB. KTA RBS");
			type = trtPembayaranKKRBSKTA;
			break;

		case mnuPembayaranKKStanCharKK:
			strcpy(name,"PEMB. KK SCB");
			type = trtPembayaranKKStanCharKK;
			break;

		case mnuPembayaranKKStanCharKTA:
			strcpy(name,"PEMB. KTA SCB");
			type = trtPembayaranKKStanCharKTA;
			break;

		case mnuPembayaranZakat:
		case mnuPembayaranZakatDhuafa:
		case mnuPembayaranZakatYBM:
			strcpy(name,"PEMBAYARAN ZAKAT");
			type = trtPembayaranZakat;
			break;

		case mnuPembayaranInfaqDhuafa:
		case mnuPembayaranInfaqYBM:
			strcpy(name,"PEMBAYARAN INFAQ");
			type = trtPembayaranZakat;
			break;

		case mnuPembayaranDPLKDhuafa:
			strcpy(name,"PEMBAYARAN DPLK");
			type = trtPembayaranZakat;
			break;

		case mnuSetorPasti:
			strcpy(name,"SETOR PASTI");
			type = trtSetorPasti;
			break;

		case mnuSetorSimpanan:
			strcpy(name,"SETOR SIMPANAN");
			type = trtSetorSimpanan;
			break;

		case mnuSetorPinjaman:
			strcpy(name,"SETOR PINJAMAN");
			type = trtSetorPinjaman;
			break;

		case mnuTarikTunai:
			strcpy(name,"TARIK TUNAI");
			type = trtTarikTunai;
			break;

		case mnuVoidTarikTunai:
			strcpy(name,"VOID TARIK TUNAI");
			type = trtVoidTarikTunai;
			break;

		case mnuInstallment:
			mapGetByte(logVoidFlag, voidTxn);
			if(voidTxn == 1)
				strcpy(name,"V/CICILAN BRING");
			else
				strcpy(name,"CICILAN BRING");
			type = trtInstallment;
			break;

		case mnuPembayaranTvBerlangganan:
			strcpy(name,"TV BERLANGGANAN");
			type = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTvInd:
			strcpy(name,"TV INDOVISION");
			type = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTvOkTv:
			strcpy(name,"TV OKTV");
			type = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTvTopTv:
			strcpy(name,"TV TOPTV");
			type = trtPembayaranTVBerlangganan;
			break;

		case mnuPembayaranTiketPesawat:
			strcpy(name,"TIKET PESAWAT");
			type = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketGaruda:
			strcpy(name,"GARUDA");
			type = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketLionAir:
			strcpy(name,"LION AIR");
			type = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketSriwijaya:
			strcpy(name,"SRIWIJAYA AIR");
			type = trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranTiketMandala:
			strcpy(name,"MANDALA");
			type=trtPembayaranTiketPesawat;
			break;

		case mnuPembayaranPdam:
			strcpy(name,"PDAM");
			type = trtPembayaranPdam;
			break;

		case mnuPembayaranPdamSby:
			strcpy(name,"PDAM SBY");
			type = trtPembayaranPdam;
			break;

		case mnuPembayaranTelkom:
			strcpy(name,"PEMB. TELKOM");
			type = trtPembayaranTelkom;
			break;

		case mnuPembayaranDPLKR:
			strcpy(name,"PEMB. DPLK");
			type = trtPembayaranDPLKR;
			break;

		case mnuPembayaranBriva:
			strcpy(name,"PEMB. BRIVA");
			type = trtPembayaranBriva;
			break;

		case mnuRegistrasiTrxTransfer:
			strcpy(name,"TRX. PHONE TRANSFER");
			type = trtTrxPhoneBankingTransfer;
			break;

		case mnuRegistrasiTrxPulsa:
			strcpy(name,"TRX. PHONE PULSA");
			type = trtTrxPhoneBankingPulsa;
			break;

		case mnuPembayaranSPP:
			strcpy(name,"PEMB. SPP");
			type = trtPembayaranSPP;
			break;

		case mnuPembayaranHalo:
			strcpy(name,"HALO");
			type = trtPembayaranPascaBayar;
			break;

		case mnuPembayaranMatrix:
			strcpy(name,"MATRIX");
			type = trtPembayaranPascaBayar;
			break;

		case mnuPIN:
		case mnuGantiPIN:
			strcpy(name,"UBAH PIN");
			type = trtGantiPIN;
			break;

		case mnuAktivasiKartu:
			strcpy(name,"AKTIVASI KARTU");
			type = trtAktivasiKartu;
			break;

		case mnuRenewalKartu:
			strcpy(name,"RENEWAL KARTU");
			type = trtRenewalKartu;
			break;

		case mnuGantiPasswordSPV:
			strcpy(name,"GANTI PASSWORD SPV");
			type = trtGantiPasswordSPV;
			break;

		case mnuReissuePIN:
			strcpy(name,"REISSUE PIN");
			type = trtReissuePIN;
			break;

		case mnuSale:
			mapGet(traTypeName,name,sizeof(name));
			type = trtSale;
			break;

		case mnuSaleRedeem:
			mapGet(traTypeName,name,sizeof(name));
			strcpy(name,"SALE REDEMPTION");
			type = trtSaleRedeem;
			break; //@ar - BRI

		case mnuRefund:
			mapGet(traTypeName,name,sizeof(name));
			type = trtRefund;
			break;

		case mnuAut:
			mapGet(traTypeName,name,sizeof(name));
			strcpy(name,"CARD VERIFY");
			type = trtAut;
			break;

		case mnuPreAuth:
			mapGet(traTypeName,name,sizeof(name));
			strcpy(name,"PRE AUTH");
			type = trtPreAuth;
			break;

		case mnuPreCancel:
			mapGet(traTypeName,name,sizeof(name));
			strcpy(name,"PRE CANCEL");
			type = trtPreCancel;
			break;

		case mnuPreCom:
			mapGet(traTypeName,name,sizeof(name));
			strcpy(name,"PRE COM");
			type = trtPreCom;
			break;

#ifdef T_BANK
		case mnuCashIn:
			strcpy(name,"CASH-IN");
			type = trtTCash;
			break;

		case mnuCashOut:
			strcpy(name,"CASH-OUT");
			type = trtTCashOut;
			break;
#endif
#ifdef ABSENSI
		case mnuDatang:
			strcpy(name,"ABSEN DATANG");
			type = trtAbsensi;
			break;

		case mnuPulangs:
			strcpy(name,"ABSEN PULANG");
			type = trtAbsensi;
			break;

		case mnuIstirahatA:
			strcpy(name,"ISTIRAHAT MULAI");
			type = trtAbsensi;
			break;

		case mnuIstirahatB:
			strcpy(name,"ISTIRAHAT SELESAI");
			type = trtAbsensi;
			break;

		case mnuLemburA:
			strcpy(name,"LEMBUR MULAI");
			type = trtAbsensi;
			break;

		case mnuLemburB:
			strcpy(name,"LEMBUR SELESAI");
			type = trtAbsensi;
			break;

		case mnuShifts:
			strcpy(name,"SHIFT");
			type = trtAbsensi;
			break;

		case mnuShiftsI:
			strcpy(name,"SHIFT 1");
			type = trtAbsensi;
			break;

		case mnuShiftsIA:
			strcpy(name,"DATANG");
			type = trtAbsensi;
			break;

		case mnuShiftsIB:
			strcpy(name,"PULANG");
			type = trtAbsensi;
			break;

		case mnuShiftsII:
			strcpy(name,"SHIFT 2");
			type = trtAbsensi;
			break;

		case mnuShiftsIIA:
			strcpy(name,"DATANG");
			type = trtAbsensi;
			break;

		case mnuShiftsIIB:
			strcpy(name,"PULANG");
			type = trtAbsensi;
			break;

		case mnuShiftsIII:
			strcpy(name,"SHIFT 3");
			type = trtAbsensi;
			break;

		case mnuShiftsIIIA:
			strcpy(name,"DATANG");
			type = trtAbsensi;
			break;

		case mnuShiftsIIIB:
			strcpy(name,"PULANG");
			type = trtAbsensi;
			break;
#endif
#ifdef SSB
		case mnuSIMBaruA :
			strcpy(name,"SIM A BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruAU :
			strcpy(name,"SIM A UMUM BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruB1 :
			strcpy(name,"SIM B1 BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruB1U :
			strcpy(name,"SIM B1 UMUM BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruB2 :
			strcpy(name,"SIM B2 BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruB2U :
			strcpy(name,"SIM B2 UMUM BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruC :
			strcpy(name,"SIM C BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMBaruD :
			strcpy(name,"SIM D BARU");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangA :
			strcpy(name,"SIM A PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangAU :
			strcpy(name,"SIM A UMUM PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB1 :
			strcpy(name,"SIM B1 PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB1U :
			strcpy(name,"SIM B1 UMUM PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB2 :
			strcpy(name,"SIM B2 PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangB2U :
			strcpy(name,"SIM B2 UMUM PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangC :
			strcpy(name,"SIM C PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPanjangD :
			strcpy(name,"SIM C PERPANJANGAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanAU :
			strcpy(name,"SIM A UMUM PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB1 :
			strcpy(name,"SIM B1 PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB1U :
			strcpy(name,"SIM B1 UMUM PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB2 :
			strcpy(name,"SIM B2 PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPeningkatanB2U :
			strcpy(name,"SIM B2 UMUM PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanA :
			strcpy(name,"SIM A PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanAU :
			strcpy(name,"SIM A UMUM PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB1 :
			strcpy(name,"SIM B1 PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB1U :
			strcpy(name,"SIM B1 UMUM PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB2 :
			strcpy(name,"SIM B2 PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMPenurunanB2U :
			strcpy(name,"SIM B2 UMUM PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamA :
			strcpy(name,"SIM A MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamAU :
			strcpy(name,"SIM A UMUM MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB1 :
			strcpy(name,"SIM B1 MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB1U :
			strcpy(name,"SIM B1 UMUM MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB2 :
			strcpy(name,"SIM B2 MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamB2U :
			strcpy(name,"SIM B2 UMUM MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamC :
			strcpy(name,"SIM C MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMGolSamD :
			strcpy(name,"SIM D MUTASI GOLONGAN SAMA");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanAU :
			strcpy(name,"SIM A UMUM MUTASI PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB1 :
			strcpy(name,"SIM B1 MUTASI PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB1U :
			strcpy(name,"SIM B1 UMUM MUTASI PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB2 :
			strcpy(name,"SIM B2 MUTASI PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPeningkatanB2U :
			strcpy(name,"SIM B2 UMUM MUTASI PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanA :
			strcpy(name,"SIM A MUTASI PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanAU :
			strcpy(name,"SIM A UMUM MUTASI PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB1 :
			strcpy(name,"SIM B1 MUTASI PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB1U :
			strcpy(name,"SIM B1 UMUM MUTASI PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB2 :
			strcpy(name,"SIM B2 MUTASI PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMMutasiPenurunanB2U :
			strcpy(name,"SIM B2 UMUM MUTASI PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangA :
			strcpy(name,"SIM A HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangAU :
			strcpy(name,"SIM A UMUM HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangB1 :
			strcpy(name,"SIM B1 HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangB1U :
			strcpy(name,"SIM B1 UMUM HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangB2 :
			strcpy(name,"SIM B2 HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangB2U :
			strcpy(name,"SIM B2 UMUM HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangC :
			strcpy(name,"SIM C HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangD :
			strcpy(name,"SIM D HILANG");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanAU :
			strcpy(name,"SIM A UMUM HILANG PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB1 :
			strcpy(name,"SIM B1 HILANG PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB1U :
			strcpy(name,"SIM B1 UMUM HILANG PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB2 :
			strcpy(name,"SIM B2 HILANG PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPeningkatanB2U :
			strcpy(name,"SIM B2 UMUM HILANG PENINGKATAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanA :
			strcpy(name,"SIM A HILANG PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanAU :
			strcpy(name,"SIM A UMUM HILANG PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanB1 :
			strcpy(name,"SIM B1 HILANG PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanB1U :
			strcpy(name,"SIM B1 UMUM HILANG PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuSIMHilangPenurunanB2 :
			strcpy(name,"SIM B2 HILANG PENURUNAN");
			type = trtPembayaranSSB;
			break;

		case mnuUjiSim :
			strcpy(name,"UJI SIMULATOR");
			type = trtPembayaranSSB;
			break;

		case mnuBPKBBaruRoda2 :
			strcpy(name,"BPKB RODA 2 BARU");
			type = trtPembayaranSSB;
			break;

		case mnuBPKBBaruRoda4 :
			strcpy(name,"BPKB RODA 4 BARU");
			type = trtPembayaranSSB;
			break;

		case mnuBPKBGantiRoda2 :
			strcpy(name,"BPKB RODA 2 GANTI PEMILIK");
			type = trtPembayaranSSB;
			break;

		case mnuBPKBGantiRoda4 :
			strcpy(name,"BPKB RODA 4 GANTI PEMILIK");
			type = trtPembayaranSSB;
			break;

		case mnuSTCK :
			strcpy(name,"STCK");
			type = trtPembayaranSSB;
			break;

		case mnuMutasiRanRoda2 :
			strcpy(name,"MUTASI RANMOR RODA 2");
			type = trtPembayaranSSB;
			break;

		case mnuMutasiRanRoda4 :
			strcpy(name,"MUTASI RANMOR RODA 4");
			type = trtPembayaranSSB;
			break;
#endif
#ifdef PREPAID
		case mnuInfoDeposit:
			strcpy(name,"INFO DEPOSIT");
			type = trtPrepaidInfoDeposit;
			break;

		case mnuPrepaidPayment:
			strcpy(name,"PEMBAYARAN");
			type = trtPrepaidPayment;
			break;

		case mnuTopUpOnline:
			strcpy(name,"TOP UP");
			type = trtPrepaidTopUp;
			break;

		case mnuTopUpDeposit:
			strcpy(name,"DEPOSIT");
			type = trtPrepaidDeposit;
			break;

		case mnuAktivasiDeposit:
			strcpy(name,"AKTIVASI DEPOSIT");
			type = trtPrepaidAktivasi;
			break;

		case mnuPrepaidRedeem:
			strcpy(name,"REDEEM");
			type = trtPrepaidRedeem;
			break;

		//++@agmr - brizzi2
		case mnuPrepaidReaktivasi:
			strcpy(name,"REAKTIVASI");
			type = trtPrepaidReaktivasi;
			break;

		case mnuPrepaidVoid:
			strcpy(name,"VOID PEMBAYARAN");
			type = trtPrepaidVoid;
			break;

		//--@agmr - brizzi2
		case mnuPrepaidPaymentDisc:
			strcpy(name,"PEMBAYARAN DISC");
			type = trtPrepaidPaymentDisc;
			break;
#endif
		default:
			mapGet(traTypeName,name,sizeof(name));
			break;
	}

	if(txnType != NULL)
		*txnType = type;
	if(transName != NULL)
		strcpy(transName,name);
}

//++ @agmr - BRI2

int briPrintData(word txnType, word *rlo, byte *isOneReceipt,byte isDuplicate)
{
	int  	ret;
	byte idx;
	char amtS[lenAmt + 1];
	char curr_sign[lenCurrSign + 1], fmtCurr[5+1];
	char buffExpDat[lenExpDat + 1], ExpDat[lenExpDat + 2];
	char Pan[lenPan + 1];
	card Amt;
	byte singleReceipt=0;
	char str[128];
	char buf[256],buf1[128],buf2[128],buf3[128],buf4[128];
	char amount[20];
	char amount1[20];
	char RspCod[lenRspCod + 1];
	byte subType;
	word key;
	char name[30];
	char strExpDate[30];
	char BlockedPan[lenPan+1];
	char EntMod;
	char traName[dspW + 1];
	byte isChip = 0;
	byte isVoid = 0;
	byte isEMV = 0;
	byte tempCardFlag = 0;
	byte isNegative = 0;
    	byte voidTxn;
    	byte res[5];
	byte im3temp, jenis;
    	int  	i=0, temp=0,x=0;
	char buf23[400];
	card 	i48len;
	card 	dupROC;
	char TID[lenTID + 1];
	char MID[lenMid + 1];
	char BlockedTID[lenTID+1];
	char BlockedMID[lenMid+1];
	word txnTypeTemp;
	SEGMENT 	Infos;


	MAPGETBYTE(traEntMod, EntMod, lblKO);
	switch (EntMod)
	{
		case 'c':
		case 'C':
			ret = 1;
			break;

		default:
			ret = valIsPrintRequired();
			CHECK(ret >= 0, lblEnd);
		break;
	}

	if(ret == 0)
	{
		goto lblEnd;
	}

	//++ @agmr
	switch (EntMod)
	{
		case 'c':
		case 'C':
			isChip = 1;
			MAPPUTSTR(traChipSwipe, " (Chip)", lblKO);
			break;

#ifdef PREPAID
		case 't':
			MAPPUTSTR(traChipSwipe, " (Fly)", lblKO);
			break;
#endif

		case 'm':
		case 'M':
			MAPPUTSTR(traChipSwipe, " (Swipe)", lblKO);
			break;

		default:
			{
				if((txnType == trtPreAuth) ||(txnType == trtPreCancel) || (txnType == trtPreCom) )
				{
					MAPPUTSTR(traChipSwipe, " (Manual)", lblKO);
				}
				else if(txnType == trtVoid)
				{
					MAPGETBYTE(traOriginalTxnType, idx, lblKO);
					txnTypeTemp = idx;
					if((txnTypeTemp == trtPreAuth) ||(txnTypeTemp == trtPreCancel) || (txnTypeTemp == trtPreCom) )
					{
						MAPPUTSTR(traChipSwipe, " (Manual)", lblKO);
					}
				}
				else
					MAPPUTSTR(traChipSwipe, " (Swipe)", lblKO);
			}
			break;
	}
	//-- @agmr

	if(isBRITransaction() == 1 && key != trtInstallment) //@agmr - BRI
		ret = usrInfo(infPrinting);
	else
		ret = usrInfo(infMerCopyPrinting);
	CHECK(ret >= 0, lblKO);

	if(isBRISaveToBatch() == 0)
		ret = strFormatDatTim(traDatTim);
	else
		ret = strFormatTraDatTim();
	CHECK(ret >= 0, lblKO);

	if(txnType == trtVoid)
	{
		isVoid = 1;
		MAPPUTBYTE(appTempStanVoidFlag,1,lblKO);
		MAPGETBYTE(traOriginalTxnType, idx, lblKO);
		txnType = idx;
	}

	if(isDuplicate)
	{
		MAPGETBYTE(logVoidFlag, voidTxn, lblKO);
		if(voidTxn == 1)
		isVoid = 1;
	}

	if(emvFullProcessCheck() != 0)
		isEMV = 1;
	else
		isEMV = 0;

	if(isEMV == 1)// && (isDuplicate == 1 || isVoid == 1))
	{
		memset(buf,0,sizeof(buf));
		ret = mapGet(traEmvTVR, buf, lenTVR+1);
		CHECK(ret > 0, lblKO);
		ret = mapPut(emvTVR,buf,lenTVR+1);

		memset(buf,0,sizeof(buf));
		ret = mapGet(traEmvIssTrnCrt,buf,lenIssTrnCrt+1);
		CHECK(ret > 0, lblKO);
		ret = mapPut(emvIssTrnCrt,buf,lenIssTrnCrt+1);
		CHECK(ret > 0, lblKO);
	}

	memset(traName,0,sizeof(traName));
	mapMove(rqsBeg, txnType - 1);
	MAPGET(rqsDisp, traName, lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);

	memset(buf,0,sizeof(buf));
	MAPGET(traAmt, buf, lblKO);
	ret = fmtAmtReceipt2(traFmtAmt, buf);
	CHECK(ret > 0, lblKO);

	memset(buf1,0,sizeof(buf1));
	MAPGET(traTipAmt, buf1, lblKO);
	ret = fmtAmtReceipt2(traFmtTip, buf1);
	CHECK(ret > 0, lblKO);
	addStr(amtS, buf, buf1);

	ret = fmtAmtReceipt2(traTotAmt, amtS);
	CHECK(ret > 0, lblKO);
	MAPGET(appCurrSign, curr_sign, lblKO);
	MEM_0x20(fmtCurr);
	memcpy(fmtCurr, curr_sign, 3);
	MAPPUTSTR(appCurrSignSpaces, fmtCurr, lblKO);

	memset(buffExpDat,0,sizeof(buffExpDat));
	memset(ExpDat,0,sizeof(ExpDat));
	MAPGET(traExpDat, buffExpDat, lblKO);
	FmtExpDat(ExpDat, buffExpDat);
	MAPPUTSTR(traExpDatFmt, ExpDat, lblKO);

	memset(Pan,0,sizeof(Pan));
	mapGet(traPan,Pan,sizeof(Pan));
	memset(BlockedPan, '*', sizeof BlockedPan);
	memcpy(BlockedPan + 12, Pan + 12, lenPan-12); //@agmr
	BlockedPan[sizeof(BlockedPan)-1]=0;	          //@agmr
	MAPPUTSTR(traBlockPan, BlockedPan,lblKO);


	memset(TID,0,sizeof(TID));
	MAPGET(acqTID,TID,lblKO);
	memset(BlockedTID, '*', sizeof BlockedTID);
	memcpy(BlockedTID + 4, TID + 4, lenTID-4);
	BlockedTID[sizeof(BlockedTID)-1]=0;
	MAPPUTSTR(traBlockTID, BlockedTID,lblKO);

	memset(MID,0,sizeof(MID));
	MAPGET(acqMID, MID, lblKO);
	memset(BlockedMID, '*', sizeof BlockedMID);
	memcpy(BlockedMID + 11, MID + 11, lenMid-11);
	BlockedMID[sizeof(BlockedMID)-1]=0;
	MAPPUTSTR(traBlockMID, BlockedMID,lblKO);

	memset(buf1,0,sizeof(buf1));
	info_seg(4, &Infos);
	//sprintf(buf1,"          Sinarmas I.220 %s (Secured)",Infos.libelle);
	sprintf(buf1,"          Sinarmas I.220 %s (Secured)",VERSI_APP);//@@SIMAS-APP_VERSION
	MAPPUTSTR(rptBufVers, buf1,lblKO);

	MAPPUTSTR(rptBuf1,"",lblKO);
	MAPPUTSTR(rptBuf2,"",lblKO);
	MAPPUTSTR(rptBuf3,"",lblKO);
	MAPPUTSTR(rptBuf4,"",lblKO);
	MAPPUTSTR(rptBuf5,"",lblKO);
	MAPPUTSTR(rptBuf6,"",lblKO);
	MAPPUTSTR(rptBuf7,"",lblKO);
	MAPPUTSTR(rptBuf8,"",lblKO);
	MAPPUTSTR(rptBuf9,"",lblKO);
	MAPPUTSTR(rptBuf10,"",lblKO);
	MAPPUTSTR(rptBuf11,"",lblKO);
	MAPPUTSTR(rptBuf12,"",lblKO);
	MAPPUTSTR(rptBuf13,"",lblKO);
	MAPPUTSTR(rptBuf14,"",lblKO);
	MAPPUTSTR(rptBuf15,"",lblKO);
	MAPPUTSTR(rptBuf16,"",lblKO);
	MAPPUTSTR(rptBuf17,"",lblKO);

	if(txnType == trtSale || txnType == trtAut || txnType == trtRefund || txnType == trtSaleRedeem || txnType == trtInstallment
		 || txnType == trtPreCom || trtOffline /* @@SIMAS-OFFLINE-AMOUNT*/)
	{
		memset(name,0,sizeof(name));
		if(isVoid)
		{
			strcpy(name, "VOID ");
			if(txnType != trtRefund)
				isNegative = 1;
		}
		else
		{
			if(txnType == trtRefund)
				isNegative = 1;
		}
		memset(strExpDate,0,sizeof(strExpDate));
		sprintf(strExpDate,"EXPY DATE :  %s",ExpDat);

		memset(buf,0,sizeof(buf));
		memset(buf1,0,sizeof(buf1));
		if(isNegative ==1)
			strcpy(buf1,"-Rp. ");
		else
			strcpy(buf1,"Rp. ");

		mapGet(traAmt,buf,sizeof(buf));
		fmtAmt(buf1+strlen(buf1),buf,0,",.");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"AMOUNT %40s",buf1);
		MAPPUTSTR(rptBuf2,buf,lblKO);

		memset(buf,0,sizeof(buf));
		memset(buf1,0,sizeof(buf1));
		if(isNegative ==1)
			strcpy(buf1,"-Rp. ");
		else
			strcpy(buf1,"Rp. ");

		mapGet(traTotAmt,buf,sizeof(buf));
		strcat(buf1,buf);
		memset(buf,0,sizeof(buf));
		sprintf(buf,"AMOUNT %17s",buf1);
		MAPPUTSTR(rptBuf3,buf,lblKO);

		if(txnType == trtInstallment && isVoid == 0)
			MAPPUTSTR(rptBuf3,"",lblKO);
    	}

	memset(buf1, 0, sizeof(buf));
	strcpy(buf1," ");
	MAPPUTSTR(rptJudul2,buf1,lblKO);

	switch (txnType)
	{
		case trtSale:
			strcat(name,"SALE");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));

			MAPPUTSTR(rptBuf1,buf,lblKO);
			if(isEMV)
				key = rloTrxEMV;
			else
				key = rloTrx;
			break;

		case trtSaleRedeem:
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));
			MAPPUTSTR(rptBuf1,buf,lblKO);

			if(!isVoid){
				memset(buf,0,sizeof(buf));
				strcpy(buf,"INFORMASI REDEMPTION");
				MAPPUTSTR(rptBuf5,buf,lblKO);

				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traInquiryAmt,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"SALES AMOUNT     : %25s",buf);
				MAPPUTSTR(rptBuf6,str,lblKO);

				// redem Amount
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traPelangganId,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				strcpy(buf,"-Rp. ");
				fmtAmt(buf+5, amount, 0, ",.");
				sprintf(str,"REDEEM AMOUNT    : %24s",buf);
				MAPPUTSTR(rptBuf7,str,lblKO);

				// net sales amount
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traBunga,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"NET SALE AMOUNT  : %25s",buf);
				MAPPUTSTR(rptBuf8,str,lblKO);

				//redem point
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traNomorRekeningAsalBRI,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				strcpy(buf,"-");
				fmtAmt(buf+1, amount, 0, ",.");
				sprintf(str,"REDEEM POINT     : %25s",buf);
				MAPPUTSTR(rptBuf9,str,lblKO);

				// balance point
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traNamaBankAsal,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				fmtAmt(buf, amount, 0, ",.");
				sprintf(str,"POINT BALANCE    : %25s",buf);
				MAPPUTSTR(rptBuf10,str,lblKO);
			}

			key = rloTrxEMVRedeem;
			break;

		case trtPreAut:
			key = rloPreAuth;
			break;

		case trtPreAuth:
			memset(name,0,sizeof(name));
			strcat(name,"PRE AUTH");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));

			MAPPUTSTR(rptBuf1,buf,lblKO);
			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"AMOUNT : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);
			MAPPUTSTR(rptBuf3,str,lblKO);
			if(isEMV)
			{
				key = rloTrxEMV;
			}
			else
			{
				key = rloPreAuthentic;
			}
			break;

		case trtPreCom:
			memset(name,0,sizeof(name));
			if(isVoid)
			{
				strcpy(name, "VOID ");
			}

			strcat(name,"PRE COMP");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));

			MAPPUTSTR(rptBuf1,buf,lblKO);
			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			if(isNegative ==1)
				strcpy(buf,"-Rp. ");
			else
				strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"AMOUNT    : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);
		    MAPPUTSTR(rptBuf3,str,lblKO);

			if(isEMV)
			{
				key = rloTrxEMV;
			}
			else
			{
				key = rloPreAuthentic;
			}
			break;

		case trtPreCancel:
			memset(name,0,sizeof(name));
			strcat(name,"PRE CANCEL");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));

			MAPPUTSTR(rptBuf1,buf,lblKO);
			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"AMOUNT    : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);
		    	MAPPUTSTR(rptBuf3,str,lblKO);

			if(isEMV)
			{
				key = rloTrxEMV;
			}
			else
			{
				key = rloPreAuthentic;
			}
			break;

		case trtAut:
			strcat(name,"CARD VERIFY");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));

			MAPPUTSTR(rptBuf1,buf,lblKO);
			if(isEMV)
			{
				key = rloTrxEMV;
			}
			else
			{
				key = rloTrx;
			}
			MAPGETBYTE(traTempCardFlag, tempCardFlag, lblKO);
			if(tempCardFlag==1)
				MAPPUTBYTE(traTempCardFlag, 0, lblKO);
			break;

		//-- @agmr - BRI5
		case trtRefund:
			strcat(name,"REFUND");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));

			MAPPUTSTR(rptBuf1,buf,lblKO);
			if(isEMV)
			{
				key = rloTrxEMV;
			}
			else
			{
				key = rloTrx;
			}
			break;

		case trtInstallment:
			strcat(name,"CICILAN BRING");
			memset(buf,0,sizeof(buf));
			memcpy(buf,name,strlen(name));
			MAPPUTSTR(rptBuf1,buf,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			memset(buf,0,sizeof(buf));
			fmtAmt(buf, amount, 0, ",.");
			if(isNegative == 1)
				sprintf(str,"AMOUNT  -Rp. %7s",buf);
			else
				sprintf(str,"AMOUNT  Rp. %7s",buf);

			MAPPUTSTR(rptBuf4,str,lblKO);

			if(isNegative == 1)
				sprintf(str,"  POKOK          -Rp. %26s",buf);
			else
				sprintf(str,"  POKOK           Rp. %26s",buf);

			MAPPUTSTR(rptBuf6,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traInstallmentPlan,buf,lenInstallmentPlan);
			strcpy(str,"INFORMASI CICILAN BRING");
			MAPPUTSTR(rptBuf5,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traInstallmentTerm,buf,lenInstallmentTerm);
			strcat(buf," BULAN");
			sprintf(str,"  JANGKA WAKTU %33s",buf);
			MAPPUTSTR(rptBuf7,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traInstallmentCicilan,amount,lenInstallmentCicilan);
			amount[strlen(amount)-2] = 0;
			memset(buf,0,sizeof(buf));
			fmtAmt(buf, amount, 0, ",.");
			sprintf(str,"  CICILAN BULANAN Rp. %26s",buf);
			MAPPUTSTR(rptBuf8,str,lblKO);

			key = rloInstallmentEMV;
			break;

		case trtCash:
			key = rloTrx;
			break;

		case trtVerify:
			key = rloCrdVrfy;
			break;

		case trtOffline:
			MAPPUTSTR(traTransType, "OFFLSALE", lblKO);
			key = rloOfflineSale;
			break;

// ++ @agmr - BRI - receipt
		case trtInfoSaldo:
			singleReceipt=1;

			memset(buf,0,sizeof(buf));
			strcpy(buf,"INFO SALDO");
			MAPPUTSTR(rptJudul,buf,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaPengirim,buf,lenNamaPemilik);
			sprintf(str,"Nama           : %30s",buf); //@@OA
			MAPPUTSTR(rptNamaNasabah,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traSisaSaldo,amount,sizeof(amount));
			amount[strlen(amount) -2] = 0;
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Saldo          : %s",buf);//@@OA
			MAPPUTSTR(rptBuf1,str,lblKO);

			key = rloInfoSaldo;
			break;

			case trtInfoSaldoBankLain:
				singleReceipt=1;

				memset(buf,0,sizeof(buf));
				strcpy(buf,"INFO SALDO BANK LAIN");
				MAPPUTSTR(rptJudul,buf,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaBankAsal,buf,lenNamaBank);
				sprintf(str,"Nasabah/Bank   : %s",buf);//@@OA
				MAPPUTSTR(rptNamaNasabahBank,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traJenisRekening,buf,1);
				if(buf[0] == 1)
				{
					MAPPUTSTR(rptJenisRekening,"Jenis Rekening : Tabungan",lblKO);
				}
				else
				{
					MAPPUTSTR(rptJenisRekening,"Jenis Rekening : Giro",lblKO);
				}

				memset(amount,0,sizeof(amount));
				mapGet(traSisaSaldo,amount,sizeof(amount));
				amount[strlen(amount)-2] = 0;
				memset(buf,0,sizeof(buf));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"Saldo          : %s",buf);//@@OA
				MAPPUTSTR(rptBuf1,str,lblKO);

				key = rloInfoSaldoBankLain;
				break;

			case trtMiniStatement:
				singleReceipt=1;
				memset(buf,0,sizeof(buf));
				strcpy(buf,"MINI STATEMENT");
				MAPPUTSTR(rptJudul,buf,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNomorRekeningTujuanBRI,buf,lenNomorRekeningBRI);
				sprintf(str,"No. Rekening   : %30s",buf); //Adrian_rx bit 48 digit mulai dari digit ke 106 , 15 digit
				MAPPUTSTR(rptBuf1,str,lblKO);

				memset(buf,0,sizeof(buf));
				sprintf(str,"Date               Trx           Amount %30s",buf); //@@OA
				MAPPUTSTR(rptBuf2,str,lblKO);

				memset(buf,0,sizeof(buf));
				sprintf(str,"----------       --------        ---------- %30s",buf); //@@OA
				MAPPUTSTR(rptBuf3,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traBuffer6,buf,sizeof(buf));

				for(i=0; i<5; i++)
				{
					memset(amount,0,sizeof(amount));
					memset(buf2,0,sizeof(buf2));
					memset(buf23,0,sizeof(buf23));

					temp = 21 * i;
					strncpy(buf23,    buf+(temp),  2);     		//tanggal
					strncpy(buf23+2,  "/", 1);  		 		//
					strncpy(buf23+3,  buf+(temp+2), 2); 		//bulan
					strncpy(buf23+5,  "/", 1);   				//
					strncpy(buf23+6,  buf+(temp + 4), 4); 	//tahun
					strncpy(buf23+10, "          ", 10); 		// space 10
					strncpy(buf23+20, buf+(temp + 8), 1); 	//debit atau kredit
					strncpy(buf23+21, "      Rp.", 9); 			//space sebanyak 9
					strncpy(amount,    buf+(temp + 9), 10);  	// RP.nilai uang
					fmtAmt(buf2, amount, 0, ",.");
					strncpy(buf23+30 ,buf2, 15);
					sprintf(str, " %30s", buf23);
					MAPPUTSTR(rptBuf5+i, str, lblKO);
				}

				key = rloMiniStatement;
		    		break;

			case trtMutRek:
				singleReceipt=1;
				memset(buf,0,sizeof(buf));
				strcpy(buf,"MUTASI REKENING");
				MAPPUTSTR(rptJudul,buf,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNomorRekeningTujuanBRI,buf,lenNomorRekeningBRI);
				sprintf(str,"No. Rekening   : %30s",buf); //Adrian_rx bit 48 digit mulai dari digit ke 106 , 15 digit
				MAPPUTSTR(rptBuf1,str,lblKO);

				memset(buf,0,sizeof(buf));
				sprintf(str,"Date               Trx           Amount %30s",buf); //@@OA
				MAPPUTSTR(rptBuf2,str,lblKO);

				memset(buf,0,sizeof(buf));
				sprintf(str,"    Remarks %30s",buf);
				MAPPUTSTR(rptBuf3,str,lblKO);


				memset(buf,0,sizeof(buf));
				sprintf(str,"----------       --------        ---------- %30s",buf); //@@OA
				MAPPUTSTR(rptBuf4,str,lblKO);

				ret = mapGetCard(traField48Len,i48len);
				memset(buf,0,sizeof(buf));
				mapGet(traBuffer7,buf,lenBuffer7);
				x=strlen(buf)/44;
				for(i=0; i<x; i++)
					{

					memset(amount,0,sizeof(amount));
					memset(buf2,0,sizeof(buf2));
					memset(buf23,0,sizeof(buf23));
					memset(str,0,sizeof(str));


					temp = 44 * i; 							//panjang data looping
					strncpy(buf23, buf+(temp),  10);     	//tanggal
					strncpy(buf23+10, "          ", 10); 		// space 10
					strncpy(buf23+20,  buf+(temp+10), 1); 		//Debit
					strncpy(buf23+21, "      Rp.", 9); 			//space sebanyak 9
					strncpy(amount,    buf+(temp + 11), 10);  	// RP.nilai uang
					fmtAmt(buf2, amount, 0, ",.");
					strncpy(buf23+30 ,buf2, 10);
					sprintf(str, " %s", buf23);
					MAPPUTSTR(rptBuf5+(i*2), str, lblKO);


					memset(buf23,0,sizeof(buf23));
					memset(str,0,sizeof(str));
					strncpy(buf23, buf+(temp + 21),23);     	//Remark
					strncpy(buf23, "  ", 2); 		// space 2
					sprintf(str, "%s", buf23);
					MAPPUTSTR(rptBuf6+(i*2), str, lblKO);
					}

				key = rloMiniStatement;
		    		break;


				case trtPrevilege:
					singleReceipt=1;
					//			memset(RspCod, 0, sizeof(RspCod));
					MAPGET(traRspCod, RspCod, lblKO);
					if(strcmp(RspCod, "01") == 0)
					{
					memset(buf,0,sizeof(buf));
					strcpy(buf,"PREVILEGE PRIORITAS");
					MAPPUTSTR(rptJudul,buf,lblKO);

					memset(buf,0,sizeof(buf));
					strcpy(buf,"TRANSACTION DENIED");
					MAPPUTSTR(rptBuf10,buf,lblKO);

					memset(buf,0,sizeof(buf));
					strcpy(buf,"Mohon maaf untuk sementara fasilitas");
					MAPPUTSTR(rptPLNMsg1,buf,lblKO);

					memset(buf,0,sizeof(buf));
					strcpy(buf,"ini tidak dapat Anda pergunakan");
					MAPPUTSTR(rptPLNMsg2,buf,lblKO);

					key = rloPembayaranTiket;
					}else{

					memset(buf,0,sizeof(buf));
					strcpy(buf,"PREVILEGE ACCEPTED");
					MAPPUTSTR(rptJudul,buf,lblKO);

					memset(buf,0,sizeof(buf));
					strcpy(buf,"TRANSACTION ACCEPTED");
					MAPPUTSTR(rptBuf10,buf,lblKO);

					memset(buf,0,sizeof(buf));
					strcpy(buf,"Silakan menikmati Privilege Prioritas");
					MAPPUTSTR(rptPLNMsg1,buf,lblKO);

					memset(buf,0,sizeof(buf));
					strcpy(buf,"BRI");
					MAPPUTSTR(rptPLNMsg2,buf,lblKO);

					key = rloPembayaranTiket;	}
					break;


			case trtTransferSesamaBRI:
				singleReceipt=1; //@@OA
				memset(buf,0,sizeof(buf));
				strcpy(buf,"TRANSFER SESAMA BRI");
				MAPPUTSTR(rptJudul,buf,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNomorRekeningTujuanBRI,buf,lenNomorRekeningBRI);
				sprintf(str,"No. Rek        : %s",buf);//@@OA
				MAPPUTSTR(rptNomorRekeningPenerima,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaPenerima,buf,lenNamaPemilik);
				sprintf(str,"Nama           : %s",buf);//@@OA
				MAPPUTSTR(rptNamaPenerima,str,lblKO);

				//amount
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,sizeof(amount));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"TOTAL         : %30s",buf);
				MAPPUTSTR(rptBuf1,str,lblKO);

				key = rloTransferSesamaBRI;
				break;

			case trtTransferAntarBank:
				singleReceipt=1; //@@OA
				memset(buf,0,sizeof(buf));
				strcpy(buf,"TRANSFER ANTAR BANK");
				MAPPUTSTR(rptJudul,buf,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traJenisRekening,buf,1);
				if(buf[0] == 1)
				    mapPut(rptJenisRekening,"Dari Tabungan",13);
				else
				    mapPut(rptJenisRekening,"Dari Giro",9);

				memset(buf,0,sizeof(buf));
				mapGet(traJenisTransfer,buf,lenJenisTransfer);
				if(memcmp(buf,"00",2) == 0)
				    mapPut(rptJenisTransfer,"melalui BRI",11);
				else if(memcmp(buf,"01",2) == 0)
				    mapPut(rptJenisTransfer,"melalui LINK",12);
				else if(memcmp(buf,"02",2) == 0)
				    mapPut(rptJenisTransfer,"melalui PRIMA",13);
				else if(memcmp(buf,"03",2) == 0)
				    mapPut(rptJenisTransfer,"melalui BERSAMA",15);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaBankAsal,buf,lenNamaBank);
				sprintf(str,"Bank           : %s",buf);//@@OA
				MAPPUTSTR(rptBankAsal,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaPengirim,buf,lenNamaPemilik);
				sprintf(str,"Nama           : %s",buf);//@@OA
				MAPPUTSTR(rptNamaPengirim,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNomorRekeningAsal,buf,lenNomorRekening);
				sprintf(str,"No Rek         : %s",buf);//@@OA
				MAPPUTSTR(rptNomorRekeningPengirim,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaBankTujuan,buf,lenNamaBank);
				sprintf(str,"Bank           : %s",buf);//@@OA
				MAPPUTSTR(rptBankTujuan,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaPenerima,buf,lenNamaPemilik);
				sprintf(str,"Nama           : %s",buf);//@@OA
				MAPPUTSTR(rptNamaPenerima,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNomorRekeningTujuan,buf,lenNomorRekening);
				sprintf(str,"No Rek         : %s",buf);//@@OA
				MAPPUTSTR(rptNomorRekeningPenerima,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traCustomerRefNumber,buf,lenCustomerRefNumber);
				sprintf(str,"Cust Ref No    : %s",buf);//@@OA
				MAPPUTSTR(rptCustomerRef,str,lblKO);

				//amount
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,sizeof(amount));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"Jumlah         : %30s",buf);
				MAPPUTSTR(rptBuf1,str,lblKO);

				key = rloTransferAntarBank;
				break;

			/*Changed By Ali*/
			case trtPembayaranTVBerlangganan:
				singleReceipt=1; //@@OA
				memset(str,0,sizeof(str));
				mapGet(traSubType,buf,1);
				switch(buf[0])
				{
					case TV_INDOVISION:
						strcpy(str,"PEMBAYARAN INDOVISION");
						strcpy(buf1,"   INDOVISION MENYATAKAN TANDA TERIMA INI");
						break;

					case TV_OKTV:
						strcpy(str,"PEMBAYARAN OK TV");
						strcpy(buf1,"      OK TV MENYATAKAN TANDA TERIMA INI");
						break;

					case TV_TOPTV:
						strcpy(str,"PEMBAYARAN TOP TV");
						strcpy(buf1,"      TOP TV MENYATAKAN TANDA TERIMA INI");
						break;
				}

				MAPPUTSTR(rptPLNMsg1,buf1,lblKO);
				strcpy(buf1,"      SEBAGAI BUKTI PEMBAYARAN YANG SAH");
				MAPPUTSTR(rptPLNMsg2,buf1,lblKO);
				MAPPUTSTR(rptJudul,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaPenerima,buf,sizeof(buf));
				sprintf(str,"No ID Pelanggan  : %s",buf);
				MAPPUTSTR(rptBuf1,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traNamaPengirim,buf,sizeof(buf));
				sprintf(str,"Nama Pelanggan   : %s",buf);
				MAPPUTSTR(rptBuf2,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traLWBP,buf,sizeof(buf));
				sprintf(str,"Periode          : %s",buf);
				MAPPUTSTR(rptBuf3,str,lblKO);

				//amount
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traTotalAmount,amount,sizeof(amount));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"Jumlah           : %28s",buf);
				MAPPUTSTR(rptBuf4,str,lblKO);

				key = rloPembayaranCicilan;

				break;

		case trtPembayaranTiketPesawat:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			mapGet(traSubType,buf,1);
			switch(buf[0])
			{
				case TIKET_GARUDA:
					strcpy(str,"PEMB TIKET GARUDA");
					strcpy(buf1,"GARUDA MENYATAKAN TANDA TERIMA INI");
					break;
				case TIKET_LION_AIR:
					strcpy(str,"PEMB TIKET LION AIR");
					strcpy(buf1,"LION AIR MENYATAKAN TANDA TERIMA INI");
					break;
				case TIKET_SRIWIJAYA_AIR:
					strcpy(str,"PEMB SRIWIJAYA AIR");
					strcpy(buf1,"SRIWIJAYA AIR MENYATAKAN TANDA TERIMA INI");
					break;
				case TIKET_MANDALA:
					strcpy(str,"PEMB TIKET MANDALA AIR");
					strcpy(buf1,"MANDALA AIR MENYATAKAN TANDA TERIMA INI");
					break;
			}

			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);
			strcpy(buf1,"SEBAGAI BUKTI PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNoTelpon,buf,sizeof(buf));
			sprintf(str,"Kode Bayar    : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaBankAsal,buf,sizeof(buf));
			sprintf(str,"Kode Booking  : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traBuffer3,buf,sizeof(buf));
			sprintf(str,"Nama          : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traBillStatus,buf,sizeof(buf));
			sprintf(str,"Jumlah Flight : %s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Jumlah        : %28s",buf);
			MAPPUTSTR(rptBuf5,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(0,buf,sizeof(buf));
			sprintf(str,"No Penerbangan: %s",buf);
			MAPPUTSTR(rptBuf6,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			mapGet(traBillStatus,buf1,sizeof(buf1));
			for(i=0; i <= atoi(buf1); i++)
			{
				memset(buf23, 0, sizeof(buf23));
				memset(buf,0,sizeof(buf));

				strcpy(buf, tempTiket[i]);       //@ar - v0.45

				strncpy(buf23,       buf+2, 2);
				strncpy(buf23+2,   " ", 1);
				strncpy(buf23+3,   buf+11, 4);
				strncpy(buf23+7,   " ", 1);
				strncpy(buf23+8,   buf+4, 1);
				strncpy(buf23+9,   " ", 1);
				strncpy(buf23+10, buf+5 , 3);
				strncpy(buf23+13, "-", 1);
				strncpy(buf23+14, buf+8, 3);
				strncpy(buf23+17, " ", 1);
				strncpy(buf23+18, buf+15,2);
				strncpy(buf23+20, "/", 1);
				strncpy(buf23+21, buf+17,2);
				strncpy(buf23+23, " ", 1);
				strncpy(buf23+24, buf+19,2);
				strncpy(buf23+26, ":", 1);
				strncpy(buf23+27, buf+20,2);
				strncpy(buf23+29, ":", 1);
				strncpy(buf23+30, buf+1,1);

				sprintf(str," %s seat",buf23);
				MAPPUTSTR((rptBuf8+i), str, lblKO);
			}

			key = rloPembayaranTiket;

			break;

		case trtPembayaranPdam:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			mapGet(traSubType,buf,1);

			strcpy(str,"PEMBAYARAN PDAM SURABAYA");
			MAPPUTSTR(rptJudul,str,lblKO);

			//No Pelanggan
			memset(buf,0,sizeof(buf));
			mapGet(traNamaPelangganPLN,buf,sizeof(buf));
			sprintf(str,"NO PELANGGAN      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);


			//NAMA Pelanggan
			memset(buf,0,sizeof(buf));
			mapGet(traCustomerName,buf,sizeof(buf));
			sprintf(str,"NAMA PELANGGAN    : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);


			//Jumlah Pakai
			memset(buf,0,sizeof(buf));
			mapGet(traBuffer4,buf,sizeof(buf));
			sprintf(str,"JUMLAH PAKAI     : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			//periode
			memset(buf,0,sizeof(buf));
			mapGet(traJenisTransfer,buf,sizeof(buf));
			sprintf(str,"PERIODE           : %s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			//Kode askoper
			memset(buf,0,sizeof(buf));
			mapGet(traPokokHutang,buf,sizeof(buf));
			sprintf(str,"KODE ASKOPER      : %s",buf);
			MAPPUTSTR(rptBuf5,str,lblKO);

			//No Reff
			memset(buf,0,sizeof(buf));
			mapGet(traRrn,buf,sizeof(buf));
			sprintf(str,"NO REFF           : %s",buf);
			MAPPUTSTR(rptBuf6,str,lblKO);

			//Jumlah Tagihan
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traTotalAmount,amount,sizeof(amount));

			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"JML TAGIHAN       : %28s",buf);
			MAPPUTSTR(rptBuf7,str,lblKO);

			//memset(buf,0,sizeof(buf));
			strcpy(str,"Jumlah tagihan sudah termasuk biaya adminl");
			MAPPUTSTR(rptBuf8,str,lblKO);

			//memset(buf,0,sizeof(buf));
			strcpy(str,"Transaksi Berhasil");
			MAPPUTSTR(rptBuf9,str,lblKO);

			key = rloPembayaranPdam;

			break;

		case trtPembayaranTelkom:

			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			mapGet(traSubType,buf,1);

			strcpy(str,"PEMBAYARAN TELKOM");
			strcpy(buf1,"      TELKOM MENYATAKAN TANDA TERIMA INI");

			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);
			strcpy(buf1,"      SEBAGAI BUKTI PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traPaymentNumber,buf,sizeof(buf));
			memset(buf1,0,sizeof(buf1));
			mapGet(traAdminCharge,buf1,sizeof(buf1));
			sprintf(str,"No Telepon       : 0%d%d",atoi(buf),atoi(buf1));
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaPengirim,buf,sizeof(buf));
			sprintf(str,"Nama Pelanggan   : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traTotalAmount,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Jumlah           : %28s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			key = rloPembayaranCicilan;

			break;

#ifdef SSB
case trtPembayaranSSB:

			singleReceipt=1; //@@OA
			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			strcpy(str,"PEMBAYARAN SIM");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(str,0,sizeof(str));
			memset(buf,0,sizeof(buf));
			mapGet(traTotalOutstanding,buf,lenTotalOutstanding);

			if(strcmp(buf,"11")==0)
				strcpy(str,"BARU A");
			else if(strcmp(buf,"17")==0)
				strcpy(str,"BARU C");
			else if(strcmp(buf,"18")==0)
				strcpy(str,"BARU D");
			else if(strcmp(buf,"21")==0)
				strcpy(str,"PERPANJANGAN A");
			else if(strcmp(buf,"22")==0)
				strcpy(str,"PERPANJANGAN A UMUM");
			else if(strcmp(buf,"23")==0)
				strcpy(str,"PERPANJANGAN B1");
			else if(strcmp(buf,"24")==0)
				strcpy(str,"PERPANJANGAN B1 UMUM");
			else if(strcmp(buf,"25")==0)
				strcpy(str,"PERPANJANGAN B2");
			else if(strcmp(buf,"26")==0)
				strcpy(str,"PERPANJANGAN B2 UMUM");
			else if(strcmp(buf,"27")==0)
				strcpy(str,"PERPANJANGAN SIM C");
			else if(strcmp(buf,"28")==0)
				strcpy(str,"PERPANJANGAN SIM D");
			else if(strcmp(buf,"32")==0)
				strcpy(str,"PENINGKATAN KE A UMUM");
			else if(strcmp(buf,"33")==0)
				strcpy(str,"PENINGKATAN KE B1");
			else if(strcmp(buf,"34")==0)
				strcpy(str,"PENINGKATAN KE B1 UMUM");
			else if(strcmp(buf,"35")==0)
				strcpy(str,"PENINGKATAN KE B2");
			else if(strcmp(buf,"36")==0)
				strcpy(str,"PENINGKATAN KE B2 UMUM");
			else if(strcmp(buf,"41")==0)
				strcpy(str,"PENURUNAN KE A");
			else if(strcmp(buf,"42")==0)
				strcpy(str,"PENURUNAN KE A UMUM");
			else if(strcmp(buf,"43")==0)
				strcpy(str,"PENURUNAN KE B1");
			else if(strcmp(buf,"44")==0)
				strcpy(str,"PENURUNAN KE B1 UMUM");
			else if(strcmp(buf,"45")==0)
				strcpy(str,"PENURUNAN KE B2");
			else if(strcmp(buf,"51")==0)
				strcpy(str,"MUTASI A");
			else if(strcmp(buf,"52")==0)
				strcpy(str,"MUTASI A UMUM");
			else if(strcmp(buf,"53")==0)
				strcpy(str,"MUTASI B1");
			else if(strcmp(buf,"54")==0)
				strcpy(str,"MUTASI B1 UMUM");
			else if(strcmp(buf,"55")==0)
				strcpy(str,"MUTASI B2");
			else if(strcmp(buf,"56")==0)
				strcpy(str,"MUTASI B2 UMUM");
			else if(strcmp(buf,"57")==0)
				strcpy(str,"MUTASI C");
			else if(strcmp(buf,"58")==0)
				strcpy(str,"MUTASI D");
			else if(strcmp(buf,"5B")==0)
				strcpy(str,"PENINGKATAN KE A UMUM");
			else if(strcmp(buf,"5C")==0)
				strcpy(str,"PENINGKATAN KE B1");
			else if(strcmp(buf,"5D")==0)
				strcpy(str,"PENINGKATAN KE B1 UMUM");
			else if(strcmp(buf,"5E")==0)
				strcpy(str,"PENINGKATAN KE B2 ");
			else if(strcmp(buf,"5F")==0)
				strcpy(str,"PENINGKATAN KE B2 UMUM");
			else if(strcmp(buf,"5K")==0)
				strcpy(str,"PENURUNAN KE A");
			else if(strcmp(buf,"5L")==0)
				strcpy(str,"PENURUNAN KE A UMUM");
			else if(strcmp(buf,"5M")==0)
				strcpy(str,"PENURUNAN KE B1");
			else if(strcmp(buf,"5N")==0)
				strcpy(str,"PENURUNAN KE B1 UMUM");
			else if(strcmp(buf,"5O")==0)
				strcpy(str,"PENURUNAN KE B2");
			else if(strcmp(buf,"61")==0)
				strcpy(str,"HILANG/RUSAK A");
			else if(strcmp(buf,"62")==0)
				strcpy(str,"HILANG/RUSAK A UMUM");
			else if(strcmp(buf,"63")==0)
				strcpy(str,"HILANG/RUSAK B1");
			else if(strcmp(buf,"64")==0)
				strcpy(str,"HILANG/RUSAK B1 UMUM");
			else if(strcmp(buf,"65")==0)
				strcpy(str,"HILANG/RUSAK B2");
			else if(strcmp(buf,"66")==0)
				strcpy(str,"HILANG/RUSAK B2 UMUM");
			else if(strcmp(buf,"67")==0)
				strcpy(str,"HILANG/RUSAK C");
			else if(strcmp(buf,"68")==0)
				strcpy(str,"HILANG/RUSAK D");
			else if(strcmp(buf,"6B")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6C")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6D")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6E")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6F")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6K")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6L")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6M")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6N")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"6O")==0)
				strcpy(str,"HILANG/RUSAK");
			else if(strcmp(buf,"70")==0)
				strcpy(str,"UJI SIMULATOR");
			else if(strcmp(buf,"81")==0)
				strcpy(str,"BPKB BARU RODA 2/3");
			else if(strcmp(buf,"82")==0)
				strcpy(str,"BPKB BARU RODA 4/LEBIH");
			else if(strcmp(buf,"91")==0)
				strcpy(str,"GANTI PEMILIK RODA 2/3");
			else if(strcmp(buf,"92")==0)
				strcpy(str,"GANTI PEMILIK");
			else if(strcmp(buf,"A0")==0)
				strcpy(str,"STCK");
			else if(strcmp(buf,"B1")==0)
				strcpy(str,"MUTASI RANMOR KENDARAAN");
			else if(strcmp(buf,"B2")==0)
				strcpy(str,"MUTASI RANMOR KENDARAAN");
			MAPPUTSTR(rptJudul1,str,lblKO);

			memset(str,0,sizeof(str));
			if(strcmp(buf,"B2")==0)
				strcpy(str,"RODA 4/LEBIH");
			else if(strcmp(buf,"B1")==0)
				strcpy(str,"RODA 2/3");
			else if(strcmp(buf,"92")==0)
				strcpy(str,"RODA 4/LEBIH");
			else if(strcmp(buf,"6O")==0)
				strcpy(str,"PENURUNAN KE B2");
			else if(strcmp(buf,"6N")==0)
				strcpy(str,"PENURUNAN KE B1 UMUM");
			else if(strcmp(buf,"6M")==0)
				strcpy(str,"PENURUNAN KE B1");
			else if(strcmp(buf,"6L")==0)
				strcpy(str,"PENURUNAN KE A UMUM");
			else if(strcmp(buf,"6K")==0)
				strcpy(str,"PENURUNAN KE A");
			else if(strcmp(buf,"6F")==0)
				strcpy(str,"PENINGKATAN B2 UMUM");
			else if(strcmp(buf,"6E")==0)
				strcpy(str,"PENINGKATAN B2");
			else if(strcmp(buf,"6D")==0)
				strcpy(str,"PENINGKATAN KE B1 UMUM");
			else if(strcmp(buf,"6C")==0)
				strcpy(str,"PENINGKATAN KE B1");
			else if(strcmp(buf,"6B")==0)
				strcpy(str,"PENINGKATAN KE A UMUM");

			MAPPUTSTR(rptJudul2,str,lblKO);





			//Kode staps
			memset(str,0,sizeof(str));
			mapGet(traPaymentNumber,buf1,sizeof(buf1));
			sprintf(str,"Kode Satpas        : %s",buf1);
			MAPPUTSTR(rptBuf1,str,lblKO);

			// nama Stapas
			memset(str,0,sizeof(str));
			mapGet(traCustomerName,buf1,sizeof(buf1));
			sprintf(str,"Nama Satpas        : %s",buf1);
			MAPPUTSTR(rptBuf2,str,lblKO);

			//No  rekening
			memset(str,0,sizeof(str));
			mapGet(traNomorKartuLama,buf1,sizeof(buf1));
			sprintf(str,"No Rekening        : %s",buf1);
			MAPPUTSTR(rptBuf3,str,lblKO);

			// NO Reff
			memset(str,0,sizeof(str));
			mapGet(traRrn,buf1,sizeof(buf1));
			sprintf(str,"No Reff            : %s",buf1);
			MAPPUTSTR(rptBuf4,str,lblKO);

			memset(buf1, 0, sizeof(buf));
			mapGet(traTotalOutstanding,buf1,lenTotalOutstanding);
			if(
				(strcmp(buf1, "81") == 0) ||
				(strcmp(buf1, "82") == 0) ||
				(strcmp(buf1, "91") == 0) ||
				(strcmp(buf1, "92") == 0)

			)
			{
				memset(buf,0,sizeof(buf));
				memset(str,0,sizeof(str));
				mapGet(traBillStatus,buf,lenBillStatus);
				sprintf(str,"Jumlah             : %s",buf);
				MAPPUTSTR(rptBuf5,str,lblKO);

				//amount
				memset(str,0,sizeof(str));
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"Jumlah Setoran     : %s",buf);
				MAPPUTSTR(rptBuf6,str,lblKO);

			}
			else
			{
				//amount
				memset(str,0,sizeof(str));
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,sizeof(amount));
				amount[strlen(amount) - 2] = 0;
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"Jumlah Setoran     : %25s",buf);
				MAPPUTSTR(rptBuf5,str,lblKO);
			}

			key = rloPembayaranSim;
			break;


#endif

		case trtPembayaranDPLKR:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"Setoran DPLK");
			MAPPUTSTR(rptJudul,str,lblKO);

			mapGet(traAdminCharge,buf1,sizeof(buf1));
			sprintf(str,"No DPLK           : %s",buf1);
			MAPPUTSTR(rptBuf1,str,lblKO);

			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Jumlah Setoran    : %28s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			key = rloPembayaranDplk;
			break;

			case trtPembayaranBriva:

			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			strcpy(str,"PEMBAYARAN BRIVA");
			MAPPUTSTR(rptJudul,str,lblKO);

			//NO BRIVA
			memset(buf,0,sizeof(buf));
			mapGet(traNoVoucherMitra,buf,lenNoVoucherMitra);
			sprintf(str,"NO BRIVA      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			//NAMA INSTITUSI
			memset(buf,0,sizeof(buf));
			mapGet(traBuffer1,buf,lenBuffer1);
			sprintf(str,"NAMA INSTITUSI: %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			//NAMA PELANGGAN
			memset(buf,0,sizeof(buf));
			mapGet(traBuffer3,buf,lenBuffer3);
			sprintf(str,"NAMA PELANGGAN: %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			//KETERANGAN
			memset(buf,0,sizeof(buf));
			mapGet(traCustomerName, buf,lenCustomerName);
			sprintf(str,"KETERANGAN    :  %s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traBillStatus, buf,lenBillStatus);
			if(strcmp(buf, "Y")==0)
			{
				//TAGIHAN
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traTotalAmount,amount,sizeof(amount));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"TAGIHAN       : %28s",buf);
				MAPPUTSTR(rptBuf5,str,lblKO);
			}
			else
			{
				//TAGIHAN
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,sizeof(amount));
				amount[strlen(amount)-2] = 0;
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"TAGIHAN       : %28s",buf);
				MAPPUTSTR(rptBuf5,str,lblKO);
			}

			//Pembayaran
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			amount[strlen(amount)-2] = 0;
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"PEMBAYARAN    : %28s",buf);
			MAPPUTSTR(rptBuf6,str,lblKO);


			//ADMIN BANK
			memset(buf1,0,sizeof(buf1));
			mapGet(traFee,buf1,sizeof(buf1));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, buf1, 0, ",.");
			sprintf(str,"ADMIN BANK    : %28s",buf);
			MAPPUTSTR(rptBuf7,str,lblKO);

			//TOT BAYAR
			memset(buf,0,sizeof(buf));
			memset(buf2,0,sizeof(buf2));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,buf2,sizeof(buf2));
			buf2[strlen(buf2)-2] = 0;
			sprintf(amount, "%d", (atoi(buf2) + atoi(buf1)));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL PEMBYRAN: %28s",buf);
			MAPPUTSTR(rptBuf8,str,lblKO);


			//BERHASIL
			strcpy(str,"Berhasil");
			MAPPUTSTR(rptBuf9,str,lblKO);

			key = rloPembayaranBriva;

			break;

		case trtTrxPhoneBankingTransfer:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str," ");
			MAPPUTSTR(rptJudul,str,lblKO);

			strcpy(str,"REG TRX TRANSFER MELALUI");
			MAPPUTSTR(rptBuf1,str,lblKO);

			strcpy(str,"PHONE BANKING");
			MAPPUTSTR(rptBuf2,str,lblKO);

			strcpy(str," BERHASIL");
			MAPPUTSTR(rptBuf3,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf4,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traLWBP, buf, sizeof(buf));
			sprintf(str,"No Rek yang didaftarkan : %s",buf);
			MAPPUTSTR(rptBuf5,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaPengirim, buf, sizeof(buf));
			sprintf(str,"Nama Pelanggan          : %s",buf);
			MAPPUTSTR(rptBuf6,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf7,str,lblKO);

			strcpy(str,"Sekarang Anda Dapat melakukan transfer ke");
			MAPPUTSTR(rptBuf8,str,lblKO);


			strcpy(str,"No Rekening tersebut melalui Phone Banking");
			MAPPUTSTR(rptBuf9,str,lblKO);


			strcpy(str,"BRI.");
			MAPPUTSTR(rptBuf10,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf11,str,lblKO);

			strcpy(str,"Untuk Keamanan transaksi, jaga kerahasiaan");
			MAPPUTSTR(rptBuf12,str,lblKO);

			strcpy(str,"PIN ATM anda.");
			MAPPUTSTR(rptBuf13,str,lblKO);

			key = rloTrxPhoneBanking;
			break;

		case trtTrxPhoneBankingPulsa:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			mapGet(traSubType,buf,1);

			strcpy(str," ");
			MAPPUTSTR(rptJudul,str,lblKO);

			strcpy(str,"REG TRX BELI PULSA MELALUI");
			MAPPUTSTR(rptBuf1,str,lblKO);

			strcpy(str,"PHONE BANKING");
			MAPPUTSTR(rptBuf2,str,lblKO);

			strcpy(str," BERHASIL");
			MAPPUTSTR(rptBuf3,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf4,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(buf1,0,sizeof(buf1));
			memset(buf2,0,sizeof(buf2));
			memset(buf3,0,sizeof(buf3));
			memset(buf4,0,sizeof(buf4));

			mapGet(traNoTelpon, buf, sizeof(buf));
			fmtDelSpace(buf3, buf, lenNoTelpon);
			strncpy(buf2,(buf+(strlen(buf3)-3)), 3);
			fmtPad(buf1, (strlen(buf3)-3), '*');
			strcat(buf1,buf2);
			sprintf(str,"No HP yang didaftarkan : %s",buf1);
			MAPPUTSTR(rptBuf5,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf6,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf7,str,lblKO);

			strcpy(str,"Sekarang Anda Dapat melakukan pembelian");
			MAPPUTSTR(rptBuf8,str,lblKO);


			strcpy(str,"pulsa untuk no HP tersebut melalui Phone");
			MAPPUTSTR(rptBuf9,str,lblKO);


			strcpy(str,"Banking BRI.");
			MAPPUTSTR(rptBuf10,str,lblKO);

			strcpy(str," ");
			MAPPUTSTR(rptBuf11,str,lblKO);

			strcpy(str,"Untuk Keamanan transaksi, jaga kerahasiaan");
			MAPPUTSTR(rptBuf12,str,lblKO);

			strcpy(str,"PIN ATM anda.");
			MAPPUTSTR(rptBuf13,str,lblKO);


			key = rloTrxPhoneBanking;
			break;

		case trtPembayaranSPP:
			singleReceipt=1; //@@OA
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Pembayaran SPP Online");
			MAPPUTSTR(rptJudul,buf,lblKO);

			//No billing
			memset(buf,0,sizeof(buf));
			mapGet(traNomorRekeningAsal,buf,lenNomorRekening);
			sprintf(str,"NO Billing         : %s",buf);//@@OA
			MAPPUTSTR(rptBuf1,str,lblKO);

			//Nama Mahasiswa
			memset(buf,0,sizeof(buf));
			mapGet(traCustomerName,buf,lenNomorRekening);
			sprintf(str,"Nama Mhs           : %s",buf);//@@OA
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"PEN");
			MAPPUTSTR(rptBuf3,buf,lblKO);

			//NO MHS
			memset(buf,0,sizeof(buf));
			mapGet(traNomorRekeningTujuan,buf,lenNomorRekening);
			sprintf(str,"No Mhs             : %s",buf);//@@OA
			MAPPUTSTR(rptBuf4,str,lblKO);

			//Type
			memset(buf,0,sizeof(buf));
			mapGet(traNoVoucherMitra,buf,lenNomorRekening);
			sprintf(str,"Tipe               : %s",buf);//@@OA
			MAPPUTSTR(rptBuf6,str,lblKO);


			//TAGIHAN
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Jumlah             : %s",buf);
			MAPPUTSTR(rptBuf7,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Berhasil");
			MAPPUTSTR(rptBuf9,buf,lblKO);



			key = rloPembayaranBriva;

			break;
		/*End Of Changed By Ali*/

		case trtPembayaranPLNPasca:

			singleReceipt=1; //@@OA  TAG PLN
			memset(buf,0,sizeof(buf));
			strcpy(buf,"");
			MAPPUTSTR(rptJudul,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"STRUK PEMBAYARAN");
			MAPPUTSTR(rptBuf1,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"TAGIHAN LISTRIK");
			MAPPUTSTR(rptBuf2,buf,lblKO);
			//id pelanggan
			memset(buf,0,sizeof(buf));
			mapGet(traInquiryAmt,buf,lenInquiryAmt);
			sprintf(str,"IDPEL        : %s",buf);//@@OA
			MAPPUTSTR(rptBuf3,str,lblKO);

			//Nama
			memset(buf,0,sizeof(buf));
			mapGet(traNamaPenerima,buf,lenNamaPemilik);
			sprintf(str,"Nama         : %s",buf);//@@OA
			MAPPUTSTR(rptBuf4,str,lblKO);

			//Tarif/daya
			memset(str,0,sizeof(str));
			memset(buf,0,sizeof(buf));
			memset(buf1,0,sizeof(buf1));
			memset(buf2,0,sizeof(buf2));
			mapGet(traCustomerSegment,buf,lenCustomerSegment);
			trimRight(buf,0x20);
			trimLeft(buf,0x20);
			mapGet(traPenaltyFee,buf1,lenAdminCharge);
			trimRight(buf1,0x20);
			trimLeft(buf1,0x20);
			sprintf(str,"Tarif/Daya   : %s/%d VA",buf,atoi(buf1));
			MAPPUTSTR(rptBuf5,str,lblKO);

			//BL / TH
			memset(buf,0,sizeof(buf));
			mapGet(traBuffer1,buf,lenBuffer1);
			sprintf(str,"BL/TH        : %s",buf);//@@OA
			MAPPUTSTR(rptBuf6,str,lblKO);

			//LWBP
			memset(str,0,sizeof(str));
			memset(buf,0,sizeof(buf));
			mapGet(traLWBP,buf,lenLWBP); //@@OA
			sprintf(str,"STAND METER  : %s",buf);//@@OA
			MAPPUTSTR(rptBuf7,str,lblKO);

			//TAGIHAN
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traNominalVoucher,amount,lenNominalVoucher); // @@OA
			sprintf(amount1, "%d", atoi(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount1, 0, ",.");
			sprintf(str,"RP TAG PLN   : %s",buf);//@@OA
			MAPPUTSTR(rptBuf8,str,lblKO);

			//MPLO
			memset(buf,0,sizeof(buf));
			mapGet(traUMK,buf,lenUMK);
			sprintf(str,"MLPO REF.    : %s",buf);//@@OA
			MAPPUTSTR(rptBuf9,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(str,"     PLN MENYATAKAN STRUK INI SEBAGAI BUKTI      ");
			MAPPUTSTR(rptPLNMsg1,str,lblKO);

			//Admin
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAdminCharge,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"ADMIN BANK : %s",buf);
			MAPPUTSTR(rptBuf10,str,lblKO);

			//amount
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR: %s",buf);
			MAPPUTSTR(rptBuf11,str,lblKO);
			MAPPUTSTR(rptBuf12," ",lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traTotalOutstanding,buf,lenTotalOutstanding);

			if((atoi(buf) > 0)){
				sprintf(str,"Anda Masih Memiliki Sisa Tunggakan %d Bulan",atoi(buf));
			}else
				strcpy(str,"*** Terima Kasih ***");

			MAPPUTSTR(rptBuf13,str,lblKO);
			MAPPUTSTR(rptBuf14," ",lblKO);
			MAPPUTSTR(rptBuf15,"Rincian Tagihan Dapat Diakses di",lblKO);
			MAPPUTSTR(rptBuf16,"         www.pln.co.id         ",lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNoKartuInternetBanking,buf,lenNoKartuInternetBanking);
			sprintf(str,"Atau Hub. PLN Terdekat : %s",buf);//@@OA
			MAPPUTSTR(rptBuf17,str,lblKO);

			key = rloPembayaranPLN;
			break;

		case trtPembayaranPLNToken:
			singleReceipt=1; //@@OA
			memset(buf,0,sizeof(buf));
			strcpy(buf,"PENCETAKAN TOKEN");
			MAPPUTSTR(rptBuf1,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"LISTRIK PRABAYAR");
			MAPPUTSTR(rptBuf2,buf,lblKO);

			//NO METER
			memset(buf,0,sizeof(buf));
			mapGet(traInstallmentPokok,buf,lenInstallmentPokok);
			sprintf(str,"NO METER           : %s",buf);//@@OA
			MAPPUTSTR(rptBuf3,str,lblKO);

			//ID Pelanggan
			memset(buf,0,sizeof(buf));
			mapGet(traPokokHutang,buf,lenPokokHutang);
			sprintf(str,"ID PEL             : %s",buf);//@@OA
			MAPPUTSTR(rptBuf4,str,lblKO);

			//NAMA
			memset(buf,0,sizeof(buf));
			mapGet(traNamaPenerima,buf,lenNamaPemilik);
			sprintf(str,"NAMA               : %s",buf);//@@OA
			MAPPUTSTR(rptBuf5,str,lblKO);

			//TIPE TARIF/DAYA
			memset(buf,0,sizeof(buf));
			memset(buf1,0,sizeof(buf1));
			mapGet(traBillPeriod1,buf,lenBillPeriod);
			trimRight(buf,0x20);
			trimLeft(buf,0x20);
			mapGet(traPenaltyFee,buf1,lenPenaltyFee);
			sprintf(str,"TARIF/DAYA         : %s/%d VA",buf,atoi(buf1));
			MAPPUTSTR(rptBuf6,str,lblKO);

			//MLPO REF
			memset(buf,0,sizeof(buf));
			mapGet(traUMK,buf,lenUMK);
			sprintf(str,"MLPO REF           : %s",buf);//@@OA
			MAPPUTSTR(rptBuf7,str,lblKO);
			sprintf(str,"                     %s",buf+27);//@@OA
			MAPPUTSTR(rptBuf8,str,lblKO);

			//RP BAYAR
			memset(buf,0,sizeof(buf));
			memset(amount, 0, sizeof(amount));
			memset(amount1, 0, sizeof(amount1));
			mapGet(traAmt,amount,lenAmt);
			strncpy(amount+strlen(amount),"00", 2);
			strcpy(buf,"Rp. ");
			fmtDelSpace(amount1, amount, lenAmt+2);
			fmtAmt(buf+4, amount1, 2, ",.");
			sprintf(str,"RP BAYAR           : %s",buf);//@@OA
			MAPPUTSTR(rptBuf9,str,lblKO);

			//ADMIN CA
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAdminCharge,amount,lenAdminCharge);
			strncpy(amount+strlen(amount),"00", 2);
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 2, ",.");
			sprintf(str,"ADMIN BANK         : %s",buf);//@@OA
			MAPPUTSTR(rptBuf10,str,lblKO);

			//MATERAI
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traPowerConsuming,amount,lenPowerConsuming);
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 2, ",.");
			sprintf(str,"METERAI            : %s",buf);//@@OA
			MAPPUTSTR(rptBuf11,str,lblKO);

			//PPN
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traInstallmentCicilan,amount,lenInstallmentCicilan);
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 2, ",.");
			sprintf(str,"PPN                : %s",buf);//@@OA
			MAPPUTSTR(rptBuf12,str,lblKO);

			//PPJ
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traBunga,amount,lenBunga);
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 2, ",.");
			sprintf(str,"PPJ                : %s",buf);//@@OA
			MAPPUTSTR(rptBuf13,str,lblKO);

			//ANGSURAN
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traBuffer1,amount,lenBuffer1);
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 2, ",.");
			sprintf(str,"ANGSURAN           : %s",buf);//@@OA
			MAPPUTSTR(rptBuf14,str,lblKO);

			//RP TOKEN
			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traNominalVoucher,amount,lenNominalVoucher);
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 2, ",.");
			sprintf(str,"RP STROOM/TOKEN    : %s",buf);//@@OA
			MAPPUTSTR(rptBuf15,str,lblKO);

			//JML KWH
			memset(buf,0,sizeof(buf));
			mapGet(traSisaSaldo,buf,lenSisaSaldo);
			sprintf(str,"JML KWH            : %s",buf);//@@OA
			MAPPUTSTR(rptBuf16,str,lblKO);

			//TOKEN
			memset(buf,0,sizeof(buf));
			sprintf(str,"STROOM/TOKEN       : ");//@@OA
			MAPPUTSTR(rptBuf17,str,lblKO);
			mapGet(traNoVoucherMitra,buf,lenNoVoucherMitra);
			memset(buf3,0,sizeof(buf3));
			strncpy(buf3, buf, lenCustomerSegment);
			strncpy(buf3+4, " ",1);
			strncpy(buf3+5,buf+4, lenCustomerSegment);
			strncpy(buf3+9," ",1);
			strncpy(buf3+10,buf+8, lenCustomerSegment);
			strncpy(buf3+14," ",1);
			strncpy(buf3+15,buf+12, lenCustomerSegment);
			strncpy(buf3+19," ",1);
			strncpy(buf3+20,buf+16, lenCustomerSegment);

			sprintf(str,"%s", buf3);//@@OA
			MAPPUTSTR(rptBuf18,str,lblKO);

			MAPPUTSTR(rptBuf19, "INFORMASI HUBUNGI CALL CENTER 123",lblKO);
			MAPPUTSTR(rptBuf20, "ATAU HUBUNGI PLN TERDEKAT",lblKO);


			//NO TELP
			memset(buf,0,sizeof(buf));
			mapGet(traNoTelpon,buf,lenNoTelpon);
			sprintf(str,"%s",buf);//@@OA
			MAPPUTSTR(rptBuf21,str,lblKO);

			key  = rloPembayaranPLNToken;
			break;

		case trtPembayaranPLNPra:

			MAPGET(traRspCod, RspCod, lblKO);
			if(strcmp(RspCod, "02") == 0){
				singleReceipt=1; //@@OA
				memset(buf,0,sizeof(buf));
				strcpy(buf,"PEMBAYARAN PLN");
				MAPPUTSTR(rptJudul,buf,lblKO);

				memset(buf,0,sizeof(buf));
				strcpy(buf,"STRUK PEMBELIAN");
				MAPPUTSTR(rptBuf1,buf,lblKO);

				memset(buf,0,sizeof(buf));
				strcpy(buf,"LISTRIK PRABAYAR");
				MAPPUTSTR(rptBuf2,buf,lblKO);

				//No Meteran
				memset(buf,0,sizeof(buf));
				mapGet(traTotalAmount,buf,lenTotalAmount);
				sprintf(str,"NO METER           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf3,str,lblKO);

				//ID Pelanggan
				memset(buf,0,sizeof(buf));
				mapGet(traPokokHutang,buf,lenPokokHutang);
				sprintf(str,"ID PEL             : %s",buf);//@@OA
				MAPPUTSTR(rptBuf4,str,lblKO);

				//Nama
				memset(buf,0,sizeof(buf));
				mapGet(traNamaPenerima,buf,lenNamaPemilik);
				sprintf(str,"NAMA               : %s",buf);//@@OA
				MAPPUTSTR(rptBuf5,str,lblKO);

				//Tarif/daya
				memset(str,0,sizeof(str));
				memset(buf,0,sizeof(buf));
				memset(buf1,0,sizeof(buf1));
				mapGet(traPaymentNumber,buf,lenPaymentNumber);
				trimRight(buf,0x20);
				trimLeft(buf,0x20);
				mapGet(traPenaltyFee,buf1,lenPenaltyFee);
				sprintf(str,"TARIF/DAYA         : %s/%d VA",buf,atoi(buf1));
				MAPPUTSTR(rptBuf6,str,lblKO);

				//MLPO REF
				memset(buf,0,sizeof(buf));
				mapGet(traUMK,buf,lenUMK);
				sprintf(str,"MLPO REF           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf7,str,lblKO);
				sprintf(str,"                     %s",buf+27);//@@OA
				MAPPUTSTR(rptBuf8,str,lblKO);

				//RP BAYAR
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,lenAmt);
				strncpy(amount+strlen(amount),"00", 2);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"RP BAYAR           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf9,str,lblKO);

				//ADMIN CA
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAdminCharge,amount,lenAdminCharge);
				strncpy(amount+strlen(amount),"00", 2);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"ADMIN BANK         : %s",buf);//@@OA
				MAPPUTSTR(rptBuf10,str,lblKO);

				MAPPUTSTR(rptBuf11, "Transaksi Belum Berhasil.",lblKO);
				MAPPUTSTR(rptBuf12, "Lakukan Cetak Token melalui EDC BRI",lblKO);
				MAPPUTSTR(rptBuf13, "di menu Cetak Token Listrik,",lblKO);
				MAPPUTSTR(rptBuf14, "HARI INI",lblKO);
				MAPPUTSTR(rptBuf15, "dengan menginput :",lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traTotalAmount,buf,lenTotalAmount);
				sprintf(str,"- Nomer Meteran : %s",buf);//@@OA
				MAPPUTSTR(rptBuf16,str,lblKO);

				memset(buf,0,sizeof(buf));
				mapGet(traRrn,buf,lenRrn);
				sprintf(str,"- Ref No Transaksi : %s",buf);//@@OA
				MAPPUTSTR(rptBuf17,str,lblKO);

				//strcpy(str,"*** Terima Kasih ***");
				//MAPPUTSTR(rptBuf16,str,lblKO);
				MAPPUTSTR(rptBuf18, "INFORMASI HUBUNGI CALL CENTER 123",lblKO);
				MAPPUTSTR(rptBuf19, "ATAU HUBUNGI PLN TERDEKAT",lblKO);

				key = rloPembayaranPLNPra;
			}
			else
			{
				singleReceipt=1; //@@OA
				memset(buf,0,sizeof(buf));
				memset(buf,0,sizeof(buf));
				strcpy(buf,"STRUK PEMBELIAN");
				MAPPUTSTR(rptBuf1,buf,lblKO);

				memset(buf,0,sizeof(buf));
				strcpy(buf,"LISTRIK PRABAYAR");
				MAPPUTSTR(rptBuf2,buf,lblKO);

				//NO METER
				memset(buf,0,sizeof(buf));
				mapGet(traInstallmentPokok,buf,lenInstallmentPokok);
				sprintf(str,"NO METER           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf3,str,lblKO);

				//ID Pelanggan
				memset(buf,0,sizeof(buf));
				mapGet(traPokokHutang,buf,lenPokokHutang);
				sprintf(str,"ID PEL             : %s",buf);//@@OA
				MAPPUTSTR(rptBuf4,str,lblKO);

				//NAMA
				memset(buf,0,sizeof(buf));
				mapGet(traNamaPenerima,buf,lenNamaPemilik);
				sprintf(str,"NAMA               : %s",buf);//@@OA
				MAPPUTSTR(rptBuf5,str,lblKO);

				//TIPE TARIF/DAYA
				memset(buf,0,sizeof(buf));
				memset(buf1,0,sizeof(buf1));
				mapGet(traBillPeriod1,buf,lenBillPeriod);
				trimRight(buf,0x20);
				trimLeft(buf,0x20);
				mapGet(traPenaltyFee,buf1,lenPenaltyFee);
				sprintf(str,"TARIF/DAYA         : %s/%d VA",buf,atoi(buf1));
				MAPPUTSTR(rptBuf6,str,lblKO);

				//MLPO REF
				memset(buf,0,sizeof(buf));
				mapGet(traUMK,buf,lenUMK);
				sprintf(str,"MLPO REF           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf7,str,lblKO);
				sprintf(str,"                     %s",buf+27);//@@OA
				MAPPUTSTR(rptBuf8,str,lblKO);

				//RP BAYAR
				memset(buf,0,sizeof(buf));
				memset(amount1, 0, sizeof(amount1));
				mapGet(traAmt,amount,lenAmt);
				strncpy(amount+strlen(amount),"00", 2);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"RP BAYAR           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf9,str,lblKO);

				//ADMIN CA
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAdminCharge,amount,lenAdminCharge);
				strncpy(amount+strlen(amount),"00", 2);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"ADMIN BANK         : %s",buf);//@@OA
				MAPPUTSTR(rptBuf10,str,lblKO);

				//MATERAI
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traPowerConsuming,amount,lenPowerConsuming);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"METERAI            : %s",buf);//@@OA
				MAPPUTSTR(rptBuf11,str,lblKO);

				//PPN
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traInstallmentCicilan,amount,lenInstallmentCicilan);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"PPN                : %s",buf);//@@OA
				MAPPUTSTR(rptBuf12,str,lblKO);

				//PPJ
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traBunga,amount,lenBunga);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"PPJ                : %s",buf);//@@OA
				MAPPUTSTR(rptBuf13,str,lblKO);

				//ANGSURAN
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traBuffer1,amount,lenBuffer1);
				amount[strlen(amount) -2] = 0;
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"ANGSURAN           : %s",buf);//@@OA
				MAPPUTSTR(rptBuf14,str,lblKO);

				//RP TOKEN
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traNominalVoucher,amount,lenNominalVoucher);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 2, ",.");
				sprintf(str,"RP STROOM/TOKEN    : %s",buf);//@@OA
				MAPPUTSTR(rptBuf15,str,lblKO);

				//JML KWH
				memset(buf,0,sizeof(buf));
				mapGet(traSisaSaldo,buf,lenSisaSaldo);
				sprintf(str,"JML KWH            : %s",buf);//@@OA
				MAPPUTSTR(rptBuf16,str,lblKO);

				//TOKEN
				memset(buf,0,sizeof(buf));
				sprintf(str,"STROOM/TOKEN       : ");//@@OA
				MAPPUTSTR(rptBuf17,str,lblKO);
				mapGet(traNoVoucherMitra,buf,lenNoVoucherMitra);
				memset(buf3,0,sizeof(buf3));
				strncpy(buf3, buf, lenCustomerSegment);
				strncpy(buf3+4, " ",1);
				strncpy(buf3+5,buf+4, lenCustomerSegment);
				strncpy(buf3+9," ",1);
				strncpy(buf3+10,buf+8, lenCustomerSegment);
				strncpy(buf3+14," ",1);
				strncpy(buf3+15,buf+12, lenCustomerSegment);
				strncpy(buf3+19," ",1);
				strncpy(buf3+20,buf+16, lenCustomerSegment);
				sprintf(str,"%s", buf3);//@@OA

				MAPPUTSTR(rptBuf18,str,lblKO);
				MAPPUTSTR(rptBuf19, "INFORMASI HUBUNGI CALL CENTER 123",lblKO);
				MAPPUTSTR(rptBuf20, "ATAU HUBUNGI PLN TERDEKAT",lblKO);

				//NO TELP
				memset(buf,0,sizeof(buf));
				mapGet(traNoTelpon,buf,lenNoTelpon);
				sprintf(str,"%s",buf);//@@OA
				MAPPUTSTR(rptBuf21,str,lblKO);

				key  = rloPembayaranPLNToken;
			}
			break;

		case trtPembayaranCicilan:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			mapGet(traSubType,buf,1);

			switch(buf[0])
			{
				case CICILAN_FIF:
					strcpy(str,"PEMBAYARAN CICILAN FIF");
					strcpy(buf1,"       FIF MENYATAKAN TANDA TERIMA INI");
					break;

				case CICILAN_BAF:
					strcpy(str,"PEMBAYARAN CICILAN BAF");
					strcpy(buf1,"       BAF MENYATAKAN TANDA TERIMA INI");
					break;

				case CICILAN_OTO:
					strcpy(str,"PEMBAYARAN CICILAN OTO");
					strcpy(buf1,"       OTO MENYATAKAN TANDA TERIMA INI");
					break;

				case CICILAN_FNS:
					strcpy(str,"PEMB CICILAN FINANSIA");
					strcpy(buf1,"      FINANSIA MENYATAKAN TANDA TERIMA INI");
					break;

		    		case CICILAN_VER:
			    		strcpy(str,"PEMB CICILAN VERENA");
			    		strcpy(buf1,"      VERENA MENYATAKAN TANDA TERIMA INI");
			    		break;

		    		case CICILAN_WOM:
			    		strcpy(str,"PEMBAYARAN CICILAN WOM");
			    		strcpy(buf1,"       WOM MENYATAKAN TANDA TERIMA INI");
			    		break;
			}

			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);
			strcpy(buf1,"      SEBAGAI BUKTI PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traContractNumber,buf,sizeof(buf));
			sprintf(str,"No Kontrak  : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traCustomerName,buf,sizeof(buf));
			sprintf(str,"Nama        : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(buf1,0,sizeof(buf));
			mapGet(traDueDate,buf,sizeof(buf));
			fmtIns(buf1,buf,"-",2,sizeof(buf1));
			memset(buf,0,sizeof(buf));
			fmtIns(buf,buf1,"-",5,sizeof(buf));
			sprintf(str,"Jatuh tempo : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traPaymentNumber,buf,sizeof(buf));
			sprintf(str,"Angsuran Ke : %s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traTotalAmount,amount,sizeof(amount));
			dec2num(&Amt, amount, strlen(amount));
			memset(amount,0,sizeof(amount));
			Amt = Amt/100;
			num2dec(amount,Amt,0);
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Tagihan     : %s",buf);
			MAPPUTSTR(rptBuf5,str,lblKO);

			sprintf(str,"Pembayaran  : %s",buf);
			MAPPUTSTR(rptBuf6,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMPLO,buf,sizeof(buf));
			sprintf(str,"MPLO Ref    : %s",buf);
			MAPPUTSTR(rptBuf7,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranPulsa:
			singleReceipt=1; //@@
			memset(buf,0,sizeof(buf));
			mapGet(traNoTelpon,buf,sizeof(buf));

			memset(buf2,0,sizeof(buf2));
			mapGet(traSubType,buf2,1);

			if((buf2[0]/10) == 5){
				memset(buf1,0,sizeof(buf1));
				mapGet(traBillPeriod1,buf1,sizeof(buf1));
				strcat(buf1,buf);
				sprintf(str,"No HP         : %s",buf1);
			} else if((buf2[0]/10) == 3){
				memset(buf1,0,sizeof(buf1));
				memset(buf2,0,sizeof(buf2));
				memset(buf3,0,sizeof(buf3));
				memset(buf4,0,sizeof(buf4));
				mapGet(traNoTelpon,buf1,sizeof(buf1));
				fmtDelSpace(buf2,buf1,lenNoTelpon);
				MAPGETBYTE(traJenisKartu, im3temp, lblKO);

				if(im3temp==10){
					strncpy(buf3, buf2, 4);
					strncpy(buf4, buf2+4, 3);
					if((atoi(buf4) == 0))
						strncpy(buf3+4, buf2+7, strlen(buf2)-7);
				}
				else if(im3temp==11){
					strncpy(buf3, buf2, 4);
					strncpy(buf4, buf2+4, 2);
					if((atoi(buf4) == 0))
						strncpy(buf3+4, buf2+6, strlen(buf2)-6);
				}
				else if(im3temp==12){
					strncpy(buf3, buf2, 4);
					strncpy(buf4, buf2+4, 1);
					if((atoi(buf4) == 0))
						strncpy(buf3+4, buf2+5, strlen(buf2)-5);
				}
				else
					strncpy(buf3+4, buf2+4, strlen(buf2)-4);
				sprintf(str,"No HP         : %s",buf3);
			}
			else
				sprintf(str,"No HP         : %s",buf);

			MAPPUTSTR(rptBuf1,str,lblKO);
			memset(amount,0,sizeof(amount));
			mapGet(traInquiryAmt,amount,sizeof(amount));
			amount[strlen(amount) - 2] = 0; // @@OA
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Amount        : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(str,"Pulsa Otomatis Bertambah");
			MAPPUTSTR(rptPLNMsg1,str,lblKO);

			mapGet(traSubType,buf,1);
			MAPPUTSTR(rptBuf3,"",lblKO);
			MAPPUTSTR(rptBuf4,"",lblKO);
			switch(buf[0]/10)
			{
				case 1: //SIMPATI
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG SIMPATI/AS");
					MAPPUTSTR(rptJudul,str,lblKO);

					memset(buf,0,sizeof(buf));
					mapGet(traPeriode,buf,sizeof(buf));
					sprintf(str,"Periode       : %s",buf);
					MAPPUTSTR(rptBuf3,str,lblKO);

					memset(buf,0,sizeof(buf));
					mapGet(traNoVoucher,buf,sizeof(buf));
					sprintf(str,"Voucher NO.   : %s",buf);
					MAPPUTSTR(rptBuf4,str,lblKO);

					key = rloPembayaranPulsaSimpati;
					break;

				case 2: //Mentari
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG MENTARI");
					MAPPUTSTR(rptJudul,str,lblKO);
					key = rloPembayaranPulsaMentari;
					break;

				case 3: //IM3
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG IM3");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					mapGet(traPeriode,buf,sizeof(buf));
					sprintf(str,"Periode       : %s",buf);
					MAPPUTSTR(rptBuf3,str,lblKO);

					key = rloPembayaranPulsaIM3;
					break;

				case 4: //XL
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG XL");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					mapGet(traNoVoucherMitra,buf,sizeof(buf));
					sprintf(str,"Kode Vouchers : %s",buf);
					MAPPUTSTR(rptBuf3,str,lblKO);
					key = rloPembayaranPulsaXL;
					break;

				/*Changed By Ali*/
				case 5: //Esia
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG ESIA");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					key = rloPembayaranPulsaXL;
					break;

				case 6: //Smart
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG SMART FRIEND");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					key = rloPembayaranPulsaXL;
					break;

				case 7: //Fren
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG FREN");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					key = rloPembayaranPulsaXL;
					break;

				case 8: //Three
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG THREE");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					key = rloPembayaranPulsaXL;
					break;

				case 9: //Axis
					memset(str,0,sizeof(str));
					strcpy(str,"ISI ULANG AXIS");
					MAPPUTSTR(rptJudul,str,lblKO);
					memset(buf,0,sizeof(buf));
					key = rloPembayaranPulsaXL;
					break;
				/*End Of Changed By Ali*/
			}
			break;

		case trtPembayaranZakat:
			singleReceipt=1; //@@OA
			memset(buf,0,sizeof(buf));
			mapGet(traSubType,buf,1);
			if( (buf[0]/10) == 5)
				mapPut(rptJudul,"SETORAN ZAKAT",13);
			else
				mapPut(rptJudul,"SETORAN INFAQ",13);

			if((buf[0]%10) == 1)
				mapPut(rptNamaPenerima,"Nama Pengelola     : Dompet Dhuafa",34);
			else
				mapPut(rptNamaPenerima,"Nama Pengelola     : YBM",24);

			memset(buf,0,sizeof(buf));
			mapGet(traNomorRekeningTujuanBRI,buf,lenNomorRekeningBRI);
			sprintf(str,"Rekening Pengelola : %s",buf);
			MAPPUTSTR(rptNomorRekeningPenerima,str,lblKO);
			MAPPUTSTR(rptJumlah,"Jumlah             : ",lblKO);

			key = rloPembayaranZakat;
			break;

		case trtSetorPasti:
			singleReceipt = 1;
			strcpy(str,"SETOR PASTI");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(RspCod, 0, sizeof(RspCod));
			MAPGET(traRspCod, RspCod, lblKO);
			if(strcmp(RspCod, "68") == 0)
			{
				memset(buf,0,sizeof(buf));
				MAPGETBYTE(traKategoriKartu,jenis,lblKO);
             			if(jenis > 2)
             			{
					mapGet(traTotalOutstanding,buf,lenTotalOutstanding);

					if((strcmp(buf, "00") == 0) || (strcmp(buf, "01") == 0) || (strcmp(buf, "02") == 0) || (strcmp(buf, "03") == 0))
					{
						memset(buf1,0,sizeof(buf1));
						strcpy(buf1,"Melalui Bank Lain");
						MAPPUTSTR(rptJudul1,buf1,lblKO);

						memset(amount,0,sizeof(amount));
						mapGet(traAmt,amount,lenAmt);
						strcpy(buf,"Rp. ");
						fmtAmt(buf+4, amount, 0, ",.");
						sprintf(str,"Jumlah Transaksi             : %s",buf);
						MAPPUTSTR( rptBuf11,str,lblKO);
						MAPPUTSTR( rptPLNMsg1, "Transaksi Sedang Diproses.",lblKO);
						MAPPUTSTR( rptBuf12, "Cek Saldo Anda atau Hubungi Call BRI",lblKO);
						MAPPUTSTR( rptBuf13, "14017.",lblKO);

						key = rloPembayaranPasti;
					}
             			}
				else
				{
					memset(buf1,0,sizeof(buf1));
					strcpy(buf1,"Antar BRI");

					MAPPUTSTR(rptJudul1,buf1,lblKO);
					strcpy(buf1," ");
					MAPPUTSTR(rptJudul2,buf1,lblKO);
					memset(amount,0,sizeof(amount));
					mapGet(traAmt,amount,lenAmt);
					strcpy(buf,"Rp. ");
					fmtAmt(buf+4, amount, 0, ",.");
					sprintf(str,"Jumlah Transaksi             : %s",buf);
					MAPPUTSTR(rptBuf11,str,lblKO);
					MAPPUTSTR( rptPLNMsg1, "Transaksi Sedang Diproses.",lblKO);
					MAPPUTSTR( rptBuf12, "Cek Saldo Anda atau Hubungi Call BRI",lblKO);
					MAPPUTSTR( rptBuf13, "14017.",lblKO);

					key = rloPembayaranPasti;
				}
			}
			else
			{
				MAPGETBYTE(traKategoriKartu,jenis,lblKO);
             			if(jenis > 2)
             			{
					memset(buf,0,sizeof(buf));
					mapGet(traTotalOutstanding,buf,lenTotalOutstanding);
					if((strcmp(buf, "00") == 0)|| (strcmp(buf, "01") == 0) || (strcmp(buf, "02") == 0) || (strcmp(buf, "03") == 0))
					{
						memset(buf1,0,sizeof(buf1));
						strcpy(buf1,"Melalui Bank Lain");
						MAPPUTSTR(rptJudul1,buf1,lblKO);

						memset(buf1,0,sizeof(buf1));
						if(strcmp(buf, "01") == 0)
						strcpy(buf1,"(LINK)");
						else if(strcmp(buf, "02") == 0)
						strcpy(buf1,"(PRIMA)");
						else if(strcmp(buf, "03") == 0)
						strcpy(buf1,"(BERSAMA)");

						MAPPUTSTR(rptJudul2,buf1,lblKO);

						//ASAL
						memset(buf,0,sizeof(buf));
						strcpy(str,"ASAL");
						MAPPUTSTR(rptBuf3,str,lblKO);

						//BANK
						memset(buf,0,sizeof(buf));
						mapGet(traNamaBankAsal,buf,sizeof(buf));
						sprintf(str,"Bank               : %s",buf);
						MAPPUTSTR(rptBuf4,str,lblKO);

						//Nama
						memset(buf,0,sizeof(buf));
						mapGet(traNomorRekeningTujuanBRI,buf,lenNomorRekeningBRI);
						sprintf(str,"Nama               : %s",buf);
						MAPPUTSTR(rptBuf5,str,lblKO);

						//NO. REK
						memset(buf,0,sizeof(buf));
						mapGet(traNamaBankTujuan,buf,lenNamaBank);
						sprintf(str,"No. Rek            : %s",buf);
						MAPPUTSTR(rptBuf6,str,lblKO);

						//TUJUAN
						memset(buf,0,sizeof(buf));
						strcpy(str,"TUJUAN");
						MAPPUTSTR(rptBuf7,str,lblKO);

						//BANK
						memset(buf,0,sizeof(buf));
						strcpy(buf,"BRI");
						sprintf(str,"Bank               : %s",buf);
						MAPPUTSTR(rptBuf8,str,lblKO);

						//Nama
						memset(buf,0,sizeof(buf));
						mapGet(traNomorRekeningTujuan,buf,lenNomorRekening);
						sprintf(str,"Nama               : %s",buf);
						MAPPUTSTR(rptBuf9,str,lblKO);

						//NO. REK
						memset(buf,0,sizeof(buf));
						mapGet(traNomorRekeningAsalBRI,buf,lenNomorRekeningBRI);
						sprintf(str,"No. Rek            : %s",buf);
						MAPPUTSTR(rptBuf10,str,lblKO);
						MAPPUTSTR( rptPLNMsg1, "BERHASIL",lblKO);
					}
             			}
				else
				{
					memset(buf,0,sizeof(buf));
					strcpy(buf,"Antar BRI");
					MAPPUTSTR(rptJudul1,buf,lblKO);

					strcpy(buf1," ");
					MAPPUTSTR(rptJudul2,buf1,lblKO);

					//ASAL
					memset(buf,0,sizeof(buf));
					strcpy(str,"ASAL");
					MAPPUTSTR(rptBuf3,str,lblKO);

					//BANK
					memset(buf,0,sizeof(buf));
					strcpy(buf,"BRI");
					sprintf(str,"Bank               : %s",buf);
					MAPPUTSTR(rptBuf4,str,lblKO);

					//Nama
					memset(buf,0,sizeof(buf));
					mapGet(traNomorRekeningTujuanBRI,buf,lenNomorRekeningBRI);
					sprintf(str,"Nama               : %s",buf);
					MAPPUTSTR(rptBuf5,str,lblKO);

					//NO. REK
					memset(buf,0,sizeof(buf));
					mapGet(traNomorRekeningAsalBRI,buf,lenNomorRekeningBRI);
					sprintf(str,"No. Rek            : %s",buf);
					MAPPUTSTR(rptBuf6,str,lblKO);

					//TUJUAN
					memset(buf,0,sizeof(buf));
					strcpy(str,"TUJUAN");
					MAPPUTSTR(rptBuf7,str,lblKO);

					//BANK
					memset(buf,0,sizeof(buf));
					strcpy(buf,"BRI");
					sprintf(str,"Bank               : %s",buf);
					MAPPUTSTR(rptBuf8,str,lblKO);

					//Nama
					memset(buf,0,sizeof(buf));
					mapGet(traNomorRekeningAsal,buf,lenNomorRekening);
					sprintf(str,"Nama               : %s",buf);
					MAPPUTSTR(rptBuf9,str,lblKO);

					//NO. REK
					memset(buf,0,sizeof(buf));
					mapGet(traNamaBankAsal,buf,lenNamaBank);
					sprintf(str,"No. Rek            : %s",buf);
					MAPPUTSTR(rptBuf10,str,lblKO);
					MAPPUTSTR( rptPLNMsg1, "BERHASIL",lblKO);
				}

				//JUMLAH
				memset(buf,0,sizeof(buf));
				memset(amount,0,sizeof(amount));
				mapGet(traAmt,amount,lenAmt);
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4, amount, 0, ",.");
				sprintf(str,"Jumlah             : %s",buf);
				MAPPUTSTR(rptBuf11,str,lblKO);


				key = rloPembayaranPasti;
			}
			break;

		case trtSetorSimpanan:
		case trtSetorPinjaman:
			singleReceipt=1; //@@OA
			if(txnType == trtSetorSimpanan)
			strcpy(str,"SETORAN SIMPANAN");
			else
			strcpy(str,"SETORAN PINJAMAN");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNomorRekeningAsalBRI,buf,lenNomorRekeningBRI);
			sprintf(str,"No Rekening : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaPenerima,buf,lenNamaPemilik);
			sprintf(str,"Nama        : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Total       : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			key = rloSetorSimpanan;
			break;

		case trtTarikTunai:
		case trtVoidTarikTunai:
			singleReceipt=0; //@@OA
			if(txnType == trtTarikTunai)
				strcpy(str,"TARIK TUNAI");
			else
				strcpy(str,"VOID TARIK TUNAI");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Jml Penarikan : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traFee,amount,sizeof(amount));
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Fee           : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traSisaSaldo,amount,sizeof(amount));
			amount[strlen(amount) - 2] = 0;
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Saldo         : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			key = rloTarikTunai;
			break;

		case trtTCash:
			singleReceipt = 1;
			strcpy(str,"CASH IN T-BANK");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaPenerima,buf,lenNamaPemilik);
			sprintf(str,"No HandPhone : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"Jumlah       : %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloSetorSimpanan;
			break;

		case trtTCashOut:
			break;

		case trtRegistrasiInternetBanking:
			singleReceipt = 1;
			strcpy(str,"REG INTERNET BANKING");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"       BERHASIL");
			MAPPUTSTR(rptBuf1,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Internet Banking BRI dapat diakses");
			MAPPUTSTR(rptBuf2,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"melalui Internet dengan alamat");
			MAPPUTSTR(rptBuf3,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"https://ib.bri.co.id");
			MAPPUTSTR(rptBuf4,buf,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traUserId,buf,lenUserId);
			sprintf(str,"User ID : %s",buf);
			MAPPUTSTR(rptBuf5,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Gunakan User ID dan PIN Internet");
			MAPPUTSTR(rptBuf6,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Banking Anda untuk login. Selanjutnya");
			MAPPUTSTR(rptBuf7,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"ikuti petunjuk yang ada di Internet");
			MAPPUTSTR(rptBuf8,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Banking BRI.");
			MAPPUTSTR(rptBuf9,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Untuk transaksi Finansial melalui");
			MAPPUTSTR(rptBuf10,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Internet Banking, daftarkan nomor HP");
			MAPPUTSTR(rptBuf11,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Anda sebagai Token di kantor BRI");
			MAPPUTSTR(rptBuf12,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"terdekat.");
			MAPPUTSTR(rptBuf13,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Untuk keamanan transaksi, jaga");
			MAPPUTSTR(rptBuf14,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"kerahasiaan User ID dan Password");
			MAPPUTSTR(rptBuf15,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Internet Banking Anda.");
			MAPPUTSTR(rptBuf16,buf,lblKO);

			key = rloRegistrasiInternetBanking;
			break;

		case trtRegistrasiPhoneBanking:
			singleReceipt = 1;
			strcpy(str,"REGISTRASI PHONE BANKING");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"   REGISTRASI BERHASIL");
			MAPPUTSTR(rptBuf1,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"Mulai saat ini anda dapat");
			MAPPUTSTR(rptBuf2,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"melakukan transaksi Phone Banking");
			MAPPUTSTR(rptBuf3,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"melalui Call BRI");
			MAPPUTSTR(rptBuf4,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"ke 14017 atau 021-57987400");
			MAPPUTSTR(rptBuf5,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"JAGA KERAHASIAAN PIN KARTU ANDA.");
			MAPPUTSTR(rptBuf6,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"HATI-HATI TERHADAP PENIPUAN.");
			MAPPUTSTR(rptBuf7,buf,lblKO);

			key = rloRegistrasiPhoneBanking;
			break;

		/*Changed By Ali*/
		case trtRegistrasiSmsBanking:
			singleReceipt = 1;
			strcpy(str,"REGISTRASI SMS BANKING");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNoTelpon,buf,sizeof(buf));
			sprintf(str,"Nomer HP  : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"   REGISTRASI BERHASIL");
			MAPPUTSTR(rptBuf2,buf,lblKO);

			key = rloRegistrasiSmsBanking;
			break;

		case trtPembayaranKKBRI:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));
			mapGet(traSubType,buf,1);

			strcpy(str,"PEMB. KK BRI");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traNamaPengirim,buf,sizeof(buf));
			sprintf(str,"NAMA          : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traPelangganId,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL TAGIHAN : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traTotalAmount,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"MIN PAYMENT   : %28s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traDueDate,buf,sizeof(buf));
			sprintf(str,"JATUH TEMPO   : %s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf5,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKANZ:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KK ANZ");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKCITIKK:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KK CITIBANK");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKCITIKTA:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KTA CITIBANK");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKHSBCKK:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KK HSBC");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKHSBCKTA:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KTA HSBC");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKRBSKK:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KK RBS");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKRBSKTA:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KTA RBS");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKStanCharKK:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KK SCB");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			memset(buf1,0,sizeof(buf1));
			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;

		case trtPembayaranKKStanCharKTA:
			singleReceipt=1; //@@OA
			memset(str,0,sizeof(str));

			strcpy(str,"PEMB. KTA SCB");
			MAPPUTSTR(rptJudul,str,lblKO);

			strcpy(buf1,"      HARAP STRUK INI DISIMPAN SEBAGAI BUKTI");
			MAPPUTSTR(rptPLNMsg1,buf1,lblKO);

			strcpy(buf1,"              PEMBAYARAN YANG SAH");
			MAPPUTSTR(rptPLNMsg2,buf1,lblKO);

			memset(buf,0,sizeof(buf));
			mapGet(traMeter,buf,sizeof(buf));
			sprintf(str,"No Kartu      : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(amount,0,sizeof(amount));
			mapGet(traAmt,amount,sizeof(amount));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, amount, 0, ",.");
			sprintf(str,"TOTAL BAYAR   : %28s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			key = rloPembayaranCicilan;
			break;
		/*End Of Changed By Ali*/

		case trtAktivasiKartu:
			singleReceipt = 1;
			strcpy(str,"AKTIVASI KARTU");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traPan,buf,sizeof(buf));
			sprintf(str,"Nomor Kartu   : %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traNomorRekeningAsalBRI,buf,sizeof(buf));
			sprintf(str,"Nomor Rekening: %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traNamaPenerima,buf,30);
			trimRight(buf,0x20);
			sprintf(str,"Nama Nasabah  : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"AKTIFASI KARTU BERHASIL");
			MAPPUTSTR(rptBuf4,buf,lblKO);

			MAPGET(traRspCod, RspCod, lblKO);
			if(strcmp(RspCod, "NC") == 0)
			{
				strcpy(str,"Tetapi Pembentukan PIN baru gagal");
				MAPPUTSTR(rptBuf8,str,lblKO);
				strcpy(str,"Segera lakukan Reissue PIN");
				MAPPUTSTR(rptBuf9,str,lblKO);
				strcpy(str,"Melalui EDC BRI");
				MAPPUTSTR(rptBuf10,str,lblKO);
			}
			else
			{
				MAPPUTSTR(rptBuf8,"",lblKO);
				MAPPUTSTR(rptBuf9,"",lblKO);
				MAPPUTSTR(rptBuf10,"",lblKO);
			}

			memset(buf,0,sizeof(buf));
			strcpy(buf,"PASTIKAN ANDA REGISTRASI");
			MAPPUTSTR(rptBuf5,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"PHONE BANKING & SMS BANKING");
			MAPPUTSTR(rptBuf6,buf,lblKO);

			memset(buf,0,sizeof(buf));
			strcpy(buf,"MELALUI EDC ATAU ATM BRI");
			MAPPUTSTR(rptBuf7,buf,lblKO);

			key = rloAktivasiKartu;

			break;

		case trtRenewalKartu:
		{
			int isPB = 0, isSB = 0, isPIN = 1, isSBRegUnreg = 0;
			singleReceipt = 1;
			strcpy(str,"RENEWAL KARTU");
			MAPPUTSTR(rptJudul,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traNomorKartuLama,buf,sizeof(buf));
			sprintf(str,"Nomor Kartu Lama: %s",buf);
			MAPPUTSTR(rptBuf1,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traPan,buf,sizeof(buf));
			sprintf(str,"Nomor Kartu Baru: %s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traNomorRekeningAsalBRI,buf,sizeof(buf));
			sprintf(str,"Nomor Rekening  : %s",buf);
			MAPPUTSTR(rptBuf3,str,lblKO);

			memset(buf,0,sizeof(buf));
			memset(str,0,sizeof(str));
			mapGet(traNamaPenerima,buf,30);
			trimRight(buf,0x20);
			sprintf(str,"Nama Nasabah    : %s",buf);
			MAPPUTSTR(rptBuf4,str,lblKO);

			// @@OA - mulai
			memset(buf,0,sizeof(buf));
			mapGet(traStatusKartu,buf,2);
			if(strcmp(buf, "03") == 0)
				isPB = 1;
			MAPGET(traRspCod, RspCod, lblKO);

			if(strcmp(RspCod, "NC") == 0)
				isPIN = 0;

			memset(buf,0,sizeof(buf));
			mapGet(traNoTelpon,buf,lenNoTelpon);

			if(memcmp(buf,"0000000000000",13)==0)
			{
				isSB = 0;
			}
			else if(memcmp(buf,"9999999999999",13)==0)
			{
				isSBRegUnreg = 1;
			}
			else
				isSB = 1;

			memset(str,0,sizeof(str));
			MAPPUTSTR(rptBuf5,str,lblKO);
			MAPPUTSTR(rptBuf6,str,lblKO);
			MAPPUTSTR(rptBuf7,str,lblKO);
			MAPPUTSTR(rptBuf8,str,lblKO);
			MAPPUTSTR(rptBuf9,str,lblKO);
			MAPPUTSTR(rptBuf10,str,lblKO);
			MAPPUTSTR(rptBuf11,str,lblKO);
			MAPPUTSTR(rptBuf12,str,lblKO);
			MAPPUTSTR(rptBuf13,str,lblKO);
			MAPPUTSTR(rptBuf14,str,lblKO);
			MAPPUTSTR(rptBuf15,str,lblKO);
			MAPPUTSTR(rptBuf16,str,lblKO);

			if(isSB ||isPB)
			{
				strcpy(str,"REGISTRASI YANG SUDAH DILAKUKAN");
				MAPPUTSTR(rptBuf5,str,lblKO);
				strcpy(str,"");
				if(isPB)
				{
					strcpy(str,"- PHONE BANKING");
				}
				MAPPUTSTR(rptBuf6,str,lblKO);
				strcpy(str,"");

				if(isSB)
				{
					strcpy(str,"- SMS BANKING");
				}
				MAPPUTSTR(rptBuf7,str,lblKO);
				strcpy(str," ");
				MAPPUTSTR(rptBuf8,str,lblKO);
				strcpy(str,"RENEWAL KARTU BERHASIL");
				MAPPUTSTR(rptBuf9,str,lblKO);

				if(isPIN)
				{
					if(isSB != isPB)
					{
						strcpy(str," ");
						MAPPUTSTR(rptBuf10,str,lblKO);
						strcpy(str,"PASTIKAN ANDA MELAKUKAN REGISTRASI");
						MAPPUTSTR(rptBuf11,str,lblKO);
						if(!isSB)
						strcpy(str,"SMS BANKING");
						else
						strcpy(str,"PHONE BANKING");
						MAPPUTSTR(rptBuf12,str,lblKO);
						strcpy(str,"MELALUI ATM ATAU EDC BRI");
						MAPPUTSTR(rptBuf13,str,lblKO);
					}
				}
				else
				{
					strcpy(str,"Tetapi Pembentukan PIN baru gagal");
					MAPPUTSTR(rptBuf10,str,lblKO);
					strcpy(str,"Segera lakukan Reissue PIN");
					MAPPUTSTR(rptBuf11,str,lblKO);
					strcpy(str,"Melalui EDC BRI");
					MAPPUTSTR(rptBuf12,str,lblKO);

					if(isSB != isPB)
					{
						strcpy(str," ");
						MAPPUTSTR(rptBuf13,str,lblKO);
						strcpy(str,"PASTIKAN ANDA MELAKUKAN REGISTRASI");
						MAPPUTSTR(rptBuf14,str,lblKO);
						if(!isSB)
							strcpy(str,"SMS BANKING");
						else
							strcpy(str,"PHONE BANKING");

						MAPPUTSTR(rptBuf15,str,lblKO);
						strcpy(str,"MELALUI ATM ATAU EDC BRI");
						MAPPUTSTR(rptBuf16,str,lblKO);
					}
				}
			}
			else
			{
				strcpy(str,"RENEWAL KARTU BERHASIL");
				MAPPUTSTR(rptBuf9,str,lblKO);
				if(isPIN)
				{
					strcpy(str," ");
					MAPPUTSTR(rptBuf10,str,lblKO);
					strcpy(str,"PASTIKAN ANDA MELAKUKAN REGISTRASI");
					MAPPUTSTR(rptBuf11,str,lblKO);
					strcpy(str,"SMS BANKING & PHONE BANKING");
					MAPPUTSTR(rptBuf12,str,lblKO);
					strcpy(str,"MELALUI ATM ATAU EDC BRI");
					MAPPUTSTR(rptBuf13,str,lblKO);
				}
				else
				{
					strcpy(str,"Tetapi Pembentukan PIN baru gagal");
					MAPPUTSTR(rptBuf10,str,lblKO);
					strcpy(str,"Segera lakukan Reissue PIN");
					MAPPUTSTR(rptBuf11,str,lblKO);
					strcpy(str,"Melalui EDC BRI");
					MAPPUTSTR(rptBuf12,str,lblKO);
					strcpy(str," ");
					MAPPUTSTR(rptBuf13,str,lblKO);
					strcpy(str,"PASTIKAN ANDA MELAKUKAN REGISTRASI");
					MAPPUTSTR(rptBuf14,str,lblKO);
					strcpy(str,"SMS BANKING & PHONE BANKING");
					MAPPUTSTR(rptBuf15,str,lblKO);
					strcpy(str,"MELALUI ATM ATAU EDC BRI");
					MAPPUTSTR(rptBuf16,str,lblKO);
			        }
            		}
		key = rloRenewalKartu;
	}
		break;

	case trtReissuePIN:
		singleReceipt = 1;
		strcpy(str,"REISSUE PIN");
		MAPPUTSTR(rptJudul,str,lblKO);

		memset(buf,0,sizeof(buf));
		memset(str,0,sizeof(str));
		mapGet(traPan,buf,sizeof(buf));
		sprintf(str,"No Kartu : %s",buf);
		MAPPUTSTR(rptBuf1,str,lblKO);

		memset(buf,0,sizeof(buf));
		memset(str,0,sizeof(str));
		mapGet(traNomorRekeningAsalBRI,buf,sizeof(buf));
		sprintf(str,"No REK   : %s",buf);
		MAPPUTSTR(rptBuf2,str,lblKO);

		memset(buf,0,sizeof(buf));
		strcpy(buf,"REISSUE PIN BERHASIL");
		MAPPUTSTR(rptBuf3,buf,lblKO);

		memset(buf,0,sizeof(buf));
		strcpy(buf,"KARTU TELAH AKTIF KEMBALI DENGAN PIN BARU");
		MAPPUTSTR(rptBuf4,buf,lblKO);

		key = rloReissuePIN;
		break;

	case trtGantiPIN:
		singleReceipt=1;
		strcpy(str,"UBAH PIN");
		MAPPUTSTR(rptJudul,str,lblKO);

		strcpy(str,"BERHASIL");
		MAPPUTSTR(rptBuf1,str,lblKO);

		key = rloGantiPIN;
		break;

	case trtGantiPasswordSPV:
		singleReceipt=1;
		strcpy(str,"UBAH PASSWORD SPV");
		MAPPUTSTR(rptJudul,str,lblKO);

		strcpy(str,"PASSWORD SUPERVISOR BERHASIL DIUBAH");
		MAPPUTSTR(rptBuf1,str,lblKO);
		key = rloGantiPasswordSPV;
		break;

	case trtPembayaranPascaBayar:
		MAPGET(traRspCod, RspCod, lblKO);
		mapGet(traSubType,&subType,1);

		switch(subType)
		{
			case PASCA_BAYAR_HALO:
				strcpy(str,"PEMBAYARAN KARTU HALO");
				MAPPUTSTR(rptJudul,str,lblKO);

				strcpy(str,"No HALO        :");
				mapGet(traNoTelpon,buf,sizeof(buf));
				strcat(str,buf);
				MAPPUTSTR(rptBuf1,str,lblKO);

				strcpy(str,"Nama           :");
				mapGet(traNamaPenerima,buf,sizeof(buf));
				trimRight(buf,'0');
				strcat(str,buf);
				MAPPUTSTR(rptBuf2,str,lblKO);

				strcpy(str,"Jumlah Tagihan :");

				mapGet(traTotalAmount,buf1,sizeof(buf1));
				trimLeft(buf1,'0');
				if(strlen(buf1) >=2)
					buf1[strlen(buf1)-2] = 0;
				memset(buf,0,sizeof(buf));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4,buf1,0,",.");
				strcat(str,buf);

				MAPPUTSTR(rptBuf3,str,lblKO);

				strcpy(str,"PEMBAYARAN BERHASIL");
				MAPPUTSTR(rptBuf4,str,lblKO);
				if(strcmp(RspCod, "68") == 0)
				{
					singleReceipt = 1;
					strcpy(str,"PEMBAYARAN SEDANG DIPROSES");
					MAPPUTSTR(rptBuf4,str,lblKO);
					key = rloPembayaranPascaBayarSedangProses;
				}
				else
				{
					singleReceipt = 1;
					strcpy(str,"PEMBAYARAN BERHASIL");
					MAPPUTSTR(rptBuf4,str,lblKO);
					key = rloPembayaranPascaBayarHalo;
				}
				break;

			case PASCA_BAYAR_MATRIX:
				strcpy(str,"PEMBAYARAN MATRIX");
				MAPPUTSTR(rptJudul,str,lblKO);

				strcpy(str,"No MATRIX      : ");
				mapGet(traNoTelpon,buf,sizeof(buf));
				strcat(str,buf);
				MAPPUTSTR(rptBuf1,str,lblKO);

				strcpy(str,"Nama           :");
				mapGet(traNamaPenerima,buf,sizeof(buf));
				trimRight(buf,'0');
				strcat(str,buf);
				MAPPUTSTR(rptBuf2,str,lblKO);

				strcpy(str,"Jumlah Tagihan :");

				mapGet(traTotalAmount,buf1,sizeof(buf1));
				buf1[strlen(buf1)-2] = 0;
				memset(buf,0,sizeof(buf));
				strcpy(buf,"Rp. ");
				fmtAmt(buf+4,buf1,0,",.");
				strcat(str,buf);

				MAPPUTSTR(rptBuf3,str,lblKO);
				if(strcmp(RspCod, "68") == 0)
				{
					singleReceipt = 1;
					strcpy(str,"PEMBAYARAN SEDANG DIPROSES");
					MAPPUTSTR(rptBuf4,str,lblKO);
					key = rloPembayaranPascaBayarSedangProses;
				}
				else
				{
					singleReceipt = 1;
					strcpy(str,"PEMBAYARAN BERHASIL");
					MAPPUTSTR(rptBuf4,str,lblKO);
					key = rloPembayaranPascaBayarMatrix;
				}
				break;
		}
		break;

#ifdef PREPAID
	case trtPrepaidPayment:
		singleReceipt=0;

		mapGet(traPan,Pan,sizeof(Pan));
		memcpy(BlockedPan, Pan, lenPan); //@agmr
		BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr
		MAPPUTSTR(traBlockPan, BlockedPan,lblKO);

		MAPPUTSTR(rptJudul, "BRIZZI PEMBAYARAN", lblKO);
		strcpy(str,"PEMBAYARAN");

		//amount
		memset(amount,0,sizeof(amount));
		mapGet(traAmt,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"PEMBAYARAN %13s",buf);
		MAPPUTSTR(rptBuf1,str,lblKO);

		//saldo
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"SALDO %18s",buf);
		MAPPUTSTR(rptBuf2,str,lblKO);

		memset(buf,0,sizeof(buf));
		memset(str,0,sizeof(str));
		MAPGET(TRA_HASH,buf,lblKO);
		bin2hex(str,buf,HASH_LEN);
		MAPPUTSTR(rptBuf16,str,lblKO);

		key = rloPrepaidPayment;
		break;

	case trtPrepaidPaymentDisc:
		singleReceipt=0;

		mapGet(traPan,Pan,sizeof(Pan));
		memcpy(BlockedPan, Pan, lenPan); //@agmr
		BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr
		MAPPUTSTR(traBlockPan, BlockedPan,lblKO);

		MAPPUTSTR(rptJudul, "BRIZZI PEMBAYARAN DISC", lblKO);
		strcpy(str,"PEMBAYARAN");

		//amount sebelum Disc
		memset(amount,0,sizeof(amount));
		mapGet(traTotalAmount,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"PEMBAYARAN %37s",buf);
		MAPPUTSTR(rptBuf1,str,lblKO);

		memset(buf1, 0, sizeof(buf1));
		MAPGETBYTE(appBriziDiscFlag,res,lblKO);
		MAPGET(appBriziDiscValue,buf1,lblKO);
		if(res[0] == 2){
			//Disc
			sprintf(str,"DISCOUNT   %35s %%",buf1);
			MAPPUTSTR(rptBuf2,str,lblKO);
		}
		else{
			//Disc
			memset(buf,0,sizeof(buf));
			strcpy(buf,"Rp. ");
			fmtAmt(buf+4, buf1, 0, ",.");
			sprintf(str,"DISCOUNT   %37s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);
		}


		//amount setelah Disc
		memset(amount,0,sizeof(amount));
		mapGet(traAmt,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"TOTAL      %37s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);


		//saldo
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"SALDO %18s",buf);
		MAPPUTSTR(rptBuf4,str,lblKO);

		memset(buf,0,sizeof(buf));
		memset(str,0,sizeof(str));
		MAPGET(TRA_HASH,buf,lblKO);
		bin2hex(str,buf,HASH_LEN);
		MAPPUTSTR(rptBuf16,str,lblKO);

		key = rloPrepaidPaymentDisc;
		break;

	case trtPrepaidDeposit:
		MAPPUTSTR(rptJudul, "ISI ULANG DEPOSIT", lblKO);
		MAPPUTSTR(rptBuf17, "BRIZZI", lblKO);
		MAPPUTSTR(rptBuf1, "No. Kartu BRIZZI :", lblKO);
		memset(Pan,0,sizeof(Pan));
		mapGet(TRA_NOMOR_KARTU_BRIZZI,Pan,sizeof(Pan));
		MAPPUTSTR(rptBuf2, Pan, lblKO);

		//nominal
		memset(amount,0,sizeof(amount));
		mapGet(traAmt,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"NOMINAL %16s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);

		key = rloPrepaidDeposit;
		break;

	case trtPrepaidTopUp:
		MAPPUTSTR(rptJudul, "ISI ULANG BRIZZI", lblKO);

		//No. kartu
		MAPPUTSTR(rptBuf1, "No. Kartu BRIZZI :", lblKO);
		memset(buf,0,sizeof(buf));
		mapGet(TRA_NOMOR_KARTU_BRIZZI,buf,LEN_TRA_NOMOR_KARTU_BRIZZI);
		MAPPUTSTR(rptBuf2, buf, lblKO);

		//nominal
		memset(amount,0,sizeof(amount));
		mapGet(traAmt,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"NOMINAL %16s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);

		//saldo
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"SALDO %18s",buf);
		MAPPUTSTR(rptBuf4,str,lblKO);

		key = rloPrepaidTopUp;
		break;

	case trtPrepaidAktivasi:
		MAPPUTSTR(rptJudul, "AKTIVASI DEPOSIT", lblKO);
		MAPPUTSTR(rptBuf17, "BRIZZI", lblKO);

		//No. kartu
		memset(Pan,0,sizeof(Pan));
		mapGet(traPan,Pan,sizeof(Pan));
		sprintf(buf,"No. Kartu BRIZZI : %s",Pan);
		MAPPUTSTR(rptBuf1, buf, lblKO);

		//saldo awal
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SALDO_AWAL,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Saldo Awal       : %s",buf);
		MAPPUTSTR(rptBuf2,str,lblKO);

		//amount AKTIVASI
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SALDO_DEPOSIT,amount,sizeof(amount));
		amount[LEN_TRA_SALDO_DEPOSIT-2]=0;
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Aktivasi Deposit : %s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);

		//saldo akhir
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Saldo Akhir      : %s",buf);
		MAPPUTSTR(rptBuf4,str,lblKO);

		//sisa deposit
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO_DEPOSIT,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Sisa Deposit     : %s",buf);
		MAPPUTSTR(rptBuf5,str,lblKO);

		//SALDO AKHIR
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"SALDO %18s",buf);
		MAPPUTSTR(rptBuf6,str,lblKO);

		key = rloPrepaidAktivasi;
		break;

	case trtPrepaidRedeem:
		MAPPUTSTR(rptJudul, "Redeem BRIZZI", lblKO);

		//No. kartu
		memset(buf,0,sizeof(buf));
		mapGet(TRA_NOMOR_KARTU_BRIZZI,buf,LEN_TRA_NOMOR_KARTU_BRIZZI);
		sprintf(str,"No Kartu BRIZZI : %s",buf);
		MAPPUTSTR(rptBuf1, str, lblKO);

		//saldo kartu
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		amount[AMOUNT_LEN_S]=0;
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Saldo Kartu     : %s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);

		//saldo Deposit
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SALDO_DEPOSIT,amount,sizeof(amount));
		amount[AMOUNT_LEN_S]=0;
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Saldo Deposit   : %s",buf);
		MAPPUTSTR(rptBuf4,str,lblKO);

		//fee
		memset(amount,0,sizeof(amount));
		mapGet(TRA_FEE,amount,sizeof(amount));
		amount[AMOUNT_LEN_S]=0;
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Fee             : %s",buf);
		MAPPUTSTR(rptBuf5,str,lblKO);

		//Total Redeem
		memset(amount,0,sizeof(amount));
		mapGet(TRA_TOTAL_REDEEM,amount,sizeof(amount));
		amount[AMOUNT_LEN_S]=0;
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Total Redeem    : %s",buf);
		MAPPUTSTR(rptBuf6,str,lblKO);

		MAPPUTSTR(rptBuf7,"Status Kartu    : CLOSE",lblKO);
		sprintf(str,"%18s","Redeem BERHASIL");
		MAPPUTSTR(rptBuf8,str,lblKO);

		key = rloPrepaidRedeem;
		break;
	//++ @agmr - brizzi2
	case trtPrepaidReaktivasi:
		MAPPUTSTR(rptJudul, "Reaktivasi Kartu BRIZZI", lblKO);

		//No. kartu
		memset(Pan,0,sizeof(Pan));
		mapGet(traPan,Pan,sizeof(Pan));
		sprintf(buf,"No. Kartu BRIZZI : %s",Pan);
		MAPPUTSTR(rptBuf1, buf, lblKO);

		//saldo kartu
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Saldo Kartu      : %s",buf);
		MAPPUTSTR(rptBuf2,str,lblKO);

		//saldo deposit
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SALDO_DEPOSIT,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Saldo Deposito   : %s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);

		//biaya admin
		memset(amount,0,sizeof(amount));
		ret = mapGet(TRA_BIAYA_ADMIN,amount,LEN_TRA_BIAYA_ADMIN);
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+4, amount, 0, ",.");
		sprintf(str,"Biaya admin      : %s",buf);
		MAPPUTSTR(rptBuf5,str,lblKO);

		//status kartu
		memset(buf,0,sizeof(buf));
		mapGet(TRA_STATUS_KARTU,buf,sizeof(buf));
		if(memcmp(buf,"aa",2) == 0)
		strcpy(buf,"ACTIVE");
		if(memcmp(buf,"cl",2) == 0)
		strcpy(buf,"CLOSED");
		sprintf(str,"Status Kartu     : %s",buf);
		MAPPUTSTR(rptBuf6,str,lblKO);

		MAPPUTSTR(rptBuf7,"Reaktivasi Kartu BERHASIL",lblKO);

		key = rloPrepaidReaktivasi;
		break;

	case trtPrepaidVoid:
		singleReceipt=0;
		mapGet(traPan,Pan,sizeof(Pan));
		memcpy(BlockedPan, Pan, lenPan); //@agmr
		BlockedPan[sizeof(BlockedPan)-1]=0;	                      //@agmr
		MAPPUTSTR(traBlockPan, BlockedPan,lblKO);

		MAPPUTSTR(rptJudul, "Void Transaksi BRIZZI", lblKO);

		//no Kartu
		sprintf(str,"No Kartu e-Money : %29s",Pan);
		MAPPUTSTR(rptBuf1,str,lblKO);

		//trace no.
		{
			card tmp;

			memset(buf,0,sizeof(buf));

			MAPGETCARD(regInvNum,tmp,lblKO);
			sprintf(buf,"%06lu",tmp);

			sprintf(str,"Trace #          : %29s",buf);
			MAPPUTSTR(rptBuf2,str,lblKO);
		}

		//void amount
		memset(amount,0,sizeof(amount));
		mapGet(traAmt,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"-Rp. ");
		fmtAmt(buf+strlen(buf), amount, 0, ",.");
		sprintf(str,"Jumlah Void      : %29s",buf);
		MAPPUTSTR(rptBuf3,str,lblKO);

		//saldo akhir
		memset(amount,0,sizeof(amount));
		mapGet(TRA_SISA_SALDO,amount,sizeof(amount));
		memset(buf,0,sizeof(buf));
		strcpy(buf,"Rp. ");
		fmtAmt(buf+strlen(buf), amount, 0, ",.");
		sprintf(str,"Saldo Akhir      : %29s",buf);
		MAPPUTSTR(rptBuf4,str,lblKO);

		key = rloPrepaidVoid;
		break;
//-- @agmr - brizzi2

#endif
// -- @agmr - BRI - receipt

	default:
		ret = 0;
		goto lblEnd;
	}

	if(isOneReceipt != NULL)
		*isOneReceipt = singleReceipt;
	if(rlo != NULL)
		*rlo = key;

	ret = 1;
	goto lblEnd;

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		trcFN("briPrintData ret=%d\n", ret);

	return ret;
}


void briLogDuplicata(byte jenisLog)
{
	int 	ret, logCount;
	char rocRef[lenInvNum + 1];
	char CrdType[lenCardTransType + 1];
	char traName[prtW + 1];
	char entMod;
	word txnType, ptrTrx, trxNum, key, idx = 0;
	card logRoc;
	byte voidTxn;

	//++ @agmr - BRI
	byte singleReceipt=0;
	word transType, menuItm;
	transType = key;
	word regBriLogSize,regBriLogIdx,briLogBeg,briLogROC,briLogEntMod;
	word briLogCardType,briLogMnuItm,briLogVoidFlag,briLogDatTim;

	trcS("briLogDuplicata Beg\n");
	//-- @agmr - BRI

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize   	= regCLogSize;
			regBriLogIdx    	= regCLogIdx;
			briLogBeg       	= cLogBeg;
			briLogROC       	= cLogROC;
			briLogEntMod    	= cLogEntMod;
			briLogCardType  	= cLogCardType;
			briLogMnuItm    	= cLogMnuItm;
			briLogVoidFlag  	= cLogVoidFlag;
			briLogDatTim    	= cLogDatTim;
			briLogVoidFlag  	= cLogVoidFlag;
			break;

		case MINI_ATM_LOG:
			regBriLogSize   	= regMLogSize;
			regBriLogIdx    	= regMLogIdx;
			briLogBeg       	= mLogBeg;
			briLogROC       	= mLogROC;
			briLogEntMod    	= mLogEntMod;
			briLogCardType  	= mLogCardType;
			briLogMnuItm    	= mLogMnuItm;
			briLogVoidFlag  	= mLogVoidFlag;
			briLogDatTim    	= mLogDatTim;
			briLogVoidFlag  	= mLogVoidFlag;
			break;

		case TUNAI_LOG:
			regBriLogSize   	= regTLogSize;
			regBriLogIdx    	= regTLogIdx;
			briLogBeg       	= tLogBeg;
			briLogROC       	= tLogROC;
			briLogEntMod    	= tLogEntMod;
			briLogCardType  	= tLogCardType;
			briLogMnuItm    	= tLogMnuItm;
			briLogVoidFlag  	= tLogVoidFlag;
			briLogDatTim    	= tLogDatTim;
			briLogVoidFlag  	= tLogVoidFlag;
			break;

		case TCASH_LOG:
			regBriLogSize   	= regTCashLogSize;
			regBriLogIdx    	= regTCashLogIdx;
			briLogBeg       	= tCashLogBeg;
			briLogROC       	= tCashLogROC;
			briLogEntMod    	= tCashLogEntMod;
			briLogCardType  	= tCashLogCardType;
			briLogMnuItm    	= tCashLogMnuItm;
			briLogVoidFlag  	= tCashLogVoidFlag;
			briLogDatTim    	= tCashLogDatTim;
			briLogVoidFlag  	= tCashLogVoidFlag;
			break;

		default:
			goto lblKO;
	}

	memset(rocRef, 0x00, sizeof(rocRef));

	MAPGETWORD(regBriLogSize, trxNum, lblKO);
	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	MAPGETWORD(regBriLogIdx, ptrTrx, lblKO);

	if(ptrTrx > 0)
	{
		trxNum = ptrTrx;
		mapMove(briLogBeg, trxNum - 1);
	}
	else
	{
		mapMove(briLogBeg, trxNum - 1);
	}

	MAPGETCARD(briLogROC, logRoc, lblKO);
	num2dec(rocRef, logRoc, 0);

	MAPPUTSTR(traFindRoc, rocRef, lblKO);

	ret = briLogLoadByRoc(jenisLog);
	CHECK(ret >= 0, lblKO);

	if(ret == 0)
		goto lblEnd;

	ret = mapGetByte(briLogEntMod, entMod);
	CHECK(ret >= 0, lblKO);

	MAPGET(briLogCardType, CrdType, lblKO);
	MAPPUTSTR(traCardType, CrdType, lblKO);

	MAPGETWORD(briLogMnuItm, menuItm, lblKO);

	key = 0;
	getBriTransTypeName(&txnType, NULL, menuItm);
	ret = briPrintData(txnType, &key, &singleReceipt,1);
	if(ret <= 0)
		goto lblKO;
	idx = txnType;

	VERIFY(key);

	MAPGETBYTE(briLogVoidFlag, voidTxn, lblKO);

	ret = usrInfo(infDuplicatePrinting);
	CHECK(ret >= 0, lblKO);

	mapMove(rqsBeg, idx - 1);

	MAPGET(rqsRpt, traName, lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);

	ret = strFormatDatTim(briLogDatTim);
	CHECK(ret >= 0, lblKO);

	trcS("Printing Receipt\n");
	MAPPUTSTR(rptDuplicate,"*DUPLICATE*",lblKO);
	rptReceipt(rloDuplicateLine2);
	ret = rptReceipt(key);

	rptReceipt(rloFeed);
	ret = briLogLoadCurContext(jenisLog);
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
		trcS("briLogDuplicata End\n");
}

void briLogPrintTxn(byte jenisLog)
{
	int 	ret, logCount;
	char rocRef[lenInvNum + 1];
	char CrdType[lenCardTransType + 1];
	char traName[prtW + 1];
	char entMod;
	word txnType, trxNum, key, idx = 0;
	byte voidTxn;

	//++ @agmr - BRI
	byte singleReceipt=0;
	word transType;
	transType = key;
	word regBriLogSize,regBriLogIdx,briLogBeg,briLogROC,briLogEntMod;
	word briLogCardType,briLogMnuItm,briLogVoidFlag,briLogDatTim;
	word menuItm;
	//-- @agmr - BRI


	trcS("logPrintTxn Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize   	= regCLogSize;
			regBriLogIdx    	= regCLogIdx;
			briLogBeg       	= cLogBeg;
			briLogROC       	= cLogROC;
			briLogEntMod    	= cLogEntMod;
			briLogCardType  	= cLogCardType;
			briLogMnuItm    	= cLogMnuItm;
			briLogVoidFlag  	= cLogVoidFlag;
			briLogDatTim    	= cLogDatTim;
			briLogVoidFlag  	= cLogVoidFlag;
			break;

		case MINI_ATM_LOG:
			regBriLogSize   	= regMLogSize;
			regBriLogIdx    	= regMLogIdx;
			briLogBeg       	= mLogBeg;
			briLogROC       	= mLogROC;
			briLogEntMod    	= mLogEntMod;
			briLogCardType  	= mLogCardType;
			briLogMnuItm    	= mLogMnuItm;
			briLogVoidFlag  	= mLogVoidFlag;
			briLogDatTim    	= mLogDatTim;
			briLogVoidFlag  	= mLogVoidFlag;
			break;

		case TCASH_LOG:
			regBriLogSize   	= regTCashLogSize;
			regBriLogIdx    	= regTCashLogIdx;
			briLogBeg       	= tCashLogBeg;
			briLogROC       	= tCashLogROC;
			briLogEntMod    	= tCashLogEntMod;
			briLogCardType  	= tCashLogCardType;
			briLogMnuItm   	= tCashLogMnuItm;
			briLogVoidFlag  	= tCashLogVoidFlag;
			briLogDatTim    	= tCashLogDatTim;
			briLogVoidFlag  	= tCashLogVoidFlag;
			break;

		case TUNAI_LOG:
			regBriLogSize   	= regTLogSize;
			regBriLogIdx    	= regTLogIdx;
			briLogBeg       	= tLogBeg;
			briLogROC       	= tLogROC;
			briLogEntMod    	= tLogEntMod;
			briLogCardType  	= tLogCardType;
			briLogMnuItm   	= tLogMnuItm;
			briLogVoidFlag  	= tLogVoidFlag;
			briLogDatTim    	= tLogDatTim;
			briLogVoidFlag  	= tLogVoidFlag;
			break;

		default:
			goto lblKO;
	}


	memset(rocRef, 0x00, sizeof(rocRef));

	MAPGETWORD(regBriLogSize, trxNum, lblKO);
	logCount = trxNum;
	CHECK(logCount > 0, lblNoTran);

	ret = scrDlg(infEnterTrace, traFindRoc);
	CHECK(ret >= 0, lblKO);

	ret = briLogLoadByRoc(jenisLog);
	CHECK(ret >= 0, lblKO);

	if(ret == 0)
		goto lblEnd;

	ret = mapGetByte(briLogEntMod, entMod);
	CHECK(ret >= 0, lblKO);

	MAPGET(briLogCardType, CrdType, lblKO);
	MAPPUTSTR(traCardType, CrdType, lblKO);

	MAPGETWORD(briLogMnuItm, menuItm, lblKO);

	key = 0;
	getBriTransTypeName(&txnType, NULL, menuItm);
	ret = briPrintData(txnType, &key, &singleReceipt,1);
	if(ret <= 0)
		goto lblKO;
	idx = txnType;

	VERIFY(key);

	MAPGETBYTE(briLogVoidFlag, voidTxn, lblKO);

	ret = usrInfo(infDuplicatePrinting);
	CHECK(ret >= 0, lblKO);

	if(voidTxn)
		key = rloVoidDup;

	mapMove(rqsBeg, idx - 1);

	MAPGET(rqsRpt, traName, lblKO);
	MAPPUTSTR(traTransType, traName, lblKO);

	ret = strFormatDatTim(briLogDatTim);
	CHECK(ret >= 0, lblKO);

	trcS("Printing Receipt\n");
	MAPPUTSTR(rptDuplicate,"*DUPLICATE*",lblKO);
	rptReceipt(rloDuplicateLine2);
	rptReceipt(rloFeed);
	ret = briLogLoadCurContext(jenisLog);
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


int briLogSaveCurContext(byte jenisLog)
{
	int 		ret;
	byte 	idx;
	word 	key;
	byte		buf[256];
	word 	briLogBeg,briLogEnd;

	trcS("logSaveCurContext Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			briLogBeg       = cLogBeg;
			briLogEnd       = cLogEnd;
			break;

		case MINI_ATM_LOG:
			briLogBeg       = mLogBeg;
			briLogEnd       = mLogEnd;
			break;

		case TCASH_LOG:
			briLogBeg       = tCashLogBeg;
			briLogEnd       = tCashLogEnd;
			break;

		case TUNAI_LOG:
			briLogBeg       = tLogBeg;
			briLogEnd       = tLogEnd;
			break;

		default:
			goto lblKO;
	}

	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);
		switch(jenisLog)
		{
			case CARD_SERVICE_LOG:
				key = cLogKey[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(cLogKeyContext[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;

			case TCASH_LOG:
				key = tCashLogKey[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(tCashLogKeyContext[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;

			case MINI_ATM_LOG:
				key = mLogKey[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(mLogKeyContext[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;

			case TUNAI_LOG:
				key = tLogKey[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(tLogKeyContext[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;
		}

	}
	ret = 1;
	goto lblEnd;

	lblKO:
	lblDBA:
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("logSaveCurContext: ret=%d #\n", ret);

	return ret;

}

int briLogLoadCurContext(byte jenisLog)
{
	int 		ret;
	word 	key;
	byte 	idx;
	byte 	buf[256];
	card 	dupStan, dupROC;
	word 	regBriLogSize,regBriLogIdx,briLogBeg,briLogEnd;

	trcS("logLoadCurContext Beg\n");

	switch(jenisLog)
	{
		case CARD_SERVICE_LOG:
			regBriLogSize   = regCLogSize;
			regBriLogIdx    = regCLogIdx;
			briLogBeg       = cLogBeg;
			briLogEnd       = cLogEnd;
			break;

		case MINI_ATM_LOG:
			regBriLogSize   = regMLogSize;
			regBriLogIdx    = regMLogIdx;
			briLogBeg       = mLogBeg;
			briLogEnd       = mLogEnd;
			break;

		case TCASH_LOG:
			regBriLogSize   = regTCashLogSize;
			regBriLogIdx    = regTCashLogIdx;
			briLogBeg       = tCashLogBeg;
			briLogEnd       = tCashLogEnd;
			break;

		case TUNAI_LOG:
			regBriLogSize   = regTLogSize;
			regBriLogIdx    = regTLogIdx;
			briLogBeg       = tLogBeg;
			briLogEnd       = tLogEnd;
			break;

		default:
			goto lblKO;
	}

	for (key = briLogBeg + 1; key < briLogEnd; key++)
	{
		idx = (byte) (key - briLogBeg - 1);

		switch(jenisLog)
		{
			case CARD_SERVICE_LOG:
				key = cLogKeyContext[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(cLogKey[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;

			case MINI_ATM_LOG:
				key = mLogKeyContext[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(mLogKey[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;

			case TCASH_LOG:
				key = tCashLogKeyContext[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(tCashLogKey[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;

			case TUNAI_LOG:
				key = tLogKeyContext[idx];
				MAPGET(key, buf, lblDBA);
				ret = mapPut(tLogKey[idx], buf, mapDatLen(key));
				CHECK(ret >= 0, lblDBA);
				break;
		}

	}

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

	lblKO:
	lblDBA:
		ret = -1;
		goto lblEnd;
	lblEnd:
		trcFN("logLoadCurContext: ret=%d #\n", ret);

	return ret;
}

void briPrintDetail(byte jenisLog)
{
	int 		ret;
	char 	buf[20], buf1[15];
	card 	dd, mm, yy;

	dspClear();
	dspLS(2,"DDMMYYYY");

	while(1)
	{
		memset(buf,0,sizeof(buf));
		ret = enterPhr(3, buf, 9);

		if(ret !=kbdVAL)
			return;
		if(strlen(buf)<8)
			continue;

		dec2num(&dd,buf,2);
		dec2num(&mm,buf+2,2);
		dec2num(&yy,buf+4,4);

		if(mm>12 || mm<=0)
			continue;

		if(dd>31 || dd<=0)
			continue;

		break;
	}

	memset(buf1,0,sizeof(buf1));
	memcpy(buf1,buf+4,4);
	memcpy(buf1+4,buf+2,2);
	memcpy(buf1+6,buf,2);

	briLogPrintDetail(jenisLog,buf1);
}

void briPrintDetailToday(byte jenisLog)
{
	briLogPrintDetail(jenisLog,"");
}

void briPrintTotal(byte jenisLog)
{
	int 		ret;
	char 	buf[20], buf1[15];
	card 	dd, mm, yy;

	dspClear();
	dspLS(2,"DDMMYYYY");

	while(1)
	{
		memset(buf,0,sizeof(buf));
		ret = enterPhr(3, buf, 9);

		if(ret !=kbdVAL)
			return;
		if(strlen(buf)<8)
			continue;

		dec2num(&dd,buf,2);
		dec2num(&mm,buf+2,2);
		dec2num(&yy,buf+4,4);

		if(mm>12 || mm<=0)
			continue;

		if(dd>31 || dd<=0)
			continue;

		break;
	}

	memset(buf1,0,sizeof(buf1));
	memcpy(buf1,buf+4,4);
	memcpy(buf1+4,buf+2,2);
	memcpy(buf1+6,buf,2);

	briLogPrintTotal(jenisLog,buf1);
}

void briPrintTotalToday(byte jenisLog)
{
    briLogPrintTotal(jenisLog,"");
}

//-- @agmr - BRI2

//++ @agmr - BRI3
int briGetAcq(byte cardType, byte* acquirerId)
{
	word 	menuItem;
	int 		ret;
	char 	name[20],expectedName[20];
	byte 	idx;

	memset(name,0,sizeof(name));
	if(acquirerId == NULL)
		goto lblKO;

	MAPGETWORD(traMnuItm, menuItem, lblKO);
	//@@SIMAS - start
    	idx=0;
    	if (menuItem==mnuDebitLogon) {
        	ret = mapMove(acqBeg, idx);
        	CHECK(ret >= 0, lblKO);
        	mapGetByte(acqID, *acquirerId);
        	return 1;
    	}
    	//@@SIMAS - end
	switch(menuItem)
	{
		//pakai acq asli
		case mnuSale:
		case mnuSaleRedeem:  //@ar -BRI
		case mnuInstallment:
		case mnuPreaut: //BRI5
		case mnuAut: //BRI5
		case mnuPreAuth:
		case mnuPreCom:
		case mnuPreCancel:
		case mnuRefund:
			if(cardType == DEBIT_CARD)
				mapGetByte(binDbAcqID, *acquirerId);
			else
				mapGetByte(binCrAcqID, *acquirerId);
			return 1;

		case mnuInfoSaldo:
		case mnuInfoSaldoBankLain:
		case mnuMiniStatement:
		case mnuMutRek:
		case mnuPrevilege:
		case mnuTransferSesamaBRI:
		case mnuTransferAntarBank:
		case mnuPembayaranTvBerlangganan:
		case mnuPembayaranTvInd:
		case mnuPembayaranTvOkTv:
		case mnuPembayaranTvTopTv:
		case mnuPembayaranTiketPesawat:
		case mnuPembayaranTiketGaruda:
		case mnuPembayaranTiketLionAir:
		case mnuPembayaranTiketSriwijaya:
		case mnuPembayaranTiketMandala:
		case mnuPembayaranPdam:
		case mnuPembayaranPdamSby:
		case mnuRegistrasiSmsBanking:
		case mnuRegistrasiTrxTransfer:
		case mnuRegistrasiTrxPulsa:
		case mnuPembayaranSPP:
		case mnuSetorPasti:
		case mnuPembayaranEsia25:
		case mnuPembayaranEsia50:
		case mnuPembayaranEsia75:
		case mnuPembayaranEsia100:
		case mnuPembayaranEsia150:
		case mnuPembayaranEsia200:
		case mnuPembayaranSmart20:
		case mnuPembayaranSmart25:
		case mnuPembayaranSmart50:
		case mnuPembayaranSmart100:
		case mnuPembayaranSmart150:
		case mnuPembayaranSmart200:
		case mnuPembayaranSmart300:
		case mnuPembayaranSmart500:
		case mnuPembayaranFren20:
		case mnuPembayaranFren25:
		case mnuPembayaranFren50:
		case mnuPembayaranFren100:
		case mnuPembayaranFren150:
		case mnuPembayaranFren200:
		case mnuPembayaranFren300:
		case mnuPembayaranFren500:
		case mnuPembayaranThree20:
		case mnuPembayaranThree50:
		case mnuPembayaranThree75:
		case mnuPembayaranThree100:
		case mnuPembayaranThree150:
		case mnuPembayaranThree300:
		case mnuPembayaranThree500:
		case mnuPembayaranAxis25:
		case mnuPembayaranAxis50:
		case mnuPembayaranAxis75:
		case mnuPembayaranAxis100:
		case mnuPembayaranAxis150:
		case mnuPembayaranAxis200:
		case mnuPembayaranAxis300:
		case mnuPembayaranKKBRI:
		case mnuPembayaranKKCITIKK:
		case mnuPembayaranKKCITIKTA:
		case mnuPembayaranKKStanCharKK:
		case mnuPembayaranKKStanCharKTA:
		case mnuPembayaranKKHSBCKK:
		case mnuPembayaranKKHSBCKTA:
		case mnuPembayaranKKRBSKK:
		case mnuPembayaranKKRBSKTA:
		case mnuPembayaranKKANZ:
		case mnuPembayaranPLNPasca:
		case mnuPembayaranPLNPra:
		case mnuPembayaranPLNToken:
		case mnuPembayaranCicilanFIF:
		case mnuPembayaranCicilanBAF:
		case mnuPembayaranCicilanOTO:
		case mnuPembayaranCicilanFinansia:
		case mnuPembayaranCicilanVerena:
		case mnuPembayaranCicilanWOM:
		case mnuPembayaranSimpati50:
		case mnuPembayaranSimpati100:
//		case mnuPembayaranSimpati150:
		case mnuPembayaranSimpati200:
		case mnuPembayaranSimpati300:
		case mnuPembayaranSimpati500:
		case mnuPembayaranSimpati1000:
		case mnuPembayaranMentari25:
		case mnuPembayaranMentari50:
		case mnuPembayaranMentari100:
		case mnuPembayaranMentari250:
		case mnuPembayaranMentari500:
		case mnuPembayaranMentari1000:
		case mnuPembayaranIm325:
		case mnuPembayaranIm350:
		case mnuPembayaranIm375:
		case mnuPembayaranIm3100:
		case mnuPembayaranIm3150:
		case mnuPembayaranIm3200:
		case mnuPembayaranIm3500:
		case mnuPembayaranIm31000:
		case mnuPembayaranXl25:
		case mnuPembayaranXl50:
		case mnuPembayaranXl75:
		case mnuPembayaranXl100:
		case mnuPembayaranXl150:
		case mnuPembayaranXl200:
		case mnuPembayaranXl300:
		case mnuPembayaranXl500:
		case mnuPembayaranTelkom:
		case mnuPembayaranDPLKR:
		case mnuPembayaranBriva:
		case mnuPembayaranZakat:
		case mnuPembayaranZakatDhuafa:
		case mnuPembayaranZakatYBM:
		case mnuPembayaranInfaqDhuafa:
		case mnuPembayaranInfaqYBM:
		case mnuPembayaranDPLKDhuafa:
		case mnuPembayaranHalo:
		case mnuPembayaranMatrix:
		case mnuRegistrasiInternetBanking:
		case mnuRegistrasiPhoneBanking:
#ifdef T_BANK
		case mnuCashIn:
		case mnuCashOut:
#endif

#ifdef SSB
		case mnuSIMBaruA :
		case mnuSIMBaruAU :
		case mnuSIMBaruB1 :
		case mnuSIMBaruB1U :
		case mnuSIMBaruB2 :
		case mnuSIMBaruB2U :
		case mnuSIMBaruC :
		case mnuSIMBaruD :
		case mnuSIMPanjangA :
		case mnuSIMPanjangAU :
		case mnuSIMPanjangB1 :
		case mnuSIMPanjangB1U :
		case mnuSIMPanjangB2 :
		case mnuSIMPanjangB2U :
		case mnuSIMPanjangC :
		case mnuSIMPanjangD :
		case mnuSIMPeningkatanAU :
		case mnuSIMPeningkatanB1 :
		case mnuSIMPeningkatanB1U :
		case mnuSIMPeningkatanB2 :
		case mnuSIMPeningkatanB2U :
		case mnuSIMPenurunanA :
		case mnuSIMPenurunanAU :
		case mnuSIMPenurunanB1 :
		case mnuSIMPenurunanB1U :
		case mnuSIMPenurunanB2 :
		case mnuSIMPenurunanB2U :
		case mnuSIMGolSamA :
		case mnuSIMGolSamAU :
		case mnuSIMGolSamB1 :
		case mnuSIMGolSamB1U :
		case mnuSIMGolSamB2 :
		case mnuSIMGolSamB2U :
		case mnuSIMGolSamC :
		case mnuSIMGolSamD :
		case mnuSIMMutasiPeningkatanAU :
		case mnuSIMMutasiPeningkatanB1 :
		case mnuSIMMutasiPeningkatanB1U :
		case mnuSIMMutasiPeningkatanB2 :
		case mnuSIMMutasiPeningkatanB2U :
		case mnuSIMMutasiPenurunanA :
		case mnuSIMMutasiPenurunanAU :
		case mnuSIMMutasiPenurunanB1 :
		case mnuSIMMutasiPenurunanB1U :
		case mnuSIMMutasiPenurunanB2 :
		case mnuSIMMutasiPenurunanB2U :
		case mnuSIMHilangA :
		case mnuSIMHilangAU :
		case mnuSIMHilangB1 :
		case mnuSIMHilangB1U :
		case mnuSIMHilangB2 :
		case mnuSIMHilangB2U :
		case mnuSIMHilangC :
		case mnuSIMHilangD :
		case mnuSIMHilangPeningkatanAU :
		case mnuSIMHilangPeningkatanB1 :
		case mnuSIMHilangPeningkatanB1U :
		case mnuSIMHilangPeningkatanB2 :
		case mnuSIMHilangPeningkatanB2U :
		case mnuSIMHilangPenurunanA :
		case mnuSIMHilangPenurunanAU :
		case mnuSIMHilangPenurunanB1 :
		case mnuSIMHilangPenurunanB1U :
		case mnuSIMHilangPenurunanB2 :
		case mnuUjiSim :
		case mnuBPKBBaruRoda2 :
		case mnuBPKBBaruRoda4 :
		case mnuBPKBGantiRoda2 :
		case mnuBPKBGantiRoda4 :
		case mnuSTCK :
		case mnuMutasiRanRoda2 :
		case mnuMutasiRanRoda4 :
#endif
		//card service

		case mnuAktivasiKartu:
		case mnuRenewalKartu:
		case mnuReissuePIN:
		case mnuPIN:
		case mnuGantiPIN:
		case mnuGantiPasswordSPV:
			if(cardType == CREDIT_CARD)
				strcpy(expectedName,"MINI ATM C");
			else //cardType = DEBIT_CARD, NON_BRI_DEBIT_CARD DAN NON_BRI_CREDIT_CARD
				strcpy(expectedName,"MINI ATM D");
			break;

		//tunai
		case mnuSetorSimpanan:
		case mnuSetorPinjaman:
		case mnuTarikTunai:
		case mnuVoidTarikTunai:
			if(cardType == CREDIT_CARD)
				strcpy(expectedName,"TUNAI C");
			else if(cardType == DEBIT_CARD)
				strcpy(expectedName,"TUNAI D");
			else //NON_BRI_CREDIT_CARD dan NON_BRI_DEBIT_CARD
				strcpy(expectedName,"MINI ATM D");
			break;

#ifdef PREPAID
		case mnuTopUpDeposit:
		case mnuTopUpOnline:
		case mnuAktivasiDeposit:
		case mnuInfoDeposit:
		case mnuPrepaidSettlement:
		case mnuPrepaidPayment:
		case mnuPrepaidRedeem:
		case mnuPrepaidPaymentDisc:
		//++@agmr - brizzi2
		case mnuPrepaidReaktivasi:
		case mnuPrepaidVoid:
		//--@agmr - brizzi2
			strcpy(expectedName,"BRIZZI");
			break;
#endif

#ifdef ABSENSI
		//Absensi
		case mnuAbsen:
		case mnuDatang:
		case mnuPulangs:
		case mnuIstirahatA:
		case mnuIstirahatB:
		case mnuLemburA:
		case mnuLemburB:
		case mnuShifts:
		case mnuShiftsI:
		case mnuShiftsIA:
		case mnuShiftsIB:
		case mnuShiftsII:
		case mnuShiftsIIA:
		case mnuShiftsIIB:
		case mnuShiftsIII:
		case mnuShiftsIIIA:
		case mnuShiftsIIIB:
#endif
		case mnuDebitLogon: //@@AS0.0.26
			strcpy(expectedName,"MINI ATM D");
			break;

		default:
			return 1;
	}

	for (idx = 0; idx < dimAcq; idx++)
	{
		ret = mapMove(acqBeg, idx);
		CHECK(ret >= 0, lblKO);
		MAPGET(acqName, name, lblKO);

		if(memcmp(expectedName, name, strlen(expectedName)) == 0)
		{
			mapGetByte(acqID, *acquirerId);
			return 1;
		}
	}
	*acquirerId = 0;

	return 0;

	lblKO:
		return -1;
}
//-- @agmr - BRI3

void cLogReset(void)
{
	int 	ret;

	ret = usrInfo(infConfirmClearClog);
	if(ret != kbdVAL)
		return;

	ret = mapReset(cLogBeg);
	CHECK(ret >= 0, lblKO)
	MAPPUTWORD(regCLogIdx, 0, lblKO);
	MAPPUTWORD(regCLogSize, 0, lblKO);
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblEnd:
	;
}

void mLogReset(void)
{
	int ret;

	ret = usrInfo(infConfirmClearMlog);
	if(ret != kbdVAL)
		return;

	ret = mapReset(mLogBeg);
	CHECK(ret >= 0, lblKO)
	MAPPUTWORD(regMLogIdx, 0, lblKO);
	MAPPUTWORD(regMLogSize, 0, lblKO);
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblEnd:
	;
}

void tLogReset(void)
{
	int ret;

	ret = usrInfo(infConfirmClearTlog);
	if(ret != kbdVAL)
		return;

	ret = mapReset(tLogBeg);
	CHECK(ret >= 0, lblKO)
	MAPPUTWORD(regTLogIdx, 0, lblKO);
	MAPPUTWORD(regTLogSize, 0, lblKO);
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblEnd:
	;
}

void tCashLogReset()
{
	int ret;

	ret = usrInfo(infConfirmClearTlog);
	if(ret != kbdVAL)
		return;

	ret = mapReset(tCashLogBeg);
	CHECK(ret >= 0, lblKO)
	MAPPUTWORD(regTCashLogIdx, 0, lblKO);
	MAPPUTWORD(regTCashLogSize, 0, lblKO);
	goto lblEnd;

	lblKO:
		usrInfo(infProcessingError);
		ret = 0;
		goto lblEnd;

	lblEnd:
	;
}


#ifndef REVERSAL_TERPISAH
int setBriReversalIdx()
{
	int ret;

	MAPPUTBYTE(appBriRevIndex,0,lblKO);
	return 1;

	//	 ***dibuka dan dirubah bila reversal tergantung acquirer
	//   ***dimana Mini atm dan Tunai satu kelompok, dan sale serta INSTALLMENT
	//   ***dalam kelompok lain
	//	for (i = 0; i < dimAcq; i++)
	//	{
	//		ret = mapMove(acqBeg, i);
	//		CHECK(ret >= 0, lblKO);
	//		MAPGET(acqName, name, lblKO);
	//
	//		if(memcmp("MINI ATM", name, strlen("MINI ATM")) == 0 ||
	//		   memcmp("TUNAI", name, strlen("TUNAI")) == 0
	//		  )
	//		{
	//		    ret = mapGetByte(acqID, idx);
	//		    if(ret < 0)
	//		        goto lblKO;
	//		    if(idx > 0)
	//		    {
	//		        ret = mapPutByte(appBriRevIndex,idx-1);
	//		        if(ret < 0)
	//		            goto lblKO;
	//		    }
	//		    else
	//		        goto lblKO;
	//		    ret = 1;
	//		    goto lblEnd;
	//		}
	//	}

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		return ret;
}

int getBriReversalIdx()
{
	int 	ret;
	byte idx;

	//semua transaksi reversal masuk ke stau record saja
	//tidak tergantung acquirer
	MAPGETBYTE(appBriRevIndex,idx,lblKO);
	mapMove(revBeg,idx);

	//	 ***dibuka dan dirubah bila reversal tergantung acquirer
	//   ***dimana Mini atm dan Tunai satu kelompok, dan sale serta INSTALLMENT
	//   ***dalam kelompok lain
	//    tmp = getJenisLog();
	//    if(tmp>0)
	//    {
	//        MAPGETBYTE(appBriRevIndex,idx, lblKO);
	//    }
	//    else
	//    {
	//        MAPGETBYTE(traAcqIdx,idx,lblKO);
	//    }


	//    mapMove(revBeg, idx);
	return 1;

	lblKO:
		return -1;
}
#endif

int getJenisLog()
{
	word menuItem;
	int 	ret;

	MAPGETWORD(traMnuItm, menuItem, lblKO);
	switch(menuItem)
	{
		//Mini ATM
		case mnuInfoSaldo:
		case mnuInfoSaldoBankLain:
		case mnuMiniStatement:
		case mnuMutRek:
		case mnuPrevilege:
		case mnuTransferSesamaBRI:
		case mnuTransferAntarBank:
		case mnuPembayaranPLNPasca:
		case mnuPembayaranPLNPra:
		case mnuPembayaranPLNToken:
		case mnuPembayaranCicilanFIF:
		case mnuPembayaranCicilanBAF:
		case mnuPembayaranCicilanOTO:
		case mnuPembayaranCicilanFinansia:
		case mnuPembayaranCicilanVerena:
		case mnuPembayaranCicilanWOM:
		case mnuPembayaranSimpati50:
		case mnuPembayaranSimpati100:
//		case mnuPembayaranSimpati150:
		case mnuPembayaranSimpati200:
		case mnuPembayaranSimpati300:
		case mnuPembayaranSimpati500:
		case mnuPembayaranSimpati1000:
		case mnuPembayaranMentari25:
		case mnuPembayaranMentari50:
		case mnuPembayaranMentari100:
		case mnuPembayaranMentari250:
		case mnuPembayaranMentari500:
		case mnuPembayaranMentari1000:
		case mnuPembayaranIm325:
		case mnuPembayaranIm350:
		case mnuPembayaranIm375:
		case mnuPembayaranIm3100:
		case mnuPembayaranIm3150:
		case mnuPembayaranIm3200:
		case mnuPembayaranIm3500:
		case mnuPembayaranIm31000:
		case mnuPembayaranXl25:
		case mnuPembayaranXl50:
		case mnuPembayaranXl75:
		case mnuPembayaranXl100:
		case mnuPembayaranXl150:
		case mnuPembayaranXl200:
		case mnuPembayaranXl300:
		case mnuPembayaranXl500:
		case mnuPembayaranZakat:
		case mnuPembayaranZakatDhuafa:
		case mnuPembayaranZakatYBM:
		case mnuPembayaranInfaqDhuafa:
		case mnuPembayaranInfaqYBM:
		case mnuPembayaranDPLKDhuafa:
		case mnuPembayaranHalo:
		case mnuPembayaranMatrix:
		case mnuRegistrasiInternetBanking:
		case mnuRegistrasiPhoneBanking:
		case mnuRegistrasiSmsBanking:
		case mnuPembayaranTvBerlangganan:
		case mnuPembayaranTvInd:
		case mnuPembayaranTvOkTv:
		case mnuPembayaranTvTopTv:
		case mnuPembayaranTiketPesawat:
		case mnuPembayaranTiketGaruda:
		case mnuPembayaranTiketLionAir:
		case mnuPembayaranTiketSriwijaya:
		case mnuPembayaranTiketMandala:
		case mnuPembayaranPdam:
		case mnuPembayaranPdamSby:
		case mnuRegistrasiTrxPulsa:
		case mnuRegistrasiTrxTransfer:
		case mnuPembayaranSPP:
		case mnuPembayaranEsia25:
		case mnuPembayaranEsia50:
		case mnuPembayaranEsia75:
		case mnuPembayaranEsia100:
		case mnuPembayaranEsia150:
		case mnuPembayaranEsia200:
		case mnuPembayaranSmart20:
		case mnuPembayaranSmart25:
		case mnuPembayaranSmart50:
		case mnuPembayaranSmart100:
		case mnuPembayaranSmart150:
		case mnuPembayaranSmart200:
		case mnuPembayaranSmart300:
		case mnuPembayaranSmart500:
		case mnuPembayaranFren20:
		case mnuPembayaranFren25:
		case mnuPembayaranFren50:
		case mnuPembayaranFren100:
		case mnuPembayaranFren150:
		case mnuPembayaranFren200:
		case mnuPembayaranFren300:
		case mnuPembayaranFren500:
		case mnuPembayaranThree20:
		case mnuPembayaranThree50:
		case mnuPembayaranThree75:
		case mnuPembayaranThree100:
		case mnuPembayaranThree150:
		case mnuPembayaranThree300:
		case mnuPembayaranThree500:
		case mnuPembayaranAxis25:
		case mnuPembayaranAxis50:
		case mnuPembayaranAxis75:
		case mnuPembayaranAxis100:
		case mnuPembayaranAxis150:
		case mnuPembayaranAxis200:
		case mnuPembayaranAxis300:
		case mnuPembayaranKKBRI:
		case mnuPembayaranKKCITIKK:
		case mnuPembayaranKKCITIKTA:
		case mnuPembayaranKKStanCharKK:
		case mnuPembayaranKKStanCharKTA:
		case mnuPembayaranKKHSBCKK:
		case mnuPembayaranKKHSBCKTA:
		case mnuPembayaranKKRBSKK:
		case mnuPembayaranKKRBSKTA:
		case mnuPembayaranKKANZ:
		case mnuPembayaranTelkom:
		case mnuPembayaranDPLKR:
		case mnuPembayaranBriva:
			return MINI_ATM_LOG;

		//card service
		case mnuAktivasiKartu:
		case mnuRenewalKartu:
		case mnuReissuePIN:
		case mnuPIN:
		case mnuGantiPIN:
		case mnuGantiPasswordSPV:
			return CARD_SERVICE_LOG;

		//tunai
		case mnuSetorPasti:
		case mnuSetorSimpanan:
		case mnuSetorPinjaman:
		case mnuTarikTunai:
		case mnuVoidTarikTunai:
			return CARD_SERVICE_LOG;

#ifdef T_BANK
		//tcash
		case mnuCashIn:
		case mnuCashOut:
			return TCASH_LOG;
#endif
#ifdef ABSENSI
		//Absensi
		case mnuAbsen:
		case mnuDatang:
		case mnuPulangs:
		case mnuIstirahatA:
		case mnuIstirahatB:
		case mnuLemburA:
		case mnuLemburB:
		case mnuShifts:
		case mnuShiftsI:
		case mnuShiftsIA:
		case mnuShiftsIB:
		case mnuShiftsII:
		case mnuShiftsIIA:
		case mnuShiftsIII:
		case mnuShiftsIIIA:
		case mnuShiftsIIIB:
#endif

#ifdef SSB
		case mnuSIMBaruAU :
		case mnuSIMBaruB1 :
		case mnuSIMBaruB1U :
		case mnuSIMBaruB2 :
		case mnuSIMBaruB2U :
		case mnuSIMBaruC :
		case mnuSIMBaruD :
		case mnuSIMPanjangA :
		case mnuSIMPanjangAU :
		case mnuSIMPanjangB1 :
		case mnuSIMPanjangB1U :
		case mnuSIMPanjangB2 :
		case mnuSIMPanjangB2U :
		case mnuSIMPanjangC :
		case mnuSIMPanjangD :
		case mnuSIMPeningkatanAU :
		case mnuSIMPeningkatanB1 :
		case mnuSIMPeningkatanB1U :
		case mnuSIMPeningkatanB2 :
		case mnuSIMPeningkatanB2U :
		case mnuSIMPenurunanA :
		case mnuSIMPenurunanAU :
		case mnuSIMPenurunanB1 :
		case mnuSIMPenurunanB1U :
		case mnuSIMPenurunanB2 :
		case mnuSIMPenurunanB2U :
		case mnuSIMGolSamA :
		case mnuSIMGolSamAU :
		case mnuSIMGolSamB1 :
		case mnuSIMGolSamB1U :
		case mnuSIMGolSamB2 :
		case mnuSIMGolSamB2U :
		case mnuSIMGolSamC :
		case mnuSIMGolSamD :
		case mnuSIMMutasiPeningkatanAU :
		case mnuSIMMutasiPeningkatanB1 :
		case mnuSIMMutasiPeningkatanB1U :
		case mnuSIMMutasiPeningkatanB2 :
		case mnuSIMMutasiPeningkatanB2U :
		case mnuSIMMutasiPenurunanA :
		case mnuSIMMutasiPenurunanAU :
		case mnuSIMMutasiPenurunanB1 :
		case mnuSIMMutasiPenurunanB1U :
		case mnuSIMMutasiPenurunanB2 :
		case mnuSIMMutasiPenurunanB2U :
		case mnuSIMHilangA :
		case mnuSIMHilangAU :
		case mnuSIMHilangB1 :
		case mnuSIMHilangB1U :
		case mnuSIMHilangB2 :
		case mnuSIMHilangB2U :
		case mnuSIMHilangC :
		case mnuSIMHilangD :
		case mnuSIMHilangPeningkatanAU :
		case mnuSIMHilangPeningkatanB1 :
		case mnuSIMHilangPeningkatanB1U :
		case mnuSIMHilangPeningkatanB2 :
		case mnuSIMHilangPeningkatanB2U :
		case mnuSIMHilangPenurunanA :
		case mnuSIMHilangPenurunanAU :
		case mnuSIMHilangPenurunanB1 :
		case mnuSIMHilangPenurunanB1U :
		case mnuSIMHilangPenurunanB2 :
		case mnuUjiSim :
		case mnuBPKBBaruRoda2 :
		case mnuBPKBBaruRoda4 :
		case mnuBPKBGantiRoda2 :
		case mnuBPKBGantiRoda4 :
		case mnuSTCK :
		case mnuMutasiRanRoda2 :
		case mnuMutasiRanRoda4 :
#endif

		default:
			return 0;
	}

	lblKO:
		return -1;
}

int getAllTid(ACQ_T* acq)
{
	int 		ret;
	int 		i,j,idx=0;
	byte 	id;
	char 	tid[9];
	char 	name[lenName+1];

	for(i=0;i<dimAcq;i++)
	{
		mapMove(acqBeg,i);
		MAPGETBYTE(acqID,id,lblKO);

		memset(tid,0,sizeof(tid));
		MAPGET(acqTID,tid,lblKO);

		memset(name,0,sizeof(name));
		MAPGET(acqName, name, lblKO);

		if(tid[0] == 0)
			continue;

		for(j=0;j<idx;j++)
		{
			if(memcmp((acq+j)->tid,tid,8)==0)
			{
				break;
			}
		}

		if(j==idx)
		{
			memcpy((acq+idx)->tid,tid,8);
			(acq+idx)->acqIdx = id;
			memcpy((acq+idx)->acqName,name,lenName);
			idx++;
		}
	}
	return idx;

	lblKO:
		return -1;
}

int getAllAcq(ACQ_T* acq)
{
	int 	ret;
	int 	i,idx=0;
	byte id;
	char tid[9];
	char name[lenName+1];

	for(i=0;i<dimAcq;i++)
	{
		mapMove(acqBeg,i);
		MAPGETBYTE(acqID,id,lblKO);

		memset(tid,0,sizeof(tid));
		MAPGET(acqTID,tid,lblKO);

		memset(name,0,sizeof(name));
		MAPGET(acqName, name, lblKO);

		if(name[0] == 0)
			continue;

		(acq+idx)->acqIdx = id;
		memcpy((acq+idx)->tid,tid,8);
		memcpy((acq+idx)->acqName,name,lenName);

		idx++;
	}
	return idx;

	lblKO:
		return -1;
}

int selectAcquirer()
{
	ACQ_T 	acq[dimAcq];
	byte 	sta;
	int 		ret;
	int 		i;
	int 		jumlahAcq;
	char 	mnu[MNUMAX][dspW + 1];
	char 	*ptr[MNUMAX];
	int 		temp;
	byte 	acquirerId[MNUMAX+2];

	memset(acq,0,sizeof(acq));
	memset(mnu,0,sizeof(mnu));

	ret = getAllAcq(acq);
	if(ret < 0)
		goto lblKO;

	jumlahAcq = ret;

	for(i=0;i<MNUMAX;i++)
	{
		ptr[i]=NULL;
	}

	strcpy(mnu[0],"Select Acquirer");
	ptr[0] = mnu[0];

	strcpy(mnu[1],"Semua Acquirer");
	ptr[1] = mnu[1];

	temp = 2;
	for(i=0;i<jumlahAcq;i++)
	{
		if( (memcmp(acq[i].acqName,"MINI ATM",8) == 0) || (memcmp(acq[i].acqName,"TUNAI",5) == 0)   || (memcmp(acq[i].acqName,"BRIZZI",5) == 0))
		continue;

		strcpy(mnu[temp],acq[i].acqName);
		ptr[temp] = mnu[temp];
		acquirerId[temp]=acq[i].acqIdx;
		temp++;
	}
	sta = 0;

	ret = mnuSelect((Pchar *) ptr, sta, 60);
	if(ret <= 0)
		goto lblKO;

	if(ret == 0)
		goto lblEnd;

	sta    = ret;
	temp = sta % MNUMAX;

	if(temp == 1)
		ret = 0xff;
	else
		ret = (int)acquirerId[temp];

	goto lblEnd;

	lblKO:
		return -1;
	lblEnd:
		return ret;
}

int selectAcquirer1()
{
	ACQ_T 	acq[dimAcq];
	byte 	sta;
	int 		ret;
	int 		i;
	int 		jumlahAcq;
	char 	mnu[MNUMAX][dspW + 1];
	char 	*ptr[MNUMAX];
	int 		temp;
	byte 	acquirerId[MNUMAX+2];

	memset(acq,0,sizeof(acq));
	memset(mnu,0,sizeof(mnu));

	ret = getAllAcq(acq);
	if(ret < 0)
		goto lblKO;
	jumlahAcq = ret;

	for(i=0;i<MNUMAX;i++)
	{
		ptr[i]=NULL;
	}

	strcpy(mnu[0],"Select Acquirer");
	ptr[0] = mnu[0];

	temp = 1;
	for(i=0;i<jumlahAcq;i++)
	{
		if( (memcmp(acq[i].acqName,"MINI ATM",8) == 0) || (memcmp(acq[i].acqName,"TUNAI",5) == 0)   || (memcmp(acq[i].acqName,"BRIZZI",5) == 0))
			continue;

		strcpy(mnu[temp],acq[i].acqName);
		ptr[temp] = mnu[temp];
		acquirerId[temp]=acq[i].acqIdx;
		temp++;
	}
	sta = 0;

	ret = mnuSelect((Pchar *) ptr, sta, 60);
	if(ret < 0)
		goto lblKO;
	if(ret == 0)
		goto lblEnd;

	sta    = ret;
	temp = sta % MNUMAX;
	ret    = (int)acquirerId[temp];

	goto lblEnd;

	lblKO:
		return -1;
	lblEnd:
		return ret;
}

int selectTerminalId(ACQ_T *acq, byte* jumlahTid)
{
	byte 	sta;
	int 		ret;
	int 		i;
	char 	mnu[MNUMAX][dspW + 1];
	char 	*ptr[MNUMAX];
	int 		temp;

	ret = getAllTid(acq);
	if(ret < 0)
		goto lblKO;

	*jumlahTid = ret;

	for(i=0;i<MNUMAX;i++)
	{
		ptr[i]=NULL;
	}

	memset(mnu,0,sizeof(mnu));

	strcpy(mnu[0],"Pilih TID");
	ptr[0] = mnu[0];
	strcpy(mnu[1],"Semua TID");
	ptr[1] = mnu[1];

	for(i=0;i<*jumlahTid;i++)
	{
		strcpy(mnu[i+2],(acq+i)->tid);
		ptr[i+2] = mnu[i+2];
	}
	sta = 0;

	ret = mnuSelect((Pchar *) ptr, sta, 60);
	if(ret <= 0)
		goto lblKO;
	if(ret == 0)
		goto lblEnd;

	sta    = ret;
	temp = sta % MNUMAX;
	ret    = (int) temp;

	goto lblEnd;

	lblKO:
		return -1;
	lblEnd:
		return ret;
}

int checkPendingSettlement()
{
	byte 	isPending;
	int 		ret;

	MAPGETBYTE(acqIsPendingSettlement,isPending,lblKO);
	ret = (int) isPending;
	goto lblEnd;

	lblKO:
		return -1;

	lblEnd:
		if(ret == 1)
		usrInfo(infRetrySettle);

	return ret;
}

void ShowData(byte* txt, word len, int mode, int awalKolom, int banyakKolom)
{
	char 	buf[70];
	word 	c=0,i=0,j=0;

	memset(buf,0x20,awalKolom);
	j+=awalKolom;

	while(len != c)
	{
		if(mode == 0)
		{
#ifdef HEX_PAKAI_X
			sprintf(&buf[j],"x%02X ",txt[c]);
			j+=4;
#else
			sprintf(&buf[j],"%02X ",txt[c]);
			j+=3;
#endif
		}
		else if(mode == 1)
		{
			sprintf(&buf[j],"%c ",txt[c]);
			j+=2;
		}
		else
		{
			if(txt[c] < 0x20 || txt[c] > 0x7e)
			{
#ifdef HEX_PAKAI_X
				sprintf(&buf[j],"x%02X ",txt[c]);
				j+=4;
#else
				sprintf(&buf[j],"%02X ",txt[c]);
				j+=3;
#endif
			}
			else
			{
				sprintf(&buf[j],"%c ",txt[c]);
				j+=2;
			}
		}

		i++;
		c++;

		if(i==banyakKolom || len==c)
		{
			prtES("\x1B\x20", buf);
			i=j=0;
			memset(buf,0x20,awalKolom);
			j+=awalKolom;
		}
   	}
}

void viewMiniAtmLog()
{
	briViewBat(MINI_ATM_LOG,0);
}

void viewCardServiceLog()
{
	briViewBat(CARD_SERVICE_LOG,0);
}

void viewTunaiLog()
{
	briViewBat(TUNAI_LOG,0);
}

void viewTCashLog()
{
	briViewBat(TCASH_LOG,0);
}



#ifdef TEST_BRI
#if 0
void testTulisNvm()
{
    int ret;
    byte buf[20];
    card tmp;
    word idx;

    mapReset(briLogBeg);
    MAPPUTCARD(regBriLogIdx, 0, lblDBA);

    mapPut(traBriBankId,"111",3);
    MAPPUTCARD(traBriLogROC,1,lblDBA);
    mapPut(traBriLogDateTime,"20110317121212",lenDatTim);
    ret = briLogSave();
    briLogLoadByIndex(0);

    mapPut(traBriBankId,"222",3);
    MAPPUTCARD(traBriLogROC,2,lblDBA);
    mapPut(traBriLogDateTime,"20220318121212",lenDatTim);
    ret = briLogSave();
    briLogLoadByIndex(1);

    mapPut(traBriBankId,"333",3);
    MAPPUTCARD(traBriLogROC,3,lblDBA);
    mapPut(traBriLogDateTime,"20330319121212",lenDatTim);
    ret = briLogSave();
    briLogLoadByIndex(2);

    mapPut(traBriBankId,"444",3);
    MAPPUTCARD(traBriLogROC,4,lblDBA);
    mapPut(traBriLogDateTime,"20440320121212",lenDatTim);
    ret = briLogSave();
    briLogLoadByIndex(3);

    mapPut(traBriBankId,"555",3);
    MAPPUTCARD(traBriLogROC,5,lblDBA);
    mapPut(traBriLogDateTime,"20550320121212",lenDatTim);
    ret = briLogSave();
    briLogLoadByIndex(0);
    briLogLoadByIndex(3);


lblDBA:
	return;
}

void testBacaNvmIndex()
{
    int ret;
    byte buf[20];
    word idx;
    int i;

    for(i=0;i<5;i++)
    {
        ret = briLogLoadByIndex((card)i);
    }

lblDBA:
	return;
}

void testBacaNvmDate()
{
    int ret;
    byte buf[20];
    word idx;
    int i;

    for(i=0;i<5;i++)
    {
        ret = briLogLoadByDate((card)i,"20440320");
    }

lblDBA:
	return;
}

void testBacaNvmRoc()
{
    int ret;
    byte buf[20];
    word idx;
    int i;

    MAPPUTSTR(traFindRoc,"1",lblDBA);
    ret = briLogLoadByRoc();

    MAPPUTSTR(traFindRoc,"2",lblDBA);
    ret = briLogLoadByRoc();

    MAPPUTSTR(traFindRoc,"3",lblDBA);
    ret = briLogLoadByRoc();

    MAPPUTSTR(traFindRoc,"4",lblDBA);
    ret = briLogLoadByRoc();

    MAPPUTSTR(traFindRoc,"5",lblDBA);
    ret = briLogLoadByRoc();


lblDBA:
	return;
}
#endif

void testBacaLogSemua()
{
	int 	ret;
	int 	i=0;

	i=0;
	do
	{
		ret = briLogLoadByIndex(MINI_ATM_LOG,(card)i++);
	}while(ret > 0);

	i=0;
	do
	{
		ret = briLogLoadByIndex(CARD_SERVICE_LOG,(card)i++);
	}while(ret > 0);

	i=0;
	do
	{
		ret = briLogLoadByIndex(TUNAI_LOG,(card)i++);
	}while(ret > 0);

	i=0;
	do
	{
		ret = briLogLoadByIndex(TCASH_LOG,(card)i++);
	}while(ret > 0);

	return;
}

void testPrintTotalHariIni()
{
	briPrintTotalToday(MINI_ATM_LOG);
}

void testPrintTotalHariSemua()
{
	briLogPrintTotal(MINI_ATM_LOG,NULL);
}

void testPrintTotal()
{
    briPrintTotal(MINI_ATM_LOG);
}

void testPrintDetail()
{
    briPrintDetail(MINI_ATM_LOG);
}

void testReprintLastMiniAtm()
{
    briLogDuplicata(MINI_ATM_LOG);
}

void testReprintMiniAtm()
{
    briLogPrintTxn(MINI_ATM_LOG);
}

#endif

#ifdef DUMP_ISO_MESSAGE
void setDumpIsoMessage()
{
	int 		iRet;
	char 	txt[20];
	int 		ret;

	dspClear();
	MAPPUTWORD(traMnuItm, mnuFn99, lblKO);
	ret = valOpr();
	CHECK(ret > 0, lblKO);

	dspLS(0,"Print Iso Msg");
	dspLS(1,"0-Mati 1-Nyala");
	dspLS(2,"Pilih?");

	while(1)
	{
		memset(txt,0,sizeof(txt));
		iRet = enterStr(3, txt, 2);
		if(iRet <= 0)
			return;

		if(txt[0] == 0x30)
		{
			MAPPUTBYTE(appDumpIsoMessageFlag,0,lblDBA);
			break;
		}
		else if(txt[0] == 0x31)
		{
			MAPPUTBYTE(appDumpIsoMessageFlag,1,lblDBA);
			break;
		}
	}

	return;

	lblDBA:
		usrInfo(infDataAccessError);
	lblKO:
		return;
}
#endif


#ifdef BRIZI_DISCOUNT
void setDiscount(void)
{
	char 	txt[20], txts[20];
	char 	buff[20];
	int 		ret;
	byte 	Exp;

	dspClear();
	MAPPUTWORD(traMnuItm, mnuFn99, lblKO);
	ret = valOpr();
	CHECK(ret > 0, lblKO);

	dspLS(0,"Parameter Disc :");
	dspLS(1,"1-(Rp)  2-(%)");
	dspLS(2,"Pilih?");

	while(1)
	{
		memset(txt,0,sizeof(txt));
		ret = enterStr(3, txt, 2);
		if(ret == kbdANN || ret == 0)
			goto lblKO;

		if(txt[0] == 0x31)
		{
			dspClear();
			dspLS(0,"Nom. Disc (Rp):");
			MAPGETBYTE(appExp, Exp, lblKO);
			while(1)
			{
				memset(txts,0,sizeof(txts)-1);
				ret = enterAmt(2,txts,Exp);
				if(ret <= 0)
					return;
				if(ret == kbdANN || ret == 0)
					goto lblKO;
				if(strlen(txts) >= 2)
					break;
				Beep();
			}

			MAPPUTBYTE(appBriziDiscFlag,1,lblKO);
			MAPPUTSTR(appBriziDiscValue, txts, lblKO);

			MAPGET(appBriziDiscValue,buff,lblKO);
			break;
		}
		else if(txt[0] == 0x32)
		{
			dspClear();
			dspLS(0,"Nom. Disc (%):");
			MAPGETBYTE(appExp, Exp, lblKO);
			while(1)
			{
				memset(txts,0,sizeof(txts)-1);
				ret = enterStr(2,txts,3);
				if(ret <= 0)
					return;
				if(ret == kbdANN || ret == 0)
					goto lblKO;
				if(strlen(txts) >= 1)
					break;
				Beep();
			}
			MAPPUTBYTE(appBriziDiscFlag,2,lblKO);
			MAPPUTSTR(appBriziDiscValue, txts,lblKO);

			MAPGET(appBriziDiscValue,buff,lblKO);
			break;
		}
	}

	lblKO:
		return;
}
#endif



void setDebugData()
{
	int 		iRet;
	char 	txt[20];
	int 		ret;

	dspClear();
	MAPPUTWORD(traMnuItm, mnuFn99, lblKO);
	ret = valOpr();
	CHECK(ret > 0, lblKO);

	dspLS(0,"Debug Data");
	dspLS(1,"0-Mati 1-Nyala");
	dspLS(2,"Pilih?");

	while(1)
	{
		memset(txt,0,sizeof(txt));
		iRet = enterStr(3, txt, 2);
		if(iRet <= 0)
			return;
		if(txt[0] == 0x30)
		{
			MAPPUTBYTE(appDebugDataFlag,0,lblDBA);
			break;
		}
		else if(txt[0] == 0x31)
		{
			MAPPUTBYTE(appDebugDataFlag,1,lblDBA);
			break;
		}
	}

	return;

	lblDBA:
		usrInfo(infDataAccessError);
	lblKO:
		return;
}

//@@AS-SIMAS : Start
int parseField57(const byte* val, word len)
{
	int 			ret;
	byte 		txnType;
	const byte 	*ptr;

	ptr = val;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch (txnType)
	{
		case trtDebitLogon:
#ifdef _DEBUG_TLE
			hex_dump_char("--DE-57 RESP--",val,len);
#endif
			ret=mapPut(traTPKenc,val+10,lenMKey);
			CHECK(ret >= 0, lblKO);

			ret=mapPut(traTAKenc,val+26,lenMKey);
			CHECK(ret >= 0, lblKO);

			ret=mapPut(traTLEenc,val+42,lenMKey);
			CHECK(ret >= 0, lblKO);
			break;
		default:
			break;
	}

	return 0;

	lblKO:
		return -1;
}
//@@AS-SIMAS : End



//@@AS0.0.26 - start
int parseField62(const byte* val, word len)
{
	int 			ret;
	byte 		txnType;
	const byte 	*ptr;
	byte 		WK[lenMKey];
	char 		buf[20];

	ptr = val;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch (txnType)
	{
		case trtDebitLogon:
#ifdef _DEBUG_TLE
			prtS("WK:");
			ShowData(val,16,0,0,8);
#endif
			if (len == lenMKey*2) {
				hex2bin(WK,val,lenMKey);
			}
			else {
				hex2bin(WK,val,lenMKey/2);
				hex2bin(&WK[lenMKey/2],val,lenMKey/2);
			}
			ret = mapPut(traWorkingKey,WK,lenMKey);
			CHECK(ret >= 0, lblKO);
			break;

		case trtPreAuth:
		case trtPreCancel:
			MAPGET(traPINInternetBanking,buf,lblKO);
			memcpy(ptr,buf,len);
			ptr+=len;
			ret = mapPut(traRspBit11,ptr,len);
			CHECK(ret >= 0, lblKO);
			break;

		case trtPreCom:
			ret = mapPut(traRspBit11,ptr,len);
			CHECK(ret >= 0, lblKO);
			break;

		default:
			break;
	}

	return 0;

	lblKO:
		return -1;
}

int resetLogonDateTime()
{
	int ret=0;
	MAPPUTSTR(appLogonDT,"00000000000000",lblKO);

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		return ret;
}

int setFirstPowerOnFlag(byte flag_value)
{
	int ret=0;
	MAPPUTBYTE(appIsFirstPowerOn,flag_value,lblKO);

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		return ret;
}

//@@AS0.0.26 - end

//@@SIMAS-INIT_DB -start
int setFirstPowerOnTempFlag(byte flag_value)
{
	int ret=0;
	MAPPUTBYTE(appIsFirstPowerOnTemp,flag_value,lblKO);

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		return ret;
}
//@@SIMAS-INIT_DB - end



#ifdef _USE_ECR
int setECRFlag(byte flag_value)
{
	int ret=0;
	MAPPUTBYTE(appIsECR,flag_value,lblKO);

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		return ret;
}

int getInfoEcr(byte flag_value, char* infoEcr)
{
	int 		ret=0;
	char 	TID[20];
	char 	MID[20];
	char 	temp[5];
	char 	PAN[128];
	char 	buf[25], buf2[25], buf3[25];
	char 	currBat[100];
	card 	dupStan;
	byte 	tnxType;
	char 	str[128];

	memset(buf, 0, sizeof(buf));
	memset(buf2, 0, sizeof(buf2));
	memset(buf3, 0, sizeof(buf3));
	memset(currBat, 0, sizeof(currBat));
	memset(temp, 0, sizeof(temp));
	memset(infoEcr, 0, sizeof(infoEcr));
	memset(TID, 0, sizeof(TID));
	memset(MID, 0, sizeof(MID));
	memset(PAN, 0, sizeof(PAN));
	memset(str, 0, sizeof(str));

	sprintf(temp, "%d", flag_value);
	MAPGET(acqTID, TID, lblKO);
	MAPGET(acqMID, MID, lblKO);
	MAPGET(acqCurBat, currBat, lblKO);
	mapGet(traPan,PAN,sizeof(PAN));

	//tipe transaksi
	strcat(infoEcr, temp);
	strcat(infoEcr, ";");

	//TID
	strcat(infoEcr, TID);
	strcat(infoEcr, ";");

	//MID
	strcat(infoEcr, MID);
	strcat(infoEcr, ";");

	//PAN
	if(flag_value != 3)
	{
		if(strlen(PAN) <= 0)
			strcat(infoEcr, "0");
		else
			strcat(infoEcr, PAN);
	}
	else
	{
		mapGet(TRA_NOMOR_KARTU_BRIZZI,PAN,sizeof(PAN));
		strcat(infoEcr, PAN);
	}
	strcat(infoEcr, ";");

	//TIME
	memset(buf, 0, sizeof(buf));
	memset(buf3, 0, sizeof(buf3));
	MAPGET(traDatTim, buf, lblKO);
	strFormatDatTim(traDatTim);
	memset(buf2, 0, sizeof(buf2));
	memcpy(buf2,buf+8,TIME_LEN);
	memcpy(buf3,buf2,2);
	memcpy(buf3+2,":",1);
	memcpy(buf3+3,buf2+2,2);
	strcat(infoEcr, buf3);
	strcat(infoEcr, ";");


	//DATE
	memset(buf, 0, sizeof(buf));
	memset(buf3, 0, sizeof(buf3));
	MAPGET(traDatTim, buf, lblKO);
	strFormatDatTim(traDatTim);
	memset(buf2, 0, sizeof(buf2));
	memcpy(buf2,buf+2,DATE_LEN);
	strcpy(buf3,"20");
	memcpy(buf3+2,buf2,DATE_LEN);
	strcat(infoEcr, buf3);
	strcat(infoEcr, ";");


	//STAN
	MAPGETCARD(regSTAN, dupStan, lblKO);
	memset(buf2, 0, sizeof(buf2));
	sprintf(buf2, "%u", dupStan);
	strcat(infoEcr, buf2);
	strcat(infoEcr, ";");

	MAPGETBYTE(traTxnType, tnxType, lblKO);
	//APPROVAL CODE
	if((tnxType >= trtPrepaidPayment) && (tnxType <= trtPrepaidSettlement))
	{
		memset(buf2, 0, sizeof(buf2));
		MAPGET(TRA_HASH,buf2,lblKO);
		bin2hex(str,buf2,HASH_LEN);
		strcat(infoEcr, str);
		strcat(infoEcr, ";");
	}
	else
	{
		memset(buf2, 0, sizeof(buf2));
		mapGet(traAutCod, buf2, lenAutCod);
		if(strlen(buf2) > 0){
			strcat(infoEcr, buf2);
			strcat(infoEcr, ";");
		}
		else{
			strcat(infoEcr, "00;");
		}
	}

	//BATCH NUMBER
	strcat(infoEcr, currBat);
	strcat(infoEcr, ";");


	if((tnxType >= trtPrepaidPayment) && (tnxType <= trtPrepaidSettlement))
	{
		//ERROR NUMBER
		strcat(infoEcr, "00");
		strcat(infoEcr, ";");
	}
	else
	{
		//ERROR NUMBER
		memset(buf2, 0, sizeof(buf2));
		MAPGET(traRspCod, buf2, lblKO);
		if(strlen(buf2) > 0){
			strcat(infoEcr, buf2);
			strcat(infoEcr, ";");
		}
		else{
			strcat(infoEcr, "00;");
		}
	}

	//AMOUNT
	memset(buf2, 0, sizeof(buf2));
	MAPGET(traAmt, buf2, lblKO);
	fmtPad(buf2,-12,'0');
	strcat(infoEcr, buf2);

	strcat(infoEcr, "\x0A");

	setECRFlag(0);

	lblKO:
		ret = -1;
		goto lblEnd;

	lblEnd:
		return ret;
}
#endif

//@@AS0.0.29 - start
byte OptionEnabled(byte idx, byte optflag) {
	char 	tempBuf[51];
	byte 	byteBuf[13];

	if (idx > 12)
		return (0);

	memset(tempBuf,0,sizeof(tempBuf));
	mapGet(rptHdrSiteAddr4, tempBuf, 0);

	hex2bin(byteBuf, tempBuf, 12);
	return(byteBuf[idx] & optflag);
}

void modifyMnuLayout(void) {
	byte tempBuf[25];
/*
	mapGet(mnuSale,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_SALE))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuSale,tempBuf,0);

	mapGet(mnuVoid,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_VOID))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuVoid,tempBuf,0);

	mapGet(mnuSettlement,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_SETTLEMENT))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuSettlement,tempBuf,0);
*/
	mapGet(mnuPre,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_PRE))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuPre,tempBuf,0);

	mapGet(mnuPReprint,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_PREPRINT))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuPReprint,tempBuf,0);

	mapGet(mnuMrcLog,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_MERCHLOG))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuMrcLog,tempBuf,0);



	mapGet(mnuCardService,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_CARDSERVICES))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuCardService,tempBuf,0);

	mapGet(mnuTunai,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_TUNAI))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuTunai,tempBuf,0);

	mapGet(mnuPembelian,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_PEMBELIAN))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuPembelian,tempBuf,0);

	mapGet(mnuMiniATM,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_MINIATM))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuMiniATM,tempBuf,0);

#ifdef T_BANK
	mapGet(mnuTCash,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_TBANK))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuTCash,tempBuf,0);
#endif

#ifdef PREPAID
	mapGet(mnuBrizzi,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_BRIZZI))
	{
		mapPutByte(appIsPrepaidEnable,1);
		tempBuf[0] = '1';
	}
	else
	{
		mapPutByte(appIsPrepaidEnable,0);
		tempBuf[0] = '?';
	}
		mapPut(mnuBrizzi,tempBuf,0);
#endif

#ifdef ABSENSI
	mapGet(mnuAbsen,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_ABSENSI))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuAbsen,tempBuf,0);
#endif

#ifdef SSB
	mapGet(mnuSSB,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_SSB))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuSSB,tempBuf,0);
#endif

	mapGet(mnuLogon,tempBuf,sizeof(tempBuf));
	if (OptionEnabled(OPT1_IDX,OPT1_MNUL1_LOGON))
		tempBuf[0] = '1';
	else tempBuf[0] = '?';
		mapPut(mnuLogon,tempBuf,0);

	//checkSubMnuLayout();
	mapSave(mnuBeg);

	mapCache(-mnuBeg); //disable cache
	mapCache(mnuBeg);
	mapLoad(mnuBeg);
}

//@@AS0.0.29 - end

void checkSubMnuLayout(void)
{
	int 	i, x=0;
	byte temp, tempBuf[25];
	int 	ret;

	for(i=mnuBeg; i <= mnuEnd; i++)
	{
		x = i - mnuBeg;
		MAPGETBYTE(appmnuBeg + x, temp, lblKO);
		mapGet(mnuBeg + x, tempBuf, sizeof(tempBuf));
		if((mnuBeg + x) == mnuMainMenu)
		{
			tempBuf[0] = '0';
			mapPut(mnuBeg + x, tempBuf, 0);
		}
		else
		{
			if(temp == 0)
			{
				/*
				if(
					(i == mnuCardService) ||
					(i == mnuTunai) ||
					(i == mnuPembelian) ||
					(i == mnuMiniATM) ||
					(i == mnuBrizzi) ||
					(i == mnuAbsen) ||
					(i == mnuSSB) ||
					(i == mnuLogon)
				)
				{
					tempBuf[0] = '1';
					mapPut(mnuBeg + x, tempBuf, 0);
				}
				else*/ if( ((i > mnuCardService) && (i <= mnuCReprint)) ||
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
					((i >= mnuInfoSaldo) && (i < mnuInfoKodeBank1)) ||
					((i >= mnuTransferSesamaBRI) && (i <= mnuInfoKodeBank)) ||
					((i >= mnuPembayaranBriva) && (i <= mnuPembayaranPostPaid)) ||
					(i == mnuPembayaranDPLKR) ||
					(i == mnuPembayaranTiketPesawat) ||
					(i == mnuPembayaranPdam) ||
					(i == mnuPembayaranPLN) ||
					(i == mnuPembayaranKK) ||
					(i == mnuPembayaranPendidikan) ||
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
					((i >= mnuPembayaranIm325) &&(i <= mnuPembayaranIm3150)) ||
					((i >= mnuPembayaranIm3500) &&(i <= mnuPembayaranIm31000)) ||
		//			((i >= mnuPembayaranEsia25) &&(i <= mnuPembayaranEsia200)) ||
					((i >= mnuPembayaranEsia50) &&(i <= mnuPembayaranEsia50)) ||
					((i >= mnuPembayaranEsia100) &&(i <= mnuPembayaranEsia200)) ||
					((i >= mnuPembayaranSmart20) &&(i <= mnuPembayaranSmart500)) ||
		//			((i >= mnuPembayaranFren20) &&(i <= mnuPembayaranFren500)) ||   disable menu pulsa
					((i >= mnuPembayaranFren25) &&(i <= mnuPembayaranFren500)) ||
					((i >= mnuPembayaranThree20) &&(i <= mnuPembayaranThree500)) ||
					((i >= mnuPembayaranAxis25) &&(i <= mnuPembayaranAxis300)) ||
		//			((i >= mnuPembayaranXl25) &&(i <= mnuPembayaranXl500)) ||
					((i >= mnuPembayaranXl25) &&(i <= mnuPembayaranXl50)) ||
					((i >= mnuPembayaranXl100) &&(i <= mnuPembayaranXl100)) |
					((i >= mnuPembayaranXl200) &&(i <= mnuPembayaranXl200)) ||
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
			}
			else if((temp == 1) && (tempBuf[0] != '?'))
			{
				tempBuf[0] = '?';
				mapPut(mnuBeg + x, tempBuf, 0);
			}
		}
	}

	lblKO:
		;
}

void modifySubMnuLayout(void)
{
	int 	x =0, i=0;
	byte 	tempBuf[25];

	for(i=mnuBeg; i <= mnuEnd; i++)
	{
		x = i - mnuBeg;
		mapGet(mnuBeg + x, tempBuf, sizeof(tempBuf));
		if(tempBuf[0] == '?')
		{
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
					(i == mnuPembayaranPendidikan) ||
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
					((i >= mnuPembayaranSPP) && (i <= mnuPembayaranKodeUniv)) ||
					((i >= mnuPembayaranKKBRI) && (i <= mnuPembayaranKKCITI)) ||
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
		}
	}

	mapSave(mnuBeg);
	mapCache(-mnuBeg);
	mapCache(mnuBeg);
	mapLoad(mnuBeg);
}

int briModifyPinEntry()
{
	byte 	kartu;
	word 	mnuItem;
	int 		ret;

	MAPGETBYTE(traKategoriKartu,kartu,lblKO);
	MAPGETWORD(traMnuItm,mnuItem,lblKO);

	switch(kartu)
	{
		case CREDIT_CARD:
		case NON_BRI_DEBIT_CARD:
		case NON_BRI_CREDIT_CARD:
			if(! (mnuItem > mnuPembelian && mnuItem <= mnuSettlement))
				return 1; //@@AS0.0.33 - change it to 1
			else
				return 100;
			break;

		default:
			return 100;
	}

	return 100;

	lblKO:
		return -1;
}

int adaTransPembelian()
{
	byte buf[25];

	memset(buf,0,sizeof(buf));
	mapGet(mnuPre,buf,sizeof(buf));
	if(buf[0] == '1')
		return 1;

	return 0;
}

#ifdef _USE_ECR
int POSPrintReferenceNum(void)
{
	int 	ret;
	char buf[20];

	dspClear();
	dspLS(0|0x40,"      POS         ");
	dspLS(1,"No Reference:");

	memset(buf,0,sizeof(buf));
	while(1)
	{
		ret = enterPhr(2, buf, lenLWBP+4);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		if(ret < 0)
			goto lblKO;
		if(strlen(buf) >= 4)
			break;

		Beep();
	}

	MAPPUTSTR(traLWBP,buf,lblKO);

	return 1;
	lblBatal:
		ret = 0;
		goto lblEnd;
	lblKO:
		ret = -1;
	lblEnd:
		return ret;
}

#endif




