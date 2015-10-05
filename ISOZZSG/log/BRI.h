
#ifndef __BRI_H__
#define __BRI_H__

#include "str.h"



#define VERSI_APP	"V1.01"  //@@SIMAS-APP_VERSION

//@@AS0.0.29 - start
enum {
	OPT1_IDX,
	OPT2_IDX,
	OPT3_IDX,
	OPT4_IDX,
	OPT5_IDX,
	OPT6_IDX,
	OPT7_IDX,
	OPT8_IDX,
	OPT9_IDX,
	OPT10_IDX,
	OPT11_IDX,
	OPT12_IDX
};

#define OPT1_MNUL1_SALE			0x01
#define OPT1_MNUL1_VOID			0x02
#define OPT1_MNUL1_SETTLEMENT	0x03
#define OPT1_MNUL1_PRE			0x04
#define OPT1_MNUL1_PREPRINT		0x05
#define OPT1_MNUL1_MERCHLOG		0x06


#define OPT1_MNUL1_CARDSERVICES	0x10
#define OPT1_MNUL1_TUNAI			0x20
#define OPT1_MNUL1_PEMBELIAN		0x30
#define OPT1_MNUL1_MINIATM		0x40
#define OPT1_MNUL1_BRIZZI			0x50
#define OPT1_MNUL1_ABSENSI		0x60
#define OPT1_MNUL1_SSB				0x70
#define OPT1_MNUL1_LOGON			0x80
#define OPT1_RFU4					0x90
#define OPT1_MNUL1_TBANK			0x99

//@@AS0.0.29 - end

//typedef enum
enum ReportType
{
    //card service
    GANTI_PIN,
    AKTIVASI_KARTU,
    RENEWAL_KARTU,
    GANTI_PASSWORD_SPV,
    REISSUE_PIN,

    //mini atm
    INFO_SALDO,
    INFO_SALDO_BANK_LAIN,
    MINI_STATEMENT,
    PREVILEGE,
    MUTASI_REKENING,
    TRANSFER_SESAMA_BRI,
    TRANSFER_ANTAR_BANK,
    /*Changed By Ali*/
    PEMBAYARAN_TV,
   	PEMBAYARAN_TIKET_PESAWAT,
    PEMBAYARAN_PDAM,
    PEMBAYARAN_TELKOM,
    PEMBAYARAN_DPLKR,
    PEMBAYARAN_BRIVA,
    /*End Of Changed By Ali*/
    PEMBAYARAN_PLN_PASCA,
   	PEMBAYARAN_PLN_PRA,
   	PEMBAYARAN_PLN_TOKEN,
    PEMBAYARAN_CICILAN,
    PEMBAYARAN_SIMPATI,
    PEMBAYARAN_MENTARI,
    PEMBAYARAN_IM3,
    PEMBAYARAN_XL,
    PEMBAYARAN_ESIA,
    PEMBAYARAN_SMART,
    PEMBAYARAN_FREN,
    PEMBAYARAN_AXIS,
    PEMBAYARAN_THREE,
    PEMBAYARAN_KK_BRI,
    PEMBAYARAN_KK_CITI,
    PEMBAYARAN_KK_STANCHAR,
    PEMBAYARAN_KK_HSBC,
    PEMBAYARAN_KK_RBS,
    PEMBAYARAN_KK_ANZ,
    PEMBAYARAN_ZAKAT,
    PEMBAYARAN_INFAQ,

    PEMBAYARAN_HALO,
    PEMBAYARAN_MATRIX,

    //tunai
    TARIK_TUNAI,
    VOID_TARIK_TUNAI,
    SETOR_SIMPANAN,
    SETOR_PINJAMAN,
	/*Changed By Ali*/
	TRX_TRANSFER,
	TRX_PULSA,
	PEMBAYARAN_SPP,
	TRX_SETOR_PASTI,
	/*End Of Changed By Ali*/

   TCASH,
#ifdef PREPAID
    PREPAID_PAYMENT,
    PREPAID_PAYMENT_DISC,
    PREPAID_VOID, //++@agmr - brizzi2
    TOPUP_DEPOSIT,
    TOPUP_ONLINE,
    AKTIVASI_DEPOSIT,
    PREPAID_REDEEM,
    //++@agmr - brizzi2
    PREPAID_REAKTIVASI,
//--@agmr - brizzi2
#endif

    tTotalEnd
};
//}tTotalIdx;


#define CICILAN_FIF 0
#define CICILAN_BAF 1
#define CICILAN_OTO 2
#define CICILAN_FNS 3
#define CICILAN_VER 4
#define CICILAN_WOM 5

/*Changed By Ali*/
#define TV_INDOVISION 	0
#define TV_TOPTV		1
#define TV_OKTV			2
#define PEMB_TELKOM     3
#define PEMB_PDAMSBY    4

/*End Of Changed By Ali*/

#define TIKET_GARUDA 			0
#define TIKET_LION_AIR 			1
#define TIKET_SRIWIJAYA_AIR 	2
#define TIKET_MANDALA 			3

#ifdef ABSENSI
/*Changed By Ali*/
#define SHIFTIA     0
#define SHIFTIB     1
#define SHIFTIIA    2
#define SHIFTIIB    3
#define SHIFTIIIA   4
#define SHIFTIIIB   5
#define DATANG		6
#define PULANG		7
#define ISTIMUL		8
#define ISTIAKH		9
#define LEMTANG		10
#define LEMLANG		11
/*End Of Changed By Ali*/
#endif

#define SIMPATI_50      10
#define SIMPATI_100     11
#define SIMPATI_150     12
#define SIMPATI_200     13
#define SIMPATI_300     14
#define SIMPATI_500     15
#define SIMPATI_1000    16

#define MENTARI_25      20
#define MENTARI_50      21
#define MENTARI_100     22
#define MENTARI_250     23
#define MENTARI_500     24
#define MENTARI_1000    25

#define IM3_25      31
#define IM3_50      32
#define IM3_75      33
#define IM3_100     34
#define IM3_150     35
#define IM3_200     36
#define IM3_500     37
#define IM3_1000    38

#define XL_25       41
#define XL_50       42
#define XL_75		43
#define XL_100      44
#define XL_150      45
#define XL_200      46
#define XL_300      47
#define XL_500      48

#define ESIA_25     50
#define ESIA_50     51
#define ESIA_75	 	52
#define ESIA_100    53
#define ESIA_150    54
#define ESIA_200    55

#define SMART_20    60
#define SMART_25    64
#define SMART_50    61
#define SMART_100   62
#define SMART_150   65
#define SMART_200   63
#define SMART_300   66
#define SMART_500   67


#define FREN_20     70
#define FREN_25     71
#define FREN_50 	72
#define FREN_100    73
#define FREN_150    74
#define FREN_200    75
#define FREN_300 	76
#define FREN_500    77

#define THREE_20    80
#define THREE_50    81
#define THREE_75    82
#define THREE_100   83
#define THREE_150   84
#define THREE_300 	85
#define THREE_500   86

#define AXIS_25		91
#define AXIS_50		92
#define AXIS_75		93
#define AXIS_100	94
#define AXIS_150	95
#define AXIS_200	96
#define AXIS_300	97

#define ZAKAT_DHUAFA    51
#define ZAKAT_YBM       52

#define INFAQ_DHUAFA    61
#define INFAQ_YBM       62

#define DPLK_DHUAFA     71
#define DPLK_YBM        72

#define PASCA_BAYAR_HALO 1
#define PASCA_BAYAR_MATRIX 2

#define REK_TABUNGAN 1
#define REK_GIRO     2

#define INQUIRY_AMT_LEN 8
#define NAMA_NASABAH_LEN 30

#define SIMABARU 	1
#define SIMAUBARU 	2
#define SIMB1BARU 	3
#define SIMB1UBARU	4
#define SIMB2BARU	5
#define SIMB2UBARU	6
#define SIMCBARU	7
#define SIMDBARU	8
#define SIMPA		9
#define SIMPAU		10
#define SIMPB1		11
#define SIMPB1U		12
#define SIMPB2		13
#define SIMPB2U		14
#define SIMPC		15
#define SIMPD		16
#define SIMPNAU		17
#define SIMPNB1		18
#define SIMPNB1U	19
#define SIMPNB2		20
#define SIMPNB2U	21
#define SIMPUA		22
#define SIMPUAU		22
#define SIMPUB1		23
#define SIMPUB1U	24
#define SIMPUB2		25
#define SIMPUB2U	26
#define SIMAGOLSAM	27
#define SIMAUGOLSAM 28
#define SIMB1GOLSAM 29
#define SIMB1UGOLSAM 30
#define SIMB2GOLSAM 31
#define SIMB2UGOLSAM 32
#define SIMCGOLSAM 	33
#define SIMDGOLSAM 	34
#define SIMAUMP		35
#define SIMB1MP		36
#define SIMB1UMP	37
#define SIMB2MP		38
#define SIMB2UMP	39
#define SIMAMPU		70
#define SIMAUMPU	40
#define SIMB1MPU	41
#define SIMB1UMPU	42
#define SIMB2MPU	43
#define SIMB2UMPU	44
#define SIMAH		45
#define SIMAUH		46
#define SIMB1H		47
#define SIMB1UH		48
#define SIMB2H		49
#define SIMB2UH		50
#define SIMCH		51
#define SIMDH		52
#define SIMAUHP		53
#define SIMB1HP		54
#define SIMB1UHP	55
#define SIMB2HP		56
#define SIMB2UHP	57
#define SIMAUHPU	58
#define SIMAHPU		71
#define SIMB1HPU	59
#define SIMB1UHPU	60
#define SIMB2HPU	61
#define UJISIM		62
#define BPKBR2B		63
#define BPKBR4B		64
#define BPKBR2G		65
#define BPKBR4G		66
#define STCK			67
#define MRR2			68
#define MRR4			69

//++ BRI report
#define CARD_SERVICE_LOG    1
#define MINI_ATM_LOG        2
#define TUNAI_LOG           3
#define NORMAL_LOG          4
#define BRIZZI_LOG          5
#define TCASH_LOG          6

#define CREDIT_CARD 1
#define DEBIT_CARD  2
//#define NON_BRI_CARD  3 //NON BRI CARD
#define NON_BRI_DEBIT_CARD  3
#define NON_BRI_CREDIT_CARD 4

#define MAX_TAGIHAN_PLN 4
#define MAX_PREPAID_SETTLEMENT 15

typedef struct
{
    char totAmt[lenTotAmt + 1];
    word count;
    char transName[21];
} TOTAL_T;
//-- BRI report


typedef struct
{
    char tid[9];
    byte acqIdx;
    char acqName[lenName+1];
} ACQ_T;

void hapusAwalan(char* str,char c);
void trimRight(char* buf, byte chr);
void trimLeft(char* buf, byte chr);
char* convertBulan(byte jenis, char* bulanHuruf, byte bulanAngka);

char scrollDisplay(char (*msg)[21],byte jumlahMsg, byte jumlahBaris,char* atas, char* bawah);
char scrollDisplay8(char (*msg)[22],byte jumlahMsg, byte jumlahBaris,char* atas, char* bawah);
int displayLongText(char *txtInput, int txtLen, char* baris1, char* baris2, int maxKolom);

int parseField48(const byte* val, word len);
int parseField60(const byte * val, word len);
int parseField61(const byte* val, word len);
int parseField63(const byte* val, word len);
int parseField13(const byte* val, word len);

int pmtBRIKonfirmasiTanpaInquiry(byte txnType);
int pmtBRIKonfirmasiSesudahInquiry();
int pmtNewPinInput(void);
int pmtBRICustomInput();

int inputInfoSaldoBankLain();
int inputTCash();
int inputTransferSesamaBRI();
int inputTransferAntarBank();
int inputPembayaranPLN();
int inputPembayaranPLNPra();
int inputTiketPesawat();
int inputPembayaranPLNToken();
int inputPembayaranCicilan();
int inputPembayaranPulsa();
int inputSetoran(byte txnType);
int inputTarikTunai();
int inputPembayaranZakat();
int inputSetorPasti();
int inputInstallment();
int inputPembayaranPascaBayar();
int inputReissuePIN();
int inputGantiPasswordSPV();
int inputInternetBankingPin();
//int inputAktivasiKartu();
//int inputRenewalKartu();
/*Changed By Ali*/
int inputPembayaranTV();
int inputPembayaranTelkom();
int inputPembayaranSSB();
int inputPembayaranDPLKR();
int inputSmsBanking();
int inputTrxTransfer();
int inputTrxPulsa();
int inputPembayaranKKBRI();
int inputPembayaranKKANZ();
int inputPembayaranKKCITI();
int inputPembayaranKKHSBC();
int inputPembayaranKKRBS();
int inputPembayaranKKStanChar();
int inputPembayaranSPP();

int KonfirmasiMinInput();
/*End Of Changed By Ali*/
int KonfirmasiAktivasiKartu();
//int KonfirmasiRenewalKartu();
//nt KonfirmasiReissuePIN();
int KonfirmasiPembayaranPulsa();
int KonfirmasiPembayaranCicilan();
int KonfirmasiPembayaranCicilan8();
int KonfirmasiSetoran(byte txnType);
int KonfirmasiTarikTunai();
int KonfirmasiPembayaranZakat();
int KonfirmasiSetorPasti();
int KonfirmasiRegistrasiInternetBanking();
int KonfirmasiTCash();
int KonfirmasiRegistrasiPhoneBanking();
int KonfirmasiKartuOtorisasi();
int KonfirmasiPembayaranPLN();
int KonfirmasiPembayaranPLNPra();
int KonfirmasiPembayaranBriva();
int KonfirmasiPembayaranBriva2();

/*Changed By Ali*/
int KonfirmasiPembayaranSPP();
int KonfirmasiPembayaranTV();
int KonfirmasiPembayaranTelkom();
int KonfirmasiPembayaranSSB();
int KonfirmasiPembayaranDPLKR();
int KonfirmasiRegistrasiSmsBanking();
int KonfirmasiTrxTransfer();
int KonfirmasiTrxPulsa();
int KonfirmasiPembayaranKKBRI();
int KonfirmasiPembayaranKKANZ();
int KonfirmasiPembayaranKKCITI();
int KonfirmasiPembayaranKKHSBC();
int KonfirmasiPembayaranKKRBS();
int KonfirmasiPembayaranKKStanChar();
int KonfirmasiPembayaranKKBRIBayar();
int KonfirmasiPembayaranTiketPesawat();
#ifdef _USE_ECR
int KonfirmasiPembayaranECR();
int POSPrintReferenceNum(void);
int KonfirmasiShowOption(void);
int KonfirmasiSetMQPSVEPS(void);
int KonfirmasiSetPIN(void);
int magStripeInput(void);
int smartCardInput(void);
int prepaidInput(void);
#endif
/*End Of Changed By Ali*/
#ifdef ABSENSI
int KonfirmasiAbsensi();
#endif
int konfirmasiSettingTIDMID();
int printVersion();
int KonfirmasiTransferAntarBank();
int KonfirmasiTransferSesamaBRI();
int KonfirmasiInstallment();
int KonfirmasiPembayaranPascaBayar();
int KonfirmasiInfoSaldo8();
int KonfirmasiInfoSaldoLain8();
int KonfirmasiPembayaranPascaBayar8();
int KonfirmasiVoidTarikTunai();

int isBRIPrint(void);
int isBRITransaction (void);

void functionMenu();

int getField48(tBuffer *req);
int getIso57(tBuffer * req);
int getField61(tBuffer *req);
int getField60(tBuffer *req);
int getField63(tBuffer *req);

int isBRIInquiryTransaction(void);
int inputInfoSaldoBankLain(void);
int inputPreAuth();
int isBRITransaction(void);
int isBRIPaymentTransaction(void);
int isBRISaveToBatch(void);
int doBRIReversal(void);
int whichBRITransaction (void); //@agmr - BRI1
int isPembelianTransaction (void);

int briPrintData(word txnType, word *rlo, byte *isOneReceipt,byte isDuplicate); //BRI2
int briGetAcq(byte cardType, byte* acqId); //@agmr - BRI3
#ifndef REVERSAL_TERPISAH
int getBriReversalIdx();
int setBriReversalIdx();
#endif
int getJenisLog();
int parseField57(const byte* val, word len); //@@AS-SIMAS
int parseField62(const byte* val, word len); //@@AS0.0.26
int resetLogonDateTime(); //AS0.0.26
int setFirstPowerOnFlag(byte flag_value);//AS0.0.26
#ifdef _USE_ECR
int setECRFlag(byte flag_value);
int getInfoEcr(byte flag_value, char* infoEcr);
#endif //_USE_ECR
int strFormatDatTimBRI2(char* datTim, char* fmtDatTim);

extern int infoKodeBank(void);
extern int infoKodeUniv(void);

extern void hapusAwalan(char* str,char c);
extern void trimRight(char* buf, byte chr);
extern void trimLeft(char* buf, byte chr);

extern char scrollDisplay(char (*msg)[21],byte jumlahMsg, byte jumlahBaris,char* atas, char* bawah);
extern char scrollDisplay8(char (*msg)[22],byte jumlahMsg, byte jumlahBaris,char* atas, char* bawah);

extern int parseField48(const byte* val, word len);
extern int parseField60(const byte * val, word len);
extern int parseField13(const byte* val, word len);

extern int inputRedeem();
extern int pmtBRIKonfirmasiTanpaInquiry(byte txnType);
extern int pmtBRIKonfirmasiSesudahInquiry();
extern int inputInfoSaldoBankLain();
extern int inputPreAuth();
extern int inputTCash();
extern int inputTransferSesamaBRI();
extern int inputTransferAntarBank();
extern int inputPembayaranPLN();
extern int inputPembayaranPLNPra();
extern int inputTiketPesawat();
extern int inputPembayaranPLNToken();
extern int inputPembayaranDPLKR();
extern int inputPembayaranBriva();
extern int inputPembayaranPdam();
extern int inputPembayaranCicilan();
extern int inputPembayaranPulsa();
extern int inputSetoran(byte txnType);
extern int inputTarikTunai();
extern int inputPembayaranZakat();
extern int inputSetorPasti();
extern int inputInstallment();
extern int inputPembayaranPascaBayar();
extern int KonfirmasiPembayaranPulsa();
extern int KonfirmasiPembayaranCicilan();
extern int KonfirmasiPembayaranCicilan8();
extern int KonfirmasiSetoran(byte txnType);
extern int KonfirmasiTarikTunai();
extern int KonfirmasiPembayaranZakat();
extern int KonfirmasiSetorPasti();
extern int KonfirmasiRegistrasiInternetBanking();
extern int KonfirmasiTCash();
extern int KonfirmasiRegistrasiPhoneBanking();
extern int KonfirmasiKartuOtorisasi();
extern int KonfirmasiPembayaranPdam();
#ifdef PREPAID
extern int inputPrepaidDeposit();
extern word uploadedPrepaidIdx[MAX_PREPAID_SETTLEMENT+1];

#endif

extern int getField48(tBuffer *req);
extern int getIso57(tBuffer * req);
extern int getField61(tBuffer *req);
extern int getField63(tBuffer *req);

extern int openCom(void);
extern int closeCom(void);
extern int downloadKey(byte* msg, int msgLen);

int display_text(int line, int y, char *text, int alignment);

//++ BRI report
extern int briLogSave(byte jenisLog);
extern int briLogUpdate(byte jenisLog);
extern int briLogLoadByRoc(byte jenisLog);
extern int briLogLoadByIndex(byte jenisLog, card index);
extern int briLogLoadByDate(byte jenisLog, char *dateTime);

extern int fmtAmtReceipt2(word key, char *amt); //BRI2

int viewBatch1(byte jenisLog,byte flag);
int viewBatch2(byte jenisLog,byte flag);
//int viewBatch3(byte jenisLog);
void briViewBat(byte jenisLog, byte posisiLog);
extern void briViewBat(byte jenisLog, byte posisiLog);

//++BRI2
void getBriTransTypeName(word *txnType, char *transName, word key); //BRI2
extern void getBriTransTypeName(word *txnType, char *transName, word key); //BRI2

int briLogLoadCurContext(byte jenisLog);
int briLogSaveCurContext(byte jenisLog);
void briLogPrintTxn(byte jenisLog);
void briLogDuplicata(byte jenisLog);
//--BRI2

//void getBriTransName(char *transName, word key);
//extern void getBriTransName(char *transName, word key);

int briLogCalcTotal(byte jenisLog, TOTAL_T *total, char* dateTime);
int briLogPrintTotal(byte jenisLog,char* dateTime);
extern int briLogPrintTotal(byte jenisLog,char* dateTime);
int briLogPrintDetail(byte jenisLog, char* dateTime);
void briPrintDetail(byte jenisLog);
void briPrintDetailToday(byte jenisLog);
void briPrintTotal(byte jenisLog);
void briPrintTotalToday(byte jenisLog);
//-- BRI report

int getAllTid(ACQ_T* acq);
int getAllAcq(ACQ_T* acq);
int selectAcquirer();
int selectAcquirer1();
int selectTerminalId(ACQ_T *acq, byte* jumlahTid);
void pmtEchoTest(void);

void tCashLogReset(void);
void cLogReset(void);
void mLogReset(void);
void tLogReset(void);
void viewCardServiceLog(void);
void viewMiniAtmLog(void);
void viewTunaiLog(void);

void viewTCashLog();

int checkPendingSettlement();

void ShowData(byte* txt, word len, int mode, int awalKolom, int banyakKolom);

void isIct220();

byte OptionEnabled(byte idx, byte optflag); //@@AS0.0.29
void modifyMnuLayout(void); //@@AS0.0.29
void modifySubMnuLayout(void);
void checkSubMnuLayout(void);
int adaTransPembelian();

#ifdef TEST_BRI
void testBacaNvmRoc();
void testBacaNvmDate();
void testBacaNvmIndex();
void testTulisNvm();
void testBacaLogSemua();
void testPrintTotal();
void testPrintDetail();
void testPrintTotalHariSemua();
void testPrintTotalHariIni();
void testReprintLastMiniAtm();
void testReprintMiniAtm();
#endif

#ifdef DEBUG_PREPAID_DATA
void setDebugData();
#endif

#ifdef DUMP_ISO_MESSAGE
extern void setDumpIsoMessage(void);
#endif

#ifdef BRIZI_DISCOUNT
extern void setDiscount(void);
#endif

int briModifyPinEntry();

#endif


