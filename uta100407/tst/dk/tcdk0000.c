
#include <string.h>
#include <stdio.h>

#include "Sys.h"
#include "tst.h"

#ifdef __EMV__

#include "emv.h"

#define MAX_RID_LENGTH      5
#define MAX_KEY_LENGTH      256
#define MAX_EXPONENT_LENGTH 5
#define MAX_HASH_LENGTH     20
#define MAX_PUBLIC_KEYS		6

typedef struct {
    byte Rid[MAX_RID_LENGTH];
    byte Index;
    byte KeyLength;
    byte KeyData[MAX_KEY_LENGTH];
    byte ExponentLength;
    byte ExponentData[MAX_EXPONENT_LENGTH];
    byte AlgorithmIndicator;
    byte HashLength;
    byte HashData[MAX_HASH_LENGTH];

} PublicKey_t;

// Init que
static int InitiateAppProcessing(tQueue * mQue) {

    VERIFY(mQue);

    quePutTlv(mQue, tagTrmCap, 3, (byte *) "\xE8\xF8\xC8"); //M.Terminal Capabilities
    quePutTlv(mQue, tagAddTrmCap, 5, (byte *) "\xe0\x00\xf0\xf0\x01");  //O.Additional Terminal Capabilities
    quePutTlv(mQue, tagAccCntCod, 2, (byte *) "\x00\x56");  //M.Card Acceptor Country Code
    quePutTlv(mQue, tagTrmId, 0, (byte *) "00000000");  //O.Terminal Identification
    quePutTlv(mQue, tagTrmTyp, 1, (byte *) "\x21"); //M.Terminal Type
    quePutTlv(mQue, tagTrnCurCod, 2, (byte *) "\x09\x78");  //Transaction Currency Code
    quePutTlv(mQue, tagTrnTyp, 1, (byte *) "\x00"); //Transaction Type
    quePutTlv(mQue, tagTrnDat, 3, (byte *) "\x04\x02\x03"); //M.Transaction Date
    quePutTlv(mQue, tagTrmAvn, 2, (byte *) "\x00\x8C"); //M.Application Version Number Terminal
    quePutTlv(mQue, tagPOSEntMod, 1, (byte *) "\x81");
    quePutTlv(mQue, tagTrmFlrLim, 4, (byte *) "\x00\x00\x27\x10");  //M.Terminal Floor Limit

    quePutTlv(mQue, tagThrVal, 4, (byte *) "\x00\x00\x00\x40"); //M.Threshold value
    quePutTlv(mQue, tagTarPer, 1, (byte *) "\x20"); //M.Target percentage
    quePutTlv(mQue, tagMaxTarPer, 1, (byte *) "\x50");  //M.Maximum target percentage
    quePutTlv(mQue, tagDftValDDOL, 15, (byte *) "\x9F\x02\x06\x5F\x2a\x02\x9a\x03\x9c\x01\x95\x05\x9f\x37\x04");    //M.Default value for DDOL

    quePutTlv(mQue, tagTrnTim, 3, (byte *) "\x09\x29\x12"); //O.Transaction Time
    quePutTlv(mQue, tagTrnCurExp, 1, (byte *) "\x02");  //O.Transaction Currency Exponent
    quePutTlv(mQue, tagTrnSeqCnt, 4, (byte *) "\x00\x00\x00\x02");  //O.Transaction Sequence Counter
    quePutTlv(mQue, tagMrcCatCod, 2, (byte *) "\x61\x11");  //O.Merchant Category Code

    quePutTlv(mQue, tagDftValTDOL, 3, (byte *) "\x9F\x08\x02"); //Default value for TDOL
    quePutTlv(mQue, tagAcqId, 5, (byte *) "\x00\x00\x47\x61\x73");  //O.Acquirer Identifier (BIN)
    quePutTlv(mQue, tagTrnRefCur, 2, (byte *) "\x00\x56");  //O.Transaction Reference Currency
    quePutTlv(mQue, tagTrnRefExp, 1, (byte *) "\x02");  //O.Transaction Reference Currency Exponent
    quePutTlv(mQue, tagTACDen, 5, (byte *) "\x00\x00\x00\x00\x00"); //Terminal Action Code Denial
    quePutTlv(mQue, tagTACOnl, 5, (byte *) "\x00\x00\x00\x00\x00"); //Terminal Action Code Online
    quePutTlv(mQue, tagTACDft, 5, (byte *) "\x00\x00\x00\x00\x00"); //Terminal Action Code Default

    quePutTlv(mQue, tagSchemasKSU, 4, (byte *) "\x01\x00\x00\x00");
    quePutTlv(mQue, tagIFDSerNum, 0, (byte *) "10812267");

    quePutTlv(mQue, 0x9F841E, 1, (byte *) "\x00");  //TAG_CUST_TRM_OVERPASS_AIP
    quePutTlv(mQue, tagAid, 7, (byte *) "\0xa0\x00\x00\x00\x03\x10\x10");   //TAG_AID_ICC
    quePutTlv(mQue, 0x9F8140, 8, (byte *) "\x07\0xa0\x00\x00\x00\x03\x10\x10"); //TAG_AID_ICC
    quePutTlv(mQue, tagMrcID, 8, (byte *) "\x01\x02\x00\x00\x00\x00\x00\x00");  //TAG_MERCHANT_IDENTIFIER

    quePutTlv(mQue, tagAmtBin, 4, (byte *) "\x00\x00\x27\x10");
    quePutTlv(mQue, tagAmtNum, 6, (byte *) "\x00\x00\x00\x02\x00\x00");
    quePutTlv(mQue, tagAmtOthBin, 4, (byte *) "\x00\x00\x00\x00");
    quePutTlv(mQue, tagAmtOthNum, 6, (byte *) "\x00\x00\x00\x00\x00\x00");

    quePutTlv(mQue, tagPinPadOK, 1, (byte *) "\x01");   // TAG_PP_OK

    return 0;
}

static byte gKey[] = {
    0xab, 0x79, 0xfc, 0xc9, 0x52, 0x08, 0x96, 0x96,
    0x7e, 0x77, 0x6e, 0x64, 0x44, 0x4e, 0x5d, 0xcd,
    0xd6, 0xe1, 0x36, 0x11, 0x87, 0x4f, 0x39, 0x85,
    0x72, 0x25, 0x20, 0x42, 0x52, 0x95, 0xee, 0xa4,
    0xbd, 0x0c, 0x27, 0x81, 0xde, 0x7f, 0x31, 0xcd,
    0x3d, 0x04, 0x1f, 0x56, 0x5f, 0x74, 0x73, 0x06,
    0xee, 0xd6, 0x29, 0x54, 0xb1, 0x7e, 0xda, 0xba,
    0x3a, 0x6c, 0x5b, 0x85, 0xa1, 0xde, 0x1b, 0xeb,
    0x9a, 0x34, 0x14, 0x1a, 0xf3, 0x8f, 0xcf, 0x82,
    0x79, 0xc9, 0xde, 0xa0, 0xd5, 0xa6, 0x71, 0x0d,
    0x08, 0xdb, 0x41, 0x24, 0xf0, 0x41, 0x94, 0x55,
    0x87, 0xe2, 0x03, 0x59, 0xba, 0xb4, 0x7b, 0x75,
    0x75, 0xad, 0x94, 0x26, 0x2d, 0x4b, 0x25, 0xF2,
    0x64, 0xaf, 0x33, 0xde, 0xdc, 0xf2, 0x8e, 0x09,
    0x61, 0x5e, 0x93, 0x7d, 0xe3, 0x2e, 0xdc, 0x03,
    0xc5, 0x44, 0x45, 0xfe, 0x7e, 0x38, 0x27, 0x77
};

typedef struct {
    char TVR_Str[128];
    byte TSI_Str[128];
    byte Sign_Str[128];
    byte CVMRes[128];

    byte RSAKeyIdx;
    byte Signature;
    byte StatCVP;
    byte RemainPin;
} Results;

Results gResults;

static int updateResVal(tQueue * queV) {
    card tag;
    word len;
    int ret;
    byte buff[256];
    byte tmp[256];

    VERIFY(queV);

    queRewind(queV);

    while(queLen(queV)) {
        memset(buff, 0, 256);
        memset(tmp, 0, 256);

        ret = queGetTlv(queV, &tag, &len, buff);
        VERIFY(ret == len);
        VERIFY(len <= 256);

        switch (tag) {
          case tagTVR:
              ret = bin2hex(gResults.TVR_Str, buff, (byte) ret);
              break;
          case tagTSI:
              ret = bin2hex(gResults.TSI_Str, buff, (byte) ret);
              break;
          case tagCVMRes:
              ret = bin2hex(gResults.TSI_Str, buff, (byte) ret);
              break;
          case tagCapkIdx:
              memcpy(&gResults.RSAKeyIdx, buff, 1);
              break;
          case tagSignature:
              memcpy(&gResults.Signature, buff, 1);
              break;
          case tagStatusCVP:
              memcpy(&gResults.StatCVP, buff, 1);
              break;
          case tagRemainPin:
              memcpy(&gResults.RemainPin, buff, 1);
              break;

          default:
              break;
        }
    }
    return 1;
}

static int OffCVM(byte lastTry) {
    int ret = -1;
    int key;

    char msg[dspW + 1 + dspW + 1 + 1];

    trcS("OffCVM: Beg\n");
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    dspStop();                  //close channel to give cryptomodule access to HMI
    strcpy(msg, "PLEASE ENTER\nPIN:\n\n");

    crySetPpd("0124C*301030");
    //         1204C2301060
    ret = cryEmvPin(msg);
    key = ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);     //now we can open HMI again

    ret = key;
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return ret;
  lblEnd:
    cryStop('m');
    trcFN("OffCVM: ret=%d\n", ret);
    return ret;
}

// print Result (TVR, TSI)
static void printResult() {
    byte bFallBack;

    prtS("95. TVR:");
    prtS(gResults.TVR_Str);

    prtS("9B. TSI:");
    prtS(gResults.TSI_Str);

    prtS("9F43. CVMRes:");
    prtS(gResults.CVMRes);

    if(gResults.Signature)
        prtS("Signature required");

    bFallBack = emvGetFallBack();
    if(bFallBack)
        prtS("Request for FallBack");
}

static int CVMProcessing(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA,
                         tQueue * qTKA) {

    int ret;
    byte idx, i;
    byte off;
    int opt = 0;

    card tak[] =
        { tagStatusCVP, tagTVR, tagCVMRes, tagRemainPin, tagSignature, 0 };

    idx = 0;
    off = 0;

    VERIFY(qVAK);
    VERIFY(qTAK);
    VERIFY(qVKA);
    VERIFY(qTKA);

    while(++idx) {

        queReset(qVKA);
        queReset(qTAK);

        i = 0;
        while(tak[i])
            quePutTag(qTAK, tak[i++]);  //Terminal Verification Result

        if(idx == 1) {
            VERIFY(!off)
                ret = emvCVMstart(qVAK, qTAK, qVKA);    //start CVM processing
        } else if(off) {
            ret = emvCVMoff(qTAK, qVKA);
        } else {
            ret = emvCVMnext(opt, qTAK, qVKA);
        }

        updateResVal(qVKA);
        CHECK(ret >= 0, lblKO);

        if(gResults.StatCVP == cvmINPUT_PIN_OK)
            if(off)
                dspLS(0, "PIN input OK");

        off = 0;

        if(gResults.StatCVP == cvmINPUT_PIN_OFF) {
            // input offline pin

            opt = 1;
            off = 1;

            if(gResults.RemainPin == 1) {
                dspLS(0, "Last PIN attempt");
                tmrPause(2);
            }
            ret = OffCVM(gResults.RemainPin);
            CHECK(ret > 0, lblKO);

            if(ret == kbdANN)
                goto lblPinByPass;
        }

        if(gResults.StatCVP == cvmINPUT_PIN_ON) {
            dspLS(0, "Online PIN");
            tmrPause(2);
            // need to add online pin input 
        }

        if(gResults.StatCVP == cvmNO_REMAINING_PIN) {
            dspLS(0, "No PIN remaining");
            tmrPause(4);
        }

        if(gResults.StatCVP == cvmNO_INPUT) {
            ret = 1;
            goto lblEnd;
        }
    }
    // *******************************************************************************************
  lblPinByPass:
    ret = 0;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void tcdk0000() {

    int ret = 0;
    int i;

    //DataElement datElem;
    enum { tSize = 256, vSize = 1024, MAX_AIDS = 4 };

    //char AIDNum = 3;
    //word ErrCode = 0;
    byte SelectedAID[128];

    unsigned char AIDS[MAX_AIDS][8] = {
        {0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10},
        {0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x20, 0x10},
        {0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x80, 0x10},
        {0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10},
    };

    //word wAPDURes = 0;
    char language[8 + 1];

    tQueue qTAK;
    tBuffer bTAK;
    byte dTAK[tSize];           //tags Application -> Kernel
    tQueue qTKA;
    tBuffer bTKA;
    byte dTKA[tSize];           //tags Kernel -> Application
    tQueue qVAK;
    tBuffer bVAK;
    byte dVAK[vSize];           //vals Application -> Kernel
    tQueue qVKA;
    tBuffer bVKA;
    byte dVKA[vSize];           //vals Kernel -> Application

    byte prnBuf[128];

    //word MnuItm;
    //byte aid[1 + 16];

    byte cry;

    enum { dSize = (16 + 4) * 16 };
    tQueue qAid;
    tBuffer bAid;
    byte dAid[dSize];

    memset(&gResults, 0, sizeof(gResults));

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    dspLS(0, "EMV Start");

    //  EMV Start
    ret = emvStart();
    CHECK(ret > 0, lblKO);

    bufInit(&bAid, dAid, dSize);
    queInit(&qAid, &bAid);

    for (i = 0; i < MAX_AIDS; i++)
        quePut(&qAid, AIDS[i] + 1, 7);

    // init AIDS supported by terminal
    ret = emvInit(&qAid);
    CHECK(ret > 0, lblKO);

    // select candidate list;   
    queReset(&qAid);
    dspClear();
    dspLS(0, "EMV Select");
    ret = emvSelect(&qAid);
    CHECK(ret > 0, lblKO);

    // Get first AID from queue of selected AIDS
    queRewind(&qAid);
    ret = queLen(&qAid);
    CHECK(ret > 0, lblKO);

    ret = queGet(&qAid, SelectedAID + 1);
    CHECK(ret > 0, lblKO);

    *SelectedAID = (byte) ret;

    // Final selection
    dspClear();
    dspLS(0, "Final Select");
    ret = emvFinalSelect((const byte *) SelectedAID);   // Check what AID we use
    CHECK(ret > 0, lblKO);

    // Get language
    ret = emvGetLang(language);
    CHECK(ret >= 0, lblKO);

// -------------------
// Initiate app processing  
// -------------------

    dspClear();
    dspLS(0, "Emv Context");

    bufInit(&bTAK, dTAK, tSize);
    queInit(&qTAK, &bTAK);
    bufInit(&bTKA, dTKA, tSize);
    queInit(&qTKA, &bTKA);
    bufInit(&bVAK, dVAK, vSize);
    queInit(&qVAK, &bVAK);
    bufInit(&bVKA, dVKA, vSize);
    queInit(&qVKA, &bVKA);

    InitiateAppProcessing(&qVAK);

    ret = emvContext(&qVAK, &qTKA);
    CHECK(ret > 0, lblKO);

    queReset(&qVAK);
    queReset(&qVKA);
    queReset(&qTKA);
    dspClear();
    dspLS(0, "Emv Prepare");
    // Request tags
    queReset(&qTAK);

    quePutTag(&qTAK, tagCapkIdx);   // RSA PUB KEY INDEX
    quePutTag(&qTAK, tagPAN);
    //quePutTag(&qTAK, 0x9F8140); // TAG_AID_PROPRIETARY
    quePutTag(&qTAK, tagPANSeq);    // 

    ret = emvPrepare(&qVAK, &qTAK, &qVKA, &qTKA);
    updateResVal(&qVKA);
    CHECK(ret > 0, lblKO);

// -------------------
// Offline data authentication  
// -------------------

    dspClear();
    dspLS(0, "Offline Data Auth.");

    bufInit(&bTAK, dTAK, tSize);
    queInit(&qTAK, &bTAK);
    bufInit(&bTKA, dTKA, tSize);
    queInit(&qTKA, &bTKA);
    bufInit(&bVAK, dVAK, vSize);
    queInit(&qVAK, &bVAK);
    bufInit(&bVKA, dVKA, vSize);
    queInit(&qVKA, &bVKA);

    //GetRSAKey(gResults.RSAKeyIdx, )

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTlv(&qVAK, tagRSAKey, sizeof(gKey), gKey);    //RSA key
    quePutTlv(&qVAK, tagRSAKeyExp, 0x01, (byte *) "\x03");  //Public Key Exponent
    quePutTlv(&qVAK, tagODA, 0x01, (byte *) "\x01");    //TAG_ODA_PROCESSING
    quePutTlv(&qVAK, tagDetFaitPriTAA, 0x01, (byte *) "\x01");  //TAG_DETECT_FAIL_PRIOR_TAA
    quePutTlv(&qVAK, tagCDANevReqARQ1, 0x01, (byte *) "\x00");  //TAG_CDA_NEVER_REQUEST_ARQC_1GENAC
    quePutTlv(&qVAK, tagCDANevReqONL2, 0x01, (byte *) "\x00");  //TAG_CDA_NEVER_REQUEST_ONLINE_2GENAC
    quePutTlv(&qVAK, tagHotLst, 0x02, (byte *) "\x00\x00");

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information

    ret = emvAuthenticate(&qVAK, &qTAK, &qVKA, &qTKA);
    updateResVal(&qVKA);
    CHECK(ret > 0, lblKO);

// -------------------
//  CVM Processing
// -------------------
    dspClear();
    dspLS(0, "CVM Process");

    bufInit(&bTAK, dTAK, tSize);
    queInit(&qTAK, &bTAK);
    bufInit(&bTKA, dTKA, tSize);
    queInit(&qTKA, &bTKA);
    bufInit(&bVAK, dVAK, vSize);
    queInit(&qVAK, &bVAK);
    bufInit(&bVKA, dVKA, vSize);
    queInit(&qVKA, &bVKA);

    quePutTlv(&qVAK, tagAmtBin, 4, (byte *) "\x00\x00\x00\x7B");    //Amount, Authorised (Binary), =123
    quePutTlv(&qVAK, tagTrnTyp, 1, (byte *) "\x00");

    ret = CVMProcessing(&qVAK, &qTAK, &qVKA, &qTKA);
    updateResVal(&qVKA);
    CHECK(ret > 0, lblKO);

// -------------------
// Terminal Risk Management
// -------------------

    dspClear();
    dspLS(0, "Emv Validate");
    bufInit(&bTAK, dTAK, tSize);
    queInit(&qTAK, &bTAK);
    bufInit(&bTKA, dTKA, tSize);
    queInit(&qTKA, &bTKA);
    bufInit(&bVAK, dVAK, vSize);
    queInit(&qVAK, &bVAK);
    bufInit(&bVKA, dVKA, vSize);
    queInit(&qVKA, &bVKA);

    quePutTlv(&qVAK, tagMaxTarPer, 1, (byte *) "\x50"); //TAG_MAX_TARG_PERC_FOR_BIASED_RAND_SEL
    quePutTlv(&qVAK, tagTarPer, 1, (byte *) "\x20");
    quePutTlv(&qVAK, tagThrVal, 4, (byte *) "\x00\x00\x00\x40");
    quePutTlv(&qVAK, tagTrmFlrLim, 4, (byte *) "\x00\x00\x27\x10"); //TAG_TERMINAL_FLOOR_LIMIT
    quePutTlv(&qVAK, 0x9F819F, 1, (byte *) "\x01"); //TAG_FLOOR_LIMIT_CHECKING
    quePutTlv(&qVAK, 0x9F81A0, 1, (byte *) "\x01"); //TAG_RANDOM_TRANSACTION_SELECT
    quePutTlv(&qVAK, 0x9F81A1, 1, (byte *) "\x01"); //TAG_VELOCITY_CHECKING
    quePutTlv(&qVAK, 0x9F81A3, 1, (byte *) "\x00"); //TAG_TRM_OVERPASS_AIP
    quePutTlv(&qVAK, tagAmtPrv, 4, (byte *) "\x00\x00\x00\x00");

    ret = emvValidate(&qVAK, &qTAK, &qVKA, &qTKA);
    updateResVal(&qVKA);
    CHECK(ret > 0, lblKO);

    // Terminal Action Analysis 
    dspClear();
    dspLS(0, "Action Analysis");
    bufInit(&bTAK, dTAK, tSize);
    queInit(&qTAK, &bTAK);
    bufInit(&bTKA, dTKA, tSize);
    queInit(&qTKA, &bTKA);
    bufInit(&bVAK, dVAK, vSize);
    queInit(&qVAK, &bVAK);
    bufInit(&bVKA, dVKA, vSize);
    queInit(&qVKA, &bVKA);

    quePutTlv(&qVAK, 0x9F8113, 1, (byte *) "\x01"); //TAG_EXCEPTION_FILE_RESULT
    quePutTlv(&qVAK, tagEMVDCOpt, 2, (byte *) "\x00\x00");  //TAG_ICC_EMV_DC_OPTION
    quePutTlv(&qVAK, 0x9f8148, 1, (byte *) "\x01"); //TAG_SKIP_TAC_IAC_DEFAULT
    quePutTlv(&qVAK, 0x9F819E, 1, (byte *) "\x01"); //TAG_SKIP_TAC_IAC_ONLINE

    quePutTag(&qTAK, tagCVMRes);
    quePutTag(&qTAK, tagIssTrnCrt);
    quePutTag(&qTAK, tagCID);
    quePutTag(&qTAK, tagIAD);
    quePutTag(&qTAK, tagUnpNum);
    quePutTag(&qTAK, tagATC);
    quePutTag(&qTAK, tagTVR);
    quePutTag(&qTAK, tagAIP);
    quePutTag(&qTAK, tagAid);
    quePutTag(&qTAK, tagPinCode);

    ret = emvAnalyse(&qVAK, &qTAK, &qVKA, &qTKA);
    updateResVal(&qVKA);
    CHECK(ret > 0, lblKO);

    // Go online and send back response code;

    dspClear();
    dspLS(0, "Emv Complete");
    dspLS(1, "done");
    bufInit(&bTAK, dTAK, tSize);
    queInit(&qTAK, &bTAK);
    bufInit(&bTKA, dTKA, tSize);
    queInit(&qTKA, &bTKA);
    bufInit(&bVAK, dVAK, vSize);
    queInit(&qVAK, &bVAK);
    bufInit(&bVKA, dVKA, vSize);
    queInit(&qVKA, &bVKA);

    quePutTlv(&qVAK, tagAuthRes, 1, (byte *) "\x01");   //Online process status: 0 declined, 1 accepted
    quePutTlv(&qVAK, tagRspCod, 2, (byte *) "00");  // Transaction response code
    quePutTlv(&qVAK, tagCommOk, 2, (byte *) "\x00\x01");    // Transaction response code
    quePutTlv(&qVAK, tagIssSc1, 6, (byte *) "\x80\xCA\x9F\x36\x00\x00");    // Issuer script 1
    quePutTlv(&qVAK, tagIssSc2, 6, (byte *) "\x80\xCA\x9F\x36\x00\x00");    // Transaction response code

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTag(&qTAK, tagCVMRes);    //Cardholder Verification Method (CVM) Results
    quePutTag(&qTAK, tagCID);   //Cryptogram Information Data (CID)
    quePutTag(&qTAK, tagScrRes);    //result of script execution
    quePutTag(&qTAK, tagIssTrnCrt); //Issuer Transaction Certificate
    quePutTag(&qTAK, tagIAD);   //Issuer Transaction Certificate
    quePutTag(&qTAK, tagRspCod);

    cry = 0;                    // this value in SAGEM is not used 
    ret = emvComplete(cry, &qVAK, &qTAK, &qVKA);
    updateResVal(&qVKA);
    CHECK(ret > 0, lblKO);

    // EMV Stop
    dspClear();
    dspLS(0, "EMV Stop");
    prtS("---\nTransaction complete");
    goto lblEnd;

  lblKO:
    prtS("---\nTransaction failed");
    //sprintf(prnBuf, "Last Err = %04X", GetEMVLastErr());
    prtS(prnBuf);

  lblEnd:

    printResult();
    prtS("\n\n");
    emvStop();
    prtStop();

}
#else
void tcdk0000() {
   
}
#endif
