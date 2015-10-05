/** \file
 * Cryptographic functions for UNICAPT SSA WorldWide
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/c32.c $
 *
 * $Id: c32.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "sys.h"
#ifdef __CRYSTD__
#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include <ssaStd.h>
#include <ssasec.h>

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcC32))

static uint32 hSsa = 0;
static char ssaMode = 0;

#ifdef __FONT__
int ssaStarted(void) {
    return ssaMode;
}
int ssaHandle(void) {
    VERIFY(ssaStarted());
    return hSsa;
}
#else
int ssaStarted(void) {
    return 0;
}
#endif

/** Open the associated channel.
 * Should be called before calling any  cryptomodule processing or before dspStart() function
 * if local fonts are used.
 * \param  mod (I) Can be 'm','p' or 's' or the same letters in uppercase.
 *    - The letters correspond to merchant, primary and secondary pinpad.
 *    - Uppercase letters are used to initiate local font downloading.
 *    - In this case the related version of SSA should be downloaded into terminal and the macro __FONT__ should be defined in def.h.
 *    - Usually cryStart(‘M’) is called before an external event treatment if local fonts are used;
 *      otherwise there is no need to call this function.
 *      The call cryStart(‘m’) is performed before any cryptography treatment.
 * \return
 *    - negative if failure.
 *    - non-negative if OK.
 * \remark
 *    - opens default hmi channel in immediate mode, return 1
 * \header sys\\sys.h
 * \source sys\\c16.c, sys\\c32.c
 * \test tcab0021.c
 */
int cryStart(char mod) {
    int ret;

    ret = ssaStarted();
    trcFN("ssaStarted = %d\n", ret);
    if(ret) {
        switch (mod) {
          case 'm':
              VERIFY(ssaMode == 'M');
              break;
          case 'p':
              VERIFY(ssaMode == 'P');
              break;
          default:
              VERIFY(mod == 's');
              VERIFY(ssaMode == 'S');
              break;
        }
        goto lblEnd;
    }
    switch (mod) {
      case 'M':
      case 'm':
          ssaMode = SSA_MERCHANT;
          trcS("SSA_MERCHANT\n");
          break;
      case 'P':
      case 'p':
          ssaMode = SSA_CUSTOMER_PRIMARY;
          trcS("SSA_CUSTOMER_PRIMARY\n");
          break;
      default:
          VERIFY(mod == 'S' || mod == 's');
          ssaMode = SSA_CUSTOMER_SECONDARY;
          trcS("SSA_CUSTOMER_SECONDARY\n");
          break;
    }
    ret = ssaSecOpen(ssaMode, &hSsa);
    trcFN("cryStart: ret=%d\n", ret);
    CHECK(ret == RET_OK, lblKO);
    ssaMode = mod;

    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return 1;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control to another application.
 * \param mod (I) Should be the same as in related cryStart() function call.
 * \return
 *    - negative if failure.
 *    - non-negative if OK.
 * \remark
 *    closes default ssa channel
 * \header sys\\sys.h
 * \source sys\\c16.c, sys\\c32.c
 * \test tcab0021.c
 */
int cryStop(char mod) {
    int ret;

    if(mod != ssaMode)
        return hSsa;            //no real stop if it was intermediate call
    if(hSsa) {
        ret = ssaSecClose(hSsa);
        trcFN("cryStop: %d\n", ret);
        CHECK(ret == RET_OK, lblKO);
    }
    hSsa = 0;
    ssaMode = 0;
    return hSsa;
  lblKO:
    trcErr(ret);
    return -1;
}

typedef struct {
    int16 sta;
    uint16 len;
    char buf[256];
} tResponse;

//wait for ssa result for sec seconds; get the response into rsp
static void getSsaRsp(tResponse * rsp, word sec) {
    int ret;
    uint32 tmp;

    VERIFY(rsp);

    tmp = sec * 100;
    ret = psyPeripheralResultWait(0, PSY_INFINITE_TIMEOUT, PSY_WAIT_ON_EVENT);
    memset(rsp, 0, sizeof(*rsp));
    if(ret & PSY_EVENT_RECEIVED) {
        if(ret & SSA_WAIT) {
            ssaSecResultGet(hSsa, sizeof(*rsp), rsp);
            //while((ret = ssaSecResultGet(hSsa, sizeof(*rsp), rsp)) != RET_OK);

        }
    }
}

typedef struct {
    byte line1;                 //1st display line
    byte line2;                 //2nd display line
    byte line3;                 //3rd display line
    byte min;                   //minimum length of pincode
    byte max;                   //maximum length of pincode
    char msk;                   //mask character
    byte inp;                   //input line number
    word col;                   //column mask line
    byte dly1st;                //1st character timeout
    byte dlyNxt;                //Intercharacter timeout
    byte dlyRsp;                //response waiting delay
#ifdef __DUKPT__
    byte dukptMod;              //0 - 0X00 -Ask Pin and update dukpt key ,1- 0xFF - do not ask pin ,only update dukpt key
#endif
} tPpd;

#ifdef __DUKPT__
static tPpd ppdCfg = { 1, 2, 0, 4, 12, '*', 2, 0x0FFF, 30, 10, 60, 0 };
#else
static tPpd ppdCfg = { 1, 2, 0, 4, 12, '*', 2, 0x0FFF, 30, 10, 60 };
#endif
/** Default pinpad configuration settings.
 * \param cfg (I) pinpad configuration parameters. Each character corresponds to a default parameter:
 *  - cfg[0] is the line number of pinpad screen where to display the first message
 *  - cfg[1] is the line number of pinpad screen where to display the second message
 *  - cfg[2] is the line number of pinpad screen where to display the third message
 *  - cfg[3] minimum length of pincode allowed
 *  - cfg[4] maximum length of pincode allowed
 *  - cfg[5] input line number / mask character
 *  - cfg[6],cfg[7] 1st character timeout in seconds
 *  - cfg[8],cfg[9] intercharacter character timeout in seconds
 *  - cfg[10],cfg[11] response waiting timeout in seconds
 *  - cfg[12] Dukpt mod
 * \return  N/A
 * \pre
 *  - strlen(cfg)==12
 *  - cfg[0],cfg[1],cfg[2],cfg[3] are decimal characters
 *  - cfg[4] is a hexadecimal character
 *  - cfg[6]-cfg[11] are decimal characters
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0023.c
 */
void crySetPpd(const char *cfg) {
    byte tmp;
    byte bit[2];
    card num;

#ifdef __DUKPT__
    if(!cfg)
        cfg = "1204C23010600";  //default configuration
    VERIFY(strlen(cfg) == 13);
#else
    if(!cfg)
        cfg = "1204C2301060";   //default configuration
    VERIFY(strlen(cfg) == 12);
#endif

    VERIFY('0' <= cfg[0]);
    VERIFY(cfg[0] <= '9');
    ppdCfg.line1 = cfg[0] - '0';

    VERIFY('0' <= cfg[1]);
    VERIFY(cfg[1] <= '9');
    ppdCfg.line2 = cfg[1] - '0';

    VERIFY('0' <= cfg[2]);
    VERIFY(cfg[2] <= '9');
    ppdCfg.line3 = cfg[2] - '0';

    VERIFY('0' <= cfg[3]);
    VERIFY(cfg[3] <= '9');
    ppdCfg.min = cfg[3] - '0';

    tmp = cfg[4];
    VERIFY('0' <= tmp);
    if(tmp > '9') {
        VERIFY('A' <= tmp);
        VERIFY(tmp <= 'F');
        tmp -= 'A';
        tmp += 10;
    } else
        tmp -= '0';
    ppdCfg.max = tmp;

    ppdCfg.msk = '*';
    ppdCfg.inp = 2;
    if('0' <= cfg[5] && cfg[5] <= '9')
        ppdCfg.inp = cfg[5] - '0';
    else
        ppdCfg.msk = cfg[5];

    memset(bit, 0, 2);
    tmp = ppdCfg.max;
    while(tmp--) {              //calculate the set of columns to accept input
        bitOn(bit, (byte) (16 - tmp));
    }
    ppdCfg.col = WORDHL(bit[0], bit[1]);

    dec2num(&num, cfg + 6, 2);
    ppdCfg.dly1st = (byte) num;

    dec2num(&num, cfg + 8, 2);
    ppdCfg.dlyNxt = (byte) num;

    dec2num(&num, cfg + 10, 2);
    ppdCfg.dlyRsp = (byte) num;
#ifdef __DUKPT__
    if(cfg[12] == '1')
        ppdCfg.dukptMod = 0xFF;
    else
        ppdCfg.dukptMod = 0x00;
#endif
}

#ifdef __EMV__
/** Wrapper around the function ssaSecEmvl2PinOfflineReq.
 * Ask for an offline pin for an EMV card.
 * The pin entered is sent to EMV Kernel and a command APDU is generated and sent to the card.
 * The pin is not visible to the application.
 * Implemented for 32bit platform only.
 * \param msg (I) Contains the messages to be displayed separated by '\n'
 * \return
 *    - the last key pressed (kbdVal) if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0017.c
 */
int cryEmvPin(const char *msg) {
    psyPid_t pid;
    ssaSecDispKbdInput_t inp;
    ssaSecInput_adv_t adv;
    int32 ret;
    tResponse rsp;
    char msg1[dspW + 1];
    char msg2[dspW + 1];
    char msg3[dspW + 1];
    byte idx;
    char sap[16 + 1];
    byte is3010 = 0;

    VERIFY(msg);

    memset(msg1, 0, dspW + 1);
    memset(msg2, 0, dspW + 1);
    memset(msg3, 0, dspW + 1);

    //Check the pin pad (I3010)
    ret = getSapSer(sap, 0, 'p');
    if(ret > 0 && strstr(sap, "3010"))
        is3010 = 1;

    idx = 0;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg1[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg2[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg3[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    if(*msg1) {
        trcFS("msg1=%s\n", msg1);
    }
    if(*msg2) {
        trcFS("msg2=%s\n", msg2);
    }
    if(*msg3) {
        trcFS("msg3=%s\n", msg3);
    }

    memset(&inp, 0, sizeof(inp));
    memset(&rsp, 0, sizeof(rsp));
    memset(&adv, 0, sizeof(adv));
    pid = getPid();
    if(*msg3 && !is3010) {
        adv.version = 0;
        adv.lineNbText1 = ppdCfg.line1;
        adv.flagText1 = 0;
        memcpy(adv.text1, msg1, strlen(msg1));

        memset(adv.text1alt, 0, sizeof(adv.text1alt));

        adv.lineNbText2 = ppdCfg.line2;
        adv.fontText2 = 0;
        adv.flagText2 = 0;
        memcpy(adv.text2, msg2, strlen(msg2));

        adv.lineNbText3 = ppdCfg.line3;
        adv.fontText3 = 0;
        adv.flagText3 = 0;
        memcpy(adv.text3, msg3, strlen(msg3));

        adv.lineNbInput = ppdCfg.inp;
        adv.fontInput = 0;
        //memset(inp.textInput, 0, sizeof(inp.textInput));
    } else {
        inp.lineNbText = ppdCfg.line1;
        strcpy(inp.text, msg1);
        inp.lineNbInput = ppdCfg.inp;
        strcpy(inp.textInput, msg2);

    }

#ifdef __FONT__
    inp.fontText = is3010 ? 0 : SSA_INTERNAL_FONT_RAM(1);
#else
    inp.fontText = 0;
#endif
#if __SSA__>=517
    adv.fontText1 = inp.fontText;
    adv.fontText2 = inp.fontText;
    adv.fontText3 = inp.fontText;
#endif

    inp.inputFieldDefinition = ppdCfg.col;
    inp.direction = 1;
    inp.minimumKeys = ppdCfg.min;
    inp.maximumKeys = ppdCfg.max;
    inp.endKey = 0;
    inp.echoCharacter = ppdCfg.msk;
    inp.parameters1 = 0;
    inp.corKeyAction = 0;
    inp.timeOutFirstKey = ppdCfg.dly1st;
    inp.timeOutInterKey = ppdCfg.dlyNxt;
    inp.replacementCharacter = 0;
    inp.RFU1 = 0;
    inp.RFU2 = 0;
    memset(inp.values, 0, 4);

    if(*msg3 && !is3010) {
#if __SSA__>=517
        adv.inputFieldDefinition = 0xFFF0;
        adv.inputFieldDefinition2 = 0x0000;
        adv.direction = 0;
        adv.minimumKeys = ppdCfg.min;
        adv.maximumKeys = ppdCfg.max;
        adv.endKey = 0x00;
        adv.echoCharacter = ppdCfg.msk;
        adv.parameters1 = 0;
        adv.corKeyAction = 0;
        adv.timeOutFirstKey = ppdCfg.dly1st;
        adv.timeOutInterKey = ppdCfg.dlyNxt;
        adv.replacementCharacter = 0;
        memset(adv.values, 0, 4);
        adv.beepParameter = 0x00;

        ret = ssaSecPinEntryConfig(hSsa, SWITCH_AUTO, SSASECINPUT_ADV,
                                   (void *) &adv);
#endif
    }

    ret = ssaSecPinEntryEmvl2OfflineReq(hSsa, SWITCH_AUTO, 0xDF, &pid, &inp);

    trcFN("cryEmvPin: %d ", ret);
    trcFS("msg1=[%s] ", msg1);
    trcFS("msg2=[%s]\n", msg2);
    CHECK(ret == RET_RUNNING, lblKO);

    getSsaRsp(&rsp, ppdCfg.dlyRsp);
    switch (rsp.sta) {
      case RET_OK:
          break;
      case ERR_USER_CANCEL_KEY:
      case ERR_TIMEOUT_FIRST_KEY:
      case ERR_TIMEOUT_INTER_KEY:
          return kbdANN;
      default:
          goto lblKO;
    }

    return rsp.buf[0];

  lblKO:
    trcErr(ret);
    trcErr(rsp.sta);
    if(rsp.sta == ERR_USER_CANCEL_KEY)
        return kbdANN;
    return -1;
}
#endif

//This function is for internal usage used to load a testing key into a slot
//It is not documented intentionally
static int ssaChangeMK(void) {  //change Manufacturer Key temporarily
    int ret;
    byte *ptr;
    ssaInMsg_t mIn;
    ssaOutMsg_t mOut;

    ptr = mIn.inBuffContent.inBuffType11.msg;

    mIn.header.functionNb = SECURITY_SYNC0; //0x80
    mIn.header.subFunctionNb = 0x00;
    mIn.header.type = INBUFFTYPE11; //0x11
    mIn.inBuffContent.inBuffType11.lgMsg = 18;
    ptr[0] = 0x5A;
    memcpy(&ptr[1], "01111111111111111", 17);
    ret = ssaSecCall(hSsa, &mIn, &mOut);
    CHECK(ret == RET_OK, lblKO);

    mIn.header.functionNb = SECURITY_SYNC0;
    mIn.header.subFunctionNb = 0x00;
    mIn.header.type = INBUFFTYPE11;
    mIn.inBuffContent.inBuffType11.lgMsg = 18;
    ptr[0] = 0x5A;
    memcpy(&ptr[1], "11111111111111111", 17);
    ret = ssaSecCall(hSsa, &mIn, &mOut);
    CHECK(ret == RET_OK, lblKO);

    mIn.header.functionNb = SECURITY_SYNC0; //0x80
    mIn.header.subFunctionNb = 0x00;
    mIn.header.type = INBUFFTYPE11; //0x11
    mIn.inBuffContent.inBuffType11.lgMsg = 18;
    mIn.inBuffContent.inBuffType11.msg[0] = 0x5B;
    memcpy(&mIn.inBuffContent.inBuffType11.msg[1], "01111111111111111", 17);
    ssaSecCall(hSsa, &mIn, &mOut);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Inject the key 1111111111111111 into the location loc.
 * Using this key it is possible to download various keys without using a standard certified injection
 * procedure.
 * The function below can be used only for testing purposes.
 * In real applications the keys should be downloaded in a proper certified way.
 * \param loc (I) Destination location of  the key.
 *   The first nibble (loc/16) is the array number;
 *   the second one (loc%16) is the slot number inside the array.
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0021.c
 */
int cryLoadTestKey(byte loc) {
    int ret;
    byte key[16];
    byte *ptr;
    ssaSecKeyDescription_t dsc;
    ssaInMsg_t mIn;
    ssaOutMsg_t mOut;

    trcFN("cryLoadTestKey loc=%02X\n", (word) loc);

    dsc.array = loc / 16;
    dsc.slot = loc % 16;
    ptr = mIn.inBuffContent.inBuffType11.msg;

    ret = ssaChangeMK();
    CHECK(ret > 0, lblKO);

    memcpy(key, "4F58207DA7FE1201", 16);    //"1111111111111111" encrypted by manufacturer keys 0 and 1

    mIn.header.functionNb = SECURITY_SYNC0;
    mIn.header.subFunctionNb = 0x00;
    mIn.header.type = INBUFFTYPE11;
    mIn.inBuffContent.inBuffType11.lgMsg = 20;
    ptr[0] = 0x40;
    ptr[1] = 0x30 + dsc.array;
    ptr[2] = 0x30;
    ptr[3] = 0x30 + dsc.slot;
    memcpy(&ptr[4], key, 16);
    ret = ssaSecCall(hSsa, &mIn, &mOut);
    CHECK(ret == RET_OK, lblKO);

    mIn.header.functionNb = SECURITY_SYNC0;
    mIn.header.subFunctionNb = 0x00;
    mIn.header.type = INBUFFTYPE11;
    mIn.inBuffContent.inBuffType11.lgMsg = 20;
    ptr[0] = 0x40;
    ptr[1] = 0x30 + dsc.array;
    ptr[2] = 0x30;
    ptr[3] = 0x30 + dsc.slot + 1;
    memcpy(&ptr[4], key, 16);
    ret = ssaSecCall(hSsa, &mIn, &mOut);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Calculate key certificate.
 * The key certificate is the first four bytes of encryption of the value 0000000000000000 by the key.
 * This certificate can be used to ensure that the key at a given location was not changed
 *  after initial downloading.
 * \param loc (I) The location of  the key.
 *    The first nibble (loc/16) is the array number;
 *    the second one (loc%16) is the slot number inside the array.
 * \param crt (O) The resulting certificate. It is the pointer to an array of 4 bytes.
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0021.c
 */
int cryVerify(byte loc, byte * crt) {
    int ret;
    ssaSecKeyDescription_t dsc;

    VERIFY(crt);

    trcFN("cryVerify loc=%02X\n", (word) loc);

    dsc.array = loc / 16;
    dsc.slot = loc % 16;
    ret = ssaSecDblKeyVerificationCode(hSsa, &dsc, crt);
    CHECK(ret == RET_OK, lblKO);

    trcS("crt= ");
    trcBN(crt, 4);
    trcS("\n");

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Load double key key of property prp into location loc using a double transport key located
 * at the location trn.
 * The transport key must be downloaded into trn before calling this function.
 * The value key should be calculated as a result of triple DES encryption of the working key
 * to use by the transport key.
 * During this function call these data are decrypted using the transport key located at trn
 * and the result is saved at loc.
 * Usually this function is called to update working pinblock or MAC key.
 * \param loc (I) The location of  the key to be loaded into the cryptomodule.
 *   The first nibble (loc/16) is the array number;
 *   the second one (loc%16) is the slot number inside the array.
 * \param trn (I) The location of  the transport key.
 *   The first nibble (trn/16) is the array number;
 *   the second one (trn %16) is the slot number inside the array.
 * \param key (O) The result of EDE encrypting of the working key by the transport key.
 * \param prp (I) Key property, can take following values:
 *    - 'p': pin key, it will be used to calculate pinblock
 *    - 'm': mac key, it will be used to calculate MAC
 *    - 'd': to be used for DUKPT
 *    - 'g': generic key, it will be used for various purposes, e.g. as a transport key
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0022.c
 */
int cryLoadDKuDK(byte loc, byte trn, const byte * key, char prp) {
    int ret;
    ssaSecKeyDescription_t kDsc, tDsc;

#ifdef __TEST__
    char sav = prp;
#endif

    VERIFY(key);

    kDsc.array = loc / 16;
    kDsc.slot = loc % 16;

    tDsc.array = trn / 16;
    tDsc.slot = trn % 16;

    switch (prp) {
      case 'd':
          prp = DUKPT_INITIAL_KEY;
          break;                // Dukpt bkassovic
      case 'p':
          prp = TRIPLE_DES_PIN_KEY;
          break;
      case 'm':
          prp = MAC_KEY;
          break;
      default:
          VERIFY(prp == 'g');
          prp = GEN_KEY;
          break;
    }

    ret = ssaSecLoadDblKeyUsingDblKey(hSsa, prp, &tDsc, &kDsc, (byte *) key);
    trcFN("cryLoadDKuDK: ret=%d ", ret);
    trcFN("loc=%02X ", loc);
    trcFN("trn=%02X ", trn);
    trcFN("prp=%c ", sav);
    trcS("key=");
    trcBN(key, 16);
    trcS("\n");
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Diversify double key with double key
 * \param loc (I) destination key location
 *   The first nibble (loc/16) is the array number;
 *   the second one (loc%16) is the slot number inside the array.
 * \param div (I) diversify key location
 *   The first nibble (loc/16) is the array number;
 *   the second one (loc%16) is the slot number inside the array.
 * \param key (I) the destination key diversified by diversify key
 * \param prp (I) Key property, can take following values:
 *    - 'p': pin key, it will be used to calculate pinblock
 *    - 'm': mac key, it will be used to calculate MAC
 *    - 'd': to be used for DUKPT
 *    - 'g': generic key, it will be used for various purposes, e.g. as a transport key
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 */
int cryDiversifyDKwDK(byte loc, byte div, const byte * key, char prp) {
    int ret;
    ssaSecKeyDescription_t kDsc, tDsc;

    VERIFY(key);

    kDsc.array = loc / 16;
    kDsc.slot = loc % 16;

    tDsc.array = div / 16;
    tDsc.slot = div % 16;

    switch (prp) {
      case 'd':
          prp = DUKPT_INITIAL_KEY;
          break;
      case 'p':
          prp = TRIPLE_DES_PIN_KEY;
          break;
      case 'm':
          prp = MAC_KEY;
          break;
      default:
          VERIFY(prp == 'g');
          prp = GEN_KEY;
          break;
    }

    ret =
        ssaSecDiversifyDblKeyWithDblKey(hSsa, prp, &tDsc, &kDsc, (byte *) key,
                                        (byte *) key + 8);
    trcFN("cryDiversifyDKwDK: ret=%d ", ret);
    trcFN("loc=%02X ", loc);
    trcFN("div=%02X ", div);
    trcFN("prp=%c ", prp);
    trcS("key=");
    trcBN(key, 16);
    trcS("\n");
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Ask user to enter PIN.
 * The pin block blk is calculated according ANSI X9.8 algorithm
 * using the working key downloaded into the location loc
 * and using account number acc.
 * The pin dialog is prompted by two lines:
 * msg1 (usually, transaction amount) and msg2.
 * \param msg (I) The messages to be displayed on the first,second and third lines, separated by '\n'
 * \param acc (I) Account built on the PAN of the card in the following manner.
 *    At first an 16-characters ASCII string is built:
 *    - first four characters are zeroes
 *    - they are concatenated with 12 characters extracted from PAN
 *      (12 digits before Luhn code)
 *    - The string is converted into bcd format to obtain acc.
 * \param loc (I) The location of  the pin key downloaded into the cryptomodule.
 *    The first nibble (loc/16) is the array number;
 *    the second one (loc%16) is the slot number inside the array.
 * \param blk (O) The resulting pin block, the pointer to a buffer containing 8 bytes
 * \return
 *    - number of digits entered if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0023.c
 */
int cryGetPin(const char *msg, const byte * acc, byte loc, byte * blk) {
    int ret;
    tResponse rsp;
    ssaSecDispKbdInput_t inp;
    ssaSecKeyDescription_t dsc;
    char sap[16 + 1];
    byte is3010 = 0;

#if __SSA__>=517
    ssaSecInput_adv_t adv;
#endif
    char msg1[dspW + 1];
    char msg2[dspW + 1];
    char msg3[dspW + 1];
    byte idx;

    VERIFY(msg1);
    VERIFY(msg2);
    VERIFY(acc);
    VERIFY(blk);

    trcFN("cryGetPin: loc=%02X ", loc);
    trcS("acc=");
    trcBN(acc, 8);
    trcS("\n");

    //Check the pin pad (I3010)
    ret = getSapSer(sap, 0, 'p');
    if(ret > 0 && strstr(sap, "3010"))
        is3010 = 1;

    memset(msg1, 0, dspW + 1);
    memset(msg2, 0, dspW + 1);
    memset(msg3, 0, dspW + 1);

    idx = 0;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg1[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg2[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg3[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    if(*msg1) {
        trcFS("msg1=%s\n", msg1);
    }
    if(*msg2) {
        trcFS("msg2=%s\n", msg2);
    }
    if(*msg3) {
        trcFS("msg3=%s\n", msg3);
    }

    dsc.array = loc / 16;
    dsc.slot = loc % 16;

    memset(&inp, 0, sizeof(inp));
#if __SSA__>=517
    memset(&adv, 0, sizeof(adv));
#else
    VERIFY(*msg3 == 0);
#endif

    if(*msg3 && !is3010) {
#if __SSA__>=517
        adv.version = 0;
        adv.lineNbText1 = ppdCfg.line1;
        adv.flagText1 = 0;
        memcpy(adv.text1, msg1, strlen(msg1));

        memset(adv.text1alt, 0, sizeof(adv.text1alt));

        adv.lineNbText2 = ppdCfg.line2;
        adv.fontText2 = 0;
        adv.flagText2 = 0;
        memcpy(adv.text2, msg2, strlen(msg2));

        adv.lineNbText3 = ppdCfg.line3;
        adv.fontText3 = 0;
        adv.flagText3 = 0;
        memcpy(adv.text3, msg3, strlen(msg3));

        adv.lineNbInput = ppdCfg.inp;
        adv.fontInput = 0;
        //memset(inp.textInput, 0, sizeof(inp.textInput));
#endif
    } else {
        inp.lineNbText = ppdCfg.line1;
        strcpy(inp.text, msg1);
        inp.lineNbInput = ppdCfg.inp;
        strcpy(inp.textInput, msg2);
    }

#ifdef __FONT__
    inp.fontText = is3010 ? 0 : SSA_INTERNAL_FONT_RAM(1);
#else
    inp.fontText = 0;
#endif
#if __SSA__>=517
    adv.fontText1 = inp.fontText;
    adv.fontText2 = inp.fontText;
    adv.fontText3 = inp.fontText;
#endif

    inp.inputFieldDefinition = ppdCfg.col;
    inp.direction = 1;
    inp.minimumKeys = ppdCfg.min;
    inp.maximumKeys = ppdCfg.max;
    inp.endKey = 0;
    inp.echoCharacter = ppdCfg.msk;
    inp.parameters1 = 0;
    inp.corKeyAction = 0;
    inp.timeOutFirstKey = ppdCfg.dly1st;
    inp.timeOutInterKey = ppdCfg.dlyNxt;
    inp.replacementCharacter = 0;
    inp.RFU1 = 0;
    inp.RFU2 = 0;
    inp.fontInput = 0;
    memset(inp.values, 0, sizeof(inp.values));

    if(*msg3 && !is3010) {
#if __SSA__>=517
        adv.inputFieldDefinition = 0xFFF0;
        adv.inputFieldDefinition2 = 0x0000;
        adv.direction = 0;
        adv.minimumKeys = ppdCfg.min;
        adv.maximumKeys = ppdCfg.max;
        adv.endKey = 0x00;
        adv.echoCharacter = ppdCfg.msk;
        adv.parameters1 = 0;
        adv.corKeyAction = 0;
        adv.timeOutFirstKey = ppdCfg.dly1st;
        adv.timeOutInterKey = ppdCfg.dlyNxt;
        adv.replacementCharacter = 0;
        memset(adv.values, 0, 4);
        adv.beepParameter = 0x00;

        ret =
            ssaSecPinEntryConfig(hSsa, SWITCH_AUTO, SSASECINPUT_ADV,
                                 (void *) &adv);
#endif
    }

    ret =
        ssaSecPinEntryTripleDesReq(hSsa, SWITCH_AUTO, &dsc, (byte *) acc, &inp);
    CHECK(ret == RET_RUNNING, lblKO);

    getSsaRsp(&rsp, ppdCfg.dlyRsp);
    switch (rsp.sta) {
      case RET_OK:
          break;
      case ERR_USER_CANCEL_KEY:
      case ERR_TIMEOUT_FIRST_KEY:
      case ERR_TIMEOUT_INTER_KEY:
          return 0;
      default:
          goto lblKO;
    }
    if(rsp.buf[0] != kbdVAL)
        return 0;
    memcpy(blk, rsp.buf + 2, 8);

    trcS("blk=");
    trcBN(blk, 8);
    trcS("\n");

    return rsp.buf[1];
  lblKO:
    trcErr(ret);
    return -1;
}

/** Calculate MAC (Message Authentication Code).
 * \param buf (I) Input buffer to calculate MAC.
 *  Its length should be a multiple of 8 and greater than 8.
 * \param loc (I) The location of  the pin key downloaded into the cryptomodule.
 *  The first nibble (loc/16) is the array number;
 *  the second one (loc%16) is the slot number inside the array.
 * \param mac (O) The resulting MAC, the pointer to a buffer containing 4 bytes
 * \return
 *    - the length of MAC (=4) if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0061.c
 */
int cryMac(tBuffer * buf, byte loc, byte * mac) {
    int ret;
    ssaSecKeyDescription_t dsc;

    VERIFY(buf);
    VERIFY(bufLen(buf) >= 16);
    VERIFY(bufLen(buf) % 8 == 0);
    VERIFY(mac);

    trcFN("cryMac: loc=%02X ", loc);
    trcS("buf=");
    trcBN(bufPtr(buf), bufLen(buf));
    trcS("\n");

    dsc.array = loc / 16;
    dsc.slot = loc % 16;
    ret =
        ssaSecMacCalculationMethod1(hSsa, &dsc, bufLen(buf),
                                    (byte *) bufPtr(buf), mac);
    CHECK(ret == RET_OK, lblKO);

    trcS("mac=");
    trcBN(mac, 8);
    trcS("\n");

    return 8;
  lblKO:
    return -1;
}

/** Erase the array of the keys.
 * \param array (I) Array of the keys, what will be erase.
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcgs0033.c
*/
int cryEraseKeysArray(byte array) {
    int ret;

    trcFN("cryEraseKeysArray array=%02d\n", (word) array);

    ret = ssaSecEraseArrayOfKeys(hSsa, array);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

#ifdef __RSA__
static void strRev(byte * data, word datalen) {
    byte c;
    byte *b;
    byte *e;

    VERIFY(data);

    if(NULL != data && 1 < datalen) {
        b = data;
        e = data + datalen - 1;

        do {
            c = *b;
            *b++ = *e;
            *e-- = c;
        } while(b < e);
    }
}

/** The SHA (Secure Hash Algorithm)
 * \param buf (I) Data to be Hashed by SHA-1 algorithm.
 * \param len (I) The length of data.
 * \param hash (O) The output (hashed data).
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcfa0002.c
 */
int cryCalculateSha1(tBuffer * buf, int len, byte * hash) {
    int ret;

    VERIFY(buf);
    VERIFY(hash);

    ret = psyCryShaCalculate(buf->ptr, len, hash);

    switch (ret) {
      case RET_OK:             // Successful
          goto lblEnd;
      case ERR_SYS_RESOURCE_PB:    // Resource Problem
          return -1;
      case PSY_ERR_CRY_SHA_INTERNAL_ERROR: // SHA routines failed
          return -1;
    }

  lblEnd:
    return 1;
}

/** RSA CRT Algorithm Encryption
 * \param p (I) Prime 1
 * \param q (I) Prime 2
 * \param d (I) private key modulus
 * \param dataIn (I) Data to be signed (Hashed Data).
 * \param size (I) Size of the RSA Key in bytes.
 * \param dataOut (O) The output (Signed Data by private key).
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcfa0002.c
 */
int cryRsaSignData(byte * p, byte * q, byte * d, byte * dataIn, byte * dataOut,
                   word size) {
    int ret;
    unsigned char keyPtr[1024] = "";

    VERIFY(p);
    VERIFY(q);
    VERIFY(d);
    VERIFY(dataIn);
    VERIFY(dataOut);

    strRev(dataIn, size);

    strRev(p, (word) (size / 2));
    strRev(q, (word) (size / 2));
    strRev(d, size);

    *(uint32 *) keyPtr = size / sizeof(uint32); // keysize in words

    memcpy(keyPtr + sizeof(uint32), p, size / 2);   //p: Little endian format
    memcpy(keyPtr + sizeof(uint32) + size / 2, q, size / 2);    //q: Little endian format
    memcpy(keyPtr + sizeof(uint32) + size, d, size);    //d: Little endian format

    ret = psyCryRsaEncryptCRT(0, keyPtr, 0, dataIn, 0, dataOut);

    strRev(dataOut, size);

    switch (ret) {
      case RET_OK:
          goto lblEnd;
      case ERR_SYS_RESOURCE_PB:
          return -1;
      case PSY_ERR_CRY_RSA_LENGTH_ZERO:
          return -1;
      case PSY_ERR_CRY_RSA_MAX_LEN_EXCEEDED:
          return -1;
      case PSY_ERR_CRY_RSA_PRIME_NOT_ODD:
          return -1;
      case PSY_ERR_CRY_RSA_ALIGNMENT_ERROR:
          return -1;
      case PSY_ERR_CRY_RSA_INTERNAL_ERROR:
          return -1;
    }

  lblEnd:
    return 1;
}

/** Encode SHA-1 hash value in EMSA-PKCS1-v1_5 formatting.
 * With RSA signatures, the hash value is encoded as described in pkcs-1 section 9.2.1 of 
 * RSA cryptography standard using PKCS-1 encoding type EMSA-PKCS1-v1_5 this requires
 * inserting the hash value as an octet string into an ASN.1 structure. The object 
 * identifier for the type of hash being used is included in the structure.
 * \n The ASN.1 OIDs are:
 * \n   - MD5:        1.2.840.113549.2.5
 * \n   - RIPEMD-160: 1.3.36.3.2.1
 * \n   - SHA-1:      1.3.14.3.2.26
 * \n   - SHA256:     2.16.840.1.101.3.4.2.1
 * \n   - SHA384:     2.16.840.1.101.3.4.2.2
 * \n   - SHA512:     2.16.840.1.101.3.4.2.3
 * \n In practice this amounts to prefixing the hash with one of the following, then padding as described in PKCS #1:
 * \n   - MD5: 0x30, 0x20, 0x30, 0x0C, 0x06, 0x08, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05, 0x05, 0x00,0x04, 0x10
 * \n   - RIPEMD-160: 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x24, 0x03, 0x02, 0x01, 0x05, 0x00, 0x04, 0x14
 * \n   - SHA-1: 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x04, 0x14
 * \n   - SHA256: 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
 * \n   - SHA384: 0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30
 * \n   - SHA512: 0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40
 * \n The value emLen needed for the padding is equal to the length in bytes of the RSA public modulus, n.
 * Once the hash has been encoded and padded, the resulting string is encrypted with the RSA private key as described in [RSA].
 * \n \line
 * Below is a portion of this document where EMSA-PKCS1-v1_5 is described.
 * \n \b EMSA-PKCS1-v1_5
 * This encoding method is deterministic and only has an encoding operation. 
 * EMSA-PKCS1-v1_5-ENCODE (M, emLen)
 * Option:	Hash	hash function (hLen denotes the length in octets of the hash function output)
 * Input:	M	message to be encoded
 * emLen	intended length in octets of the encoded message, at least tLen + 11, where tLen is the octet length of the DER encoding T of a certain value computed during the encoding operation
 * Output:	EM	encoded message, an octet string of length emLen
 * Errors:	“essage too long” “ntended encoded message length too short”
 * Steps:
 * \n 1. Apply the hash function to the message M to produce a hash value H:
 * \n H = Hash (M).
 * \n If the hash function outputs “essage too long,”output “essage too long”and stop.
 * \n 2. Encode the algorithm ID for the hash function and the hash value into an ASN.1 value of type DigestInfo (see Appendix Error! Reference source not found.) with the Distinguished Encoding Rules (DER), where the type DigestInfo has the syntax
 * \n DigestInfo ::= SEQUENCE {digestAlgorithm AlgorithmIdentifier, digest OCTET STRING}
 * \n The first field identifies the hash function and the second contains the hash value. Let T be the DER encoding of the DigestInfo value (see the notes below) and let tLen be the length in octets of T.
 * \n 3. If emLen < tLen + 11, output “ntended encoded message length too short”and stop.
 * \n 4. Generate an octet string PS consisting of emLen –tLen –3 octets with hexadecimal value 0xff. The length of PS will be at least 8 octets.
 * \n 5. Concatenate PS, the DER encoding T, and other padding to form the encoded message EM as
 * \n EM = 0x00 || 0x01 || PS || 0x00 || T .
 * \n 6.	Output EM.
 * \n Note:
 * \n For the six hash functions mentioned in Appendix Error! Reference source not found., the DER encoding T of the DigestInfo value is equal to the following:
 * \n   - MD2: 	(0x)30 20 30 0c 06 08 2a 86 48 86 f7 0d 02 02 05 00 04 10 || H.
 * \n   - MD5: 	(0x)30 20 30 0c 06 08 2a 86 48 86 f7 0d 02 05 05 00 04 10 || H.
 * \n   - SHA-1: 	(0x)30 21 30 09 06 05 2b 0e 03 02 1a 05 00 04 14 || H.
 * \n   - SHA-256:	(0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 || H.
 * \n   - SHA-384:	(0x)30 41 30 0d 06 09 60 86 48 01 65 03 04 02 02 05 00 04 30 || H.
 * \n   - SHA-512:	(0x)30 51 30 0d 06 09 60 86 48 01 65 03 04 02 03 05 00 04 40 || H.
 * \n \line
 * \param hash (I) SHA-1 hashed data to be formated in to PKCS#1.
 * \param size (I) Size of the RSA key (in bytes), Which will use in the Encryption.
 * Ex: If key size (=1024) bits, size must send in param is 128.
 * \param pkcs (O) The output in PKCS#1 format
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcfa0002.c
 */
int crySha1ToPkcs1(byte * hash, word size, byte * pkcs) {
    int ret;
    byte dBuf[1000];
    tBuffer bBuf;

    bufInit(&bBuf, dBuf, 1000);
    bufReset(&bBuf);

    VERIFY(pkcs);
    VERIFY(hash);

    //Begin characters of PKCS1# format.
    ret = bufApp(&bBuf, (byte *) "\x00\x01", 2);
    CHECK(ret >= 0, lblKO);

    //Append n of \xff, which n is:
    //sizeOfKey in bytes - sizeOfHashData (always 20 bytes) - PKCS#1 end characters (always 16 bytes)    
    while(bufLen(&bBuf) < size - 16 - 20) {
        ret = bufApp(&bBuf, (byte *) "\xFF", 1);
        CHECK(ret >= 0, lblKO);
    }

    //End Characters (16 bytes) for SHA-1 algorithm, should be at the end of each PKCS#1 buffer.
    ret = bufApp(&bBuf, (byte *)
                 "\x00\x30\x21\x30\x09\x06\x05\x2B\x0E\x03\x02\x1A\x05\x00\x04\x14",
                 16);
    CHECK(ret >= 0, lblKO);
    ret = bufApp(&bBuf, (byte *) hash, 20);
    CHECK(ret >= 0, lblKO);

    memcpy(pkcs, bBuf.ptr, size);
    return 1;

  lblKO:
    return -1;
}
#endif

#ifdef __OWF__
/** One Way Function Internal:
 * Used to transform a key to another key loaded internaly (SSA) with a one way function algorithm
 * \param datLeft (I) Data left
 * \param datRight (I) Data right
 * \param locSrcKey (I) Location of the source key (Can be a KEY_REGISTER_KEY or GEN_KEY).
 *  The first nibble (loc/16) is the array number;
 *  the second one (loc%16) is the slot number inside the array.
 * \param locDstKey (I) Location of destination key.
 *  The first nibble (loc/16) is the array number;
 *  the second one (loc%16) is the slot number inside the array.
 * \param prp (I) Property of the destination key (Can be a MAC_KEY, TRIPLE_DES_PIN_KEY or GEN_KEY)
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 */
int cryOwfInt(const byte * datLeft, const byte * datRight, byte locSrcKey,
              byte locDstKey, char prp) {
    int ret;
    ssaSecKeyDescription_t dscSK, dscDK;
    ssaSecOneWayFunc_t param;

    VERIFY(datLeft);
    VERIFY(datRight);

    dscSK.array = locSrcKey / 16;
    dscSK.slot = locSrcKey % 16;

    dscDK.array = locDstKey / 16;
    dscDK.slot = locDstKey % 16;

    switch (prp) {
      case 'p':
          prp = TRIPLE_DES_PIN_KEY;
          break;
      case 'm':
          prp = MAC_KEY;
          break;
      default:
          VERIFY(prp == 'g');
          prp = GEN_KEY;
          break;
    }

    param.oneWayVariant = SSA_SEC_ONE_WAY_INTERNAL; // Return result to SSA
    param.keyUsage = prp;       // Destination Key property
    memcpy(param.dataLeft, (uint8 *) datLeft, 8);   // Data Left
    memcpy(param.dataRight, (uint8 *) datRight, 8); // Data Right
    memcpy(&param.keyRegister, &dscSK, sizeof(ssaSecKeyDescription_t)); // Source Key (Register)
    memcpy(&param.keyTarget, &dscDK, sizeof(ssaSecKeyDescription_t));   // Destination Key (Result to SSA)

    ret = ssaSecOneWayFunction(hSsa, &param);

    trcFN("cryOwfInt: ret=%d ", ret);
    trcFN("locSrcKey=%02X ", locSrcKey);
    trcFN("locDstKey=%02X ", locDstKey);
    trcFN("prp=%d\n", prp);

    trcS("datLeft=");
    trcBN(datLeft, 8);
    trcS("\n");

    trcS("datRight=");
    trcBN(datRight, 8);
    trcS("\n");

    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** One Way Function External:
 * Used to transform a key to another key or data externaly (Application) with one way function algorithm
 * \param datLeft (I) Data left
 * \param datRight (I) Data right
 * \param locSrcKey (I) Location of the source key
 *  (Can be a KEY_REGISTER_KEY, GEN_KEY, MAC_KEY or TRIPLE_DES_PIN_KEY).
 *  The first nibble (loc/16) is the array number;
 *  the second one (loc%16) is the slot number inside the array.
 * \param resLeft (O) Result left
 * \param resRight (O) Result right
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 */
int cryOwfExt(const byte * datLeft, const byte * datRight, byte locSrcKey,
              byte * resLeft, byte * resRight) {
    int ret;
    ssaSecKeyDescription_t dscSK;
    ssaSecOneWayFunc_t param;

    VERIFY(datLeft);
    VERIFY(datRight);

    dscSK.array = locSrcKey / 16;
    dscSK.slot = locSrcKey % 16;

    param.oneWayVariant = SSA_SEC_ONE_WAY_EXTERNAL; // Return result to Application
    memcpy(param.dataLeft, (uint8 *) datLeft, 8);   // Data Left
    memcpy(param.dataRight, (uint8 *) datRight, 8); // Data Right
    memcpy(&param.keyRegister, &dscSK, sizeof(ssaSecKeyDescription_t)); // Source Key (Register)

    ret = ssaSecOneWayFunction(hSsa, &param);

    memcpy((uint8 *) resLeft, param.keyOut.left, 8);    // return Result Left
    memcpy((uint8 *) resRight, param.keyOut.right, 8);  // return Result Right

    trcFN("cryOwfExt: ret=%d ", ret);
    trcFN("locSrcKey=%02X\n", locSrcKey);

    trcS("datLeft=");
    trcBN(datLeft, 8);
    trcS("\n");

    trcS("datRight=");
    trcBN(datRight, 8);
    trcS("\n");

    trcS("resLeft=");
    trcBN(resLeft, 8);
    trcS("\n");

    trcS("resRight=");
    trcBN(resRight, 8);
    trcS("\n");

    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}
#endif

/** Clear pin pad display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tctd0003.c
 */
int ppdClear() {
    byte idx;

    for (idx = 0; idx < ppdH; idx++)
        if(ppdLS(idx, "") < 0)
            return -1;
    return 1;
}

/** Display string at a given location
 * \param loc (I) Pinpad display line number where the string should be displayed
 * \param str (I) string to be displayed
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tctd0003.c
 */
int ppdLS(byte loc, const char *str) {
    static dispLine_t description[ppdH];

    VERIFY(str);
    VERIFY(loc < ppdH);
    trcFN("ppdLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    if(ssaMode == 0 || hSsa == 0)
        return -1;

    description[loc].col = 0;
    memcpy(description[loc].text, str, ppdW);
    description[loc].text[ppdW] = 0;    // truncate the string if it is too long

    ssaSecDisDisplayText(hSsa, PPAD_1, description);
    return 1;
}

#ifdef __DUKPT__
/** Key management DUKPT Initialisation.
 * \param loc (I) The location of  the dukpt key downloaded into the cryptomodule.
 *  The first nibble (loc/16) is the array number;
 *  the second one (loc%16) is the slot number inside the array.
 * \param SerialNumberKey (I) KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcbk0001.c
 */
int cryDukptInit(byte loc, byte * SerialNumberKey) {
    int ret;
    ssaSecKeyDescription_t dsc;

    VERIFY(SerialNumberKey);

    trcFN("cryDukptInit: loc=%02x ", loc);
    trcS("SNkey=");
    trcBN(SerialNumberKey, 10);
    trcS("\n");

    dsc.array = loc / 16;
    dsc.slot = loc % 16;

    ret = ssaSecDukptInit(hSsa, 0x00, dsc, SerialNumberKey);
    CHECK(ret == RET_OK, lblKO);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return 1;
}

int cryDukptGetPin(const char *msg1, const char *msg2, const byte * acc,
                   word sec, byte * SerialNumberKey, byte * blk) {
    int ret;
    tResponse rsp;
    ssaSecDispKbdInput_t inp;

    VERIFY(msg1);
    VERIFY(msg2);
    VERIFY(acc);
    VERIFY(blk);

    trcS("cryDukptGetPin\n");
    trcS("acc=");
    trcBN(acc, 8);
    trcS("\n");
    trcFS("msg1=%s\n", msg1);
    trcFS("msg2=%s\n", msg2);

    memset(&inp, 0, sizeof(inp));
    inp.lineNbText = ppdCfg.line1;
    inp.fontText = 0;
    strcpy(inp.text, msg1);
    inp.lineNbInput = ppdCfg.inp;
    inp.fontInput = 0;
    strcpy(inp.textInput, msg2);
    inp.inputFieldDefinition = 0x0FFF;
    inp.minimumKeys = ppdCfg.min;
    inp.maximumKeys = ppdCfg.max;
    inp.endKey = 0;
    inp.echoCharacter = ppdCfg.msk;
    inp.parameters1 = 0;
    inp.corKeyAction = 0;
    inp.timeOutFirstKey = ppdCfg.dly1st;
    inp.timeOutInterKey = ppdCfg.dlyNxt;
    inp.replacementCharacter = 0;
    inp.RFU1 = 0;
    inp.RFU2 = 0;
    memset(inp.values, 0, 4);

    ret = ssaSecPinEntryDukptReq(hSsa, TERM, 0x00, (byte *) acc, &inp);
    CHECK(ret == RET_RUNNING, lblKO);

    getSsaRsp(&rsp, sec);
    CHECK(rsp.sta == RET_OK, lblKO);

    if(rsp.buf[0] != kbdVAL)
        return 0;
    memcpy(SerialNumberKey, rsp.buf + 2, 10);
    memcpy(blk, rsp.buf + 12, 8);

    trcS("SerialNumberKey=");
    trcBN(SerialNumberKey, 10);
    trcS("\n");
    trcS("blk=");
    trcBN(blk, 8);
    trcS("\n");

    return rsp.buf[1];
  lblKO:
    trcErr(ret);
    return -1;
}

/** Key management 3DES DUKPT Initialisation
 * \param loc (I) The location of  the dukpt key downloaded into the cryptomodule.
 *  The first nibble (loc/16) is the array number;
 *  the second one (loc%16) is the slot number inside the array.
 * \param SerialNumberKey (I) KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcbk0003.c
 */
int cry3DESDukptInit(byte loc, byte * SerialNumberKey) {
    int ret;
    ssaSecKeyDescription_t dsc;

    VERIFY(SerialNumberKey);

    trcFN("cry3DESDukptInit: loc=%02x ", loc);
    trcS("SNkey=");
    trcBN(SerialNumberKey, 10);
    trcS("\n");

    dsc.array = loc / 16;
    dsc.slot = loc % 16;

    ret = ssaSecDukpt3DESInit(hSsa, dsc, 0x03, SerialNumberKey);
    CHECK(ret == RET_OK, lblKO);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:
    return 1;
}

/** Pin entry 3DES DUKPT : pin process (ANSI X9.24-2002)
 * \param msg1 (I) Message to be displayed on the top
 * \param msg2 (I) Message to be displayed on the second line
 * \param acc (I) Account number
 * \param sec (I) timeout in second
 * \param SerialNumberKey (O) Resulting KeySerialNumber 10 bytes long
 *    (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \param blk (O) Resulting pinblock, the pointer to a buffer containing 8 bytes
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcbk0003.c
 */
int cry3DESDukptGetPin(const char *msg1, const char *msg2, const byte * acc,
                       word sec, byte * SerialNumberKey, byte * blk) {
    int ret;
    tResponse rsp;
    ssaSecDispKbdInput_t inp;

    VERIFY(msg1);
    VERIFY(msg2);
    VERIFY(acc);
    VERIFY(blk);

    trcS("cry3DESDukptGetPin\n");
    trcS("acc=");
    trcBN(acc, 8);
    trcS("\n");
    trcFS("msg1=%s\n", msg1);
    trcFS("msg2=%s\n", msg2);

    memset(&inp, 0, sizeof(inp));
    inp.lineNbText = ppdCfg.line1;
    inp.fontText = 0;
    strcpy(inp.text, msg1);
    inp.lineNbInput = ppdCfg.inp;
    inp.fontInput = 0;
    strcpy(inp.textInput, msg2);
    inp.inputFieldDefinition = 0x0FFF;
    inp.minimumKeys = ppdCfg.min;
    inp.maximumKeys = ppdCfg.max;
    inp.endKey = 0;
    inp.echoCharacter = ppdCfg.msk;
    inp.parameters1 = 0;
    inp.corKeyAction = 0;
    inp.timeOutFirstKey = ppdCfg.dly1st;
    inp.timeOutInterKey = ppdCfg.dlyNxt;
    inp.replacementCharacter = 0;
    inp.RFU1 = 0;
    inp.RFU2 = 0;
    memset(inp.values, 0, 4);

    ret = ssaSecPinEntryDukpt3DESReq(hSsa, TERM, 0, 0x00, (byte *) acc, &inp);
    CHECK(ret == RET_RUNNING, lblKO);

    getSsaRsp(&rsp, sec);
    CHECK(rsp.sta == RET_OK, lblKO);

    if(rsp.buf[0] != kbdVAL)
        return 0;
    memcpy(SerialNumberKey, rsp.buf + 2, 10);
    memcpy(blk, rsp.buf + 12, 8);

    trcS("SerialNumberKey=");
    trcBN(SerialNumberKey, 10);
    trcS("\n");
    trcS("blk=");
    trcBN(blk, 8);
    trcS("\n");

    return rsp.buf[1];
  lblKO:
    trcErr(ret);
    return -1;
}

#if (__INGEDEV__>5)
/** Ask user to enter PIN.
 * The pin block blk is calculated according ANSI X9.8 algorithm
 * using the working key downloaded into the location loc
 * and using account number acc.
 * The pin dialog is prompted by two lines:
 * msg1 (usually, transaction amount) and msg2.
 * \param msg (I) The messages to be displayed on the first,second and third lines, separated by '\n'
 * \param acc (I) Account built on the PAN of the card in the following manner.
 * \param sec (I) timeout in second
 * \param SerialNumberKey (O) Resulting KeySerialNumber 10 bytes long
 *    (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \param blk (O) Resulting pinblock, the pointer to a buffer containing 8 bytes
 * \param loc (I) The location of  the dukpt key downloaded into the cryptomodule.
 *    At first an 16-characters ASCII string is built:
 *    - first four characters are zeroes
 *    - they are concatenated with 12 characters extracted from PAN
 *      (12 digits before Luhn code)
 *    - The string is converted into bcd format to obtain acc.
 * \param loc (I) The location of  the MAC key downloaded into the cryptomodule.
 *    The first nibble (loc/16) is the array number;
 *    the second one (loc%16) is the slot number inside the array.
 * \param blk (O) The resulting pin block, the pointer to a buffer containing 8 bytes
 * \return  
 *    - number of digits entered if OK.
 *    - negative if failure.
 * \remark
 *    This function is not supported in SSA541.
 *    SSA 557 that supports this function is delivered for Ingedev 6 only.
 *    So, it is disabled for Ingedev 5.
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcap0017.c 
*/
int cry3DESDukptGetPinUpdateKey(const char *msg, const byte * acc,
                                word sec, byte * SerialNumberKey,
                                byte * blk, byte loc) {
    int ret;
    tResponse rsp;
    ssaSecDispKbdInput_t inp;

    // ssaSecKeyDescription_t dsc;
    ssaSecKeyDescription_t MCKey;

    ssaSecInput_adv_t adv;
    char msg1[dspW + 1];
    char msg2[dspW + 1];
    char msg3[dspW + 1];
    byte idx;

    VERIFY(msg1);
    VERIFY(msg2);
    VERIFY(acc);
    VERIFY(blk);

    trcS("cry3DESDukptGetPin_UpdateMacKey\n");
    trcS("acc=");
    trcBN(acc, 8);
    trcS("\n");

    memset(msg1, 0, dspW + 1);
    memset(msg2, 0, dspW + 1);
    memset(msg3, 0, dspW + 1);

    idx = 0;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg1[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg2[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg3[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    if(*msg1) {
        trcFS("msg1=%s\n", msg1);
    }
    if(*msg2) {
        trcFS("msg2=%s\n", msg2);
    }
    if(*msg3) {
        trcFS("msg3=%s\n", msg3);
    }

    MCKey.array = loc / 16;
    MCKey.slot = loc % 16;

    memset(&inp, 0, sizeof(inp));
    memset(&adv, 0, sizeof(adv));

    if(*msg3) {
        adv.version = 0;
        adv.lineNbText1 = ppdCfg.line1;
        adv.flagText1 = 0;
        memcpy(adv.text1, msg1, strlen(msg1));

        memset(adv.text1alt, 0, sizeof(adv.text1alt));

        adv.lineNbText2 = ppdCfg.line2;
        adv.fontText2 = 0;
        adv.flagText2 = 0;
        memcpy(adv.text2, msg2, strlen(msg2));

        adv.lineNbText3 = ppdCfg.line3;
        adv.fontText3 = 0;
        adv.flagText3 = 0;
        memcpy(adv.text3, msg3, strlen(msg3));

        adv.lineNbInput = ppdCfg.inp;
        adv.fontInput = 0;
        //memset(inp.textInput, 0, sizeof(inp.textInput));

    } else {
        inp.lineNbText = ppdCfg.line1;
        strcpy(inp.text, msg1);
        inp.lineNbInput = ppdCfg.inp;
        strcpy(inp.textInput, msg2);

    }

#ifdef __FONT__
    inp.fontText = SSA_INTERNAL_FONT_RAM(1);
#else
    inp.fontText = 0;
#endif
    adv.fontText1 = inp.fontText;
    adv.fontText2 = inp.fontText;
    adv.fontText3 = inp.fontText;

    inp.inputFieldDefinition = ppdCfg.col;
    inp.direction = 1;
    inp.minimumKeys = ppdCfg.min;
    inp.maximumKeys = ppdCfg.max;
    inp.endKey = 0;
    inp.echoCharacter = ppdCfg.msk;
    inp.parameters1 = 0;
    inp.corKeyAction = 0;
    inp.timeOutFirstKey = ppdCfg.dly1st;
    inp.timeOutInterKey = ppdCfg.dlyNxt;
    inp.replacementCharacter = 0;
    inp.RFU1 = 0;
    inp.RFU2 = 0;
    memset(inp.values, 0, 4);

    if(*msg3) {
        adv.inputFieldDefinition = 0xFFF0;
        adv.inputFieldDefinition2 = 0x0000;
        adv.direction = 0;
        adv.minimumKeys = ppdCfg.min;
        adv.maximumKeys = ppdCfg.max;
        adv.endKey = 0x00;
        adv.echoCharacter = ppdCfg.msk;
        adv.parameters1 = 0;
        adv.corKeyAction = 0;
        adv.timeOutFirstKey = ppdCfg.dly1st;
        adv.timeOutInterKey = ppdCfg.dlyNxt;
        adv.replacementCharacter = 0;
        memset(adv.values, 0, 4);
        adv.beepParameter = 0x00;

        ret =
            ssaSecPinEntryConfig(hSsa, SWITCH_AUTO, SSASECINPUT_ADV,
                                 (void *) &adv);
    }

    ret =
        ssaSecPinEntryDukpt3DESReq3(hSsa, 0, SWITCH_AUTO, ppdCfg.dukptMod, 0x03,
                                    (byte *) acc, &inp, &MCKey);

    CHECK(ret == RET_RUNNING, lblKO);

    getSsaRsp(&rsp, ppdCfg.dlyRsp);
    switch (rsp.sta) {
      case RET_OK:
          break;
      case ERR_USER_CANCEL_KEY:
      case ERR_TIMEOUT_FIRST_KEY:
      case ERR_TIMEOUT_INTER_KEY:
          return 0;
      default:
          goto lblKO;
    }
    if(rsp.buf[0] != kbdVAL)
        return 0;

    memcpy(SerialNumberKey, rsp.buf + 2, 10);
    memcpy(blk, rsp.buf + 12, 8);

    trcS("SerialNumberKey=");
    trcBN(SerialNumberKey, 10);
    trcS("\n");
    trcS("blk=");
    trcBN(blk, 8);
    trcS("\n");

    return rsp.buf[1];
  lblKO:
    trcErr(ret);
    return -1;
}
#endif
#endif

/** Retrieve SAP code and Serial number of terminal
 * \param sap (O) char[16+1] S.A.P. code of the terminal
 * \param ser (O) char[12+1] Serial number of the terminal
 * \param dvc (I) device code:
 *  - 't' or 0: terminal (zero means 'default')
 *  - 'p': pinpad
 * \return non-negative value if OK; negative otherwise
 *
 * If one of the input pointer is zero, the value is not retrieved
 *
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcab0050.c
 */
int getSapSer(char *sap, char *ser, char dvc) {
    ssaStaticConf2_t cfg;
    byte idx;

    if(sap)
        memset(sap, 0, 16 + 1);
    if(ser)
        memset(ser, 0, 12 + 1);

    // Get the SSA identification structure 2
    if(ssaIdent2(&cfg) != RET_OK)
        return -1;

    idx = 0;
    switch (dvc) {
      case 't':
          idx = 0;
          break;
      case 'p':
          idx = 1;
          break;
      default:
          idx = 0;
          break;
    }
    //VERIFY(cfg.ssaNb >= idx + 1);
    CHECK(cfg.ssaNb >= idx + 1, lblKO); //Since we can not simulate pin pad in simulator VERIFY is changed to  CHECK
#ifndef WIN32
    if(ser)
        memcpy(ser, cfg.descriptor[idx].serialNumber, 12);
    if(sap)
        memcpy(sap, cfg.descriptor[idx].sapCode, 16);
#else
    if(ser)
        strcpy(ser, "123456789012");
    if(sap)
        strcpy(sap, "SAP0123456789012");
#endif
    return 1;
  lblKO:
    return -1;
}

/** 
 * To display 2 line message in PinPad and to get selected key value.
 * \param msg (I) Contains the messages to be displayed on the first,second lines separated by '\n'
 * \param lngIndicator (O) is the response from the user
 * \return  
 *    - 1 the last key pressed (kbdVal) is OK.
 *    - 0 the last key pressed (kbdVal) is CANCEL.
 *    - negative if others.
 * 
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcda0004.c
*/
int ppdDspKbdKey(char *msg, char *lngIndicator) {
    int16 ret;
    ssaSecDispKbdInput_t inp;
    ssaSig_t sig;
    tResponse rsp;

    char msg1[dspW + 1];
    char msg2[dspW + 1];
    byte idx;

    VERIFY(msg1);
    VERIFY(msg2);

    memset(msg1, 0, dspW + 1);
    memset(msg2, 0, dspW + 1);

    idx = 0;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg1[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    idx = 0;
    msg++;
    while(*msg != '\n') {
        VERIFY(idx <= dspW);
        msg2[idx++] = *msg++;
    }
    VERIFY(*msg == '\n');

    if(*msg1) {
        trcFS("msg1=%s\n", msg1);
    }
    if(*msg2) {
        trcFS("msg2=%s\n", msg2);
    }

    memset(&rsp, 0, sizeof(rsp));

    sig.length = 0;

    memset(inp.values, 0, 4);
    inp.RFU1 = 0;
    inp.RFU2 = 0;
    inp.replacementCharacter = 0;
    inp.fontText = 0;
    inp.fontInput = 0;

    inp.lineNbText = 0;
    inp.lineNbInput = 1;
    memcpy(inp.text, msg1, strlen(msg1));
    inp.text[strlen(msg1)] = 0;
    memset(inp.textInput, 0, 16);
    memcpy(inp.textInput, msg2, strlen(msg2));
    inp.textInput[strlen(msg2)] = 0;

    inp.minimumKeys = 1;
    inp.maximumKeys = 2;
    inp.endKey = 0;
    //inp.inputFieldDefinition = 0x0FFF;
    inp.inputFieldDefinition = 0x0001;
    inp.direction = 1;
    inp.echoCharacter = 0;
    inp.corKeyAction = 0;
    inp.parameters1 = 0;
    inp.timeOutFirstKey = 10;
    inp.timeOutInterKey = 5;
    ret = ssaSecLafMultipleInputToReq(hSsa, SWITCH_AUTO, &inp, &sig);
    CHECK(ret == RET_RUNNING, lblKO);
    while((ret = ssaSecResultGet(hSsa, sizeof(rsp), &rsp)) != RET_OK);
    *lngIndicator = rsp.buf[2];
    switch (rsp.sta) {
      case RET_OK:
          break;
      case ERR_USER_CANCEL_KEY:
      case ERR_TIMEOUT_FIRST_KEY:
      case ERR_TIMEOUT_INTER_KEY:
          return 0;
      default:
          goto lblKO;
    }

    if(rsp.buf[0] == kbdVAL) {
        return 1;
    } else if(rsp.buf[0] != kbdVAL) {
        return 0;
    }

  lblKO:
    return -1;
}

/** 
 * To load a general key (transport key) from Tellium Smartcard.
 * \param loc (I) Contains the SSA location where the key should be loaded. Must be less then 0x50.
 * \param cardData (I) data obtained from Tellium transport card.
 * \return  
 *    - 0 succesful key load (RET_OK)
 *    - negative problem.
 * 
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tcts0007.c
 */

int cryLoadTK(byte loc, const byte * cardData) {
#if(__SSA__ >= 561)             /* final SSA release is not known yet. Current working beta is 561 */
    ssaSecKeyDescription_t kDsc;

    VERIFY(loc < 0x50);
    VERIFY(cardData);

    kDsc.array = loc / 16;
    kDsc.slot = loc % 16;

#ifdef WIN32
    return cryLoadTestKey(loc); //simulator not tested, usually not worked
#else
    return ssaSecLoadSkmtTk(hSsa, &kDsc, SKMT1, 16, (uint8 *) & cardData[16],
                            (uint8 *) & cardData[36], cardData[35]);
#endif                          //WIN32
#else
    return -1;
#endif                          //SSA
}
#endif
