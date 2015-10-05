/** \file
 * TELIUM external devices processing
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlmext.c $
 *
 * $Id: tlmext.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "ctx.h"
#include "SDK30.H"
#include "sys.h"

#ifdef __MFC__
#ifndef __CLS__
#define __CLS__
#endif
#endif

#ifdef __BIO__
#include "oem_morpho.h"
#define MAX_FIELD_CNT   20
static T_TemplateList *bioRefTemp = NULL;
#endif
#ifdef __CLS__
//Old includes
//#include "oem_cless.h"          // Telium Pass include driver
//#include "oem_clmf1.h"          // Mifare
//#include "DataElement.h"
//#include "TPass.h"              // Telium Pass include DLL

#include "TlvTree.h"
#include "del_lib.h"
#include "bitmap.h"
#include "MSGlib.h"

// Cless includes
#include "oem_cless.h"
#include "oem_clmf1.h"

// Generic Tool Library includes
//#include "GTL_Assert.h"
#include "GTL_StdTimer.h"
#include "GTL_Traces.h"
#include "GTL_TagsInfo.h"
#include "GTL_BerTlv.h"
#include "GTL_BerTlvDecode.h"
#include "GTL_Convert.h"
#include "GTL_TagsInfo.h"
#include "GTL_DataStorage.h"
#include "GTL_SharedExchange.h"
#include "GTL_SHA.h"

#include "TPass.h"

// Security includes
#include "SEC_interface.h"

// EMV tags definition
#include "EmvLib_Tags.h"
#include "_emvdctag_.h"

// Entry Point includes
#include "EntryPoint_Tags.h"

// Common cless kernel includes
#include "Common_Kernels_Tags.h"
#include "Common_Kernels_API.h"

#ifdef __EMV__
#include "emv.h"

#ifdef __PAYPASS__
// PayPass kernel includes
#include "PayPass_Tags.h"
#include "PayPass_API.h"
#endif

#ifdef __PAYWAVE__
// payWave kernel includes
#include "payWave_API.h"
#include "payWave_Tags.h"
#endif

#ifdef __VISAWAVE__
#include "VisaWave_API.h"
#include "VisaWave_Tags.h"
#endif
#endif

// Buffers & Defines
#define C_INDEX_TEST_BASE						0x0999  // See global range
#define SERVICE_CUSTOM_KERNEL					C_INDEX_TEST_BASE + 0x0000
#define SHARED_KERNEL_BUFFER_SIZE	16384   /*!< Maximum size of the shared buffer exchanged between this application and the kernels. */
#define TAG_SAMPLE_ENCIPHERED_PIN_CODE		0x9F918800  /*!< Sample parameters : Enciphered pin code for on-line verification. */
//static infoData_Cless dataBuf ;
static TLV_TREE_NODE tlvTree;
T_SHARED_DATA_STRUCT *kernelBuffer = NULL;
T_SHARED_DATA_STRUCT *sharedBuffer = NULL;
T_SHARED_DATA_STRUCT *aidShared = NULL;
char cTest[128];

/** Open the contactless reader
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test TODO
 */
int clsStart(void) {
    int ret;
    static byte dllOK = 0;

    trcS("clsStart\n");
    if(!dllOK) {
    	//@@ADE 2014-08-07 Deprecated
        //ret = TPasslib_open();
    	//CHECK(ret == 0, lblKO);
        dllOK = 1;
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Stop the contactless reader and
 * Close the associated channel.
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test TODO
 */
void clsStop(void) {
    trcS("clsStop\n");          //nothing to do
}

/** Open the mifare card reader, but not wait the mifare card
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcab0208.c
 */
int mfcStart(void) {
    int ret;

    trcS("mfcStart\n");
    ret = clsStart();
    CHECK(ret > 0, lblKO);
    //@@ADE 2014-08-07 Deprecated
    //ret = MF_IsDriverOpened();
    ret = ClessMifare_IsDriverOpened();
    if(!ret) {
    	//@@ADE 2014-08-07 Deprecated
        //ret = MF_OpenDriver();
    	ret = ClessMifare_OpenDriver();
        CHECK(ret > 0, lblKO);
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Stop the mifare card reader and
 * Close the associated channel.
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcab0208.c
 */
void mfcStop(void) {
    trcS("mfcStop\n");
    //@@ADE 2014-08-07 Deprecated
    //MF_CloseDriver();
    ClessMifare_CloseDriver();
}

/** Detect mifare card
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcab0208.c
 */
int mfcDetect(byte dly) {
    int ret;
    int num;

    //@@ADE 2014-08-07 Deprecated
    //ret = MF_DetectCards(NULL, &num, dly);
    ret = ClessMifare_DetectCards(&num, dly);
    CHECK(ret == MF_OK, lblKO);

    //@@ADE 2014-08-07 Deprecated
    //ret = MF_ActiveCard(NULL, 0);
    ret = ClessMifare_ActiveCard(0);
    CHECK(ret == MF_OK, lblKO);

    ret = num;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("mfcDetect ret= %d\n", ret);
    return ret;
}

static byte keyId(byte ks) {
    switch (ks & 0xF0) {
      case 0xA0:
          ks = 0x60;
          break;                //key A
      case 0xB0:
          ks = 0x61;
          break;                //key B
      default:
          VERIFY(ks = 0xA0);
          break;                //error
    }
    return ks;
}

/** Authenticate mifare card
 * \param key (I) Key value (NULL means factory key)
 * \param kidsec (I) key ID and sector 0xKS, K - key A or B, S - sector to authenticate
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcab0208.c
 */
int mfcAut(const byte * key, byte kidsec) {
    int ret;
    static const byte *fackey = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

    if(!key)
        key = fackey;
    //@@ADE 2014-08-07 Deprecated
    //ret = MF_Authentication(NULL, keyId(kidsec), (byte *) key, kidsec & 0x0F);
    ret = ClessMifare_Authentication(keyId(kidsec), (byte *) key, kidsec & 0x0F);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcS("mfcAut(");
    trcBN(&kidsec, 1);
    trcS("): ");
    trcBN(key, 8);
    trcS("\n");
    return ret;
}

/** Load data from mifare card
 * \param dat (O) buffer to get data loaded, 16 bytes length
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \param fmt (I) format of data, 'D' data 16 bytes, 'V' value 4 bytes
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcab0208.c
 */
int mfcLoad(byte * dat, byte loc, byte fmt) {
    int ret;
    byte len;

    VERIFY(dat);
    switch (fmt) {
      case 'V':
          fmt = VALUE;
          len = 4;
          break;
      case 'D':
      default:
          fmt = DATA;
          len = 16;
          break;
    }
    memset(dat, 0, len);
    //@@ADE 2014-08-07 Deprecated
    //ret = MF_ReadBlock(NULL, fmt, loc / 0x10, loc % 0x10, dat);
    ret = ClessMifare_ReadBlock(fmt, loc / 0x10, loc % 0x10, dat);
    CHECK(ret == 0, lblKO);

    trcFN("mfcLoad loc=%02x: ", loc);
    trcBN(dat, len);
    trcS("\n");

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Save data to mifare card
 * \param data (I) data value
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \param fmt (I) format of data, 'D' data 16 bytes, 'V' value 4 bytes
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcik0159.c
 */
int mfcSave(const byte * data, byte loc, byte fmt) {
    int ret;
    byte blk;
    byte sec;
    char dataBuf[16];
    byte idx, len;

    memset(dataBuf, 0, sizeof(dataBuf));
    //memset(valueBuf,0,sizeof(valueBuf));
    sec = (loc & 0xF0) >> 4;
    blk = loc & 0x0F;

    if((!sec) && (!blk)) {      //manufacture data
        return 0;               //block read only
    };

    switch (fmt) {
      case 'V':
          fmt = VALUE;
          len = 4;
          break;
      case 'D':
      default:
          fmt = DATA;
          len = 16;
          break;
    }
    //len = 4;
    if(blk == 3) {
        len = 16;
    }
    for (idx = 0; idx < len; idx++) {
        dataBuf[idx] = data[idx];
    }

    //@@ADE 2014-08-07 Deprecated
    //ret = MF_WriteBlock(NULL, fmt, sec, blk, dataBuf);
    ret = ClessMifare_WriteBlock(fmt, sec, blk, dataBuf);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Value operation with mifare card
 * \param data (I) value
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \param cmd (I) Value command, increase or decrease
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcik0159.c
 */

static int mfcValue(byte * data, byte loc, byte cmd) {
    int ret;
    byte blk;
    byte sec;
    byte idx;
    char buf[32];

    memset(buf, 0, sizeof(buf));
    sec = (loc & 0xF0) >> 4;
    blk = loc & 0x0F;

    for (idx = 0; idx < 4; idx++) {
        buf[idx] = data[idx];
    }

    //@@ADE 2014-08-07 Deprecated
    //ret = MF_ValueOperation(NULL, cmd, sec, blk, blk, buf);
    ret = ClessMifare_ValueOperation(cmd, sec, blk, blk, buf);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Debit mifare card
 * \param data (I) Data value
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcik0159.c
 */

int mfcDebit(const byte * data, byte loc) {
    int ret;

    if(!data)
        data = (byte *) "\x00\x00\x00\x01";
    trcS("mfcDebit: ");
    trcBAN(data, 4);
    trcS("\n");
    ret = mfcValue((byte *) data, loc, MF_DECREMENT);
    return ret;
}

/** Credit mifare card
 * \param data (I) Data value
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcik0159.c
 */

int mfcCredit(const byte * data, byte loc) {
    int ret;

    if(!data)
        data = (byte *) "\x00\x00\x00\x01";
    trcS("mfcCredit: ");
    trcBAN(data, 4);
    trcS("\n");

    ret = mfcValue((byte *) data, loc, MF_INCREMENT);
    return ret;
}

/** Play bip sound in contactless reader device
 * \param dly (I) value, bip duration in milliseconds
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcik0161.c
 */
int clsBip(byte dly) {

    if(!dly)
        dly = 5;
    TPass_BuzzerBeep(500, 0xFF, dly);
    return 1;
}

/** LED manipulation in contactless reader device
 * \param ctl (I) value, data type 0xLT, L - LEDs to manupulate and T - manipulation type
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test TODO
 */
static int clsLEDN(byte ctl) {
    byte led, type;

    led = (ctl & 0xF0) >> 4;
    type = ctl & 0x0F;
    switch (type) {
      case 0x00:               //off
          TPass_LedsOff(led);
          break;
      case 0x0F:               //on
          TPass_LedsOn(led);
          break;
      default:                 //blink
          TPass_LedsBlink(led, type, type);
          break;
    }

    return 1;
}

/** LED manipulation in VENDING PASS reader device
 * \param ctl (I) of data type 0xLT, L - LEDs to manupulate and T - manipulation type
 * \param color (I): LED color
 *  - L format : LED1=0x01 , LED2=0x02, LED3=0x04, LED4 = 0x08 , LED1+LED2 = 0x03
 *  - T format : 0 stop, F on, dly for blink
 *  - C color  : REG=0x10 , YELLOW=0x10, GREEN=0x40, default: no color
 *
 * if input parameters atr zeros - turn off everything
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcns0031.c
 */
int clsLED(byte ctl, byte color) {
    byte led = 0;
    byte type = 0;
    byte C = 0;

    if(color == 'N')
        return clsLEDN(ctl);

    // clear color states
    TPass_LedsOff(0xF0);

    switch (color) {
      case 'R':
          C = 0x10;
          break;

      case 'Y':
          C = 0x20;
          break;

      case 'G':
          C = 0x40;
          break;

      default:
          C = 0x00;
          break;

    };

    type = ctl & 0x0F;

    led = ((ctl & 0xF0) >> 4) | C;

    // if mode 0,0 => turn everything off
    if(!type && !led) {
        TPass_LedsOff(0xFF);
        return 1;
    };

    switch (type) {
      case 0x00:               //off
          TPass_LedsOff(led);
          break;
      case 0x0F:               //on
          TPass_LedsOn(led);
          break;
      default:                 //blink
          TPass_LedsBlink(led, type, type);
          break;
    }

    return 1;
}

/** Image display in contactless device
 * \param img (I) pointer to bitmap of image
 * \param ImgLen (I) size of bitmap
 * \param Lsize (I) horizontal size (lenght) of image
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcik0161.c
 */

int clsImg(const byte * img, word ImgLen, word Lsize) {

    InitContexteGraphique(PERIPH_C30);
    DisplayBitmap(0, 0, (byte *) img, 7, _OFF_);
    PaintGraphics();
    InitContexteGraphique(PERIPH_DISPLAY);

    return 1;                   //does not work yet
}

/** Play melody in contectless device
 * \param melody (I) contain couple sound frequancy divisor and duration in milliseconds
 * \param MelodyLen (I) length of melody
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsSound(const byte * melody, word MelodyLen) {
    int freq;
    byte dly;
    word i;

    for (i = 0; i < MelodyLen; i++) {
        freq = melody[i] * 100;
        i++;
        dly = melody[i];
        TPass_Buzzer(freq, 200);    //ON
        ttestall(0, dly);
        TPass_Buzzer(freq, 0);  //OFF

    }

    return 1;
}

/** Show text string on contactless screen
 * \param line (I) line to display string
 * \param text (I) sting to display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsText(byte line, char *text) {
    char buf[64];

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s\n", text);
    TPass_DisplayXY(0, line, buf);
    return 1;
}

/** Clear contectless screen
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsClearDsp(void) {
    InitContexteGraphique(PERIPH_C30);
    _clrscr();
    PaintGraphics();
    InitContexteGraphique(PERIPH_DISPLAY);
    return 1;
}

/** Turns on/off the backlight of the contactless reader
 * \param level (I) - level of the light from 0 == OFF till 255 == FULLLIGHT
 * \header sys\\sys.h
 * \test tcns0031.c
 */
void clsBacklightLevel(byte level) {
    TPass_BacklightLevel(level);
}

//! \brief Extract the status code (TAG_KERNEL_STATUS_CODE) from a shared exchange buffer.
//! \param[in] &sharedBuffer Shared exchange buffer from which the status code shall be extracted.
//! \return The exctracted status code.

static char debugSHexDigit(char hexValue) {
    return (hexValue < (char) 10) ? hexValue + '0' : hexValue - 10 + 'A';
}

static int DebugHexDump(char const *title, const void *data,
                        unsigned long length) {
    char hex[34], ascii[12];
    char *hexPtr, *asciiPtr;
    unsigned char *buff = (unsigned char *) data;
    unsigned long count;
    char tempString[50];

    memset(hex, 0, sizeof(hex));
    memset(ascii, 0, sizeof(ascii));

    hexPtr = hex;
    asciiPtr = ascii;

    if(title != NULL) {
        memset(tempString, 0, sizeof tempString);
        sprintf(tempString, "\x1b\x20" "%s=", title);
        pprintf(tempString);
    }

    for (count = 0; count < length; ++count) {
        *hexPtr++ =
            (unsigned char)
            debugSHexDigit((unsigned char) ((buff[count] >> 4) & 0x0F));
        *hexPtr++ =
            (unsigned char)
            debugSHexDigit((unsigned char) (buff[count] & 0x0F));
        *hexPtr++ = ' ';

        *asciiPtr++ = ((buff[count] > 32) && (buff[count] < 127)) ?
            buff[count] : '.';

        if(hexPtr - hex >= 33) {
            *hexPtr = 0;
            *asciiPtr = 0;

            memset(tempString, 0, sizeof tempString);
            {
                sprintf(tempString, "\x1b\x20" "%-33s| %-11s", hex, ascii);
                prtS(tempString);
            }

            hexPtr = hex;
            asciiPtr = ascii;
        }
    }

    if(hexPtr > hex) {
        *hexPtr = 0;
        *asciiPtr = 0;

        sprintf(tempString, "\x1b\x20" "%-33s| %-11s", hex, ascii);
        prtS(tempString);
    }
    return 0;
}

//! \brief Init the shared buffre exchanged with the contactleResultoccured.
static int clsMVInitBuffer(void) {
    trcS("clsMVInitBuffer Beg\n");
    int ret = TRUE;

    if(kernelBuffer == NULL)
        kernelBuffer =
            (void *) GTL_SharedExchange_InitShared(SHARED_KERNEL_BUFFER_SIZE);
    else
        GTL_SharedExchange_ClearEx(kernelBuffer, FALSE);

    if(sharedBuffer == NULL)
        sharedBuffer = (void *) GTL_SharedExchange_InitShared(10240);
    else
        GTL_SharedExchange_ClearEx(sharedBuffer, FALSE);

    if(aidShared == NULL)
        aidShared = (void *) GTL_SharedExchange_InitShared(10240);
    else
        GTL_SharedExchange_ClearEx(aidShared, FALSE);

    tlvTree = TlvTree_New(0);
    CHECK(tlvTree != NULL, lblKO);

  lblKO:
    trcFN("clsMVInitBuffer :%d\n", ret);
    return ret;
}

//! \brief Fill output buffer with data for _give_info use.
//! \param[out] Output TLV Tree filled with all the necessary data for Application Selection and card detection.
//! \param[in] pParamTlvTree TLV Tree containing all the parameters (supported AIDs, etc).
//! \param[in] bExplicitSelection \a TRUE if Explicit Selection is used. \a FALSE if Implicit Selection is used.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.
int clsMVInit(tQueue * que, int explicit) {
    int ret;
    word len;
    byte buf[256];
    card tag;
    TLV_TREE_NODE pTlvAidInfo;

    trcS("clsMVInit Beg\n");

    queRewind(que);
    clsMVInitBuffer();

    while(queLen(que)) {
        memset(buf, 0, 256);

        ret = queGetTlv(que, &tag, &len, buf);
        CHECK(ret >= 0, lblKO);
        CHECK(ret == len, lblKO);
        CHECK(len <= 256, lblKO);

        switch (tag) {
          case cmvTagDetectType:
              TlvTree_AddChild(tlvTree, TAG_GENERIC_DETECTION_TYPE, buf, len);
              break;
          case cmvTagDetectNBtoDetect:
              TlvTree_AddChild(tlvTree,
                               TAG_GENERIC_DETECTION_NB_CARDS_TO_DETECT, buf,
                               len);
              break;
          case cmvTagGlobalTO:
              TlvTree_AddChild(tlvTree, TAG_GENERIC_DETECTION_GLOBAL_TIMEOUT,
                               buf, len);
              break;

              //Todo: create loop for all AID data
          case cmvTagAidInfo:
              pTlvAidInfo =
                  TlvTree_AddChild(tlvTree, TAG_EP_AID_INFORMATION, NULL, 0);
              break;
          case tagAIDT:
              TlvTree_AddChild(pTlvAidInfo, TAG_EMV_AID_TERMINAL, buf, len);
              break;
          case cmvTagAidPropId:    // for visa only
              TlvTree_AddChild(pTlvAidInfo,
                               TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER, buf,
                               len);
              break;
          case cmvTagKernelToUse:
              TlvTree_AddChild(pTlvAidInfo, TAG_EP_KERNEL_TO_USE, buf, len);
              break;
          case cmvTagAidOpt:
              TlvTree_AddChild(pTlvAidInfo, TAG_EP_AID_OPTIONS, buf, len);
              break;
          case cmvTagClessTxnLim:
              TlvTree_AddChild(pTlvAidInfo, TAG_EP_CLESS_TRANSACTION_LIMIT, buf,
                               len);
              break;
          case cmvTagClessCvmReqdLimit:
              TlvTree_AddChild(pTlvAidInfo, TAG_EP_CLESS_CVM_REQUIRED_LIMIT,
                               buf, len);
              break;
          case cmvTagClessFlrLim:
              TlvTree_AddChild(pTlvAidInfo, TAG_EP_CLESS_FLOOR_LIMIT, buf, len);
              break;
          case cmvTagTrmTxnQualifier:
              TlvTree_AddChild(pTlvAidInfo,
                               TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS, buf,
                               len);
              break;
          case tagTrmFlrLim:
              TlvTree_AddChild(pTlvAidInfo, TAG_TERMINAL_FLOOR_LIMIT, buf, len);
              break;
          case tagAmtBin:
              TlvTree_AddChild(tlvTree, TAG_EMV_AMOUNT_AUTH_BIN, buf, len);
              break;
          default:
              trcS("clsMVInit: Unknown Tag\n");
              break;
        }
    }

    ret = Cless_ExplicitSelection_LoadData(tlvTree);
    CHECK(ret == CLESS_STATUS_OK, lblKO);

    ret = Cless_ExplicitSelection_EntryPoint_TransactionPreProcessing();
    CHECK(ret == CLESS_STATUS_OK, lblKO);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcFN("clsMVInit: %d\n", ret);
    ret = -1;
  lblEnd:
    trcS("clsMVInit End\n");
    return ret;
}

int clsMVDetect(void) {
    int ret;
    FILE *kbd;

    trcS("clsMVDetect Beg\n");
    // Wait contactless event (or cancel if red key is pressed  
    ret = Cless_ExplicitSelection_GlobalCardDetection();
    CHECK(ret == CLESS_STATUS_OK, lblKO);

    kbd = fopen("KEYBOARD", "r");

    while(1) {
        ret = ttestall(KEYBOARD | CLESS, 0);
        if(ret == KEYBOARD) {
            if(getc(kbd) == T_ANN) {
                ret = Cless_ExplicitSelection_GlobalCardDetectionCancel();
                CHECK(ret == CLESS_STATUS_OK, lblKO);
                trcS("Card detection cancelled\n");
            }
        } else {
            trcS("Card detected\n");
            break;
        }
    }

    if(kbd != NULL) {
        fclose(kbd);
    }

    ret = Cless_Generic_CardDetectionGetResults(sharedBuffer, TRUE);
    CHECK(ret == CLESS_STATUS_OK, lblKO);
    ret = 1;
    goto lblEnd;

  lblKO:
    trcFN("clsMVDetect: %d\n", ret);
    ret = -1;
  lblEnd:
    trcS("clsMVDetect End\n");
    return ret;
}

int clsMVSelect(void) {
    int ret;

    trcS("clsMVSelect Beg\n");
    ret =
        Cless_ExplicitSelection_Selection_ApplicationSelectionProcess
        (sharedBuffer);
    CHECK(ret == CLESS_STATUS_OK, lblKO);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcFN("clsMVSelect: %d\n", ret);
    ret = -1;
  lblEnd:
    trcS("clsMVSelect End\n");
    return ret;
}

//! \brief Get the correct AID parameters (to perform the transaction) according to the application selection result.
//! \param[out] kernelBuffer Shared buffer to be filled with the correct AID parameters (depending on the Application Selection results).
//! \param[in] sharedBuffer Shared buffer containing the application selection results.
//! \param[out] pKernelToUse Indicates the kernel to be used for the transaction.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.
int clsMVFinalSelect(tQueue * que) {
    int ret, pos;
    card tag, len, lenElement;
    const byte *data, *dataElement;
    T_SHARED_DATA_STRUCT *aidParameters = NULL;

    trcS("clsMVFinalSelect Beg\n");

    queRewind(que);
    if(kernelBuffer == NULL) {
        kernelBuffer =
            (void *) GTL_SharedExchange_InitShared(SHARED_KERNEL_BUFFER_SIZE);
    } else {
        GTL_SharedExchange_ClearEx(kernelBuffer, FALSE);
    }

    // Copy TLVTree in a temporary struct    
    pos = SHARED_EXCHANGE_POSITION_NULL;
    ret =
        GTL_SharedExchange_FindNext(sharedBuffer, &pos,
                                    TAG_EP_CANDIDATE_LIST_ELEMENT, &len, &data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    aidParameters = GTL_SharedExchange_InitShared(queLen(que));
    if(aidParameters != NULL) {
        aidParameters->ulDataLength = (card) queLen(que);
        memcpy(aidParameters->nPtrData, (byte *) que->buf->ptr, queLen(que));
    }
    // Copy AIDs' parameters in final Share Buffer
    ret =
        GTL_SharedExchange_AddSharedBufferContent(kernelBuffer, aidParameters);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);
    GTL_SharedExchange_DestroyShare(aidParameters);

    // Add some of the candidate element tags in the output kernel shared buffer
    pos = DS_POSITION_NULL;
    ret = STATUS_SHARED_EXCHANGE_OK;

    while(ret == STATUS_SHARED_EXCHANGE_OK) {
        ret =
            GTL_SharedExchange_GetNext(sharedBuffer, &pos, &tag, &lenElement,
                                       &dataElement);
        if(ret != STATUS_SHARED_EXCHANGE_OK) {
            break;
        }

        switch (tag) {
          case (TAG_EP_AID_ADDITIONAL_RESULTS):
          case (TAG_EMV_AID_TERMINAL):
          case (TAG_EP_CLESS_APPLI_CAPABILITY_TYPE):
              //case (TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS):
          case (TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER):
              ret =
                  GTL_SharedExchange_AddTag(kernelBuffer, tag, lenElement,
                                            dataElement);
              CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);
              break;

          default:
              break;
        }
    }

    pos = SHARED_EXCHANGE_POSITION_NULL;
    ret =
        GTL_SharedExchange_FindNext(sharedBuffer, &pos,
                                    TAG_EP_FINAL_SELECT_STATUS_WORD, &len,
                                    &data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    // Parameters add in final struct
    ret =
        GTL_SharedExchange_AddTag(kernelBuffer, TAG_EP_FINAL_SELECT_STATUS_WORD,
                                  len, data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    pos = SHARED_EXCHANGE_POSITION_NULL;
    ret =
        GTL_SharedExchange_FindNext(sharedBuffer, &pos,
                                    TAG_EP_FINAL_SELECT_RESPONSE, &len, &data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    // Parameters add in final struct
    ret =
        GTL_SharedExchange_AddTag(kernelBuffer, TAG_EP_FINAL_SELECT_RESPONSE,
                                  len, data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    pos = SHARED_EXCHANGE_POSITION_NULL;
    ret =
        GTL_SharedExchange_FindNext(sharedBuffer, &pos,
                                    TAG_EP_FINAL_SELECT_COMMAND_SENT, &len,
                                    &data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    // Parameters add in final struct
    ret =
        GTL_SharedExchange_AddTag(kernelBuffer,
                                  TAG_EP_FINAL_SELECT_COMMAND_SENT, len, data);
    CHECK(ret == STATUS_SHARED_EXCHANGE_OK, lblKO);

    ret = 1;
    goto lblEnd;

  lblKO:
    GTL_SharedExchange_ClearEx(kernelBuffer, FALSE);
    trcFN("clsMVFinalSelect: %d", ret);
    ret = -1;
  lblEnd:
    trcS("clsMVFinalSelect end\n");
    return ret;
}

int clsMVContext(tQueue * que) {
    int ret;
    word len;
    card tag;
    byte buf[256];

    trcS("clsMVContext Beg\n");

    while(queLen(que)) {
        memset(buf, 0, 256);

        ret = queGetTlv(que, &tag, &len, buf);
        CHECK(ret >= 0, lblKO);
        CHECK(ret == len, lblKO);
        CHECK(len <= 256, lblKO);

        switch (tag) {
          case tagTrnDat:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_TRANSACTION_DATE,
                                        len, buf);
              break;

          case tagTrnTim:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_TRANSACTION_TIME,
                                        len, buf);
              break;

          case tagAmtBin:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_AMOUNT_AUTH_BIN,
                                        len, buf);
              break;

          case tagAmtNum:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_AMOUNT_AUTH_NUM,
                                        len, buf);
              break;

          case tagAmtOthBin:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_AMOUNT_OTHER_BIN,
                                        len, buf);
              break;

          case tagAmtOthNum:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_AMOUNT_OTHER_NUM,
                                        len, buf);
              break;

          case tagTrnCurCod:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_EMV_TRANSACTION_CURRENCY_CODE, len,
                                        buf);
              break;

          case tagTrnCurExp:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_EMV_TRANSACTION_CURRENCY_EXPONENT,
                                        len, buf);
              break;

          case tagPPassIntTxnType:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_EMV_INT_TRANSACTION_TYPE, len, buf);
              break;

          case tagTrnTyp:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_TRANSACTION_TYPE,
                                        len, buf);
              break;

          case tagIFDSerNum:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_IFD_SERIAL_NUMBER,
                                        len, buf);
              break;

          case tagTrnSeqCnt:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_EMV_TRANSACTION_SEQUENCE_COUNTER,
                                        len, buf);
              break;

              //paypass internal tags 
          case cmvKernelTrmLang:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES,
                                        len, buf);
              break;
          case tagPPassIntMagTrmAVNList:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_PAYPASS_INT_MSTRIPE_TERMINAL_AVN_LIST,
                                        len, buf);
              break;
          case tagPPassIntChipTrmAVNList:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_PAYPASS_INT_MCHIP_TERMINAL_AVN_LIST,
                                        len, buf);
              break;
          case tagPPassTrmCapCvmReq:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_PAYPASS_TERMINAL_CAPABILITIES_CVM_REQ,
                                        len, buf);
              break;
          case tagPPassTrmCapNoCvmReq:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_PAYPASS_TERMINAL_CAPABILITIES_NO_CVM_REQ,
                                        len, buf);
              break;
          case tagPPassDftUDOL:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_PAYPASS_DEFAULT_UDOL,
                                        len, buf);
              break;
          case tagPPassMagIndicator:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_PAYPASS_MSTRIPE_INDICATOR, len,
                                        buf);
              break;
          case tagPPassIntTACDft:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_INT_TAC_DEFAULT,
                                        len, buf);
              break;
          case tagPPassIntTACDnl:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_INT_TAC_DEFAULT,
                                        len, buf);
              break;
          case tagPPassIntTACOnl:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_INT_TAC_ONLINE,
                                        len, buf);
              break;
          case tagPPassTxnCatCod:
              GTL_SharedExchange_AddTag(kernelBuffer,
                                        TAG_PAYPASS_TRANSACTION_CATEGORY_CODE,
                                        len, buf);
              break;
          case tagPPassIntDftTDOL:
              GTL_SharedExchange_AddTag(kernelBuffer, TAG_EMV_INT_DEFAULT_TDOL,
                                        len, buf);
              break;

          default:
              trcS("clsMVContext: Unknown Tag\n");
              break;

        }
    }

    ret = 1;
    goto lblEnd;

  lblKO:
    trcFN("clsMVContext :%d\n", ret);
    ret = -1;
  lblEnd:
    trcS("clsMVContext End\n");
    return (ret);
}

static void clsMVEndTransaction(void) {
    CLESS_DeselectCard(NULL, 0, TRUE, FALSE);

    // Transaction is completed, clear PayPass kernel transaction data
    PayPass_Clear();
    payWave_Clear();
    VisaWave_Clear();

    // Return result    
    if(tlvTree != NULL)
        TlvTree_Release(tlvTree);

    // Release shared buffer if allocated
    if(&sharedBuffer != NULL)
        GTL_SharedExchange_DestroyShare(sharedBuffer);

}

int clsMVCommonGetStatusCode(T_SHARED_DATA_STRUCT * sharedBuffer) {
    trcS("clsMVCommonGetStatusCode Beg\n");
    int nResult;
    int nStatusCode = -1;
    unsigned long ulTag, ulReadLength;
    const unsigned char *pReadValue;
    int nPosition;

    nPosition = DS_POSITION_NULL;
    ulTag = TAG_KERNEL_STATUS_CODE;
    nResult =
        GTL_SharedExchange_FindNext(sharedBuffer, &nPosition, ulTag,
                                    &ulReadLength, &pReadValue);

    if(nResult == STATUS_SHARED_EXCHANGE_OK)    // If tag found
    {
        memcpy(&nStatusCode, pReadValue, ulReadLength);
    }

    trcFN("clsMVCommonGetStatusCode :%d\n", nStatusCode);
    return (nStatusCode);
}

//! \brief Get a specific information in a shared buffer.
//! \param[out] pSpendingAmount pointer
//! \return
//!     - \ref TRUE if correctly retreived.
//!     - \ref FALSE if an error occured.

int clsMVCommonRetreiveInfo(T_SHARED_DATA_STRUCT * pResultDataStruct,
                            unsigned long ulTag, unsigned char **pInfo) {
    trcS("clsMVCommonRetrieveInfo Beg\n");
    int nResult = TRUE;
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pInfo = NULL;              // Default result : no information

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, ulTag,
                                     &ulReadLength, &pReadValue);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("ClessSample_Common_RetreiveInfo : Unable to getinfo from the kernel response (tag=%02lx, cr=%02x)",
             ulTag, cr);
        nResult = FALSE;
        goto End;
    }
    // Get the transaction outcome
    *pInfo = (unsigned char *) pReadValue;

  End:
    trcFN("clsMVCommonRetrieveInfo %d\n", nResult);
    return (nResult);
}

//**** CLESS EMV ****//

/*
//! \brief Get the PayPass transaction outcome.
//! \param[out] pTransactionOutcome Retreived transaction outcome :
//!		- \a PAYPASS_OUTCOME_APPROVED if transaction is approved.
//!		- \a PAYPASS_OUTCOME_ONLINE_REQUEST if an online authorisation is requested.
//!		- \a PAYPASS_OUTCOME_DECLINED if the transaction is declined.
//!		- \a PAYPASS_OUTCOME_TRY_ANOTHER_INTERFACE if another interface shall be used.
//!		- \a PAYPASS_OUTCOME_END_APPLICATION if the transaction is terminated.
//! \return
//!		- \ref TRUE if correctly retreived.
//!		- \ref FALSE if an error occured.

static int __ClessSample_PayPass_RetreiveTransactionOutcome (T_SHARED_DATA_STRUCT * pResultDataStruct, unsigned char * pTransactionOutcome)
{
	int nResult = TRUE;
	int nPosition, cr;
	unsigned long ulReadLength;
	const unsigned char * pReadValue;
	
	nPosition = SHARED_EXCHANGE_POSITION_NULL;
	* pTransactionOutcome = PAYPASS_OUTCOME_END_APPLICATION; // Default result
	
	cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition, TAG_PAYPASS_TRANSACTION_OUTCOME, &ulReadLength, &pReadValue);
	
	if (cr != STATUS_SHARED_EXCHANGE_OK)
	{
		GTL_Traces_TraceDebug("__ClessSample_PayPass_RetreiveTransactionOutcome : Unable to get transaction outcome from the PayPass kernel response (cr = %02x)", cr);
		nResult = FALSE;
		goto End;
	}
	
	// Get the transaction outcome
	* pTransactionOutcome = pReadValue[0];
	
End:	
	return (nResult);
}
*/

//! \brief Get the PayPass transaction outcome.
//! \param[out] pTransactionOutcome Retreived transaction outcome :
//!     - \a PAYPASS_CVM_NO_CVM No CVM to be performed.
//!     - \a PAYPASS_CVM_SIGNATURE if signature shall be performed.
//!     - \a PAYPASS_CVM_ONLINE_PIN if online PIN shall be performed.
//! \return
//!     - \ref TRUE if correctly retreived.
//!     - \ref FALSE if an error occured.

static int clsMVPayPassRetreiveCvmToApply(T_SHARED_DATA_STRUCT *
                                          pResultDataStruct,
                                          unsigned char *pCvm) {
    trcS("clsMVPayPassRetrieveCvmToApply Beg\n");
    int nResult = TRUE;
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pCvm = PAYPASS_CVM_NO_CVM; // Default result

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition,
                                     TAG_PAYPASS_TRANSACTION_CVM, &ulReadLength,
                                     &pReadValue);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        nResult = FALSE;
        goto End;
    }
    // Get the transaction outcome
    *pCvm = pReadValue[0];

  End:
    trcFN("clsMVPayPassRetrieveCvmToApply :%d\n", nResult);
    return (nResult);
}

//! \brief Get the PayPass transaction outcome.
//! \param[out] pTransactionOutcome Retreived card type :
//!     - \a 0 If card type not found.
//!     - \a 0x8501 for MStripe card.
//!     - \a 0x8502 for MChip card.
//! \return
//!     - \ref TRUE if correctly retreived.
//!     - \ref FALSE if an error occured.

static int clsMVPayPassRetreiveCardType(T_SHARED_DATA_STRUCT *
                                        pResultDataStruct,
                                        unsigned short *pCardType) {
    trcS("clsMVPayPassRetrieveCardType Beg\n");
    int nResult = TRUE;
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pCardType = 0;             // Default result

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition,
                                     TAG_KERNEL_CARD_TYPE, &ulReadLength,
                                     &pReadValue);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        nResult = FALSE;
        goto End;
    }
    // Get the transaction outcome
    *pCardType = (pReadValue[0] << 8) + pReadValue[1];

  End:
    trcFN("clsMVPayPassRetrieveCardType :%d\n", nResult);
    return (nResult);
}

//! \brief Fill buffer with specific PayPass for transaction.
//! \param[out] pDataStruct Shared exchange structure filled with the specific PayPass data.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.

static int clsMVPayPassAddPayPassSpecificData(T_SHARED_DATA_STRUCT *
                                              pDataStruct) {
    trcS("clsMVPayPassAddPayPassSpecificData Beg\n");
    int cr, nResult;
    object_info_t ObjectInfo;

    //unsigned char bMerchantForcedTrOnline = 0x01;
    T_KERNEL_TRANSACTION_FLOW_CUSTOM sTransactionFlowCustom;
    unsigned char StepInterruption[KERNEL_PAYMENT_FLOW_STOP_LENGTH];    // Bit field to stop payment flow,

    // if all bit set to 0 => no stop during payment process
    // if right bit set to 1 : stop after payment step number 1
    unsigned char StepCustom[KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH];    // Bit field to custom payment flow,

    // if all bit set to 0 => no stop during payment process
    // if right bit set to 1 : stop after payment step number 1

    if(pDataStruct == NULL) {
        GTL_Traces_TraceDebug
            ("__ClessSample_PayPass_AddPayPassSpecificData : Invalid Input data");
        nResult = FALSE;
        goto End;
    }
    // Init parameteters
    memset(StepInterruption, 0, sizeof(StepInterruption));  // Default Value : not stop on process
    memset(StepCustom, 0, sizeof(StepCustom));  // Default Value : not stop on process
    ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &ObjectInfo);
    nResult = TRUE;

    // Add a tag for Do_Txn management
    cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_PAYMENT_FLOW_STOP,
                                   KERNEL_PAYMENT_FLOW_STOP_LENGTH,
                                   (const unsigned char *) StepInterruption);
    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_PayPass_AddPayPassSpecificData : Unable to add TAG_KERNEL_PAYMENT_FLOW_STOP in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }

    // Customize steps
    ADD_STEP_CUSTOM(STEP_PAYPASS_MSTRIPE_REMOVE_CARD, StepCustom);  // To do GUI when MStripe card has been read
    ADD_STEP_CUSTOM(STEP_PAYPASS_MCHIP_REMOVE_CARD, StepCustom);    // To do GUI when MChip card has been read
    ADD_STEP_CUSTOM(STEP_PAYPASS_MCHIP_GET_CERTIFICATE, StepCustom);    // To provide the CA key data for ODA
/*
	if (ClessSample_IsBlackListPresent())	
		ADD_STEP_CUSTOM(STEP_PAYPASS_MCHIP_EXCEPTION_FILE_GET_DATA, StepCustom); // To check if PAN is in the blacklist
*/
    memcpy((void *) &sTransactionFlowCustom, (void *) StepCustom,
           KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH);
    sTransactionFlowCustom.usApplicationType = ObjectInfo.application_type; // Kernel will call this application for customisation 
    sTransactionFlowCustom.usServiceId = SERVICE_CUSTOM_KERNEL; // Kernel will call SERVICE_CUSTOM_KERNEL service id for customisation 

    cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_PAYMENT_FLOW_CUSTOM,
                                   sizeof(T_KERNEL_TRANSACTION_FLOW_CUSTOM),
                                   (const unsigned char *)
                                   &sTransactionFlowCustom);
    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_PayPass_AddPayPassSpecificData : Unable to add TAG_KERNEL_PAYMENT_FLOW_CUSTOM in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    // Add data indicating if merchant forced transaction online or not
    //if (ClessSample_Menu_IsMerchantForcedOnline())
    //{
    cr = GTL_SharedExchange_AddTag(pDataStruct,
                                   TAG_PAYPASS_INT_MERCHANT_FORCE_ONLINE, 1, 0);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_PayPass_AddPayPassSpecificData : Unable to add TAG_PAYPASS_INT_MERCHANT_FORCE_ONLINE in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    //}

  End:
    trcFN("clsMVPayPassAddPayPassSpecificData :%d\n", nResult);
    return (nResult);
}

/*
//! \brief Perform the Online PIN input and encipher PIN.
//! \param[out] pStructureForOnlineData Data returned by the kernel in which the enciphered online PIN would be added.
//! \return
//!		- \ref TRUE if correctly performed.
//!		- \ref FALSE if an error occured.

static int __ClessSample_PayPass_OnlinePinManagement (T_SHARED_DATA_STRUCT * pStructureForOnlineData, int nCardHolderLang)
{
	T_SHARED_DATA_STRUCT * pDataRequest;
	int nResult = TRUE;
	int cr;
	int nPosition;
	const unsigned char * pPan;
	unsigned long ulPanLength;
	const unsigned char * pAmount;
	unsigned long ulAmountLength;
	unsigned long ulAmount = 0;
	BUFFER_SAISIE buffer_saisie;
	
	
	pDataRequest = GTL_SharedExchange_InitShared (128);

	if (pDataRequest != NULL)
	{
		// Clear shared buffer
		GTL_SharedExchange_ClearEx (pDataRequest, FALSE);

		// Indicate tag to be requested
		GTL_SharedExchange_AddTag (pDataRequest, TAG_EMV_APPLI_PAN, 0, NULL);
		GTL_SharedExchange_AddTag (pDataRequest, TAG_EMV_AMOUNT_AUTH_NUM, 0, NULL);

		cr = PayPass_GetData (pDataRequest);

		if (cr != KERNEL_STATUS_OK)
		{
			GTL_Traces_TraceDebug("__ClessSample_PayPass_OnlinePinManagement : An error occured when getting tags from the PayPass kernel (cr=%02x)", cr);
			nResult = FALSE;
			goto End;
		}

		// Tags have been got (if present), get the PAN
		nPosition = SHARED_EXCHANGE_POSITION_NULL;
		if (GTL_SharedExchange_FindNext (pDataRequest, &nPosition, TAG_EMV_APPLI_PAN, &ulPanLength, &pPan) != STATUS_SHARED_EXCHANGE_OK)
		{
			GTL_Traces_TraceDebug ("__ClessSample_PayPass_OnlinePinManagement : Missing PAN for Online PIN");
			nResult = FALSE;
			goto End;
		}

		// Get the transaction amount (numeric)
		nPosition = SHARED_EXCHANGE_POSITION_NULL;
		if (GTL_SharedExchange_FindNext (pDataRequest, &nPosition, TAG_EMV_AMOUNT_AUTH_NUM, &ulAmountLength, &pAmount) != STATUS_SHARED_EXCHANGE_OK)
		{
			GTL_Traces_TraceDebug ("__ClessSample_PayPass_OnlinePinManagement : Missing Amount for Online PIN");
			nResult = FALSE;
			goto End;
		}
		
		// Convert amount
		GTL_Convert_DcbNumberToUl(pAmount, &ulAmount, ulAmountLength);

		// Request online PIN entry
		//cr = ClessSample_PinManagement_OnLinePinManagement ((unsigned char*)pPan, 1, ulAmount, 30000, 10000, nCardHolderLang, &buffer_saisie);

		if (cr == INPUT_PIN_ON)
		{
			cr = GTL_SharedExchange_AddTag(pStructureForOnlineData, TAG_SAMPLE_ENCIPHERED_PIN_CODE, buffer_saisie.nombre , (const unsigned char *)buffer_saisie.donnees);

			if (cr != STATUS_SHARED_EXCHANGE_OK)
			{
				GTL_Traces_TraceDebug ("__ClessSample_PayPass_OnlinePinManagement : Unable to add TAG_SAMPLE_ENCIPHERED_PIN_CODE in the shared buffer (cr = %02x)", cr);
				nResult = FALSE;
				goto End;
			}
		}
	}

End:
	return (nResult);
}
*/

//! \brief Create a shared buffer, and requests the selected language to the PayPass kernel.
//! \param[out] pCardholderLanguage Selected language (equal to \a nMerchantLanguage if selected language is not found).
//! \param[in] nMerchantLanguage Merchant language as configured by the manager.

static void clsMVPaypassGetSelectedPreferedLanguage(int *pCardholderLanguage,
                                                    int nMerchantLanguage) {
    trcS("clsMVPaypassGetSelectedPreferredLanguage Beg\n");
    int ret;
    T_SHARED_DATA_STRUCT *pStruct;

    //unsigned char * pInfo;

    // By default, cardholder language is the same as the merchant language
    *pCardholderLanguage = nMerchantLanguage;

    // Init a shared buffer to get the prefered selected language
    pStruct = GTL_SharedExchange_InitShared(128);

    if(pStruct == NULL) {
        // An error occured when creating the shared buffer
        GTL_Traces_TraceDebug
            ("__ClessSample_Paypass_GetSelectedPreferedLanguage : An error occured when creating the shared buffer");
        goto End;
    }
    // Add tag in the shared buffer to request it
    ret =
        GTL_SharedExchange_AddTag(pStruct,
                                  TAG_KERNEL_SELECTED_PREFERED_LANGUAGE, 0,
                                  NULL);

    if(ret != STATUS_SHARED_EXCHANGE_OK) {
        // An error occured when adding the TAG_KERNEL_SELECTED_PREFERED_LANGUAGE tag in the structure
        GTL_Traces_TraceDebug
            ("__ClessSample_Paypass_GetSelectedPreferedLanguage : An error occured when adding the TAG_KERNEL_SELECTED_PREFERED_LANGUAGE tag in the structure (ret = %02x)",
             ret);
        goto End;
    }
    // Request data to PayPass kernel
    ret = PayPass_GetData(pStruct);

    if(ret != KERNEL_STATUS_OK) {
        // An error occured when getting data from the PayPass kernel
        GTL_Traces_TraceDebug
            ("__ClessSample_Paypass_GetSelectedPreferedLanguage : An error occured when getting data from the PayPass kernel (ret = %02x)",
             ret);
        goto End;
    }
    // Search the tag in the kernel response structure
    /*
       if (ClessSample_Common_RetreiveInfo (pStruct, TAG_KERNEL_SELECTED_PREFERED_LANGUAGE, &pInfo))
       *pCardholderLanguage = ClessSample_Term_GiveLangNumber(pInfo);
       pCardholderLanguage = "EN" ;
     */
  End:
    // Destroy the shared buffer if created
    if(pStruct != NULL)
        GTL_SharedExchange_DestroyShare(pStruct);
    trcFN("clsMVPaypassGetSelectedPreferredLanguage :%d\n", ret);
}

//! \brief Calls the PayPass kernel to perform the transaction.
//! \param[in] pDataStruct Data buffer to be filled and used for PayPass transaction.
//! \return
//!     - PayPass kernel result.

int clsMVPayPassPerformTransaction(void) {
    trcS("clsMVPayPassTransaction Beg\n");
    int res, ret = CLESS_CR_MANAGER_END;
    byte ucCvm;
    int merchLang, nCardHolderLang;
    word usCardType = 0;

    merchLang = PSQ_Give_Language();

    // Indicate PayPass kernel is going to be used (for customisation purposes)
    //ClessSample_Customisation_SetUsedPaymentScheme (CLESS_SAMPLE_CUST_PAYPASS);

    // Get the PayPass
    if(!clsMVPayPassAddPayPassSpecificData(kernelBuffer)) {
        GTL_Traces_TraceDebug
            ("ClessSample_PayPass_PerformTransaction : ClessSample_Fill_PayPassTransaction failed\n");
    } else {
        // Debug purpose : if you need to dump the data provided to the kernel
        ///ClessSample_DumpData_DumpSharedBufferWithTitle(pDataStruct, "INPUT DATA");
        //DebugHexDump("ABC" , sharedBuffer->nPtrData , 200 ) ;//sharedBuffer->ulDataLength ) ;
        ret = PayPass_DoTransaction(kernelBuffer);

        // Get prefered card language (a request shall be done to the PayPass kernel as the TAG_KERNEL_SELECTED_PREFERED_LANGUAGE tag is not defaulty returned by the PayPass kernel).
        clsMVPaypassGetSelectedPreferedLanguage(&nCardHolderLang, merchLang);

        // Get the TAG_PAYPASS_TRANSACTION_CVM to identify the CVM to be performed :
        //  - PAYPASS_CVM_NO_CVM (0x01) : "No CVM" method has been applied.
        //  - PAYPASS_CVM_SIGNATURE (0x02) : "Signature" method has been applied.
        //  - PAYPASS_CVM_ONLINE_PIN (0x04) : "Online PIN" method has been applied.
        if(!clsMVPayPassRetreiveCvmToApply(kernelBuffer, &ucCvm)) {
            ucCvm = PAYPASS_CVM_NO_CVM;
        }
        // Retreive the card type
        if(!clsMVPayPassRetreiveCardType(kernelBuffer, &usCardType)) {
            usCardType = 0;
        }
        // Additional possible processing :
        //  - Perform an online authorisation if necessary
        //  - Save the transaction in the batch if transaction is accepted
        //  - Perform CVM processing if necessary
        if((ucCvm == PAYPASS_CVM_ONLINE_PIN)
           && (ret == KERNEL_STATUS_OFFLINE_APPROVED)) {
            ret = KERNEL_STATUS_ONLINE_AUTHORISATION;
        }

        sprintf(cTest, "CVMRESULT:%d", ret);
        prtS(cTest);

        switch (ret) {
          case (KERNEL_STATUS_OFFLINE_APPROVED):
              res = cmvKernelStatusOfflineTC;
              break;

          case (KERNEL_STATUS_OFFLINE_DECLINED):
              res = cmvKernelStatusOfflineAAC;
              break;

          case (KERNEL_STATUS_ONLINE_AUTHORISATION):
              res = cmvKernelStatusARQC;
              break;

          case (KERNEL_STATUS_USE_CONTACT_INTERFACE):
              res = cmvKernelStatusUseContactInterface;
              break;

          case (KERNEL_STATUS_COMMUNICATION_ERROR):
              res = cmvKernelStatusCommError;
              break;

          case (KERNEL_STATUS_CARD_BLOCKED):
              res = cmvKernelStatusCardBlock;
              break;

          case (KERNEL_STATUS_APPLICATION_BLOCKED):
              res = cmvKernelStatusAppBlock;
              break;

          default:             // Error case
              res = cmvKernelStatusUnknown;
              break;
        }

    }
    trcFN("clsMVPayPassTransaction :%d\n", res);

    clsMVEndTransaction();
    return (res);
}

//! \brief Get the correct AID parameters (to perform the transaction) according to the application selection result.
//! \param[out] kernelBuffer Shared buffer to be filled with the correct AID parameters (depending on the Application Selection results).
//! \param[in] sharedBuffer Shared buffer containing the application selection results.
//! \param[out] pKernelToUse Indicates the kernel to be used for the transaction.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.

int clsMVDetermineAID(void) {
    trcS("clsMVDetrmineAID Beg\n");
    int nResult = TRUE;
    int cr, cr2;
    int nPosition, nPositionElement, nNbNecessaryData;
    unsigned long ulReadLength, ulReadLengthElement;
    const unsigned char *pReadValue, *pReadValueElement;
    const unsigned char *pCandidateElementPointer;
    unsigned long ulCandidateElementLen;
    T_SHARED_DATA_STRUCT SubSharedExchange;
    int nAidIndex = 0xff;
    unsigned char *pCardAid;
    unsigned int nCardAidLength;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    nNbNecessaryData = 0;

    // Loop on each candidate element
    cr = GTL_SharedExchange_FindNext(sharedBuffer, &nPosition,
                                     TAG_EP_CANDIDATE_LIST_ELEMENT,
                                     &ulReadLength, &pReadValue);

    // Here, only the first canddate element is checked. But it could be possible to have several candidate element at the same time.
    if(cr == STATUS_SHARED_EXCHANGE_OK) // If tag with AID informations found
    {
        // Parameters information memorisation
        pCandidateElementPointer = pReadValue;
        ulCandidateElementLen = ulReadLength;

        if(GTL_SharedExchange_InitEx
           (&SubSharedExchange, ulReadLength, ulReadLength,
            (unsigned char *) pReadValue) != STATUS_SHARED_EXCHANGE_OK) {
            GTL_Traces_TraceDebug
                ("ClessSample_Fill_AidRelatedData : GTL_SharedExchange_InitEx error");
            nResult = FALSE;
            goto End;
        }

        nPositionElement = DS_POSITION_NULL;
        cr2 =
            GTL_SharedExchange_FindNext(&SubSharedExchange, &nPositionElement,
                                        TAG_EP_KERNEL_TO_USE,
                                        &ulReadLengthElement,
                                        &pReadValueElement);
        if(cr2 == STATUS_SHARED_EXCHANGE_OK)    // If tag with AID informations found
        {
            nNbNecessaryData++; // One information has been founded on two bytes
        }

        nPositionElement = DS_POSITION_NULL;
        cr2 =
            GTL_SharedExchange_FindNext(&SubSharedExchange, &nPositionElement,
                                        TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER,
                                        &ulReadLengthElement,
                                        &pReadValueElement);
        if(cr2 == STATUS_SHARED_EXCHANGE_OK)    // If tag with AID informations found
        {
            nNbNecessaryData++; // One information has been founded
            nAidIndex =
                pReadValueElement[3] + (pReadValueElement[2] << 8) +
                (pReadValueElement[1] << 16) + (pReadValueElement[0] << 24);
        }

        nPositionElement = DS_POSITION_NULL;
        cr2 =
            GTL_SharedExchange_FindNext(&SubSharedExchange, &nPositionElement,
                                        TAG_EMV_AID_CARD, &ulReadLengthElement,
                                        &pReadValueElement);
        if(cr2 == STATUS_SHARED_EXCHANGE_OK)    // If tag with AID informations found
        {
            nNbNecessaryData++; // One information has been founded
            pCardAid = (unsigned char *) pReadValueElement;
            nCardAidLength = ulReadLengthElement;
        } else {
            nPositionElement = DS_POSITION_NULL;
            cr2 =
                GTL_SharedExchange_FindNext(&SubSharedExchange,
                                            &nPositionElement, TAG_EMV_DF_NAME,
                                            &ulReadLengthElement,
                                            &pReadValueElement);
            if(cr2 == STATUS_SHARED_EXCHANGE_OK)    // If tag with AID informations found
            {
                nNbNecessaryData++; // One information has been founded
                pCardAid = (unsigned char *) pReadValueElement;
                nCardAidLength = ulReadLengthElement;
            }
        }
    } else if(cr == STATUS_SHARED_EXCHANGE_END) // Buffer end reached
    {
        GTL_Traces_TraceDebug
            ("ClessSample_Fill_AidRelatedData : GTL_SharedExchange_FindNext End");
        nResult = FALSE;
        goto End;
    } else                      // error or end or ...
    {
        GTL_Traces_TraceDebug
            ("ClessSample_Fill_AidRelatedData : GTL_SharedExchange_FindNext Error:%x\n",
             cr);
        nResult = FALSE;
        goto End;
    }

    // All the information to retreive the AID have been found
    if(nNbNecessaryData != 3)   // AID informations (KERNEL TO USE and AID PROPRIETARY IDENTIFIER) not found !
    {
        GTL_Traces_TraceDebug
            ("ClessSample_Fill_AidRelatedData : Missing information to retreive the AID\n");
        nResult = FALSE;
        goto End;
    }

    nResult = nAidIndex;
  End:
    trcFN("clsMVDetrmineAID :%d\n", nResult);
    return nResult;

}

//! \brief Fill output buffer with data for _give_info use.
//! \param[out] Output TLV Tree filled with all the necessary data for Application Selection and card detection.
//! \param[in] pParamTlvTree TLV Tree containing all the parameters (supported AIDs, etc).
//! \param[in] bExplicitSelection \a TRUE if Explicit Selection is used. \a FALSE if Implicit Selection is used.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.

//int clsMVSetApplicationSelectionParameters (TLV_TREE_NODE * pOuputTlvTree, TLV_TREE_NODE * pParamTlvTree, const unsigned int bExplicitSelection)
int clsMVSetApplicationSelectionParameters(tQueue * que, int explicit) {
    trcS("clsMVSetApplicationSelectionParameters Beg\n");
    int nResult = TRUE;

    //int nNbAid; 
    //int nIndex;
    //unsigned char ucTempBuffer[4];
    //unsigned char * ucPtrAID;
    //int nLenAID;
    //unsigned short usTempKernelToUse;

    int queRet = 0;
    word tagLen = 0;
    byte tagBBuf[255];
    char tagCBuf[256];
    card tag;

    //byte idx ;

    queRewind(que);

    clsMVInitBuffer();

    while(queLen(que)) {
        memset(tagBBuf, 0, 256);
        memset(tagCBuf, 0, 256);

        queRet = queGetTlv(que, &tag, &tagLen, tagBBuf);
        CHECK(queRet >= 0, lblKO);
        CHECK(queRet == tagLen, lblKO);
        CHECK(tagLen <= 256, lblKO);

        switch (tag) {
          case cmvTagDetectType:
              TlvTree_AddChild(tlvTree, TAG_GENERIC_DETECTION_TYPE, tagBBuf,
                               tagLen);
              break;

          case cmvTagDetectNBtoDetect:
              TlvTree_AddChild(tlvTree,
                               TAG_GENERIC_DETECTION_NB_CARDS_TO_DETECT,
                               tagBBuf, tagLen);
              break;

              // Global detection timeout, if necessary (0x9F928212, on 4 bytes) (in explicit selection only).
          case cmvTagGlobalTO:
              TlvTree_AddChild(tlvTree, TAG_GENERIC_DETECTION_GLOBAL_TIMEOUT,
                               tagBBuf, tagLen);
              break;

          default:
              break;
        }
    }
    // For each supported AID, 
    /*
       nNbAid = CUPAR_GiveAIDNumber();

       for (nIndex=0; nIndex<nNbAid; nIndex++)
       {
       pTlvAidParam = CUPAR_GiveAIDTLVTtree(nIndex);

       // Pointer on AID information nodes
       pTlvAidInfo = TlvTree_AddChild(*pOuputTlvTree, TAG_EP_AID_INFORMATION, NULL, 0);

       // TAG_AID_TERMINAL : 0x9F06
       ucPtrAID = CUPAR_GiveAID(nIndex, &nLenAID);
       if (ucPtrAID != 0)
       {
       TlvTree_AddChild(pTlvAidInfo, TAG_EMV_AID_TERMINAL, ucPtrAID, nLenAID);
       }

       // TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER : 0x9F928202 (corresponds to the index for quick search after application selection)
       GTL_Convert_UlToBinNumber((unsigned long)nIndex, ucTempBuffer, sizeof(nIndex));
       TlvTree_AddChild(pTlvAidInfo, TAG_GENERIC_AID_PROPRIETARY_IDENTIFIER, ucTempBuffer, sizeof(ucTempBuffer));

       // TAG_EP_KERNEL_TO_USE : 0x9F928101
       usTempKernelToUse = DEFAULT_EP_KERNEL_UNKNOWN;
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_EP_KERNEL_TO_USE, 0);
       if (pTlvTmp != 0)
       {
       TlvTree_AddChild(pTlvAidInfo, TAG_EP_KERNEL_TO_USE, TlvTree_GetData(pTlvTmp), TlvTree_GetLength(pTlvTmp));
       usTempKernelToUse = (((unsigned char *)TlvTree_GetData(pTlvTmp))[0] << 8) + ((unsigned char *)TlvTree_GetData(pTlvTmp))[1];
       }

       // TAG_EP_AID_OPTIONS : 0x9F928100
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_EP_AID_OPTIONS, 0);
       if (pTlvTmp != 0)
       {
       unsigned char auc_AIDOptions[4];
       memcpy (auc_AIDOptions, TlvTree_GetData(pTlvTmp), 4);
       if (usTempKernelToUse == DEFAULT_EP_KERNEL_VISA)
       {
       // Add information  : OPTION ZERO AMOUNT ALLOWED = TRUE
       auc_AIDOptions[0] |= 0x04; 
       }

       TlvTree_AddChild(pTlvAidInfo, TAG_EP_AID_OPTIONS, auc_AIDOptions, TlvTree_GetLength(pTlvTmp));
       }

       if (usTempKernelToUse != DEFAULT_EP_KERNEL_VISA)
       {
       // TAG_EP_CLESS_TRANSACTION_LIMIT : 0x9F92810D
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_EP_CLESS_TRANSACTION_LIMIT, 0);
       if (pTlvTmp != 0)
       TlvTree_AddChild(pTlvAidInfo, TAG_EP_CLESS_TRANSACTION_LIMIT, TlvTree_GetData(pTlvTmp), TlvTree_GetLength(pTlvTmp));
       }

       // TAG_EP_CLESS_CVM_REQUIRED_LIMIT : 0x9F92810E
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, 0);
       if (pTlvTmp != 0)
       TlvTree_AddChild(pTlvAidInfo, TAG_EP_CLESS_CVM_REQUIRED_LIMIT, TlvTree_GetData(pTlvTmp), TlvTree_GetLength(pTlvTmp));

       // TAG_EP_CLESS_FLOOR_LIMIT : 0x9F92810F
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_EP_CLESS_FLOOR_LIMIT, 0);
       if (pTlvTmp != 0)
       TlvTree_AddChild(pTlvAidInfo, TAG_EP_CLESS_FLOOR_LIMIT, TlvTree_GetData(pTlvTmp), TlvTree_GetLength(pTlvTmp));

       // TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS : 0x9F66
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS, 0);
       if (pTlvTmp != 0)
       TlvTree_AddChild(pTlvAidInfo, TAG_EP_TERMINAL_TRANSACTION_QUALIFIERS, TlvTree_GetData(pTlvTmp), TlvTree_GetLength(pTlvTmp));

       // TAG_TERMINAL_FLOOR_LIMIT : 0x9F1B
       pTlvTmp = TlvTree_Find(pTlvAidParam, TAG_TERMINAL_FLOOR_LIMIT, 0);
       if (pTlvTmp != 0)
       TlvTree_AddChild(pTlvAidInfo, TAG_TERMINAL_FLOOR_LIMIT, TlvTree_GetData(pTlvTmp), TlvTree_GetLength(pTlvTmp));
       }
     */
  lblKO:
    //End:
    trcFN("clsMVSetApplicationSelectionParameters :%d\n", nResult);
    return nResult;
}

/////////////////////////////////////////////////////////////////
//// Functions //////////////////////////////////////////////////

//! \brief Fill buffer with specific payWave for transaction.
//! \param[out] pDataStruct Shared exchange structure filled with the specific payWave data.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.
static int clsMVPayWaveAddPayPassSpecificData(T_SHARED_DATA_STRUCT *
                                              pDataStruct) {
    int cr, nResult;
    T_KERNEL_TRANSACTION_FLOW_CUSTOM sTransactionFlowCustom;
    object_info_t ObjectInfo;
    unsigned char StepInterruption[KERNEL_PAYMENT_FLOW_STOP_LENGTH];    // Bit field to stop payment flow,

    // if all bit set to 0 => no stop during payment process
    // if right bit set to 1 : stop after payment step number 1
    unsigned char StepCustom[KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH];    // Bit field to custom payment flow,

    // if all bit set to 0 => no stop during payment process
    // if right bit set to 1 : stop after payment step number 1
    if(pDataStruct == NULL) {
        GTL_Traces_TraceDebug
            ("__ClessSample_payWave_AddpayWaveSpecificData : Invalid Input data");
        nResult = FALSE;
        goto End;
    }
    // Init parameteters
    memset(StepInterruption, 0, sizeof(StepInterruption));  // Default Value : not stop on process
    memset(StepCustom, 0, sizeof(StepCustom));  // Default Value : not stop on process
    nResult = TRUE;

    // Customize steps :
    ADD_STEP_CUSTOM(STEP_PAYWAVE_MSD_REMOVE_CARD, StepCustom);  // To do GUI when MStripe card has been read
    ADD_STEP_CUSTOM(STEP_PAYWAVE_QVSDC_REMOVE_CARD, StepCustom);    // To do GUI when MChip card has been read
    ADD_STEP_CUSTOM(STEP_PAYWAVE_QVSDC_GET_CERTIFICATE, StepCustom);    // To get the certifiacte for ODA step

    /*
       if (ClessSample_IsBlackListPresent())    
       ADD_STEP_CUSTOM(STEP_PAYWAVE_QVSDC_BLACK_LIST_CONTROL,StepCustom);   // To check Pan in Black list
     */

    ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &ObjectInfo);

    // Add a tag for Do_Txn management
    cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_PAYMENT_FLOW_STOP,
                                   KERNEL_PAYMENT_FLOW_STOP_LENGTH,
                                   (const unsigned char *) StepInterruption);
    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_payWave_AddpayWaveSpecificData : Unable to add TAG_KERNEL_PAYMENT_FLOW_STOP in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    // Add a tag for Do_Txn management
    memcpy((void *) &sTransactionFlowCustom, (void *) StepCustom,
           KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH);
    sTransactionFlowCustom.usApplicationType = ObjectInfo.application_type; // Kernel will call this application for customisation 
    sTransactionFlowCustom.usServiceId = SERVICE_CUSTOM_KERNEL; // Kernel will call SERVICE_CUSTOM_KERNEL service id for customisation

    cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_PAYMENT_FLOW_CUSTOM,
                                   sizeof(T_KERNEL_TRANSACTION_FLOW_CUSTOM),
                                   (const unsigned char *)
                                   &sTransactionFlowCustom);
    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_payWave_AddpayWaveSpecificData : Unable to add TAG_KERNEL_PAYMENT_FLOW_CUSTOM in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    // Add Tag TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES
    /*
       cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES, sizeof(gsTerminalSupportedLanguage), (const unsigned char *)&gsTerminalSupportedLanguage);
       if (cr != STATUS_SHARED_EXCHANGE_OK)
       {
       GTL_Traces_TraceDebug("__ClessSample_payWave_AddpayWaveSpecificData : Unable to add TAG_KERNEL_TERMINAL_SUPPORTED_LANGUAGES in shared buffer (cr=%02x)", cr);
       nResult = FALSE;
       goto End;
       }
     */

  End:
    return (nResult);

}

//! \brief Get the payWave PinOnLineRequired state.
//! \param[out] pPinOnLineRequiredState :
//!     - \a 0 No Pin OnLine Requested to made.
//!     - \a 1 Pin OnLine Requested to made.
//! \return
//!     - \ref TRUE if correctly retreived.
//!     - \ref FALSE if an error occured.
static int clsMVPayWave_PinOnLineRequiredState(T_SHARED_DATA_STRUCT *
                                               pResultDataStruct,
                                               unsigned char
                                               *pPinOnLineRequiredState) {
    int nResult = TRUE;
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pPinOnLineRequiredState = 0;   // Default result : no PinOnLineRequired

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition,
                                     TAG_KERNEL_ONLINE_PIN_REQUESTED,
                                     &ulReadLength, &pReadValue);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_payWave_RetreiveSignature : Unable to get OnLinePinRequested from the payWave kernel response (cr = %02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    // Get the transaction outcome
    *pPinOnLineRequiredState = pReadValue[0];

  End:
    return (nResult);
}

//! \brief Get the payWave signature state.
//! \param[out] pSignature :
//!     - \a 0 No signature to made.
//!     - \a 1 Signature to made.
//! \return
//!     - \ref TRUE if correctly retreived.
//!     - \ref FALSE if an error occured.
static int clsMVPayWave_RetreiveSignature(T_SHARED_DATA_STRUCT *
                                          pResultDataStruct,
                                          unsigned char *pSignature) {
    int nResult = TRUE;
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pSignature = 0;            // Default result : no signature

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition,
                                     TAG_KERNEL_SIGNATURE_REQUESTED,
                                     &ulReadLength, &pReadValue);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_payWave_RetreiveSignature : Unable to get SigantureRequested from the payWave kernel response (cr = %02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    // Get the transaction outcome
    *pSignature = pReadValue[0];

  End:
    return (nResult);
}

//! \brief Calls the payWave kernel to perform the transaction.
//! \return
//!     - payWave kernel result.
int clsMVPayWavePerformTransaction(void) {
    trcS("clsMVPayWavePerformTransaction Beg\n");
    int cr, nFound;
    int nResult = C_CLESS_CR_END;
    int nCurrencyCode;
    unsigned char ucOnLinePinRequested;

    unsigned char ucSignature;
    unsigned long ulAmount = 0;
    unsigned long ulLgAmount;
    unsigned char *pInfo;
    unsigned char *pCurrencyCode;
    int merchLang, nCardHolderLang;

    merchLang = PSQ_Give_Language();

    // Indicate payWave kernel is going to be used (for customisation purposes)
    //ClessSample_Customisation_SetUsedPaymentScheme (CLESS_SAMPLE_CUST_PAYWAVE);

    if(clsMVPayWaveAddPayPassSpecificData(kernelBuffer)) {
        GTL_Traces_TraceDebug
            ("ClessSample_payWave_PerformTransaction : __ClessSample_payWave_AddpayWaveSpecificData error\n");
    } else {
        // Dump all the data sent to the payWave kernel
        ////ClessSample_DumpData_DumpSharedBuffer(pDataStruct, 0);

        // For later use
        ulAmount = 0;
        nCurrencyCode = 0;

        //nFound = ClessSample_Common_RetreiveInfo (pDataStruct, TAG_EMV_AMOUNT_AUTH_BIN, &pInfo);
        nFound =
            clsMVCommonRetreiveInfo(kernelBuffer, TAG_EMV_AMOUNT_AUTH_BIN,
                                    &pInfo);
        if(nFound) {
            ulLgAmount = 4;
            GTL_Convert_BinNumberToUl(pInfo, &ulAmount, ulLgAmount);
        }
        //nFound = ClessSample_Common_RetreiveInfo (pDataStruct, TAG_EMV_TRANSACTION_CURRENCY_CODE, &pCurrencyCode);
        nFound =
            clsMVCommonRetreiveInfo(kernelBuffer,
                                    TAG_EMV_TRANSACTION_CURRENCY_CODE,
                                    &pCurrencyCode);
        if(nFound) {
            nCurrencyCode = ((*pCurrencyCode) << 8) + *(pCurrencyCode + 1);
        }
        // End for later use

        // Perform the payWave transaction
        cr = payWave_DoTransaction(kernelBuffer);

        // Get prefered card language (defaulty returned by the payWave kernel (if present in the kernel database)
        nCardHolderLang = merchLang;    // By default, cardholder language is set to default language
        /*
           //if (ClessSample_Common_RetreiveInfo (pDataStruct, TAG_KERNEL_SELECTED_PREFERED_LANGUAGE, &pInfo))
           if (clsMVCommonRetreiveInfo(kernelBuffer, TAG_KERNEL_SELECTED_PREFERED_LANGUAGE, &pInfo))
           nCardHolderLang = ClessSample_Term_GiveLangNumber(pInfo);
         */

        // Specific treatment for on-line/Pin management
        ucOnLinePinRequested = 0;   // Default value : no Online Pin requested
        if((cr == KERNEL_STATUS_OFFLINE_APPROVED)
           || (cr == KERNEL_STATUS_ONLINE_AUTHORISATION)) {
            clsMVPayWave_PinOnLineRequiredState(kernelBuffer,
                                                &ucOnLinePinRequested);
            if(ucOnLinePinRequested)    // If pin asked
                cr = KERNEL_STATUS_ONLINE_AUTHORISATION;    // => mandatory to go on-line
        }
        // Check if signature is requested or not
        ucSignature = 0;
        clsMVPayWave_RetreiveSignature(kernelBuffer, &ucSignature);

        switch (cr) {
          case KERNEL_STATUS_OK:
              nResult = cmvKernelStatusOK;
              break;

          case (KERNEL_STATUS_OFFLINE_APPROVED):
              nResult = cmvKernelStatusOfflineTC;
              break;

          case (KERNEL_STATUS_OFFLINE_DECLINED):
              nResult = cmvKernelStatusOfflineAAC;
              break;

          case (KERNEL_STATUS_ONLINE_AUTHORISATION):
              nResult = cmvKernelStatusARQC;
              break;

          case (KERNEL_STATUS_USE_CONTACT_INTERFACE):
              nResult = cmvKernelStatusUseContactInterface;
              break;

          case (KERNEL_STATUS_COMMUNICATION_ERROR):
              nResult = cmvKernelStatusCommError;
              break;

          case (KERNEL_STATUS_REMOVE_AID):
              nResult = cmvKernelStatusRemoveAID;
              break;

          default:
              nResult = cmvKernelStatusUnknown;
              break;
        }

        // Cless field must be stopped only if we don't try to work with an another AID
        if(nResult != CLESS_CR_MANAGER_REMOVE_AID)
            // Deselect the card
            CLESS_DeselectCard(NULL, 0, TRUE, FALSE);

    }
    clsMVEndTransaction();
    trcFN("clsMVPayWavePerformTransaction :%d\n", nResult);
    return (nResult);

}

//! \brief Fill buffer with specific VisaWave for transaction.
//! \param[out] pDataStruct Shared exchange structure filled with the specific VisaWave data.
//! \return
//!     - \ref TRUE if correctly performed.
//!     - \ref FALSE if an error occured.
//! __ClessSample_VisaWave_AddVisaWaveSpecificData
static clsMVVisaWaveAddVisaWaveSpecificData(T_SHARED_DATA_STRUCT * pDataStruct) {
    int cr, nResult;
    T_KERNEL_TRANSACTION_FLOW_CUSTOM sTransactionFlowCustom;
    object_info_t ObjectInfo;
    unsigned char StepInterruption[KERNEL_PAYMENT_FLOW_STOP_LENGTH];    // Bit field to stop payment flow,

    // if all bit set to 0 => no stop during payment process
    // if right bit set to 1 : stop after payment step number 1
    unsigned char StepCustom[KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH];    // Bit field to custom payment flow,

    // if all bit set to 0 => no stop during payment process
    // if right bit set to 1 : stop after payment step number 1
    if(pDataStruct == NULL) {
        GTL_Traces_TraceDebug
            ("__ClessSample_VisaWave_AddVisaWaveSpecificData : Invalid Input data");
        nResult = FALSE;
        goto End;
    }
    // Init parameteters
    memset(StepInterruption, 0, sizeof(StepInterruption));  // Default Value : not stop on process
    memset(StepCustom, 0, sizeof(StepCustom));  // Default Value : not stop on process
    nResult = TRUE;

    // Customize steps :
    ADD_STEP_CUSTOM(STEP_VISAWAVE_REMOVE_CARD, StepCustom); // To do GUI when card has been read
    ADD_STEP_CUSTOM(STEP_VISAWAVE_GET_CERTIFICATE, StepCustom); // To get the certifiacte for ODA step

    ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(), &ObjectInfo);

    // Add a tag for Do_Txn management
    cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_PAYMENT_FLOW_STOP,
                                   KERNEL_PAYMENT_FLOW_STOP_LENGTH,
                                   (const unsigned char *) StepInterruption);
    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_VisaWave_AddVisaWaveSpecificData : Unable to add TAG_KERNEL_PAYMENT_FLOW_STOP in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }
    // Add a tag for Do_Txn management
    memcpy((void *) &sTransactionFlowCustom, (void *) StepCustom,
           KERNEL_PAYMENT_FLOW_CUSTOM_LENGTH);
    sTransactionFlowCustom.usApplicationType = ObjectInfo.application_type; // Kernel will call this application for customisation 
    sTransactionFlowCustom.usServiceId = SERVICE_CUSTOM_KERNEL; // Kernel will call SERVICE_CUSTOM_KERNEL service id for customisation

    cr = GTL_SharedExchange_AddTag(pDataStruct, TAG_KERNEL_PAYMENT_FLOW_CUSTOM,
                                   sizeof(T_KERNEL_TRANSACTION_FLOW_CUSTOM),
                                   (const unsigned char *)
                                   &sTransactionFlowCustom);
    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        GTL_Traces_TraceDebug
            ("__ClessSample_VisaWave_AddVisaWaveSpecificData : Unable to add TAG_KERNEL_PAYMENT_FLOW_CUSTOM in shared buffer (cr=%02x)",
             cr);
        nResult = FALSE;
        goto End;
    }

  End:
    return (nResult);
}

static int clsMVVisaWaveRetrieveCvmToApply(T_SHARED_DATA_STRUCT *
                                           pResultDataStruct,
                                           unsigned char *pCvm) {
    int nResult = TRUE;
    int nPosition, cr;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;

    nPosition = SHARED_EXCHANGE_POSITION_NULL;
    *pCvm = VISAWAVE_CVM_NO_CVM;    // Default result

    cr = GTL_SharedExchange_FindNext(pResultDataStruct, &nPosition,
                                     TAG_VISAWAVE_TRANSACTION_CVM,
                                     &ulReadLength, &pReadValue);

    if(cr != STATUS_SHARED_EXCHANGE_OK) {
        nResult = FALSE;
        goto End;
    }
    // Get the transaction outcome
    *pCvm = pReadValue[0];

  End:
    return (nResult);
}

//! \brief Calls the VisaWave kernel to perform the transaction.
//! \return
//!     - VisaWave kernel result.
int clsMVVisaWavePerformTransaction(void) {
    int cr, nFound;
    int nResult = C_CLESS_CR_END;
    int nPosition;
    int nCurrencyCode;
    unsigned long ulReadLength;
    const unsigned char *pReadValue;
    unsigned char bOnlinePinError = FALSE;
    BUFFER_SAISIE buffer_saisie;
    unsigned char ucCvm;

    unsigned char cr_pin_online, ucOnLineDeclined;
    const unsigned char *pPan;
    unsigned long ulLgPan;
    unsigned long ulAmount = 0;
    unsigned long ulLgAmount;
    const unsigned char *pAmount;
    unsigned char *pInfo;
    unsigned char *pCurrencyCode;
    int bSaveInBatch = FALSE;
    int nCardHolderLang;
    MSGinfos tDisplayMsg, tDisplayMsg2;

    // Indicate VisaWave kernel is going to be used (for customisation purposes)
    //ClessSample_Customisation_SetUsedPaymentScheme (CLESS_SAMPLE_CUST_VISAWAVE);

    if(!clsMVVisaWaveAddVisaWaveSpecificData(kernelBuffer)) {
        GTL_Traces_TraceDebug
            ("ClessSample_VisaWave_PerformTransaction : __ClessSample_VisaWave_AddVisaWaveSpecificData error\n");
    } else {
        // Dump all the data sent to the VisaWave kernel
        ////ClessSample_DumpData_DumpSharedBuffer(pDataStruct, 0);

        // For later use
        ulAmount = 0;
        nCurrencyCode = 0;

        //nFound = ClessSample_Common_RetreiveInfo (pDataStruct, TAG_EMV_AMOUNT_AUTH_BIN, &pInfo);
        nFound =
            clsMVCommonRetreiveInfo(kernelBuffer, TAG_EMV_AMOUNT_AUTH_BIN,
                                    &pInfo);
        if(nFound) {
            ulLgAmount = 4;
            GTL_Convert_BinNumberToUl(pInfo, &ulAmount, ulLgAmount);
        }
        //nFound = ClessSample_Common_RetreiveInfo (pDataStruct, TAG_EMV_TRANSACTION_CURRENCY_CODE, &pCurrencyCode);
        nFound =
            clsMVCommonRetreiveInfo(kernelBuffer,
                                    TAG_EMV_TRANSACTION_CURRENCY_CODE,
                                    &pCurrencyCode);
        if(nFound) {
            nCurrencyCode = ((*pCurrencyCode) << 8) + *(pCurrencyCode + 1);
        }
        // End for later use

        // Perform the VisaWave transaction
        cr = VisaWave_DoTransaction(kernelBuffer);
        GTL_Traces_TraceDebug("VisaWave_DoTransaction result = %02x", cr);

        // Get the TAG_VISAWAVE_TRANSACTION_CVM to identify the CVM to be performed :
        //  - VISAWAVE_CVM_NO_CVM (0x01) : "No CVM" method has been applied.
        //  - VISAWAVE_CVM_SIGNATURE (0x02) : "Signature" method has been applied.
        //  - VISAWAVE_CVM_ONLINE_PIN (0x04) : "Online PIN" method has been applied.
        if(!clsMVVisaWaveRetrieveCvmToApply(kernelBuffer, &ucCvm)) {
            ucCvm = VISAWAVE_CVM_NO_CVM;
        }

        if((ucCvm == VISAWAVE_CVM_ONLINE_PIN)
           && (cr == KERNEL_STATUS_OFFLINE_APPROVED))
            cr = KERNEL_STATUS_ONLINE_AUTHORISATION;

        switch (cr) {
          case KERNEL_STATUS_OK:
              nResult = cmvKernelStatusOK;
              break;

          case (KERNEL_STATUS_OFFLINE_APPROVED):
              nResult = cmvKernelStatusOfflineTC;
              break;

          case (KERNEL_STATUS_OFFLINE_DECLINED):
              nResult = cmvKernelStatusOfflineAAC;
              break;

          case (KERNEL_STATUS_ONLINE_AUTHORISATION):
              nResult = cmvKernelStatusARQC;
              break;

          case (KERNEL_STATUS_USE_CONTACT_INTERFACE):
              nResult = cmvKernelStatusUseContactInterface;
              break;

          case (KERNEL_STATUS_COMMUNICATION_ERROR):
              nResult = cmvKernelStatusCommError;
              break;

          case (KERNEL_STATUS_REMOVE_AID):
              nResult = cmvKernelStatusRemoveAID;
              break;

          default:
              nResult = cmvKernelStatusUnknown;
              break;
        }

    }

    // Additional possible processing :
    //  - Perform an online authorisation if necessary
    //  - Save the transaction in the batch if transaction is accepted
    //  - Perform CVM processing if necessary

  End:

    // Transaction is completed, clear VisaWave kernel transaction data
    clsMVEndTransaction();
    trcFN("clsMVVisaWavePerformTransaction :%d\n", nResult);
    return (nResult);

}

/** SetLedColor force color of led for internal contactless
 * \param led : num of led 0 to 3.
 * \param color : Color RGB
 * \return non-negative value if OK; negative otherwise
 */
int clsSetLedColor(int led, int color) {
    int clr;

    switch (color) {
      case 'R':
          clr = 0xFFFF0000;
          break;

      case 'Y':
          clr = 0xFFFFFF00;
          break;

      case 'C':
          clr = 0xFF00FFFF;
          break;

      case 'O':
          clr = 0xFFFF4500;
          break;

      case 'G':
          clr = 0xFF00FF00;
          break;

      case 'B':
          clr = 0xFF0000FF;
          break;

      default:
          break;

    };

    SetLedColor(led, clr);

    return 1;

}

/** SetLedEvent set the led state for internal contactless
 * \param led   : num of led 0 to 3.
 * \param state : 1- ON or 0 - OFF.
 * \return non-negative value if OK; negative otherwise
 */
int clsSetLed(int led, int state) {

    SetLedEvent(led, state);

    return 1;

}

#endif                          //def __CLS__
#ifdef __BIO__

static const char *bioParseStr(char *dst, const char *src, int dim, char sep) {
    VERIFY(dst);
    VERIFY(src);

    while(*src) {
        if(*src == sep) {
            src++;              // skip separator
            break;
        }
        VERIFY(dim > 1);        // dim includes ending zero
        *dst++ = *src++;
        dim--;
    }

    *dst = 0;
    return src;
}

/** Open the biometrics reader
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcmp0009.c
 */
int bioStart(void) {
    int ret;
    static byte dllOK = 0;

    trcS("bioStart\n");
    if(!dllOK) {
        ret = morpholib_open(); // load morpho DLL
        CHECK(ret >= 0, lblKO)
            dllOK = 1;
    }

    SetMorphoPower(C_MORPHO_POWER_ON);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Close the biometrics reader
 * \return none
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcmp0009.c
 */
void bioStop(void) {
    trcS("bioStop\n");
    SetMorphoPower(C_MORPHO_POWER_OFF);
}

/** Create the biometrics database
 * \param rec (I) max number of records in the database
 * \param fng (I) number of fingers to scan: 1, 2, 10 or 20
 * \param fld (I) string of tags, sizes and field names separated by '|'
 * \param dly (I) timeout
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcmp0012.c
 */
int bioCreateDB(word rec, byte fng, char *fld, int dly) {
    T_DataFieldList *dta = NULL, *pDta = NULL;
    int ret;
    int cnt = 0;
    bool wait = TRUE;
    card num;
    char *init = fld;
    char tag[1 + 1];
    char sze[4 + 1];
    char sep = '|';
    byte reqSta;

    trcS("bioCreateDB\n");
    while(init[0] != 0) {
        if(pDta != NULL) {
            pDta->next = umalloc(sizeof(T_DataFieldList));
            pDta = pDta->next;
        } else {
            dta = umalloc(sizeof(T_DataFieldList));
            pDta = dta;
        }
        init = (char *) bioParseStr(tag, init, sizeof(tag), sep);
        CHECK(init != 0, lblKO);

        pDta->data_i.tag = (tag[0] == '0') ? C_MORPHO_ID_PUBLIC_FIELD :
            C_MORPHO_ID_PRIVATE_FIELD;

        init = (char *) bioParseStr(sze, init, sizeof(sze), sep);
        CHECK(init != 0, lblKO);
        dec2num(&num, sze, 3);
        CHECK(num <= 128, lblKO);
        pDta->data_i.max_size = num;

        pDta->data_i.ptr = umalloc(6 + 1);
        init = (char *) bioParseStr(pDta->data_i.ptr, init, 6 + 1, sep);
        pDta->data_i.size = strlen(pDta->data_i.ptr);
        cnt++;
    }
    pDta->next = NULL;
    ret = Morpho_SendCreateDataBase(rec, fng, dta, 0);

    CHECK(ret == COMBIO_OK, lblKO);

    //wait for keyboard or morpho event
    ret = ttestall(MORPHO | KEYBOARD, dly * 100);
    if(ret == MORPHO) {         //morpho event occurred
        ret = Morpho_ReadCreateDataBase(&reqSta);
        CHECK(ret == COMBIO_OK, lblKO);
        CHECK(reqSta == ILV_OK, lblKO);
    } else {
        wait = FALSE;
        Morpho_SendCancel();
        if(ret == KEYBOARD) {   //TODO: test cancel selected
            trcS("Remove Aborted\n");
        } else {                //timeout occured
            trcS("Timeout!!!\n");
        }

        //wait for the answer from Morpho
        ret = ttestall(MORPHO, dly);
        if(ret & MORPHO) {
            ret = Morpho_ReadCreateDataBase(&reqSta);
        } else {
            trcS("Morpho Failed Abort Seq\n");
            VERIFY(0);
        }
        goto lblKO;
    }                           /* if MORPHO | KEYBOARD | timeout */
    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    while(dta != NULL) {
        pDta = dta;
        dta = dta->next;
        ufree(pDta->data_i.ptr);
        ufree(pDta);
    }
    trcFN("bioCreateDB ret= %d\n", ret);
    return ret;

}

/** Destroy all biometrics database
 * \param dly (I) timeout
 * \return none
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcmp0014.c
 */
int bioDestroyDB(int dly) {
    int ret;
    bool wait = TRUE;
    byte reqSta;

    trcS("biodDestroyDB\n");
    ret = Morpho_SendDestroyAllBase();
    CHECK(ret == COMBIO_OK, lblKO);

    //wait for keyboard or morpho event
    ret = ttestall(MORPHO | KEYBOARD, dly * 100);
    if(ret == MORPHO) {         //morpho event occurred
        ret = Morpho_ReadDestroyAllBase(&reqSta);
        CHECK(ret == COMBIO_OK, lblKO);
        CHECK(reqSta == ILV_OK, lblKO);
    } else {
        wait = FALSE;
        Morpho_SendCancel();
        if(ret == KEYBOARD) {   //TODO: test cancel selected
            trcS("Remove Aborted\n");
        } else {                //timeout occured
            trcS("Timeout!!!\n");
        }

        //wait for the answer from Morpho
        ret = ttestall(MORPHO, dly);
        if(ret & MORPHO) {
            ret = Morpho_ReadDestroyAllBase(&reqSta);
        } else {
            trcS("Morpho Failed Abort Seq\n");
            VERIFY(0);
        }
        goto lblKO;
    }                           /* if MORPHO | KEYBOARD | timeout */
    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("bioRemove ret= %d\n", ret);
    return ret;

}

/** Register fingerprint to the database or as one of the reference templates
 * \param loc (I) location where the data will be stored
 * \param dly (I) timeout
 * \param uid (I) User ID
 * \param udt (I) User Additional Data 
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcmp0012.c
 */
int bioEnroll(int loc, int dly, char *uid, char *udt) {
    const T_LV nullId = { 0, NULL };
    T_LV userId = { 0, NULL };
    T_AddUserDataList dtaList[MAX_FIELD_CNT], *pDtaList = NULL;
    T_TemplateList bioDta;
    T_asynch_cmd cmd;
    int ctr = 0, ret;
    unsigned int usrIdx;
    byte cmdType;
    byte reqSta;
    byte enrSta;
    byte enrType = 3;
    bool wait = TRUE, saveDB = TRUE;
    char dtaStr[MAX_FIELD_CNT][128 + 1];
    char *init = udt;

    trcFN("bioEnroll Beg loc=%d\n", loc);
    if(loc == 0) {
        enrType = 1;
        saveDB = FALSE;
    } else {
        CHECK(uid != NULL, lblKO);
        CHECK(uid[0] != 0, lblKO);
        userId.ptr = uid;
        userId.size = strlen(uid);
        memset(dtaList, 0, sizeof(dtaList));
        memset(dtaStr, 0, sizeof(dtaStr));
        if(init != NULL) {
            while(ctr < MAX_FIELD_CNT) {
                init =
                    (char *) bioParseStr(dtaStr[ctr], init, sizeof(dtaStr[0]),
                                         '|');
                CHECK(init != 0, lblKO);

                dtaList[ctr].userdata_i.ptr = dtaStr[ctr];
                dtaList[ctr].userdata_i.size = strlen(dtaStr[ctr]);
                if(init[0] != 0) {
                    dtaList[ctr].next = &dtaList[ctr + 1];
                } else {
                    dtaList[ctr].next = NULL;
                    break;
                }
                ctr++;
            }
            pDtaList = dtaList;
        }
    }
    ret = Morpho_SendEnroll(dly,    // tmo: timeout
                            enrType,    // enrollment_type
                            1,  // nb of fingers
                            saveDB, // save record
                            1,  // minutiae size, export biometric data
                            userId, // user id
                            pDtaList,   // user_data
                            1,  // asynch_event, accept only commands
                            0,  // biometric_algo, use default
                            0,  // export_image, image not returned
                            0,  // compression_value, unused
                            nullId, // biometric_token, unused
                            0,  // latent detection, disabled
                            0); // export_result, no result asked
    CHECK(ret == COMBIO_OK, lblKO);

    while(wait) {
        //wait for keyboard or morpho event
        ret = ttestall(MORPHO | KEYBOARD, dly * 100);
        if(ret == MORPHO) {     //morpho event occurred
            ret =
                Morpho_ReadEnroll(&reqSta, &enrSta, &usrIdx, &bioDta, NULL,
                                  NULL);
            if(ret == COMBIO_ASYNCHRONE_ID) //asynchronous msg received
            {
                ret =
                    Morpho_ReadAsynchronousMessage(&reqSta, &cmdType, &cmd,
                                                   NULL);

                trcFN("returned status: %d ", ret);
                trcFN("request status: %d ", reqSta);
                trcFN("command type: %d ", cmdType);

                if(cmdType & C_MORPHO_ID_MESSAGE_COMMAND_CMD) {
                    trcFN("command status: %d ", cmd.command);
                    if(cmd.command == 8) {
                        dspStart();
                        dspClear();
                        Click();
                        dspLS(1, "Sample OK");
                        dspLS(2, "Next Sample");
                        dspStop();
                    } else if((cmd.command > 0) && (cmd.command < 6)) {
                        dspStart();
                        dspClear();
                        Beep();
                        dspLS(1, "Sample FAIL");
                        dspLS(2, "Try again");
                        dspStop();
                    }
                }
                if(cmdType & C_MORPHO_ID_MESSAGE_ENROLLEMENT_CMD) {
                    trcS("command enroll\n");
                }
            } else {
                CHECK(ret == COMBIO_OK, lblKO);
                CHECK(reqSta == ILV_OK, lblKO);
                CHECK(enrSta == 0, lblKO);

                //new template acquired
                trcFN("read result: %d ", reqSta);
                trcFN("enroll result: %d ", enrSta);
                trcFN("user index: %d\n", usrIdx);

                //store template as reference
                if(loc == 0) {
                    if(bioRefTemp == NULL) {
                        bioRefTemp = umalloc(sizeof(T_TemplateList));
                        bioRefTemp->next = NULL;
                        bioRefTemp->template_i = bioDta.template_i;
                        bioRefTemp->template_i.template_ptr =
                            umalloc(bioRefTemp->template_i.size);
                        memcpy(bioRefTemp->template_i.template_ptr,
                               bioDta.template_i.template_ptr,
                               bioRefTemp->template_i.size);
                    } else {
                        T_TemplateList *pList;

                        pList = bioRefTemp;
                        while(pList->next != NULL) {
                            pList = pList->next;
                        }
                        pList->next = umalloc(sizeof(T_TemplateList));
                        pList = pList->next;
                        pList->next = NULL;
                        pList->template_i = bioDta.template_i;
                        pList->template_i.template_ptr =
                            umalloc(pList->template_i.size);
                        memcpy(pList->template_i.template_ptr,
                               bioDta.template_i.template_ptr,
                               pList->template_i.size);
                    }
                }

                wait = FALSE;
                trcS("Enrollement OK\n");
            }
        } else {
            wait = FALSE;
            Morpho_SendCancel();
            if(ret == KEYBOARD) {   //TODO: test cancel selected
                trcS("Enroll Aborted\n");
            } else {            //timeout occured
                trcS("Timeout!!!\n");
            }

            //wait for the answer from Morpho
            ret = ttestall(MORPHO, dly);
            if(ret & MORPHO) {
                ret =
                    Morpho_ReadEnroll(&reqSta, &enrSta, &usrIdx, &bioDta, NULL,
                                      NULL);
            } else {
                trcS("Morpho Failed Abort Seq\n");
                VERIFY(0);
            }
            goto lblKO;
        }                       /* if MORPHO | KEYBOARD | timeout */
    }                           /* while wait */
    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("BioEnroll ret= %d\n", ret);
    return ret;
}

/** Delete all records in the biometrics database
 * \param loc (I) location to determine what will be deleted (0 to delete template list else delete database)
 * \param dly (I) timeout
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcmp0011.c
 */
int bioClearDB(int loc, int dly) {
    int ret;
    bool wait = TRUE;

    trcFN("bioClearDB Beg loc=%d\n", loc);
    //delete the list of reference templates
    if(loc == 0) {
        T_TemplateList *pList, *pCurList;

        pList = bioRefTemp;
        while(pList != NULL) {
            pCurList = pList;
            pList = pList->next;
            ufree(pCurList->template_i.template_ptr);
            ufree(pCurList);
        }
        bioRefTemp = NULL;
    } else {
        byte reqSta;

        ret = Morpho_SendEraseBase();
        CHECK(ret == COMBIO_OK, lblKO);

        //wait for keyboard or morpho event
        ret = ttestall(MORPHO | KEYBOARD, dly * 100);
        if(ret == MORPHO) {     //morpho event occurred
            ret = Morpho_ReadEraseBase(&reqSta);
            CHECK(ret == COMBIO_OK, lblKO);
            CHECK(reqSta == ILV_OK, lblKO);
            wait = FALSE;
        } else {
            wait = FALSE;
            Morpho_SendCancel();
            if(ret == KEYBOARD) {   //TODO: test cancel selected
                trcS("ClrDB Aborted\n");
            } else {            //timeout occured
                trcS("Timeout!!!\n");
            }

            //wait for the answer from Morpho
            ret = ttestall(MORPHO, dly);
            if(ret & MORPHO) {
                ret = Morpho_ReadEraseBase(&reqSta);
            } else {
                trcS("Morpho Failed Abort Seq\n");
                VERIFY(0);
            }
            goto lblKO;
        }                       /* if MORPHO | KEYBOARD | timeout */
    }
    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("bioClearDB ret= %d\n", ret);
    return ret;
}

/** Delete a record from the database
 * \param uid (I) user Id of the record to be deleted
 * \param dly (I) timeout
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmext.c
 * \test tcmp0015.c
 */
int bioRemove(char *uid, int dly) {
    T_LV userId;
    int ret;
    byte reqSta;
    bool wait = TRUE;

    trcFS("bioRemove Beg uid=%s\n", uid);

    CHECK(uid != NULL, lblKO);
    CHECK(uid[0] != 0, lblKO);
    userId.ptr = uid;
    userId.size = strlen(uid);
    ret = Morpho_SendRemoveBaseRecord(userId, 0);
    CHECK(ret == COMBIO_OK, lblKO);

    //wait for keyboard or morpho event
    ret = ttestall(MORPHO | KEYBOARD, dly * 100);
    if(ret == MORPHO) {         //morpho event occurred
        ret = Morpho_ReadRemoveBaseRecord(&reqSta);
        CHECK(ret == COMBIO_OK, lblKO);
        CHECK(reqSta == ILV_OK, lblKO);
    } else {
        wait = FALSE;
        Morpho_SendCancel();
        if(ret == KEYBOARD) {   //TODO: test cancel selected
            trcS("Remove Aborted\n");
        } else {                //timeout occured
            trcS("Timeout!!!\n");
        }

        //wait for the answer from Morpho
        ret = ttestall(MORPHO, dly);
        if(ret & MORPHO) {
            ret = Morpho_ReadRemoveBaseRecord(&reqSta);
        } else {
            trcS("Morpho Failed Abort Seq\n");
            VERIFY(0);
        }
        goto lblKO;
    }                           /* if MORPHO | KEYBOARD | timeout */
    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("bioRemove ret= %d\n", ret);
    return ret;
}

/** Compare the fingerprint to a single record in the reference template list
 * \param dly (I) timeout
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcmp0010.c
 */
char bioVerify(int dly) {
    T_asynch_cmd cmd;
    int ret;
    int wait = TRUE;
    byte reqSta;
    byte cmdType;
    char match = -1;

    trcS("bioVerify Start\n");

    if(bioRefTemp == NULL) {
        trcS("Empty Ref Templates List\n");
    }
    CHECK(bioRefTemp != NULL, lblKO);

    ret = Morpho_SendVerify(dly,    // tmo: timeout
                            5,  // matching threshold, 5 recommended
                            bioRefTemp, // pointer to reference template
                            1,  // asynch_event, accept only commands
                            0); // matching score, do not return
    trcFN("result send: %d\n", ret);
    CHECK(ret == COMBIO_OK, lblKO);

    while(wait) {
        //wait for keyboard or morpho event
        ret = ttestall(MORPHO | KEYBOARD, dly * 100);
        if(ret == MORPHO) {     //morpho event occurred
            ret = Morpho_ReadVerify(&reqSta, &match, NULL);
            if(ret == COMBIO_ASYNCHRONE_ID) //asynchronous msg received
            {
                ret =
                    Morpho_ReadAsynchronousMessage(&reqSta, &cmdType, &cmd,
                                                   NULL);

                trcFN("returned status: %d ", ret);
                trcFN("request status: %d ", reqSta);
                trcFN("command type: %d ", cmdType);

                if(cmdType & C_MORPHO_ID_MESSAGE_COMMAND_CMD) {
                    trcFN("command status: %d ", cmd.command);
                    if((cmd.command > 0) && (cmd.command < 6)) {
                        dspStart();
                        dspClear();
                        Beep();
                        dspLS(1, "Sample FAIL");
                        dspLS(2, "Try again");
                        dspStop();
                    }
                }
            } else {
                CHECK(ret == COMBIO_OK, lblKO);
                ret = reqSta;
                CHECK(reqSta == ILV_OK, lblKO);

                //sample acquired
                trcFN("read result: %d ", reqSta);
                trcFN("matching result: %d ", match);

                switch (match) {
                  case 1:
                      trcS("same finger\n");
                      ret = 1;
                      break;
                  case 2:
                      trcS("different finger\n");
                      ret = 0;
                      break;
                  default:
                      trcS("verification KO\n");
                      ret = match;
                      goto lblKO;
                      break;
                }

                wait = FALSE;
                trcS("Enrollement OK\n");
            }
        } else {
            wait = FALSE;
            Morpho_SendCancel();
            if(ret == KEYBOARD) {   //TODO: test cancel selected
                trcS("Verify Aborted\n");
            } else {            //timeout occured
                trcS("Timeout!!!\n");
            }

            //wait for the answer from Morpho
            ret = ttestall(MORPHO, dly);
            if(ret & MORPHO) {
                ret = Morpho_ReadVerify(&reqSta, &match, NULL);
            } else {
                trcS("Morpho Failed Abort Seq\n");
                VERIFY(0);
            }
            goto lblKO;
        }                       /* if MORPHO | KEYBOARD | timeout */
    }                           /* while wait */
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("BioVerify End= %d\n", ret);
    return ret;
}

/** Compare the fingerprint to a single record in the database
 * \param dly (I) timeout
 * \param uin (O) user index where the data is found in the database
 * \param uid (O) user Id of the found record
 * \param udt (O) user additional data of the record found
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcmp0013.c
 */
char bioIdentify(int dly, card * uin, char *uid, char *udt) {
    T_asynch_cmd cmd;
    T_LV userId;
    T_AddUserDataList userData, *pData;
    unsigned int userIndex;
    int ret;
    int wait = TRUE;
    byte reqSta;
    byte cmdType;
    char match = -1;

    trcS("bioVerify Start\n");
    CHECK(uin != NULL, lblKO);
    CHECK(uid != NULL, lblKO);
    CHECK(udt != NULL, lblKO);

    ret = Morpho_SendIdentify(dly,  // tmo: timeout
                              5,    // matching threshold, 5 recommended
                              1,    // asynch_event, accept only commands
                              0);   // matching score, do not return
    trcFN("result send: %d\n", ret);
    CHECK(ret == COMBIO_OK, lblKO);

    while(wait) {
        //wait for keyboard or morpho event
        ret = ttestall(MORPHO | KEYBOARD, dly * 100);
        if(ret == MORPHO) {     //morpho event occurred
            ret = Morpho_ReadIdentify(&reqSta, &match, &userIndex, &userId,
                                      &userData, NULL);

            if(ret == COMBIO_ASYNCHRONE_ID) //asynchronous msg received
            {
                ret =
                    Morpho_ReadAsynchronousMessage(&reqSta, &cmdType, &cmd,
                                                   NULL);

                trcFN("returned status: %d ", ret);
                trcFN("request status: %d ", reqSta);
                trcFN("command type: %d ", cmdType);

                if(cmdType & C_MORPHO_ID_MESSAGE_COMMAND_CMD) {
                    trcFN("command status: %d ", cmd.command);
                    if((cmd.command > 0) && (cmd.command < 6)) {
                        dspStart();
                        dspClear();
                        Beep();
                        dspLS(1, "Sample FAIL");
                        dspLS(2, "Try again");
                        dspStop();
                    }
                }
            } else {
                CHECK(ret == COMBIO_OK, lblKO);
                ret = reqSta;
                CHECK(reqSta == ILV_OK, lblKO);

                //sample acquired
                trcFN("read result: %d ", reqSta);
                trcFN("matching result: %d ", match);

                switch (match) {
                  case 1:
                      trcS("same finger\n");
                      *uin = userIndex;
                      strncpy(uid, userId.ptr, userId.size);
                      if((userData.userdata_i.size != 0) ||
                         (userData.userdata_i.ptr != NULL)) {
                          strncpy(udt, userData.userdata_i.ptr,
                                  userData.userdata_i.size);
                          pData = userData.next;
                          while(pData != NULL) {
                              if((pData->userdata_i.size == 0) ||
                                 (pData->userdata_i.ptr == NULL)) {
                                  break;
                              }
                              strcat(udt, "|");
                              strncat(udt, pData->userdata_i.ptr,
                                      pData->userdata_i.size);
                              pData = pData->next;
                          }
                      }
                      ret = 1;
                      break;
                  case 2:
                      trcS("different finger\n");
                      ret = 0;
                      break;
                  default:
                      trcS("Identify KO\n");
                      ret = match;
                      goto lblKO;
                      break;
                }

                wait = FALSE;
                trcS("Identify OK\n");
            }
        } else {
            wait = FALSE;
            Morpho_SendCancel();
            if(ret == KEYBOARD) {   //TODO: test cancel selected
                trcS("Identify Aborted\n");
            } else {            //timeout occured
                trcS("Timeout!!!\n");
            }

            //wait for the answer from Morpho
            ret = ttestall(MORPHO, dly);
            if(ret & MORPHO) {
                ret = Morpho_ReadIdentify(&reqSta, &match, &userIndex, &userId,
                                          &userData, NULL);
            } else {
                trcS("Morpho Failed Abort Seq\n");
                VERIFY(0);
            }
            goto lblKO;
        }                       /* if MORPHO | KEYBOARD | timeout */
    }                           /* while wait */
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("bioIdentify End= %d\n", ret);
    return ret;
}

#endif                          //def __BIO__
