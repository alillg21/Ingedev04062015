/** \file
 * Cryptographic functions for APR environment
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/aprcry.c $
 *
 * $Id: aprcry.c 2478 2009-10-28 16:39:49Z abarantsev $
 */

#include "sys.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcC32))

#ifdef __APR__
#include <Sec.h>
#include <pinpad.h>
#include <ssaStd.h>
static secHandle_t SessionID;
static byte locKCA = 0;
static byte locKPE = 0;
static byte locRoot = 0;

/** Open the associated channel.
 * Should be called before calling any cryptomodule processing
 * \param  mod (I) not used
 * \return
 *    - negative if failure.
 *    - non-negative if OK.
 * \header sys\\sys.h
 * \source sys\\aprsry.c
 * \test tcik0164.c
 */
int cryStart(char mod) {
    int ret;

    // Open the Session
    ret = secSessionOpen(&SessionID);
    CHECK(ret == RET_OK, lblKO);
    trcFN("cryStart: ret=%d\n", ret);

    ret = secSetTag(SessionID, SEC_SPONSOR_NAME, 4, "ISEA");
    CHECK(ret == RET_OK, lblKO);

    ret = secSetTag(SessionID, SEC_ACQUIRER_NAME, 17, "GENERIC DEBIT PIN");
    CHECK(ret == RET_OK, lblKO);

    ret = secSetTag(SessionID, SEC_MERCHANT_NUMBER, 7, "DEFAULT");
    CHECK(ret == RET_OK, lblKO);

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
 * \header sys\\sys.h
 * \source sys\\aprsry.c
 * \test tcik0164.c
 */
int cryStop(char mod) {
    int ret;

    if(SessionID) {
        ret = secSessionClose(SessionID);
        CHECK(ret == RET_OK, lblKO);
    }
    SessionID = 0;
    return SessionID;
  lblKO:
    trcErr(ret);
    return -1;
}

static uint8 const *cryGetKey(byte loc, char prp) {

    if(prp)
        goto lblPrp;

    if(loc == locKCA)
        return SEC_KCA;

    if(loc == locKPE)
        return SEC_KPE;

    if(loc == locRoot)
        return SEC_ROOT;
    else
        return SEC_KCA;

  lblPrp:
    switch (prp) {
      case 'p':
          locKPE = loc;
          return SEC_KPE;
      case 'r':
          locRoot = loc;
          return SEC_ROOT;
      case 'g':
      default:
          locKCA = loc;
          return SEC_KCA;
    }
}

/** Inject the key 1111111111111111 into the location loc.
 * Using this key it is possible to download various keys without using a standard certified injection
 * procedure.
 * The function below can be used only for testing purposes.
 * In real applications the keys should be downloaded in a proper certified way.
 * \param loc (I) not used
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\aprcry.c
 * \test tcik0164.c
 */
int cryLoadTestKey(byte loc) {
    int ret;
    size_t len;
    byte key[16];

    trcFN("cryLoadTestKey loc=%02X\n", (word) loc);
    memset(key, 0, sizeof(key));

    // Load the keys using the proper Tag
    // KCA - Cross Aquirer Key (Used to Encrypt KEK)
    len = 8;
    ret = secGetTag(SessionID, cryGetKey(loc, 'r'), &len, &key);
    CHECK(ret != RET_OK, lblEnd);   //key is already downloaded
    memcpy(key, "\x11\x11\x11\x11\x11\x11\x11\x11", 8);

    ret = secSetTag(SessionID, cryGetKey(loc, 'r'), len, key);
    CHECK(ret == RET_OK, lblKO);

  lblEnd:
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
 * \source sys\\aprcry.c
 * \test tcik0164.c
 */
int cryVerify(byte loc, byte * crt) {
    int ret;
    size_t len;

    VERIFY(crt);

    trcFN("cryVerify loc=%02X\n", (word) loc);

    len = 3;                    // 3 bytes certificate only
    ret = secGetTag(SessionID, cryGetKey(loc, 0), &len, crt);
    CHECK(ret == RET_OK, lblKO);

    //trcS("crt= ");
    //trcBN(crt, 4);
    //trcS("\n");

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
 * \param trn (I) not used
 * \param key (O) The result of EDE encrypting of the working key by the transport key.
 * \param prp (I) not used
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\aprsry.c
 * \test tcik0165.c
 */
int cryLoadDKuDK(byte loc, byte trn, const byte * key, char prp) {
    int ret;
    size_t len;

    VERIFY(key);

    len = 8;
    ret = secSetTag(SessionID, cryGetKey(loc, prp), len, key);
    CHECK(ret == RET_OK, lblKO);

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
 * \source sys\\aprcry.c
 * \test tcik0167.c
 */
int cryGetPin(const char *msg, const byte * acc, byte loc, byte * blk) {
    int ret;
    char msg1[dspW + 1];
    char msg2[dspW + 1];
    char msg3[dspW + 1];
    byte buf[8];
    byte idx;
    size_t len;

    VERIFY(msg1);
    VERIFY(msg2);
    VERIFY(acc);
    VERIFY(blk);

    trcFN("cryGetPin: loc=%02X ", loc);
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
        pp_display_clear(0);
        pp_display_text(0, 0, msg1, 0);
    }
    if(*msg2) {
        trcFS("msg2=%s\n", msg2);
        pp_display_clear(1);
        pp_display_text(1, 0, msg2, 0);
    }
    if(*msg3) {
        trcFS("msg3=%s\n", msg3);
        pp_display_clear(2);
        pp_display_text(2, 0, msg3, 0);
    }

    secSetTag(SessionID, SEC_PAN, 16, acc);

    ret = pp_pin_entry(0, 8, 1 << 26, 100);
    //CHECK(ret == RET_OK, lblKO);

    len = sizeof(buf);
    memset(buf, 0, len);
    ret = secGetTag(SessionID, SEC_STANDARD_ENCRYPTED_PIN, &len, buf);
    //CHECK(ret == RET_OK, lblKO);

    memcpy(blk, buf, len);

    trcS("blk=");
    trcBN(blk, 8);
    trcS("\n");

    return ret;
//  lblKO:
//    trcErr(ret);
//    return -1;
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
    int ret;
    char msg1[dspW + 1];
    char msg2[dspW + 1];
    char msg3[dspW + 1];
    byte idx;
    uint8 PINlen[4];
    size_t len;
    psyPid_t Pid;
    uint8 pinResult[2];

    VERIFY(msg);

    memset(msg1, 0, dspW + 1);
    memset(msg2, 0, dspW + 1);
    memset(msg3, 0, dspW + 1);

    trcS("cryEmvPin");

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

    if(*msg1) {
        trcFS("msg1=%s\n", msg1);
        pp_display_clear(0);
        pp_display_text(0, 0, msg1, 0);
    }
    if(*msg2) {
        trcFS("msg2=%s\n", msg2);
        pp_display_clear(1);
        pp_display_text(1, 0, msg2, 0);
    }
    if(*msg3) {
        trcFS("msg3=%s\n", msg3);
        pp_display_clear(2);
        pp_display_text(2, 0, msg3, 0);
    }
    len = sizeof(PINlen);

  lblPin:
    ret = pp_pin_entry(0, 8, KBD_COL_10 | KBD_LINE_4, 0);
    switch (ret) {
      case 0:
          return 0;
      case VK_ENTER:
          secGetTag(SessionID, SEC_PIN_LENGTH, &len, PINlen);
          len =
              (PINlen[3] * 0x1000000) + (PINlen[2] * 0x10000) +
              (PINlen[1] * 0x100) + PINlen[0];
          if(len >= 4) {
              Pid = getPid();
              ret = pp_emv_verify_pin(Pid);
              pinResult[0] = (ret / 0x100) % 0x100;
              pinResult[1] = ret % 0x100;
              //return pinResult[0];
              return kbdVAL;    //changed to kbdVAL to conform with tcab0017 , as MP suggested
          }
          goto lblPin;
      default:
          return ret;
    }
    return ret;
}
#endif

static int32 TermInfoGetSerialNumber(char *serial_num, int len) {
    psyManufacturingControls_t pbuffer;
    char *p;

    if(psyCfgIdentify(PSY_MANUFACTURING_CONTROLS,
                      PSY_MANUFACTURING_CONTROLS_SIZE, &pbuffer) != RET_OK) {
        return -1;
    }

    p = pbuffer.terminalSerialNb;

    /* Remove leading spaces */
    while(*p == ' ')
        p++;

    if(!strlen(p))
        return -1;

    if(len < 12 || len < strlen(p))
        return -1;

    strcpy(serial_num, p);
    return RET_OK;
}

/** Retrieve SAP code and Serial number of terminal
 * \param sap (O) char[16+1] S.A.P. code of the terminal; not implemented in APR
 * \param ser (O) char[12+1] Serial number of the terminal
 * \param dvc (I) device code: (not treated in APR)
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
    int ret;

    if(sap)
        memset(sap, 0, 16 + 1);

    if(ser) {
        memset(ser, 0, 12 + 1);
        ret = TermInfoGetSerialNumber(ser, 12);
        CHECK(ret == RET_OK, lblKO);
    }

    return 1;
  lblKO:
    return -1;
}

#endif
