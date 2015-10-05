/** \file
 * Communications channels implementation using GCL (Generic COmmunications Library)
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/gcl.c $
 *
 * $Id: gcl.c 2322 2009-04-22 11:50:57Z abarantsev $
 */

#include <string.h>
#include <stdlib.h>
#include <unicapt.h>
#include "sys.h"

#ifdef __CHN_GPRS__
#undef __CHN_GSM__
#define __CHN_GSM__
#endif

#ifdef __CHN_PPP__
#undef __CHN_TCP__
#define __CHN_TCP__
#endif

#include "gcl.h"

#ifdef __CHN_COM__
#include "gcl_asyn.h"
#endif
#ifdef __CHN_HDLC__
#include "gcl_hdlc.h"
#endif
#ifdef __CHN_PPP__
#include "gcl_ppp.h"
#endif
#ifdef __CHN_GSM__
#include "gcl_gsm.h"
#endif
#ifdef __CHN_GPRS__
#include "gcl_gprs.h"
#endif
#ifdef __CHN_TCP__
#include "gcl_eth.h"
#endif
#ifdef __CHN_SCK__
#include "gcl_sock.h"
#include "gcl_pgsup.h"
#endif
#ifdef __CHN_SERIAL__
#include "gcl_serial.h"
#endif
#ifdef __CHN_WIFI__
#include "gcl_wifi.h"
#endif
#ifdef __CHN_SSL__
#include "gcl_ssl.h"
#endif

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcGCL))

typedef struct sGclCfg {
    byte chn;                   //communications channel used
#ifdef __CHN_COM__
    gclTelephone_t tel;
    gclAsynTelephone_t asy;
#endif
#ifdef __CHN_ISDN__
    char phoneNumber[30];
#endif
#ifdef __CHN_TCP__
    gclEth_t eth;
#endif
#ifdef __CHN_SCK__
    gclSock_t sck;
#endif
#ifdef __CHN_WIFI__
    gclWiFi_t wifi;
#endif
#ifdef __CHN_PPP__
    gclPPP_t ppp;
#endif
#ifdef __CHN_GSM__
    gclGSM_t gsm;
    char *pin;                  //pin of simcard
#endif
#ifdef __CHN_GPRS__
    gclGPRS_t gprs;
#endif
#ifdef __CHN_SERIAL__
    gclSerial_t serial;
#endif
#ifdef __CHN_SSL__
    gclSSL_t ssl;
    byte isSSL;
#endif
    uint32 rcvDly;              //receiving timeout    
    const byte *rcvTrm;         //terminators string
} tGclCfg;
static tGclCfg cfg;

typedef struct {
    uint32 timeOut;
} tGclCommCfg;
static tGclCommCfg cfgComm;

#define CHK CHECK(ret==RET_OK,lblKO)
static int16 nullCallBack(gclConfig_t * gcl) {
    return RET_OK;
}
static int16 connErrorCallBack(gclConfig_t * gcl) {
    gclSetError(0);
    return gclPostMessage(GCL_RETRY);
}

#ifdef __CHN_RADIO__
static int setRadioMode(void) {
    int ret;
    uint32 value;
    uint32 radHandle;

    ret = radOpen("DEFAULT", &radHandle);
    CHECK(ret == RET_OK, lblKO);
    value = RAD_MODE_RADIO;
    ret = radControl(radHandle, RAD_SET_MODE, RAD_SET_MODE_SIZE, &value);
    CHECK(ret == RET_OK, lblKO);
    goto lblEnd;
  lblKO:
    radClose(radHandle);
    if(ret >= 0)
        ret = -1;
    return ret;
  lblEnd:
    radClose(radHandle);
    psyTimerWakeAfter(100);
    return 1;
}
#endif

#define INIT_LEN 1024
static int getTok(char *tok, const char *buf, char sep) {   //parse a token from the buffer separated by sep
    int idx = 0;

    while(*buf) {
        if(*buf == sep)
            break;
        *tok++ = *buf++;
        idx++;
        VERIFY(idx <= INIT_LEN);
    }
    *tok = 0;
    return idx;
}

//parse a token of type ".?8N12400" that includes:
// optional data compression character '.' (default - no data compression)
// optional error correction character '?' (default - no error correction)
// optional switch off detect dial tone '!'(default - detect dial tone)
// optional tone/pulse dialing/ negotiation (default - tone, no negotiation)
// mandatory number of data bits
// mandatory parity: 'N', 'E', 'O'
// mandatory number of stop bits
// mandatory baud rate
#if defined __CHN_COM__ | defined __CHN_HDLC__
static int getMdmTok(const char *init) {
    int ret, len;
    char buf[INIT_LEN + 1];
    const char *tok;
    byte fast;                  //does not negotiate baud rate?
    enum comModemModulation_t modulationType;   //V22,V22B,V32,V32B,...
    enum comBps_t bps;

    ret = getTok(buf, init, '|');
    CHECK(ret > 0, lblKO);      //retrieve the modem parameters token from initialisation string
    len = ret;
    tok = buf;

    //treat optional data compression character '.'
    cfg.tel.dataCompression = COM_COMPRESSION_NONE;
    if(*tok == '.') {
        cfg.tel.dataCompression = COM_COMPRESSION_MNP5;
        tok++;
    }
    //treat optional error correction character '?'
    cfg.tel.errorCorrection = COM_CORRECTION_NONE;
    if(*tok == '?') {
        cfg.tel.errorCorrection = COM_CORRECTION_MNP;
        tok++;
    }
    //treat optional optional detect dial tone character 'D'

    if(*tok == '!') {
        cfg.tel.detectDialTone = FALSE;
        tok++;
    } else {
        cfg.tel.detectDialTone = TRUE;
    }

    //treat optional optional tone/pulse dialing/ negotiation
    switch (*tok) {             //lowercase means baud rate negotiation
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
    cfg.tel.useToneDialing = TRUE;
    switch (*tok) {
      case 'T':
      case 't':
          cfg.tel.useToneDialing = TRUE;
          tok++;
          break;
      case 'P':
      case 'p':
          cfg.tel.useToneDialing = FALSE;
          tok++;
          break;
      default:                 //if missing, don't increment control string
          break;
    }

    //treat mandatory number of data bits
    cfg.asy.AsyncdataSize = COM_DATASIZE_8;
    switch (*tok++) {           //number of data bits
      case '7':
          cfg.asy.AsyncdataSize = COM_DATASIZE_7;
          break;
      case '8':
          cfg.asy.AsyncdataSize = COM_DATASIZE_8;
          break;
      default:
          goto lblKO;
    }

    //treat mandatory parity
    cfg.asy.Asyncparity = COM_PARITY_NONE;
    switch (*tok++) {           //parity
      case 'N':
          cfg.asy.Asyncparity = COM_PARITY_NONE;
          break;
      case 'E':
          cfg.asy.Asyncparity = COM_PARITY_EVEN;
          break;
      case 'O':
          cfg.asy.Asyncparity = COM_PARITY_ODD;
          break;
      default:
          goto lblKO;
    }

    //treat mandatory number of stop bits
    cfg.asy.AsyncstopBits = COM_STOP_NO_CHANGE;
    switch (*tok++) {           //number of stop bits
      case '0':
          cfg.asy.AsyncstopBits = COM_STOP_NO_CHANGE;
          break;
      case '1':
          cfg.asy.AsyncstopBits = COM_STOP_1;
          break;
      case '2':
          cfg.asy.AsyncstopBits = COM_STOP_2;
          break;
      default:
          goto lblKO;
    }

    //treat mandatory baud rate
    modulationType = COM_MODEM_MODULATION_DEFAULT;
    bps = COM_BPS_DEFAULT;
    if(strcmp(tok, "1200") == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ?
                                         COM_MODEM_MODULATION_V22_REQ :
                                         COM_MODEM_MODULATION_V22);
        bps = (enum comBps_t) (fast ? COM_BPS_1200 : COM_BPS_DEFAULT);
    } else if(strcmp(tok, "2400") == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ?
                                         COM_MODEM_MODULATION_V22B_REQ :
                                         COM_MODEM_MODULATION_V22B);
        bps = (enum comBps_t) (fast ? COM_BPS_2400 : COM_BPS_DEFAULT);
    } else if(strcmp(tok, "9600") == 0) {
        modulationType =
            (enum comModemModulation_t) (fast ?
                                         COM_MODEM_MODULATION_V32_REQ :
                                         COM_MODEM_MODULATION_V32);
        bps = (enum comBps_t) (fast ? COM_BPS_9600 : COM_BPS_DEFAULT);
    } else
        goto lblKO;
    cfg.tel.modulationType = modulationType;
    cfg.tel.minSpeed = bps;
    cfg.tel.maxSpeed = bps;

    return len;
  lblKO:
    return -1;
}

//parse a token of type "0-00-123-12345678" that includes:
// optional PABX
// optional prefix
// optional country code
// mandatory phone number
static int getPhoneTok(const char *init) {
    int ret, len;
    char buf[INIT_LEN + 1];
    char tmp[INIT_LEN + 1];
    const char *tok;
    const char *ptr;
    static char pabx[4 + 1];
    static char prefix[4 + 1];
    static char country[4 + 1];
    static char phone[16 + 1];

    *pabx = 0;
    *prefix = 0;
    *country = 0;

    ret = getTok(buf, init, '|');   //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);

    len = ret;
    tok = buf;
    ptr = buf;

    ret = getTok(tmp, ptr, '-');    //retrieve optional PABX
    CHECK(ret >= 0, lblKO);
    if(ret < (int) strlen(buf)) {   //extract pabx if found
        strcpy(pabx, tmp);
        strcat(pabx, ",");
        ptr += ret;
        VERIFY(*ptr == '-');
        ptr++;
    }

    ret = getTok(phone, ptr, '-');
    CHECK(ret > 0, lblKO);      //retrieve mandatory phone

    if(cfg.chn == gclPpp) {
#ifdef __CHN_PPP__
        cfg.ppp.prefix = pabx;
        cfg.ppp.phoneNumber = phone;
#endif
    } else if(cfg.chn == gclGsm) {
#ifdef __CHN_GSM__
        cfg.gsm.phoneNumber = phone;
#endif
    } else {
#ifdef __CHN_COM__
        // PSTN async & HDLC
        cfg.tel.prefix = pabx;  //"0,";
        cfg.tel.phoneNumber = phone;    //"0860922000";
#endif
    }
    return len;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_TCP__
static int getLoginTok(const char *init) {
    int ret, len;
    static char login[16 + 1];
    static char password[16 + 1];

    *login = 0;
    *password = 0;

    ret = getTok(login, init, '|'); //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    ret = getTok(password, init, '|');  //retrieve mandatory phone
    CHECK(ret > 0, lblKO);

    if(cfg.chn == gclPpp) {
#ifdef __CHN_PPP__
        cfg.ppp.loginName = login;
        cfg.ppp.password = password;
#endif
    }

    if(cfg.chn == gclGprs) {
#ifdef __CHN_GPRS__
        cfg.gprs.loginName = login; //"danfr";
        cfg.gprs.password = password;   //"mq36noqt";
#endif
    }

    len += ret;
    return len;
  lblKO:
    return -1;

}
#endif

#if (defined __CHN_TCP__) || (defined __CHN_SCK__)

/** parse a token of type 

"192.1.1.253|1000|255.255.255.0|192.1.1.1|192.1.1.167" that includes:
 * \param IP (I) Remote IP Address 
 * \param Port (I) Remote Port
 * \param mask (I) Network Mask
 * \param gateway (I) Network Gateway
 * \param locIP (I) Static Local IP Address of terminal
 * \return  
 *    - negative if failure. 
 *    - otherwise positive(equal to length of the input string).
 * \test tcap0021.c
 * \test tcfa0005.c
*/
int getTCPIPTok(const char *init) {
    int ret, len, size;
    static char IP[16 + 1] = "";
    static char Port[5 + 1] = "";
    static char mask[16 + 1] = "";
    static char gateway[16 + 1] = "";
    static char locIP[16 + 1] = ""; //Local Static IP Address for No DHCP    
    card Dynamic = 1;

    *IP = 0;
    *Port = 0;
    len = 0;
    size = strlen(init);

    if(*init) {                 //Remote IP Address - Remote Port- Mask - Gateway - Local IP Address

        ret = getTok(IP, init, '|');    //retrieve the host network IP parameters token from initialisation string
        CHECK(ret > 0, lblKO);
        len += ret;

        init += ret;            //skip token
        VERIFY(*init == '|');
        init++;
        len++;

        ret = getTok(Port, init, '|');  //retrieve mandatory port number
        CHECK(ret > 0, lblKO);
        len += ret;

        if((init[ret] == '|') && (len + 1 == size))
            size = size - 1;
        if(len != size)         //If len equals size means the connection is Dynamic, otherwise it is Static.
        {
            init += ret;        //skip token
            VERIFY(*init == '|');
            init++;
            len++;

            ret = getTok(mask, init, '|');  //retrieve mandatory subnet mask
            CHECK(ret > 0, lblKO);
            len += ret;

            init += ret;        //skip token
            VERIFY(*init == '|');
            init++;
            len++;

            ret = getTok(gateway, init, '|');   //retrieve mandatory gateway
            CHECK(ret > 0, lblKO);
            len += ret;

            init += ret;        //skip token
            VERIFY(*init == '|');
            init++;
            len++;

            ret = getTok(locIP, init, '|'); //retrieve mandatory locIP
            CHECK(ret > 0, lblKO);
            len += ret;

            Dynamic = 0;
        }
    }

    switch (cfg.chn) {
#ifdef __CHN_PPP__
      case gclPpp:
          cfg.ppp.ipAddress = IP;
          cfg.ppp.tcpPort = Port;
          break;
#endif
#ifdef __CHN_GPRS__
      case gclGprs:
          cfg.gprs.ipAddress = IP;
          cfg.gprs.tcpPort = Port;
          break;
#endif
#ifdef __CHN_TCP__
      case gclEth:
          cfg.eth.ipAddress = IP;
          cfg.eth.tcpPort = Port;

          //Following three lines are added to support static IP (when no DHCP is supported)
          if(Dynamic == 0) {
              cfg.eth.subNetworkMask = mask;
              cfg.eth.defaultDefGateway = gateway;
              cfg.eth.defaultLocalIpAddress = locIP;
          }

          cfg.eth.useDhcp = Dynamic;

          //The following 2 lines are added when we faced a problem in SAMA. If gclSend fails to transmit the message (for example the ethernet cable is out), it always returns positive but the message is kept inside the terminal and is sent some other time when the connection is available. Even if we close GCL , still the message stays in the terminal and will be sent later on.
          cfg.eth.tcpNoDelay = 1;
          cfg.eth.soLinger = 1;

          //Set communication timeout
          cfg.eth.communicationTimeout = cfgComm.timeOut;

          break;
#endif
#ifdef __CHN_SCK__
      case gclSck:
          cfg.sck.ipAddress = IP;
          cfg.sck.tcpPort = Port;
          break;
#endif
      default:
          goto lblKO;
    }
    return len;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_WIFI__
static int getWifiTok(const char *init) {
    int ret, len;
    static char desiredSSID[16 + 1];
    static char wepEncryption[16 + 1];
    static char key0Value[16 + 1];
    static char ownSSID[16 + 1];
    static char ownChannel[16 + 1];
    static char IP[16 + 1];
    static char Port[16 + 1];

    *IP = 0;
    *Port = 0;

//desiredSSID|wepEncryption|key0Value|ownSSID|ownChannel

    *desiredSSID = 0;
    *wepEncryption = 0;

    ret = getTok(IP, init, '|');    //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    cfg.wifi.ipAddress = IP;

    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    ret = getTok(Port, init, '|');  //retrieve mandatory phone
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;
    cfg.wifi.tcpPort = Port;

    ret = getTok(desiredSSID, init, '|');   //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    cfg.wifi.desiredSSID = desiredSSID;

    ret = getTok(wepEncryption, init, '|'); //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;
    cfg.wifi.wepEncryption = atoi(wepEncryption);
    ret = getTok(key0Value, init, '|'); //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;

    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    ret = getTok(ownSSID, init, '|');   //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    cfg.wifi.ownSSID = ownSSID;
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    ret = getTok(ownChannel, init, '|');    //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    cfg.wifi.ownChannel = atoi(ownChannel);
    len += ret;

    return len;
  lblKO:
    return -1;

}
#endif

#ifdef __CHN_GSM__
static int getPinTok(const char *init) {
    int ret, len;
    static char pin[16 + 1];

    *pin = 0;
    ret = getTok(pin, init, '|');
    CHECK(ret >= 0, lblKO);

    cfg.pin = pin;
    len = ret;

    return len;

  lblKO:
    return -1;
}

static int getModulationTok(const char *init) {
    int ret, len;
    char modulation[100];
    card dataMode;

    *modulation = 0;
    ret = getTok(modulation, init, '|');
    CHECK(ret > 0, lblKO);
    len = ret;

    ret = dec2num(&dataMode, modulation, 0);
    CHECK(ret >= 0, lblKO);

    switch (dataMode) {
      case 0:                  // autobaud (default)
      case 1:                  // 300 bps in V21 (*1)
      case 2:                  // 1200 bps in V22 (*1)
      case 3:                  // 1200/75 bps in V23 (*1)
      case 4:                  // 2400 bps in V22bis
      case 5:                  // 2400 bps in V26ter (*1)
      case 6:                  // 4800 bps in V32
      case 7:                  // 9600 bps in V32
      case 8:                  // specific (*1)
      case 12:                 // 9600 bps in V34 (*1)
      case 14:                 // 14400 bps in V34 (configure data and fax 14400 bearers) (*1)
      case 65:                 // 300 bps in V110 (*1)
      case 66:                 // 1200 bps in V110 (*1)
      case 68:                 // 2400 bps in V110
      case 70:                 // 4800 bps in V110)
      case 71:                 // 9600 bps in V110.
      case 75:                 // 14400 bps in V110 (configure data and fax 14400 bearers) (*1)
          cfg.gsm.dataMode = (uint8) dataMode;
          break;
      default:
          cfg.gsm.dataMode = 0;
    }
    return len;

  lblKO:
    return -1;
}
#endif

#ifdef __CHN_GPRS__
static int getAPNTok(const char *init) {
    int ret, len;
    static char Apn[16 + 1];

    *Apn = 0;
    ret = getTok(Apn, init, '|');
    CHECK(ret > 0, lblKO);
    len = ret;
    cfg.gprs.APN = Apn;

    return len;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_COM__
static int getMdmInit(const char *init) {   //retrieve cfg.tel from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    ret = getMdmTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getPhoneTok(init);
    CHECK(ret > 0, lblKO);
    //init+= ret; //skip token
    //VERIFY(*init=='|');
    //init++; //skip separator

    cfg.tel.connectionType = GCL_ASYN;
    cfg.tel.communicationTimeout = 2000;    //20 secs
    cfg.tel.connectTimeout = 6000;  //1 minute
    cfg.tel.loginTimeout = 3000;    //half minute
    cfg.tel.fastConnectMode = COM_FIELD_NOT_USED;
    cfg.tel.detectLine = /*TRUE*/ FALSE;
    cfg.tel.phoneId = 1;
    cfg.tel.retries = 1;
    cfg.tel.retryDelay = 100;
    cfg.tel.ringMode = COM_RING_NO_RING;
    cfg.tel.ringNumber = 0;
    cfg.asy.interCharTimeout = 0;

    cfg.tel.countryCode = COM_T35_France;
#ifdef __COUNTRY__
    cfg.tel.countryCode = __COUNTRY__;
#endif

    cfg.tel.Asyn_Config = &cfg.asy;

    return 1;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_HDLC__
static int getHdlcInit(const char *init) {  //retrieve cfg.hdlc from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);
    ret = getMdmTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getPhoneTok(init);
    CHECK(ret > 0, lblKO);
    //init+= ret; //skip token
    //VERIFY(*init=='|');
    //init++; //skip separator

    cfg.tel.connectionType = GCL_HDLC;
    cfg.tel.communicationTimeout = 2000;    //20 secs
    cfg.tel.connectTimeout = 6000;  //1 minute
    cfg.tel.loginTimeout = 3000;    //half minute
    cfg.tel.fastConnectMode = COM_FIELD_USED;
    cfg.tel.detectLine = /*TRUE*/ FALSE;
    cfg.tel.phoneId = 1;
    cfg.tel.retries = 1;
    cfg.tel.ringMode = COM_RING_NO_RING;
    cfg.tel.ringNumber = 0;

    cfg.tel.countryCode = COM_T35_France;
#ifdef __COUNTRY__
    cfg.tel.countryCode = __COUNTRY__;
#endif

    return 1;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_PPP__
static int getPppInit(const char *init) {   //retrieve cfg.ppp from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    cfg.ppp.lcpFlags = NI_PPP_LCP_CHAP | NI_PPP_LCP_PAP | NI_PPP_LCP_MSCHAP;
    cfg.ppp.useToneDialing = TRUE;
    cfg.ppp.AsyncModemType = COM_MODEM_MODULATION_V34;
    cfg.ppp.communicationTimeout = 12000;
    cfg.ppp.connectionId = 1;
    cfg.ppp.retries = 1;
    cfg.ppp.connectTimeout = 12000;
    cfg.ppp.countryCode = COM_T35_Brazil;
    cfg.ppp.dataCompression = COM_COMPRESSION_NONE;
    cfg.ppp.errorCorrection = COM_CORRECTION_NONE;
    cfg.ppp.detectLine = /*TRUE*/ FALSE;
    cfg.ppp.loginTimeout = 9000;
    cfg.ppp.useToneDialing = TRUE;
    //cfg.ppp.hostName = "";
    cfg.ppp.soLinger = 1;
    //cfg.ppp.tcpNoDelay = 1;

    ret = getPhoneTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getLoginTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator 

    ret = getTCPIPTok(init);
    CHECK(ret > 0, lblKO);

    return 1;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_GSM__
static int PinSend(void) {
    int ret;
    uint8 on, nbpin1, nbpin2, nbpuk1, nbpuk2, pinstat;
    uint32 gsmHandle;

    ret = gsmOpen(&gsmHandle, "MODEM3");
    CHECK(ret == GSM_OK, lblOpen);

    on = 1;
    ret = gsmOnOff(gsmHandle, SET, &on);
    CHECK(ret == GSM_OK, lblOnOff);

    gsmGetPinStatus(gsmHandle, &pinstat, &nbpin1, &nbpin2, &nbpuk1, &nbpuk2);
    if(pinstat == 1) {
        ret = gsmEnterPin(gsmHandle, "", cfg.pin);
        CHECK(ret == GSM_OK, lblPin);
    }
    gsmClose(gsmHandle);
    return 1;

  lblPin:
    trcFN("gsmEnterPin ret=%d\n", ret);
    gsmClose(gsmHandle);
    goto lblKO;
  lblOpen:
    trcFN("gsmOpen ret=%d\n", ret);
    goto lblKO;
  lblOnOff:
    trcFN("gsmOnOff ret=%d\n", ret);
    gsmClose(gsmHandle);
    goto lblKO;
  lblKO:
    return -1;
}

static int getGsmInit(const char *init) {   //retrieve cfg.gsm from init
    int ret;

    //static char gsmPhone[20+1];

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    cfg.gsm.communicationTimeout = 5000;
    cfg.gsm.connectionId = 1;
    cfg.gsm.connectTimeout = 4000;
    cfg.gsm.dataMode = 0;
    cfg.gsm.loginTimeout = 3000;
    cfg.gsm.retries = 1;
    cfg.gsm.cnxMode = '0';

    ret = getPinTok(init);      //get the sim card pin
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    getPhoneTok(init);

    if(*init == '|') {
        init++;                 //skip separator
        getModulationTok(init); //get the modulation
    }
    ret = PinSend();
    CHECK(ret > 0, lblKO);

    return 1;
  lblKO:
    return -1;
}
#endif

#ifdef __CHN_GPRS__
static int getGprsInit(const char *init) {  //retrieve cfg.gprs from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    cfg.gprs.communicationTimeout = 5000;
    cfg.gprs.connectionId = 1;
    cfg.gprs.connectTimeout = 4000;
    cfg.gprs.contextId = 1;
    cfg.gprs.lcpFlags = NI_PPP_LCP_PAP | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP;
    cfg.gprs.loginTimeout = 3000;
    cfg.gprs.retries = 1;

    ret = getPinTok(init);

    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getAPNTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getLoginTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator 

    ret = getTCPIPTok(init);
    CHECK(ret > 0, lblKO);

    ret = PinSend();
    CHECK(ret > 0, lblKO);

    return 1;

  lblKO:
    return -1;
}
#endif

#ifdef __CHN_TCP__
static int getEthInit(const char *init) {   //retrieve cfg.eth from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    //cfg.eth.communicationTimeout = 5000 //To be set by gclSetCommCfg function.
    cfg.eth.connectionId = 1;
    cfg.eth.connectTimeout = 4000;
    cfg.eth.loginTimeout = 3000;
    cfg.eth.retries = 1;
    //cfg.eth.useDhcp = 0;//moved to inside getTCPIPTok function.

    ret = getTCPIPTok(init);
    CHECK(ret > 0, lblKO);

    return 1;

  lblKO:
    return -1;
}
#endif

#ifdef __CHN_SCK__
static int getSckInit(const char *init) {   //retrieve cfg.sck from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);
    memset(&cfg.sck, 0, sizeof(cfg.sck));

    cfg.sck.connectionId = 1;
    cfg.sck.retries = 3;
    cfg.sck.connectTimeout = 6000;
    cfg.sck.communicationTimeout = 9000;
    cfg.sck.loginTimeout = 3000;
    cfg.sck.retryDelay = 5;
    //cfg.sck.soLinger = 0;

    ret = getTCPIPTok(init);
    CHECK(ret >= 0, lblKO);

    return 1;

  lblKO:
    return -1;
}
#endif

#ifdef __CHN_WIFI__
static int getWifiInit(const char *init) {  //retrieve cfg.tel from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);
    cfg.wifi.hostName = "";
    cfg.wifi.dhcpHostName = "";
    cfg.wifi.defaultLocalIpAddress = "";
    cfg.wifi.defaultDefGateway = "192.168.10.1";
    cfg.wifi.defaultDns1IpAddress = "";
    cfg.wifi.defaultDns2IpAddress = "";
    cfg.wifi.domainName = "";
    cfg.wifi.key0Value = "";
    cfg.wifi.key1Value = "";
    cfg.wifi.key2Value = "";
    cfg.wifi.key3Value = "";

    cfg.wifi.useDhcp = 1;       //!< if one the DHCP will be used to retrieve the local configuration
    cfg.wifi.WiFiMode = NET_PRISM_MODE_INFRASTRUCTURE;  //!< use NET_PRISM_MODE_ADHOC or NET_PRISM_MODE_INFRASTRUCTURE
    cfg.wifi.allowedSpeedRate = 0x0F;   //!< bit 0 = 1Mbps, bit 1 = 2Mbps, bit 2 = 5.5Mbps, bit 3 = 11 Mbps

    //Specific if wep encryption is used
    cfg.wifi.authMode = NET_PRISM_AUTH_OPENSYSTEM;  //!< 0 or NET_PRISM_AUTH_OPENSYSTEM or NET_PRISM_AUTH_SHAREDKEY
    cfg.wifi.defaultWepKey = 0xFFFFFFFFL;   //!< 0, 1 2 or 3 for default wep key number or 0xffffffff for all keys
    cfg.wifi.key0Type = 0xFFFF;
    cfg.wifi.key1Type = 0xFFFF;
    cfg.wifi.key2Type = 0xFFFF;
    cfg.wifi.key3Type = 0xFFFF;
    //!< login timeout

    cfg.wifi.communicationTimeout = 5000;
    cfg.wifi.connectionId = 1;
    cfg.wifi.connectTimeout = 4000;
    cfg.wifi.loginTimeout = 3000;
    cfg.wifi.retries = 1;

    cfg.wifi.retryDelay = 1000; //!< the delay before retying connection
    // no delay for sending tcp data
    cfg.wifi.tcpNoDelay = 1;    //!< 1 to set the TCP_NODELAY option

    ret = getWifiTok(init);
    CHECK(ret > 0, lblKO);

    return 1;

  lblKO:
    return -1;
}
#endif

#ifdef __CHN_SERIAL__
static int getSerialInit(const char *init) {
    int ret;
    int idx, idx2, speedLen, speed;
    char buf[INIT_LEN];
    int pow = 1;
    static char comPortName[4 + 1];

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    cfg.serial.connectionId = 1;
    cfg.serial.retries = 1;
    cfg.serial.connectTimeout = 40 * 100;
    cfg.serial.communicationTimeout = 50 * 100;;
    cfg.serial.loginTimeout = 30 * 100;
    cfg.serial.speed = COM_BAUD_NO_CHANGE;
    cfg.serial.parity = COM_PARITY_NO_CHANGE;
    cfg.serial.dataSize = COM_DATASIZE_NO_CHANGE;
    cfg.serial.stopBits = COM_STOP_NO_CHANGE;
    cfg.serial.flowControl = COM_FLOW_NO_CHANGE;
    cfg.serial.interCharTimeout = 0;

    //COM port
    ret = getTok(buf, init, '|');
    init += ret;                //skip the device name

    if(strcmp(buf, "COM1") == 0 || strcmp(buf, "COM2") == 0)
        strcpy(comPortName, buf);
    else                        //Illegal device name
        return -1;

    cfg.serial.deviceName = comPortName;

    if(!*init)                  //no more parameters
        return 1;

    init++;                     //skip the separator

    ret = getTok(buf, init, '|');

    idx = 0;
    //Data size
    switch (buf[idx++]) {
      case '5':
          cfg.serial.dataSize = COM_DATASIZE_5;
          break;
      case '6':
          cfg.serial.dataSize = COM_DATASIZE_6;
          break;
      case '7':
          cfg.serial.dataSize = COM_DATASIZE_7;
          break;
      case '8':
          cfg.serial.dataSize = COM_DATASIZE_8;
          break;
      case '0':
          cfg.serial.dataSize = COM_DATASIZE_NO_CHANGE;
          break;
      case 0:
          return 1;             //exit
      default:
          return -1;            //illegal character
    }
    //Parity
    switch (buf[idx++]) {
      case 'N':
          cfg.serial.parity = COM_PARITY_NONE;
          break;
      case 'O':
          cfg.serial.parity = COM_PARITY_ODD;
          break;
      case 'E':
          cfg.serial.parity = COM_PARITY_EVEN;
          break;
      case ' ':
          cfg.serial.parity = COM_PARITY_NO_CHANGE;
          break;
      case 0:
          return 1;
      default:
          return -1;
    }
    //Stop bits
    switch (buf[idx++]) {
      case '1':
          cfg.serial.stopBits = COM_STOP_1;
          break;
      case '2':
          cfg.serial.stopBits = COM_STOP_2;
          break;
      case '0':
          cfg.serial.stopBits = COM_STOP_NO_CHANGE;
          break;
      case 0:
          return 1;
      default:
          return -1;
    }
    //Speed
    if(!buf[idx])
        return 1;

    for (speedLen = 0; buf[idx + speedLen] >= '0' && buf[idx + speedLen] <= '9';
         speedLen++);
    //Convert the speed binary
    speed = 0;
    for (idx2 = speedLen - 1; idx2 >= 0; idx2--) {
        speed += (buf[idx + idx2] - '0') * pow;
        pow *= 10;
    }
    idx += speedLen;
    switch (speed) {
      case 0:
          cfg.serial.speed = COM_BAUD_NO_CHANGE;
          break;
      case 50:
          cfg.serial.speed = COM_BAUD_50;
          break;
      case 75:
          cfg.serial.speed = COM_BAUD_75;
          break;
      case 150:
          cfg.serial.speed = COM_BAUD_150;
          break;
      case 300:
          cfg.serial.speed = COM_BAUD_300;
          break;
      case 600:
          cfg.serial.speed = COM_BAUD_600;
          break;
      case 1200:
          cfg.serial.speed = COM_BAUD_1200;
          break;
      case 2400:
          cfg.serial.speed = COM_BAUD_2400;
          break;
      case 4800:
          cfg.serial.speed = COM_BAUD_4800;
          break;
      case 9600:
          cfg.serial.speed = COM_BAUD_9600;
          break;
      case 19200:
          cfg.serial.speed = COM_BAUD_19200;
          break;
      case 38400:
          cfg.serial.speed = COM_BAUD_38400;
          break;
      case 57600:
          cfg.serial.speed = COM_BAUD_57600;
          break;
      case 76800:
          cfg.serial.speed = COM_BAUD_76800;
          break;
      case 115200:
          cfg.serial.speed = COM_BAUD_115200;
          break;
      default:
          return -1;
    }

    //Flow control
    switch (buf[idx++]) {
      case 'N':
          cfg.serial.flowControl = COM_FLOW_NONE;
          break;
      case 'H':
          cfg.serial.flowControl = COM_FLOW_HARD;
          break;
      case 0:
          return 1;
      default:
          return -1;
    }
    return 1;
}
#endif

#ifdef __CHN_SSL__
static int getSslInit(const char *init) {
    int ret;
    int skipInit = 0;
    char buf[INIT_LEN + 1];

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    cfg.isSSL = !memcmp(init, "SSL:", 4);
    if(cfg.isSSL) {
        ret = getTok(buf, init, '|');
        skipInit = ret + 1;     //skip also the '|'

        //inicialization
        init += 4;              //skip the "SSL:"
        if(!memcmp(init, "V3", 2)) {
            cfg.ssl.sslMethod = SSL_V3_CLIENT;
            init += 2;
        } else if(!memcmp(init, "TLS1", 4)) {
            cfg.ssl.sslMethod = SSL_TLS1_CLIENT;
            init += 4;
        } else
            goto lblKO;
        CHECK(*init == '|', lblKO);

        cfg.ssl.sslVerifType = SSL_NO_VERIFY;
        memset(&cfg.ssl.sslCertificatName, 0,
               sizeof(cfg.ssl.sslCertificatName));
        memset(&cfg.ssl.sslCADir, 0, sizeof(cfg.ssl.sslCADir));
        memset(&cfg.ssl.sslPrivateKeyName, 0,
               sizeof(cfg.ssl.sslPrivateKeyName));
        memset(&cfg.ssl.password, 0, sizeof(cfg.ssl.password));
        cfg.ssl.sessionCacheMode = 0;
        cfg.ssl.useActions = 0;
        memset(&cfg.ssl.actions, 0, sizeof(cfg.ssl.actions));
        cfg.ssl.ciphersList = 0;
        cfg.ssl.exportStrength = 0;
        cfg.ssl.sslPrivateKeySource = GCL_SSL_PRIVKEY_STR;
        cfg.ssl.privateKeyStr = NULL;
        cfg.ssl.maximumLifespan = 0;
    }

    return skipInit;

  lblKO:
    return -1;
}
#endif

#ifdef __CHN_COM__
static int utaGclStartAsyn(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartAsyn init=%s\n", init);

#ifdef __CHN_RADIO__
    ret = setRadioMode();
    CHECK(ret > 0, lblKO);
#endif

    ret = getMdmInit(init);
    CHECK(ret > 0, lblKO);      //parse initialisation string

    fnc.predial = gclModemPreDial;
    fnc.dial = gclAsynDial;
    fnc.connect = gclModemConnect;
    fnc.login = nullCallBack;
    fnc.hangup = gclModemHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclAsynSend;     //_gclSnd; //
    fnc.recv = gclAsynReceive;  //_gclRcv; //
    ret = gclModemTelephoneSet(&cfg.tel, &fnc, 0, NULL);
    CHK;

    return 1;
  lblKO:
    trcFN("utaGclStartAsyn err ret=%d\n", ret);
    return -1;
}
#endif

#ifdef __CHN_ISDN__

#define ERROR_CHECK(CND,COD) if (!(CND)) {gclSetError((COD)); ret= (COD); goto lblEnd;}

static int getRspCod(gclConfig_t * gcl, uint32 dly) {
    int ret;
    struct {
        int16 sta;
        uint16 len;
        char buf[1024];
    } buf;
    uint8 ctl[] = { 1, 0x0a, 0 };   //CR is the terminator character
    int idx, pow;

    ret = comReceiveMsgWait(gcl->comHandle, 1024, ctl, dly, sizeof(buf), &buf);
    if(ret != RET_OK)
        return -1;

    ret = 0;
    pow = 1;
    for (idx = buf.len - 1; idx >= 0; idx--) {
        if(buf.buf[idx] >= '0' && buf.buf[idx] <= '9') {
            ret += (buf.buf[idx] - '0') * pow;
            pow *= 10;
        }
    }
    return ret;
}

static int16 _gclISDNPreDial(gclConfig_t * gcl) {
    int16 ret;

    ret = comOpen("MODEM", &gcl->comHandle);
    ERROR_CHECK(ret == RET_OK, GCL_ERR_OPEN_DEVICE);
    //inicialize the modem
    ret =
        comSendMsgWait(gcl->comHandle, strlen("ate0v0\x0d"),
                       (uint8 *) "ate0v0\x0d", 6000);
    ret = getRspCod(gcl, 15);
    ERROR_CHECK(ret == 0, GCL_ERR_CONFIG);
    getRspCod(gcl, 15);
    ret = RET_OK;
    goto lblEnd;
  lblEnd:
    return ret;
}

static int16 _gclISDNDial(gclConfig_t * gcl) {
    int16 ret;
    char buf[128];

    strcpy(buf, "atd ");
    strcat(buf, cfg.phoneNumber);
    strcat(buf, "\x0d");
    ret = comSendMsgWait(gcl->comHandle, strlen(buf), (uint8 *) buf, 6000);
    ERROR_CHECK(ret == RET_OK, GCL_ERR_NO_CARRIER);
    goto lblEnd;
  lblEnd:
    return ret;
}

static int16 _gclISDNConnect(gclConfig_t * gcl) {
    int16 ret;

    switch (getRspCod(gcl, gcl->connectTimeout)) {
      case 0:
      case 2:
      case 3:
      case 4:
      case -1:
          ret = GCL_ERR_NO_CARRIER;
          break;
      case 7:
          ret = GCL_ERR_BUSY;
          break;
      case 6:
          ret = GCL_ERR_NO_DIALTONE;
          break;
      default:
          ret = RET_OK;
          break;
    }
    ERROR_CHECK(ret == RET_OK, ret);
    goto lblEnd;
  lblEnd:
    return ret;
}

static int16 _gclISDNHangUp(gclConfig_t * gcl) {
    int ret;
    byte try = 4;

    do {
        try--;
        tmrSleep(100);
        comSendMsgWait(gcl->comHandle, 3, (uint8 *) "+++", 6000);
        tmrSleep(100);
        while(getRspCod(gcl, 10) != -1);    //rease the buffer
        comSendMsgWait(gcl->comHandle, strlen("ath\x0d"), (uint8 *) "ath\x0d",
                       6000);
        ret = getRspCod(gcl, 15);
        if(ret == -1)
            continue;
        try = 0;
    } while(try);

    comClose(gcl->comHandle);
    gcl->comHandle = 0;
    return RET_OK;
}

static int utaGclStartISDN(const char *init) {
    int ret;
    static gclConfig_t conf;

    trcFS("utaGclStartISDN init=%s\n", init);

    strcpy(cfg.phoneNumber, init);

    memset(&conf, 0, sizeof(conf));
    conf.connectionId = 1;
    conf.connectionType = GCL_SERIAL;
    conf.connectTimeout = 7000;
    conf.communicationTimeout = 2000;
    //call back functions
    conf.predial = _gclISDNPreDial;
    conf.dial = _gclISDNDial;
    conf.connect = _gclISDNConnect;
    conf.login = nullCallBack;
    conf.hangup = _gclISDNHangUp;
    conf.retry = gclConnectRetry;
    conf.error = connErrorCallBack;
    conf.send = _gclSnd;
    conf.recv = _gclRcv;

    ret = gclAddConnection(&conf);
    CHK;

    return 1;
  lblKO:
    trcFN("utaGclStartAsyn err ret=%d\n", ret);
    return -1;
}
#endif

#ifdef __CHN_HDLC__
static int utaGclStartHdlc(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartHdlc init=%s\n", init);
#ifdef __CHN_RADIO__
    setRadioMode();
#endif
    ret = getHdlcInit(init);
    CHECK(ret > 0, lblKO);      //parse initialisation string

    fnc.predial = gclHDLCPreDial;
    fnc.dial = gclHDLCDial;
    fnc.connect = gclHDLCConnect;
    fnc.login = NULL;
    fnc.hangup = gclHDLCHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = gclConnectError;
    fnc.send = gclHDLCSend;
    fnc.recv = gclHDLCReceive;
    ret = gclModemTelephoneSet(&cfg.tel, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartHdlc err ret=%d\n", ret);
    return -1;
}
#endif

#ifdef __CHN_PPP__

int16 static _gclPPPHangUp(gclConfig_t * gcl) {
    gclPPPConfig_t *p;

    p = gcl->extraData;
    p->modemParameters.hangUpMode = COM_HANG_UP_COMMAND;
    gcl->extraData = p;

    gclPPPHangUp(gcl);

    return RET_OK;
}

static int utaGclStartPpp(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartPpp init=%s\n", init);
    ret = getPppInit(init);
    CHECK(ret > 0, lblKO);      //parse initialisation string

    fnc.predial = gclPPPPreDial;
    fnc.dial = gclPPPDial;
    fnc.connect = gclPPPConnect;
    fnc.login = nullCallBack;
    fnc.hangup = _gclPPPHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclPPPSend;
    fnc.recv = gclPPPReceive;

    ret = gclPPPSet(&cfg.ppp, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartPpp err ret=%d\n", ret);
    return -1;
}
#endif

#ifdef __CHN_GSM__
static int utaGclStartGsm(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartGsm init=%s\n", init);
    ret = getGsmInit(init);     //parse initialisation string
    CHECK(ret > 0, lblKO);

    fnc.predial = gclGSMPreDial;
    fnc.dial = gclGSMDial;
    fnc.connect = gclGSMConnect;
    fnc.hangup = gclGSMHangUp;
    fnc.error = connErrorCallBack;
    fnc.login = nullCallBack;
    fnc.retry = gclConnectRetry;
    fnc.send = gclGSMSend;
    fnc.recv = gclGSMReceive;   /*utaGsmRecvFunc *///_gclRcv;//

    ret = gclGSMSet(&cfg.gsm, &fnc, 0, NULL);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    trcFN("utaGclStartGsm err ret=%d\n", ret);
    return -1;

}
#endif

#ifdef __CHN_GPRS__
static int utaGclStartGprs(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartGprs init=%s\n", init);
    ret = getGprsInit(init);    //parse initialisation string
    CHECK(ret > 0, lblKO);

    fnc.predial = gclGPRSPreDial;
    fnc.dial = gclGPRSDial;
    fnc.connect = gclGPRSConnect;
    fnc.login = nullCallBack;
    fnc.hangup = gclGPRSHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclGPRSSend;
    fnc.recv = gclGPRSReceive;

    ret = gclGPRSSet(&cfg.gprs, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartGprs err ret=%d\n", ret);
    return -1;

}
#endif

#ifdef __CHN_TCP__

/**
 * This function has to be called by the SEND callback if the connection type
 * is Ethernet connection.
 * It sends data but before sending, first it checks if the Ethernet cable is connected or not.
 * \param gcl (I) the configuration of the connection.
 * \param data (I) a pointer to the buffer to be sent
 * \param dataSize (I) the size of the buffer to be sent   
 * \return  
 *    - negative if ethernet cable is not connected(-gclErrCableRemoved).
 *    - negative if other failure happend. 
 *    - otherwise positive
 * \test tcfa0005.c
*/
static int16 gclEthSendCableChk(gclConfig_t * gcl, uint8 * data,
                                uint32 dataSize) {
    int16 ret;
    netNiCtrl_t control;
    int32 link;
    uint32 ni;

    ni = ((gclEthConfig_t *) (gcl->extraData))->ni;

    control.cmd = NET_NI_CMD_GET;
    control.opt = NET_NI_OPT_ETH_LINK;
    control.bufsize = sizeof(link);
    control.buffer = &link;

    ret = netNiCtrl(ni, &control);

    if(ret == RET_OK) {
        if(link == NET_NI_ETH_LINK_DOWN) {
            return -gclErrCableRemoved;
        }
    }
    ret = gclEthSend(gcl, data, dataSize);
    return ret;
}
static int utaGclStartEth(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartEth init=%s\n", init);
    ret = getEthInit(init);
    CHECK(ret > 0, lblKO);      //parse initialisation string

    fnc.predial = gclEthPreDial;
    fnc.dial = gclEthDial;
    fnc.connect = gclEthConnect;
    fnc.login = nullCallBack;
    fnc.hangup = gclEthHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
#ifdef _ING_SIMULATION
    fnc.send = gclEthSend;
#else
    fnc.send = gclEthSendCableChk;  // gclEthSend;   
#endif
    fnc.recv = gclEthReceive;

    ret = gclEthSet(&cfg.eth, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartEth err ret=%d\n", ret);
    return -1;

}
#endif

#ifdef __CHN_SCK__
static int utaGclStartSck(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    memset(&fnc, 0, sizeof(fnc));
    trcFS("utaGclStartSck init=%s\n", init);
    ret = gclPgSupReset();
    CHECK(ret == RET_OK, lblKO);
    ret = getSckInit(init);     //parse initialisation string
    CHECK(ret > 0, lblKO);

    fnc.predial = gclSockPreDial;
    fnc.dial = gclSockDial;
    fnc.connect = gclSockConnect;
    fnc.login = nullCallBack;
    fnc.hangup = gclSockHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclSockSend;
    fnc.recv = gclSockReceive;

    ret = gclSockSet(&cfg.sck, &fnc, 0, NULL);
    CHK;
#ifdef __CHN_SSL__
    if(cfg.isSSL) {
        ret = gclSSLAttach();
        CHK;
    }
#endif
    ret = gclPgSupAttach(6000); // half minute timeout
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartSck err ret=%d\n", ret);
    return -1;

}
#endif

#ifdef __CHN_WIFI__
static int16 cfgErrorAnalyses(gclConfig_t * gcl) {
    gclPostMessage(GCL_RETRY);
    return RET_OK;
}

static int utaGclStartWifi(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartWifi init=%s\n", init);
    ret = getWifiInit(init);
    CHECK(ret > 0, lblKO);      //parse initialisation string

    fnc.predial = gclWiFiPreDial;
    fnc.dial = gclWiFiDial;
    fnc.connect = gclWiFiConnect;
    fnc.login = gclNULLFunction;
    fnc.hangup = gclWiFiHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = cfgErrorAnalyses;
    fnc.send = gclWiFiSend;
    fnc.recv = gclWiFiReceive;
    ret = gclWiFiSet(&cfg.wifi, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartWifi err ret=%d\n", ret);
    return -1;

}
#endif

#ifdef __CHN_SERIAL__
static int utaGclStartSerial(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("utaGclStartSerial init=%s\n", init);

#ifdef __CHN_RADIO__
    ret = setRadioMode();
    CHECK(ret > 0, lblKO);
#endif

    ret = getSerialInit(init);
    CHECK(ret > 0, lblKO);      //parse initialisation string

    fnc.predial = gclSerialPreDial;
    fnc.dial = nullCallBack;
    fnc.connect = nullCallBack;
    fnc.login = nullCallBack;
    fnc.hangup = gclSerialHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclSerialSend;
    fnc.recv = gclSerialReceive;    //_gclRcv;//

    ret = gclSerialSet(&cfg.serial, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStartSerial err ret=%d\n", ret);
    return -1;

}
#endif

#ifdef __CHN_SSL__
static int utaGclStartSsl(const char *init) {
    int ret;
    int skip;

    trcFS("utaGclStartSsl init=%s\n", init);

    ret = getSslInit(init);
    CHECK(ret >= 0, lblKO);     //parse initialisation string

    if(!ret)                    //no SSL using
        skip = 0;
    else {
        skip = ret;
        ret = gclSSLInit(&cfg.ssl);
        CHECK(ret == RET_OK, lblKO);
    }
    return skip;
  lblKO:
    trcFN("utaGclStartSsl err ret=%d\n", ret);
    return -1;

}
#endif

/** Open the associated communication channel.
 * \param chn (I) Channel type from enum eGcl.
 * \param init (I) Initialization string, depends on the channel
 *  - TCP/IP connection (gclEth) with the '|' as the separator
 *    - If the *init!=0, then use static IP adress
 *      - remote IP adress
 *      - remote IP port
 *      - local IP Adress
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclStart(byte chn, const char *init) {
#ifdef __CHN_SSL__
    int ret;
#endif
    VERIFY(gclBeg < chn);
    VERIFY(chn < gclEnd);
    VERIFY(init);

    gclReset();
    memset(&cfg, 0, sizeof(cfg));
#ifdef __CHN_SSL__
    gclSSLReset();
    ret = utaGclStartSsl(init);
    CHECK(ret >= 0, lblKO);
    init += ret;
#endif
    cfg.chn = chn;
    switch (chn) {
#ifdef __CHN_HDLC__
      case gclHdlc:
          return utaGclStartHdlc(init);
#endif
#ifdef __CHN_COM__
      case gclAsyn:
          return utaGclStartAsyn(init);
#endif
#ifdef __CHN_ISDN__
      case gclISDN:
          return utaGclStartISDN(init);
#endif
#ifdef __CHN_SERIAL__
      case gclSer:
          return utaGclStartSerial(init);
#endif
          //case gclWnb: break;
#ifdef __CHN_PPP__
      case gclPpp:
          return utaGclStartPpp(init);
#endif
          //case gclPppCom: break;        
          //case gclPppEth: break;
#ifdef __CHN_GSM__
      case gclGsm:
          return utaGclStartGsm(init);
#endif
#ifdef __CHN_GPRS__
      case gclGprs:
          return utaGclStartGprs(init);
#endif
#ifdef __CHN_TCP__
      case gclEth:
          return utaGclStartEth(init);
#endif
#ifdef __CHN_SCK__
      case gclSck:
          return utaGclStartSck(init);
#endif
#ifdef __CHN_WIFI__
      case gclWifi:
          return utaGclStartWifi(init);
#endif
          //case gclPppGsm: break;
          //case gclSck: break;
      default:                 //impossible or not implemented yet
          break;
    }
    VERIFY(chn == gclBeg);
#ifdef __CHN_SSL__
  lblKO:
#endif
    return -1;
}

/** Close the associated channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclStop(void) {
    int ret;

    trcS("utaGclStop\n");
    ret = gclStop();
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclStop err ret=%d\n", ret);
    return -1;
}

/** Start connection to a remote server.
 * \return non-negative if OK; -eGclErr enumeration otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclDial(void) {
    int ret;

    trcS("utaGclDial\n");
    ret = gclStart();
    CHK;

    ret = gclStartConnection();
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclDial ret=%d\n", ret);
    return -1;
}

/** Wait until connection is finished
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclDialWait(void) {
    int ret;

    ret = gclWaitConnection();
    CHK;
    return 1;
  lblKO:
    trcFN("gclWaitConnection ret=%d\n", ret);
    return -1;
}

/** Retrieve the state of the communication
 * \return enumeration value from eGclSta
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclDialInfo(void) {
    int ret;

    ret = gclTaskIsAlive();
    if(!ret)
        return gclStaFinished;
    ret = gclTaskGetState();
    switch (ret) {
      case GCL_PREDIAL:
          ret = gclStaPreDial;
          break;
      case GCL_DIAL:
          ret = gclStaDial;
          break;
      case GCL_CONNECT:
          ret = gclStaConnect;
          break;
      case GCL_LOGIN:
          ret = gclStaLogin;
          break;
      case GCL_CONNECTED:
          ret = gclStaFinished;
          break;
      default:
          ret = -1;
          break;
    }
    return ret;
}

/** Get the communication error code
 * \return enumeration value from eGclErr
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclDialErr(void) {
    int ret;

    ret = gclLastError();

    switch (ret) {
      case GCL_ERR_CANCEL_BY_USER:
          return gclErrCancel;
      case GCL_ERR_DIAL:
          return gclErrDial;
      case GCL_ERR_BUSY:
          return gclErrBusy;
      case GCL_ERR_NO_DIALTONE:
          return gclErrNoDialTone;
      case GCL_ERR_NO_CARRIER:
          return gclErrNoCarrier;
      case GCL_ERR_NO_ANSWER:
          return gclErrNoAnswer;
      case GCL_ERR_LOGIN:
          return gclErrLogin;
      default:
          return gclErrEnd;
    }
}

/** Send a byte b to the communication channel.
 * \param b (I) Byte to send
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
*/
int utaGclSend(byte b) {
    int ret;

    ret = gclSend(&b, 1);
    CHK;
    return 1;
  lblKO:
    trcFN("gclSend ret=%d\n", ret);
    return ret;
}

/** Receive a byte into b to the communication channel using a timeout value dly.
 * \param b (I) Pointer to a buffer to receive a byte
 * \param dly (I) Timeout in seconds
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test tcab0078.c
*/
int utaGclRecv(byte * b, int dly) {
    int ret;
    uint32 len;

    cfg.rcvDly = dly * 100;
    gclChageCommTimeout(dly * 100);
    len = 1;
    ret = gclReceive(b, 1, &len);

    trcFN("utaGclRecv b=%02X\n", (int) *b);
    CHK;
    return (int) len;
  lblKO:
    switch (gclLastError()) {
      case GCL_ERR_TIMEOUT:
          ret = 0;
          break;
      case GCL_ERR_RECEIVE:
          trcS("GCL_ERR_RECEIVE");
          ret = -1;
      default:
          ret = -1;
          break;
    }
    gclSetError(0);
    trcFN("utaGclRecv err ret=%d\n", ret);
    return ret;                 //-1 will overwrite timeout case
}

/** Send len  bytes from  msg to the communication port.
 * \param msg (I) Pointer to buffer to send
 * \param len (I) Number of bytes to send
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
*/
int utaGclSendBuf(const byte * msg, word len) {
    int ret;

    trcFS("comSendBuf=%s\n", (char *) msg);
    trcS("bin: ");
    trcBN(msg, len);
    trcS("\n");

    ret = gclSend(msg, len);
    CHK;
    return len;
  lblKO:
    trcFN("utaGclSend err ret=%d\n", ret);
    return ret;

}

#ifdef __RSP_LENGTH__
#define RSP_LEN __GCL_RSP_LENGTH__
#else
#define RSP_LEN 1024
#endif
/** Receive a sequence of bytes terminated by a character from trm  into buffer msg
 * to the communication channel using a timeout value dly. 
 * The terminator control string consists of characters that can indicate the end of message
 * (ETX, EOT etc).
 * \param msg (O) Pointer to buffer to accept characters received; should be large enough 
 * \param trm (I) zero-terminated string of terminator control characters describing the messages ending
 * \param dly (I) timeout in seconds
 * \return  
 *    - negative if failure. 
 *    - zero if timeout. 
 *    - otherwise positive.
 * \header sys\\sys.h
 * \source sys\\i32.c
 * \test tcik0136.c
*/
int utaGclRecvBuf(tBuffer * msg, const byte * trm, byte dly) {
    int ret;
    uint32 len;
    byte tmp[RSP_LEN];

    if(msg)
        bufReset(msg);
    gclChageCommTimeout(dly * 100);
    cfg.rcvDly = dly * 100;
    //cfg.rcvTrm = trm;
    if(trm) {
        ret = gclSetTerminators((byte *) trm, sizeof(trm));
        if(ret < 0)
            goto lblKO;
    }
    len = RSP_LEN;
    if(!trm)
        len = bufDim(msg);
    ret = gclReceive(tmp, len, &len);
    CHECK(ret == RET_OK, lblKO);
    if(msg) {
        CHECK(len <= bufDim(msg), lblKO);
        ret = bufApp(msg, tmp, len);
        CHECK(ret >= 0, lblKO);
        trcBN(bufPtr(msg), bufLen(msg));
    }
    trcS("\n");
    return (int) len;
  lblKO:
    switch (gclLastError()) {
      case GCL_ERR_TIMEOUT:
          ret = 0;
          break;
      default:
          ret = -1;
          break;
    }
    gclSetError(0);
    trcFN("utaGclRecv err ret=%d\n", ret);
    cfg.rcvTrm = 0;
    return ret;
}

/** Release Upper comms layer of the associated channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\gcl.c
 * \test 
*/
int utaGclHangup(void) {
    int ret;

    trcS("utaGclHangup\n");

    ret = gclHangup(600);
    CHK;
    return 1;
  lblKO:
    trcFN("utaGclHangup err ret=%d\n", ret);
    return -1;
}

/**
 * This function may call from application to configure gcl communiaction configurations. To be called before "utaGclStart" function
 * Now it is only implemented for tcpip(Ethernet) receive communication timeout.
 * \param timOut (I) The ethernet communication timeout.   
 * \return  
 *    - No return value
 * \test tcap0021.c
*/
void utaGclSetCommCfg(card timOut) {
    cfgComm.timeOut = timOut * 100;
}
