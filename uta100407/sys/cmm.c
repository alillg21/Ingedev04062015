/** \file
 * Communication channels implementation in APR environment (via commpsapp module)
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/cmm.c $
 *
 * $Id: cmm.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#ifdef __CMM__
#include "cmm.h"
#include "tagcmm.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcCHN))

#define RSP_LEN 512

typedef struct {
    int16 sta;
    uint16 len;
    byte buf[RSP_LEN];
} tResponse;

typedef struct sComChn {
    byte chn;                   // communication channel used
    byte chnRole;
    union {
#ifdef __CHN_COM__
        struct {                //chn= chnMdm or chnHdlc or chnCom
            uint32 hdl;
            uint32 errCorrType;
            uint32 dataComprType;
            uint8 phoneLen;
            char *phoneNumber;

        } mdm;
#endif
#ifdef __CHN_TCP__
        struct {                //chn= chnTcp...
            uint32 hdl;
            uint16 localAdr[4];
            uint16 localPort;
            uint16 mask[4];
            uint16 gateway[4];
            uint16 hostAdr[4];
            uint16 hostPort;

        } tcp;
#endif
#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs...
            uint32 hdl;
            uint16 localAdr[4];
            uint16 localPort;
            uint16 mask[4];
            uint16 gateway[4];
            char apn[64 + 1];
            char username[8 + 1];
            char password[8 + 1];
            char pin[8 + 1];
            uint16 priHostAdr[4];
            uint16 priHostPort;
            uint16 secHostAdr[4];
            uint16 secHostPort;
            uint16 terHostAdr[4];
            uint16 terHostPort;
            uint16 timeout;
            uint16 tryAtt;
        } gprs;
#endif
#ifdef __CHN_WIFI__
        struct {                //chn= chnWifi
            uint32 hdl;
            char ssidLocal[16];
            char ssidRemote[16];
            byte keyData[16];
        } wifi;
#endif
        byte a;                 //if all is undefined, the union cannot be empty (compiler complainte)
        char sep;               //separator
    } prm;
} tComChn;
static tComChn com;

tlv_t tlv;
uint8 tlvBuffer[1024];

static int comStartMdm(void) {
#ifdef __CHN_COM__
    int ret;

    trcS("comStartMdm Beg\n");
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    com.prm.mdm.hdl = 0;
    com.prm.sep = '|';

    ret = 1;
    goto lblEnd;
//  lblKO:
//    ret = -1;
//    comClose(com.prm.mdm.hdl);
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
    ret = RET_OK;
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
    //ret = comOpen(ptr, &com.prm.mdm.hdl);
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

static int comStartTcp(void) {
#ifdef __CHN_TCP__
    int ret;

    trcS("comStartTcp Beg\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780);

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    com.prm.tcp.hdl = 0;
    com.prm.sep = '|';

    ret = 1;
    goto lblEnd;
//  lblKO:
//    ret = -1;
  lblEnd:
    trcFN("comStartTcp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comStartGprs(void) {
#ifdef __CHN_GPRS__
    int ret;

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    com.prm.gprs.hdl = 0;
    com.prm.sep = '|';
    com.prm.gprs.timeout = 1500;

    ret = 1;
    goto lblEnd;
    //lblKO:
    //ret = -1;
  lblEnd:
    trcFN("comStartTcp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comStartWifi(void) {
#ifdef __CHN_WIFI__
    int ret;

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    com.prm.wifi.hdl = 0;
    com.prm.sep = '|';

    ret = 1;
    goto lblEnd;
    //lblKO:
    //ret = -1;
  lblEnd:
    trcFN("comStartWifi ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

/** Open the associated communication channel.
 * \param chn (I) Channel type from enum eChn.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\cmm.c
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
          return comStartCom(); //comStartCom();
      case chnTcp5100:
      case chnTcp7780:
          return comStartTcp();
      case chnPpp:
          return -1;            //comStartPpp(); //PPP
      case chnGprs:
          return comStartGprs();    //comStartGprs();    //GPRS
      case chnWifi:
          return comStartWifi();    //comStartWifi();    //Wifi         
      case chnUSB:
          return -1;            //comStartUSB(); // USB
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSetMdm(const char *init) {
#ifdef __CHN_COM__
    int ret;
    byte modulationType;
    byte stopbits;
    byte parity;
    byte dialmode;
    byte datasize;
    byte bps;
    byte blindDial;
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
          dialmode = FALSE;
          init++;
          break;
      case 'P':
      case 'p':
          dialmode = TRUE;
          init++;
          break;
      default:
          dialmode = FALSE;
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

        blindDial = (enum comFieldStatus_t) (*init == 'D' ? FALSE : TRUE);

    com.prm.mdm.errCorrType =
        (enum comErrorCorrection_t) (correction ? COM_CORRECTION_MNP :
                                     COM_CORRECTION_NONE);
    com.prm.mdm.dataComprType =
        (enum comDataCompression_t) (compression ? COM_COMPRESSION_MNP5 :
                                     COM_COMPRESSION_NONE);

    if(com.chn == chnHdlc) {
        tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceModem);
        tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolHDLC);
    } else {
        VERIFY(com.chn == chnMdm);

        tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceModem);
        tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTransparent);
    }

    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, 6000); //60secs
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_CONNECT, 4500); //45secs
    tlv_add_value8(&tlv, TAG_CMM_DIAL_BLIND, blindDial);
    tlv_add_value8(&tlv, TAG_CMM_DIAL_PULSE, dialmode);
    tlv_add_value8(&tlv, TAG_CMM_DIAL_FASTCONNECT, fast);
    tlv_add_value8(&tlv, TAG_CMM_DIAL_SPEED, bps);
    tlv_add_value8(&tlv, TAG_CMM_DIAL_COMPRESSION_TYPE,
                   com.prm.mdm.dataComprType);
    tlv_add_value8(&tlv, TAG_CMM_DIAL_CORRECTION_TYPE, com.prm.mdm.errCorrType);
    tlv_add_value8(&tlv, TAG_CMM_DIAL_BACKGROUND, FALSE);
    tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_PRI, 4, "*808");
    tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_PRI, 2);
    tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_SEC, 4, "*808");
    tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_SEC, 2);
    tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_TER, 4, "*808");
    tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_TER, 2);

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

/** Parse a string for extracting a part of it
 * \param *dst (O) Pointer for destination. dim characters are reserved at dst
 * \param *src (I) Pointer to the source string to parse.
 * \param  dim (I) size of the destination
 * \return a pointer to the string following the extracted part of it
 */
#ifdef __CHN_TCP__
#ifndef __TEST__
static
#endif
const char *parseStr(char *dst, const char *src, int dim) {
    VERIFY(dst);
    VERIFY(src);

    while(*src) {
        if(*src == com.prm.sep) {
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
static int getAddr(const char *inAdr, word * outAdr) {
    int ret;
    card tmp;
    char buf[3 + 1];
    byte i;
    byte adr1 = 0, adr2 = 0, adr3 = 0, adr4 = 0;

    com.prm.sep = '.';
    for (i = 0; i < 4; i++) {
        inAdr = parseStr(buf, inAdr, sizeof(buf));
        ret = dec2num(&tmp, buf, 0);
        //outAdr[i]= (word)tmp;
        switch (i) {
          case 0:
              adr1 = (byte) tmp;
              break;
          case 1:
              adr2 = (byte) tmp;
              break;
          case 2:
              adr3 = (byte) tmp;
              break;
          case 3:
              adr4 = (byte) tmp;
              break;
        }
    }
    outAdr[0] = WORDHL(adr2, adr1);
    outAdr[1] = WORDHL(adr4, adr3);

    com.prm.sep = '|';
    return ret;
}
#endif

static int comSetCom(const char *init) {
#ifdef __CHN_COM__
    int ret = 0;
    enum comDataSize_t datasize;    //data bits: 7 or 8
    enum comParity_t parity;    //odd,even,none
    enum comStopBits_t stopbits;    //1,2
    enum comSpeed_t baud;       //The speed

    trcFS("comSetCom %s\n", init);
    VERIFY(init);
    VERIFY(com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);

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
    memset(tlvBuffer, 0, sizeof(tlvBuffer));

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));

    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, 1500);
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_SEND, 1500);
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_INTER_CHARACTER, 50);
    tlv_add_value8(&tlv, TAG_CMM_RS232_SPEED, baud);
    tlv_add_value8(&tlv, TAG_CMM_RS232_PARITY, parity);
    tlv_add_value8(&tlv, TAG_CMM_RS232_DATASIZE, datasize);
    tlv_add_value8(&tlv, TAG_CMM_RS232_STOPBITS, stopbits);
    tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTransparent);
    tlv_add_value8(&tlv, TAG_CMM_LENGTH_FORMAT, cmmLengthNone);

    if(com.chn == chnCom2) {
        tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceRS232Com2);
    } else {
        tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceRS232);
    }
    goto lblEnd;

  lblKO:
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
//#ifdef WIN32
//    return 1;
//#else
    int ret;
    char gateway[15 + 1];
    char adr[15 + 1];
    char msk[15 + 1];
    char port[10];
    card dPort = 0;
    byte dhcp;
    byte IpAdr[4];

    trcS("comSetTcp Beg\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780);
    VERIFY(init);

    dhcp = FALSE;
    if(!*init) {
        dhcp = TRUE;
    }
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_SET);
    tlv_add_value8(&tlv, TAG_CMM_ETH_DHCP, dhcp);

    if(*init) {                 // static IP address 
        init = parseStr(adr, init, sizeof(adr));
        ret = getAddr(adr, com.prm.tcp.localAdr);
        tlv_add_valueBytes(&tlv, TAG_CMM_ETH_ADDRESS, 4, com.prm.tcp.localAdr);

        VERIFY(*init);          // verify port follows
        init = parseStr(port, init, sizeof(port));
        dec2num(&dPort, port, 0);
        com.prm.tcp.localPort = (word) dPort;
        tlv_add_value16Little(&tlv, TAG_CMM_ETH_PORT, com.prm.tcp.localPort);

        if(*init) {             // verify msk follows
            init = parseStr(msk, init, sizeof(msk));
            ret = getAddr(msk, com.prm.tcp.mask);
            tlv_add_valueBytes(&tlv, TAG_CMM_ETH_SUBNET_MASK, 4,
                               com.prm.tcp.mask);
        }

        if(*init) {             // parse default gateway
            (void) parseStr(gateway, init, sizeof(gateway));
            ret = getAddr(gateway, com.prm.tcp.gateway);
            tlv_add_valueBytes(&tlv, TAG_CMM_ETH_GATEWAY, 4,
                               com.prm.tcp.gateway);
        }
    }
    ret = cmmGeneral(&tlv);
    CHECK(ret == RET_OK, lblKO);

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_LNET_UP);
    ret = cmmGeneral(&tlv);
    CHECK(ret == RET_OK, lblKO);

    //Get Ethernet info
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_GET);
    ret = cmmGeneral(&tlv);

    if(tlv_tag_search(&tlv, TAG_CMM_ETH_ADDRESS) == 0) {
        memset(IpAdr, 0, sizeof(IpAdr));
        ret = tlv_get_valueBytes(&tlv, 0, IpAdr, 4);
        memset(adr, 0, sizeof(adr));
        sprintf(adr, "%u.%u.%u.%u", LBYTE(IpAdr[0]), LBYTE(IpAdr[1]),
                LBYTE(IpAdr[2]), LBYTE(IpAdr[3]));
    }
    if(tlv_tag_search(&tlv, TAG_CMM_ETH_PRESENT) == 0) {
        ret = tlv_get_value8(&tlv, 0);
    }
    if(tlv_tag_search(&tlv, TAG_CMM_ETH_CABLE_CONNECTED) == 0) {
        ret = tlv_get_value8(&tlv, 0);
        CHECK(ret == 1, lblKO);
    }

    ret = 1;
    goto lblEnd;

  lblKO:
    trcFN("Channnel Start Error %d\n", ret);
    comClose(com.prm.tcp.hdl);
  lblEnd:
    trcFN("comSetTcp ret=%d\n", ret);
    return ret;
//#endif
#else
    return -1;
#endif
}

/*
static int comGprsConfig(void) {
#ifdef __CHN_GPRS__
	int 	ret;

	trcS("comGprsConfig Beg\n");
	tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
	tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_GPRS_INFO_GET);

	ret = cmmGeneral(&tlv);
	trcFN("gprsInfoGet =%d\n", ret);
	CHECK(ret == 0, lblEnd);

	//set DHCP
	tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
	tlv_add_value8(&tlv, TAG_CMM_ETH_DHCP, TRUE);
	tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_ETH_INFO_SET);

	ret = cmmGeneral(&tlv);
	trcFN("gprsDhcpSet =%d\n", ret);

	//set gprs parameters
	tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
	tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_GPRS_INFO_SET);
	tlv_add_valueBytes(&tlv, TAG_CMM_GPRS_APN, 21, com.prm.gprs.apn);
	if(*com.prm.gprs.username) {
	    tlv_add_valueBytes(&tlv, TAG_CMM_USERNAME, strlen(com.prm.gprs.username), com.prm.gprs.username);
	}
	if(*com.prm.gprs.password) {
	    tlv_add_valueBytes(&tlv, TAG_CMM_PASSWORD, strlen(com.prm.gprs.password), com.prm.gprs.password);
	}
	ret = cmmGeneral(&tlv);
	trcFN("gprsInfoSet %d\n", ret);

	tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
	tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_UP);
	tlv_add_value8(&tlv, TAG_CMM_GSM_SIGNAL_INTERVAL, 5);
	tlv_add_valueBytes(&tlv, TAG_CMM_GPRS_APN, strlen(com.prm.gprs.apn), com.prm.gprs.apn);	
	if(*com.prm.gprs.username) {
		tlv_add_valueBytes(&tlv, TAG_CMM_USERNAME, strlen(com.prm.gprs.username), com.prm.gprs.username);
	}	
	if(*com.prm.gprs.password) {
		tlv_add_valueBytes(&tlv, TAG_CMM_PASSWORD, strlen(com.prm.gprs.password), com.prm.gprs.password);
	}	
	tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_PRI, 8, "*99***1#");

	ret = cmmGeneral(&tlv);
	trcFN("gsmUp =%d\n", ret);

	if(ret == cmmErrGprsAttachInProgress)
	{
		if( tlv_tag_search(&tlv, TAG_CMM_ERROR_CODE) == 0)
		{
			ret = tlv_get_value8(&tlv, 0);
			trcFN("state =%d\n", ret);		
			goto lblKO;
		}
	}

	goto lblEnd;
	lblKO:
		trcS("comGprsConfig KO\n");
		return -1;
	lblEnd:
		trcS("comGprsConfig End\n");
		return 1;
#else
	return -1;
#endif
}
*/

static int comSetGprs(const char *init) {
#ifdef __CHN_GPRS__

    int ret;
    char gStatus;

    VERIFY(com.chn == chnGprs);
    VERIFY(init);
    trcS("comSetGprs Beg\n");

    init = parseStr(com.prm.gprs.pin, init, sizeof(com.prm.gprs.pin));  // extract pin
    if(*com.prm.gprs.pin) {
        tlv_add_valueBytes(&tlv, TAG_CMM_GSM_PIN1, 8, com.prm.gprs.pin);
    }

    init = parseStr(com.prm.gprs.apn, init, sizeof(com.prm.gprs.apn));  // extract apn
    if(*init) {
        init = parseStr(com.prm.gprs.username, init, sizeof(com.prm.gprs.username));    // extract usr
    }
    if(*init) {
        init = parseStr(com.prm.gprs.password, init, sizeof(com.prm.gprs.password));    // extract password
    }
    //set gprs config
    //comGprsConfig();

    //set gprs parameters
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_GPRS_INFO_SET);
    tlv_add_valueBytes(&tlv, TAG_CMM_GPRS_APN, 21, com.prm.gprs.apn);
    if(*com.prm.gprs.username) {
        tlv_add_valueBytes(&tlv, TAG_CMM_USERNAME,
                           strlen(com.prm.gprs.username),
                           com.prm.gprs.username);
    }
    if(*com.prm.gprs.password) {
        tlv_add_valueBytes(&tlv, TAG_CMM_PASSWORD,
                           strlen(com.prm.gprs.password),
                           com.prm.gprs.password);
    }
    ret = cmmGeneral(&tlv);
    trcFN("gis %d\n", ret);
    CHECK(ret == 0, lblKO);

    //attach to gprs network
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_UP);
    ret = cmmGeneral(&tlv);
    trcFN("gsmUp %d\n", ret);
    switch (ret) {
      case cmmErrGprsAttachInProgress:
      case cmmSuccess:
          break;
      default:
          break;
    }
    CHECK(ret == 0, lblKO);

    com.prm.gprs.tryAtt = 5;
    while(1) {
        tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
        tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_GPRS_INFO_GET);
        ret = cmmGeneral(&tlv);
        trcFN("infoget =%d\n", ret);

        ret = tlv_tag_search(&tlv, TAG_CMM_GSM_GPRS_STATUS);
        CHECK(ret == 0, lblKO);

        gStatus = tlv_get_value8(&tlv, 0);
        trcFN("Gprs stat=%d\n", gStatus);

        switch (gStatus) {
          case cmmGprsConnected:
              goto lblEnd;
          case cmmGprsGsmDown:
              com.prm.gprs.tryAtt--;
              if(!com.prm.gprs.tryAtt)
                  goto lblKO;
          default:
              break;
        }

        tmrPause(2);
    }
    goto lblEnd;
  lblKO:
    trcS("GPRS layer KO\n");
    return -1;
  lblEnd:
    return 1;
#else
    return -1;
#endif
}

static int comSetWifi(const char *init) {
#ifdef __CHN_WIFI__

    int ret;

    VERIFY(com.chn == chnWifi);
    VERIFY(init);
    trcS("comSetWifi Beg\n");

    init = parseStr(com.prm.wifi.ssidLocal, init, sizeof(com.prm.wifi.ssidLocal));  // extract ssidLocal
    if(*init) {
        init = parseStr(com.prm.wifi.ssidRemote, init, sizeof(com.prm.wifi.ssidRemote));    // extract ssidRemote
    }
    if(*init) {
        init = parseStr((char *) com.prm.wifi.keyData, init, sizeof(com.prm.wifi.keyData)); // extract keyData
    }
    //isWifi ?
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_WIFI_INFO_GET);
    ret = cmmGeneral(&tlv);
    CHECK(ret == 0, lblKO);

    if(tlv_tag_search(&tlv, TAG_CMM_WIFI_PRESENT) == 0) {
        if(tlv_get_value8(&tlv, 0) == 0) {
            return -1;          //not wifi
        }
    } else {
        return -1;              //not wifi
    }

    //set wifi parameters
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_WIFI_INFO_SET);
    tlv_add_value8(&tlv, TAG_CMM_WIFI_AUTH_SHARED_KEY, FALSE);
    tlv_add_value8(&tlv, TAG_CMM_WIFI_CHANNEL, 1);
    tlv_add_value8(&tlv, TAG_CMM_WIFI_MODE_ADHOC, FALSE);
    tlv_add_valueBytes(&tlv, TAG_CMM_WIFI_SSID_LOCAL,
                       strlen(com.prm.wifi.ssidLocal), com.prm.wifi.ssidLocal);
    tlv_add_valueBytes(&tlv, TAG_CMM_WIFI_SSID_REMOTE,
                       strlen(com.prm.wifi.ssidRemote),
                       com.prm.wifi.ssidRemote);
    if(com.prm.wifi.keyData) {
        tlv_add_value8(&tlv, TAG_CMM_WIFI_WEP_ENABLED, TRUE);
        tlv_add_value8(&tlv, TAG_CMM_WIFI_WEP_KEY_TYPE, 0);
        tlv_add_valueBytes(&tlv, TAG_CMM_WIFI_WEP_KEY_DATA,
                           strlen((char *) com.prm.wifi.keyData),
                           com.prm.wifi.keyData);
    }

    ret = cmmGeneral(&tlv);
    CHECK(ret == 0, lblKO);

    goto lblEnd;
  lblKO:
    trcS("WIfi layer KO\n");
    return -1;
  lblEnd:
    return 1;
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
          return -1;            //comSetPpp(init);
      case chnGprs:
          return comSetGprs(init);
      case chnWifi:
          return comSetWifi(init);
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
    char *pabx;
    int pabxLen;
    char *prefix;
    char *country;
    char tmp[64 + 1];
    char buf[64 + 1];

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc);
    VERIFY(phone);
    VERIFY(strlen(phone) < sizeof(tmp));

    memset(tmp, 0x00, sizeof(tmp));
    memset(buf, 0x00, sizeof(buf));

    trcFS("comDialMdm phone = %s\n", phone);
    ret = 0;
    len = 0;
    strcpy(tmp, phone);
    pabx = "";
    prefix = "";
    country = "";
    phone = tmp;
    pabxLen = 0;
    while(tmp[len]) {           //extract pabx if found
        if(tmp[len] != '-') {
            len++;
            pabxLen++;
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

    strcpy(buf, pabx);
    strcat(buf, prefix);
    if(*country) {
        strcat(buf, ",");
        strcat(buf, country);
        strcat(buf, ",");
    }
    strcat(buf, phone);
    com.prm.mdm.phoneNumber = (char *) buf;
    com.prm.mdm.phoneLen = strlen(buf);
    //if(*pabx)
    //  tlv_add_valueBytes(&com.prm.tlv, TAG_CMM_DIAL_PABX, pabxLen, &pabx);

    tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_PRI, com.prm.mdm.phoneLen,
                       com.prm.mdm.phoneNumber);
    tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_PRI, 1);
    tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_SEC, com.prm.mdm.phoneLen,
                       com.prm.mdm.phoneNumber);
    tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_SEC, 1);
    tlv_add_valueBytes(&tlv, TAG_CMM_DIAL_NUMBER_TER, com.prm.mdm.phoneLen,
                       com.prm.mdm.phoneNumber);
    tlv_add_value8(&tlv, TAG_CMM_CONNECT_ATTEMPTS_TER, 1);

    ret = cmmInit(&com.prm.mdm.hdl, &tlv);
    switch (ret) {
      case cmmSuccess:
          trcS("cmmSucess\n");
          ret = 0;
          break;
      case cmmErrDialBusy:
          trcS("cmmBusy\n");
          ret = -comBusy;
          break;
      case cmmErrDialNoTone:
          trcS("cmmNoDialTone\n");
          ret = -comNoDialTone;
          break;
      case cmmErrDialNoCarrier:
      case cmmErrDeviceInUse:
      case cmmErrLostCarrier:
          trcS("cmmNoCarrier\n");
          ret = -comNoCarrier;
          break;
      case cmmErrTimeout:
          trcS("cmmTimeout\n");
          ret = -comTimeout;
      default:
          ret = -comKO;
          break;
    }
    goto lblEnd;
//  lblKO:
//    return -1;
  lblEnd:
    kbdStop();
    return ret;
#else
    return -1;
#endif
}

static int comDialIp(const char *srv) {
#ifdef __CHN_TCP__
    int ret = 0;

#ifdef _ING_SIMULATION
#endif
    char adr[16];               //tcp address NNN.NNN.NNN.NNN
    card dRemotePort;
    char RemotePort[10];

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 || com.chn == chnPpp);
    VERIFY(srv);

    trcFS("comDialIp srv = %s\n", srv);
    com.chnRole = chnRoleClt;

    srv = parseStr(adr, srv, sizeof(adr));
    ret = getAddr(adr, com.prm.tcp.hostAdr);

    VERIFY(*srv);
    srv = parseStr(RemotePort, srv, sizeof(RemotePort));
    ret = dec2num(&dRemotePort, RemotePort, 0);
    com.prm.tcp.hostPort = LWORD(dRemotePort);

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceEthClient);
    tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTCP);
    tlv_add_value8(&tlv, TAG_CMM_LENGTH_FORMAT, cmmLengthNone);

    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_PRI, 4, com.prm.tcp.hostAdr);
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_PRI,
                          com.prm.tcp.hostPort);

#ifdef __CHN_SSL__
    tlv_add_value8(&tlv, TAG_CMM_ETH_SSL, TRUE);
    tlv_add_value8(&tlv, TAG_CMM_ETH_SSL_METHOD, SSL_V3_CLIENT);
#endif

    ret = cmmInit(&com.prm.tcp.hdl, &tlv);
    switch (ret) {
      case cmmSuccess:
          trcS("cmmSucess\n");
          ret = 0;
          break;
      case cmmErrDialBusy:
          trcS("cmmBusy\n");
          ret = -comBusy;
          break;
      case cmmErrDialNoTone:
          trcS("cmmNoDialTone\n");
          ret = -comNoDialTone;
          break;
      case cmmErrDialNoCarrier:
      case cmmErrDeviceInUse:
      case cmmErrLostCarrier:
          trcS("cmmNoCarrier\n");
          ret = -comNoCarrier;
          break;
      case cmmErrTimeout:
          trcS("cmmTimeout\n");
          ret = -comTimeout;
      default:
          ret = -comKO;
          break;
    }
    CHECK(ret == 0, lblConnect);

    goto lblEnd;
  lblConnect:
    trcFN("connect Error %d\n", ret);
    comHangStart();
    comHangWait();
    goto lblEnd;
  lblEnd:
    trcFN("comDialIp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comDialGprs(const char *srv) {
#ifdef __CHN_GPRS__
    int16 ret;
    uint8 IpSta;
    uint8 GprsSta;
    char adr[15 + 1];
    char port[10];
    card dPort = 0;

    VERIFY(com.chn == chnWifi || com.chn == chnGprs);
    trcFS("comDialGprsIp srv = %s\n", srv);

    //TCP/IP layer
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_DEVICE_TYPE, cmmDeviceEthClient);
    tlv_add_value8(&tlv, TAG_CMM_PROTOCOL, cmmProtocolTransparent);
    tlv_add_value8(&tlv, TAG_CMM_LENGTH_FORMAT, cmmLengthNone);

    srv = parseStr(adr, srv, sizeof(adr));  // extract pri host addr
    ret = getAddr(adr, com.prm.gprs.priHostAdr);
    tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_PRI, 4,
                       com.prm.gprs.priHostAdr);

    VERIFY(*srv);               // verify port follows
    srv = parseStr(port, srv, sizeof(port));
    dec2num(&dPort, port, 0);
    com.prm.gprs.priHostPort = (word) dPort;
    tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_PRI,
                          com.prm.gprs.priHostPort);

    if(*srv) {
        memset(adr, 0, sizeof adr);
        srv = parseStr(adr, srv, sizeof(adr));  // extract pri host addr
        ret = getAddr(adr, com.prm.gprs.secHostAdr);
        tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_SEC, 4,
                           com.prm.gprs.secHostAdr);
    }
    if(*srv) {
        srv = parseStr(port, srv, sizeof(port));
        dec2num(&dPort, port, 0);
        com.prm.gprs.secHostPort = (word) dPort;
        tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_SEC,
                              com.prm.gprs.secHostPort);
    }
    if(*srv) {
        memset(adr, 0, sizeof adr);
        srv = parseStr(adr, srv, sizeof(adr));  // extract pri host addr
        ret = getAddr(adr, com.prm.gprs.terHostAdr);
        tlv_add_valueBytes(&tlv, TAG_CMM_ETH_HOST_ADDR_TER, 4,
                           com.prm.gprs.terHostAdr);
    }
    if(*srv) {
        srv = parseStr(port, srv, sizeof(port));
        dec2num(&dPort, port, 0);
        com.prm.gprs.terHostPort = (word) dPort;
        tlv_add_value16Little(&tlv, TAG_CMM_ETH_HOST_PORT_TER,
                              com.prm.gprs.terHostPort);
    }
    //memcpy(ipHost, "\xd2\x01\x4e\x4e", 4);        //210.1.78.78

    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_RECEIVE, com.prm.gprs.timeout);
    tlv_add_value16Little(&tlv, TAG_CMM_TIMEOUT_CONNECT, com.prm.gprs.timeout);

    ret = cmmInit(&com.prm.gprs.hdl, &tlv);
    trcFN("cmmInit %d\n", ret);
    switch (ret) {
      case cmmSuccess:
          trcS("cmmSucess\n");
          ret = 0;
          break;
      case cmmErrDialBusy:
          trcS("cmmBusy\n");
          ret = -comBusy;
          break;
      case cmmErrDialNoTone:
          trcS("cmmNoDialTone\n");
          ret = -comNoDialTone;
          break;
      case cmmErrDialNoCarrier:
      case cmmErrDeviceInUse:
      case cmmErrLostCarrier:
          trcS("cmmNoCarrier\n");
          ret = -comNoCarrier;
          break;
      case cmmErrTimeout:
          trcS("cmmTimeout\n");
          ret = -comTimeout;
      default:
          ret = -comKO;
          break;
    }
    CHECK(ret == 0, lblEnd);

    trcS("Checking IP/GPRS status\n");
    //are we connected to TCP/IP layer?
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));

    while(1) {
        ret = cmmStatus(com.prm.gprs.hdl, &tlv);
        trcFN("cmmStatus %d\n", ret);
        CHECK(ret == 0, lblKO);

        ret = tlv_tag_search(&tlv, TAG_CMM_STATUS_CONNECTION);
        CHECK(ret == 0, lblKO);

        IpSta = tlv_get_value8(&tlv, 0);
        trcFN("ip status %d\n", IpSta);
        switch (IpSta) {
          case cmmConnectedPrimary:
          case cmmConnectedSecondary:
          case cmmConnectedTertiary:
          case cmmConnectedPeer:
              ret = 0;
              goto lblEnd;
          default:
              break;
        }

        ret = tlv_tag_search(&tlv, TAG_CMM_GSM_GPRS_STATUS);
        CHECK(ret == 0, lblKO);

        GprsSta = tlv_get_value8(&tlv, 0);
        trcFN("gprs status %d\n", GprsSta);

        if(GprsSta == cmmGprsGsmDown)
            goto lblKO;

        tmrPause(1);
    }
    trcFN("Connected to %d\n", IpSta);
    ret = 0;
    goto lblEnd;
  lblKO:
    trcS("IP layer KO\n");
    return -1;
  lblEnd:
    return ret;
#else
    return -1;
#endif
}

static int comDialCom(const char *srv) {
#ifdef __CHN_COM__
    int16 ret;

    VERIFY(com.chn == chnCom1 || com.chn == chnCom2 || com.chn == chnCom3);
    trcFS("comDialCom srv = %s\n", srv);

    ret = cmmInit(&com.prm.mdm.hdl, &tlv);
    trcFN("cmmInit %d\n", ret);

    switch (ret) {
      case cmmSuccess:
          trcS("cmmSucess\n");
          ret = 0;
          break;
      case cmmErrDialBusy:
          trcS("cmmBusy\n");
          ret = -comBusy;
          break;
      case cmmErrDialNoTone:
          trcS("cmmNoDialTone\n");
          ret = -comNoDialTone;
          break;
      case cmmErrDialNoCarrier:
      case cmmErrDeviceInUse:
      case cmmErrLostCarrier:
          trcS("cmmNoCarrier\n");
          ret = -comNoCarrier;
          break;
      case cmmErrTimeout:
          trcS("cmmTimeout\n");
          ret = -comTimeout;
      default:
          ret = -comKO;
          break;
    }
    CHECK(ret == 0, lblEnd);

    goto lblEnd;
//  lblKO:
//    trcS("Com Dial KO\n");
//    return -1;
  lblEnd:
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
          return comDialCom(srv);
      case chnMdm:
      case chnHdlc:
          return comDialMdm(srv);
      case chnTcp5100:
      case chnTcp7780:
      case chnPpp:
          return comDialIp(srv);
      case chnGprs:
      case chnWifi:
          return comDialGprs(srv);  //comDialGprsIp(srv);
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

    ret = cmmEnd(com.prm.mdm.hdl);
    CHECK(ret == RET_OK, lblKO);
    com.prm.mdm.hdl = 0;

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
        ret = cmmEnd(com.prm.mdm.hdl);
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
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780);

    ret = cmmEnd(com.prm.tcp.hdl);
    CHECK(ret == RET_OK, lblKO);
    com.prm.mdm.hdl = 0;

    return com.prm.tcp.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopGprs(void) {
#ifdef __CHN_GPRS__
    int ret;

    trcS("comStopTcp\n");
    VERIFY(com.chn == chnGprs);

    ret = cmmEnd(com.prm.gprs.hdl);
    CHECK(ret == RET_OK, lblKO);
    com.prm.mdm.hdl = 0;

    return com.prm.gprs.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comStopWifi(void) {
#ifdef __CHN_WIFI__
    int ret;

    trcS("comStopWifi\n");
    VERIFY(com.chn == chnWifi);

    ret = cmmEnd(com.prm.wifi.hdl);
    CHECK(ret == RET_OK, lblKO);
    com.prm.mdm.hdl = 0;

    return com.prm.wifi.hdl;
  lblKO:
    return -1;
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
          ret = comStopCom();   //comStopCom();
          break;
      case chnTcp5100:
      case chnTcp7780:
          ret = comStopTcp();
          break;
      case chnPpp:
          ret = -1;             //comStopPpp();
          break;
      case chnGprs:
          ret = comStopGprs();  //comStopGprs();
          break;
      case chnWifi:
          ret = comStopWifi();  //comStopWifi();
          break;
      case chnUSB:
          ret = 0;
          break;
      default:
          VERIFY(com.chn < chnEnd);
          ret = -1;
          break;
    }
    com.chn = 0;
    return ret;
}

static int comSendCom(byte b) {
#ifdef __CHN_COM__
    int ret;

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);
    ret = cmmSend(com.prm.mdm.hdl, &b, 1);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comSendIp(byte b) {
#ifdef __CHN_COM__
    int ret;

    ret = cmmSend(com.prm.tcp.hdl, &b, 1);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comSendGprs(byte b) {
#ifdef __CHN_GPRS__
    int ret;

    ret = cmmSend(com.prm.gprs.hdl, &b, 1);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

static int comSendWifi(byte b) {
#ifdef __CHN_WIFI__
    int ret;

    ret = cmmSend(com.prm.wifi.hdl, &b, 1);
    CHECK(ret == RET_OK, lblKO);
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
          return comSendIp(b);
      case chnGprs:
          return comSendGprs(b);
      case chnWifi:
          return comSendWifi(b);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comSendBufCom(const byte * msg, word len) {
#ifdef __CHN_COM__
    int ret;

    ret = cmmSend(com.prm.mdm.hdl, msg, len);
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
    int ret;

    ret = cmmSend(com.prm.tcp.hdl, msg, len);
    CHECK(ret == RET_OK, lblKO);

    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSendBufIp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSendBufGprs(const byte * msg, word len) {
#ifdef __CHN_GPRS__
    int ret;

    ret = cmmSend(com.prm.gprs.hdl, msg, len);
    CHECK(ret == RET_OK, lblKO);

    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSendBufGprs ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comSendBufWifi(const byte * msg, word len) {
#ifdef __CHN_WIFI__
    int ret;

    ret = cmmSend(com.prm.wifi.hdl, msg, len);
    CHECK(ret == RET_OK, lblKO);

    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcFN("comSendBufWifi ret=%d\n", ret);
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
          return comSendBufIp(msg, len);
      case chnGprs:
          return comSendBufGprs(msg, len);
      case chnWifi:
          return comSendBufWifi(msg, len);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comRecvBufCom(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_COM__
    int ret = 0;
    uint8 ctl[1 + 16];
    tResponse rsp;
    word len;
    word sec;

    //byte b;
    //byte index;
    card counter;

    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);

    memset(ctl, 0, 1 + 16);
    if(trm) {
        *ctl = strlen((char *) trm);
        strcpy((char *) ctl + 1, (char *) trm); //zero is added at the end
        if(msg)
            len = bufDim(msg);
    }

    sec = 1;
    if(dly)
        sec = dly * 100;
    tmrStart(0, sec);
    counter = 0;

    /* //receive one byte at a time
       while(counter < RSP_LEN) {
       ret = cmmRecv(com.prm.mdm.hdl, &b, &len, 1);
       if (ret == RET_OK)
       goto lblAppend;
       CHECK((len > 0), lblAppend)
       for (index = 0; index < 16; index++) {
       // MODIF AS 24072008: Character NULL (\x00) is not considered as terminator
       if((b == ctl[index]) && (b != 0)) {
       goto lblAppend;
       }
       }
       rsp.buf[counter] = b;
       counter++;
       if(!tmrGet(0))
       goto lblKO;
       }

       lblAppend:
       if(msg) {
       ret = bufApp(msg, rsp.buf, counter);
       CHECK(ret >= 0, lblKO);
       }
       ret = counter;
     */
    ret = cmmRecv(com.prm.mdm.hdl, (byte *) rsp.buf, &rsp.len, sizeof(rsp.buf));
    CHECK((ret == RET_OK), lblKO);
    ret = rsp.len;
    bufApp(msg, (byte *) rsp.buf, ret);
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    tmrStop(0);
    trcFN("comRecvBufCom ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comRecvBufIp(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_TCP__
    int ret;
    uint8 ctl[1 + 16];
    tResponse rsp;
    word len;
    word sec;
    card counter;

    memset(ctl, 0, 1 + 16);
    if(trm) {
        *ctl = strlen((char *) trm);
        strcpy((char *) ctl + 1, (char *) trm); //zero is added at the end
        if(msg)
            len = bufDim(msg);
    }
    sec = 1;
    if(dly)
        sec = dly * 100;
    tmrStart(0, sec);
    counter = 0;

    ret = cmmRecv(com.prm.tcp.hdl, (byte *) rsp.buf, &rsp.len, sizeof(rsp.buf));
    CHECK((ret == RET_OK), lblKO);
    ret = rsp.len;
    bufApp(msg, (byte *) rsp.buf, ret);
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    tmrStop(0);
    trcFN("comRecvBufIp ret=%d\n", ret);
    return ret;
#else
    return -1;
#endif
}

static int comRecvBufGprs(tBuffer * msg, const byte * trm, byte dly) {
#ifdef __CHN_GPRS__
    int ret;
    uint8 ctl[1 + 16];
    tResponse rsp;
    word len;
    word sec;
    card counter;

    memset(ctl, 0, 1 + 16);
    if(trm) {
        *ctl = strlen((char *) trm);
        strcpy((char *) ctl + 1, (char *) trm); //zero is added at the end
        if(msg)
            len = bufDim(msg);
    }
    sec = 1;
    if(dly)
        sec = dly * 100;
    tmrStart(0, sec);
    counter = 0;

    ret = cmmRecv(com.prm.gprs.hdl, rsp.buf, &rsp.len, sizeof(rsp.buf));
    CHECK((ret == RET_OK), lblKO);
    ret = rsp.len;
    bufApp(msg, (byte *) rsp.buf, ret);
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    tmrStop(0);
    trcFN("comRecvBufGprs ret=%d\n", ret);
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
      case chnPpp:
          ret = comRecvBufIp(msg, trm, dly);
          break;
      case chnGprs:
      case chnWifi:
          ret = comRecvBufGprs(msg, trm, dly);  //comRecvBufGprs(msg, trm, dly);
          break;
      case chnUSB:
          ret = -1;             //comRecvBufUSB(msg, trm, dly);
      default:
          VERIFY(com.chn < chnEnd);
          ret = -1;
          break;
    }

    if(ret < 0)
        return ret;

    if(msg) {
        trcFS("comRecvBuf msg=%s\n", (char *) bufPtr(msg));
        trcS("bin: ");
        trcBN(bufPtr(msg), bufLen(msg));
        trcS("\n");
    } else if(trm) {
        trcFS("comRecvBuf trm=%s\n", (char *) trm);
        trcS("bin: ");
        trcBN(trm, strlen((char *) trm));
        trcS("\n");
    }
    return ret;
}

static int comRecvCom(byte * b, int dly) {
#ifdef __CHN_COM__
    int ret;
    tResponse rsp;

    VERIFY(b);
    VERIFY(com.chn == chnMdm || com.chn == chnHdlc || com.chn == chnCom1
           || com.chn == chnCom2 || com.chn == chnCom3);

    ret = cmmRecv(com.prm.mdm.hdl, rsp.buf, &rsp.len, 1);
    trcFN("cmmRecv ret=%d\n", ret);
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
    int ret;
    tResponse rsp;

    VERIFY(b);

    ret = cmmRecv(com.prm.tcp.hdl, rsp.buf, &rsp.len, 1);
    trcFN("cmmRecv ret=%d\n", ret);
    CHECK(ret == RET_OK, lblKO);
    //CHECK(rsp.len == 1, lblKO); //IP receive all message
    *b = rsp.buf[0];

    return 1;
  lblKO:
    trcFN("comRecvIp ret=%d\n", ret);
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
      case chnWifi:
          return comRecvIp(b, dly);
      case chnUSB:
          return -1;            //comRecvUSB(b, dly);
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comHangStartMdm(void) {
#ifdef __CHN_COM__
    trcS("comHangStartMdm\n");
    return 1;
//  lblKO:
//    return -1;
#else
    return -1;
#endif
}

static int comHangStartIp(void) {
#ifdef __CHN_TCP__
    int ret;

    trcS("comHangStartIp\n");

    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_LNET_DOWN);
    ret = cmmGeneral(&tlv);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
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
      case chnWifi:
          return comHangStartIp();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

static int comHangWaitMdm(void) {
#ifdef __CHN_COM__
    trcS("comHangWaitMdm\n");
    return 1;
//  lblKO:
//    return -1;
#else
    return -1;
#endif
}

static int comHangWaitIp(void) {
#ifdef __CHN_TCP__
    trcS("comHangWaitIp\n");
    return 1;
//  lblKO:
//    return -1;
#else
    return -1;
#endif
}

static int comHangWaitGprs(void) {
#ifdef __CHN_GPRS__
    int ret;

    trcS("comHangWaitGprs\n");
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    tlv_add_value8(&tlv, TAG_CMM_COMMAND, CMM_COMMAND_GSM_DOWN);
    ret = cmmGeneral(&tlv);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
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
      case chnWifi:
          return comHangWaitIp();
      case chnGprs:
          return comHangWaitGprs();
      default:
          VERIFY(com.chn < chnEnd);
          break;
    }
    return -1;
}

#endif
