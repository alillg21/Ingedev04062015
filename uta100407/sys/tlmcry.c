/** \file
 * TELIUM security schemes wrappers
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlmcry.c $
 *
 * $Id: tlmcry.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "sdk30.h"              // EFT30 SDK includes
//#include "pp30_def.h"
#include "SEC_interface.h"
#include "schVar_def.h"         // scheme includes
#include "tlvVar_def.h"

#include "sys.h"
#include "log.h" // @@OA
#include "bri.h" //bri v.3.04

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcC32))

typedef struct {
    byte line1;                 //1st display line
    byte line2;                 //2nd display line
    byte line3;                 //3rd display line
    byte min;                   //in hex min length of pincode 4
    byte max;                   //in hex max length of pincode 12
    //int MinMax;               //in hex min and max length of pincode 4 and 12 value:0x0000040A
    char msk;                   //mask character
    byte inp;                   //input line number
    //word col;                 //column mask line
    byte dly1st;                //1st character timeout in sec
    byte dlyNxt;                //Intercharacter timeout in sec
    //byte dlyRsp;              //response waiting delay
} tPpd;

//static tPpd ppdCfg = { 1, 2, 0, 0x0000040A, '*', 2, 6, 60};
//static tPpd ppdCfg = { 0, 1, 0, 4, 12, '*', 2, 6, 60 };
static tPpd ppdCfg = { 0, 1, 0, 4, 6, '*', 2, 40, 60 };
static word pinLen = 0;
typedef struct {
    byte Booster;               //if pinpad connected ppd booster, else terminal
    byte ppd;
    byte ppdSize;
} tSecArea;

static tSecArea secCfg;

#define CRY_MAX_NB_PARTS 10

static int cryLoadDukptDrvKey(byte loc, byte trn, const byte * key);

/** Get Terminal Secret Areas configuration.
 *
 * \return
 *    - negative if failure.
 *    - non-negative if OK.
 * \source sys\\tlmcry.c
 */
static int cryGetConfig() {
    int ret;
    T_SEC_CONFIG SecCfg[CRY_MAX_NB_PARTS];
    int num;
    byte i;
#ifdef _DEBUG_MK
	byte prtBuf[50];
#endif

    ret = SEC_GetConfig(&num, SecCfg);
#ifdef _DEBUG_MK
	sprintf(prtBuf,"SEC_GetConfig=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret == OK, lblKO);
    trcFN("SEC_GetConfig: %d ", ret);

    secCfg.ppd = 0;
    secCfg.Booster = SecCfg[0].cBoosterType;    //C_SEC_PINCODE

    for (i = 0; i < num; i++) {
        if(memcmp
           (SecCfg[i].ptszBoosterPeripheral, IAPP_USB_NAME,
            strlen(IAPP_USB_NAME)) == 0)
            secCfg.ppd = 1;
    }

    if(secCfg.ppd) {
        ret = PSQ_Pinpad_Value();
        switch (ret) {
          case P30_PPAD:
              secCfg.ppdSize = 8;
              break;
          default:
          case PP30_PPAD:
          case PP30S_PPAD:
          case PPC30_PPAD:
          case PPR30_PPAD:
              secCfg.ppdSize = 2;
              break;
        }
    }

    return 1;
  lblKO:
    return -1;
}

/** Create Secret Area.
 *
 * The following defines from ctx.h are used:
 * - VAR_NUMBER_SIGN: the unique number given to a development team
 * - CARD_NUMBER_SIGN: the signing card number; there can be several cards given to a team
 * - ID_SCR_XXX_BANK: the bank secret area identifier
 * \param mod (I) not used in Telium
 * \return
 *    - negative if failure.
 *    - non-negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0021.c
 */
int cryStart(char mod) {
    int ret;
    static byte dllOK = 0;
#ifdef _DEBUG_MK
	byte prtBuf[50];
#endif

    if(!dllOK) {
        ret = SEClib_Open();
#ifdef _DEBUG_MK
        sprintf(prtBuf,"SEClib_Open=%d",ret);prtS(prtBuf);
#endif
        CHECK(ret == 0, lblKO);
        dllOK = 1;
    }

    ret = cryGetConfig();
#ifdef _DEBUG_MK
    sprintf(prtBuf,"cryGetConfig=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret >= 0, lblKO);

    ret = SEC_isSecretArea(C_SEC_PINCODE, (SEG_ID) ID_SCR_XXX_BANK);
    if(ret == OK)
        return 1;               //Secret Area alredy created

    ret = SEC_CreateSecretArea( //give permission access for a secret area in a BL1 booster; If the SecureType has been configured to BL2 booster, no treatment is done.
                                  C_SEC_PINCODE,    //secureType Secure part to use for high level functions relative to the pincode entries
                                  (SEG_ID) ID_SCR_XXX_BANK, //Secret Area Id
                                  CARD_NUMBER_SIGN, //RSA card number
                                  VAR_NUMBER_SIGN   //VAR Id number
        );
#ifdef _DEBUG_MK
	sprintf(prtBuf,"SEC_CreateSecretArea=%d",ret);prtS(prtBuf);
	sprintf(prtBuf,"ID_SCR=%08X",ID_SCR_XXX_BANK);prtS(prtBuf);
	sprintf(prtBuf,"CARDSIGN=%08X",CARD_NUMBER_SIGN);prtS(prtBuf);
	sprintf(prtBuf,"VARID=%08X",VAR_NUMBER_SIGN);prtS(prtBuf);
#endif
    CHECK(ret == OK, lblKO);
    trcFN("SEC_CreateSecretArea (C_SEC_PINCODE): %d ", ret);

    return 1;
  lblKO:
    return -1;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control to another application.
 * \param mod (I) not used in Telium
 * \return
 *    - negative if failure.
 *    - non-negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0021.c
 */
int cryStop(char mod) {

    return 1;
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
    int ret = 0;
    T_SEC_DATAKEY_ID EraseId;
#ifdef _DEBUG_MK
    byte prtBuf[50];
#endif

    EraseId.iSecretArea = ID_SCR_XXX_BANK;
    EraseId.cAlgoType = 0;
    EraseId.usNumber = array;
    EraseId.uiBankId = 0x80000200 + array * 2;

    ret = SEC_FreeSecret(C_SEC_PINCODE, &EraseId);
#ifdef _DEBUG_MK
    sprintf(prtBuf,"cryEraseKeysArray=%d",ret);prtS(prtBuf);
#endif
    CHECK(ret == OK, lblKO);

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
 * \param loc (I)  real value of location must be devided by 8.
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0021.c
 */
int cryLoadTestKey(byte loc) {
    int ret = 0;
    unsigned char key[TDES_KEY_SIZE + 1];
    T_SEC_DATAKEY_ID LoadId;
    int usage;

    LoadId.iSecretArea = ID_SCR_XXX_BANK;
    LoadId.cAlgoType = TLV_TYPE_KTDES;  // This ROOT key is a TDES Key
    LoadId.usNumber = loc * 8;  //ROOT key location
    LoadId.uiBankId = 0x80000200 + loc * 2;
    usage = CIPHERING_KEY;      // Key to cipher other key

    switch (secCfg.Booster) {
      case C_SEC_BL1:
          memcpy(key,
                 "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",
                 TDES_KEY_SIZE + 1);
          ret = SEC_LoadKey(    //load a key in a secret area
                               C_SEC_PINCODE,   //Secure part to treat
                               NULL,    //Key Id of the ciphering key
                               &LoadId, //Key Id to load
                               (unsigned char *) key,   //DES/TDES Ciphered key value (its length is 8 or 16 bytes)
                               usage    //Key usage (CIPHERING_KEY, CIPHERING_PIN)
              );
          break;
      case C_SEC_BL2:

#if defined (SST_DEBUG) || defined (SAT_DEBUG)
          memcpy(key,
                 "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",
                 TDES_KEY_SIZE + 1);
          ret = SEC_LoadKey(    //load a key in a secret area
                               C_SEC_PINCODE,   //Secure part to treat
                               NULL,    //Key Id of the ciphering key
                               &LoadId, //Key Id to load
                               (unsigned char *) key,   //DES/TDES Ciphered key value (its length is 8 or 16 bytes)
                               usage    //Key usage (CIPHERING_KEY, CIPHERING_PIN)
              );
#else
          trcS("Root Key must be loaded with SKMT2, key ID = 800002FC");
#endif
          break;
      default:
          goto lblKO;
    }

    CHECK(ret == OK, lblKO);
    return ret;
  lblKO:
    return -1;
}

/** Calculate key certificate.
 * The key certificate is the first four bytes of encryption of the value 0000000000000000 by the key.
 * This certificate can be used to ensure that the key at a given location was not changed
 *  after initial downloading.
 * \param loc (I) The location of  the key, must be devided by 8.
 * \param crt (O) The resulting certificate. It is the pointer to an array of 4 bytes.
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0021.c
 */
int cryVerify(byte loc, byte * crt) {
    int ret = 0;
    unsigned char dat[TDES_KEY_SIZE];
    unsigned char rsp[TDES_KEY_SIZE];
    T_SEC_DATAKEY_ID key;
    T_SEC_PARTTYPE zone;
    int len;

    VERIFY(crt);

    zone = C_SEC_PINCODE;

    key.iSecretArea = ID_SCR_XXX_BANK;
    key.cAlgoType = TLV_TYPE_KTDES;
    key.usNumber = loc * 8;
    key.uiBankId = 0x80000200 + loc * 2;

    if(crt[0] == 'm') {
        key.cAlgoType = TLV_TYPE_KDES;
        zone = C_SEC_CIPHERING;
    }
    if(crt[0] == 'd') {
        key.cAlgoType = TLV_TYPE_DESDUKPT;
    }

    memset(dat, 0, sizeof(dat));
    len = sizeof(rsp);

    switch (secCfg.Booster) {
      case C_SEC_BL1:
          ret = SEC_ECBCipher(  //DES/TDES ciphering/deciphering data in ECB mode (Electronic Code Book)
                                 &key,  //Key Id of the key to use
                                 C_SEC_CIPHER_FUNC, //C_SEC_CIPHER_FUNC or C_SEC_DECIPHER_FUNC
                                 dat,   //Data buffer to cipher/decipher
                                 TDES_KEY_SIZE, //Length in bytes of the Input Data buffer
                                 rsp,   //Output Data buffer
                                 &len   //Length in bytes of the Output Data buffer
              );
          break;
      case C_SEC_BL2:
#if defined (SST_DEBUG) || defined (SAT_DEBUG) || defined (SAT_PROD)
		  ret = OK;
#else
          ret = SEC_KeyVerify(zone, &key,   //Key Id of the key to use
                              rsp,  //Output Data buffer
                              &len  //Length in bytes of the Output Data buffer
              );
#endif
          break;
      default:
          goto lblKO;
    }
    CHECK(ret == OK, lblKO);

    memcpy(crt, rsp, 4);
    trcS("crt= ");
    trcBN(crt, 4);
    trcS("\n");

    return OK;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Load double key key of property prp into location loc using a double transport key located at the location trn.
 * The transport key must be downloaded into trn before calling this function.
 * The value key should be calculated as a result of triple DES encryption of the working key to use by the transport key.
 * During this function call these data are decrypted using the transport key located at trn
 * and the result is saved at loc.
 * Usually this function is called to update working pinblock or MAC key.
 * \param loc (I) The location of  the key to be loaded into the cryptomodule.
 * \param trn (I) The location of  the transport key.
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
 * \source sys\\tlmcry.c
 * \test tcab0022.c
 */

int cryLoadDKuDK(byte loc, byte trn, const byte * key, char prp) {
    int ret = 0;
    T_SEC_DATAKEY_ID dscTrn;    //transport key descriptor
    T_SEC_DATAKEY_ID dscKey;    //key descriptor (to be downloaded)
    int usage = 0;
    T_SEC_PARTTYPE zone;

    zone = C_SEC_PINCODE;

    dscTrn.iSecretArea = ID_SCR_XXX_BANK;
    dscTrn.cAlgoType = TLV_TYPE_KTDES;
    dscTrn.usNumber = trn * 8;
    dscTrn.uiBankId = 0x80000200 + trn * 2;

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
    dscKey.cAlgoType = TLV_TYPE_KTDES;  // This PIN key is a TDES Key or redoubled DES
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    switch (prp) {
      case 'g':
          usage = CIPHERING_KEY;
          break;
      case 'm':
          dscKey.cAlgoType = TLV_TYPE_KDES; //For MAC key only DES is accepted!
          usage = CIPHERING_DATA;
          zone = C_SEC_CIPHERING;
          break;
      case 'p':
          usage = CIPHERING_PIN;
          break;
      case 'd':
          ret = cryLoadDukptDrvKey(loc, trn, key);
          return ret;
      default:
          break;
    }
    //VERIFY(usage); //Removed because always fail for CIPHERING_DATA

    ret = SEC_LoadKey(zone, &dscTrn, &dscKey, (unsigned char *) key, usage);
    CHECK(ret == OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

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
 * \return  N/A
 * \pre
 *  - strlen(cfg)==12
 *  - cfg[0],cfg[1],cfg[2],cfg[3] are decimal characters
 *  - cfg[4] is a hexadecimal character
 *  - cfg[6]-cfg[11] are decimal characters
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0023.c
 */
void crySetPpd(const char *cfg) {
    byte tmp;
    byte bit[2];
    card num;
    int max, min;

    if(!cfg)
        cfg = "1204C2301060";   //default configuration
    VERIFY(strlen(cfg) == 12);

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
    min = cfg[3] - '0';
    //ppdCfg.MinMax= min<<2;
    ppdCfg.min = min;

    tmp = cfg[4];
    VERIFY('0' <= tmp);
    if(tmp > '9') {
        VERIFY('A' <= tmp);
        VERIFY(tmp <= 'F');
        tmp -= 'A';
        tmp += 10;
    } else
        tmp -= '0';
    //ppdCfg.MinMax += tmp;
    max = tmp;
    ppdCfg.max = max;

    ppdCfg.msk = '*';
    ppdCfg.inp = 2;
    if('0' <= cfg[5] && cfg[5] <= '9')
        ppdCfg.inp = cfg[5] - '0';
    else
        ppdCfg.msk = cfg[5];

    memset(bit, 0, 2);
    tmp = max;
    while(tmp--) {              //calculate the set of columns to accept input
        bitOn(bit, (byte) (16 - tmp));
    }
    //ppdCfg.col = WORDHL(bit[0], bit[1]);

    dec2num(&num, cfg + 6, 2);
    ppdCfg.dly1st = (byte) num;

    dec2num(&num, cfg + 8, 2);
    ppdCfg.dlyNxt = (byte) num;

    dec2num(&num, cfg + 10, 2);
    //ppdCfg.dlyRsp = (byte) num;
}

/** Clear pin pad display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\c32.c
 * \test tctd0003.c
 */
int ppdClear() {

    PPS_clear_display();
    PPS_firstline();

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
    int idx;
    static int curLine = 0;

    VERIFY(str);
    trcFN("ppdLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    if(loc > (secCfg.ppdSize - 1))
        loc = loc % (secCfg.ppdSize);

    idx = abs(curLine - loc);

    while(idx) {
        PPS_newline();
        idx--;
    }

    curLine = loc;
    PPS_clearline();
    PPS_Display((char *) str);

    return 1;
}

static int cryEnterPin(const char *msg) {
    int ret = 0;
    T_SEC_ENTRYCONF cfg;
    unsigned int evt;
    int ctl;                    //control parameter: to continue or not the PIN entry
    char key;
    byte idx = 0;
    char buf[dspW + 1];
    char *pBuf = buf;
    char *mBuf = buf;
	byte txnType; // @@OA
	byte jeniskartu; //bri v.3.04

    memset(buf, 0, sizeof(buf));
    while(*msg) {
        if(*msg == '\n') {
            msg++;              // skip separator
            break;
        }
        *pBuf++ = *msg++;
    }
    if(secCfg.ppd) {
        ppdClear();
        ppdLS(ppdCfg.line1, buf);
    } else
        dspLS(ppdCfg.line1, buf);

    memset(buf, 0, sizeof(buf));

    while(*msg) {
        if(*msg == '\n') {
            msg++;              // skip separator
            break;
        }
        *mBuf++ = *msg++;
    }
    dspLS(ppdCfg.line2, buf);

    if((*msg) && (*msg != '\n'))
        dspLS(ppdCfg.line3, msg);

    cfg.ucEchoLine = ppdCfg.inp;
    cfg.ucEchoColumn = 0;
    cfg.ucEchoChar = ppdCfg.msk;
//@@OA    cfg.ucMinDigits = 0;
//@@OA    cfg.ucMaxDigits = ppdCfg.max;
	MAPGETBYTE(traTxnType,txnType,lblKO);
	switch(txnType)
	{
	    case trtSale:
	    case trtSaleRedeem: //@ar - BRI
	    case trtPreAut:
	    case trtAut:
	    case trtRefund:
	    case trtInstallment:
	    case trtVoid:
		case trtInfoSaldoBankLain:
		case trtTransferAntarBankInquiry:
			cfg.ucMinDigits = 4;
			break;
		default:
			cfg.ucMinDigits = 6;
			break;
	}

	//bri v.3.04
	MAPGETBYTE(traKategoriKartu, jeniskartu, lblKO);
	if(jeniskartu == NON_BRI_DEBIT_CARD)
		cfg.ucMinDigits = 4;
	//=======

	cfg.ucMaxDigits = 6;
    cfg.iFirstCharTimeOut = 1000 * ppdCfg.dly1st;
    cfg.iInterCharTimeOut = 1000 * ppdCfg.dlyNxt;

    trcS("SEC_PinEntryInit\n");
    ret = SEC_PinEntryInit(     //initialise the parameters for the secure pincode entry
                              &cfg, //Pincode entry configuration
                              C_SEC_PINCODE //(expected value for PIN offline processing by old versions of EMV-DC is C_SEC_CARD
        );
    CHECK(ret == OK, lblKO);

    evt = KEYBOARD;
    ctl = 1;
    idx = 0;
    memset(buf, 0, sizeof(buf));
    while(ctl) {
        trcS("SEC_PinEntry\n");
        ret = SEC_PinEntry(     //manage the secure pincode entry
                              &evt, //Events to wait during the pin entry/Received events
                              &key, //Last input key or 0
                              &ctl  //Input : FALSE to stop the pincode entry treatment; TRUE to continue the pincode entry; Output : TRUE if the pincode Entry is halted
            );
        CHECK(ret == OK, lblKO);

        if(key == ppdCfg.msk) {
            if(idx < MAX_PIN_CODE_SIZE) {
                buf[idx++] = key;
                if(secCfg.ppd)
                    ppdLS(ppdCfg.inp, buf);
                else
                    dspLS(ppdCfg.inp, buf);
            } else
                ctl = 0;
        } else {
            switch (key) {
              case 0x01:       //CANCEL Response
              case T_ANN:      // CANCEL Key
                  ctl = 0;
                  ret = 0;
                  goto lblEnd;
                  //break;
              case T_VAL:
                  ctl = 0;
                  trcFN(" PIN valid(%d key)\n", idx);
                  break;
              case T_CORR:
                  if(idx != 0) {
                      buf[--idx] = 0;
                      if(secCfg.ppd)
                          ppdLS(ppdCfg.inp, buf);
                      else
                          dspLS(ppdCfg.inp, buf);
                  }
                  break;
              default:
                  trcFN(" No PIN Entered(%d key)\n", idx);
                  CHECK(idx == 0, lblKO);   //check if a key was entered
                  ret = 0;      // timeout & no key entered
                  goto lblEnd;
                  break;
            }
        }
    }
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("ret = %d\n", ret);
    pinLen = (word) idx;
    return ret;
}

word getPinLen(void) {
    return pinLen;
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
 * \param blk (O) The resulting pin block, the pointer to a buffer containing 8 bytes
 * \return
 *    - number of digits entered if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0023.c
 */
int cryGetPin(const char *msg, const byte * acc, byte loc, byte * blk) {
    int ret = 0;
    unsigned int len;
    T_SEC_DATAKEY_ID dscKey;

    ret = cryEnterPin(msg);
    CHECK(ret >= 0, lblKO);

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
    dscKey.cAlgoType = TLV_TYPE_KTDES;  // PIN key is a DES Key
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    ret = SEC_Iso9564(          //ISO9564 Pin ciphering (ANSI X9.8)
                         C_SEC_PINCODE, //Secure part used to store the CipherKey
                         &dscKey,   //Key Id of the ciphering key
                         ISO9564_F0_TDES,   //ISO9564 format
                         (unsigned char *) acc, //PAN value (8 bytes)
                         blk,   //Output Data buffer
                         &len   //Length in bytes of the Output Data buffer
        );
    CHECK(ret == OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    if (ret==ERR_PIN_CODE_NOT_FOUND) //@@SIMAS-PIN_BYPASS
    	ret=-2; // PIN not input'ed //@@SIMAS-PIN_BYPASS
    else //@@SIMAS-PIN_BYPASS
    	ret = -1;
  lblEnd:
    return ret;
}

/* Ask for an offline pin for an EMV card.
 * The pin entered is sent to EMV Kernel and a command APDU is generated and sent to the card. // ???
 * The pin is not visible to the application.
 * \param msg (I) Contains the messages to be displayed separated by '\n'
 * \return
 *    - the last key pressed (kbdVal) if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcab0017.c
 */
int cryEmvPin(const char *msg) {
    int ret = 0;

    ret = dspStart();
    CHECK(ret > 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = cryEnterPin(msg);
    CHECK(ret >= 0, lblKO);
    if(ret == 0)
        ret = kbdANN;
    else
        ret = kbdVAL;

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    dspStop();
    return ret;
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
 * \source sys\\tlmcry.c
 * \test tcab0061.c
 */
int cryMac(tBuffer * buf, byte loc, byte * mac) {
    int ret = 0;
    int len;
    unsigned char *msg;
    T_SEC_DATAKEY_ID MacKey;

    VERIFY(buf);
    VERIFY(bufLen(buf) >= 16);
    VERIFY(bufLen(buf) % 8 == 0);
    VERIFY(mac);
    trcFN("cryMac: loc=%d ", loc);
    trcS("buf=");
    trcBN(bufPtr(buf), bufLen(buf));
    trcS("\n");

    // length of buffer plus length of initial vector
    // Insert a initial vector (default value: 0x00)
    len = bufLen(buf);
    msg = (byte *) bufPtr(buf);

    MacKey.iSecretArea = ID_SCR_XXX_BANK;
    MacKey.cAlgoType = TLV_TYPE_KDES;   // MAC key is a DES Key (SEC DLL requirement)
    MacKey.usNumber = loc * 8;
    MacKey.uiBankId = 0x80000200 + loc * 2;

    ret = SEC_ComputeMAC(&MacKey, msg, len, NULL, mac);
    CHECK(ret == 0, lblKO);

    trcS("mac=");
    trcBN(mac, 8);
    trcS("\n");

    ret = 8;
    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:
    return ret;
}

/** Diagnostique Secret Area.
 * This function is called when other cry functions return KO
 * in case of non-empty memory and more then 1 secret area id
 * advise to reset booster memory
 * \param NbSecId (I) pointer to number of secret airea
 * \param tab (I) pointer to table for secret airea identifiers
 * \return
 *    - negative if failure.
 *    - number of secret area and thiers id if OK.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcik0150.c
 */
int cryDiag(int *NbSecId, int *tab) {
    int ret;
    int num;
    T_SEC_CONFIG SecCfg[10];

    ret = SEC_listSecureId(C_SEC_PINCODE, NbSecId, tab);
    CHECK(ret == OK, lblKO);
    trcFN("cryDiag (C_SEC_PINCODE): %d ", ret);

    ret = SEC_GetConfig(&num, SecCfg);
    CHECK(ret == OK, lblKO);
    trcFN("SEC_GetConfig: %d ", ret);

    return 1;
  lblKO:
    return -1;
}

/** Get Terminal Serial Number.
 * This function is for retrieving terminal serial number
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcjl000.c
 */
void tlmGetSer(char *SerialNum) {
    PSQ_Donner_noserie((char *) SerialNum); //get terminal serial number
}

/** Download Derivation Key before DUKPT Initialisation.
 * This function is call by cryLoadDKuDK with prp = 'd'
 * \param loc (I) The location of  the dukpt key downloaded into the cryptomodule.
 * \param trn (I) The location of  the transport key.
 * \param key (O) The result of EDE encrypting of the working key by the transport key.
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcik0177.c
 */
static int cryLoadDukptDrvKey(byte loc, byte trn, const byte * key) {
    int ret = 0;
    T_SEC_DATAKEY_ID dscTrn;    //transport key descriptor
    T_SEC_DATAKEY_ID dscKey;    //key descriptor (to be downloaded)
    T_SEC_PARTTYPE zone;

    zone = C_SEC_PINCODE;

    dscTrn.iSecretArea = ID_SCR_XXX_BANK;
    dscTrn.cAlgoType = TLV_TYPE_KTDES;
    dscTrn.usNumber = trn * 8;
    dscTrn.uiBankId = 0x80000200 + trn * 2;

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
#ifdef __TDES__
    dscKey.cAlgoType = TLV_TYPE_TDESDUKPT;  // This PIN key is a TDES Key or redoubled DES
#else
    dscKey.cAlgoType = TLV_TYPE_DESDUKPT;   // This PIN key is a TDES Key or redoubled DES
#endif
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    ret =
        SEC_DukptLoadDerivationKey(C_SEC_PINCODE, &dscKey, &dscTrn,
                                   (unsigned char *) key);
    CHECK(ret == OK, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Key management DUKPT Initialisation.
 * \param loc (I) The location of  the dukpt key into the cryptomodule.
 * \param SerialNumberKey (I) KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcik0177.c
 */
int cryDukptInit(byte loc, byte * SerialNumberKey) {
    int ret;
    T_SEC_DATAKEY_ID dscKey;    //key descriptor of DUKPT

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
    dscKey.cAlgoType = TLV_TYPE_DESDUKPT;   // This PIN key is a TDES Key or redoubled DES
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    ret = SEC_DukptLoadKSN(C_SEC_PINCODE, &dscKey, (char *) SerialNumberKey);
    CHECK(ret == OK, lblKO);

    ret = SEC_DukptGenerateInitialKey(C_SEC_PINCODE, &dscKey);
    CHECK(ret == OK, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Ask for PIN with DUKPT.
 * \param msg1 (I) Contains the message to be displayed in line 1
 * \param msg2 (I) Contains the message to be displayed in line 2
 * \param loc (I) The location of  the dukpt key downloaded into the cryptomodule.
 * \param acc (I) Account built on the PAN of the card in the following manner.
 *    At first an 16-characters ASCII string is built:
 *    - first four characters are zeroes
 *    - they are concatenated with 12 characters extracted from PAN
 *      (12 digits before Luhn code)
 *    - The string is converted into bcd format to obtain acc.
 * \param loc (I) The location of  the pin key downloaded into the cryptomodule.
 * \param SerialNumberKey (I) KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \param blk (O) The resulting pin block, the pointer to a buffer containing 8 bytes
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcik0177.c
 */

int cryDukptGetPin(const char *msg1, const char *msg2, const byte * acc,
                   word loc, byte * SerialNumberKey, byte * blk) {
    int ret;
    T_SEC_DATAKEY_ID dscKey;
    char msg[2 * prtW + 1];

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
    sprintf(msg, "%s\n%s\n", msg1, msg2);

    ret = cryEnterPin(msg);
    CHECK(ret >= 0, lblKO);

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
    dscKey.cAlgoType = TLV_TYPE_DESDUKPT;   // PIN key is a DES Key
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    ret = SEC_DukptEncryptPin(C_SEC_PINCODE,
                              &dscKey,
                              DUKPT_ENCRYPT_PIN,
                              (unsigned char *) acc,
                              &blk[0], &SerialNumberKey[0]
        );

    CHECK(ret == OK, lblKO);

    trcS("SerialNumberKey=");
    trcBN(SerialNumberKey, DUKPT_SMID_SIZE);
    trcS("\n");
    trcS("blk=");
    trcBN(blk, PIN_BLOC_SIZE);
    trcS("\n");

    ret = 1;
    return ret;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Key management 3DES DUKPT Initialisation
 * \param loc (I) The location of  the dukpt key downloaded into the cryptomodule.
 * \param SerialNumberKey (I) KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcbk0003.c
 */
int cry3DESDukptInit(byte loc, byte * SerialNumberKey) {
    int ret;
    T_SEC_DATAKEY_ID dscKey;    //key descriptor of DUKPT

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
    dscKey.cAlgoType = TLV_TYPE_TDESDUKPT;  // This PIN key is a TDES Key or redoubled DES
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    ret = SEC_isSecretArea(C_SEC_PINCODE, ID_SCR_XXX_BANK);
    CHECK(ret == OK, lblKO);

    ret = SEC_DukptLoadKSN(C_SEC_PINCODE, &dscKey, (char *) SerialNumberKey);
    CHECK(ret == OK, lblKO);

    ret = SEC_DukptGenerateInitialKey(C_SEC_PINCODE, &dscKey);
    CHECK(ret == OK, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Pin entry 3DES DUKPT : pin process (ANSI X9.24-2002)
 * \param msg1 (I) Message to be displayed on the top
 * \param msg2 (I) Message to be displayed on the second line
 * \param acc (I) Account number
 * \param loc (I) The location of  the pin key downloaded into the cryptomodule.
 * \param SerialNumberKey (O) Resulting KeySerialNumber 10 bytes long
 *    (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \param blk (O) Resulting pinblock, the pointer to a buffer containing 8 bytes
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tcbk0003.c
 */
int cry3DESDukptGetPin(const char *msg1, const char *msg2, const byte * acc,
                       word loc, byte * SerialNumberKey, byte * blk) {
    int ret;
    T_SEC_DATAKEY_ID dscKey;
    char msg[2 * prtW + 1];

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
    sprintf(msg, "%s\n%s\n", msg1, msg2);

    ret = cryEnterPin(msg);
    CHECK(ret >= 0, lblKO);

    dscKey.iSecretArea = ID_SCR_XXX_BANK;
    dscKey.cAlgoType = TLV_TYPE_TDESDUKPT;  // PIN key is a TDES Key
    dscKey.usNumber = loc * 8;
    dscKey.uiBankId = 0x80000200 + loc * 2;

    ret = SEC_DukptEncryptPin(C_SEC_PINCODE,
                              &dscKey,
                              DUKPT_ENCRYPT_PIN,
                              (unsigned char *) acc,
                              &blk[0], &SerialNumberKey[0]
        );

    CHECK(ret == OK, lblKO);

    trcS("SerialNumberKey=");
    trcBN(SerialNumberKey, DUKPT_SMID_SIZE);
    trcS("\n");
    trcS("blk=");
    trcBN(blk, PIN_BLOC_SIZE);
    trcS("\n");

    ret = 1;
    return ret;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Pin entry 3DES DUKPT : pin process (ANSI X9.24-2004)
 * T_SEC_DATAKEY_ID is populated by TSA application
 * \param SecretArea (I) SecretArea ID
 * \param AlgoType (I) Algorithm type (DES DUKPT, TDES DUKPT)
 * \param Number (I) Key/data number
 * \param BankId (I) Bank Identifier
 * \param msg1 (I) Message to be displayed on the top
 * \param msg2 (I) Message to be displayed on the second line
 * \param acc (I) Account number
 * \param SerialNumberKey (O) Resulting KeySerialNumber 10 bytes long
 *    (KET SET ID,TRMS ID,TRANSACTION COUNTER)
 * \param blk (O) Resulting pinblock, the pointer to a buffer containing 8 bytes
 * \return
 *    - 1 if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmcry.c
 * \test tchs0003.c
 */
int cryDukptGetPinTSA(const int SecretArea, const char AlgoType,
                      const word Number, const unsigned int BankId,
                      const char *msg1, const char *msg2, const byte * acc,
                      byte * SerialNumberKey, byte * blk) {
    int ret;
    char msg[2 * prtW + 1];
    T_SEC_DATAKEY_ID dscKey;

    VERIFY(SecretArea);
    VERIFY(AlgoType);
    VERIFY(Number);
    VERIFY(BankId);
    VERIFY(msg1);
    VERIFY(msg2);
    VERIFY(acc);
    VERIFY(blk);

    trcS("cryDukptGetPinTSA Begin\n");
    trcS("acc=");
    trcBN(acc, 8);
    trcS("\n");
    trcFS("msg1=%s\n", msg1);
    trcFS("msg2=%s\n", msg2);
    sprintf(msg, "%s\n%s\n", msg1, msg2);
    dscKey.iSecretArea = SecretArea;
    dscKey.cAlgoType = AlgoType;
    dscKey.usNumber = Number;
    dscKey.uiBankId = BankId;
    trcFN("iSecretArea:0x000%x\n", dscKey.iSecretArea);
    trcFN("cAlgoType:%d\n", dscKey.cAlgoType);
    trcFN("usNumber%d\n", dscKey.usNumber);
    trcFN("uiBankId:0x%x\n", dscKey.uiBankId);
    ret = cryEnterPin(msg);
    CHECK(ret >= 0, lblKO);

    ret = SEC_DukptEncryptPin(C_SEC_PINCODE,
                              &dscKey,
                              DUKPT_ENCRYPT_PIN,
                              (unsigned char *) acc,
                              &blk[0], &SerialNumberKey[0]
        );

    CHECK(ret == OK, lblKO);

    trcS("SerialNumberKey=");
    trcBN(SerialNumberKey, DUKPT_SMID_SIZE);
    trcS("\n");
    trcS("blk=");
    trcBN(blk, PIN_BLOC_SIZE);
    trcS("\n");
    trcS("cryDukptGetPinTSA End\n");
    ret = 1;
    return ret;
  lblKO:
    trcErr(ret);
    return -1;
}
