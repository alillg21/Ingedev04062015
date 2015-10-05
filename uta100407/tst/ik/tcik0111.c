/// \file tcik0111.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unicapt.h>
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>

/** NON UTA Unitary test case for CDMA connection testing.
 */

static uint32 hHmi = 0;
static uint32 hPrt = 0;
static comInit_t init;
static comUartInit_t uartInit;

#define dftLogin "icicipos"
#define dftPwd "icicbank"
#define dftDomain "defaultDomain"
//free.fr
static const char *dftIP = "212.27.42.12";
static const char *dftPort = "0110";

// AT Command to initialize the modem
static char txCMD[30][60] = {

    "ATE0X1\r",                 // echo off & simple CONNECT messages
    "AT+crm=1\r",               // CDMA AT command
    //"AT$LGPKT=3\r",
    "AT&D0\r"
};

// LNET dial script...
static netScriptCmd_t hwtS_scriptCmd[] = {
    {"atdt#777\r", "CONNECT", NULL, -1, 0, 60}, // CDMA dial string
    {NULL, "NO CARRIER", NULL, -1, 1, 0},
    {NULL, "NO ANSWER", NULL, -1, 2, 0},
    {NULL, "NO DIAL", NULL, -1, 3, 0},
    {NULL, "BUSY", NULL, -1, 4, 0},
    {NULL, NULL, NULL, -1, 5, 0},   /* timeout */
};

typedef struct {
    int16 status;
    uint16 length;
    uint8 data[100];
} comOneResult_t;

static netScript_t hwtS_script = { hwtS_scriptCmd, 0 };
static uint8 Counter;

static char buffer[1024];
static dnsConfig_t dnsConfig;   // Set and configure DNS parameters

//static    netIpAddr_t         dnsAddr;
static struct in_addr inAddr;
static struct sockaddr_in destAddr;
static uint32 comHandle;        // comm handle
static comOneResult_t comResult;    // comm result structure

static long gRecieveBuffer(int16 lFramer,
                           int8 * o_pbyteBuffer, int16 * o_pjub2Size);
static int16 hSocket;
static uint32 ni;

static void show(const char *str, int idx) {
    static uint16 row = 0;

    if(idx == 0)
        row = 0;
    hmiADClearLine(hHmi, row);
    hmiADDisplayText(hHmi, row, 0, (char *) str);
    row += 1;
    if(row >= 15)
        row = 0;
}

static void print(const char *str) {
    prnPrint(hPrt, (char *) str);
}

/*************************************************************************/
/*      FUNCTION :  CDMA MODULE                                              */
/*      -----------------                                                */
/*      This function open, initialize, send and receive bytes on COM2.  */
/*************************************************************************/

int cdmaComConnect(void) {

// The following variables are used by LNET
    int ret;
    int16 retCode;              // check return code - now integer
    int16 state;

    print("cdmaComConnect");
    show("CDMA Connect", 0);

    memset(buffer, 0, sizeof(buffer));

    //retCode = comOpen ("MODEM3", &comHandle); --> MODEM3 in case of MODEM3 is Added in config file (Indian cable).
    retCode = comOpen("COM2", &comHandle);
    if(retCode == RET_OK)
        print("COM2/MODEM3 openOK");
    else {
        print("COM2/MODEM3 openNOK");
        return -1;
    }

// prior initialisation to use comSetPortParameters

    // set connection speed
    uartInit.speed = COM_BAUD_115200;
    uartInit.parity = COM_PARITY_NONE;
    uartInit.dataSize = COM_DATASIZE_8;
    uartInit.stopBits = COM_STOP_1;
    uartInit.rBufferSize = 1024;
    uartInit.sBufferSize = 1024;

    init.type = COM_INIT_TYPE_UART;
    init.level = COM_INIT_LEVEL_1;
    init.init.uartInit = &uartInit;

    // Set port parameters TEMPORARY

    state = comSetPortParameters(comHandle, COM_MODIF_PARAM_TEMPORARY, &init);
    if(state != RET_OK) {
        sprintf(buffer, "comSetPortParameters : %02x", state);
        print(buffer);
    } else {
        // print what is set inside
        /*
           print("modem :");
           sprintf(buffer,"speed : %02x", init.init.uartInit.speed);
           print(buffer);
           sprintf(buffer,"parity : %02x", init.init.uartInit.parity);
           print(buffer);
           sprintf(buffer,"dataSize : %02x", init.init.uartInit.dataSize);
           print(buffer);
           sprintf(buffer,"stopBits : %02x", init.init.uartInit.stopBits);
           print( buffer);
           sprintf(buffer,"rBufferSize : %02x", init.init.uartInit.rBufferSize);
           print(buffer);
           sprintf(buffer,"sBufferSize : %02x", init.init.uartInit.sBufferSize);
           print( buffer);
         */
    }

//  ***********************************

    if(state == RET_OK) {
        // The configuration process was successul
        // Send the AT Commands to the modem
        // It stops when the line size is 0
        for (Counter = 0; strlen(txCMD[Counter]); ++Counter) {
            // Send the AT commands / Asynchronous without the result
            retCode =
                comSendMsg(comHandle, (uint8) (strlen(txCMD[Counter])),
                           (uint8 *) txCMD[Counter]);
            memset(buffer, 0, sizeof(buffer));
            // Display on screen the AT command sent to the Modem. Start at line 0 column 0
            sprintf(buffer, "SEND AT COMMANDS\r");
            show(buffer, 1);
            memset(buffer, 0, sizeof(buffer));
            //  with length :   sprintf(buffer, "TX : %s : %d\r\n",txCMD[Counter], strlen(txCMD[Counter]));
            sprintf(buffer, "TX : %s", txCMD[Counter]);
            show(buffer, 1);

            if(retCode != RET_OK) {
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "Send failed %x", retCode);
                show(buffer, 1);
                print(buffer);
                break;
            }
            // Receiving the AT commands execution status from the modem
            // clear result structure first
            memset(&comResult, 0, sizeof(comResult));
            retCode =
                comReceiveMsgWait(comHandle, (uint16) sizeof(comResult.data),
                                  NULL, 1000, (uint32) sizeof(comResult),
                                  &comResult);
            if((retCode != RET_OK) && (retCode != COM_TIMEOUT)
               && (retCode != COM_INTER_TIMEOUT)) {
                memset(buffer, 0, sizeof(buffer));

                switch (retCode) {
                  case ERR_NOT_OPEN:
                      sprintf(buffer, "ERR_NOT_OPEN ");
                      break;
                  case ERR_INVALID_HANDLE:
                      sprintf(buffer, "ERR_INVALID_HANDLE ");
                      break;
                  case ERR_CONFLICT:
                      sprintf(buffer, "ERR_CONFLICT ");
                      break;
                  case ERR_SYS_RESOURCE_PB:
                      sprintf(buffer, "ERR_SYS_RESOURCE_PB ");
                      break;
                  case ERR_BUSY:
                      sprintf(buffer, "ERR_BUSY ");
                      break;
                  case ERR_PERIPHERAL:
                      sprintf(buffer, "ERR_PERIPHERAL ");
                      break;
                  case ERR_INSUFFICIENT_OUT_BUFFER:
                      sprintf(buffer, "ERR_INSUFFICIENT_OUT_BUFFER");
                      break;
                  case COM_BAD_BUFFERSIZE:
                      sprintf(buffer, "COM_BAD_BUFFERSIZE");
                      break;
                  case COM_OVERFLOW:
                      sprintf(buffer, "COM_OVERFLOW");
                      break;
                  case COM_TIMEOUT:
                      sprintf(buffer, "COM_TIMEOUT ");
                      break;
                  case COM_INTER_TIMEOUT:
                      sprintf(buffer, "COM_INTER_TIMEOUT ");
                      break;
                  case COM_ERR_USB_NO_PREINIT:
                      sprintf(buffer, "COM_ERR_USB_NO_PREINIT ");
                      break;
                  case COM_ERR_USB_NOT_CONNECTED:
                      sprintf(buffer, "COM_ERR_USB_NOT_CONNECTED ");
                      break;
                  case COM_ERR_DCD:
                      sprintf(buffer, "COM_ERR_DCD ");
                      break;
                  default:
                      sprintf(buffer, "UNKNOWN RETURN %X", retCode);
                }
                show(buffer, 1);
                print(buffer);
                break;
            }

            if(retCode < 0) {

                // flush buffer before each sprintf
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "Sta Rece: %02x", retCode);
                print(buffer);

            } else {
                // flush buffer before each sprintf
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "STATUS : %02x", comResult.status);
                print(buffer);
                sprintf(buffer, "LENGTH : %02x", comResult.length);
                print(buffer);

            }
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "RX : %s\n", comResult.data);
            show(buffer, 1);
            // wait for a key press
            //  sysWaitForKey();
        }
    } else {
        print("INIT COM failed");
    }
    // All the AT commands were executed successfully
    // close COM1
    retCode = comClose(comHandle);

    if(retCode != RET_OK) {
        print("COM Close failed");
        return -1;
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;

}

static int16 tcpClient(char *szEnderecoIP, char *szPorta) {
    int16 errorCode;
    int i, maxSegSize;
    struct timeval rxTimeout;

    memset(&destAddr, 0, sizeof(destAddr));

    /* Specify struct sock address */
    destAddr.sin_family = AF_INET;
//   destAddr.sin_port = htons_f (atoi(szPorta));
    destAddr.sin_port = htons((uint16) atoi(szPorta));
    destAddr.sin_addr.s_addr = inet_addr(szEnderecoIP);

    for (i = 0; i < 8; i++) {
        destAddr.sin_zero[i] = 0;
    }

    /* Create a socket */
    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(hSocket == -1) {
        errorCode = socketerror();
        show("SOCKET ERROR", 0);
        print("socket() error");
        return -1;
    }

    maxSegSize = 1450;

    /* The default TCP_MAXSEG is 536 bytes, try at 1450 bytes */
    if(setsockopt
       (hSocket, SOL_TCP, TCP_MAXSEG, (void *) &maxSegSize,
        sizeof(maxSegSize)) == 0) {
        print("setsockopt(TCP_MAXSEG) = OK");
    } else {
        print("setsockopt(TCP_MAXSEG) = Fail");
    }

    rxTimeout.tv_sec = 10;
    rxTimeout.tv_usec = 0;

    /* The default SO_RCVTIMEO is 60 seconds, try at 10 seconds */
    if(setsockopt
       (hSocket, SOL_SOCKET, SO_SNDTIMEO, (void *) &rxTimeout,
        sizeof(rxTimeout)) == 0) {
        print("setsockopt(SO_SNDTIMEO) = OK");
    } else {
        print("setsockopt(SO_SNDTIMEO) = Fail");
    }
    rxTimeout.tv_sec = 10;
    rxTimeout.tv_usec = 0;

    /* The default SO_RCVTIMEO is 60 seconds, try at 10 seconds */
    if(setsockopt
       (hSocket, SOL_SOCKET, SO_RCVTIMEO, (void *) &rxTimeout,
        sizeof(rxTimeout)) == 0) {
        print("setsockopt(SO_RCVTIMEO) = OK");
    } else {
        print("setsockopt(SO_RCVTIMEO) = Fail");
    }

    /* Connect to the server */
    errorCode =
        connect(hSocket, (struct sockaddr *) &destAddr, sizeof(destAddr));

    //inet_ntoa(destAddr.sin_addr.s_addr, buffer);
    print("Destination Address:");
    print(buffer);

    if(errorCode == -1) {
        show("connect sock err", 1);
        print("Connect socket() error");
        return -1;
    } else {
        show("connect SOCKET", 1);
        print("Connect socket() Success");
        return 1;
    }

}

int PPPConnect(void) {

    netChannelList_t pBuffer[NET_CHANNEL_MAX];
    netNiConfig_t config;
    int16 i, stat;
    int16 retCode;              // check return code - now integer

    show("PACE LNETTEST", 0);
    show("START LNETTEST", 1);

    // open PPP com handle
    netNiOpen(NET_NI_PPP, &ni);
    netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE, (void *) pBuffer);

    // display channels
    for (i = 0; pBuffer[i].name[0] != '\0'; i++) {
        // use the modem
        if(!strcmp(pBuffer[i].name, "COM2")) {
            break;
        }
    }

    memset(buffer, 0, sizeof(buffer));
    // format the response
    sprintf(buffer, "LNET Channel : %s\n", pBuffer[i].name);
    print(buffer);

    //channel not found should be checked here!
    if(pBuffer[i].name[0] == '\0') {
        // Channel not found
        show("CHN NOT FOUND", 1);
        return -1;
    }

    memset(&config, 0, sizeof(config));
    config.ppp.channel = i;
    config.ppp.fields = NI_PPP_IN_ECHO |
        NI_PPP_F_CHANNEL |
        NI_PPP_F_PPPCFG | NI_PPP_F_SERIALCONFIG | NI_PPP_F_SCRIPTCONFIG;

    config.ppp.serialConfig.speed = NET_SERIAL_BAUD_115200;
    config.ppp.serialConfig.parity = NET_PARITY_NONE;
    config.ppp.serialConfig.dataSize = NET_DATASIZE_8;
    config.ppp.serialConfig.stopBits = NET_STOP_1;

    config.ppp.pppCfg.fields = NI_PPP_F_LCPFLAGS |
        NI_PPP_F_IPCPFLAGS |
        NI_PPP_F_VJMAX |
        NI_PPP_F_LOCALUSERNAME | NI_PPP_F_LOCALPASSWORD | NI_PPP_F_INFLAGS;
    config.ppp.pppCfg.inFlags = NI_PPP_IN_DEFAULT_ROUTE;
    config.ppp.pppCfg.lcpFlags = NI_PPP_LCP_PAP |
        NI_PPP_LCP_CHAP | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP;
    config.ppp.pppCfg.ipcpFlags = NI_PPP_IPCP_ACCEPT_LOCAL |
        NI_PPP_IPCP_ACCEPT_REMOTE | NI_PPP_IPCP_DNS1 | NI_PPP_IPCP_DNS2;
    config.ppp.pppCfg.vjMax = 48;

    config.ppp.pppCfg.localUsername = dftLogin; // username for CDMA
    config.ppp.pppCfg.localPassword = dftPwd;   // password for CDMA

    config.ppp.scriptConfig = &hwtS_script;
    // Try to configure the PPP
    stat = netNiConfigSet(ni, NET_NI_LEVEL_1, &config);

    if(stat == RET_OK)
        print("netni config set ok!");
    else if(stat == NET_ERR_INVALID_NI)
        print("net err invalid ni!");
    else if(stat == NET_ERR_WRONG_APP)
        print("net err wrong app! ");
    else if(stat == NET_ERR_NI_IN_WRONG_STATE)
        print("net err ni in wrong state!");
    else if(stat == NET_ERR_LEVEL_NOT_SUPPORTED)
        print("err internal!");
    else if(stat == NET_ERR_READ_ONLY)
        print("err internal! ");
    else if(stat == NET_ERR_CHANNEL_NOT_FOUND)
        print("err channel not found! ");
    else if(stat == ERR_INTERNAL)
        print("err internal! ");
    else if(stat == NET_ERR_NI_NOT_CONFIGURED)
        print("net ni not configured! ");
    else
        print("net ni others err ");
    //start NI and block until negociation is finished
    stat = netNiStart(ni, 120 * SYS_TIME_SECOND);   //sd chaned from 120

    if(stat == RET_OK)
        print("LNET PPP is ok! ");
    else if(stat == NET_ERR_INVALID_NI)
        print("net err invalid ni! ");
    else if(stat == NET_ERR_WRONG_APP)
        print("net err wrong app! ");
    else if(stat == NET_ERR_NI_IN_WRONG_STATE)
        print("net err ni in wrong state! ");
    else if(stat == ERR_INTERNAL)
        print("err internal! ");
    else if(stat == NET_ECONNREFUSED)
        print("net econnrefused! ");
    else if(stat == NET_ETIMEDOUT)
        print("net etimedout ");
    else
        print("err others! ");

    if(stat != RET_OK) {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "ERR : stat is: %02d", stat);
        print(buffer);

        // Stop the interface
        netNiStop(ni);

        // Close the channel
        netNiClose(ni);
        print("End of LNET test");
        return -1;
    }
    //retrieve negociated local Ip address and dns addresses
    memset(&config, 0, sizeof(config));
    config.ppp.fields = NI_PPP_F_LOCALIPADDRESS | NI_PPP_F_PPPCFG;
    config.ppp.pppCfg.fields = NI_PPP_F_DNS1IPADDRESS | NI_PPP_F_DNS2IPADDRESS;

    if(netNiConfigGet(ni, NET_NI_LEVEL_1, &config) == RET_OK)
        print(">LNET config is ok!");
    else
        print(">LNET config is not ok!");

    // Set DNS server addresses parameters
    dnsConfig.fields =
        DNS_FIELD_DOMAINNAME | DNS_FIELD_DNSSERVER1 | DNS_FIELD_DNSSERVER2;
    dnsConfig.domainName = dftDomain;   //"defaultDomain";  //"websfr";
    dnsConfig.dnsServer1 = config.ppp.pppCfg.dns1IpAddress;
    dnsConfig.dnsServer2 = config.ppp.pppCfg.dns2IpAddress;

    inAddr.s_addr = config.ppp.pppCfg.dns1IpAddress;
    inet_ntoa(inAddr, buffer);
    print(buffer);

    inAddr.s_addr = config.ppp.pppCfg.dns2IpAddress;
    inet_ntoa(inAddr, buffer);
    print(buffer);

    retCode = dnsConfigSet(DNS_LEVEL_1, &dnsConfig);
    if(retCode != RET_OK) {
        print("Set DNS failed");
    }

    if((i = tcpClient((char *) dftIP, (char *) dftPort)) < 0)   // IP and Port required
    {
        print(">open socket not ok!");
        show("OPEN SOCKET FAIL", 1);
        return -1;
    } else {
        print(">open socket ok!");
        show("OPEN SOCKET OK", 1);
    }

    return 1;

}

int cdmaDisconnect(void) {

// The following variables are used by LNET
    int ret;
    int16 retCode;              // check return code - now integer
    int16 state;
    char message[10 + 1];

    print("cdmaDisconnect");

    memset(buffer, 0, sizeof(buffer));

    retCode = comOpen("COM2", &comHandle);

    if(retCode == RET_OK)
        print("COM/MODEM openOK");
    else {
        print("COM/MODEM openNOK");
        return -1;
    }
// prior initialisation to use comSetPortParameters
    // set connection speed
    uartInit.speed = COM_BAUD_115200;
    uartInit.parity = COM_PARITY_NONE;
    uartInit.dataSize = COM_DATASIZE_8;
    uartInit.stopBits = COM_STOP_1;
    uartInit.rBufferSize = 1024;
    uartInit.sBufferSize = 1024;

    init.type = COM_INIT_TYPE_UART;
    init.level = COM_INIT_LEVEL_1;
    init.init.uartInit = &uartInit;

    // Set port parameters TEMPORARY

    state = comSetPortParameters(comHandle, COM_MODIF_PARAM_TEMPORARY, &init);

    if(state != RET_OK) {
        sprintf(buffer, "comSetPortParameters : %02x", state);
        print(buffer);
    } else {
        // print what is set inside
        //  prnLinefeed(prnHandle,2);
        /*
           print("modem :");
           sprintf(buffer,"speed : %02x", init.uartInit.speed);
           print( buffer);
           sprintf(buffer,"parity : %02x", init.uartInit.parity);
           print( buffer);
           sprintf(buffer,"dataSize : %02x", init.uartInit.dataSize);
           print(buffer);
           sprintf(buffer,"stopBits : %02x", init.uartInit.stopBits);
           print( buffer);
           sprintf(buffer,"rBufferSize : %02x", init.uartInit.rBufferSize);
           print(buffer);
           sprintf(buffer,"sBufferSize : %02x", init.uartInit.sBufferSize);
           print(buffer);
         */
        //  prnLinefeed(prnHandle,4);
    }

    strcpy(message, "+++\r");

    if(retCode == RET_OK) {

        retCode =
            comSendMsg(comHandle, (uint8) (strlen(message)), (uint8 *) message);

        memset(buffer, 0, sizeof(buffer));
        // Display on screen the AT command sent to the Modem. Start at line 0 column 0

        if(retCode != RET_OK) {
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "Send failed %x", retCode);
            show(buffer, 1);
            print(buffer);
            return -1;
        }

        memset(&comResult, 0, sizeof(comResult));
        retCode =
            comReceiveMsgWait(comHandle, (uint16) sizeof(comResult.data), NULL,
                              1000, (uint32) sizeof(comResult), &comResult);
        if((retCode != RET_OK) && (retCode != COM_TIMEOUT)
           && (retCode != COM_INTER_TIMEOUT)) {
            memset(buffer, 0, sizeof(buffer));
            show("COM FAIL", 1);
            tmrPause(1);
            print(buffer);
            return -1;
        }

        strcpy(message, "ATH\r");

        tmrPause(1);

        retCode =
            comSendMsg(comHandle, (uint8) (strlen(message)), (uint8 *) message);

        memset(buffer, 0, sizeof(buffer));
        // Display on screen the AT command sent to the Modem. Start at line 0 column 0

        if(retCode != RET_OK) {
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "Send failed %x", retCode);
            show(buffer, 1);
            print(buffer);
            return -1;
        }

        memset(&comResult, 0, sizeof(comResult));
        retCode =
            comReceiveMsgWait(comHandle, (uint16) sizeof(comResult.data), NULL,
                              1000, (uint32) sizeof(comResult), &comResult);
        if((retCode != RET_OK) && (retCode != COM_TIMEOUT)
           && (retCode != COM_INTER_TIMEOUT)) {
            memset(buffer, 0, sizeof(buffer));
            show("COM FAIL", 1);
            tmrPause(1);
            print(buffer);
            return -1;
        }
        tmrPause(1);

    } else {
        print("INIT COM failed");
    }
    // All the AT commands were executed successfully
    // close COM1
    retCode = comClose(comHandle);

    if(retCode != RET_OK) {
        print("COM Close failed");
        return -1;
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void cdmaClose(void) {
    close(hSocket);
    netNiStop(ni);
    netNiClose(ni);

    cdmaDisconnect();
}

int cdmaConnect(void) {
    int ret;

    show("CdmaConnect..", 1);
   /********COM INIT********/
    ret = cdmaComConnect();
    if(ret < 0)
        goto lblKO;

    ret = PPPConnect();
    if(ret < 0)
        goto lblKO;

    ret = 1;
    goto lblEnd;

  lblKO:
    ret = -1;

  lblEnd:
    show("CdmaConnect Done", 1);
    return ret;

}

int cdmaSend(void *buffer, int bufferLength) {
    return write(hSocket, (void *) buffer, bufferLength);
}

int cdmaRecv(void *buffer, int bufferLength) {
    int iRetVal = 0;
    int iCnt = 0;
    char sbuf[100];

    print("cdma rec");
    tmrPause(10);
    while(1) {
        iRetVal = read(hSocket, (void *) buffer, bufferLength);
        if(iRetVal < 0) {
            sprintf(sbuf, "Error Reading %d ", iRetVal);
            print(sbuf);
            return (-1);
        }
        if(iRetVal == 0) {
            if(++iCnt > 10000) {
                prtS("Read Timeout ");
            } else
                continue;
        }
        sprintf(sbuf, "Received %d ", iRetVal);
        print(sbuf);
        return iRetVal;
    }
}

static int testPing(void) {
    int ret;
    icmpPingResult_t result;
    char buf[256];

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    show("ping 1...", 0);
    ret = icmpPingEx((char *) dftIP, 1000, 54, 32, &result);
    if(ret > 0)
        sprintf(buf, "1: %d", result.time);
    else
        strcpy(buf, "ping failed");
    show(buf, 1);

    show("ping 2...", 1);
    ret = icmpPingEx((char *) dftIP, 1000, 54, 32, &result);
    if(ret > 0)
        sprintf(buf, "2: %d", result.time);
    else
        strcpy(buf, "ping failed");
    show(buf, 1);

    show("ping 3...", 1);
    ret = icmpPingEx((char *) dftIP, 1000, 54, 32, &result);
    if(ret > 0)
        sprintf(buf, "3: %d", result.time);
    else
        strcpy(buf, "ping failed");
    show(buf, 1);

    return 1;
}

void tcik0111(void) {
    char mess_Rec[512];
    char mess_syn[] =
        "\x60\x00\x01\x00\x00"
        "\x02\x00"
        "\x70\x3C\x05\x80\x00\xC0\x00\x00"
        "\x16\x63\x91\x78\x01\x10\x13\x91\x82"
        "\x00\x00\x00"
        "\x00\x00\x00\x00\x01\x00"
        "\x00\x00\x01"
        "\x17\x40\x20"
        "\x12\x27"
        "\x05\x07"
        "\x01\x02"
        "\x00\x01"
        "\x00"
        "\x31\x32\x33\x34\x35\x36\x37\x38"
        "\x31\x32\x33\x34\x35\x36\x37\x38\x20\x20\x20\x20\x20\x20\x20";

    hmiOpen("DEFAULT", &hHmi);
    prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);

    cdmaConnect();
    testPing();
    cdmaSend(mess_syn, sizeof(mess_syn));
    cdmaRecv(mess_Rec, 512);
    cdmaClose();

    hmiClose(hHmi);
    prnClose(hPrt);
}
