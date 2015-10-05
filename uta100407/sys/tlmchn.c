/** \file
 * TELIUM communication channels implementation using LL (Link Layer)
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlmchn.c $
 *
 * $Id: tlmchn.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "SDK30.h"
#include "OEM_Public.h"
#include "sys.h"
#undef __USE_DEL_INTERFACE__
#include "TlvTree.h"
#include "LinkLayer.h"
#include "oem_dgprs.h"
#ifdef __CHN_GPRS__
#include "gprs_lib.h"
#endif

#ifdef __CHN_SSL__
#include <string.h>
#include <SSL_.h>
#include <X509_.h>
#endif

#ifdef __CHN_FTP__
#include <FTP_.h>
#endif

#ifdef __CHN_TCP__
#include<IP_.h>
#endif

#ifdef __CHN_SCK__
#include<IP_.h>
#endif

typedef struct sComChn {
    byte chn;                   // communication channel used
    byte chnRole;
    struct {
        LL_HANDLE hdl;
        char separator;
        unsigned long int conn_delay;   // 4 Second delay
        unsigned long int ll_conn_delay;
        unsigned int send_timeout;
        int ifconn;
#ifdef __CHN_COM__
        byte BperB;             //BITS_PER_BYTE 7 or not
        byte Parity;            //PARITY
#endif
#ifdef __CHN_SCK__
        int hdlSck;
        struct sockaddr_in peeraddr_in; // For peer socket address.
#endif
#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs
            unsigned int cTimeout;
            unsigned int cValue;
        } gprs;
#endif
#ifdef __CHN_SSL__
        SSL_HANDLE hdlSsl;
        SSL_PROFILE_HANDLE hdlProfile;
        char SslProfName[PROFILE_NAME_SIZE + 1];
#endif
    } prm;
    TLV_TREE_NODE hCfg;
    TLV_TREE_NODE hPhyCfg;
    TLV_TREE_NODE hDLinkCfg;
    TLV_TREE_NODE hTransCfg;
} tComChn;
static tComChn com;

#define RSP_LEN 1024
#define DEFAULT_INIT_STRING_ASYNC		"ATZE0"
#define DEFAULT_INIT_STRING_SYNC		"ATE0X3S6=1$M249$M251F4S144=16"

const char *parseStr(char *dst, const char *src, int dim, char separator) {
    VERIFY(dst);
    VERIFY(src);

    while(*src) {
        if(*src == separator) {
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

const char *parseIp(char *adr, const char *str) {
    int idx;

    VERIFY(str);
    VERIFY(adr);
    memset(adr, 0, 4);

    for (idx = 0; *str != '\0' && *str != com.prm.separator; str++) {
        if(*str == '.')         // next value will follow
        {
            idx++;
        } else {                // extracting value
            adr[idx] *= 10;
            adr[idx] += *str - '0';
        }
    }

    if(*str == com.prm.separator) {
        str++;                  // skip separator
    }

    return str;
}

static int _comCheckConnected(void) {
    // Special code to introduce an arbiteral delay
    if(com.prm.ifconn == FALSE) {
        while((get_tick_counter() - com.prm.ll_conn_delay) <
              com.prm.conn_delay);
    }
    com.prm.ifconn = TRUE;
    return 1;
}

static int comStartUSB(void) {
    com.hCfg = NULL;
    com.hDLinkCfg = NULL;
    com.hPhyCfg = NULL;
    com.hTransCfg = NULL;
#ifdef __CHN_USB__
    return 1;
#else
    return -1;
#endif
}

static int comStartMdm(void) {
#ifdef __CHN_COM__
    com.hCfg = NULL;
    com.hDLinkCfg = NULL;
    com.hPhyCfg = NULL;
    com.hTransCfg = NULL;
    return 1;
#else
    return -1;
#endif
}
static int comStartCom(void) {
#ifdef __CHN_COM__
    com.hCfg = NULL;
    com.hDLinkCfg = NULL;
    com.hPhyCfg = NULL;
    com.hTransCfg = NULL;
    return 1;
#else
    return -1;
#endif

}

static int comStartGprs(void) {
#ifdef __CHN_GPRS__
    com.hCfg = NULL;
    com.hDLinkCfg = NULL;
    com.hPhyCfg = NULL;
    com.hTransCfg = NULL;
    gprslib_open();
    return 1;
#else
    return -1;
#endif
}

static int comStartTcp(void) {
#ifdef __CHN_TCP__
    com.hCfg = NULL;
    com.hDLinkCfg = NULL;
    com.hPhyCfg = NULL;
    com.hTransCfg = NULL;
    return 1;
#else
    return -1;
#endif
}

static int comStartPpp(void) {
#ifdef __CHN_TCP__
    com.hCfg = NULL;
    com.hDLinkCfg = NULL;
    com.hPhyCfg = NULL;
    com.hTransCfg = NULL;
    return 1;
#else
    return -1;
#endif
}

static int comStartSck(void) {
#ifdef __CHN_SCK__
    com.prm.hdlSck = socket(AF_INET, SOCK_STREAM, 0);
    if(com.prm.hdlSck < 0)
        return -1;
    return 1;
#else
    return -1;
#endif
}

static int comStartSsl(void) {
#ifdef __CHN_SSL__
    com.prm.hdlSsl = 0;
    com.prm.hdlProfile = 0;
    ssllib_open();
    return 1;
#else
    return -1;
#endif
}

/** Open the associated communication channel.
 * \param chn (I) Channel type from enum eChn.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
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
      case chnSck:
          return comStartSck(); // socket
      case chnSsl:
          return comStartSsl(); // SSL
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSetCom(const char *init) {
#ifdef __CHN_COM__
    int ret;
    byte datasize;              //data bits: 7 or 8
    byte parity;                //odd,even,none
    byte stopbits;              //1,2
    unsigned long sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
    unsigned char physConf;

    trcFS("comSetCom %s\n", init);
    VERIFY(init);
    VERIFY(com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);

    com.prm.separator = '|';    //common for all types of chn
    com.prm.conn_delay = 4 * 100;   //4 Second delay
    com.prm.send_timeout = 30 * 100;
    com.prm.ifconn = FALSE;

    datasize = LL_PHYSICAL_V_8_BITS;

    switch (*init++) {          //number of data bits
      case '7':
          datasize = LL_PHYSICAL_V_7_BITS;
          break;
      case '8':
          datasize = LL_PHYSICAL_V_8_BITS;
          break;
      default:
          CHECK(!init, lblKOConfigure);
    }

    parity = LL_PHYSICAL_V_NO_PARITY;
    switch (*init++) {          //parity
      case 'N':
          parity = LL_PHYSICAL_V_NO_PARITY;
          break;
      case 'E':
          parity = LL_PHYSICAL_V_EVEN_PARITY;
          break;
      case 'O':
          parity = LL_PHYSICAL_V_ODD_PARITY;
          break;
      default:
          CHECK(!init, lblKOConfigure);
    }

    stopbits = LL_PHYSICAL_V_1_STOP;
    switch (*init++) {          //number of stop bits
      case '0':
          stopbits = LL_PHYSICAL_V_1_STOP;
          break;
      case '1':
          stopbits = LL_PHYSICAL_V_1_STOP;
          break;
      case '2':
          stopbits = LL_PHYSICAL_V_2_STOP;
          break;
      default:
          CHECK(!init, lblKOConfigure);
    }

    if(*init == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
        init++;
    } else if(memcmp(init, "300", 3) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_300;
        init += 3;
    } else if(memcmp(init, "1200", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_1200;
        init += 4;
    } else if(memcmp(init, "2400", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_2400;
        init += 4;
    } else if(memcmp(init, "4800", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_4800;
        init += 4;
    } else if(memcmp(init, "9600", 4) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_9600;
        init += 4;
    } else if(memcmp(init, "19200", 5) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_19200;
        init += 5;
    } else if(memcmp(init, "38400", 5) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_38400;
        init += 5;
    } else if(memcmp(init, "57600", 5) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_57600;
        init += 5;
    } else if(memcmp(init, "115200", 6) == 0) {
        sgmBaudRate = LL_PHYSICAL_V_BAUDRATE_115200;
        init += 6;
    }

    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    com.hPhyCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

    physConf =
        (com.chn == chnCom1 ? LL_PHYSICAL_V_COM0 :
         com.chn == chnCom1 ? LL_PHYSICAL_V_COM1 : LL_PHYSICAL_V_COM2);
    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK, physConf,
                            LL_PHYSICAL_L_LINK);

    CHECK(sgmBaudRate, lblKOConfigure);
    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BAUDRATE, sgmBaudRate,
                            LL_PHYSICAL_L_BAUDRATE);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BITS_PER_BYTE, datasize,
                            LL_PHYSICAL_L_BITS_PER_BYTE);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_STOP_BITS, stopbits,
                            LL_PHYSICAL_L_STOP_BITS);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_PARITY, parity,
                            LL_PHYSICAL_L_PARITY);

    ret = LL_Configure(&com.prm.hdl, com.hCfg);
    CHECK(ret == LL_ERROR_OK
          || ret == LL_ERROR_ALREADY_CONNECTED, lblKOConfigure);

    ret = 1;
    goto lblEnd;
  lblKOConfigure:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("comSetCom ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

/*!
 *\par
 * AS Comment: For HDLC in V22, change the AT command string.
 * \n
*/
static int comSetMdm(const char *init) {
    int ret;
    int ctr;
    byte datasize;
    byte parity;
    byte stopbits;
    int blindDial;
    unsigned long bps;
    byte fast;
    byte compression;
    byte correction;
    byte hangUpDtr;             // NOT IMPLEMENTED
    char InitStr[64 + 1];
    char initSettings[13 + 1];

    memset(InitStr, 0, sizeof(InitStr));

    trcFS("comSet %s\n", init);
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);
    com.prm.separator = '|';    //common for all types of chn
    com.prm.conn_delay = 4 * 100;   //4 Second delay
    com.prm.send_timeout = 30 * 100;
    com.prm.ifconn = FALSE;

    init =
        parseStr(initSettings, init, sizeof(initSettings), com.prm.separator);
    init = parseStr(InitStr, init, sizeof(InitStr), com.prm.separator);
    ctr = 0;

    if(com.chn == chnHdlc)
        memcpy(initSettings, "8N11200", 7);
    VERIFY(initSettings);

    //parse control string
    //optional compression field:
    if(initSettings[ctr] == '.') {
        compression = 1;
        ctr++;
    } else
        compression = 0;
    //optional correction field:
    if(initSettings[ctr] == '?') {
        correction = 1;
        ctr++;
    } else
        correction = 0;

    //optional hang up field:
    if(initSettings[ctr] == '#') {
        hangUpDtr = 1;
        ctr++;
    } else
        hangUpDtr = 0;

    switch (initSettings[ctr]) {    //lowercase means baud rate negotiation
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
    switch (initSettings[ctr]) {
      case 'T':
      case 't':
      case 'P':
      case 'p':
          ctr++;                //not used in Teluim
          break;
      default:
          break;                //if missing, don't increment control string
    }

    datasize = LL_PHYSICAL_V_8_BITS;
    switch (initSettings[ctr]) {    //number of data bits
      case '7':
          //datasize = LL_PHYSICAL_V_7_BITS; //Not supported directy by Telium modem
          com.prm.BperB = 1;
          break;
      case '8':
          datasize = LL_PHYSICAL_V_8_BITS;
          com.prm.BperB = 0;
          break;
      default:
          CHECK(!initSettings[ctr], lblKO);
    }
    ctr++;

    parity = LL_PHYSICAL_V_NO_PARITY;
    switch (initSettings[ctr]) {    //parity
      case 'N':
          parity = LL_PHYSICAL_V_NO_PARITY;
          com.prm.Parity = 0;
          break;
      case 'E':
          //parity = LL_PHYSICAL_V_EVEN_PARITY;
          com.prm.Parity = 'E';
          break;
      case 'O':
          //parity = LL_PHYSICAL_V_ODD_PARITY;
          com.prm.Parity = 'O';
          break;
      default:
          CHECK(!initSettings[ctr], lblKO);
    }
    ctr++;

    stopbits = LL_PHYSICAL_V_1_STOP;

    switch (initSettings[ctr]) {    //number of stop bits
      case '0':
          stopbits = LL_PHYSICAL_V_1_STOP;  // Sagem dosent support 0 stop bits from the definitions
          return (-1);
          break;
      case '1':
          stopbits = LL_PHYSICAL_V_1_STOP;
          break;
      case '2':
          stopbits = LL_PHYSICAL_V_2_STOP;
          break;
      default:
          CHECK(!initSettings[ctr], lblKO);
    }
    ctr++;

    bps = LL_PHYSICAL_V_BAUDRATE_1200;

    if(memcmp(&initSettings[ctr], "300", 3) == 0) {
        bps = (fast ? LL_PHYSICAL_V_BAUDRATE_300 : LL_PHYSICAL_V_BAUDRATE_1200);
        ctr += 3;
    } else if(memcmp(&initSettings[ctr], "1200", 4) == 0) {
        bps =
            (fast ? LL_PHYSICAL_V_BAUDRATE_1200 : LL_PHYSICAL_V_BAUDRATE_1200);
        ctr += 4;
    } else if(memcmp(&initSettings[ctr], "2400", 4) == 0) {
        bps =
            (fast ? LL_PHYSICAL_V_BAUDRATE_1200 : LL_PHYSICAL_V_BAUDRATE_2400);
        ctr += 4;
    } else if(memcmp(&initSettings[ctr], "9600", 4) == 0) {
        bps =
            (fast ? LL_PHYSICAL_V_BAUDRATE_1200 : LL_PHYSICAL_V_BAUDRATE_9600);
        ctr += 4;
    } else if(memcmp(&initSettings[ctr], "19200", 5) == 0) {
        bps =
            (fast ? LL_PHYSICAL_V_BAUDRATE_1200 : LL_PHYSICAL_V_BAUDRATE_19200);
        ctr += 5;
    } else
        //CHECK(initSettings[0] == 0, lblKO); //removed: added additonal Init string for telium

        blindDial = (initSettings[ctr] == 'D' ? 0 : 1);

    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    com.hPhyCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
                            LL_PHYSICAL_V_MODEM, LL_PHYSICAL_L_LINK);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BAUDRATE, bps,
                            LL_PHYSICAL_L_BAUDRATE);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BITS_PER_BYTE, datasize,
                            LL_PHYSICAL_L_BITS_PER_BYTE);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_STOP_BITS, stopbits,
                            LL_PHYSICAL_L_STOP_BITS);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_PARITY, parity,
                            LL_PHYSICAL_L_PARITY);

    if(com.chn == chnHdlc) {
        if(*InitStr) {
            TlvTree_AddChildInteger(com.hPhyCfg, LL_MODEM_T_TYPE,
                                    LL_MODEM_V_TYPE_STANDARD, LL_MODEM_L_TYPE);
            TlvTree_AddChildInteger(com.hPhyCfg, LL_MODEM_T_CMD_TERMINATOR,
                                    LL_MODEM_V_CMD_TERMINATOR_CR,
                                    LL_MODEM_L_CMD_TERMINATOR);
        } else {
            strcpy(InitStr, DEFAULT_INIT_STRING_SYNC);
        }
        TlvTree_AddChildString(com.hPhyCfg, LL_MODEM_T_INIT_STRING, InitStr);

    } else {
        VERIFY(com.chn == chnMdm);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_MODEM_T_CMD_TERMINATOR,
                                LL_MODEM_V_CMD_TERMINATOR_CR,
                                LL_MODEM_L_CMD_TERMINATOR);
        if(!*InitStr) {
            strcpy(InitStr, DEFAULT_INIT_STRING_ASYNC);
        }
        TlvTree_AddChildString(com.hPhyCfg, LL_MODEM_T_INIT_STRING, InitStr);
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comSetMdm ret=%d\n", ret);
    return ret;
    return -1;
}

static int comSetUSB(const char *init) {
    int ret;

    trcS("comSetUSB Beg\n");

    com.prm.ifconn = FALSE;

    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    com.hPhyCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
                            (unsigned char) LL_PHYSICAL_V_USB,
                            LL_PHYSICAL_L_LINK);

    ret = LL_Configure(&com.prm.hdl, com.hCfg);
    CHECK(ret == LL_ERROR_OK, lblKO);

    trcFN("comSetUSB ret=%d\n", ret);
    return 1;
  lblKO:
    return -1;
}

#ifdef __CHN_GPRS__
static int comSetGprs(const char *init) {
    char usr[LL_PPP_L_LOGIN_MAX + 1];
    char pwd[LL_PPP_L_PASSWORD_MAX + 1];
    char pin[8 + 1];
    char m_apn[64 + 1];
    byte tryOp, tryAtt;         //number of attempts to select operator and to attach network
   // int nStatus;		//ori from template
    int nStatus = -1;		//opa 06052010
    int nError;
    unsigned long nTick, nTotTimeout;
    unsigned long nTick2;
    int ret;

    VERIFY(com.chn == chnGprs);
    VERIFY(init);

    trcS("comSetGprs Beg\n");
    com.prm.separator = '|';    //common for all types of chn
    com.prm.conn_delay = 4 * 100;   //4 Second delay
    com.prm.send_timeout = 60 * 100;
    com.prm.ifconn = FALSE;
    com.prm.gprs.cTimeout = 120 * 100;
    com.prm.gprs.cValue = 10 * 100;
    tryOp = 5;                  //by default try 5 times to select operator
    tryAtt = 15;                //by default try 15 times to attach netwoerk

    init = parseStr(pin, init, sizeof(pin), com.prm.separator); // extract pin
    init = parseStr(m_apn, init, sizeof(m_apn), com.prm.separator); // extract apn
    init = parseStr(usr, init, sizeof(usr), com.prm.separator); // extract usr
    init = parseStr(pwd, init, sizeof(pwd), com.prm.separator); // extract password

    nTotTimeout = 0;
    nStatus = LL_GPRS_Start(*pin ? pin : NULL, m_apn);
    CHECK(nStatus == OK, lblKOAttach);

    nError = LL_Network_GetStatus(LL_PHYSICAL_V_GPRS, &nStatus);

    if(nStatus == LL_STATUS_GPRS_CONNECTED) {
        com.prm.ifconn = TRUE;
        goto lblOK;
    }
    if(nError == LL_ERROR_NETWORK_ERROR)
        goto lblKOAttach;

    while((nError == LL_ERROR_NETWORK_NOT_READY)
          && (nStatus != LL_STATUS_GPRS_AVAILABLE)) {
        nError = LL_Network_GetStatus(LL_PHYSICAL_V_GPRS, &nStatus);
        if(nStatus == LL_STATUS_GPRS_AVAILABLE)
            break;
        ttestall(0, com.prm.conn_delay);
        nTotTimeout += com.prm.conn_delay;
        if(nTotTimeout >= com.prm.gprs.cTimeout)
            goto lblKOAttach;
    }
    trcFN("comSetGprs - LL_GPRS_Start : %d\n\r", nStatus);

    nTick = get_tick_counter(); //Timer referance
    do {
        nStatus = LL_GPRS_Connect(m_apn, usr, pwd, com.prm.gprs.cValue);
        CHECK(nStatus == OK, lblKOAttach);
        nStatus = LL_Network_GetStatus(LL_PHYSICAL_V_GPRS, &nError);
        nTick2 = get_tick_counter();
        CHECK(nStatus != LL_STATUS_GPRS_CONNECTED, lblOK);
        switch (nError & 0x0F) {
          case 0:              // Connected
              trcS("CONNECTED\n\r");
              goto lblConnect;
              break;
          case 1:              // Disconnected
              //trcS("DISCONNECTED\n\r");
              break;
          case 2:              //  NO SIM
              trcS("NO SIM\n\r");
              ret = -1;
              goto lblEnd;
          case 3:              // BAD PIN
              trcS("BAD PIN\n\r");
              ret = -1;
              goto lblEnd;
          case 4:              // NO PIN
              trcS("NO PIN\n\r");
              ret = -1;
              goto lblEnd;
          case 5:              // PPP ERROR
              trcS("PPP ERROR\n\r");
              ret = -1;
              goto lblEnd;
          case 6:              // UNKNNOWN ERROR
              trcS("UNKNOWN ERROR\n\r");
              ret = -1;
              goto lblEnd;
        }
        CHECK((nError & 0x0F) >= 0, lblOK);
    } while((nTick2 - nTick) < com.prm.gprs.cTimeout);
  lblConnect:
    trcFN("comSetGprs - LL_GPRS_Connect : %d\n\r", nStatus);
    CHECK((nError & 0x0F) == 0, lblKOAttach);

    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    com.hPhyCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
                            (unsigned char) LL_PHYSICAL_V_GPRS,
                            LL_PHYSICAL_L_LINK);

//    if(*usr) {
//        com.hDLinkCfg =
//            TlvTree_AddChild(com.hCfg, LL_TAG_DATA_LINK_LAYER_CONFIG, NULL, 0);
//
//        TlvTree_AddChildInteger(com.hDLinkCfg, LL_DATA_LINK_T_PROTOCOL,
//                                LL_DATA_LINK_V_PPP, LL_DATA_LINK_L_PROTOCOL);
//
//        TlvTree_AddChildString(com.hDLinkCfg, LL_PPP_T_LOGIN, usr);
//        TlvTree_AddChildString(com.hDLinkCfg, LL_PPP_T_PASSWORD, pwd);
//    }
    ret = 1;
    goto lblEnd;
  lblKOAttach:
    ret = -1000 - (nError & 0x0F);
    goto lblEnd;
  lblOK:
    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    com.hPhyCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
                            (unsigned char) LL_PHYSICAL_V_GPRS,
                            LL_PHYSICAL_L_LINK);
    ret = 1;
  lblEnd:
    trcFN("#END comSetGPRS() : %d\n", ret);
    return ret;
}
#else
static int comSetGprs(const char *init) {
    return -1;
}
#endif
//
//static int comSetTcp(const char *init) {
//#ifdef __CHN_TCP__
//    int ret;
//    char adr[LL_TCPIP_L_HOST_NAME_MAX + 1]; //it can be IpAdr or url
//    char locAdr[LL_TCPIP_L_HOST_NAME_MAX + 1];  //IpAdr
//    char msk[15 + 1];
//    char port[9 + 1];
//    card dPort;
//    char gateway[15 + 1];       // for TAMAS
//    unsigned int local_addr = 0;
//    unsigned int netmask_addr = 0;
//    unsigned int gateway_addr = 0;
//    char *p;
//    char buf[64];
//
//#ifndef _SIMULPC_
//    int sta;
//#endif
//
//    trcS("comSetTcp Beg\n");
//    VERIFY((com.chn == chnTcp5100) || (com.chn == chnTcp7780));
//    VERIFY(init);
//    com.prm.separator = '|';    //common for all types of chn
//    com.prm.conn_delay = 4 * 100;   //4 Second delay
//
//    init = parseStr(adr, init, sizeof(adr), com.prm.separator);
//    init = parseStr(port, init, sizeof(port), com.prm.separator);
//    ret = dec2num(&dPort, port, 0);
//    init = parseStr(msk, init, sizeof(msk), com.prm.separator);
//    init = parseStr(gateway, init, sizeof(gateway), com.prm.separator);
//    init = parseStr(locAdr, init, sizeof(locAdr), com.prm.separator);
//
//    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);
//
//    com.hPhyCfg =
//        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);
//
//    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
//                            LL_PHYSICAL_V_ETHERNET, LL_PHYSICAL_L_LINK);
//
//    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BAUDRATE,
//                            LL_PHYSICAL_V_BAUDRATE_115200,
//                            LL_PHYSICAL_L_BAUDRATE);
//
//    com.hTransCfg =
//        TlvTree_AddChild(com.hCfg, LL_TAG_TRANSPORT_LAYER_CONFIG, NULL, 0);
//
//    TlvTree_AddChildInteger(com.hTransCfg, LL_TRANSPORT_T_PROTOCOL,
//                            LL_TRANSPORT_V_TCPIP, LL_TRANSPORT_L_PROTOCOL);
//
//    TlvTree_AddChildString(com.hTransCfg, LL_TCPIP_T_HOST_NAME, adr);
//
//    TlvTree_AddChildInteger(com.hTransCfg, LL_TCPIP_T_PORT,
//                            (unsigned int) dPort, LL_TCPIP_L_PORT);
//
//    ret = LL_Configure(&com.prm.hdl, com.hCfg);
//    CHECK(ret == LL_ERROR_OK, lblKO);
//
//#ifndef _SIMULPC_
//    ret = LL_Network_GetStatus(LL_PHYSICAL_V_ETHERNET, &sta);
//    CHECK(ret == LL_ERROR_OK, lblKO);
//    CHECK(sta == LL_STATUS_ETHERNET_AVAILABLE, lblKO);
//#endif
//
//    EthernetGetOption(ETH_IFO_ADDR, &local_addr);
//    p = (char *) &local_addr;
//    sprintf(buf, "Local Addr: %d.%d.%d.%d\n", (unsigned char) p[0],
//            (unsigned char) p[1], (unsigned char) p[2], (unsigned char) p[3]);
//    trcS(buf);
//
//    if(*locAdr) {
//        local_addr = __inet_addr(locAdr);   //Force local adress
//        EthernetSetOption(ETH_IFO_ADDR, &local_addr);
//
//        EthernetGetOption(ETH_IFO_ADDR, &local_addr);
//        p = (char *) &local_addr;
//        sprintf(buf, "Local Addr: %d.%d.%d.%d\n", (unsigned char) p[0],
//                (unsigned char) p[1], (unsigned char) p[2],
//                (unsigned char) p[3]);
//        trcS(buf);
//    }
//
//    if(*msk) {
//        //EthernetGetOption( ETH_IFO_NETMASK, &netmask_addr );
//        netmask_addr = __inet_addr(msk);    //Force msk
//        EthernetSetOption(ETH_IFO_NETMASK, &netmask_addr);
//    }
//    if(*gateway) {
//        gateway_addr = __inet_addr(gateway);    //Force msk
//        EthernetSetOption(ETH_IFO_GATEWAY, &gateway_addr);
//
//    }
//
//    ret = 1;
//    goto lblEnd;
//  lblKO:
//    trcErr(ret);
//    ret = -1;
//  lblEnd:
//    trcFN("comSetTcp() : %d\n", ret);
//    return ret;
//#else
//    return -1;
//#endif
//}


static int comSetTcp(const char *init) {
#ifdef __CHN_TCP__
    int ret;
    char adr[LL_TCPIP_L_HOST_NAME_MAX + 1]; //it can be IpAdr or url
    char msk[15 + 1];
    char gateway[15 + 1];       // for TAMAS
    unsigned int local_addr = 0;
    unsigned int netmask_addr = 0;
    unsigned int gateway_addr = 0;
    char *p;
    char buf[64];

#ifndef _SIMULPC_
    int sta;
#endif

    trcS("comSetTcp Beg\n");
    VERIFY((com.chn == chnTcp5100) || (com.chn == chnTcp7780));
    VERIFY(init);
    com.prm.separator = '|';    //common for all types of chn
    com.prm.conn_delay = 4 * 100;   //4 Second delay

    init = parseStr(adr, init, sizeof(adr), com.prm.separator);
    init = parseStr(msk, init, sizeof(msk), com.prm.separator);
    init = parseStr(gateway, init, sizeof(gateway), com.prm.separator);

    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    com.hPhyCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
                            LL_PHYSICAL_V_ETHERNET, LL_PHYSICAL_L_LINK);

    TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BAUDRATE,
                            LL_PHYSICAL_V_BAUDRATE_115200,
                            LL_PHYSICAL_L_BAUDRATE);

#ifndef _SIMULPC_
    ret = LL_Network_GetStatus(LL_PHYSICAL_V_ETHERNET, &sta);
    CHECK(ret == LL_ERROR_OK, lblKO);
    CHECK(sta == LL_STATUS_ETHERNET_AVAILABLE, lblKO);
#endif

    EthernetGetOption(ETH_IFO_ADDR, &local_addr);
    p = (char *) &local_addr;
    sprintf(buf, "Local Addr: %d.%d.%d.%d\n", (unsigned char) p[0],
            (unsigned char) p[1], (unsigned char) p[2], (unsigned char) p[3]);
    trcS(buf);

    if(*adr) {
        local_addr = __inet_addr(adr);  //Force local adress
        EthernetSetOption(ETH_IFO_ADDR, &local_addr);

        EthernetGetOption(ETH_IFO_ADDR, &local_addr);
        p = (char *) &local_addr;
        sprintf(buf, "Local Addr: %d.%d.%d.%d\n", (unsigned char) p[0],
                (unsigned char) p[1], (unsigned char) p[2],
                (unsigned char) p[3]);
        trcS(buf);
    }

    if(*msk) {
        //EthernetGetOption( ETH_IFO_NETMASK, &netmask_addr );
        netmask_addr = __inet_addr(msk);    //Force msk
        EthernetSetOption(ETH_IFO_NETMASK, &netmask_addr);
    }
    if(*gateway) {
        gateway_addr = __inet_addr(gateway);    //Force msk
        EthernetSetOption(ETH_IFO_GATEWAY, &gateway_addr);

    }

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSetTcp() : %d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSetPpp(const char *init) {
#ifdef __CHN_PPP__
    int ret;
    char usr[LL_PPP_L_LOGIN_MAX + 1];
    char pwd[LL_PPP_L_PASSWORD_MAX + 1];
    char adr[LL_TCPIP_L_HOST_NAME_MAX + 1];
    char delai[7 + 1];
    char phone[32];             //= "00860922000";
    card dly;                   //= 60;
    unsigned int dns_addr = 0;
    char *p;
    char buf[64];

    VERIFY(com.chn == chnPpp);
    VERIFY(init);
    trcS("comSetPpp Beg\n");
    com.prm.separator = '|';    //common for all types of chn

    init = parseStr(phone, init, sizeof(phone), com.prm.separator);
    init = parseStr(usr, init, sizeof(usr), com.prm.separator);
    init = parseStr(pwd, init, sizeof(pwd), com.prm.separator);
    init = parseStr(delai, init, sizeof(delai), com.prm.separator);
    dec2num(&dly, delai, 0);
    if(*init) {                 //if DNS sever adress parse it
        init = parseStr(adr, init, sizeof(adr), com.prm.separator);
    }

    com.prm.conn_delay = dly * 100; //dly seconds delay

    if(*adr) {
        ResolverGetOption(RSLVO_SERV1_IPADDR, &dns_addr);
        p = (char *) &dns_addr;
        sprintf(buf, "0 DNS srv Addr: %d.%d.%d.%d\n", (unsigned char) p[0],
                (unsigned char) p[1], (unsigned char) p[2],
                (unsigned char) p[3]);
        trcS(buf);

        dns_addr = __inet_addr(adr);    //Force local adress
        ResolverSetOption(RSLVO_SERV1_IPADDR, &dns_addr);

        ResolverGetOption(RSLVO_SERV1_IPADDR, &dns_addr);
        p = (char *) &dns_addr;
        sprintf(buf, "1 DNS srv Addr: %d.%d.%d.%d\n", (unsigned char) p[0],
                (unsigned char) p[1], (unsigned char) p[2],
                (unsigned char) p[3]);
        trcS(buf);
    }
    com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);

    if(*phone) {
        com.hCfg = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);
        com.hPhyCfg =
            TlvTree_AddChild(com.hCfg, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_LINK,
                                LL_PHYSICAL_V_MODEM, LL_PHYSICAL_L_LINK);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BAUDRATE,
                                LL_PHYSICAL_V_BAUDRATE_19200,
                                LL_PHYSICAL_L_BAUDRATE);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_BITS_PER_BYTE,
                                LL_PHYSICAL_V_8_BITS,
                                LL_PHYSICAL_L_BITS_PER_BYTE);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_STOP_BITS,
                                LL_PHYSICAL_V_1_STOP, LL_PHYSICAL_L_STOP_BITS);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_PHYSICAL_T_PARITY,
                                LL_PHYSICAL_V_NO_PARITY, LL_PHYSICAL_L_PARITY);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_MODEM_T_CMD_TERMINATOR,
                                LL_MODEM_V_CMD_TERMINATOR_CR,
                                LL_MODEM_L_CMD_TERMINATOR);
        TlvTree_AddChildString(com.hPhyCfg, LL_MODEM_T_INIT_STRING,
                               DEFAULT_INIT_STRING_ASYNC);
        TlvTree_AddChildString(com.hPhyCfg, LL_MODEM_T_PHONE_NUMBER, phone);
        TlvTree_AddChildInteger(com.hPhyCfg, LL_MODEM_T_DIAL_TIMEOUT,
                                LL_MODEM_V_DIAL_TIMEOUT_DEFAULT,
                                LL_MODEM_L_DIAL_TIMEOUT);
    }

    com.hDLinkCfg =
        TlvTree_AddChild(com.hCfg, LL_TAG_DATA_LINK_LAYER_CONFIG, NULL, 0);
    TlvTree_AddChildInteger(com.hDLinkCfg, LL_DATA_LINK_T_PROTOCOL,
                            LL_DATA_LINK_V_PPP, LL_DATA_LINK_L_PROTOCOL);
    TlvTree_AddChildString(com.hDLinkCfg, LL_PPP_T_LOGIN, usr);
    TlvTree_AddChildString(com.hDLinkCfg, LL_PPP_T_PASSWORD, pwd);

    ret = LL_Configure(&com.prm.hdl, com.hCfg);
    CHECK(ret == LL_ERROR_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSetPpp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSetSck(const char *init) {
#ifdef __CHN_SCK__
    int ret;
    char adr[LL_TCPIP_L_HOST_NAME_MAX + 1];
    char port[9 + 1];
    card dPort;

    VERIFY(com.chn == chnSck);
    VERIFY(init);
    trcS("comSetSck Beg\n");
    com.prm.separator = '|';    //common for all types of chn
    init = parseStr(adr, init, sizeof(adr), com.prm.separator);
    init = parseStr(port, init, sizeof(port), com.prm.separator);
    if(*port) {
        ret = dec2num(&dPort, port, 0);
        com.prm.peeraddr_in.sin_port = htons(dPort);
    }
    if(*adr) {
        com.prm.peeraddr_in.sin_addr.s_addr = __inet_addr(adr);
    }

    com.prm.peeraddr_in.sin_family = AF_INET;

    ret = 1;
    goto lblEnd;
    //lblKO:
    //  trcErr(ret);
    //  ret = -1;
  lblEnd:
    trcFN("comSetSck ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSetSsl(const char *init) {
#ifdef __CHN_SSL__
    int ret = 0;
    char ProfNum[2 + 1];

    //char name[PROFILE_NAME_SIZE + 1];
    char keyFile[64 + 1];
    char crtFile[64 + 1];
    char caFile[64 + 1];

    VERIFY(com.chn == chnSsl);
    VERIFY(init);
    trcS("comSetSsl Beg\n");
    com.prm.separator = '|';    //common for all types of chn
    init = parseStr(ProfNum, init, sizeof(ProfNum), com.prm.separator);
    init = parseStr(keyFile, init, sizeof(keyFile), com.prm.separator);
    init = parseStr(crtFile, init, sizeof(crtFile), com.prm.separator);
    init = parseStr(caFile, init, sizeof(caFile), com.prm.separator);

    if(*ProfNum) {
        sprintf(com.prm.SslProfName, "PROFILE_%s", ProfNum);
    } else {                    //default
        memcpy(com.prm.SslProfName, "PROFILE_04", PROFILE_NAME_SIZE);
    }
    com.prm.hdlProfile = SSL_NewProfile(com.prm.SslProfName, &ret);
    if(com.prm.hdlProfile == NULL) {
        SSL_UnloadProfile(com.prm.SslProfName);
        SSL_DeleteProfile(com.prm.SslProfName);
        com.prm.hdlProfile = SSL_NewProfile(com.prm.SslProfName, &ret);
    }
    CHECK(com.prm.hdlProfile != NULL, lblLoad);

    ret = SSL_ProfileSetProtocol(com.prm.hdlProfile, SSLv3);
    CHECK(ret == SSL_PROFILE_EOK, lblKO);

    ret = SSL_ProfileSetCipher(com.prm.hdlProfile,
                               SSL_RSA | SSL_DES | SSL_3DES | SSL_RC4 | SSL_RC2
                               | SSL_MAC_MASK | SSL_SSLV3,
                               SSL_HIGH | SSL_NOT_EXP);
    CHECK(ret == SSL_PROFILE_EOK, lblKO);

    if(!*keyFile) {             //default
        memset(keyFile, '\0', sizeof(keyFile));
    }
    ret = SSL_ProfileSetKeyFile(com.prm.hdlProfile, keyFile, FALSE);
    CHECK(ret == SSL_PROFILE_EOK, lblKO);

    if(!*crtFile) {             //default
        memset(crtFile, '\0', sizeof(crtFile));
    }
    ret = SSL_ProfileSetCertificateFile(com.prm.hdlProfile, crtFile);
    CHECK(ret == SSL_PROFILE_EOK, lblKO);

    if(!*caFile) {              //default
        memcpy(caFile, "/SYSTEM/CA.CRT", 14);
    }
    ret = SSL_ProfileAddCertificateCA(com.prm.hdlProfile, caFile);
    CHECK(ret == SSL_PROFILE_EOK, lblKO);

    ret = SSL_SaveProfile(com.prm.hdlProfile);
    CHECK(ret == SSL_PROFILE_EOK, lblKO);

  lblLoad:
    com.prm.hdlProfile = SSL_LoadProfile(com.prm.SslProfName);
    CHECK(com.prm.hdlProfile != NULL, lblKO);

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
 *      - It has to have the "adress-port-mask-gateway" format (the '|' is the separator).
 *  - PPP connection
 *    - The separator character is '|'. The fields:
 *      - Phone: pohe number
 *      - User: user id
 *      - Password: password of the user
 *      - Timeout (in ASCI): timeout of connection in second
 *      - DNS server address (optional): Address of DNS server
 *  - GPRS connection
 *    - The separator character is '|'. The fields:
 *      - Phone: phone number
 *      - Apn: apn adress
 *      - User: user id
 *      - Password: password of the user
 *      - IP: ip adress
 *  - Socket connection
 *    - The separator character is '|'. The fields:
 *      - IP: host ip adress
 *      - IP: host ip port
 * - SSL connection
 *		- The separator character is '|'. The fields:
 *			- SSL: Profile number (00-19)
 *			- SSL: Key File path and name
 *			- SSL: Certificate File path and name
 *			- SSL: Certificate CA path and name
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
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
      case chnUSB:
          return (comSetUSB(init));
      case chnSck:
          return (comSetSck(init));
      case chnSsl:
          return (comSetSsl(init));
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comDialUSB(const char *srv) {
    int ret = -1;

    trcS("comDialUSB Beg\n");
    VERIFY(com.chn == chnUSB || com.chn == chnCom1 || com.chn == chnCom2
           || com.chn == chnCom3);

    ret = LL_Connect(com.prm.hdl);
    CHECK(ret == LL_ERROR_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    trcErr(ret);
  lblEnd:
    trcFN("comDialUSB() : %d\n", ret);
    return (ret);
}

static int comDialMdm(const char *phone) {
    int ret;
    int len;
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
    memset(buf, 0, sizeof(buf));
    strcpy(buf, pabx);
    strcat(buf, prefix);

    if(*country) {
        strcat(buf, ",");
        strcat(buf, country);
        strcat(buf, ",");
    }

    strcat(buf, phone);

    TlvTree_AddChildString(com.hPhyCfg, LL_MODEM_T_PHONE_NUMBER, buf);

    if(com.chn == chnMdm) {
        TlvTree_AddChildInteger(com.hPhyCfg, LL_MODEM_T_DIAL_TIMEOUT,
                                LL_MODEM_V_DIAL_TIMEOUT_DEFAULT,
                                LL_MODEM_L_DIAL_TIMEOUT);
    }

    if(com.chn == chnHdlc) {
        com.hDLinkCfg =
            TlvTree_AddChild(com.hCfg, LL_TAG_DATA_LINK_LAYER_CONFIG, NULL, 0);
        TlvTree_AddChildInteger(com.hDLinkCfg, LL_DATA_LINK_T_PROTOCOL,
                                LL_DATA_LINK_V_HDLC, LL_DATA_LINK_L_PROTOCOL);
        TlvTree_AddChildInteger(com.hDLinkCfg, LL_HDLC_T_MIN_RESEND_REQUESTS, 2,
                                LL_HDLC_L_MIN_RESEND_REQUESTS);
    }

    ret = LL_Configure(&com.prm.hdl, com.hCfg);
    CHECK(ret == LL_ERROR_OK, lblKOConfigure);

    ret = LL_Connect(com.prm.hdl);
    CHECK(ret == LL_ERROR_OK, lblKOConnect);

    com.prm.ll_conn_delay = get_tick_counter();

    return (ret);
  lblKOConfigure:
    return (-comKO);
  lblKOConnect:
    switch (ret) {
      case LL_MODEM_ERROR_SEND_BUFFER_OVERRUN:
      case LL_MODEM_ERROR_RESPONSE_ERROR:
      case LL_MODEM_ERROR_RESPONSE_BLACKLISTED:
      case LL_MODEM_ERROR_RESPONSE_DELAYED:
      case LL_MODEM_ERROR_RESPONSE_INVALID:
          ret = -comKO;
          break;
      case LL_MODEM_ERROR_RESPONSE_NO_DIALTONE:
          ret = -comNoDialTone;
          break;
      case LL_MODEM_ERROR_RESPONSE_BUSY:
      case LL_MODEM_ERROR_RESPONSE_NO_ANSWER:
          ret = -comBusy;
          break;
      case LL_MODEM_ERROR_RESPONSE_NO_CARRIER:
          ret = -comNoCarrier;
          break;
    }
    return (ret);
}

static int comDialCom(const char *srv) {
    int ret;

    VERIFY(com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);
    //trcFS("comDialCom srv = %s\n", srv);

    ret = LL_Connect(com.prm.hdl);
    CHECK(ret == LL_ERROR_OK || ret == LL_ERROR_ALREADY_CONNECTED, lblKO);

    return (ret);
  lblKO:
    return -1;
}

static int comDialIp(const char *srv) {
    char IpAdr[LL_TCPIP_L_HOST_NAME_MAX + 1];   //it can be IpAdr or url
    card dPort;
    char port[10];
    int ret = -1;

    trcFS("comDialIp() : %s\n", srv);
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs);
    VERIFY(srv);
    CHECK(srv, lblKO);

    srv = parseStr(IpAdr, srv, sizeof(IpAdr), com.prm.separator);
    (void) parseStr(port, srv, sizeof(port), com.prm.separator);
    ret = dec2num(&dPort, port, 0);

    trcFS("IP : %s", IpAdr);
    trcFS("   Port : %s\n", port);

    com.hTransCfg = TlvTree_AddChild(com.hCfg, LL_TAG_TRANSPORT_LAYER_CONFIG,   // TAG Transport layer parameters
                                     NULL,  // VALUE (Null)
                                     0);    // LENGTH 0

    TlvTree_AddChildInteger(com.hTransCfg, LL_TRANSPORT_T_PROTOCOL, // TAG
                            LL_TRANSPORT_V_TCPIP,   // VALUE
                            LL_TRANSPORT_L_PROTOCOL);   // LENGTH 1 byte

    TlvTree_AddChildString(com.hTransCfg, LL_TCPIP_T_HOST_NAME, IpAdr);
    TlvTree_AddChildInteger(com.hTransCfg, LL_TCPIP_T_PORT, dPort,
                            LL_TCPIP_L_PORT);

    if(com.chn == chnGprs) {
#ifdef __CHN_GPRS__
        TlvTree_AddChildInteger(com.hTransCfg, LL_TCPIP_T_CONNECT_TIMEOUT,
                                (unsigned int) com.prm.gprs.cTimeout,
                                LL_TCPIP_L_CONNECT_TIMEOUT);
#endif
    }
    //lblConn:
    ret = LL_Configure(&com.prm.hdl, com.hCfg);
    CHECK(ret == LL_ERROR_OK, lblKO); 
    ret = LL_Connect(com.prm.hdl);  
    CHECK(ret == LL_ERROR_OK || ret == LL_ERROR_NETWORK_ALREADY_CONNECTED
          || ret == LL_ERROR_ALREADY_CONNECTED, lblKO);

    com.prm.ll_conn_delay = get_tick_counter();

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comDialIp() : %d\n", ret);
    return (ret);
}

static int comDialSck(const char *init) {
#ifdef __CHN_SCK__
    int ret;
    char adr[LL_TCPIP_L_HOST_NAME_MAX + 1];
    char port[9 + 1];
    card dPort;

    VERIFY(com.chn == chnSck);
    VERIFY(init);
    trcS("comDialSck Beg\n");
    com.prm.separator = '|';    //common for all types of chn
    init = parseStr(adr, init, sizeof(adr), com.prm.separator);
    init = parseStr(port, init, sizeof(port), com.prm.separator);
    if(*port) {
        ret = dec2num(&dPort, port, 0);
        com.prm.peeraddr_in.sin_port = htons(dPort);
    }
    if(*adr) {
        com.prm.peeraddr_in.sin_addr.s_addr = __inet_addr(adr);
    }
    com.prm.peeraddr_in.sin_family = AF_INET;

    ret =
        connect(com.prm.hdlSck, (const struct sockaddr *) &com.prm.peeraddr_in,
                sizeof(struct sockaddr_in));
    CHECK(ret == LL_ERROR_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comDialSck ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comDialSsl(const char *init) {
#ifdef __CHN_SSL__
    int ret;
    char adr[LL_TCPIP_L_HOST_NAME_MAX + 1];
    char port[9 + 1];
    card dPort;

    VERIFY(com.chn == chnSsl);
    VERIFY(init);
    trcS("comDialSsl Beg\n");

    memset(adr, 0, sizeof(adr));
    memset(port, 0, sizeof(port));
    dPort = 0;
    com.prm.separator = '|';    //common for all types of chn

    init = parseStr(adr, init, sizeof(adr), com.prm.separator);
    init = parseStr(port, init, sizeof(port), com.prm.separator);
    if(*port) {
        ret = dec2num(&dPort, port, 0);
    }

    ret = SSL_New(&com.prm.hdlSsl, com.prm.hdlProfile);
    CHECK(ret == 0, lblKO);

    ret = SSL_Connect(com.prm.hdlSsl, adr, dPort, 2000);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comDialSsl ret=%d\n", ret);
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
 *   - For internal modem phone number containing pabx and prefix separated by ‘-‘:
 *     srv="0-00-1234" consists of pabx="0", prefix="00" and phone itself "1234"
 *     srv="0-1234" consists of pabx="0" and phone itself "1234" without prefix
 *     srv="1234" does not contain any pabx and prefix
 *   - For TCP it is TCP/IP address of the server and the port number in the format like:
 *     "10.10.59.27|2000"
 *   - For IP channels (TCP and PPP) this parameter contains IP address and port to connect
 *     in the format NNN.NNN.NNN.NNN|PPPPP
 *   - For PPP channel address can be replaced by hostname in ASCII, port value can contain empty value
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
 * \source sys\\tlmchn.c
 * \test tcab0008.c
 */
int comDial(const char *srv) {
    VERIFY(srv);
    trcFS("comDial srv=%s\n", srv);
    switch (com.chn) {
      case chnCom1:
      case chnCom2:
      case chnCom3:
          return comDialCom(srv);
      case chnMdm:
      case chnHdlc:
          return comDialMdm(srv);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comDialIp(srv);
      case chnUSB:
          return (comDialUSB(srv));
      case chnSck:
          return (comDialSck(srv));
      case chnSsl:
          return (comDialSsl(srv));
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
    //CHECK(com.prm.hdl, lblKO); //It is zero in file simulation mode
    ret = _comCheckConnected();
    CHECK(ret, lblKO);

    if(com.prm.Parity) {
        tbxParity(&b, 1, 8, com.prm.Parity);
    }

    ret = LL_Send(com.prm.hdl, 1, &b, 10);
    CHECK(ret == 1, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comSendIp(byte b) {
    int ret;

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 || com.chn == chnPpp
           || com.chn == chnGprs);
    //CHECK(com.prm.hdl, lblKO); //It is zero in file simulation mode

    ret = LL_Send(com.prm.hdl, 1, &b, 10);
    CHECK(ret == 1, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int comSendSck(byte b) {
#ifdef __CHN_SCK__
    int ret;

    VERIFY(com.chn == chnSck);
    CHECK(com.prm.hdlSck, lblKO);
    ret = send(com.prm.hdlSck, &b, 1, TCP_NODELAY);
    CHECK(ret == 1, lblKO);
    return 1;
  lblKO:
#endif
    return -1;
}

static int comSendSsl(byte b) {
#ifdef __CHN_SSL__
    int ret;

    VERIFY(com.chn == chnSsl);
    CHECK(com.prm.hdlSsl, lblKO);
    ret = SSL_Write(com.prm.hdlSsl, &b, 1, 300);
    CHECK(ret == 1, lblKO);
    return 1;
  lblKO:
#endif
    return -1;
}

/** Send a byte b to the communication channel.
 * \param b (I) Byte to send
 * \return
 *    - negative if failure.
 *    - zero if timeout; otherwise positive.
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
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
      case chnSck:
          return comSendSck(b);
      case chnSsl:
          return comSendSsl(b);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSendBufCom(const byte * msg, word len) {
#ifdef __CHN_COM__
    int ret;

    if(com.prm.Parity) {
        tbxParity((byte *) msg, (byte) len, 8, com.prm.Parity);
    }

    ret = LL_Send(com.prm.hdl, len, msg, LL_INFINITE);
    CHECK(ret == len, lblKO);
    ttestall(0, 100);           //to make sure sending is finished before receiving

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
    int ret = 0;

    ret = LL_Send(com.prm.hdl, len, msg, com.prm.send_timeout);
    CHECK(ret == len, lblKO);
    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    trcFN("comSendBufIp ret = %d\n", ret);
    return ret;
}

static int comSendBufSck(const byte * msg, word len) {
    int ret = -1;

#ifdef __CHN_SCK__

    CHECK(com.prm.hdlSck, lblKO);
    ret = send(com.prm.hdlSck, &msg, len, TCP_NODELAY);
    CHECK(ret == len, lblKO);
    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    trcFN("comSendBufIp ret = %d\n", ret);
#endif
    return ret;
}

static int comSendBufSsl(const byte * msg, word len) {
    int ret = -1;

#ifdef __CHN_SSL__

    //CHECK(com.prm.hdlSsl, lblKO);
    ret = SSL_Write(com.prm.hdlSsl, msg, len, 300);
    CHECK(ret == len, lblKO);
    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    trcFN("comSendBufSsl ret = %d\n", ret);
#endif
    return ret;
}

/** Send len  bytes from  msg to the communication port.
 * \param msg (I) Pointer to buffer to send
 * \param len (I) Number of bytes to send
 * \return
 *    - negative if failure.
 *    - zero if timeout.
 *    - otherwise positive.
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
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
      case chnUSB:
          return comSendBufCom(msg, len);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comSendBufIp(msg, len);
      case chnSck:
          return comSendBufSck(msg, len);
      case chnSsl:
          return comSendBufSsl(msg, len);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comRecvIp(byte * b, int dly) {
    int ret = -1;

    VERIFY(b);
    trcFN("comRecvIp : %d\n", dly);
    CHECK(com.prm.hdl, lblKO);

    ret = LL_Receive(com.prm.hdl, 1, b, dly * 100);
    CHECK(ret == 1, lblKO);

    goto lblEnd;
  lblKO:
    ret = -1;
    trcErr(ret);
  lblEnd:
    trcFN("comRecvIp : %d\n", ret);
    return (ret);
}
static int comRecvCom(byte * b, int dly) {
#ifdef __CHN_COM__
    int ret = 0;

    VERIFY(b);
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);

    ret = _comCheckConnected();
    CHECK(ret, lblKO);

    ret = LL_Receive(com.prm.hdl, 1, b, dly * 100);
    CHECK(ret == 1, lblKO);

    if(com.prm.Parity) {
        ret = tbxCheckParity(b, 1, 8, com.prm.Parity);
        CHECK(ret == 1, lblKO);
        *b = (*b & 0x7F);
    }

    return (ret);
  lblKO:
    trcFN("comRecvCom ret=%d\n", ret);
    return -1;
#else
    return -1;
#endif
}

static int comRecvSck(byte * b, int dly) {
#ifdef __CHN_SCK__
    int ret = 0;

    VERIFY(b);
    VERIFY(com.chn == chnSck);
    CHECK(com.prm.hdlSck, lblKO);
    ret = recv(com.prm.hdlSck, &b, 1, dly * 100);
    CHECK(ret >= 0, lblKO);

    return (ret);
  lblKO:
    trcFN("comRecvSck ret=%d\n", ret);
    return -1;
#else
    return -1;
#endif
}

static int comRecvSsl(byte * b, int dly) {
#ifdef __CHN_SSL__
    int ret = 0;

    VERIFY(b);
    VERIFY(com.chn == chnSsl);
    CHECK(com.prm.hdlSsl, lblKO);
    ret = SSL_Read(com.prm.hdlSsl, &b, 1, dly * 100);
    CHECK(ret >= 0, lblKO);

    return (ret);
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
 * \source sys\\tlmchn.c
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
      case chnUSB:
          return comRecvIp(b, dly);
      case chnSck:
          return comRecvSck(b, dly);
      case chnSsl:
          return comRecvSsl(b, dly);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

/*!
 *\par Comment: the NULL charracter is not considered as a terminator
 * Therefore, this function allows to receive datas with NULL character inserted
 * See tcas0001()
 * \n
*/
static int comRecvBufCom(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_COM__
    int ret;
    byte ctl[1 + 16];
    unsigned char rsp[RSP_LEN];
    word len;
    card counter;
    byte b;
    byte index;

    memset(&rsp, 0, sizeof(rsp));
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);

    memset(ctl, 0, 1 + 16);
    if(trm) {
        *ctl = strlen((char *) trm);
        strcpy((char *) ctl + 1, (char *) trm); //zero is added at the end
        if(msg)
            len = bufDim(msg);
    }

    if(trm) {
        counter = 0;
        while(counter < RSP_LEN) {
            ret = comRecvCom(&b, dly);
            CHECK((ret > 0), lblAppend);
            for (index = 0; index < 16; index++) {
                if((b == ctl[index]) && (b != 0)) {
                    rsp[counter++] = b;
                    goto lblAppend;
                }
            }
            if(com.prm.Parity) {
                ret = tbxCheckParity(&b, 1, 1, com.prm.Parity); //bitTest func starts from index 1
                CHECK(ret == 1, lblKO);
                b = b & 0x7F;
            }
            rsp[counter] = b;
            counter++;
        }
    } else {
        ret =
            LL_Receive(com.prm.hdl, (int) bufDim(msg), (void *) bufPtr(msg),
                       dly * 100);
        ttestall(0, 100);
        if(ret > 0)
            msg->pos = (word) ret;
        counter = ret;
        CHECK((ret > 0), lblKO);
        if(com.prm.Parity) {
            for (index = 0; index < counter; index++) {
                ret = tbxCheckParity(&msg->ptr[index], 1, 1, com.prm.Parity);   //bitTest func starts from index 1
                CHECK(ret == 1, lblKO);
                msg->ptr[index] = msg->ptr[index] & 0x7F;
            }
        }
        goto lblEnd;
    }
  lblAppend:
    if(msg) {
        ret = bufApp(msg, (byte *) rsp, counter);
        CHECK(ret >= 0, lblKO);
    }
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

static int comRecvBufUSB(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_USB__
    int ret = -1;

    trcS("comRecvBufUSB Beg\n");
    VERIFY(msg);

    bufReset(msg);
    ret =
        LL_Receive(com.prm.hdl, (int) bufDim(msg), (void *) bufPtr(msg),
                   dly * 100);
    CHECK(ret > 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    if(ret == LL_ERROR_TIMEOUT)
        ret = 0;
    goto lblEnd;
  lblEnd:
    trcFN("comRecvBufUSB: ret=%d\n", ret);
    return (ret);
#else
    return -1;
#endif
}

#if 0 //@agmr - ganti dengan yang dibawah
static int comRecvBufGprs(tBuffer * msg, const byte * trm, byte dly) {
    int ret = 0;
    struct timeval tim;

    VERIFY(msg);
    bufReset(msg);

    tmrStart(0, dly * 100);
    tim.tv_sec = dly;
    tim.tv_usec = 0;
    ret =
        LL_Receive(com.prm.hdl, (int) bufDim(msg), (void *) bufPtr(msg),
                   dly * 100);
    if(ret > 0)
        msg->pos = (word) ret;
    CHECK(tmrGet(0), lblKO);
    goto lblEnd;
  lblKO:
    trcFN("comRecvBufGprs ret=%d\n", ret);
    ret = -1;
  lblEnd:
    tmrStop(0);
    return ret;
}
#endif 

static int comRecvBufGprs(tBuffer * msg, const byte * trm, byte dly) {
    int ret = 0;
    struct timeval tim;
    word expectedLen; //@agmr
    byte buf[2];      //@agmr
    int delay2;
    int lenData = 0;

    VERIFY(msg);
    bufReset(msg);

#ifdef TEST_TERIMA_GPRS   
    dly = 1000;
#endif

    tmrStart(0, dly * 100);
    tim.tv_sec = dly;
    tim.tv_usec = 0;
    ret =
        LL_Receive(com.prm.hdl, (int) bufDim(msg), (void *) bufPtr(msg),
                   dly * 100);

#ifdef TEST_TERIMA_GPRS                   
    hex_dump_char("recv 1", bufPtr(msg), ret); //hapus
#endif
    
    if(ret > 0)   
    {        
        lenData = ret;
        msg->pos = (word) ret; 
        buf[0] = *(bufPtr(msg));
        buf[1] = *(bufPtr(msg)+1);
        expectedLen = buf[0]*0xff + buf[1];

#ifdef TEST_TERIMA_GPRS         
{
    char buf[50];
    
    sprintf(buf,"len1 %i explen %i",ret,expectedLen);
    prtS(buf);
    
    sprintf(buf,"pos %i ",msg->pos);
    prtS(buf);    
}        
#endif    
           
        if(ret < expectedLen)
        {
            ret = tmrGet(0);
            if(ret <= 0)
                goto lblKO;
            tmrStop(0);
            delay2 = ret;
            
            tmrStart(0, delay2);     
            ret = LL_Receive(com.prm.hdl, (int) bufDim(msg)-lenData, (void *) (bufPtr(msg)+msg->pos),
                   delay2);  
#ifdef TEST_TERIMA_GPRS                     
            hex_dump_char("recv 2",(bufPtr(msg)+msg->pos), ret); //hapus
#endif                 
            if(ret > 0)
            {                
                lenData += ret;   
                msg->pos += (word) lenData;
                ret = lenData;
            }
                
        }
    }     
    CHECK(tmrGet(0), lblKO); //nanti buka
    goto lblEnd;
  lblKO:
    trcFN("comRecvBufGprs ret=%d\n", ret);
    ret = -1;
  lblEnd:
    tmrStop(0);
    return ret;
}

static int comRecvBufIp(tBuffer * msg, const byte * trm, byte dly) {
    int ret;
    word dim, idx;
    byte b = 0x00;

    tmrStart(0, dly * 100);
    if(trm) {
        dim = 0;
        while(tmrGet(0)) {
            if(dim >= bufDim(msg))
                break;
            ret = LL_Receive(com.prm.hdl, 1, msg, dly * 100);
            if(ret != 1)
                continue;
            if(msg) {
                ret = bufSet(msg, b, 1);
                CHECK(ret >= 0, lblKO);
            }
            dim++;
            if(!trm)
                continue;
            idx = 0;
            while(trm[idx]) {   //is it a terminator character
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
    } else {
        ret =
            LL_Receive(com.prm.hdl, (int) bufDim(msg), (void *) bufPtr(msg),
                       dly * 100);
        if(ret > 0)
            msg->pos = (word) ret;
        CHECK(tmrGet(0), lblKO);
        goto lblEnd;
    }
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comRecvBufIp ret=%d\n", ret);
    tmrStop(0);
    return ret;
}

static int comRecvBufSck(tBuffer * msg, const byte * trm, byte dly) {
    int ret = -1;

#ifdef __CHN_SCK__
    word dim, idx;
    byte b = 0x00;

    CHECK(com.prm.hdlSck, lblKO);
    tmrStart(0, dly * 100);
    dim = 0;
    while(tmrGet(0)) {
        if(dim >= bufDim(msg))
            break;
        ret = recv(com.prm.hdlSck, &b, 1, dly * 100);
        if(ret != 1)
            continue;
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
    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comRecvBufSck ret=%d\n", ret);
    tmrStop(0);
#endif
    return ret;
}

static int comRecvBufSsl(tBuffer * msg, const byte * trm, byte dly) {
    int ret = -1;

#ifdef __CHN_SSL__
    word dim, idx;
    byte b = 0x00;

    CHECK(com.prm.hdlSsl, lblKO);
    tmrStart(0, dly * 100);
    dim = 0;
    if(trm) {
        while(tmrGet(0)) {
            if(dim >= bufDim(msg))
                break;
            ret = SSL_Read(com.prm.hdlSsl, &b, 1, dly);
            if(ret != 1)
                continue;
            if(msg) {
                ret = bufSet(msg, b, 1);
                CHECK(ret >= 0, lblKO);
            }
            dim++;
            if(!trm)
                continue;
            idx = 0;
            while(trm[idx]) {   //is it a terminator character
                if(b == trm[idx])
                    break;
                idx++;
            }
            if(b == trm[idx])
                break;
        }
        ret = dim;
    } else {
        ret =
            SSL_Read(com.prm.hdlSsl, (void *) bufPtr(msg), (int) bufDim(msg),
                     dly * 100);
        if(ret > 0)
            msg->pos = (word) ret;
    }
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comRecvBufSsl ret=%d\n", ret);
    tmrStop(0);
#endif
    return ret;
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
 * \source sys\\tlmchn.c
 * \test tcab0008.c
 */
int comRecvBuf(tBuffer * msg, const byte * trm, byte dly) {
    int ret;

    trcS("comRecvBuf Beg\n");

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
          ret = comRecvBufIp(msg, trm, dly);
          break;
      case chnGprs:
      case chnPpp:	  	
          ret = comRecvBufGprs(msg, trm, dly);
          break;
      case chnUSB:
          ret = comRecvBufUSB(msg, trm, dly);
          break;
      case chnSck:
          ret = comRecvBufSck(msg, trm, dly);
          break;
      case chnSsl:
          ret = comRecvBufSsl(msg, trm, dly);
          break;
      default:
          VERIFY(com.chn < chnEnd);
          ret = -1;
          break;
    }

    if(ret < 0)
        return ret;

    if(msg) {
        trcFS("comRecvBuf msg=%s\n", (char *) bufPtr(msg));
        trcBAN(bufPtr(msg), bufLen(msg));
        trcS("\n");
    } else if(trm) {
        trcFS("comRecvBuf trm=%s\n", (char *) trm);
        trcBAN(trm, strlen((char *) trm));
        trcS("\n");
    }
    return ret;
}

static int comHangStartIp(void) {
    int ret = 0;

    trcS("comHangStartIp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs);

    ret = LL_Disconnect(com.prm.hdl);
    ret = LL_Configure(&com.prm.hdl, NULL);
    CHECK(ret == LL_ERROR_OK, lblKO);
    com.prm.hdl = 0;
    return (ret);
  lblKO:
    trcFN("comHangStartIp KO : %d\n", ret);
    return -1;
}

static int comHangStartMdm(void) {
    int ret = -1;

    trcS("comHangStartMdm\n");
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc ||
           com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);

    ret = LL_Disconnect(com.prm.hdl);
    ret = LL_Configure(&com.prm.hdl, NULL);
    CHECK(ret == LL_ERROR_OK, lblKO);
    com.prm.hdl = 0;
    trcFS("comHangStartMdm : %s", "SUCCESS");

    return 1;
  lblKO:
    trcFN("comHangStartMdm KO : %d\n", ret);
    return -1;
}

static int comHangStartSck(void) {
#ifdef __CHN_SCK__
    int ret = -1;

    trcS("comHangStartSck\n");
    VERIFY(com.chn == chnSck);
    CHECK(com.prm.hdlSck, lblKO);

    ret = shutdownsocket(com.prm.hdlSck, SHUT_RD);
    ret = closesocket(com.prm.hdlSck);
    CHECK(ret == LL_ERROR_OK, lblKO);
    ttestall(0, 100);
    com.prm.hdlSck = 0;
    trcFS("comHangStartSck : %s\n", "SUCCESS");

    return 1;
  lblKO:
    trcFN("comHangStartSck KO : %d\n", ret);
#endif
    return -1;
}

static int comHangStartSsl(void) {
#ifdef __CHN_SSL__
    int ret = -1;

    trcS("comHangStartSsl\n");
    VERIFY(com.chn == chnSsl);
    CHECK(com.prm.hdlSsl, lblKO);

    ret = SSL_Disconnect(com.prm.hdlSsl);
    //CHECK(ret == 0, lblKO);

    ret = SSL_Free(com.prm.hdlSsl);
    //CHECK(ret == 0, lblKO);

    ret = SSL_UnloadProfile(com.prm.SslProfName);
    //CHECK(ret == SSL_PROFILE_EOK, lblKO);

    ret = SSL_DeleteProfile(com.prm.SslProfName);
    //CHECK(ret == SSL_PROFILE_EOK, lblKO);

    com.prm.hdlSsl = 0;
    memset(com.prm.SslProfName, 0, sizeof(com.prm.SslProfName));
    return 1;
  lblKO:
    trcFN("comHangStartSsl KO : %d\n", ret);
#endif
    return -1;
}

/** For internal modem initiates hanging up. Can be applied to internal modem only.
 * For 32bit it does not wait for result for performance reasons.
 * However, it is necessary to call comHangWait() before closing com port.
 * For TCP closes the socket.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0008.c
 */
int comHangStart(void) {
    trcS("comHangStart\n");
    switch (com.chn) {
      case chnCom1:
      case chnCom2:
      case chnCom3:
      case chnMdm:
      case chnHdlc:
          return comHangStartMdm();
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comHangStartIp();
      case chnSck:
          return comHangStartSck();
      case chnSsl:
          return comHangStartSsl();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comHangWaitMdm(void) {
    int ret = 0;

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc ||
           com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);
    trcS("comHangWaitMdm\n");
    //ret = LL_Configure(&com.ptcomSession,pConfigNULL);
    if(com.prm.hdl) {
        ret = LL_Disconnect(com.prm.hdl);
        CHECK((ret == LL_ERROR_OK)
              || (ret == LL_ERROR_INVALID_HANDLE), lblOKHandle);
    }
    com.prm.hdl = 0;

    trcS("comHangWaitMdm Closed\n");
  lblOKHandle:
    com.prm.ifconn = FALSE;
    trcFN("comHandWaitMdm (ret) : %d\n", ret);
    return 1;

#if 0
#ifdef __CHN_COM__

    trcS("comHangWaitMdm\n");
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
#endif
    return (-1);
}

static int comHangWaitIp(void) {
    trcS("comHangWaitIp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780
           || com.chn == chnPpp || com.chn == chnGprs);
    return 1;
}

static int comHangWaitSck(void) {
#ifdef __CHN_SCK__
    int ret = -1;

    trcS("comHangWaitSck\n");
    VERIFY(com.chn == chnSck);

    if(com.prm.hdlSck) {
        ret = shutdownsocket(com.prm.hdlSck, SHUT_RD);
        ret = closesocket(com.prm.hdlSck);
        CHECK(ret == LL_ERROR_OK, lblKO);
        ttestall(0, 100);
        com.prm.hdlSck = 0;
    }
    return 1;
  lblKO:
    trcFN("comHangWaitSck KO : %d\n", ret);
#endif
    return -1;
}

static int comHangWaitSsl(void) {
#ifdef __CHN_SSL__
    int ret = -1;

    trcS("comHangWaitSsl\n");
    VERIFY(com.chn == chnSsl);

    if(com.prm.hdlSsl) {
        ret = SSL_Disconnect(com.prm.hdlSsl);
        //CHECK(ret == 0, lblKO);

        com.prm.hdlSsl = 0;
    }

    if(!strcmp(com.prm.SslProfName, "")) {
        ret = SSL_UnloadProfile(com.prm.SslProfName);
        //CHECK(ret == SSL_PROFILE_EOK, lblKO);
        ret = SSL_DeleteProfile(com.prm.SslProfName);
        //CHECK(ret == SSL_PROFILE_EOK, lblKO);
        memset(com.prm.SslProfName, 0, sizeof(com.prm.SslProfName));
    }
    return 1;
//  lblKO:
//    trcFN("comHangWaitSsl KO : %d\n", ret);
#endif
    return -1;
}

/** For internal modem wait while hang up finishes.
 * The function comHangStart() should be started before.
 * For TCP does nothing.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0008.c
 */
int comHangWait(void) {
    trcS("comHangWait\n");
    switch (com.chn) {
      case chnCom1:
      case chnCom2:
      case chnCom3:
      case chnMdm:
      case chnHdlc:
          return comHangWaitMdm();
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
      case chnGprs:
          return comHangWaitIp();
      case chnSck:
          return comHangWaitSck();
      case chnSsl:
          return comHangWaitSsl();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comStopMdm(void) {
#ifdef __CHN_COM__
    int ret = 0;

    trcS("comStopMdm\n");
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc ||
           com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);

    TlvTree_Release(com.hCfg);

    LL_Configure(&com.prm.hdl, NULL);
    if(com.prm.hdl) {
        ret = LL_Disconnect(com.prm.hdl);
        CHECK(ret == LL_ERROR_OK, lblKO);

        com.prm.hdl = 0;
    }
    return ret;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopGprs(void) {
    int ret = 0;

    trcS("comStopGprs\n");
    VERIFY(com.chn == chnGprs);

    TlvTree_Release(com.hCfg);

    ret = LL_GPRS_Stop();
    CHECK(ret == LL_ERROR_OK, lblKO);

    return ret;
  lblKO:
    return -1;
}

static int comStopUsb(void) {
    int ret = 0;

    trcS("comStopUsb\n");
    VERIFY(com.chn == chnUSB);

    TlvTree_Release(com.hCfg);

    LL_Configure(&com.prm.hdl, NULL);

    if(com.prm.hdl) {
        ret = LL_Disconnect(com.prm.hdl);
        CHECK(ret == LL_ERROR_OK, lblKO);

        com.prm.hdl = 0;
    }
    return ret;
  lblKO:
    return -1;
}

static int comStopTcp(void) {
    int ret = 0;

    trcS("comStopTcp\n");
    VERIFY((com.chn == chnTcp5100) || (com.chn == chnTcp7780));

    TlvTree_Release(com.hCfg);

    if(com.prm.hdl) {
        ret = LL_Disconnect(com.prm.hdl);
        CHECK(ret == LL_ERROR_OK, lblKO);
        LL_Configure(&com.prm.hdl, NULL);
        com.prm.hdl = 0;
    }
    return ret;
  lblKO:
    return -1;
}

static int comStopPpp(void) {
    int ret = 0;

    trcS("comStopTcp\n");
    VERIFY(com.chn == chnPpp);

    TlvTree_Release(com.hCfg);

    if(com.prm.hdl) {
        ret = LL_Disconnect(com.prm.hdl);
        CHECK(ret == LL_ERROR_OK, lblKO);

        LL_Configure(&com.prm.hdl, NULL);
        com.prm.hdl = 0;
    }
    return ret;
  lblKO:
    return -1;
}

static int comStopSck(void) {
#ifdef __CHN_SCK__
    int ret = -1;

    trcS("comStopSck Beg\n");
    VERIFY(com.chn == chnSck);

    if(com.prm.hdlSck) {
        ret = shutdownsocket(com.prm.hdlSck, SHUT_RD);
        ret = closesocket(com.prm.hdlSck);
        CHECK(ret == LL_ERROR_OK, lblKO);
        ttestall(0, 100);
        com.prm.hdlSck = 0;
    }
    return 1;
  lblKO:
    trcFN("comStopSck KO : %d\n", ret);
#endif
    return -1;
}

static int comStopSsl(void) {
#ifdef __CHN_SSL__
    int ret = -1;

    trcS("comStopSsl\n");
    VERIFY(com.chn == chnSsl);

    if(com.prm.hdlSsl) {
        ret = SSL_Disconnect(com.prm.hdlSsl);
        //CHECK(ret == 0, lblKO);
        ret = SSL_Free(com.prm.hdlSsl);
        //CHECK(ret == 0, lblKO);
        com.prm.hdlSsl = 0;
    }

    if(!strcmp(com.prm.SslProfName, "")) {
        ret = SSL_UnloadProfile(com.prm.SslProfName);
        //CHECK(ret == SSL_PROFILE_EOK, lblKO);
        ret = SSL_DeleteProfile(com.prm.SslProfName);
        //CHECK(ret == SSL_PROFILE_EOK, lblKO);
        memset(com.prm.SslProfName, 0, sizeof(com.prm.SslProfName));
    }
    return 1;
//  lblKO:
//    trcFN("comStopSsl KO : %d\n", ret);
#endif
    return -1;
}

/** Close the associated channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0008.c
 */
int comStop(void) {
    int ret = 0;

    switch (com.chn) {
      case chnMdm:
      case chnHdlc:
          ret = comStopMdm();
          break;
      case chnCom1:
      case chnCom2:
      case chnCom3:
          ret = comStopMdm();
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
          ret = comStopUsb();
          break;
      case chnSck:
          ret = comStopSck();
          break;
      case chnSsl:
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

/**Get IMSI of GSM SIM.
 * \remark
 * \param *pImsi (O) Pointer for IMSI value. 15+1 characters should be reserved at pImsi
 * \param *pPin  (I) Pointer for PIN value. If not required can be zero pointer
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
 * \test tcik0051.c
 */
int comGetIMSI(char *pImsi, const char *pPin) { ///<Get IMSI of GSM SIM
    int ret = 0;
    FILE *gprs = NULL;

    //T_DGPRS_GET_INFORMATION info;

    if((gprs = stdperif((char *) "DGPRS", NULL)) != NULL) {
        ret = fioctl(DGPRS_FIOCTL_GET_IMSI, pImsi, gprs);
        switch (ret) {
          case DGPRS_SUCCES:
              ret = DGPRS_SUCCES;
              break;
          case DGPRS_NO_MORE_CONNECTED:
              ret = DGPRS_NO_MORE_CONNECTED;
              break;
          case DGPRS_ALREADY_CONNECTED:
              ret = DGPRS_ALREADY_CONNECTED;
              break;
          case DGPRS_WRITE_DENIED:
              ret = DGPRS_WRITE_DENIED;
              break;
          case DGPRS_NOT_IMPLEMENTED:
              ret = DGPRS_NOT_IMPLEMENTED;
              break;
          case DGPRS_BAD_PARAMETER:
              ret = DGPRS_BAD_PARAMETER;
              break;
          case DGPRS_BUSY:
              ret = DGPRS_BUSY;
              break;
          default:
              break;
        }
        //memcpy(pImsi, info.simIccId, sizeof(info.simIccId));
    }
    CHECK(ret != DGPRS_SUCCES, lblKO);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    goto lblEnd;
  lblEnd:
    return ret;

}

/** Get IMEI of GSM modem.
 * \remark
 *  - established connection first before calling this function
 * \param *pImei (O) Pointer for IMEI value. 20+1 characters should be reserved at pImei
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\chn.c
*/
int comGetIMEI(char *pImei) {
    int ret = 0;
    FILE *gprs = NULL;
    T_DGPRS_GET_INFORMATION info;

    if((gprs = stdperif((char *) "DGPRS", NULL)) != NULL) {
        ret = fioctl(DGPRS_FIOCTL_GET_INFORMATION, &info, gprs);
        CHECK(ret >= 0, lblKO);
        switch (ret) {
          case DGPRS_SUCCES:
              ret = DGPRS_SUCCES;
              break;
          case DGPRS_NO_MORE_CONNECTED:
              ret = DGPRS_NO_MORE_CONNECTED;
              break;
          case DGPRS_ALREADY_CONNECTED:
              ret = DGPRS_ALREADY_CONNECTED;
              break;
          case DGPRS_WRITE_DENIED:
              ret = DGPRS_WRITE_DENIED;
              break;
          case DGPRS_NOT_IMPLEMENTED:
              ret = DGPRS_NOT_IMPLEMENTED;
              break;
          case DGPRS_BAD_PARAMETER:
              ret = DGPRS_BAD_PARAMETER;
              break;
          case DGPRS_BUSY:
              ret = DGPRS_BUSY;
              break;
          default:
              break;
        }
        memcpy(pImei, info.module_imei_number, sizeof(info.module_imei_number));
    }

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    return ret;
}

/** This function retrieves the last error.
 * \return last error that occurred on the specified communication 
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test none
 */
int comGetLastError(void) {
    return LL_GetLastError(com.prm.hdl);
}

/** Get Connection Status
 * \return status indicating the state of the connection 
 - LL_STATUS_DISCONNECTED	
 - LL_STATUS_CONNECTING
 - LL_STATUS_CONNECTED
 - LL_STATUS_DISCONNECTING
 - LL_STATUS_PHYSICAL_DISCONNECTION
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test none
 */
int comGetDialInfo(void) {
    return LL_GetStatus(com.prm.hdl);
}

#ifdef __CHN_FTP__
static FTP_HANDLE hFtp = 0;
static char ftp_dir[128 + 1];

/** Open ftp channel.
 * The IP connection should be established before starting FTP.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0111.c
 */
int ftpStart(void) {
    int ret;

    trcS("ftpStart\n");
    hFtp = 0;
    memset(ftp_dir, 0, sizeof(ftp_dir));
    ret = 1;
    return ret;
}

/** Disconnect and close ftp channel.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0111.c
 */
int ftpStop(void) {
    trcS("ftpStop\n");

    if(hFtp != NULL)            // ftp session is open
    {
        FTP_Disconnect(hFtp);
        hFtp = 0;
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
 * \source sys\\tlmchn.c
 * \test tcab0111.c
 */
int ftpDial(const char *srv, const char *usr, const char *pwd, char mod) {
    int ret;
    byte port = 21;             //default value

    trcFN("ftpDial mod=%c\n", mod);
    VERIFY(srv);

    ret = -1;
    mod = 0;                    //forced IK
    if(mod != 'p')
        hFtp = FTP_Connect((char *) usr, (char *) pwd,
                           (char *) srv, (unsigned short) port, FALSE);
    if(hFtp == 0 && mod == 0)
        mod = 'p';

    if(mod == 'p')
        hFtp = FTP_Connect((char *) usr, (char *) pwd,
                           (char *) srv, (unsigned short) port, TRUE);
    CHECK(hFtp, lblKO);

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
 * \source sys\\tlmchn.c
 * \test tcab0111.c
 */
int ftpCD(const char *dir) {
    int ret;

    VERIFY(dir);
    trcFS("ftpCD dir = %s\n", dir);

    VERIFY(strlen(ftp_dir) + strlen(dir) <= 128);
    strcat(ftp_dir, dir);
    strcat(ftp_dir, "/");

    //This function does not work properly; the workaround is to use static variable ftp_dir, see above
    //ret = FTP_ChangeDir( hFtp, (char*) dir );
    //CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
    //lblKO:
    //  trcFN("ftpChDir error = %d\n", ret);
    //  ret = -1;

  lblEnd:
    trcFN("ftpCD ret = %d\n", ret);
    return ret;
}

static int ftpGetBuf(byte sec, const char *filePath) {
    int ret;
    unsigned char data[1024];
    unsigned short len;
    unsigned long ofs;

    VERIFY(filePath);
    trcFS("ftpGetBuf filePath=%s\n", filePath);

    ret = FTP_Get(hFtp, filePath);  // Open FTP data channel
    trcFN("FTP_Get ret=%d\n", ret);
    CHECK(ret == 0, lblKO);

    len = 0;
    ofs = 0;
    do {
        memset(data, 0, sizeof(data));
        ret = FTP_GetData(hFtp, (char *) data, sizeof(data));
        CHECK(ret >= 0, lblKO);
        len = ret;
        nvmSave(sec, data, ofs, len);
        ofs = ofs + len;
    } while(len > 0);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = FTP_GetError(hFtp);
    trcFN("ftpRetrieve error = %d\n", ret);
    ret = -1;
  lblEnd:
    trcFN("ftpGetBuf ret=%d\n", ret);
    FTP_CloseData(hFtp);
    return ret;
}

/** Gets a file from the remote FTP file system and stores it locally in the non volatile memory.
 * \param sec (I) The nvm section where to retrieve the file
 * \param file (I) The remote file name
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0111.c
 */
int ftpGet(byte sec, const char *file) {
    int ret;
    char buf[64 + 1];
    char filePath[128 + 1];

    VERIFY(file);
    trcFS("ftpGet file=%s ", file);

    nvmPathFileName(buf, sec);
    trcFS("sec=%s\n", buf);
    strcpy(filePath, ftp_dir);
    VERIFY(strlen(filePath) + strlen(file) <= 128);
    strcat(filePath, file);

    //there is a bug while getting file in simulation mode
    //the workaround is: try to get it as a file; if it does not work, get it by pieces into a buffer
    //there is no problem in real mode
    ret = FTP_GetFile(hFtp, filePath, buf);
    trcFN("FTP_GetFile ret=%d\n", ret);
    if(ret == -1)
        ret = ftpGetBuf(sec, filePath);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = FTP_GetError(hFtp);
    trcFN("FTP error = %d\n", ret);
    ret = -1;
  lblEnd:
    trcFN("ftpGet ret=%d\n", ret);
    return ret;
}

/** Put a file at a remote FTP file system.
 * \param sec (I) The nvm section where to be uploaded
 * \param file (I) The remote file name
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmchn.c
 * \test tcab0111.c
 */
int ftpPut(byte sec, const char *file) {
    int ret;
    char buf[64 + 1];
    char filePath[128 + 1];

    VERIFY(file);
    trcFS("ftpPut file = %s ", file);
    nvmPathFileName(buf, sec);
    trcFS("sec=%s\n", buf);

    strcpy(filePath, ftp_dir);
    VERIFY(strlen(filePath) + strlen(file) <= 128);
    strcat(filePath, file);

    ret = FTP_PutFile(hFtp, (char *) filePath, buf);
    CHECK(ret == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcFN("FTP error = %d\n", ret);
    ret = -1;

  lblEnd:
    trcFN("ftpGet ret = %d\n", ret);
    return ret;
}
#endif
