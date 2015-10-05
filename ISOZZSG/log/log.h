//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/log.h $
//$Id: log.h 1944 2009-04-28 10:40:17Z ajloreto $

/// \file

#ifndef __LOG_H

/** \defgroup loggroup LOG Logical components
 *
 * The logical component functions are declared in the header file log.h in log directory.
 * The iso8583 protocol is defined in iso.h and described in iso.c.
 * They are implemented in the files:
 *  - HDL.C: entry point of all external events
 *  - VAL.C: validation functions
 *  - PMT.C: payment transaction flow treatment
 *  - ADM.C: various administrative operations: terminal ID input, communications settings, parameters download etc
 *  - ONL.C: online authorization, batch upload, hot list and parameters download treatment, connection scenarios
 *  - TLE.C: low level protocol implementation including message envelopes, special character processing, timeouts etc
 *  - REQ.C: request building according to the host protocol specification
 *  - RSP.C: response parsing according to the host protocol specification
 *  - LOG.C: transaction log processing, settlement, totals, duplicata etc
 *  - EMV.C: emv application select and transaction flow implementation
 *  - ISO.C: iso8583 fields format definition
 *
 * @{
 */

#include "str.h"


/** \addtogroup hdlgroup HDL HanDLer
 * HDL component contains the functions processing the messages coming from master application.
 * It is the entry point component of the UTA.
 * @{
 */
void hdlBefore(void);           ///<called before processing any event
void hdlAfter(void);            ///<called after processing any event
void hdlPowerOn(void);          //POWER ON even handler
void hdlMainMenu(void);         ///<main menu
int hdlTimer(card id, card * date, card * time);    ///<scheduled wakeup call event
void hdlWakeUp(void);           ///<this function is called by hdlTimer() once per minute in the default configuration
int hdlAskMagStripe(const char *buf);   ///<asks whether the card swiped is treated by the application
void hdlMagStripe(const char *buf); ///<process magnetic stripe card if validated by hdlAskMagStripe()
void hdlSmartCard(const byte * buf);    ///<process EMV card inserted

/** @} */

/** \addtogroup valgroup VAL VALidate
 * VAL component contains the functions that verify the validity of input data,
 * make decisions on whether to continue and retrieve parameters concerning the rtransaction flow
 * @{
 */
int valOpr(void);               ///<validate operator
int valAmount(void);            ///<validate amount
int valCard(void);              ///<validate card
int valIsPinRequired(void);     ///<is PIN required?
int valLogIsNotFull(void);      ///<is transaction log full?
int valDialOpt(void);           ///<validates TMS dial option
int valBaudRate(void);          ///<validates TMS baud rate
int valIsPrintRequired(void);   ///<validates printing requirement
int valIsTipAllowed(void);      ///<is tip allowed?
int valBufSize(void);           ///<validates TMS buffer size
int valAppliIsInit(void);       ///<is application initialised?
int valNoPendingSettlement(void);   ///<checks for pending settlement
int valTxnFound(void);          ///<checks if transaction exists in the batch
int valPwd(void);               ///<checks password
int valManualEntryAllowed(void);    ///<is manual entry allowed?
int valMagAllowed(void);        ///<is magstripe allowed?
int valChipAllowed(void);       ///<is chip allowed?
int valFloorLimit(void);        ///<checks for floor limit
int valAskPwd(word key);        ///<asks & checks for password
int valAskPin(byte *outData); //@@SIMAS-SW_PIN
int valRspCod(void);            ///<is transaction approved?
int valConnSett(void);          ///<validates Transaction Connection Setting
int valBin(void);               ///<validates binary range and sets cursor on the correct acq and iss records
byte valBinLogoAssign(const char *Pan); //<returns the logo id for depending on the PAN
int valIsTrickleFeedEnabled(void); //<checks if trickle feed is enabled in TMS

int valEMVFallbackAllowed(void);//@@SIMAS_FALLBACK

/** @} */

/** \addtogroup reqgroup REQ REQuest
 * REQ component contains the functions that builds requests.
 * @{
 */
int reqBuild(tBuffer * req);    ///<build authorization requests

/** @} */

/** \addtogroup rspgroup RSP ReSPonse
 * RSP component contains the functions that parse responses.
 * @{
 */
int rspParse(const byte * rsp, word len);   ///<parse authorization responses

/** @} */

/** \addtogroup onlgroup ONL ONLine
 * ONL component contains the functions that establish connection
 * @{
 */
int onlSession(void);           ///<connect to authorization server and perform communication session

/** @} */

/** \addtogroup tlegroup TLE Transport Layer Engine
 * TLE component contains the functions that manage send receive operations in communication session
 * @{
 */
int tleSession(char ses);       ///<manage message flow for authorization transaction
int tleSecond();
/** @} */

/** \addtogroup pmtgroup PMT PayMenT
 * PMT component contains the functions that implement financial transaction flow.
 * @{
 */
//int pmtReceiptData(word key, byte singleReceipt ); // @@OA
//int pmtReceipt2(word key); // @@OA
//int pmtPayment(byte mnuItem); //@agmr - BRI
//void pmtPayment(byte mnuItem);
int pmtPayment(word mnuItem); //@agmr - BRI
void pmtSale(void);             ///<sale transaction
void pmtSaleRedeem(void); ///<sale redeem transaction
void pmtPreaut(void);           ///<preauthorization transaction
void pmtAut(void);              ///@agmr - BRI5
void pmtRefund(void);           ///<refund transaction
void pmtOffline(void);          ///<offline transaction
void pmtVoid(void);             ///<void transaction
void pmtAdjust(void);           ///<adjust transaction
void pmtManual(void);           ///<manual input
void pmtBalance(void);			///<Balance enquiry

void pmtPreAuth(void);
void pmtPreCancel(void);
void pmtPreCom(void);

//void pmtSettlement(void);    ///<Settlement Transaction
int pmtAmountInput(void);       ///<Amount entry
int pmtPinInput(void);          ///<Pin entry (On-Line)
int pmtForceOnline(void);       ///<Force txn to go online
int pmtCardInput(void);         ///<card input process
int dmyPmtSmartCard(void);
int pmtTipInput(void);          ///<tip input process
int pmtIncrementInv(void);      ///<increments invoice number

// ++ @agmr - BRI
void pmtInfoSaldo(void);
//void pmtTarikTunai(void);
void pmtSetorSimpanan(void);
void pmtSetorPinjaman(void);
void pmtTransfer(void);
void pmtPembayaranTelkom(void);
void pmtPembayaranPLN(void);
void pmtPembayaranKK(void);
void pmtUnivTerbuka(void);
void pmtSimpati(void);
void pmtAktivasiKartu(void);
void pmtRenewalCard(void);
void pmtReissuePIN(void);
void pmtChangePIN(void);
void pmtInstallment(void);
void pmtInquiryTrans(void);
// -- @agmr - BRI

/** @} */

/** \addtogroup admgroup ADM ADMin
 * ADM component contains the functions that process administrator operations.
 * @{
 */
void admChangePwd(void);        ///<change password
void admReset(void);            ///<data base re-initialisation
void admRevReset(void);         ///<data base re-initialisation
void admRevResetWoKey(void);         ///<data base re-initialisation - @@SIMAS-INIT_DB
void admTCResetWoKey(void);          ///<data base re-initialisation - @@SIMAS-INIT_DB
void admInitRpt(void);          ///<initialization receipt
void admTermConfigM(void);      ///<terminal configuration
void admSWver(void);            ///<terminal sowftare version
void admViewTotals(void);       ///<view totals
void admViewRoc(void);          ///<view roc
void admSetRoc(void);           ///<set roc
void admViewBatchNum(void);     ///<view batch number
void admViewStan(void);         ///<view stan
void admSetStan(void);          ///<set stan
void admViewBat(void);          ///<view batch
void admTMSLoadParam(void);     ///<TMS load parameter
void admTMSViewConfig(void);    ///<TMS view config
void admTMSPrintConfig(void);   ///<TMS print config
void admTMSConfig(void);        ///<TMS configuration
void admTDT(void);              ///<date time
void admTermAppli(void);        ///<terminal application
void admAppConfig(void);        ///<application config
void admPlatformConfig(void);   ///<platform config
void admTermConfig(void);       ///<terminal config
void admDebug(void);            ///<debug
void admCmmViewSetting(void);   ///<communication view setting
void admCmmPrintSetting(void);  ///<communication print setting
void admCmmLineSetup(void);     ///<line setup
void admCmmIPSetup(void);       ///<IP setup
void admCmmEDHSetup(void);      ///<EDH setup
void admCmmCountryCode(void);   ///<country code modem setup
void admCmmDTMF(void);          ///<DTMF speed modem setup
void admCmmGConfigure(void);    ///<GPRS configuration
void admCmmGEncryption(void);   ///<GPRS encryption
void admCmmGAutoEnable(void);   ///<GPRS auto enable
void admMasterKey(void);        ///<RK, MK, WK load
void admViewDialup(void);       ///<View DialUp Connection
void admViewTCPIPGprs(byte type);   ///<View TCPIP Connection
void admViewRs232(void);        ///<View DialUp Connection
void admViewTxn(void);          ///<???
void admConnSettings(void);     ///<Transaction Connection Setting

/** @} */

/** \addtogroup trnloggroup TRNLOG transaction LOG processing
 * LOG component contains functions on transaction logs and hotlist processing
 *
 * @{
 */
void logReset(void);            ///<reset transaction log
int logSave(void);              ///<log save
//void logPrintTxn(void);         ///<print transaction receipt
//void logDuplicata(void);        ///<duplicate
void logPrintTxn(byte jenisLog);         ///<print transaction receipt
void logDuplicata(byte jenisLog);        ///<duplicate
void logSetDup(void);           ///<settlement duplicate
void logHostTotals(void);       ///<host totals
int logPrintLog(void);          ///<print log
void logBatchTotals(void);      ///<batch totals
int logIsInBlackList(void);     ///<black list
int logGetPreviousAmount(card * pulPrv);    ///<last txn amount
int logSaveCurContext(void);    ///<Save current data from context
int logLoadCurContext(void);    ///<Load context to current data before
int logLoad(void);              ///<load a transaction from transaction log into RAM
int logCalcTot(void);           ///<Calculate Totals in whole application
void logSettlement(void);       ///<settlement
void logSettlementRpt(void);    ///<settlement receipt
int logConfirmTotals(void);     ///<confirm totals sale / refund ; requested to be used again ( NB 1349 )

/** @} */

/** \addtogroup emvwgroup EMV transaction flow and application select
 * EMV component implements high level EMV transaction flow
 *
 * @{
 */
int emvInitAid(void);           ///<Send AID(s) list to EMV Kernel
int emvProcessing(void);        ///<EMV application select and transaction flow
int emvMapAidMove(const byte * pucAID); ///<Check AID selected is part of AID(s) list accepted by the application
int emvFullProcessCheck(void); // Check FULL EMV Process

/** @} */

/** \addtogroup revwgroup Reversal processing
 * Reversal transaction flow and processes
 *
 * @{
 */
int revSave(void);              ///<saves reversal data
int reversalTransaction(void);  ///<controls the reversal transaction flow

int tcSave(void);
/** @} */

/** @} */
//ADD FOR EMV MENU
int EMVFallback;
byte EMVFallbackFlagSet;
int EMVIdleFallback;
int EMVFlagSet;
int EMV_ENTRY_SERVICE_CODE;
int EMV_ENTRYMODE;
int EMV_FALLBACK_IDLE_SHOW;
int EMV_IDLE_SHOW_FLAG;
byte debugReqFlag;
byte debugRspFlag;
byte debugEmvFlag;

int display_text(int line, int y, char *text, int alignment);
char acceptable_key(int accept_key, int timeout);
int displayResponse(void);

int emvCallMultiCurr(void);
 void admBlowfish(void);
 void admEDCFunction(void);

void hex_dump_char(const char *pzTitle, const byte *buff, long len);
void call_function(int nbr_func);
int ValBfSett(void);
void BlockPan(char *origPan,char *blockedPan);
int pmtSaleFallback(void);
int CheckFallBack(int ret);
int strFormatTraDatTim(void);
int pmtFallback();
void FmtExpDat(char *dst, char *src);//For format expdat 20100820
void DisconnectFromHost();
void admTCReset(void);

int emvFullProcessSet(void);

#define ERRFALLBACK_RET		91
#define CID_GEN1 40
#define CID_GEN2 41

//++ @agmr - BRI
void pmtVoidTarikTunai(void);
void pmtBRI(void);
void pmtAbsen(void);
void printAllLog(void);
int checkTid(void);
int checkMid(void);
int checkStan(void);
int checkMti(void);
int disableSubmenu();

//-- @agmr - BRI
/* @@OA */ void FnCommunication(void);
/* @@OA */ void FnConnSetting(void);
/* @@OA */ void Fn99(void);
/* @@OA */ void FnTMS(void);
/* @@OA */ void FnTerminal(void);
#ifdef _USE_ECR
int FnSales(void);
int FnPrepaid(void);
int FnTopUp(void);
int FnPrintRef(void);
int FnshowOption(void);
int FnPrinthis(void);
#endif
void Fn100();
void FnDisableMenu();
void FnSetMQPSVEPS();
void FnSetPIN();
void FnSettingTID();
void cetakVersion();
void viewLog();

void printAllSetlleDuplicate();
void printAllLog();

//++ @agmr - multi settle
int briLogReset(byte acqId);
int markSettlementTrans(byte acqId);
int copyUnmarkToBatch();
int copyAllToBatch();
int copyAllToTemporaryBatch();
int batchSync();

int setLogIsUploaded();

int procPrepaidUpload(void);
//-- @agmr - multi settle

int onlConnect2(); //@agmr
void admSetBatchNum(void); //@agmr

#define __LOG_H
#endif
