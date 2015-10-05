//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/iso.h $
//$Id: iso.h 1490 2009-02-05 09:39:05Z jelemia $

#ifndef __ISO_H

/** \addtogroup loggroup
 * @{
 */

/** \addtogroup isogroup ISO ISO8583
 * ISO component describes the format of each field in the ISO8583 protocol used.
 * @{
 */
enum eIso {                     //ISO8583 fields
    isoBitBeg,
    isoBitMapSec,               //001 Bit Map, Secondary
    isoPan,                     //002 Primary Account Number (PAN)
    isoPrcCod,                  //003 Processing Code
    isoAmt,                     //004 Amount, Transaction
    iso005,                     //-005 not used
    iso006,                     //-006 not used
    isoDatTim,                  //-007 Transmission Date and Time
    isoMaxBuf,                  //-008 maximum buffer to be received by application from TMS
    iso009,                     //-009 not used
    iso010,                     //-010 not used
    isoSTAN,                    //011 System Trace Audit Number
    isoTim,                     //012 Transaction Time
    isoDat,                     //013 Transaction Date
    isoDatExp,                  //014 Date, Expiration
    iso015,                     //-015 not used
    iso016,                     //-016 not used
    iso017,                     //-017 not used
    iso018,                     //-018 not used
    iso019,                     //-019 not used
    iso020,                     //-020 not used
    iso021,                     //-021 not used
    isoPosEntMod,               //022 Point-of-Service Entry Mode
    isoCrdSeq,                  //023 Card Seq Number
    isoNII,                     //024 Network International Identifier
    isoPosCndCod,               //025 POS Condition Code
    isoBusCod,                  //-026 Card Acceptor Business Code
    iso027,                     //-027 not used
    isoRcnDat,                  //-028 Reconciliation Date
    iso029,                     //-029 not used
    isoOrgAmt,                  //-030 Original Amount
    iso031,                     //-031 not used
    isoAcqIIC,                  //-032 Acquirer Institution Identification Code
    iso033,                     //-033 not used
    iso034,                     //-034 not used
    isoTrk2,                    //035 Track 2 Data
    iso036,                     //-036 not used
    isoRrn,                     //037 Retrieval Reference Number
    isoAutCod,                  //038 Authorization Identification Response
    isoRspCod,                  //039 Response Code
    iso040,                     //-040 not used
    isoTid,                     //041 Card Acceptor Terminal Identification
    isoMid,                     //042 Card Acceptor Identification Code
    iso043,                     //-043 not used in SAMA
    iso044,                     //-044 not used in SAMA
    iso045,                     //-045 not used in SAMA
    iso046,                     //-046 not used in SAMA
    isoSpnsrId,                 //-047 Private Card Scheme Sponser ID
    isoPinMacKey,               //048 Pin/MAC session keys
    isoCur,                     //049 Currency Code, Transaction
    isoCurStl,                  //-050 Currency Code, Settlement
    iso051,                     //-051 not used in SAMA
    isoPinDat,                  //052 Personal Identification Number (PIN) Data
    isoSecCtl,                  //053 Security Related Control Information
    isoAddAmt,                  //054 Additional Amounts
    isoEmvPds,                  //-055 ICC System related data
    isoOrgnlDatElmnt,           //-056 Original Data Elements
    iso057,                     //-057 not used
    iso058,                     //-058 not used
    iso059,                     //-059 not used
    isoBatNum,                  //060 Batch Number
    isoRsvPvt = isoBatNum,      //060 Reserved Private (Can be used depending on the need)
    isoAmtOrg,                  //061 Amount original transaction
    //isoTot,                   //062 Reconciliation Totals
    isoRoc,                     //062 ROC-Trace No
    isoAddDat,                  //063 Additional Data
    isoMac,                     //064 Message Authentication Code (MAC)
    isoBitEnd
};

int isoFmt(byte bit);

/** @} */
/** @} */

#endif

#define __ISO_H
