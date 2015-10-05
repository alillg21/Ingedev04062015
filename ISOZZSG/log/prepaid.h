#ifndef __PREPAID_H__
#define __PREPAID_H__

#include "str.h"

#define SUCCESS                         1
#define FAIL                            -1
#define PREPAID_SUCCESS                 0x00
#define PREPAID_COMM_ERROR              0xC0
#define PREPAID_DATA_ERROR              0xD0

#define SAM_INIT		    0x30
#define INF_SALDO           0x31
#define REDEEM_INQURY       0x31
#define SALE_TRANS          0x32
#define INFO_INQURIY        0x33
#define AKTIVASI_INQUIRY    0x33
#define TOP_UP_INQUIRY      0x33
#define TOP_UP              0x34
#define AKTIVASI            0x34
#define REDEEM              0x35
#define DISPLAY_AMOUNT      0x36
#define TRANS_LOG           0x64
#define INF_KARTU           0x65

//++ @agmr - brizzi2
#define REAKTIVASI_INQUIRY  0x39
#define REAKTIVASI_TRANS    0x40
#define VOID_INQUIRY        0x33
#define VOID_BRIZZI         0x38
#define UBAH_TANGGAL        0x99
//-- @agmr - brizzi2

#define RESPONSE_CODE_LEN 1
#define DATE_LEN 6
#define TIME_LEN 6
#define DATE_HEX_LEN 3
#define TIME_HEX_LEN 3
#define TID_LEN 8
#define MID_LEN 8
#define PROC_LEN 6
#define REF_NUMBER_LEN 6
#define BATCH_NUMBER_LEN 2
#define CARD_NUMBER_LEN 8
#define RANDOM_LEN 24
#define UID_LEN 7
#define AMOUNT_LEN_S 8
#define AMOUNT_LEN_L 10
#define AMOUNT_HEX_LEN 3
#define HASH_LEN 4
#define KEY_HOST_LEN 24
#define NOMOR_KARTU_BRIZZI_LEN 16
#define SISA_SALDO_LEN AMOUNT_LEN_L
#define ISSUE_DATE_LEN 3
#define ISSUE_CABANG_LEN 2
#define STATUS_KARTU_LEN 2
#define TOTAL_LOG_LEN_LEN 2

//++ @agmr - brizzi2
#define BIAYA_ADMIN_LEN 10
#define LAMA_PASIF_LEN  2
#define LAST_TRANS_DATE_LEN 6
//-- @agmr - brizzi2

#define PROCODE "808000"

#define TRA_REF_NUMBER traPasswordPetugas
#define LOG_REF_NUMBER logRefNumber
#define LEN_TRA_REF_NUMBER lenPasswordPetugas

#define TRA_HASH traPeriode
#define LOG_HASH logHash
#define LEN_TRA_HASH lenPeriode

#define TRA_SALDO_AWAL traUserId
#define LOG_SALDO_AWAL logSaldoAwal
#define LEN_TRA_SALDO_AWAL lenUserId

#define TRA_FEE TRA_SALDO_AWAL
#define LOG_FEE LOG_SALDO_AWAL
#define LEN_TRA_FEE LEN_TRA_SALDO_AWAL

#define TRA_SISA_SALDO_DEPOSIT traPokokHutang
#define LOG_SISA_SALDO_DEPOSIT logSisaSaldoDeposit
#define LEN_TRA_SISA_SALDO_DEPOSIT lenPokokHutang

#define TRA_TOTAL_REDEEM TRA_SISA_SALDO_DEPOSIT
#define LOG_TOTAL_REDEEM LOG_SISA_SALDO_DEPOSIT
#define LEN_TRA_TOTAL_REDEEM LEN_TRA_SISA_SALDO_DEPOSIT

#define TRA_NOMOR_KARTU_BRIZZI traNoVoucher
#define LOG_NOMOR_KARTU_BRIZZI logNomorKartuBrizzi
#define LEN_TRA_NOMOR_KARTU_BRIZZI lenNoVoucher

#define TRA_PROCODE traInstallmentCicilan
#define LOG_PROCODE logInstallmentCicilan
#define LEN_TRA_PROCODE lenInstallmentCicilan

#define TRA_SISA_SALDO traSisaSaldo
#define LOG_SISA_SALDO logSisaSaldo
#define LEN_TRA_SISA_SALDO lenSisaSaldo

#define TRA_PREPAID_BATCH_NUM traInstallmentPlan
#define LOG_PREPAID_BATCH_NUM logInstallmentPlan
#define LEN_TRA_PREPAID_BATCH_NUM lenInstallmentPlan

#define TRA_SALDO_DEPOSIT traInquiryAmt
//#define LOG_SALDO_DEPOSIT logInquiryAmt //@agmr - brizzi2
#define LEN_TRA_SALDO_DEPOSIT lenInquiryAmt

#define TRA_KEY_HOST traBuffer1
#define LOG_KEY_HOST logBuffer1
#define LEN_TRA_KEY_HOST lenBuffer1

//++ @agmr - brizzi2
#define TRA_NOMOR_KARTU_DARI_HOST traNomorKartuPetugas
#define LEN_TRA_NOMOR_KARTU_DARI_HOST leNomorKartuPetugas

#define TRA_LAST_TRANS_DATE traFee
#define LEN_TRA_LAST_TRANS_DATE lenFee

#define TRA_BIAYA_ADMIN TRA_KEY_HOST
#define LOG_BIAYA_ADMIN LOG_KEY_HOST
#define LEN_TRA_BIAYA_ADMIN LEN_TRA_KEY_HOST

#define TRA_STATUS_KARTU traBuffer2
#define LOG_STATUS_KARTU logBuffer2
#define LEN_TRA_STATUS_KARTU lenBuffer2

#define TRA_LAMA_PASIF traInstallmentTerm
#define LOG_LAMA_PASIF logInstallmentTerm
#define LEN_TRA_LAMA_PASIF lenInstallmentTerm
//-- @agmr - brizzi2

#define MAX_LOG 11

typedef struct
{
    char amt[lenAmt+1];
//    char pan[lenPan+1];
    char kartuBrizzi[NOMOR_KARTU_BRIZZI_LEN];
    char txnDateTime[lenDatTim+1];
//    char refNum[lenRrn+1];
//    char expDat[lenExpDat+1];
//    char appCode[lenAutCod+1];
    char transName[21];
//    card stan;
    card roc;  
    byte discFlag;
   char discValue[21];
   char discBefAmount[21];
} PREPAID_REPORT_TRANS_T;

typedef struct
{
	byte responseCode;
	byte date[DATE_LEN];
	byte time[TIME_LEN];
//	byte transType;
	byte tid[TID_LEN];
	byte mid[MID_LEN];
//	byte procCode[PROC_LEN];
	byte refNumber[REF_NUMBER_LEN];
	byte batchNumber[BATCH_NUMBER_LEN];
	byte amountHex[AMOUNT_HEX_LEN];
	byte amountMin[AMOUNT_LEN_L];
	byte amountMax[AMOUNT_LEN_L];
	
	byte cardNumber[CARD_NUMBER_LEN];
	byte issueDate[ISSUE_DATE_LEN];
	byte issueCabang[ISSUE_CABANG_LEN];
	byte statusKartu[STATUS_KARTU_LEN];
	byte uid[UID_LEN];
	byte random[RANDOM_LEN];
	byte amount[AMOUNT_LEN_L+1];
	byte hash[HASH_LEN];
	byte keyHost[KEY_HOST_LEN]; //hex (kirim ke host string 2*KEY_HOST_LEN
	byte lastTransDate[LAST_TRANS_DATE_LEN/2]; //@agmr - brizzi2	
}PREPAID_T;

typedef struct
{
    byte tid[TID_LEN];
	byte mid[MID_LEN];
	byte date[DATE_HEX_LEN];
	byte time[TIME_HEX_LEN];	
	byte transType;
	byte amountHex[AMOUNT_HEX_LEN];
	byte currentBalance[AMOUNT_HEX_LEN];
	byte lastUpdateBalance[AMOUNT_HEX_LEN];
	
}PREPAID_TRANS_REC_T;

typedef struct
{
	byte responseCode;
	byte cardNumber[CARD_NUMBER_LEN];
	byte totalLogLen[TOTAL_LOG_LEN_LEN];
	int  totalRec;
	PREPAID_TRANS_REC_T log[MAX_LOG];
	
}PREPAID_TRANS_LOG_T;

//++ @agmr - brizzi2
typedef struct
{
    byte responseCode;
    byte tid[TID_LEN];
	byte mid[MID_LEN];
	byte date[DATE_HEX_LEN];
	byte time[TIME_HEX_LEN];
	byte amount[AMOUNT_LEN_L+1];
	byte keyHost[KEY_HOST_LEN];
	
	byte cardNumber[CARD_NUMBER_LEN];
	byte uid[UID_LEN];
	byte balance[AMOUNT_LEN_S+1];
}PREPAID_VOID_T;
//-- @agmr - brizzi2

void printPrepaidRecDetail(byte tanda,PREPAID_REPORT_TRANS_T *detail);
void xx_hex_dump(byte mode,char* title,byte* data, int len);
int getPrepaidRefNumber(byte* refNum);
int getPrepaidBatchNumber(byte* batchNum);
int updatePrepaidRefNumber();
int getPrepaidNii(char* Nii);

void decString2Hex(byte* hex, int hexLen, byte* dec, int decLen);

//int isAksesKeReader(byte txnType);

int isPrepaidTransaction(byte isByTransType);
void pmtPrepaidInit();
void pmtPrepaidInitSaldo();
int pmtPrepaidInfoSaldo();
int pmtPrepaidInfoDeposit();
int pmtPrepaidPayment();
int pmtPrepaidPaymentDisc();
int pmtPrepaidRedeem();

//++ @agmr - brizzi2
int pmtPrepaidReaktivasiInquiry();
int pmtPrepaidReaktivasi();
int pmtPrepaidVoidInquiry();
int pmtPrepaidVoid();
//-- @agmr - brizzi2

int autoSamInit();
int pmtPrepaidAktivasiDeposit();
int pmtPrepaidInfoKartu();
int pmtPrepaidPrintLog();
int pmtPrepaidSettlement();
void prepaidReprint(int isLast);
void prepaidDateSummary();
void prepaidAllSummary();
void prepaidDateDetail(byte isPaymentOnly);
void prepaidAllDetail(byte isPaymentOnly);
int prepaidBatchIncrement(void);
int prepaidSettlementReceipt();
int prepaidPrintDetail(char* dateTime,byte isPaymentOnly);
int printPrepaidSummary(char* dateTime);
int pmtPrepaidRedeemInquiry();
int clearPrepaidReversalFlag();

int selectPrepaidInit(char* minAmt, char* maxAmt);
int getPrepaidTidMid(char* tid, char* mid);

int getPrepaidVoidTrans(); //@agmr - brizzi2

void pmtBrizzi(void);
int prepaidTrans(byte txnType);
int pmtPrepaidTopUp();
int pmtPrepaidInquiry();

int KonfirmasiPrepaidRedeem();

//++ @agmr - brizzi2
int KonfirmasiPrepaidReaktivasi();
int KonfirmasiPrepaidVoid();
//-- @agmr - brizzi2


int doPrepaidReversal();

int getPrepaidCardInfo(PREPAID_T *pData);

#ifdef DEBUG_PREPAID_DATA
void debugData(byte mode,char* title,byte* data, int len);
#endif

#ifdef TEST_BRI
void testInfoSaldo();
void testPrepaidInit();
void testPrepaidSale();
#endif

#ifdef TEST_PREPAID_SETTLEMENT
void resetPrepaidUploadedFlag();
#endif
#endif //__PREPAID_H__
