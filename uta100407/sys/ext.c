/** \file
 * UNICAPT external devices
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/ext.c $
 *
 * $Id: ext.c 2610 2010-04-07 10:03:06Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include "sys.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcEXT))

#define RSP_LEN 1024

typedef struct {
    int16 sta;
    uint16 len;
    char buf[RSP_LEN];
} tResponse;

#ifdef __CLSMV__
static byte svcStaCod[2];
static int nextState;
#endif

#ifdef __BCR__

/** Open the bar code reader, but not wait the bar code
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcgs0022.c
 */
#ifdef WIN32

#if (__INGEDEV__ == 5)
static FILE *fBcr = 0;
#endif

int bcrStart(void) {
#if (__INGEDEV__ == 5)
    int ret;

    fBcr = fopen("barcode.txt", "rt");
    CHECK(fBcr, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
#else
    return -1;
#endif
}
#else

static uint32 hBcr = 0;
static uint8 bcrSta = 0;

static int bcrOpenCom(void) {
    int ret;
    comInit_t cIni;
    comUartInit_t uIni;
    comProtocolCfg_t pCfg;
    comUartProtocolCfg_t uCfg;

    trcS("bcrOpenCom beg \n");

    ret = comOpen("COM4", &hBcr);
    CHECK(ret == RET_OK, lblKO);

    cIni.type = COM_INIT_TYPE_UART;
    cIni.level = COM_INIT_LEVEL_1;
    cIni.init.uartInit = &uIni;

    uIni.speed = COM_BAUD_115200;
    uIni.parity = COM_PARITY_NONE;
    uIni.dataSize = COM_DATASIZE_8;
    uIni.stopBits = COM_STOP_1;
    uIni.sBufferSize = 1024;    //0;
    uIni.rBufferSize = 1024;    //0;

    pCfg.type = COM_PROT_CFG_TYPE_UART;
    pCfg.level = COM_PROT_CFG_LEVEL_1;
    pCfg.protCfg.uartCfg = &uCfg;

    uCfg.flowControl = COM_FLOW_HARD;   //COM_FLOW_NONE;
    uCfg.interCharTimeOut = 0;
    uCfg.xOnChar = 0x0;
    uCfg.xOffChar = 0x0;

    ret =
        comChanInit(hBcr, COM_MODIF_PARAM_TEMPORARY, &cIni,
                    COM_SET_PROTOCOL_TEMPORARY, &pCfg);
    CHECK(ret == RET_OK, lblKO);

    trcS("bcrOpenCom End \n");
    ret = RET_OK;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

int bcrStart(void) {
    int ret, ret_code;
    byte i;

    //char imgDefaultCfg[] = "PRECA2;dechdr0;DECMOD0;TRGMOD0;TRGLPT240;232LPT0."; // DEFAULT CONFIG
    char imgDefaultCfg[] = "PRECA2;TRGMOD0;TRGLPT240;DECMOD0;BEPBEP1."; // DEFAULT CONFIG

    trcS("bcrStart beg \n");
    ret = bcrOpenCom();
    CHECK(ret == RET_OK, lblKO);

    trcS("Send default config \n");
    for (i = 0; i < strlen(imgDefaultCfg); i++) {   // Send default config
        ret_code = comSendByte(hBcr, imgDefaultCfg[i]);
        switch (ret_code) {
          case RET_OK:
              break;
          case ERR_BUSY:       // byte is in com send buffer
          case COM_TIMEOUT:
              psyTimerWakeAfter(SYS_TIME_SECOND * 1);
              break;
          default:
              goto lblKO;
        }
    }
    comReceptionFlush(hBcr);
    ret = RET_OK;
    goto lblEnd;
  lblKO:
    trcS("bcrStart ERR\n");
    trcErr(ret);
  lblEnd:
    return ret;
}
#endif

/** Command the bar code reader to start the reading.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcgs0022.c
 */
#ifdef WIN32

int bcrReq(void) {
    return 1;
}

#else

static isBcrReq = 0;

int bcrReq(void) {
    int ret;
    byte i;
    char cmdReq[] = "\x16\x74\x0D";

    trcS("bcrReq ..");
    for (i = 0; i < strlen(cmdReq); i++) {
        ret = comSendByte(hBcr, cmdReq[i]);
        switch (ret) {
          case RET_OK:
              break;
          case ERR_BUSY:       // byte is in com send buffer
          case COM_TIMEOUT:
              psyTimerWakeAfter(SYS_TIME_SECOND * 1);
              break;
          default:
              goto lblKO;
        }
    }
    trcS("bcrReq ok");
    bcrSta = 1;
    isBcrReq = 1;
    ret = 1;
    goto lblEnd;
  lblKO:
    trcS("ERR SEND SYN+t+CR");
    comReceptionFlush(hBcr);
    comTransmissionFlush(hBcr);
    trcErr(ret);
  lblEnd:
    return ret;
}
#endif

/** Stop waiting a bar code and
 * Close the associated channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcgs0022.c
 */

#ifdef WIN32

void bcrStop(void) {
#if (__INGEDEV__ == 5)
    if(fBcr)
        fclose(fBcr);

    fBcr = NULL;
#endif
}

#else

void bcrStop(void) {
    int ret;

    ret = bcrCancel();
    trcS("bcrStop Beg");

    comReceptionFlush(hBcr);
    comTransmissionFlush(hBcr);

    bcrSta = 0;
    ret = comClose(hBcr);
    if(ret < 0)
        trcS("comClose ERR");
    hBcr = 0;
    trcS("bcrStop End \n");
}

#endif

/** Stop waiting a bar code
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcgs0022.c
 */
#ifdef WIN32

int bcrCancel(void) {
    return 1;
}

#else

int bcrCancel(void) {
    int ret = 0;
    byte i;
    char cmdReq[] = "\x16\x75\x0D";
    char buf[24];

    comCancel(hBcr, COM_RECEIVE_EVENT);
    isBcrReq = 0;
    trcS("bcrCancel beg \n");
    sprintf(buf, "bcrSta = %d", bcrSta);
    trcS(buf);
    if(bcrSta) {
        for (i = 0; i < strlen(cmdReq); i++) {
            ret = comSendByte(hBcr, cmdReq[i]);
            switch (ret) {
              case RET_OK:
                  break;
              case ERR_BUSY:   // byte is in com send buffer
              case COM_TIMEOUT:
                  psyTimerWakeAfter(SYS_TIME_SECOND * 1);
                  break;
              default:
                  psyTimerWakeAfter(SYS_TIME_SECOND * 2);
                  break;        //no need to goto lblKO;
            }
        }
        comCancel(hBcr, COM_SEND_EVENT);
    }
    trcS("bcrCancel end \n");
    return ret;
}

#endif

/** Check the status of the bar code reader. If code was readed, then stop
 * the reading and give back the bar code.
 * \param barCode (O) The readed bar code as zero terminated string
 * \return
 *    - negative if failure.
 *    - zero if the reading is running, but nothing was captured yet
 *    - if succes, the number of characters in the barcode captured
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcgs0022.c
 */
#ifdef WIN32

int bcrGet(char *barCode) {
#if (__INGEDEV__ == 5)
    int ret = 0;
    char *ptr;
    char tmp[256];

    VERIFY(barCode);
    memset(tmp, 0, sizeof(tmp));

    ptr = fgets(tmp, 256, fBcr);
    CHECK(ptr, lblKO);

    if(*tmp) {
        ptr = tmp + (strlen(tmp) - 1);
        if(*ptr == '\n')
            *ptr = 0;
        if(*ptr == '\r')
            *ptr = 0;
    }

    if(strlen(tmp)) {
        strcpy(barCode, tmp);
    }

    ret = strlen(barCode);
    goto lblEnd;

  lblKO:
    trcErr(ret);

  lblEnd:
    return ret;
#else
    return 0;
#endif
}

#else

int bcrGet(char *barCode) {
    int ret = 0;
    tResponse rsp;
    int idx = 0;

    trcS("bcrGet Beg\n");
    VERIFY(barCode);
    CHECK(isBcrReq, lblKO);
    memset(rsp.buf, 0, sizeof(rsp.buf));

    while(1) {
        ret = comReceiveByteReq(hBcr);
        CHECK(ret == RET_RUNNING, lblNoCode);
        ret = psyPeripheralResultWait(COM_WAIT, 50, 0); //wait 0,5 second
        if(PSY_PER_EXECUTED_OK(ret, COM_WAIT)) {
            memset(rsp.buf, '\0', 1);
            ret = comResultGet(hBcr, COM_RECEIVE_EVENT, sizeof(rsp), &rsp);
            CHECK(ret != RET_RUNNING, lblNoCode);
        } else
            break;
        if(ret == RET_OK) {
            if(rsp.sta == RET_OK)
                sprintf(barCode, "%s%s", barCode, rsp.buf); // collapse barcode
            idx++;
        }
    }
    barCode[idx] = 0;           //zero end in the string
    ret = idx;
    goto lblEnd;
  lblNoCode:
    ret = 0;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    comReceptionFlush(hBcr);
    comTransmissionFlush(hBcr);
  lblEnd:
    comCancel(hBcr, COM_RECEIVE_EVENT);
    return ret;
}
#endif

/** Read diffrent types of bar code.
 * \param ctl
 *    - (I) Control prefix
 * \param dly
 *    - (I) Delay for bar code reading
 * \param barCode
 * 	  - (O) The readed bar code as zero terminated string
 * \return
 *    - negative if failure.
 *    - zero if the reading is running, but nothing was captured yet
 *    - if succes, the number of characters in the barcode captured
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcgs0022.c
 */
#ifdef WIN32

int bcrRead(char *ctl, byte dly, char *barCode) {
    return -1;
}
#else
int bcrRead(char *ctl, byte dly, char *barCode) {
    int ret;
    tResponse rsp;
    int idx = 0;
    word tmr = 0;
    byte i;

    trcS("bcrRead Beg");
    VERIFY(barCode);

    if(ctl) {
        trcS("Send config");
        for (i = 0; i < strlen(ctl); i++) { // Send config
            ret = comSendByte(hBcr, ctl[i]);
            CHECK(ret == RET_OK, lblKO);
        }
    }

    if(dly)
        tmr = dly * 10;
    else
        tmr = 50;

    ret = comSendByte(hBcr, '\x16');
    CHECK(ret == RET_OK, lblKO);
    ret = comSendByte(hBcr, '\x74');
    CHECK(ret == RET_OK, lblKO);
    ret = comSendByte(hBcr, '\x0D');
    CHECK(ret == RET_OK, lblKO);

    bcrSta = 1;
    memset(rsp.buf, 0, sizeof(rsp.buf));
    while(1) {
        ret = comReceiveByteReq(hBcr);
        CHECK(ret == RET_RUNNING, lblNoCode);
        ret = psyPeripheralResultWait(COM_WAIT, tmr, 0);
        if(PSY_PER_EXECUTED_OK(ret, COM_WAIT)) {
            memset(rsp.buf, '\0', 1);
            ret = comResultGet(hBcr, COM_RECEIVE_EVENT, sizeof(rsp), &rsp);
            CHECK(ret != RET_RUNNING, lblNoCode);
        } else
            break;
        if(ret == RET_OK) {
            if(rsp.sta == RET_OK)
                sprintf(barCode, "%s%s", barCode, rsp.buf); // collapse barcode
            idx++;
        }
    }
    barCode[idx] = 0;           //zero end in the string
    ret = idx;
    goto lblEnd;
  lblNoCode:
    ret = 0;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    comCancel(hBcr, COM_SEND_EVENT);
  lblEnd:
    comTransmissionFlush(hBcr);
    comReceptionFlush(hBcr);
    comCancel(hBcr, COM_RECEIVE_EVENT);
    return ret;
}
#endif

#endif                          // def __BCR__

#ifdef __MFC__
#ifndef __CLS__
#define __CLS__
#endif
#endif

#ifdef __CLS__

#include <pcd.h>
#include <hmi.h>                //For mfcText function

typedef enum ClessCmd {
    MF_Open = 0x0220,
    MF_Close,
    MF_Check,
    MF_Detect,
    MF_Activate,
    MF_Auth,
    MF_Read,
    MF_Write,
    MF_Value,
    CL_SCN = 0x03,
    CL_BUZ = 0x0D,
    CL_LED = 0x0e
} ClessCmd_t;

typedef enum ClessPrm {
    PR_Msg_AskCard = 0x9F9300,
    PR_Msg = 0x9F9154,
    PR_DataBlk = 0x9F9414,
    PR_OperType = 0x9F9415,
    PR_DstBlk = 0x9F9416,
    PR_CardIdx = 0x9F941A,
    PR_KeyType = 0x9F9423,
    PR_UncodeKey = 0x9F9424,
    PR_Sector = 0x9F9425,
    PR_DataFmt = 0x9F9426,
    PR_Block = 0x9F9427
} ClessPrm_t;

typedef enum OperMF {
    OperMF_Read = 0x30,
    OperMF_Write = 0xA0,
    OperMF_Dec = 0xC0,
    OperMF_Inc = 0xC1,
    OperMF_Rst = 0xC2,
    OperMF_Trf = 0xB0
} OperMF_t;

static uint32 pcdHandle;        // handle for Contactless reader

static byte mfcCmd[20] = { 0x00, 0x90, 0xBF, 0x00,
    /* Length L */ 0x00, /* Length U */ 0x00, 0x00, 0x00,
    0x00, 0x90, 0x9F, 0x00,
    /* Length L */ 0x00, /* Length U */ 0x00, 0x00, 0x00,
    /* CMD B1 */ 0x00, /* CMD B2 */ 0x00, /* CMD B3 */ 0x00, /* CMD B4 */ 0x00
};

static int16 AddPrm(uint8 * cmd, unsigned long PrmId, char *val, int16 len) {
    int ret = 0;
    int i;
    const byte mfcStrLen = 12;

    cmd[0] = LBYTE(LWORD(PrmId));
    cmd[1] = HBYTE(LWORD(PrmId));
    cmd[2] = LBYTE(HWORD(PrmId));
    cmd[3] = 0x00;
    cmd[4] = len;
    cmd[5] = 0x00;
    cmd[6] = 0x00;
    cmd[7] = 0x00;

    i = 0;
    while(len) {
        cmd[8 + i] = val[i];
        len--;
        i++;
    }

    ret = i + 7;

    if(ret < mfcStrLen)
        ret = mfcStrLen;
    if(ret % 2)
        ret++;

    return ret;
}

static int buildPrm(uint8 * cmd, unsigned short CmdId, char *val) {
    int ret = 0;
    byte len;

    switch (CmdId) {
      case MF_Detect:
          VERIFY(val);
          ret = AddPrm(&cmd[ret], PR_CardIdx, val, 1);
          break;
      case MF_Auth:
          VERIFY(val);
          ret = ret + AddPrm(&cmd[ret], PR_KeyType, &val[0], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Sector, &val[1], 1);
          ret = ret + AddPrm(&cmd[ret], PR_UncodeKey, &val[2], 6);
          break;
      case MF_Read:
          VERIFY(val);
          ret = ret + AddPrm(&cmd[ret], PR_DataFmt, &val[0], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Sector, &val[1], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Block, &val[2], 1);
          break;
      case MF_Write:
          VERIFY(val);
          len = val[0];
          ret = ret + AddPrm(&cmd[ret], PR_DataFmt, &val[0], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Sector, &val[1], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Block, &val[2], 1);
          if(len == 1)
              len = 4;
          else
              len = 16;
          ret = ret + AddPrm(&cmd[ret], PR_DataBlk, &val[3], len);
          break;
      case MF_Value:
          ret = ret + AddPrm(&cmd[ret], PR_OperType, &val[0], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Sector, &val[1], 1);
          ret = ret + AddPrm(&cmd[ret], PR_Block, &val[2], 1);
          ret = ret + AddPrm(&cmd[ret], PR_DstBlk, &val[3], 1);
          ret = ret + AddPrm(&cmd[ret], PR_DataBlk, &val[4], 4);
          break;
      default:                 //No parameters
          ret = 0;
          break;
    }
    return ret;
}

static int16 SendCmd(unsigned short cmdId, char *val) {
    int ret;
    byte cmd[320];
    pcdRxResult_t rsp;
    word len = 0;
    const byte mfcHdrLen = 8;
    const byte mfcStrLen = 12;

    memset(cmd, 0, sizeof(cmd));
    memcpy((void *) cmd, (void *) mfcCmd, (mfcHdrLen + mfcStrLen));

    cmd[12] = 0x02;
    cmd[16] = HBYTE(cmdId);
    cmd[17] = LBYTE(cmdId);

    ret = buildPrm(&cmd[20], cmdId, val);
    len = ret + mfcStrLen;
    cmd[4] = len % 0x0100;
    cmd[5] = len / 0x0100;
    len = len + mfcHdrLen;

    ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, 0x02);
    CHECK(ret >= 0, lblKO);

    switch (cmdId) {
      case MF_Read:
      case MF_Value:
          VERIFY(val);
          memcpy(val, &rsp.Data[rsp.Length - 25], 16);
          break;
      default:
          break;
    }

    ret = rsp.Data[rsp.Length - 1];
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Open the contactless card reader
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsStart(void) {
    int ret;

    ret = pcdCOMSetReader("COM1", 0x51);
    CHECK(ret >= 0, lblKO);

    ret = pcdOpen("COM1", &pcdHandle);
    CHECK(ret >= 0, lblKO);

    psyTimerWakeAfter(400);     //wait for switch on for contactless device (simu 300)

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Open the mifare card reader, but not wait the mifare card
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcStart(void) {
    int ret;

    ret = clsStart();
    CHECK(ret >= 0, lblKO);

    ret = SendCmd(MF_Open, 0);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Stop the contactless card reader and
 * Close the associated channel.
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
void clsStop(void) {

    if(pcdHandle) {
        pcdClose(pcdHandle);
        pcdCOMClose(pcdHandle);
    }
    pcdHandle = 0;
}

/** Stop the mifare card reader and
 * Close the associated channel.
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
void mfcStop(void) {

    if(pcdHandle) {
        SendCmd(MF_Close, 0);
        //pcdClose(pcdHandle);
        //pcdCOMClose(pcdHandle);
    }
    //pcdHandle = NULL;
    clsStop();
}

/** Detect mifare card
 * \param dly (I) timeout in seconds (not used in UNICAPT)
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcDetect(byte dly) {
    int ret;

    ret = SendCmd(MF_Detect, (char *) "\x00");
    CHECK(ret >= 0, lblKO);

    ret = SendCmd(MF_Activate, 0);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Authenticate mifare card
 * \param key (I) Key value (NULL means factory key)
 * \param AutTyp (I) Authentication type 0xKS, K - key A or B, S - sector to authenticate
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcAut(const byte * key, byte AutTyp) {
    int ret;
    byte sec;
    byte keyTyp;
    byte idx;
    char buf[32];
    static const byte *fackey = (byte *) "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

    if(!key)
        key = fackey;

    keyTyp = AutTyp & 0xF0;
    memset(buf, 0, sizeof(buf));
    if(keyTyp == 0xA0) {
        keyTyp = 0x60;
    } else
        keyTyp = 0x61;

    sec = AutTyp & 0x0F;
    buf[0] = keyTyp;
    buf[1] = sec;
    for (idx = 0; idx < 6; idx++) {
        buf[2 + idx] = key[idx];
    }

    ret = SendCmd(MF_Auth, buf);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Load data from mifare card
 * \param data (O) data value
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \param fmt (I) format of data, 'D' data 16 bytes, 'V' value 4 bytes
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcLoad(byte * data, byte loc, byte fmt) {
    int ret;
    byte blk;
    byte sec;

    //byte fmt;
    byte len, idx;
    char buf[32];

    memset(buf, 0, sizeof(buf));
    sec = (loc & 0xF0) >> 4;
    blk = loc & 0x0F;

    //fmt = 1;                    //value format
    switch (fmt) {
      case 'V':
          fmt = 1;              //value format
          break;
      case 'D':
      default:
          fmt = 2;              //data format
          break;
    }
    if((!sec) && (!blk)) {      //force format for manufacture data
        fmt = 2;                //block format
    }
    if(blk == 3)                //force format for keys
        fmt = 2;

    buf[0] = fmt;
    buf[1] = sec;
    buf[2] = blk;

    ret = SendCmd(MF_Read, buf);
    CHECK(ret == 0, lblKO);

    if(fmt == 2)
        len = 16;
    else
        len = 4;
    for (idx = 0; idx < len; idx++) {
        data[idx] = buf[(16 - len) + idx];
    }

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
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcSave(const byte * data, byte loc, byte fmt) {
    int ret;
    byte blk;
    byte sec;

    //byte fmt;
    char buf[32];
    byte idx, len;

    memset(buf, 0, sizeof(buf));
    sec = (loc & 0xF0) >> 4;
    blk = loc & 0x0F;

    //fmt = 1;                    //value format
    //len = 4;
    switch (fmt) {
      case 'V':
          fmt = 1;              //value format
          len = 4;
          break;
      case 'D':
      default:
          fmt = 2;              //data format
          len = 16;
          break;
    }
    if((!sec) && (!blk)) {      //manufacture data
        return 0;               //block read only
    }
    if(blk == 3) {              //force format for keys
        fmt = 2;
        len = 16;
    }

    buf[0] = fmt;
    buf[1] = sec;
    buf[2] = blk;
    for (idx = 0; idx < len; idx++) {
        buf[3 + idx] = data[idx];
    }

    ret = SendCmd(MF_Write, buf);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Value operation with mifare card
 * \param data (I) data value
 * \param loc (I) location of data type 0xSB, S - card sector and B - block of sector
 * \param cmd (I) Value command, increase or decrease
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
static int mfcValue(byte * data, byte loc, byte cmd) {
    int ret;
    byte blk;
    byte sec;
    byte idx;
    char buf[32];

    trcS("mfcValue Beg\n");

    memset(buf, 0, sizeof(buf));
    sec = (loc & 0xF0) >> 4;
    blk = loc & 0x0F;

    buf[0] = cmd;
    buf[1] = sec;
    buf[2] = blk;
    buf[3] = blk;               //destination block

    trcS("mfcValue IN_DATA : ( ");
    trcBAN(data, sizeof(data));
    trcS(")\n");

    for (idx = 0; idx < 4; idx++) {
        buf[4 + idx] = data[idx];
    }

    ret = SendCmd(MF_Value, buf);
    trcS("MF_Value CMD : ( ");
    trcBAN((byte *) buf, sizeof(buf));
    trcS(")\n");
    CHECK(ret == 0, lblKO);

    for (idx = 0; idx < 4; idx++) {
        data[idx] = buf[12 + idx];
    }

    trcS("mfcValue OUT_DATA : ( ");
    trcBAN(data, sizeof(data));
    trcS(")\n");

    ret = 1;
    goto lblEnd;
  lblKO:
    trcS("mfcValue ERROR\n");
    ret = -1;
  lblEnd:
    trcS("mfcValue End\n");
    return ret;
}

/** Debit mifare card
 * \param data
 *    - (I) Data value
* \param loc
 *    - (I) location of data type 0xSB, S - card sector and B - block of sector
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcDebit(const byte * data, byte loc) {
    int ret;

    if(!data)
        data = (byte *) "\x00\x00\x00\x01";
    trcS("mfcDebit: ");
    trcBAN(data, 4);
    trcS("\n");
    ret = mfcValue((byte *) data, loc, OperMF_Dec);
    return ret;
}

/** Credit mifare card
 * \param data
 *    - (I) Data value
* \param loc
 *    - (I) location of data type 0xSB, S - card sector and B - block of sector
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0159.c
 */
int mfcCredit(const byte * data, byte loc) {
    int ret;

    if(!data)
        data = (byte *) "\x00\x00\x00\x01";
    trcS("mfcCredit: ");
    trcBAN(data, 4);
    trcS("\n");

    ret = mfcValue((byte *) data, loc, OperMF_Inc);
    return ret;
}

/** Play bip sound in mifare reader device
 * \param dly (I) value, bip duration in milliseconds
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsBip(byte dly) {
    int ret;
    byte cmd[16];
    pcdRxResult_t rsp;
    int16 len = 0;

    memset(cmd, 0, sizeof(cmd));
    cmd[0] = CL_BUZ;
    cmd[2] = 0x06;
    cmd[4] = 0x02;
    cmd[5] = 0x0F;              //frequency devisor
    cmd[6] = 0x05;              //duration of activation
    cmd[8] = 0x80;              //period duraion
    len = 10;
    ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
    CHECK(ret >= 0, lblKO);
    psyTimerWakeAfter(dly);
    cmd[4] = 0x01;
    ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
    CHECK(ret >= 0, lblKO);

    ret = rsp.Data[rsp.Length - 1];
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** LED manipulation in mifare reader device
 * \param ctl (I) of data type 0xLT, L - LEDs to manupulate and T - manipulation type
 * \param color (I) not implemented for UNICAPT yet: TELIUM only (for vending pass)
 * L format : LED1=0x01 , LED2=0x02, LED3=0x04, LED4 = 0x08 , LED1+LED2 = 0x03
 * T format : 0 stop, F on, dly for blink
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsLED(byte ctl, byte color) {
    int ret;
    uint8 cmd[16];
    pcdRxResult_t rsp;
    int16 len = 0;
    byte led, type;

    memset(cmd, 0, sizeof(cmd));

    led = (ctl & 0xF0) >> 4;
    type = ctl & 0x0F;
    switch (type) {
      case 0x00:               //off
          type = 0x01;
          break;
      case 0x0F:               //on
          type = 0x02;
          break;
      default:                 //blink
          cmd[6] = type;        //blink on dly
          cmd[7] = type;        //blink off dly
          type = 0x03;
          break;
    }

    cmd[0] = CL_LED;
    cmd[2] = 0x04;
    cmd[4] = type;
    cmd[5] = led;
    len = 8;

    ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
    CHECK(ret >= 0, lblKO);

    ret = rsp.Data[rsp.Length - 1];
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Image display in mifare reader device
 * \param img (I), pointer to bitmap of image
 * \param ImgLen (I), size of bitmap
 * \param Lsize (I), horizontal size (lenght) of image
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsImg(const unsigned char *img, word ImgLen, word Lsize) {
    int ret;
    uint8 cmd[2048];
    pcdRxResult_t rsp;
    int16 len = 0, SaveLen = 0;
    int16 i;
    int16 ofs = 0, iter = 0;
    int16 skip;

    SaveLen = ImgLen;
    skip = 3;

    if(ImgLen > Lsize) {
        ofs = ImgLen - (Lsize + skip);
        ImgLen = Lsize;
    }

    while(32) {
        len = ImgLen;
        memset(cmd, 0, sizeof(cmd));

        cmd[0] = CL_SCN;
        cmd[1] = 0x00;          //frame number
        cmd[2] = len % 0x0100;  //length LByte
        cmd[3] = len / 0x0100;  //length HByte
        cmd[4] = iter;          //first line of frame
        cmd[5] = 0x00;          //first column of frame
        cmd[6] = 0x07;          //last line
        cmd[7] = 0x02;          //Nb of columns
        cmd[8] = 0x01;          //type, part of screen
        cmd[9] = 0x02;          //bitmap
        cmd[10] = 0x00;

        i = 0;
        while(len) {
            //cmd[12+i] = img[i+(SaveLen - ofs)-len];
            cmd[12 + i] = img[i + (SaveLen - ofs) + skip];
            len--;
            i++;
        }

        len = ImgLen + 12;

        //show
        ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
        CHECK(ret >= 0, lblKO);

        psyTimerWakeAfter(1);

        if(ofs <= skip)
            break;
        ImgLen = ofs;
        if(ofs) {
            skip = skip + 4;    //skip bitmap structure
            if(ofs > Lsize)
                len = Lsize;
            else
                len = ofs;
            ofs = ImgLen - len;
            ImgLen = len;
            iter++;
        }
    }

    ret = rsp.Data[rsp.Length - 1];
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;

}

/** Play melody in mifare reader device
 * \param melody (I) contain couple sound frequancy divisor and duration in milliseconds
 * \param MelodyLen (I) length of melody
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsSound(const byte * melody, word MelodyLen) {
    int ret;
    byte cmd[16];
    pcdRxResult_t rsp;
    int16 len = 0;
    byte dly;
    word i;

    for (i = 0; i < MelodyLen; i++) {
        memset(cmd, 0, sizeof(cmd));
        cmd[0] = CL_BUZ;
        cmd[2] = 0x06;
        cmd[4] = 0x02;
        cmd[5] = melody[i];     //frequency devisor
        i++;
        dly = melody[i];
        cmd[6] = dly;           //duration of activation
        cmd[8] = 0x80;          //period duration
        len = 10;
        ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
        CHECK(ret >= 0, lblKO);
        psyTimerWakeAfter(dly);
        cmd[4] = 0x01;
        ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
        CHECK(ret >= 0, lblKO);
    }

    ret = rsp.Data[rsp.Length - 1];
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static void ConversionMatrice(uint8 * img, uint8 len, uint8 * bufH,
                              uint8 * bufL) {
    uint8 idx, dstIdx;
    uint8 msk, coefL, coefH, res, buf;
    uint8 *src;
    uint8 tmpH, tmpL;

    msk = 0x80;
    for (dstIdx = 0; dstIdx < len; dstIdx++) {
        tmpH = tmpL = 0x00;
        src = img;
        coefH = coefL = 1;
        for (idx = 0; idx < len; idx++) {
            buf = src[idx] & msk;
            if(buf)
                res = 1;
            else
                res = 0;
            if(idx < len / 2) {
                tmpH = tmpH + (res * coefH);
                coefH = coefH * 2;
            } else {
                tmpL = tmpL + (res * coefL);
                coefL = coefL * 2;
            }
        }
        bufH[dstIdx] = tmpH;
        bufL[dstIdx] = tmpL;
        msk = msk >> 1;         //move mask bit down
    }
}

static int clsShow(uint8 * img, uint16 ImgLen, byte line) {
    int ret;
    uint8 cmd[2048];
    pcdRxResult_t rsp;
    int16 len = 0;
    int16 i;
    uint8 type;

    len = ImgLen;
    type = 0x01;                //partial screen
    if(!len) {
        ImgLen = 1;
        type = 0x00;            //clean all screen
    }

    memset(cmd, 0, sizeof(cmd));
    cmd[0] = CL_SCN;
    cmd[1] = 0x00;              //first frame
    cmd[2] = len % 0x0100;      //length LByte
    cmd[3] = len / 0x0100;      //length HByte
    cmd[4] = line;              //first line of screen
    cmd[5] = 0x00;              //first column of screen
    cmd[6] = 0x07;              //last line
    cmd[7] = 0x02;              //Nb of columns
    cmd[8] = type;              //type of frame
    cmd[9] = 0x02;              //bitmap
    cmd[10] = 0x00;

    for (i = 0; i < ImgLen; i++) {
        cmd[12 + i] = img[i];
    }

    len = ImgLen + 12;
    ret = pcdProcessApdu(pcdHandle, len, cmd, (void *) &rsp, TRUE);
    CHECK(ret >= 0, lblKO);

    ret = rsp.Data[rsp.Length - 1];
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Show text string on mifare reader screen
 * \param line (I) line to display string
 * \param text (I) sting to display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */
int clsText(byte line, char *text) {
    int ret;
    uint8 len, idx, i, tmpLen, fontWidth;
    hmiFontChar_t fontChar;
    uint8 GlyphData[5 * 16], tmp[16], tmpH[16], tmpL[16];
    uint8 LineBufH[16 * 16], LineBufL[16 * 16];

    len = strlen(text);
    if(len > 16)
        return -1;
    if(line > 7)
        return -1;

    memset(LineBufH, 0, sizeof(LineBufH));
    memset(LineBufL, 0, sizeof(LineBufL));

    fontWidth = 6;

    for (idx = 0; idx < len; idx++) {
        ret =
            hmiFontCharRead(HMI_INTERNAL_FONT_FIXED_6_8, text[idx], &fontChar,
                            GlyphData);
        i = 0;
        tmpLen = 16;
        while(tmpLen) {
            tmp[i] = GlyphData[tmpLen];
            tmpLen--;
            i++;
        }
        memset(tmpH, 0, sizeof(tmpH));
        memset(tmpL, 0, sizeof(tmpL));
        ConversionMatrice(GlyphData, 16, tmpH, tmpL);
        for (i = 0; i < fontWidth; i++) {
            LineBufH[idx * fontWidth + i] = tmpH[i];
            LineBufL[idx * fontWidth + i] = tmpL[i];
        }
    }
    ret = clsShow(LineBufH, 256, line);

    return 1;
}

/** Turns on/off the backlight of the contactless reader
 * \param level (I) - level of the light from 0 == OFF till 255 == FULLLIGHT
 * Not implemented yet for UNICAPT
 * \header sys\\sys.h
 * \test tcns0031.c
 */
void clsBacklightLevel(byte level) {
}

/** Clear mifare reader screen
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\ext.c
 * \test tcik0161.c
 */

int clsClearDsp(void) {
    char *ptr = " ";

    return clsText(0, ptr);
}

#ifdef __CLSMV__
static word trcClsMVDbg(pcdRxResult_t * res, tQueue * rsp) {
    card *tag, *len;
    word idx = 0x14;            // this is the first data byte of the response after the header
    byte pad;
    byte buf[256];

    queReset(rsp);
    while(idx < res->Length) {
        tag = (card *) & res->Data[idx];

        if(*tag != 0x000000) {
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "T=%02X%02X%02X%02X ", (word) res->Data[idx + 3],
                    (word) res->Data[idx + 2], (word) res->Data[idx + 1],
                    (word) res->Data[idx]);
            trcS(buf);
        }
        //move index along to length field
        idx = idx + 4;
        len = (card *) & res->Data[idx];

        // move index along to data field
        idx = idx + 4;

        switch (*tag) {
          case 0x9F8101:
              memcpy(svcStaCod, &res->Data[idx], *len);
              break;

          default:
              break;
        }

        if(*tag != 0x000000) {
            memset(buf, 0, sizeof(buf));
            sprintf(buf, " L=%3u V=", (word) * len);
            trcS(buf);
            trcBN(&res->Data[idx], *len);
            trcS("\n");

            quePut(rsp, (byte *) tag, sizeof(card));    //save tag into queue
            quePut(rsp, (byte *) & res->Data[idx], *len);   //save value into queue
        }
        //we must now move on to the next tag
        idx = idx + *len;
        pad = (4 - (idx % 4)) % 4;

        // assume we are not at the end of the buffer
        idx = idx + pad;

        //check to see if our assumption is correct
        if(idx >= res->Length) {
            // we are finished parsing
            break;
        }
    }

    return queLen(rsp);
}
#endif

int clsMVTransaction(int state, byte * cmd, word cmdLen, tQueue * rsp) {
#ifdef __CLSMV__
    int ret;
    word tag9F8101;
    byte cmdBuf[320];
    pcdRxResult_t res;
    word len = 0;
    const byte mfcHdrLen = 8;
    const byte mfcStrLen = 12;

    memset(cmdBuf, 0, sizeof(cmdBuf));
    memcpy((void *) cmdBuf, (void *) mfcCmd, (mfcHdrLen + mfcStrLen));

    switch (state) {
      case IDLE:
          trcS("Initialise Reader: \n");
          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          len = 18;
          break;

      case INITIALISED:
          trcS("Get Interface Satatus: \n");
          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x02;
          cmdBuf[17] = 0x19;
          len = 18;
          break;

      case INTERFACE_OPENED:
          trcS("Clear Txn Data: \n");
          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x09;
          len = 18;
          break;

      case TXN_READY:
          trcS("Preliminary Proc: \n");
          memcpy(&cmdBuf[20], cmd, cmdLen);
          len = cmdLen + 12;
          cmdBuf[4] = len % 0x0100;
          cmdBuf[5] = len / 0x0100;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x0B;
          len = len + 8;
          break;

      case PRELIMINARY_TXN_COMPLETE:
          trcS("Start Transaction: \n");
          memcpy(&cmdBuf[20], cmd, cmdLen);
          len = cmdLen + 12;
          cmdBuf[4] = len % 0x0100;
          cmdBuf[5] = len / 0x0100;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x06;
          len = len + 8;
          break;

      case ONLINE_PROC:
          trcS("Online Auth: \n");
          memcpy(&cmdBuf[20], cmd, cmdLen);
          len = cmdLen + 12;
          cmdBuf[4] = len % 0x0100;
          cmdBuf[5] = len / 0x0100;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x0C;
          len = len + 8;
          break;

      case COMPLETE_TXN:
          trcS("Completion: \n");
          memcpy(&cmdBuf[20], cmd, cmdLen);
          len = cmdLen + 12;
          cmdBuf[4] = len % 0x0100;
          cmdBuf[5] = len / 0x0100;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x07;
          len = len + 8;
          break;

      case WAIT_FOR_CARD_REMOVAL:
          trcS("Wait for Card Removed: \n");
          tag9F8101 = (svcStaCod[0] * 256 + svcStaCod[1]);
          switch (tag9F8101) {
            case 0x0001:       //TAG_TRANSACTION_ACCEPTED
                trcS("Trans Accepted\n");
                break;

            case 0x0002:       //TAG_TRANSACTION_DECLINED
                trcS("Trans Declined\n");
                break;

            default:
                trcS("Bad Result\n");
                break;
          }

          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x14;
          len = 18;
          break;

      case CLSCARD_REMOVED:
          trcS("Card Removed: \n");
          memcpy(&cmdBuf[20], cmd, cmdLen);
          len = cmdLen + 12;
          cmdBuf[4] = len;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x08;
          len = len + 8;
          break;

      case FINALISE_TXN:
          trcS("Clear Txn Data: \n");
          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x01;
          cmdBuf[17] = 0x09;
          len = 18;
          break;

      case ERROR:
          trcS("Cancel Txn: \n");
          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x02;
          cmdBuf[17] = 0x03;
          len = 18;
          break;

      case FINISHED:
          trcS("Txn Finished: \n");
          return FINISHED;
      default:
          break;
    }

    ret = pcdProcessApdu(pcdHandle, len, cmdBuf, (void *) &res, 0x02);
    trcFN("Status = 0x%X\n", ret);
    CHECK(ret >= 0, lblKO);

    memset(svcStaCod, 0, sizeof(svcStaCod));
    trcClsMVDbg(&res, rsp);

    switch (state) {
      case IDLE:
          ret = INITIALISED;
          break;

      case INITIALISED:
          if(res.Data[res.Length - 1] == 0) {   // interface is closed
              memset(cmdBuf, 0, sizeof(cmdBuf));
              memcpy((void *) cmdBuf, (void *) mfcCmd, (mfcHdrLen + mfcStrLen));
              cmdBuf[4] = 0x0A;
              cmdBuf[12] = 0x02;
              cmdBuf[16] = 0x02;
              cmdBuf[17] = 0x10;
              ret = pcdProcessApdu(pcdHandle, 18, cmdBuf, (void *) &res, 0x02);
              trcFN("Status = 0x%X\n", ret);

              if(ret < 0) {
                  ret = FINISHED;
              } else if(res.Data[res.Length - 1] == 0x01) {
                  ret = INTERFACE_OPENED;
              } else {
                  ret = FINISHED;
              }
          } else {
              ret = INTERFACE_OPENED;
          }
          CHECK(ret >= 0, lblKO);
          memset(svcStaCod, 0, sizeof(svcStaCod));
          trcClsMVDbg(&res, rsp);
          break;

      case INTERFACE_OPENED:
          ret = TXN_READY;
          break;

      case TXN_READY:
          tag9F8101 = (svcStaCod[0] * 256 + svcStaCod[1]);
          switch (tag9F8101) {
            case 0x0000:       //TAG_PROCESS_COMPLETED
                ret = PRELIMINARY_TXN_COMPLETE;
                break;

            case 0x0011:       //TAG_PROCESSING_ERROR
            case 0x0022:       //TAG_USE_SWIPE_OR_CHIP
                ret = FINISHED;
                break;

            case 0x0001:       //TAG_TRANSACTION_ACCEPTED
            case 0x0002:       //TAG_TRANSACTION_DECLINED
            default:
                ret = ERROR;
                break;
          }
          break;

      case PRELIMINARY_TXN_COMPLETE:
          tag9F8101 = (svcStaCod[0] * 256 + svcStaCod[1]);
          switch (tag9F8101) {
            case 0x03:         //TAG_PROCESS_COMPLETED
                ret = ONLINE_PROC;
                nextState = COMPLETE_TXN;
                break;

            default:
                ret = COMPLETE_TXN;
                break;
          }
          break;

      case ONLINE_PROC:
          ret = nextState;
          nextState = 0;
          break;

      case COMPLETE_TXN:
          tag9F8101 = (svcStaCod[0] * 256 + svcStaCod[1]);
          switch (tag9F8101) {
            case 0x03:         //TAG_PROCESS_COMPLETED
                ret = ONLINE_PROC;
                nextState = WAIT_FOR_CARD_REMOVAL;
                break;

            default:
                ret = WAIT_FOR_CARD_REMOVAL;
                break;
          }
          break;

      case WAIT_FOR_CARD_REMOVAL:
          ret = CLSCARD_REMOVED;
          break;

      case CLSCARD_REMOVED:
          ret = FINALISE_TXN;
          break;

      case FINALISE_TXN:
          memset(cmdBuf, 0, sizeof(cmdBuf));
          memcpy((void *) cmdBuf, (void *) mfcCmd, (mfcHdrLen + mfcStrLen));
          cmdBuf[4] = 0x0A;
          cmdBuf[12] = 0x02;
          cmdBuf[16] = 0x02;
          cmdBuf[17] = 0x01;
          ret = pcdProcessApdu(pcdHandle, 18, cmdBuf, (void *) &res, 0x02);
          trcClsMVDbg(&res, rsp);
          cmdBuf[16] = 0x02;
          cmdBuf[17] = 0x11;
          ret = pcdProcessApdu(pcdHandle, 18, cmdBuf, (void *) &res, 0x02);
          trcClsMVDbg(&res, rsp);
          ret = FINISHED;
          break;

      case ERROR:
          ret = FINALISE_TXN;
          break;

      default:
          break;
    }

    goto lblEnd;
  lblKO:
    ret = ERROR;
  lblEnd:
    return ret;
#else
    return -1;
#endif
}

#endif                          //def __CLS__
