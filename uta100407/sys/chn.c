/** \file
 * Communication channels implementation using native UNICAPT
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/chn.c $
 *
 * $Id: chn.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include "sys.h"

#ifdef __CHN_PPP__
#define __CHN_TCP__
#endif

#ifdef __CHN_HDLC__
#define __CHN_COM__
#endif

#ifdef __CHN_GPRS__
#include <gsm.h>
#define __CHN_TCP__
#endif

#ifdef __CHN_FTP__
#include <LNetFtp.h>
#define __CHN_TCP__
#endif

#ifdef __CHN_SSL__
#include <LNetSSL.h>
#define __CHN_TCP__
#endif

#ifdef __CHN_TCP__
#include <LNet.h>
#include <LNetSocket.h>
#include <LNetTcpip.h>
#endif

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcCHN))

#define RSP_LEN 1024
#define GSM_PLEASE_WAIT 515     // Please wait, init or command processing in progress
#define GPRS_ERROR 148          // Unspecified GPRS error, may happen during attachment

typedef struct {
    int16 sta;
    uint16 len;
    char buf[RSP_LEN];
} tResponse;

typedef struct sComChn {
    byte chn;                   // communication channel used
    byte chnRole;
    union {
#ifdef __CHN_COM__
        struct {                //chn= chnMdm or chnHdlc or chnCom
            uint32 hdl;
        } mdm;
#endif
#ifdef __CHN_TCP__
        struct {                //chn= chnTcp...
            netNi_t hdl;
            int16 sck;          //it is also ppp.sck
            byte idx;
            int16 srvSck;
            int16 cltSck;
        } tcp;
#endif
#ifdef __CHN_PPP__
        struct {                //chn= chnPpp
            netNi_t hdl;
            int16 sck;          //it is also tcp.sck
            byte idx;
        } ppp;
#endif
#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs
            netNi_t hdlNet;
            int16 sck;          //it is also tcp.sck
            uint32 hdlGsm;
        } gprs;
#endif
#ifdef __CHN_USB__
        struct {                //chn= chnUsb
            uint32 hdl;
            uint32 endpoint;
            uint32 sendblock;
            uint32 recvblock;
        } usb;
#endif
#ifdef __CHN_SSL__
        struct {                //chn= chnSsl
            netNi_t hdl;
            byte idx;
            int16 sck;
            uint32 periph;
            uint32 context;
            uint32 session;
            char certKey;
            char certCA[4096];
        } ssl;
#endif
        byte a;                 //if all is undefined, the union cannot be empty (compiler complainte)
    } prm;
} tComChn;
static tComChn com;

#ifndef __CMM__

static int comStartMdm(void) {
#ifdef __CHN_COM__
    int ret;

#ifdef __COUNTRY__
    enum comT35_t cnt;

    cnt = __COUNTRY__;
    ret = comCfgAccess(COM_WRITE_MODE, COM_COUNTRY_CODE, sizeof(cnt), &cnt);
#endif
    trcS("comStartMdm Beg\n");
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);

    ret = comOpen("MODEM", &com.prm.mdm.hdl);   //open the channel
    CHECK(ret == RET_OK, lblKO);    //if ret=-4 under simelite check Simulation|Com ports mapping
    ret = 1;
    goto lblEnd;
  lblKO:
    comClose(com.prm.mdm.hdl);
    trcFN("comOpen ret=%d\n", ret);
    ret = -1;
  lblEnd:
    trcFN("comStartMdm ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comStartCom(void) {
#ifdef __CHN_COM__
    int ret;
    char *ptr;

    trcS("comStartCom Beg\n");
    ret = 0;
    ptr = 0;
    switch (com.chn) {
      case chnCom1:
          ptr = "COM1";
          break;
      case chnCom2:
          ptr = "COM2";
          break;
      case chnCom3:
          ptr = "COM3";
          break;
      default:
          break;
    }
    VERIFY(ptr);
    ret = comOpen(ptr, &com.prm.mdm.hdl);
    CHECK(ret == RET_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    comClose(com.prm.mdm.hdl);
    ret = -1;
  lblEnd:
    trcFN("comStartCom ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

#ifdef __CHN_TCP__
static void changeRadCtl(void) {
    uint32 hdl;
    uint32 val;

    val = RAD_MODE_RADIO;
    radOpen("DEFAULT", &hdl);
    radControl(hdl, RAD_SET_MODE, RAD_SET_MODE_SIZE, &val);
    radClose(hdl);
}
#endif

static int comStartTcp(void) {
#ifdef __CHN_TCP__
#ifdef WIN32
    return 1;
#else
    int ret;
    int idx;
    netChannelList_t chn[NET_CHANNEL_MAX];
    const char *ptr;

    trcS("comStartTcp Beg\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 ||
           com.chn == chnSsl5100 || com.chn == chnSsl7780);

    ptr = 0;
    switch (com.chn) {
      case chnTcp5100:
      case chnSsl5100:
          ptr = "CRYS0";
          break;
      case chnTcp7780:
      case chnSsl7780:
          ptr = "ETH0";
          changeRadCtl();
          break;
      default:
          break;
    }
    VERIFY(ptr);

    ret = netNiOpen(NET_NI_ETH, &com.prm.tcp.hdl);
    CHECK(ret == RET_OK, lblEnd);

    ret = netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE, (void *) chn);
    CHECK(ret == RET_OK, lblKO);

    ret = -1;
    for (idx = 0; chn[idx].name[0] != 0; idx++) {
        if(memcmp(chn[idx].name, ptr, strlen(ptr)) != 0)
            continue;
        com.prm.tcp.idx = idx;
        ret = RET_OK;
        break;
    }
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("Channel Error %d", socketerror());
    netNiClose(com.prm.tcp.hdl);
    ret = -1;
  lblEnd:
    trcFN("comStartTcp ret=%d\n", ret);
    return ret;
#endif
#else
    return -1;
#endif
}

static int comStartPpp(void) {
#ifdef __CHN_PPP__
    int ret;
    byte idx;
    netChannelList_t chn[NET_CHANNEL_MAX];
    comModemParameter_t mPrm;
    comModemParams_t cPrm;
    comATGRet_t rsp;

#ifdef __COUNTRY__
    enum comT35_t cnt;

    cnt = __COUNTRY__;
    ret = comCfgAccess(COM_WRITE_MODE, COM_COUNTRY_CODE, sizeof(cnt), &cnt);
#endif

    trcS("comStartPpp Beg\n");
    VERIFY(com.chn == chnPpp);

    ret = comOpen("MODEM", &com.prm.ppp.hdl);   //open the channel
    CHECK(ret == RET_OK, lblKO);

    mPrm.blindDialingMode = COM_FIELD_NOT_USED;
    mPrm.lineInUseMode = COM_FIELD_NOT_USED;
    mPrm.dialMode = COM_DIAL_TONE;
    mPrm.errorCorrectionType = COM_CORRECTION_NONE;
    mPrm.dataCompressionType = COM_COMPRESSION_NONE;
    mPrm.modulation.modulationType = COM_MODEM_MODULATION_DEFAULT;
    mPrm.fastConnectMode = COM_FIELD_NOT_USED;
    mPrm.v80Mode = COM_FIELD_NOT_USED;
    mPrm.DCDMode = COM_FIELD_USED;
    mPrm.callMode = COM_CALL_AUTOMATIC;
    mPrm.commandMode = COM_COMMAND_ESCAPE;
    mPrm.flowMode = COM_MODEM_FLOW_DEFAULT;
    mPrm.hangUpMode = COM_HANG_UP_DTR;
    mPrm.ringMode.ringMode = COM_RING_NO_RING;
    mPrm.ringMode.ringNumber = 0;

    cPrm.type = COM_MODEM_PARAMS_TYPE_STD;
    cPrm.level = COM_MODEM_PARAMS_LEVEL_1;
    cPrm.param.modemParams = &mPrm;

    ret = comSetModemParameters(com.prm.ppp.hdl, &cPrm, &rsp);
    CHECK(ret == RET_OK, lblKO);

    ret = comClose(com.prm.ppp.hdl);
    CHECK(ret == RET_OK, lblKO);

    ret = netNiOpen(NET_NI_PPP, &com.prm.ppp.hdl);
    CHECK(ret == RET_OK, lblKO);

    ret = netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE, (void *) chn);
    CHECK(ret == RET_OK, lblKO);

    com.prm.ppp.idx = 0xFF;
    for (idx = 0; chn[idx].name[0] != 0; idx++) {
        trcFN("idx=%d\n", idx);
        trcFS("chn[idx].name=%s\n", chn[idx].name);
        if(memcmp(chn[idx].name, "MODEM", 5) != 0)
            continue;
        com.prm.ppp.idx = idx;
        break;
    }
    CHECK(com.prm.ppp.idx != 0xFF, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    netNiClose(com.prm.ppp.hdl);
    ret = -1;
  lblEnd:
    trcFN("comStartPpp ret=%d\n", ret);
    return ret;

#else
    return -1;
#endif
}

static int comStartUSB(void) {
#ifdef __CHN_USB__
#ifdef WIN32
    return -1;                  //not implemented under simelite
#else
    int ret = 1;
    comInit_t comInit;
    comUsbInit_t comUsbInit;
    comProtocolCfg_t comProtocol;
    comUartProtocolCfg_t comUartProtocol;
    comDeviceTypeIng_t usbInit;
    comConnect_t stateConnect;
    comUsbCfgAcces_t conf;
    int i;

    trcS("comStartUSB\n");
    VERIFY(com.chn == chnUSB);

    if(com.prm.usb.hdl)
        goto lblEnd;

    // Connect to USB driver
    ret = comOpen("USB_DVING", &com.prm.usb.hdl);
    CHECK(ret == RET_OK, lblKO);

    comInit.type = COM_INIT_TYPE_USB;
    comInit.level = COM_INIT_LEVEL_1;

    comInit.init.UsbInit = &comUsbInit;
    comUsbInit.usbInitType = COM_USB_DEVICE_ING;
    comUsbInit.req.PtDeviceType = &usbInit;

    //  usbInit.ClassId=COM_USB_DEVICE_CLASS_NOT_DEFINED;
    // for Hid interface
    usbInit.ClassId = COM_USB_DEVICE_CLASS_HID;

    usbInit.SubClass = 0;
    usbInit.BcdDevice = 0x0100;
    usbInit.RxbufferSize = 413;
    usbInit.TxbufferSize = 413;

    comProtocol.type = COM_PROT_CFG_TYPE_UART;
    comProtocol.level = COM_PROT_CFG_LEVEL_1;
    comProtocol.protCfg.uartCfg = &comUartProtocol;

    ret = comChanInit(com.prm.usb.hdl,
                      COM_MODIF_PARAM_TEMPORARY, &comInit,
                      COM_SET_PROTOCOL_TEMPORARY, &comProtocol);
    CHECK(ret == RET_OK, lblKO);

    ret = comConnectReq(com.prm.usb.hdl, &stateConnect);
    CHECK(ret == RET_RUNNING, lblKO);

    // now we are connected let's see the real send and receive blocksizes
    ret = comCfgAccess(COM_READ_MODE, COM_USB_DEVICE_INFO, sizeof(conf), &conf);
    CHECK(ret == RET_OK, lblKO)

        com.prm.usb.endpoint = conf.Info[0].Define.Interface.NumEndpoints;

    for (i = 0; i < 10; i++) {
        if(conf.Info[i].Type == USB_DEVICE_END_POINT) {
            // check if it's interrupt endpoint
            if(conf.Info[i].Define.EndPoint.Attributes == INTERRUPT_ENDPOINT) {
                // check if it's output or input
                if(conf.Info[i].Define.EndPoint.In_or_Out == USB_DEVICE_IN) {
                    // size for receiving
                    com.prm.usb.sendblock =
                        conf.Info[i].Define.EndPoint.wMaxPacketSize;
                } else if(conf.Info[i].Define.EndPoint.In_or_Out ==
                          USB_DEVICE_OUT) {
                    // size for receiving
                    com.prm.usb.recvblock =
                        conf.Info[i].Define.EndPoint.wMaxPacketSize;
                }
            }
        }
    }                           // for

    CHECK(com.prm.usb.sendblock != 0, lblKO);
    CHECK(com.prm.usb.recvblock != 0, lblKO);

    ret = 1;
    goto lblEnd;

  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comStartUSB ret=%d\n", ret);
    return ret;
#endif
#else
    return -1;
#endif
}

#ifdef __CHN_GPRS__
static int setGsmOn(uint32 hdl) {
    int ret;
    uint8 sta;

    sta = 0;
    ret = gsmOnOff(hdl, GET, &sta); //Check if the GSM Device is turn on.
    CHECK(ret == RET_OK, lblKO);
    trcFN("setGsmOn status= %d\n", sta);
    if(sta == 1)
        return 0;               //return zero if already set
    sta = 1;
    ret = gsmOnOff(hdl, SET, &sta); //Turn on the GSM Device.
    CHECK(ret == RET_OK, lblKO);
    return sta;                 //return 1 if it is really set
  lblKO:
    return -1;
}
#endif

static int comStartGprs(void) {
#ifdef __CHN_GPRS__
    int ret;

    trcS("comStartGprs Beg\n");
    VERIFY(com.chn == chnGprs);

    // Open the GSM Interface Library.
    ret = gsmOpen(&com.prm.gprs.hdlGsm, "MODEM3");
    CHECK(ret == RET_OK, lblKO);

    ret = setGsmOn(com.prm.gprs.hdlGsm);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcFN("comStartGprs error = %d\n", ret);
    gsmClose(com.prm.gprs.hdlGsm);
    com.prm.gprs.hdlGsm = 0;
    ret = -1;

  lblEnd:
    trcFN("comStartGprs ret = %d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comStartSsl(void) {
#ifdef __CHN_SSL__
#ifdef WIN32
    return 1;
#else
    int ret;

    ret = comStartTcp();
    CHECK(ret >= 0, lblKO);

    com.prm.ssl.hdl = 0;
    com.prm.ssl.periph = 0;

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("Channel Error %d", socketerror());
    ret = -1;
  lblEnd:
    trcFN("comStartSsl ret=%d\n", ret);
    return ret;
#endif
#else
    return -1;
#endif
}

/** Open the associated communication channel.
 * \param chn (I) Channel type from enum eChn.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
*/
int comStart(byte chn) {
    trcS("comStart\n");
    com.chn = chn;
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
          return comStartMdm(); //Internal modem
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return comStartCom();
      case chnTcp5100:
      case chnTcp7780:
          return comStartTcp();
      case chnPpp:
          return comStartPpp(); //PPP
      case chnGprs:
          return comStartGprs();    //GPRS
      case chnUSB:
          return comStartUSB(); // USB
      case chnSsl5100:
      case chnSsl7780:
          return comStartSsl();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSetMdm(const char *init) {
#ifdef __CHN_COM__
    int ret;
    enum comModemModulation_t modulationType;   //V22,V22B,V32,V32B,...
    enum comDataSize_t datasize;    //data bits: 7 or 8
    enum comParity_t parity;    //odd,even,none
    enum comStopBits_t stopbits;    //1,2
    enum comDialMode_t dialmode;    //pulse,tone
    enum comBps_t bps;
    enum comFieldStatus_t blindDial= 0;
    comATModemInit_t mIni;
    comInit_t cIni;
    comModemParameter_t mPrm;
    comModemParams_t cPrm;
    comATGRet_t rsp;
    byte fast;
    byte compression;
    byte correction;
    byte hangUpDtr;

    trcFS("comSet %s\n", init);
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);
    if(com.chn == chnHdlc)
        init = "8N11200";
    VERIFY(init);

    //parse control string
    //optional compression field:
    if(*init == '.') {
        compression = 1;
        init++;
    } else
        compression = 0;
    //optional correction field:
    if(*init == '?') {
        correction = 1;
        init++;
    } else
        correction = 0;
    //optional hang up field:
    if(*init == '#') {
        hangUpDtr = 1;
        init++;
    } else
        hangUpDtr = 0;

    switch (*init) {            //lowercase means baud rate negotiation
      case 'p':
      case 't':
          fast = 0;
          break;
      case 'P':
      case 'T':
          fast = 1;
          break;
      default:
          fast = 0;
          break;
    }
    switch (*init) {
      case 'T':
      case 't':
          dialmode = COM_DIAL_TONE;
          init++;
          break;
      case 'P':
      case 'p':
          dialmode = COM_DIAL_PULSE;
          init++;
          break;
      default:
          dialmode = COM_DIAL_DEFAULT;
          break;                //if missing, don't increment control string
    }

    datasize = COM_DATASIZE_8;
    switch (*init++) {          //number of data bits
      case '7':
          datasize = COM_DATASIZE_7;
          break;
      case '8':
          datasize = COM_DATASIZE_8;
          break;
      default:
          CHECK(!init, lblKO);
    }
    parity = COM_PARITY_NONE;
    switch (*init++) {          //parity
      case 'N':
          parity = COM_PARITY_NONE;
          break;
      case 'E':
          parity = COM_PARITY_EVEN;
          break;
      case 'O':
          parity = COM_PARITY_ODD;
          break;
      default:
          CHECK(!init, lblKO);
    }
    stopbits = COM_STOP_NO_CHANGE;
    switch (*init++) {          //number of stop bits
      case '0':
          stopbits = COM_STOP_NO_CHANGE;
          break;
      case '1':
          stopbits = COM_STOP_1;
          break;
      case '2':
          stopbits = COM_STOP_2;
          break;
      default:
          CHECK(!init, lblKO);
    }

    modulationType = COM_MODEM_MODULATION_DEFAULT;
    bps = COM_BPS_DEFAULT;
    if(memcmp(init, "300", 3) == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ? COM_MODEM_MODULATION_V21_REQ :
                                         COM_MODEM_MODULATION_V21);
        bps = (enum comBps_t) (fast ? COM_BPS_300 : COM_BPS_DEFAULT);
        init += 3;
    } else if(memcmp(init, "1200", 4) == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ? COM_MODEM_MODULATION_V22_REQ :
                                         COM_MODEM_MODULATION_V22);
        bps = (enum comBps_t) (fast ? COM_BPS_1200 : COM_BPS_DEFAULT);
        init += 4;
    } else if(memcmp(init, "2400", 4) == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ? COM_MODEM_MODULATION_V22B_REQ :
                                         COM_MODEM_MODULATION_V22B);
        bps = (enum comBps_t) (fast ? COM_BPS_2400 : COM_BPS_DEFAULT);
        init += 4;
    } else if(memcmp(init, "9600", 4) == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ? COM_MODEM_MODULATION_V32_REQ :
                                         COM_MODEM_MODULATION_V32);
        bps = (enum comBps_t) (fast ? COM_BPS_9600 : COM_BPS_DEFAULT);
        init += 4;
    } else if(memcmp(init, "19200", 5) == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ? COM_MODEM_MODULATION_V32B_REQ :
                                         COM_MODEM_MODULATION_V32B);
        bps = (enum comBps_t) (fast ? COM_BPS_19200 : COM_BPS_DEFAULT);
        init += 5;
    } else if(memcmp(init, "28000", 5) == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ? COM_MODEM_MODULATION_V34_REQ :
                                         COM_MODEM_MODULATION_V34);
        bps = (enum comBps_t) (fast ? COM_BPS_28000 : COM_BPS_DEFAULT);
        init += 5;
    } else
        //CHECK(init[0] == 0, lblKO); //removed (added possible additional init string for telium)

        blindDial =
            (enum comFieldStatus_t) (*init ==
                                     'D' ? COM_FIELD_NOT_USED : COM_FIELD_USED);
    //blindDial = COM_FIELD_NOT_USED;

    //prepare modem parameters: baud rate
    memset(&mPrm, 0, sizeof(mPrm));
    memset(&cPrm, 0, sizeof(cPrm));

    mPrm.modulation.minBpsTx = COM_BPS_DEFAULT;
    mPrm.modulation.maxBpsTx = COM_BPS_DEFAULT;
    mPrm.callMode = COM_CALL_MANUAL;
    mPrm.ringMode.ringMode = COM_RING_DEFAULT;
    mPrm.ringMode.ringNumber = 0;
    if(com.chn == chnHdlc) {
        mPrm.modulation.modulationType = COM_MODEM_MODULATION_V22_REQ;
        mPrm.modulation.minBps = COM_BPS_1200;
        mPrm.modulation.maxBps = COM_BPS_1200;
        mPrm.errorCorrectionType = COM_CORRECTION_NONE;
        mPrm.dataCompressionType = COM_COMPRESSION_NONE;
        mPrm.commandMode = COM_COMMAND_ESCAPE_REQ;
        mPrm.hangUpMode = COM_HANG_UP_DTR_REQ;
        mPrm.flowMode = COM_MODEM_FLOW_DEFAULT; //COM_MODEM_FLOW_NONE_REQ;
        mPrm.DCDMode = COM_FIELD_NOT_USED;
        mPrm.lineInUseMode = COM_FIELD_NOT_USED;
        mPrm.blindDialingMode = blindDial;
        mPrm.v80Mode = COM_FIELD_USED;
        mPrm.fastConnectMode = COM_FIELD_USED;
        mPrm.dialMode = COM_DIAL_TONE;
    } else {
        VERIFY(com.chn == chnMdm);
        mPrm.modulation.modulationType = modulationType;
        mPrm.modulation.minBps = bps;
        mPrm.modulation.maxBps = bps;
        mPrm.errorCorrectionType =
            (enum comErrorCorrection_t) (correction ? COM_CORRECTION_MNP :
                                         COM_CORRECTION_NONE);
        mPrm.dataCompressionType =
            (enum comDataCompression_t) (compression ? COM_COMPRESSION_MNP5 :
                                         COM_COMPRESSION_NONE);
        mPrm.commandMode =
            (enum comActivateCommand_t) (hangUpDtr ? COM_HANG_UP_DTR :
                                         COM_COMMAND_DEFAULT);
        mPrm.hangUpMode = COM_HANG_UP_DTR;
        mPrm.flowMode = COM_MODEM_FLOW_DEFAULT;
        mPrm.DCDMode = COM_FIELD_DEFAULT;
        mPrm.lineInUseMode = COM_FIELD_NOT_USED;
        mPrm.blindDialingMode = blindDial;
        mPrm.v80Mode = (enum comFieldStatus_t) COM_FIELD_NOT_USED;
        mPrm.fastConnectMode =
            (enum comFieldStatus_t) (fast ? COM_FIELD_USED : COM_FIELD_DEFAULT);
        mPrm.dialMode = dialmode;
    }

    cPrm.type = COM_MODEM_PARAMS_TYPE_STD;
    cPrm.level = COM_MODEM_PARAMS_LEVEL_1;
    cPrm.param.modemParams = &mPrm;

    ret = comSetModemParameters(com.prm.mdm.hdl, &cPrm, &rsp);
    CHECK(ret == RET_OK, lblKO);

    //prepare port parameters: datasize,parity,stopbits
    memset(&mIni, 0, sizeof(mIni));
    memset(&cIni, 0, sizeof(cIni));

    mIni.uartInit.speed = COM_BAUD_19200;
    mIni.uartInit.parity = parity;
    mIni.uartInit.dataSize = datasize;
    mIni.uartInit.stopBits = stopbits;
    mIni.uartInit.rBufferSize = 0;
    mIni.uartInit.sBufferSize = 0;
    mIni.modemModel = COM_MODEM_NO_CHANGE;
    mIni.defPABX = 0;
    mIni.defNumber = 0;
    mIni.modemParameters = 0;

    cIni.type = COM_INIT_TYPE_MODEM;
    cIni.level = COM_INIT_LEVEL_1;
    cIni.init.ATModemInit = &mIni;

    ret =
        comSetPortParameters(com.prm.mdm.hdl, COM_MODIF_PARAM_TEMPORARY, &cIni);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    comClose(com.prm.mdm.hdl);
    ret = -1;
  lblEnd:
    trcFN("comSetMdm ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

#ifdef __CHN_TCP__
#ifndef __TEST__
static
#endif
/** Parse a string for extracting a part of it
 * \param *dst (O) Pointer for destination. dim characters are reserved at dst
 * \param *src (I) Pointer to the source string to parse.
 * \param  dim (I) size of the destination
 * \return a pointer to the string following the extracted part of it
 */
const char *parseStr(char *dst, const char *src, int dim) {
    VERIFY(dst);
    VERIFY(src);

    while(*src) {
        if(*src == '|') {
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
#endif

#ifdef __CHN_TCP__
#ifndef __TEST__
static
#endif
/** Parse a string for extracting an IP address
 * \remark the IP address is converted from string to numerical value into the 4 byte destination
 * \param adr (O) Pointer for destination. 4 characters are reserved at adr
 * \param str (I) Pointer to the source string to parse.
 * \return a pointer to the string following the extracted address,
 * \source sys\\chn.c
 * \test tcik0045.c
*/
const char *parseIp(char *adr, const char *str) {
    int idx;

    VERIFY(str);
    VERIFY(adr);
    memset(adr, 0, 4);

    for (idx = 0; *str != '\0' && *str != '|'; str++) {
        if(*str == '.')         // next value will follow
        {
            idx++;
        } else {                // extracting value
            adr[idx] *= 10;
            adr[idx] += *str - '0';
        }
    }

    if(*str == '|') {
        str++;                  // skip separator
    }

    return str;
}
#endif

#ifdef __CHN_GPRS__
static int comGsmPin(uint32 hdl, const char *pin) {
    int ret;
    uint8 pinstat, nbpin1, nbpin2, nbpuk1, nbpuk2;

    if(*pin == 0)
        return 0;               //no pin needed
    ret = gsmGetPinStatus(hdl, &pinstat, &nbpin1, &nbpin2, &nbpuk1, &nbpuk2);
    if(ret != RET_OK)
        return -1;
    if(pinstat != 1)
        return 0;
    ret = gsmEnterPin(hdl, "", (char *) pin);
    trcFN("gsmEnterPin ret= %d\n", ret);
    if(ret != RET_OK)
        return -1;
    return 1;
}

/** Get IMSI of GSM SIM carte.
 * \remark
 *  - To use only if terminal in idle or attached state
 *  - Return error if terminal is connected to any server
 * \param *pImsi (O) Pointer for IMSI value. 15+1 characters should be reserved at pImsi
 * \param *pPin  (I) Pointer for PIN value. If not required can be zero pointer
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcik0051.c
*/
int comGetIMSI(char *pImsi, const char *pPin) {
    int ret;
    uint16 err;
    uint32 gsmHdl;

    trcS("comGetIMSI ");
    VERIFY(!com.chn);
    memset(pImsi, 0, 15 + 1);

#ifdef __SIM__
    trcS("simulated\n");
    gsmHdl = 0;
    strcpy(pImsi, "208019300312118");
#else
    trcS("real\n");
    // Open the GSM Interface Library.
    ret = gsmOpen(&gsmHdl, "MODEM3");
    CHECK(ret == RET_OK, lblKO);

    ret = setGsmOn(gsmHdl);
    CHECK(ret >= 0, lblKO);

    ret = comGsmPin(gsmHdl, pPin);
    CHECK(ret >= 0, lblKO);

    ret = gsmGetImsi(gsmHdl, pImsi);
    CHECK(ret == RET_OK, lblKO);
    VERIFY(strlen(pImsi) <= 15);
#endif

    ret = strlen(pImsi);
    goto lblEnd;
  lblKO:
    gsmReadError(gsmHdl, &err);
    trcFN("comGetIMSI error = %d\n", err);
    ret = -1;

  lblEnd:
    trcFN("comGetIMSI ret = %d\n", ret);
    gsmClose(gsmHdl);
    return ret;
}

/** Get IMEI of GSM modem.
 * \remark
 *  - Return error if terminal is connected to any server
 * \param *pImei (O) Pointer for IMEI value. 15+1 characters should be reserved at pImei
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcik0051.c
*/
int comGetIMEI(char *pImei) {
    int ret;
    uint32 gsmHdl;
    uint16 err;

    trcS("comGetIMEI ");
    VERIFY(pImei);
    VERIFY(!com.chn);

#ifdef __SIM__
    trcS("simulated\n");
    gsmHdl = 0;
    strcpy(pImei, "352826003014687");
#else
    trcS("real\n");
    // Open the GSM Interface Library.
    ret = gsmOpen(&gsmHdl, "MODEM3");
    CHECK(ret == RET_OK, lblKO);

    memset(pImei, 0, 15 + 1);
    ret = gsmGetImei(gsmHdl, pImei);
    CHECK(ret == RET_OK, lblKO);
#endif

    ret = strlen(pImei);
    goto lblEnd;

  lblKO:
    gsmReadError(gsmHdl, &err);
    trcFN("comGetIMEI error = %d\n", err);
    ret = -1;

  lblEnd:
    trcFN("comGetIMEI ret = %d\n", ret);
    gsmClose(gsmHdl);
    return ret;
}

static int comGprsSetAttachStatus(byte idx) {
    int ret;
    uint16 err;
    uint8 sta;

    sta = 0;
    ret = gsmGprsAttachStatus(com.prm.gprs.hdlGsm, GET, &sta);
    trcFN("gsmGprsAttachStatus GET ret = %d\n", ret);
    trcFN("gsmGprsAttachStatus GET sta = %d\n", sta);
    if(ret == RET_OK) {
        if(sta == 1)            //already attached: nothing to do
            return idx;         //return number of not used trials
    }

    sta = 1;
    ret = gsmGprsAttachStatus(com.prm.gprs.hdlGsm, SET, &sta);  //set it to ON
    trcFN("gsmGprsAttachStatus SET ret = %d\n", ret);
    trcFN("gsmGprsAttachStatus SET sta = %d\n", sta);
    trcFN("gsmGprsAttachStatus SET idx = %d\n", idx);
    sta = 0;
    while(idx--) {
        ret = gsmGprsAttachStatus(com.prm.gprs.hdlGsm, GET, &sta);
        trcFN("gsmGprsAttachStatus GET ret = %d\n", ret);
        trcFN("gsmGprsAttachStatus GET idx = %d\n", idx);
        switch (ret) {
          case GSM_OK:
              ret = idx;        //return number of not used trials
              break;
          case GSM_CME_ERROR:
          case GSM_ERROR:
              err = 0;          //get more information
              gsmReadError(com.prm.gprs.hdlGsm, &err);
              trcFN("gsmGprsAttachStatus readError = %d\n", err);
              switch (err) {
                case GSM_PLEASE_WAIT:
                case GPRS_ERROR:
                    sta = 0;    //reset status
                    ret = 0;    //reset return code to retry
                    break;      //try once more
                default:       //real pbm, stop retrials
                    ret = -1;
                    break;
              }
          default:
              ret = -1;
              break;
        }
        CHECK(ret >= 0, lblKO); //must be commented, debug IK        
        //if((sta == 1)&&(ret>=0)) //debug IK
        if(sta == 1)            //must be commented, debug IK
            break;
        psyTimerWakeAfter(SYS_TIME_SECOND);
    }
    CHECK(sta == 1, lblKO);
    //CHECK(ret >= 0, lblKO);//debug IK
    return idx;
  lblKO:
    return -1;
}

static int comSetGprsStatus(const char *APN, byte idx) {
    int ret;
    gsmGprsContext_t ctx;
    gsmGprsProfile_t prf;
    uint8 net;
    const int actcid = 1;
    uint16 err;

    trcFS("comSetGprsStatus APN = %s\n", APN);
    VERIFY(APN);
    VERIFY(strlen(APN) < sizeof(ctx.APN));

    ret = gsmGprsGetNetworkStatus(com.prm.gprs.hdlGsm, &net);
    trcFN("gsmGprsGetNetworkStatus net = %d\n", net);
    trcFN("gsmGprsGetNetworkStatus ret = %d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    if(net == 1) {              //already connected
        ret = 0;
        goto lblEnd;
    }

    ret = comGprsSetAttachStatus(idx);
    CHECK(ret >= 0, lblKO);

    memset(&ctx, 0, sizeof(ctx));
    ctx.cid = actcid;           // PDP context identifier
    strcpy((char *) ctx.PDPType, "IP"); // Specifies the type of PDP
    strcpy((char *) ctx.APN, APN);  // Specifies the logical name to select GGSN or extern PDP
    strcpy((char *) ctx.PDPAddress, "");    // Identify the Mobile in the address space(optional)
    ctx.dComp = 0;              // To control PDP data compression(optional)
    ctx.hComp = 0;              // To control PDP header compression(optional)
    ret = gsmGprsSetPDPContext(com.prm.gprs.hdlGsm, &ctx);
    trcFN("gsmGprsSetPDPContext ret = %d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    memset(&prf, 0, sizeof(prf));
    prf.cid = actcid;           // PDP context identifier
    prf.precedence = 0;         // Precedence class
    prf.delay = 0;              // Delay class
    prf.reliability = 3;        // Reliability class
    prf.peak = 0;               // Peak throughput class
    prf.mean = 0;               // Mean throughput class
    ret = gsmGprsSetQoSProfile(com.prm.gprs.hdlGsm, &prf);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    gsmReadError(com.prm.gprs.hdlGsm, &err);
    trcFN("comSetGprsStatus readError = %d\n", err);
    gsmClose(com.prm.gprs.hdlGsm);
    com.prm.gprs.hdlGsm = 0;
    ret = -1;

  lblEnd:
    trcFN("comSetGprsStatus ret = %d\n", ret);
    return ret;
}

static int comGsmOperatorSelect(byte idx) {
    int ret;
    uint8 sta;
    uint16 err;

    trcS("comGsmOperatorSelect Beg\n");
    ret = gsmReadNetworkStatus(com.prm.gprs.hdlGsm, &sta);
    CHECK(ret == RET_OK, lblKO);
    switch (sta) {
      case 1:                  //registered, home network
      case 5:                  //registered, roaming
          return 0;             //nothing to do
      default:
          break;
    }

    while(idx--) {
        psyTimerWakeAfter(SYS_TIME_SECOND);
        ret = gsmOperatorSelection(com.prm.gprs.hdlGsm, 0, 2, "");
        switch (ret) {
          case RET_OK:
              return idx;       // found operator

          case GSM_CME_ERROR:
              gsmReadError(com.prm.gprs.hdlGsm, &err);
              // it is modem dependent only on WAVECOM modem.
              // this means : WAIT INIT ON COMMAND PROCESSING ON PROGRESS
              CHECK(err == GSM_PLEASE_WAIT, lblKO);
              break;

          default:             // real error
              goto lblKO;
        }
    }
  lblKO:
    return -1;
}
#endif

static int comSetGprs(const char *init) {
#ifdef __CHN_GPRS__
    int ret;
    int idx;
    netChannelList_t lst[NET_CHANNEL_MAX];
    netNiConfig_t cfg;
    uint16 err;
    char usr[32];
    char pwd[32];
    char pin[8 + 1];
    static char *phone = "*99***1#";    // for GPRS connection
    char m_apn[64 + 1];
    byte tryOp, tryAtt;         //number of attempts to select operator and to attach network
    char buf[4 + 1];
    card tmp;

    VERIFY(com.chn == chnGprs);
    VERIFY(init);
    trcS("comSetGprs Beg\n");
    tryOp = 5;                  //by default try 5 times to select operator
    tryAtt = 15;                //by default try 15 times to attach netwoerk

    init = parseStr(pin, init, sizeof(pin));    // extract pin
    init = parseStr(m_apn, init, sizeof(m_apn));    // extract apn
    init = parseStr(usr, init, sizeof(usr));    // extract usr
    init = parseStr(pwd, init, sizeof(pwd));    // extract password
    if(*init) {
        init = parseStr(buf, init, sizeof(buf));    // extract tryOp
        dec2num(&tmp, buf, 0);
        tryOp = (byte) tmp;
    }
    if(*init) {
        init = parseStr(buf, init, sizeof(buf));    // extract tryAtt
        dec2num(&tmp, buf, 0);
        tryAtt = (byte) tmp;
    }

    ret = comGsmPin(com.prm.gprs.hdlGsm, pin);
    CHECK(ret >= 0, lblKO);

    ret = comGsmOperatorSelect(tryOp);
    CHECK(ret >= 0, lblKO);

    ret = comSetGprsStatus(m_apn, tryAtt);
    CHECK(ret >= 0, lblKO);

    com.prm.gprs.hdlNet = 0;
    ret = netNiOpen(NET_NI_PPP, &com.prm.gprs.hdlNet);
    CHECK(ret == RET_OK, lblKO);

    // retrieve net channel list into buffer
    ret = netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE, (void *) lst);
    CHECK(ret == RET_OK, lblKO);

    // Look for "MODEM3" in channel list
    for (idx = 0;
         lst[idx].name[0] != 0 && strcmp(lst[idx].name, "MODEM3") != 0; idx++);

    // 'channel not found' checking
    CHECK(lst[idx].name[0] != 0, lblNotFound);

    // Initialize GPRS channel
    memset(&cfg, 0, sizeof(cfg));
    cfg.ppp.fields =
        NI_PPP_F_CHANNEL | NI_PPP_F_PPPCFG | NI_PPP_F_PHONENUMBER |
        NI_PPP_F_LOCALIPADDRESS;
    cfg.ppp.channel = idx;
    cfg.ppp.pppCfg.fields =
        NI_PPP_F_LCPFLAGS | NI_PPP_F_IPCPFLAGS | NI_PPP_F_VJMAX |
        NI_PPP_F_LOCALUSERNAME | NI_PPP_F_LOCALPASSWORD | NI_PPP_F_INFLAGS;
    cfg.ppp.pppCfg.lcpFlags =
        NI_PPP_LCP_PAP | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP;
    cfg.ppp.pppCfg.ipcpFlags =
        NI_PPP_IPCP_ACCEPT_LOCAL | NI_PPP_IPCP_ACCEPT_REMOTE | NI_PPP_IPCP_DNS1
        | NI_PPP_IPCP_DNS2;
    cfg.ppp.pppCfg.inFlags = NI_PPP_IN_DEFAULT_ROUTE;
    cfg.ppp.pppCfg.vjMax = 48;
    cfg.ppp.pppCfg.localUsername = usr;
    cfg.ppp.pppCfg.localPassword = pwd;
    cfg.ppp.phoneNumber = phone;

    ret = netNiConfigSet(com.prm.gprs.hdlNet, NET_NI_LEVEL_1, &cfg);
    trcFN("netNiConfigSet ret = %d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    ret = netNiStart(com.prm.gprs.hdlNet, 120 * SYS_TIME_SECOND);
    trcFN("netNiStart ret = %d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    // retrieve negociated local IP address and DNS addresses
    memset(&cfg, 0, sizeof(cfg));
    cfg.ppp.fields = NI_PPP_F_LOCALIPADDRESS | NI_PPP_F_PPPCFG;
    cfg.ppp.pppCfg.fields = NI_PPP_F_DNS1IPADDRESS | NI_PPP_F_DNS2IPADDRESS;
    ret = netNiConfigGet(com.prm.gprs.hdlNet, NET_NI_LEVEL_1, &cfg);
    trcFN("netNiConfigGet ret = %d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;

  lblNotFound:
  lblKO:
    gsmReadError(com.prm.gprs.hdlGsm, &err);
    trcFN("ReadError %d\n", err);

    if(com.prm.gprs.hdlNet != 0) {
        (void) netNiStop(com.prm.gprs.hdlNet);  // SIC: don't check return code!
        (void) netNiClose(com.prm.gprs.hdlNet); // SIC: don't check return code!
        com.prm.gprs.hdlNet = 0;
    }
    // disactivate all contexts
    gsmGprsSetPDPActivate(com.prm.gprs.hdlGsm, 0, 255);

    gsmClose(com.prm.gprs.hdlGsm);
    com.prm.gprs.hdlGsm = 0;
    ret = -1;

  lblEnd:
    trcFN("comSetGprs ret = %d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSetCom(const char *init) {
#ifdef __CHN_COM__
    int ret;
    enum comDataSize_t datasize;    //data bits: 7 or 8
    enum comParity_t parity;    //odd,even,none
    enum comStopBits_t stopbits;    //1,2
    enum comSpeed_t baud;       //The speed
    comUartInit_t uIni;
    comInit_t cIni;

    comUartProtocolCfg_t uartcfg;
    comProtocolCfg_t cfg;

    trcFS("comSetCom %s\n", init);
    VERIFY(init);
    VERIFY(com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);

    cfg.type = COM_PROT_CFG_TYPE_UART;
    cfg.level = COM_PROT_CFG_LEVEL_1;
    cfg.protCfg.uartCfg = &uartcfg;
    uartcfg.flowControl = (enum comFlowControl_t) (init[strlen(init) - 1] ==
                                                   'F' ? COM_FLOW_HARD_IN :
                                                   COM_FLOW_NONE);
    comSetProtocol(com.prm.mdm.hdl, COM_SET_PROTOCOL_TEMPORARY, &cfg);

    //parse control string
    datasize = COM_DATASIZE_8;
    switch (*init++) {          //number of data bits
      case '7':
          datasize = COM_DATASIZE_7;
          break;
      case '8':
          datasize = COM_DATASIZE_8;
          break;
      default:
          CHECK(!init, lblKO);
    }
    parity = COM_PARITY_NONE;
    switch (*init++) {          //parity
      case 'N':
          parity = COM_PARITY_NONE;
          break;
      case 'E':
          parity = COM_PARITY_EVEN;
          break;
      case 'O':
          parity = COM_PARITY_ODD;
          break;
      default:
          CHECK(!init, lblKO);
    }
    stopbits = COM_STOP_NO_CHANGE;
    switch (*init++) {          //number of stop bits
      case '0':
          stopbits = COM_STOP_NO_CHANGE;
          break;
      case '1':
          stopbits = COM_STOP_1;
          break;
      case '2':
          stopbits = COM_STOP_2;
          break;
      default:
          CHECK(!init, lblKO);
    }

    baud = COM_BAUD_NO_CHANGE;
    if(*init == 0) {            //default parameter
        baud = COM_BAUD_9600;
        init++;
    } else if(memcmp(init, "300", 3) == 0) {
        baud = COM_BAUD_300;
        init += 3;
    } else if(memcmp(init, "600", 3) == 0) {
        baud = COM_BAUD_600;
        init += 3;
    } else if(memcmp(init, "1200", 4) == 0) {
        baud = COM_BAUD_1200;
        init += 4;
    } else if(memcmp(init, "4800", 4) == 0) {
        baud = COM_BAUD_4800;
        init += 4;
    } else if(memcmp(init, "9600", 4) == 0) {
        baud = COM_BAUD_9600;
        init += 4;
    } else if(memcmp(init, "19200", 5) == 0) {
        baud = COM_BAUD_19200;
        init += 5;
    } else if(memcmp(init, "38400", 5) == 0) {
        baud = COM_BAUD_38400;
        init += 5;
    } else if(memcmp(init, "57600", 5) == 0) {
        baud = COM_BAUD_57600;
        init += 5;
    } else if(memcmp(init, "76800", 5) == 0) {
        baud = COM_BAUD_76800;
        init += 5;
    } else if(memcmp(init, "115200", 6) == 0) {
        baud = COM_BAUD_115200;
        init += 6;
    }
    //prepare port parameters: datasize,parity,stopbits
    memset(&uIni, 0, sizeof(uIni));
    memset(&cIni, 0, sizeof(cIni));

    uIni.speed = baud;
    uIni.parity = parity;
    uIni.dataSize = datasize;
    uIni.stopBits = stopbits;
    uIni.rBufferSize = 1024;    //0;
    uIni.sBufferSize = 1024;    //0;

    cIni.type = COM_INIT_TYPE_UART;
    cIni.level = COM_INIT_LEVEL_1;
    cIni.init.uartInit = &uIni;

    ret =
        comSetPortParameters(com.prm.mdm.hdl, COM_MODIF_PARAM_TEMPORARY, &cIni);
    CHECK(ret == RET_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    comClose(com.prm.mdm.hdl);
    ret = -1;
  lblEnd:
    trcFN("comSetCom ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSetTcp(const char *init) {
#ifdef __CHN_TCP__
#ifdef WIN32
    return 1;
#else
    int ret;
    netNiConfig_t cfg;
    char gateway[15 + 1];       // for TAMAS

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780);
    VERIFY(init);

    memset(&cfg, 0, sizeof(netNiConfig_t));
    cfg.eth.channel = com.prm.tcp.idx;

    if(*init)                   // static IP address
    {
        char adr[15 + 1];
        char msk[15 + 1];
        char port[10];

        init = parseStr(adr, init, sizeof(adr));
        VERIFY(*init);          // verify port follows

        // parse port
        init = parseStr(port, init, sizeof(port));
        VERIFY(*init);          // verify msk follows

        // parse msk
        init = parseStr(msk, init, sizeof(msk));

        // parse default gateway (for TAMAS)
        (void) parseStr(gateway, init, sizeof(gateway));

        cfg.eth.localIpAddress = inet_addr(adr);
        cfg.eth.subNetworkMask = inet_addr(msk);
        cfg.eth.fields =
            NI_ETH_F_CHANNEL | NI_ETH_F_LOCALIPADDRESS |
            NI_ETH_F_SUBNETWORKMASK;
    } else {
        cfg.eth.fields =
            NI_ETH_F_CHANNEL | NI_ETH_F_INFLAGS | NI_ETH_F_DHCP_TIMEOUT;
        cfg.eth.inFlags = NI_ETH_IN_IPADDR_VIA_DHCP;
        cfg.eth.dhcpReleaseTimeout = 20;
        gateway[0] = 0;         // for TAMAS
    }

    ret = netNiConfigSet(com.prm.tcp.hdl, NET_NI_LEVEL_1, &cfg);
    CHECK(ret == RET_OK || ret == NET_ERR_WRONG_APP, lblKO);

    ret = netNiStart(com.prm.tcp.hdl, 0);
    CHECK(ret == RET_OK || ret == NET_ERR_WRONG_APP, lblKO);

    // for Borica a default route is added (used by TAMAS only)
    if(gateway[0]) {
        ret = ipRouteSetDef(inet_addr(gateway));
        CHECK(ret == RET_OK, lblKO);
    }

    ret = 1;
    goto lblEnd;

  lblKO:
    ret = socketerror();
    trcFN("Channnel Start Error %d\n", ret);
    netNiClose(com.prm.tcp.hdl);

  lblEnd:
    trcFN("comSetTcp ret=%d\n", ret);
    return ret;
#endif
#else
    return -1;
#endif
}

static int readFile(char *fileName, char *cert, word * bufLen) {
    card fid = 0;
    int ret;
    psyFileStat_t statbuf;

    CHECK(((fileName != NULL) || (cert != NULL)), lblKO);

    // Getting file size
    ret = psyFileStat(fileName, &statbuf);
    CHECK(ret == RET_OK, lblEnd);
    CHECK(statbuf.stSize <= *bufLen, lblKO);

    // Certificate is loaded
    fid = psyFileOpen(fileName, "r");
    CHECK(fid != 0, lblKO);

    *bufLen = (int) statbuf.stSize;
    ret = psyFileRead(fid, cert, bufLen);
    CHECK(ret == RET_OK, lblKO);

    ret = RET_OK;
    goto lblEnd;

  lblKO:
    ret = -1;
  lblEnd:
    if(fid != 0) {
        psyFileClose(fid);
    }
    return ret;
}

static int comSetSsl(const char *init) {
#ifdef __CHN_SSL__
    int ret = 0;
    word res;
    card actions[SSL_X509_V_MAX_ERRORS];
    char ProfNum[2 + 1];
    char keyFile[64 + 1];
    char crtFile[64 + 1];
    char caFile[64 + 1];

    VERIFY(com.chn == chnSsl5100 || com.chn == chnSsl7780);
    VERIFY(init);
    trcS("comSetSsl Beg\n");
    memset(caFile, 0, sizeof(caFile));
    init = parseStr(ProfNum, init, sizeof(ProfNum));
    init = parseStr(keyFile, init, sizeof(keyFile));
    init = parseStr(crtFile, init, sizeof(crtFile));
    init = parseStr(caFile, init, sizeof(caFile));

    ret = comSetTcp("");
    CHECK(ret >= 0, lblKO);

    ret = sslContextNew(SSL_V3_CLIENT, &com.prm.ssl.context);
    CHECK(ret == RET_OK, lblKO);
    com.prm.ssl.periph = 1;

    ret = sslContextSetCipherList(com.prm.ssl.context,
                                  SSL_KEY_RSA | SSL_AUTH_RSA |
                                  SSL_ENC_3DES | SSL_MAC_SHA1 | SSL_VER_SSLV3,
                                  SSL_NO_EXPORT_CIPHER | SSL_STRENGTH_HIGH);

    //CA certificate 
    if(!*caFile) {
        memcpy(caFile, "c:\\CA.CRT", 10);
    }
    res = sizeof(com.prm.ssl.certCA);
    ret = readFile(caFile, com.prm.ssl.certCA, &res);
    CHECK(ret == RET_OK, lblKO);

    ret =
        sslContextCACertificateLoad(com.prm.ssl.context, com.prm.ssl.certCA,
                                    res);
    CHECK(ret == RET_OK, lblKO);

    //Certificate File
    if(*crtFile) {
        res = sizeof(com.prm.ssl.certCA);
        ret = readFile(crtFile, com.prm.ssl.certCA, &res);
        CHECK(ret == RET_OK, lblKO);

        ret =
            sslContextCertificateUse(com.prm.ssl.context, com.prm.ssl.certCA,
                                     res);
        CHECK(ret == RET_OK, lblKO);
    }
    //Key Certificate
    if(*keyFile) {
        res = sizeof(com.prm.ssl.certCA);
        if(readFile(keyFile, com.prm.ssl.certCA, &res) != RET_OK)
            CHECK(ret == RET_OK, lblKO);

        ret =
            sslContextPrivateKeyUse(com.prm.ssl.context, com.prm.ssl.certCA,
                                    res, NULL);
        CHECK(ret == RET_OK, lblKO);
    }
    //Default is to continue process 
    memset(actions, 0xff, sizeof(actions));
    // Stop process for the following error 
    SSL_X509_V_STOP_SET(actions, SSL_X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT);
    SSL_X509_V_STOP_SET(actions, SSL_X509_V_ERR_CERT_NOT_YET_VALID);
    SSL_X509_V_STOP_SET(actions, SSL_X509_V_ERR_CERT_HAS_EXPIRED);
    ret =
        sslContextCertificateVerifySet(com.prm.ssl.context, SSL_PEER_VERIFY,
                                       actions);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSetSsl ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSetPpp(const char *init) {
#ifdef __CHN_PPP__
    int ret;
    netNiConfig_t cfg;
    char usr[32];               //= "danfr";
    char pwd[32];               //= "mq36noqt";
    char phone[32];             //= "0,0860922000";
    card dly;                   //= 60;

    VERIFY(com.chn == chnPpp);
    VERIFY(init);

    trcS("comSetPpp Beg\n");
    memset(&cfg, 0, sizeof(netNiConfig_t));

    init = parseStr(phone, init, sizeof(phone));
    init = parseStr(usr, init, sizeof(usr));
    init = parseStr(pwd, init, sizeof(pwd));

    dec2num(&dly, init, 0);

    cfg.ppp.fields = NI_PPP_F_CHANNEL
        | NI_PPP_F_PPPCFG | NI_PPP_F_PHONENUMBER | NI_PPP_F_LOCALIPADDRESS;

    cfg.ppp.channel = com.prm.ppp.idx;

    cfg.ppp.pppCfg.fields = NI_PPP_F_LCPFLAGS
        | NI_PPP_F_IPCPFLAGS
        | NI_PPP_F_LOCALUSERNAME | NI_PPP_F_LOCALPASSWORD | NI_PPP_F_INFLAGS;

    cfg.ppp.pppCfg.inFlags = NI_PPP_IN_DEFAULT_ROUTE;
    cfg.ppp.pppCfg.lcpFlags =
        (NI_PPP_LCP_PAP | NI_PPP_LCP_CHAP | NI_PPP_LCP_MSCHAP)
        | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP | NI_PPP_LCP_MAGIC;

    cfg.ppp.pppCfg.ipcpFlags = NI_PPP_IPCP_ACCEPT_LOCAL
        | NI_PPP_IPCP_ACCEPT_REMOTE | NI_PPP_IPCP_DNS1 | NI_PPP_IPCP_DNS2;
    cfg.ppp.pppCfg.localUsername = (char *) usr;
    cfg.ppp.pppCfg.localPassword = (char *) pwd;
    cfg.ppp.phoneNumber = (char *) phone;
    cfg.ppp.localIpAddress = 0;

    trcFN("com.prm.ppp.idx = %d\n", com.prm.ppp.idx);
    trcFS("usr = %s\n", usr);
    trcFS("pwd = %s\n", pwd);

    ret = netNiConfigSet(com.prm.ppp.hdl, NET_NI_LEVEL_1, &cfg);
    CHECK(ret == RET_OK, lblKO);

    trcFN("dly=%d\n", dly);
    ret = netNiStart(com.prm.ppp.hdl, dly * SYS_TIME_SECOND);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    netNiStop(com.prm.ppp.hdl);
    ret = -1;
  lblEnd:
    trcFN("comSetPpp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

/** Initiates a channel using the parameter string init.
 * \param init (I) Parameter string.
 *  - Internal modem (chnMdm, chnHdlc):
 *    - '.':  to enable data compression (optional)
 *    - '?':  to enable error correction (optional)
 *    - '#':  to enable DTR hangup mode  (optional)
 *    - 'p'|'P'|'t'|'T':  pulse or tone dialing; uppercase allows fast connection (optional)
 *    - one digit: number of data bits in ASCII ('7' or '8')
 *    - one digit: parity ('N','E' or 'O')
 *    - one digit: number of stop bits in ASCII('0','1' or '2')
 *    - 3-5 digits: baudrate in ASCI (300,600,1200,2400,9600,19200 or 28000)
 *    - 'D':  to enable dial tone monitoring (optional)
 *    - The optional digits are independent from each other, but they are not permutable.
 *      For example ".#8N11200" is right, but "#.8N11200" is wrong.
 *    - For HDLC connection the init string is ignored (always 1200 baud)
 *  - Serial pors (chnCom1, chnCom2, chnCom3):
 *    - one digit: number of data bits in ASCII ('7' or '8')
 *    - one digit: parity ('N','E' or 'O')
 *    - one digit: number of stop bits in ASCII('0','1' or '2')
 *    - 2-6 digits: baudrate in ASCI (optional, the default value is 9600)
 *                  (300,600,1200,2400,9600,19200,38400,57600,76800,115200)
 *    - 'F':  to enable flow control hardware (optional)
 *  - TCP/IP connection
 *    - If the *init!=0, then use static IP adress
 *      - It has to have the "adress-port-mask" format (the '|' is the separator).
 *  - PPP connection
 *    - The separator character is '|'. The fields:
 *      - Phone: pohe number
 *      - User: user id
 *      - Password: password of the user
 *      - Timeout (in ASCI): timeout of connection in second
 *  - GPRS connection
 *    - The separator character is '|'. The fields:
 *      - Phone: phone number
 *      - Apn: apn adress
 *      - User: user id
 *      - Password: password of the user
 *      - IP: ip adress
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
*/
int comSet(const char *init) {
    VERIFY(init);
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
          return comSetMdm(init);
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return comSetCom(init);
      case chnTcp5100:
      case chnTcp7780:
          return comSetTcp(init);
      case chnPpp:
          return comSetPpp(init);
      case chnGprs:
          return comSetGprs(init);
      case chnSsl5100:
      case chnSsl7780:
          return comSetSsl(init);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comStopMdm(void) {
#ifdef __CHN_COM__
    int ret;

    trcS("comStopMdm\n");
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);

    if(com.prm.mdm.hdl) {
        ret = comClose(com.prm.mdm.hdl);
        CHECK(ret == RET_OK, lblKO);
        com.prm.mdm.hdl = 0;
    }
    return com.prm.mdm.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopCom(void) {
#ifdef __CHN_COM__
    int ret;

    trcS("comStopCom\n");
    VERIFY(com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);

    if(com.prm.mdm.hdl) {
        ret = comClose(com.prm.mdm.hdl);
        CHECK(ret == RET_OK, lblKO);
        com.prm.mdm.hdl = 0;
    }
    return com.prm.mdm.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopTcp(void) {
#ifdef __CHN_TCP__
    int ret;

    trcS("comStopTcp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 ||
           com.chn == chnSsl5100 || com.chn == chnSsl7780);

    if(com.prm.tcp.hdl) {
        ret = netNiStop(com.prm.tcp.hdl);
        CHECK(ret == RET_OK, lblKO);
    }
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comCloseTcp(void) {
#ifdef __CHN_TCP__
    int ret;

    trcS("comStopTcp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 ||
           com.chn == chnSsl5100 || com.chn == chnSsl7780);

    if(com.prm.tcp.hdl) {
        ret = netNiClose(com.prm.tcp.hdl);
        CHECK(ret == RET_OK, lblKO);
        com.prm.tcp.hdl = 0;
    }
    return com.prm.tcp.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopPpp(void) {
#ifdef __CHN_PPP__
    int ret;

    trcS("comStopPpp\n");
    VERIFY(com.chn == chnPpp);

    if(com.prm.ppp.hdl != 0) {
        ret = netNiStop(com.prm.ppp.hdl);
        CHECK(ret == RET_OK, lblKO);

        ret = netNiClose(com.prm.ppp.hdl);
        CHECK(ret == RET_OK, lblKO);

        com.prm.ppp.hdl = 0;
    }
    return com.prm.ppp.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopGprs(void) {
#ifdef __CHN_GPRS__
    trcS("comStopGprs\n");
    VERIFY(com.chn == chnGprs);

    if(com.prm.gprs.hdlNet) {
        netNiStop(com.prm.gprs.hdlNet); // SIC: don't check return code!
        netNiClose(com.prm.gprs.hdlNet);    // SIC: don't check return code!
        com.prm.gprs.hdlNet = 0;
    }
    // Disconnect the PDP context to avoid GSM Data problem
    // Not required if you want to do several GPRS without GSM data
    gsmGprsSetPDPActivate(com.prm.gprs.hdlGsm, 0, 255);
    gsmClose(com.prm.gprs.hdlGsm);
    com.prm.gprs.hdlGsm = 0;
    return 1;
#else
    return -1;
#endif
}

static int comStopSsl(void) {
#ifdef __CHN_SSL__
    trcS("comStopSsl\n");
    VERIFY(com.chn == chnSsl5100 || com.chn == chnSsl7780);

    switch (com.prm.ssl.periph) {
      case 3:
          close(com.prm.tcp.sck);
      case 2:
          sslSessionShutdown(com.prm.ssl.session);
          sslSessionFree(com.prm.ssl.session);
      case 1:
          sslContextFree(com.prm.ssl.context);
      default:
          comStopTcp();
          comCloseTcp();
          break;
    }

    com.prm.ssl.hdl = 0;
    com.prm.ssl.periph = 0;
    return 1;
#else
    return -1;
#endif
}

/** Close the associated channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
*/
int comStop(void) {
    int ret;

    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
          ret = comStopMdm();
          break;
      case chnCom1:
      case chnCom2:
      case chnCom3:
          ret = comStopCom();
          break;
      case chnTcp5100:
      case chnTcp7780:
          ret = comStopTcp();
          break;
      case chnPpp:
          ret = comStopPpp();
          break;
      case chnGprs:
          ret = comStopGprs();
          break;
      case chnUSB:
          ret = 0;
          break;
      case chnSsl5100:
      case chnSsl7780:
          ret = comStopSsl();
          break;
      default:
          VERIFY(com.chn < chnEnd);
          ret = -1;
          break;
    }
    com.chn = 0;
    return ret;
}

int comCloseNet(void) {
    switch (com.chn) {
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comCloseTcp();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comDialMdm(const char *phone) {
#ifdef __CHN_COM__
    int ret;
    int len;
    comConnect_t cnx;
    comModemConnect_t mdm;
    tResponse rsp;
    char *pabx;
    char *prefix;
    char *country;
    char tmp[64 + 1];
    char buf[64 + 1];

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);
    VERIFY(phone);
    VERIFY(strlen(phone) < sizeof(tmp));

    trcFS("comDialMdm phone = %s\n", phone);
    ret = 0;
    len = 0;
    strcpy(tmp, phone);
    pabx = "";
    prefix = "";
    country = "";
    phone = tmp;
    while(tmp[len]) {           //extract pabx if found
        if(tmp[len] != '-') {
            len++;
            continue;
        }
        pabx = tmp + ret;
        tmp[len++] = 0;
        ret = len;
        phone = tmp + len;
        break;
    }
    while(tmp[len]) {           //extract prefix if found
        if(tmp[len] != '-') {
            len++;
            continue;
        }
        prefix = tmp + ret;
        tmp[len++] = 0;
        ret = len;
        phone = tmp + len;
        break;
    }
    while(tmp[len]) {           //extract country code if found
        if(tmp[len] != '-') {
            len++;
            continue;
        }
        country = tmp + ret;
        tmp[len++] = 0;
        ret = len;
        phone = tmp + len;
        break;
    }

    memset(&cnx, 0, sizeof(cnx));
    memset(&mdm, 0, sizeof(mdm));

    mdm.cnxMode = CONNECT_MODE_STD;
    mdm.PABX = (char *) pabx;
    strcpy(buf, prefix);
    if(*country) {
        strcat(buf, ",");
        strcat(buf, country);
        strcat(buf, ",");
    }
    strcat(buf, phone);
    mdm.phoneNumber = (char *) buf;
    if(com.chn == chnHdlc) {
        mdm.protocolList[0] = MODEM_PROTOCOL_HDLC_V80;
        mdm.protocolList[1] = MODEM_PROTOCOL_NONE;
        mdm.protocolList[2] = MODEM_PROTOCOL_NONE;
        mdm.protocolList[3] = MODEM_PROTOCOL_NONE;
        mdm.protocolList[4] = MODEM_PROTOCOL_NONE;
        mdm.protocolParameter[0] = NULL;
        mdm.protocolParameter[1] = NULL;
        mdm.protocolParameter[2] = NULL;
        mdm.protocolParameter[3] = NULL;
        mdm.protocolParameter[4] = NULL;
    } else {
        mdm.protocolList[0] = MODEM_PROTOCOL_NONE;
    }

    cnx.type = COM_CONNECT_TYPE_MODEM;
    cnx.level = COM_CONNECT_LEVEL_1;
    cnx.connect.modemConnect = &mdm;

    ret = comConnectReq(com.prm.mdm.hdl, &cnx); // now dial
    CHECK(ret == RET_RUNNING, lblKO);

    ret = kbdStart(1);          // start kbd acquisition to be able to abort dialing
    CHECK(ret > 0, lblKO);

    ret = psyPeripheralResultWait(COM_WAIT | HMI_WAIT, PSY_INFINITE_TIMEOUT, 0);    // wait for result
    CHECK(ret >= 0, lblKO);

    if(ret & HMI_WAIT) {
        ret = -comAnn;
        goto lblEnd;
    }

    ret = comResultGet(com.prm.mdm.hdl, COM_CONNECT_EVENT, sizeof(rsp), &rsp);  // retrieve the result
    CHECK(ret == RET_OK, lblKO);

    switch (rsp.sta) {
      case COM_OK:
          ret = 0;
          break;
      case COM_BUSY_LINE:
          ret = -comBusy;
          break;
      case COM_NO_DIALTONE:
          ret = -comNoDialTone;
          break;
      case COM_NO_CARRIER:
          ret = -comNoCarrier;
          break;
      case COM_ERR_DEVICE:
          ret = -comNoCarrier;
          break;
      default:
          ret = -comKO;
          break;
    }
    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:
    kbdStop();
    return ret;
#else
    return -1;
#endif
}

#ifdef __CHN_TCP__
static int comListenIp(const char *srv) {
    int ret = 0, retErr = 0;
    char adr[15 + 1];           //tcp address NNN.NNN.NNN.NNN
    char port[4 + 1];
    struct sockaddr_in dst;
    char msk[15 + 1];
    tBuffer buf;
    byte dat[128];
    card dport;

    VERIFY(srv);
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 || com.chn == chnPpp);
    bufInit(&buf, dat, sizeof(dat));

    com.chnRole = chnRoleSrv;
    trcFS("comListenIp srv = %s\n", srv);

    srv = parseStr(adr, srv, sizeof(adr));  // INADDR_ANY is used instead of adr

    srv = parseStr(port, srv, sizeof(port));    // start port
    ret = dec2num(&dport, port, 0);

    (void) parseStr(msk, srv, sizeof(msk)); // start mask

    com.prm.tcp.cltSck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK(com.prm.tcp.cltSck >= 0, lblSCKKO);

    dst.sin_len = sizeof(dst);
    dst.sin_family = AF_INET;
    dst.sin_port = htons_f((word) dport);
    dst.sin_addr.s_addr = INADDR_ANY;
    memset(dst.sin_zero, 0, sizeof(dst.sin_zero));

    /* Bind socket */
    ret = bind(com.prm.tcp.cltSck, (struct sockaddr *) &dst, sizeof(dst));
    CHECK(ret == 0, lblKO);
    /* listen  */
    ret = listen(com.prm.tcp.cltSck, 1);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblSCKKO:
    retErr = socketerror();
    trcFN("Socket Error %d\n", retErr);
    ret = retErr;
    goto lblEnd;
  lblKO:
    retErr = socketerror();
    trcFN(" Bind Or listen problem Error %d\n", retErr);
    comHangStart();
    comHangWait();
    ret = retErr;
  lblEnd:
    trcFN("comListenIp ret=%d\n", ret);
    return ret;
}
#endif

#ifdef __CHN_TCP__
/** Accept incoming connection
 * \todo To be documented
 * \test tcnl0020.c
*/
static int comAcceptIp(const char *srv) {
    int ret = 0, retErr = 0;
    char adr[15 + 1];           // tcp address NNN.NNN.NNN.NNN
    char port[4 + 1];
    struct sockaddr_in dst;
    int destAddrLen;
    char msk[15 + 1];
    tBuffer buf;
    byte dat[128];
    card dport;

    VERIFY(srv);
    bufInit(&buf, dat, sizeof(dat));

    com.chnRole = chnRoleSrv;

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 || com.chn == chnPpp);

    trcFS("comAcceptIp srv = %s\n", srv);

    srv = parseStr(adr, srv, sizeof(adr));  // INADDR_ANY is used instead of adr

    srv = parseStr(port, srv, sizeof(port));    // start port
    ret = dec2num(&dport, port, 0);

    (void) parseStr(msk, srv, sizeof(msk)); // start mask

    dst.sin_len = sizeof(dst);
    dst.sin_family = AF_INET;
    dst.sin_port = htons_f((word) dport);
    dst.sin_addr.s_addr = INADDR_ANY;
    memset(dst.sin_zero, 0, sizeof(dst.sin_zero));

    if(com.prm.tcp.sck >= 0) {
        /* Accept connection */
        destAddrLen = sizeof(dst);
        com.prm.tcp.srvSck =
            accept(com.prm.tcp.cltSck, (struct sockaddr *) &dst, &destAddrLen);
        CHECK(com.prm.tcp.srvSck >= 0, lblAccept);
    }

    ret = 1;
    goto lblEnd;
  lblAccept:
    trcFN("Error %d", socketerror());
    comHangStart();
    comHangWait();
    ret = retErr;
  lblEnd:
    trcFN("comAcceptIp ret=%d\n", ret);
    return ret;
}
#endif

static int comDialIp(const char *srv) {
#ifdef __CHN_TCP__
    int ret = 0, retErr = 0;
    char adr[16];               //tcp address NNN.NNN.NNN.NNN
    char ladr[4];
    card dLocalPort;
    card dRemotePort;
    char RemotePort[10];
    char LocalPort[10];
    struct sockaddr_in dst;

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs
           || com.chn == chnSsl5100 || com.chn == chnSsl7780);
    VERIFY(srv);

    trcFS("comDialIp srv = %s\n", srv);
    com.chnRole = chnRoleClt;

    srv = parseStr(adr, srv, sizeof(adr));
    VERIFY(*srv);

    srv = parseStr(RemotePort, srv, sizeof(RemotePort));
    ret = dec2num(&dRemotePort, RemotePort, 0);

    memset(ladr, 0, sizeof(ladr));
    dLocalPort = 0;
    if(*srv) {
        // GET lOCAL IP
        srv = parseIp(ladr, srv);
        // extract Local Port
        VERIFY(*srv);
        parseStr(LocalPort, srv, sizeof(LocalPort));
        ret = dec2num(&dLocalPort, LocalPort, 0);
        CHECK(ret == (int) strlen(srv), lblKO);
    }
#ifdef __CHN_SSL__
    ret = sslSessionNew(com.prm.ssl.context, &com.prm.ssl.session);
    CHECK(ret == RET_OK, lblKO);
    com.prm.ssl.periph = 2;
#endif

    switch (com.chn) {
#ifdef __CHN_TCP__
      case chnTcp5100:
      case chnTcp7780:
      case chnSsl5100:
      case chnSsl7780:
          com.prm.tcp.sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
          break;
#endif
#ifdef __CHN_PPP__
      case chnPpp:
          com.prm.ppp.sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
          break;
#endif
      default:
          break;
    }
#ifdef __CHN_PPP__
    VERIFY(com.prm.tcp.sck == com.prm.ppp.sck);
#endif
    CHECK(com.prm.tcp.sck >= 0, lblSCKKO);
#ifdef __CHN_SSL__
    com.prm.ssl.periph = 3;
#endif

    if(dLocalPort > 0) {
        memset(&dst, 0, sizeof(dst));
        dst.sin_len = sizeof(dst);
        dst.sin_family = AF_INET;
        dst.sin_port = htons_f((word) dLocalPort);
        dst.sin_addr.s_addr =
            htonl_f(IP_DOT2BIN(ladr[0], ladr[1], ladr[2], ladr[3]));
        ret = bind(com.prm.tcp.sck, (struct sockaddr *) &dst, sizeof(dst));
        CHECK(ret == 0, lblKO);
    }
    memset(&dst, 0, sizeof(dst));
    dst.sin_len = sizeof(struct sockaddr_in);
    dst.sin_family = AF_INET;
    dst.sin_port = htons_f((word) dRemotePort);
    dst.sin_addr.s_addr = inet_addr(adr);
    ret =
        connect(com.prm.tcp.sck, (struct sockaddr *) &dst,
                sizeof(struct sockaddr_in));
    CHECK(ret == 0, lblConnect);

#ifdef __CHN_SSL__
    // Socket is binded
    ret =
        sslAttachSocket(com.prm.ssl.session, com.prm.tcp.sck,
                        SSL_ATTACH_BIO_NOCLOSE);
    CHECK(ret == RET_OK, lblKO);

    // Getting connexion to remote host
    ret = sslConnect(com.prm.ssl.session);
    CHECK(ret == RET_OK, lblKO);
#endif

    ret = 1;
    goto lblEnd;
  lblSCKKO:
    retErr = socketerror();
    trcFN("socket error %d\n", retErr);
    ret = retErr;
    goto lblEnd;
  lblConnect:
    ret = socketerror();
    trcFN(" connect Error %d\n", ret);
    comHangStart();
    comHangWait();
    goto lblEnd;

  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comDialIp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comDialGprsIp(const char *srv) {
#ifdef __CHN_GPRS__
    int ret = 0, retErr = 0;
    char adr[4];                // tcp address NNN.NNN.NNN.NNN
    card dRemotePort;
    char RemotePort[10];
    struct sockaddr_in dst;

    VERIFY(com.chn == chnGprs);
    trcFS("comDialGprsIp srv = %s\n", srv);

    com.chnRole = chnRoleClt;

    srv = parseIp(adr, srv);
    VERIFY(*srv);               // Verify remote port follows

    (void) parseStr(RemotePort, srv, sizeof(RemotePort));

    ret = dec2num(&dRemotePort, RemotePort, 0);

    com.prm.tcp.sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK(com.prm.tcp.sck >= 0, lblSCKKO);

    dst.sin_len = sizeof(struct sockaddr_in);
    dst.sin_family = AF_INET;
    dst.sin_port = htons_f((word) dRemotePort);
    dst.sin_addr.s_addr = htonl_f(IP_DOT2BIN(adr[0], adr[1], adr[2], adr[3]));
    memset(&dst.sin_zero, 0, sizeof(dst.sin_zero));
    ret =
        connect(com.prm.tcp.sck, (struct sockaddr *) &dst,
                sizeof(struct sockaddr_in));
    CHECK(ret == 0, lblConnect);
    ret = 1;
    goto lblEnd;

  lblSCKKO:
    retErr = socketerror();
    trcFN("Error %d\n", retErr);
    ret = retErr;
    goto lblEnd;
  lblConnect:
    retErr = socketerror();
    trcFN(" connect Error %d\n", retErr);
    comHangStart();
    comHangWait();
    ret = retErr;
    goto lblEnd;

  lblEnd:
    trcFN("comDialGprsIp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

/** If  the current channel is internal modem dials a phone number.
 * Dialing can be stopped by pressing a key.
 * The function kbdStart() is called inside this function,
 * thus keyboard waiting should not be started before calling it.
 * If the current channel is TCP connects to the server.
 * \param srv (I)
 *   - For internal modem phone number containing pabx and prefix separated by �-�:
 *     srv="0-00-1234" consists of pabx="0", prefix="00" and phone itself "1234"
 *     srv="0-1234" consists of pabx="0" and phone itself "1234" without prefix
 *     srv="1234" does not contain any pabx and prefix
 *   - For TCP it is TCP/IP address of the server and the port number in the format like:
 *     "10.10.59.27-2000"
 *   - For IP channels (TCP and PPP) this parameter contains IP address and port to connect
 *     in the format NNN.NNN.NNN.NNN-PPPPP
 * \return
 *    - non-negative value if OK.
 *    - otherwise the return code is a negative value according to:
 *        - comTimeout : timeout of 1 minute
 *        - comAnn: operation aborted by user pressing a key
 *        - comNoCarrier: NO CARRIER
 *        - comNoDialTone: NO DIAL TONE
 *        - comBusy: BUSY
 *        - comKO: any other problem
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
*/
int comDial(const char *srv) {
    VERIFY(srv);
    trcFS("comDial srv=%s\n", srv);
    switch (com.chn) {
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return 0;
      case chnMdm:
      case chnHdlc:
          return comDialMdm(srv);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnSsl5100:
      case chnSsl7780:
          return comDialIp(srv);
      case chnGprs:
          return comDialGprsIp(srv);

          //return comDialSsl(srv);       
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

/** Start listening a port
 * \todo To be documented
 * \test tcnl0020.c
*/
int comListen(const char *srv) {
#ifdef __CHN_TCP__
    VERIFY(srv);
    trcFS("comListen srv=%s\n", srv);
    switch (com.chn) {
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comListenIp(srv);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
#else
    return -1;
#endif
}

/** Accept incoming connection
 * \todo To be documented
*/
int comAccept(const char *srv) {
#ifdef __CHN_TCP__
    VERIFY(srv);
    trcFS("comAccept srv=%s\n", srv);
    switch (com.chn) {
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comAcceptIp(srv);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
#else
    return -1;
#endif
}

static int comHangStartMdm(void) {
#ifdef __CHN_COM__
    int ret;

    trcS("comHangStartMdm\n");
    comTransmissionFlush(com.prm.mdm.hdl);
    comReceptionFlush(com.prm.mdm.hdl);
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);
    ret = comDisconnectReq(com.prm.mdm.hdl);    //send disconnect request
    CHECK(ret == RET_RUNNING, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comHangStartIp(void) {
#ifdef __CHN_TCP__
    int ret = 0;

    trcS("comHangStartIp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs);

    switch (com.chnRole) {
      case chnRoleSrv:
          ret = close(com.prm.tcp.srvSck);
          ret = close(com.prm.tcp.cltSck);
          break;
      case chnRoleClt:
      default:
          ret = close(com.prm.tcp.sck);
          break;
    }
    if(ret < 0)
        return -1;

    return 1;
#else
    return -1;
#endif
}

static int comHangStartSsl(void) {
#ifdef __CHN_SSL__
    return 1;
#else
    return -1;
#endif
}

/** For internal modem initiates hanging up. Can be applied to internal modem only.
 * For 32bit it does not wait for result for performance reasons.
 * However, it is necessary to call comHangWait() before closing com port.
 * For TCP closes the socket.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
 */
int comHangStart(void) {
    trcS("comHangStart\n");
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
          return comHangStartMdm();
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comHangStartIp();
      case chnSsl5100:
      case chnSsl7780:
          return comHangStartSsl();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comHangWaitMdm(void) {
#ifdef __CHN_COM__
    int ret;
    tResponse rsp;

    trcS("comHangWaitMdm\n");
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);

    do {
        ret = psyPeripheralResultWait(COM_WAIT, PSY_INFINITE_TIMEOUT, 0);   // wait while it is executed
        CHECK(ret >= 0, lblKO);
        ret = comResultGet(com.prm.mdm.hdl, COM_DISCONNECT_EVENT, sizeof(rsp), &rsp);   // get the result
    } while(ret == RET_RUNNING);

    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comHangWaitIp(void) {
#ifdef __CHN_TCP__
    trcS("comHangWaitIp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs || com.chn == chnSsl);
    return 1;
#else
    return -1;
#endif
}

/** For internal modem wait while hang up finishes.
 * The function comHangStart() should be started before.
 * For TCP does nothing.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
 */
int comHangWait(void) {
    trcS("comHangWait\n");
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
          return comHangWaitMdm();
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
      case chnSsl5100:
      case chnSsl7780:
          return comHangWaitIp();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSendCom(byte b) {
#ifdef __CHN_COM__
    int ret;

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);
    ret = comSendByte(com.prm.mdm.hdl, b);  //just send it
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comSendIp(byte b) {
#ifdef __CHN_TCP__
    int ret;

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs);
#ifdef __CHN_PPP__
    VERIFY(com.prm.tcp.sck == com.prm.ppp.sck);
#endif
    ret = send(com.prm.tcp.sck, &b, 1, 0);
    CHECK(ret == 1, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comSendSsl(byte b) {
#ifdef __CHN_SSL__
    int ret;

    VERIFY(com.chn == chnSsl5100 || com.chn == chnSsl7780);

    ret = sslWrite(com.prm.ssl.session, &b, 1);
    CHECK(ret == 1, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

/** Send a byte b to the communication channel.
 * \param b (I) Byte to send
 * \return
 *    - negative if failure.
 *    - zero if timeout; otherwise positive.
 * \header sys\\sys.h
 * \source sys\\chn.c
 */
int comSend(byte b) {
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return comSendCom(b);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comSendIp(b);
      case chnSsl5100:
      case chnSsl7780:
          return comSendSsl(b);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comRecvCom(byte * b, int dly) {
#ifdef __CHN_COM__
    int ret;
    tResponse rsp;

    VERIFY(b);
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);

    ret = comReceiveByteReq(com.prm.mdm.hdl);   //send receive request
    //CHECK(ret == RET_RUNNING, lblKO);

    ret = psyPeripheralResultWait(COM_WAIT, dly * 100, 0);  //wait for result
    if(ret == COM_TIMEOUT)
        return 0;               //in case of timeout the ret code is not COM_TIMEOUT! it is 2=RET_RUNNING
    CHECK(ret >= 0, lblKO);

    ret = comResultGet(com.prm.mdm.hdl, COM_RECEIVE_EVENT, sizeof(rsp), &rsp);  //get it
    trcFN("comResultGet ret=%d\n", ret);
    CHECK(ret == RET_OK, lblKO);
    CHECK(rsp.len == 1, lblKO);
    *b = rsp.buf[0];

    return 1;
  lblKO:
    trcFN("comRecvCom ret=%d\n", ret);
    return -1;
#else
    return -1;
#endif
}

static int comRecvIp(byte * b, int dly) {
#ifdef __CHN_TCP__
    int ret = 0;

    VERIFY(b);
    VERIFY(com.chnRole == chnRoleSrv || com.chnRole == chnRoleClt);

    switch (com.chnRole) {
      case chnRoleSrv:
          ret = recv(com.prm.tcp.srvSck, b, 1, 0);
          break;
      case chnRoleClt:
          {
              struct timeval tim;

              tim.tv_sec = dly;
              tim.tv_usec = 0;

              setsockopt(com.prm.tcp.sck, SOL_SOCKET, SO_RCVTIMEO, &tim,
                         sizeof(tim));
              ret = read(com.prm.tcp.sck, b, 1);
              break;
          }
    }
    return ret;
#else
    return -1;
#endif
}

static int comRecvUSB(byte * b, int dly) {  // same as comRecvCom
#ifdef __CHN_USB__
    int ret;
    tResponse rsp;

    VERIFY(b);

    ret = comReceiveByteReq(com.prm.usb.hdl);   //send receive request
    CHECK(ret == RET_RUNNING, lblKO);

    if(dly != 0)
        ret = psyPeripheralResultWait(COM_WAIT, dly * 100, 0);  //wait for result
    else
        ret = psyPeripheralResultWait(COM_WAIT, 1, 0);

    if(ret == COM_TIMEOUT || ret == RET_RUNNING)
        return 0;               //in case of timeout the ret code is not COM_TIMEOUT! it is 2=RET_RUNNING
    CHECK(ret >= 0, lblKO);

    ret = comResultGet(com.prm.usb.hdl, COM_RECEIVE_EVENT, sizeof(rsp), &rsp);  // get it
    CHECK(ret == RET_OK && rsp.len == 1, lblKO);
    *b = rsp.buf[0];

    return 1;
  lblKO:
    trcFN("comRecvUSB ret=%d\n", ret);
    return -1;
#else
    return -1;
#endif
}

static int comRecvSsl(byte * b, int dly) {
#ifdef __CHN_SSL__
    int ret = 0;

    VERIFY(b);
    VERIFY(com.chn == chnSsl5100 || com.chn == chnSsl7780);

    ret = sslRead(com.prm.ssl.session, b, 1);
    CHECK(ret >= 0, lblKO);

    return ret;
  lblKO:
    trcFN("comRecvSsl ret=%d\n", ret);
    return -1;
#else
    return -1;
#endif
}

/** Receive a byte into b to the communication channel using a timeout value dly.
 * \param b (I) Pointer to a buffer to receive a byte
 * \param dly (I) Timeout in seconds
 * \return
 *    - negative if failure.
 *    - zero if timeout.
 *    - otherwise positive.
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
 */
int comRecv(byte * b, int dly) {
    VERIFY(b);
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return comRecvCom(b, dly);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comRecvIp(b, dly);
      case chnUSB:
          return comRecvUSB(b, dly);
      case chnSsl5100:
      case chnSsl7780:
          return comRecvSsl(b, dly);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSendBufCom(const byte * msg, word len) {
#ifdef __CHN_COM__
    int ret;

    ret = comSendMsgWait(com.prm.mdm.hdl, len, (byte *) msg, 60 * 100);
    CHECK(ret == RET_OK, lblKO);

    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSendBufCom ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSendBufIp(const byte * msg, word len) {
#ifdef __CHN_TCP__
    int ret = 0;

    switch (com.chnRole) {
      case chnRoleSrv:
          ret = send(com.prm.tcp.srvSck, msg, len, 0);
          break;
      case chnRoleClt:
          ret = send(com.prm.tcp.sck, msg, len, 0);
          break;
      default:
          break;
    }
    CHECK(ret == len, lblKO);
    ret = len;
    goto lblEnd;
  lblKO:
    ret = socketerror();
    trcErr(ret);
  lblEnd:
    trcFN("comSendBufIp ret = %d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSendBufSsl(const byte * msg, word len) {
#ifdef __CHN_SSL__
    int ret = 0;

    ret = sslWrite(com.prm.ssl.session, &msg, len);
    CHECK(ret == len, lblKO);
    ret = len;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comSendBufIp ret = %d\n", ret);
    return ret;
#else
    return -1;
#endif
}

/** Send len  bytes from  msg to the communication port.
 * \param msg (I) Pointer to buffer to send
 * \param len (I) Number of bytes to send
 * \return
 *    - negative if failure.
 *    - zero if timeout.
 *    - otherwise positive.
 * \header sys\\sys.h
 * \source sys\\chn.c
 */
int comSendBuf(const byte * msg, word len) {
    VERIFY(msg);

    trcFS("comSendBuf = %s\n", (char *) msg);
    trcS("bin: ");
    trcBN(msg, len);
    trcS("\n");

    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return comSendBufCom(msg, len);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comSendBufIp(msg, len);
      case chnSsl:
          return comSendBufSsl(msg, len);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comRecvBufCom(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_COM__
    int ret;
    uint8 ctl[1 + 16];
    tResponse rsp;
    word len;
    word sec;

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);

    memset(ctl, 0, 1 + 16);
    if(trm) {
        *ctl = strlen((char *) trm);
        strcpy((char *) ctl + 1, (char *) trm); //zero is added at the end
        if(msg)
            len = bufDim(msg);
    } else {
        VERIFY(com.chn == chnHdlc || com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);   //in HDLC mode there is no terminators usually
    }
    sec = 1;
    if(dly)
        sec = dly * 100;
    ret =
        comReceiveMsgWait(com.prm.mdm.hdl, RSP_LEN, ctl, sec, sizeof(rsp),
                          &rsp);
    CHECK(ret == RET_OK, lblKO);

    if(msg) {
        ret = bufApp(msg, (byte *) rsp.buf, rsp.len);
        CHECK(ret >= 0, lblKO);
    }

    ret = rsp.len;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("comRecvBufCom ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comRecvBufGprs(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_GPRS__
    int ret;

    VERIFY(msg);
    bufReset(msg);

    tmrStart(0, dly * 100);

    switch (com.chnRole) {
      case chnRoleSrv:
          ret = recv(com.prm.tcp.srvSck, (void *) bufPtr(msg), bufDim(msg), 0);
          break;
      case chnRoleClt:
          {
              struct timeval tim;

              tim.tv_sec = dly;
              tim.tv_usec = 0;
              setsockopt(com.prm.tcp.sck, SOL_SOCKET, SO_RCVTIMEO, &tim,
                         sizeof(tim));
              ret = read(com.prm.tcp.sck, (void *) bufPtr(msg), bufDim(msg));
              break;
          }

      default:
          ret = -1;
          break;
    }

    if(ret > 0) {
        msg->pos = (word) ret;
    }

    CHECK(tmrGet(0), lblKO);

    goto lblEnd;
  lblKO:
    trcFN("comRecvBufGprs ret=%d\n", ret);
    ret = -1;
  lblEnd:
    tmrStop(0);
    return ret;
#else
    return -1;
#endif
}

static int comRecvBufIp(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_TCP__
    int ret;
    word dim, idx;
    byte b;

    tmrStart(0, dly * 100);
    dim = 0;
    while(tmrGet(0)) {
        if(dim >= bufDim(msg))
            break;
        ret = comRecvIp(&b, 1);
        trcFN("comRecvIp ret=%d", ret);
        trcFN(" b=%02x\n", b);
        if(ret != 1) {
            if(!dim)            //first byte
                continue;
            else
                break;
        }
        if(msg) {
            ret = bufSet(msg, b, 1);
            CHECK(ret >= 0, lblKO);
        }
        dim++;
        if(!trm)
            continue;
        idx = 0;
        while(trm[idx]) {       //is it a terminator character
            if(b == trm[idx])
                break;
            idx++;
        }
        if(b == trm[idx])
            break;
    }
    CHECK(tmrGet(0), lblKO);
    //trcS("comRecvBufIp msg= ");
    //trcBAN(bufPtr(msg),dim);
    //trcS("\n");
    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comRecvBufIp ret=%d\n", ret);
    tmrStop(0);
    return ret;
#else
    return -1;
#endif
}

static int comRecvBufUSB(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_USB__
    int ret = 0;
    word dim, idx;
    byte b;

    VERIFY(trm);

    comCancel(com.prm.usb.hdl, PER_ALL_EVENTS);

    tmrStart(0, dly * 100);
    dim = 0;
    while(tmrGet(0)) {
        ret = comRecvUSB(&b, 1);
        if(ret != 1)
            continue;
        if(msg) {
            ret = bufSet(msg, b, 1);
            CHECK(ret >= 0, lblKO);
        }
        dim++;
        idx = 0;
        while(trm[idx]) {       //is it a terminator character
            if(b == trm[idx])
                break;
            idx++;
        }
        if(b == trm[idx])
            break;
    }
    CHECK(tmrGet(0), lblKO);
    ret = dim;
    goto lblEnd;
  lblKO:
    trcFN("comRecvBufUSB ret=%d\n", ret);
    ret = -1;
  lblEnd:
    tmrStop(0);
    comReceptionFlush(com.prm.usb.hdl);
    return ret;
#else
    return -1;
#endif
}

static int comRecvBufSsl(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_SSL__
    int ret;
    word dim, idx;
    byte b;

    tmrStart(0, dly * 100);
    dim = 0;
    while(tmrGet(0)) {
        if(dim >= bufDim(msg))
            break;
        ret = comRecvSsl(&b, 1);
        trcFN("comRecvSsl ret=%d", ret);
        trcFN(" b=%02x\n", b);
        if(ret != 1) {
            if(!dim)            //first byte
                continue;
            else
                break;
        }
        if(msg) {
            ret = bufSet(msg, b, 1);
            CHECK(ret >= 0, lblKO);
        }
        dim++;
        if(!trm)
            continue;
        idx = 0;
        while(trm[idx]) {       //is it a terminator character
            if(b == trm[idx])
                break;
            idx++;
        }
        if(b == trm[idx])
            break;
    }
    CHECK(tmrGet(0), lblKO);

    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comRecvBufSsl ret=%d\n", ret);
    tmrStop(0);
    return ret;
#else
    return -1;
#endif
}

/** Receive a sequence of bytes terminated by a character from trm  into buffer msg
 * to the communication channel using a timeout value dly.
 * The terminator control string consists of characters that can indicate the end of message
 * (ETX, EOT etc).
 * \param msg (O) Pointer to buffer to accept characters received; should be large enough
 * \param trm (I) zero-terminated string of terminator control characters
 * \param dly (I) timeout in seconds
 * \return
 *    - negative if failure.
 *    - zero if timeout.
 *    - otherwise positive.
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0008.c
 */
int comRecvBuf(tBuffer * msg, const byte * trm, byte dly) {
    int ret;

    trcFN("comRecvBuf dly=%d\n", dly);
    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
      case chnCom1:
      case chnCom2:
      case chnCom3:
          ret = comRecvBufCom(msg, trm, dly);
          break;
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
          ret = comRecvBufIp(msg, trm, dly);
          break;
      case chnGprs:
          ret = comRecvBufGprs(msg, trm, dly);
          break;
      case chnUSB:
          ret = comRecvBufUSB(msg, trm, dly);
      case chnSsl:
          ret = comRecvBufSsl(msg, trm, dly);
      default:
          VERIFY(com.chn < chnEnd);
          ret = -1;
          break;
    }

    if(ret < 0)
        return ret;

    if(msg) {
        trcFS("comRecvBuf msg=%s\n", (char *) bufPtr(msg));
        trcBN(bufPtr(msg), bufLen(msg));
        trcS("\n");
    } else if(trm) {
        trcFS("comRecvBuf trm=%s\n", (char *) trm);
        trcBN(trm, strlen((char *) trm));
        trcS("\n");
    }
    return ret;
}

#endif                          //ifndef __CMM__

static uint32 sem[3];           //semaphore IDs
static uint32 tskM;             //main task ID
static uint32 tskS;             //secondary task ID

#define UTA_TASK_NAME "T0"
#define UTA_SEM_NAME0 "S0"
#define UTA_SEM_NAME1 "S1"
#define UTA_SEM_NAME2 "S2"

/** Start a secondary task performed by the function fun with the argument arg.
 * \param fun (I) Function to be performed as a secondary task 
 * \param arg (I) An argument to be transferred to the function fun
 * \return
 *    - negative if failure.
 *    - otherwise secondary task identifier.
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0056.c
*/
int tskStart(void (*fun) (card), card arg) {
    int ret;

//        static idx= 0;

    VERIFY(fun);
    trcS("tskStart Beg\n");

//  if(!idx){
    ret = psySemaphoreCreate(UTA_SEM_NAME0, 0, PSY_SM_FIFO, &sem[0]);
    CHECK(ret == RET_OK, lblKO);
    ret = psySemaphoreCreate(UTA_SEM_NAME1, 0, PSY_SM_FIFO, &sem[1]);
    CHECK(ret == RET_OK, lblKO);
    ret = psySemaphoreCreate(UTA_SEM_NAME2, 0, PSY_SM_FIFO, &sem[2]);
    CHECK(ret == RET_OK, lblKO);
//                idx++;
//        }

    ret = psyTaskIdent(0, &tskM);
    CHECK(ret == RET_OK, lblKO);

    ret = psyTaskCreate(UTA_TASK_NAME, fun, &tskS, 0, arg);
    CHECK(ret == RET_OK, lblKO);

    psyTimerWakeAfter(100);

    goto lblEnd;
  lblKO:
    return -1;
    goto lblEnd;
  lblEnd:
    trcFN("tskStart ret= %d\n", ret);
    return (int) tskS;
}

/** Stop the secondary task. Should be called at the end of secondary task function.
 * \return no
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0056.c
*/
void tskStop(void) {
    trcS("tskStop\n");
    psySemaphoreDelete(sem[0]);
    psySemaphoreDelete(sem[1]);
    psySemaphoreDelete(sem[2]);
    psyTaskDelete();
    psyTimerWakeAfter(100);
}

/** Synchronize tasks. Implemented only for TELIUM. Empty for UNICAPT (for compatibility)
 * \return no
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0056.c
*/
void tskSync(void) {
    psyTimerWakeAfter(10);
}

/** A wrapper around semaphore acquiring function.
 * Puts the current task into waiting state until a parallel task calls semDec function.
 * \param idx (I) Semaphore index, 0..2
 * \return
 *    - negative if failure.
 *    - otherwise semaphore identifier.
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0056.c
*/
int semInc(byte idx) {
    int ret;

    trcFN("semInc Beg idx= %d\n", (int) idx);
    VERIFY(idx < 3);
    ret = psySemaphoreAcquire(sem[idx], PSY_INFINITE_TIMEOUT);
    CHECK(ret == RET_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("semInc ret= %d\n", ret);
    return (int) sem[idx];
}

/** A wrapper around semaphore releasing function.
 * Allows parallel task to continue processing.
 * \param idx (I) Semaphore index, 0..2
 * \return
 *    - negative if failure.
 *    - otherwise semaphore identifier.
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0056.c
*/
int semDec(byte idx) {
    int ret;

    trcFN("semDec Beg idx= %d\n", (int) idx);
    VERIFY(idx < 3);
    ret = psySemaphoreRelease(sem[idx]);
    CHECK(ret == RET_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("semDec ret= %d\n", ret);
    return (int) sem[idx];
}

#ifdef __CHN_COM__
/** Transfer control over the com handle among main and secondary tasks.
 * If ctl is equal to 1 the control is transferred from main task to secondary task;
 * otherwise it is transferred in an opposite direction.
 * \param ctl (I) Control value indicating the control transfer direction, 0 or 1
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcab0058.c
*/
int tskComCtl(byte ctl) {
//There is no need to perform these manipulations in version 2.5.1
//Use COM_CTRL_NO_TASK_CTRL
    int16 ret;
    comControlRequests_t req;
    comControlReqCom_t comReq;
    comControlResults_t res;
    comControlResCom_t comRes;

    VERIFY(com.prm.mdm.hdl);

    trcFN("tskComCtl ctl=%d\n", (int) ctl);
    memset(&req, 0, sizeof(req));
    memset(&comReq, 0, sizeof(comReq));
    memset(&res, 0, sizeof(res));
    memset(&comRes, 0, sizeof(comRes));

    req.type = COM_CONTROL_COM;
    req.level = COM_CONTROL_LEVEL_PROTOCOL;
    req.req.CtrlCom = &comReq;
    res.res.CtrlCom = &comRes;
    comReq.ioCmd = COM_CTRL_CHANGE_OWNER;
    comReq.req.newId.newPid = getPid();
    if(ctl) {
        comReq.req.newId.newTask = tskS;
        ret = comControl(com.prm.mdm.hdl, &req, &res);
    } else {
        comReq.req.newId.newTask = tskM;
        ret = comControl(com.prm.mdm.hdl, &req, &res);
    }
    CHECK(ret == RET_OK, lblKO);
    goto lblEnd;
  lblKO:
    return -1;
    goto lblEnd;
  lblEnd:
    trcFN("tskComCtl ret= %d\n", ret);
    return ret;
}
#endif

#ifdef __CHN_FTP__
static uint32 hFtp = 0;

/** Open ftp channel.
 * The IP connection should be established before starting FTP.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0111.c
*/
int ftpStart(void) {
    int ret;

    trcS("ftpStart\n");
#ifdef __FTPSIM__
    ret = 1;
    if(ret)
        goto lblEnd;
#endif

    hFtp = -1;
    ret = ftpOpen(&hFtp);
    CHECK(ret == RET_OK, lblKO);

    //ret= ftpType(hFtp,FTP_BINARY);
    //CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("ftpOpen error = %d\n", ret);
    ret = -1;

    (void) ftpClose(hFtp);

  lblEnd:
    trcFN("ftpStart ret = %d\n", ret);
    return ret;
}

/** Disconnect and close ftp channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0111.c
*/
int ftpStop(void) {
    trcS("ftpStop\n");

#ifdef __FTPSIM__
    if(hFtp != -2)
        return 1;
#endif

    if(hFtp != -1)              // ftp session is open
    {
        (void) ftpDisconnect(hFtp);
        (void) ftpClose(hFtp);
        hFtp = -1;
    }

    return 1;
}

/** Connect to a remote FTP server.
 * \param srv (I) String specifying the address of ftp server.
 * \param usr (I) user name to use in the connection phase to the remote server. NULL if not used.
 * \param pwd (I) password to use in the connection phase to the remote server. NULL if not used.
 * \param mod (I) connection mode:
 *  - 'a': active (port 22)
 *  - 'p': passive (port is defined by server)
 * If mod is zero it means try active, in case of failure try passive
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0111.c
*/
int ftpDial(const char *srv, const char *usr, const char *pwd, char mod) {
    int ret;

    trcFN("ftpDial mod=%c\n", mod);
    VERIFY(srv);

#ifdef __FTPSIM__
    ret = 1;
    if(ret)
        goto lblEnd;
#endif

    ret = -1;
    if(mod != 'p')
        ret =
            ftpConnect(hFtp, (char *) srv, (char *) usr, (char *) pwd,
                       FTP_ACTIVE, 0);

    if(ret != RET_OK && mod == 0)
        mod = 'p';

    if(mod == 'p')
        ret =
            ftpConnect(hFtp, (char *) srv, (char *) usr, (char *) pwd,
                       FTP_PASSIVE, 0);
    CHECK(ret == RET_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("ftpConnect error = %d\n", ret);
    ret = -1;

  lblEnd:
    trcFN("ftpDial ret = %d\n", ret);
    return ret;
}

/** Change the current directory on the remote file system.
 * \param dir (I) The new directory name
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0111.c
*/
int ftpCD(const char *dir) {
    int ret;

    VERIFY(dir);
    trcFS("ftpCD dir = %s\n", dir);

#ifdef __FTPSIM__
    ret = 1;
    if(ret)
        goto lblEnd;
#endif

    ret = ftpChDir(hFtp, (char *) dir);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("ftpChDir error = %d\n", ret);
    ret = -1;

  lblEnd:
    trcFN("ftpCD ret = %d\n", ret);
    return ret;
}

#ifdef __FTPSIM__
static void ftpSimFileName(char *str) {
    uint16 len;
    uint32 ofs;
    char tmp[256];

    len = strlen(str);
    ofs = 0;
    if(len > 8 + 1 + 3)
        ofs = len - 8 - 1 - 3;
    strcpy(tmp, str + ofs);
    memcpy(tmp, "sim", 3);
    strcpy(str, tmp);
}
#endif
/** Gets a file from the remote FTP file system and stores it locally in the non volatile memory.
 * \param sec (I) The nvm section where to retrieve the file
 * \param file (I) The remote file name
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0111.c
*/
int ftpGet(byte sec, const char *file) {
    int ret;
    char buf[8 + 1 + 3 + 1];

    VERIFY(file);
    trcFS("ftpGet file=%s\n", file);

    nvmFileName(buf, sec);

#ifdef __FTPSIM__
    {
        byte b;
        uint16 len;
        uint32 ofs;
        uint32 hdl;
        char sim[256];

        strcpy(sim, file);
        ftpSimFileName(sim);
        trcFS("ftpGet sim=%s\n", sim);
        hdl = psyFileOpen(sim, "r");
        ofs = 0;
        nvmRemove(sec);
        while(42) {
            len = 1;
            b = 0;
            ret = psyFileRead(hdl, &b, &len);
            if(ret != RET_OK)
                break;
            nvmSave(sec, &b, ofs++, 1);
        }
        psyFileClose(hdl);

        ret = 1;
        if(ret)
            goto lblEnd;
    }
#endif

    ret = ftpRetrieve(hFtp, buf, (char *) file);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("ftpRetrieve error = %d\n", ret);
    ret = -1;

  lblEnd:
    trcFN("ftpGet ret=%d\n", ret);
    return ret;
}

/** Puts a section from a non volatile memory to a file on the remote FTP file system.
 * \param sec (I) The nvm section to be sent to the remote server
 * \param file (I) The remote file name
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcab0111.c
 */
int ftpPut(byte sec, const char *file) {
    int ret;
    char buf[8 + 1 + 3 + 1];

    VERIFY(file);
    trcFS("ftpPut file = %s\n", file);

    nvmFileName(buf, sec);

#ifdef __FTPSIM__
    {
        char sim[256];

        strcpy(sim, file);
        ftpSimFileName(sim);
        psyFileRename(buf, sim);
        ftpGet(sec, sim);

        ret = 1;
        if(ret)
            goto lblEnd;
    }
#endif

    ret = ftpStore(hFtp, buf, (char *) file);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("ftpStore error = %d\n", ret);
    ret = -1;

  lblEnd:
    trcFN("ftpGet ret = %d\n", ret);
    return ret;
}

#endif
