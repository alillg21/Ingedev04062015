
#include <stdio.h>
#include <unicapt.h>
#include <string.h>
#include "sys.h"
#include "tc.h"
// for GSM
#include <gsm.h>
// for LNET
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>

//Unitary test case tcjd0066.
//Functions testing of GPRS connection:
//Function testing of GPRS connection, second part of test:
// 		GPRS is connected yet
// 		activate PDP Context 1
// 		send message via TCP/IP to server
//		GPRS disconnection
//First part of test tcjd0050

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

netIpAddr_t		localIpAddr;
netIpAddr_t		dns1IpAddr;
netIpAddr_t		dns2IpAddr;

uint32 h;

typedef struct{
	int16	status;
	uint16	length;
	uint8	datas[1];
}oneResult_t;

oneResult_t		hmiResult;

typedef struct sComChn {
    byte chn;                   //communications channel used
    byte chnRole;
    union {
         struct {                //chn= chnTcp...
            netNi_t hdl;
            int16 sck;          //it is also ppp.sck
            byte idx;
            int16 srvSck;
            int16 cltSck;
        } tcp;
        struct {                //chn= chnGprs
            netNi_t hdlNet;
            int16 sck;
            uint32 hdlGsm;
        } gprs;
    } prm;
} tComChn;
static tComChn com;

static int Start(void) {
    int ret;
    int16 err;
    uint8 OnOff;                //, rssi, ber;

    trcS("comStartGprs Beg\n");
    com.chn = chnGprs;

    ret = gsmOpen(&com.prm.gprs.hdlGsm, "MODEM3");  //Open the GSM Interface Library.
    CHECK(ret == RET_OK, lblKO);

    OnOff = 1;
    ret = gsmOnOff(com.prm.gprs.hdlGsm, GET, &OnOff);   //Check if the GSM Device is turn on.
    if((OnOff!=1)||(ret == RET_OK))
    	ret = gsmOnOff(com.prm.gprs.hdlGsm, SET, &OnOff);   //Turn on the GSM Device.
    CHECK(ret == RET_OK, lblKO);

    ret = 0;
    goto lblEnd;
  lblKO:
    trcFN("Error ret=%d\n", ret);
    ret = -1;
    if(ret == GSM_CME_ERROR) {
        gsmReadError(com.prm.gprs.hdlGsm, &err);
        if(err > 0)
            ret = -err;
        else if(err < 0)
            ret = err;
    }
    gsmClose(com.prm.gprs.hdlGsm);
  lblEnd:
    trcFN("comStartGprs ret=%d\n", ret);
    return ret;
}

static int SetGprsLocal(char *APN) {
    int ret;
    int16 err;
    int i;
    gsmGprsContext_t ctx;
    gsmGprsProfile_t prf;
    uint8 sta, net;
    const int actcid = 1;

    trcS("comStartGprs Beg\n");
    VERIFY(com.chn == chnGprs);

    memset(&ctx, 0, sizeof(ctx));
    ctx.cid = actcid;           // PDP context identifier
    strcpy((char *) ctx.PDPType, "IP"); // Specifies the type of PDP
    strcpy((char *) ctx.APN, APN);  // Specifies the logical name to select GGSN or extern PDP
    strcpy((char *) ctx.PDPAddress, "");    //Identify the Mobile in the address space(optional)
    ctx.dComp = 0;              //To control PDP data compression(optional)
    ctx.hComp = 0;              //To control PDP header compression(optional)

    memset(&prf, 0, sizeof(prf));
    prf.cid = actcid;           //PDP context identifier
    prf.precedence = 0;         //Precedence class
    prf.delay = 0;              // Delay class
    prf.reliability = 3;        //Reliability class
    prf.peak = 0;               //Peak throughput class
    prf.mean = 0;               // Mean throughput class

    trcFS("comStartGprsLocal APN=%s\n", APN);

    ret = gsmGprsGetNetworkStatus(com.prm.gprs.hdlGsm, &net);
    trcFN("comSetGprsLocal net=%d\n", net);
    CHECK(ret == RET_OK, lblKO);
    if(net == 1) {              //already connected
        ret = 1;                //registration already done
        goto lblEnd;
    } else {
        sta = 1;                //request GPRS attachment
        ret = gsmGprsAttachStatus(com.prm.gprs.hdlGsm, SET, &sta);
        trcFN("comSetGprsLocal ret=%d\n", ret);
        //CHECK(ret == RET_OK, lblKO);
		if(ret!=RET_OK){ //try to attach
		    for (i = 0; i < 6; i++) {  //wait GPRS attachment for 6, not 60 seconds
		        psyTimerWakeAfter(SYS_TIME_SECOND);
		        ret = gsmGprsAttachStatus(com.prm.gprs.hdlGsm, GET, &sta);
		        trcFN("comStartGprsLocal sta=%d\n", sta);
		        if(ret!=RET_OK){
		        	gsmReadError(com.prm.gprs.hdlGsm, &err);
    				trcFN("ReadErrorAttach %d", err);
    				if(err!=515) //Err 515 ==please wait
    					goto lblKO;
		        }
		        else{
		        	if(sta == 1)
		            	break;
		        }
		        //CHECK(ret == RET_OK, lblKO);
		        //if(sta == 1)
		        //    break;
		    }
		}
        ret = gsmGprsSetPDPContext(com.prm.gprs.hdlGsm, &ctx);
        trcFN("gsmGprsSetPDPContext ret=%d\n", ret);
        CHECK(ret == RET_OK, lblKO);
        ret = gsmGprsSetQoSProfile(com.prm.gprs.hdlGsm, &prf);
        CHECK(ret == RET_OK, lblKO);
    }
    ret = 1;
    goto lblEnd;
  lblKO:
    gsmReadError(com.prm.gprs.hdlGsm, &err);
    trcFN("ReadError %d", err);
    //prtS("Error code:");
    //num2dec(Err,err,3);
    //prtS(Err);
    gsmClose(com.prm.gprs.hdlGsm);
    ret = -1;
  lblEnd:
    trcFN("comStartGprs ret=%d\n", ret);
    return ret;
}

static int Set(const char *init) {
    int ret;
    int i;
    netChannelList_t buffer[NET_CHANNEL_MAX];
    netNiConfig_t cfg;
    char imsi[256];
    uint8 rssi, ber;
    uint16 err;
    uint8 status;
    uint8 pinstat, nbpin1, nbpin2, nbpuk1, nbpuk2;
    char *ptr;
    char usr[32];
    char pwd[32];
    char pin[32];
    static char *phone = "*99***1#";    //for GPRS connection
    char m_apn[64];
    char ip[20];

    VERIFY(com.chn == chnGprs);
    VERIFY(init);

    trcS("comSetGprs Beg\n");

    memset(pin, 0, 32);
    memset(usr, 0, 32);
    memset(pwd, 0, 32);
    memset(m_apn, 0, 50);
    memset(ip, 0, 20);

    ptr = pin;
    while(*init) {              //extract pin
        if(*init == '|')
            break;
        *ptr++ = *init++;
    }
    if(*init == '|')
        init++;                 //skip separator

    ptr = m_apn;
    while(*init) {              //extract apn
        if(*init == '|')
            break;
        *ptr++ = *init++;
    }
    if(*init == '|')
        init++;                 //skip separator

    ptr = usr;
    while(*init) {              //extract user
        if(*init == '|')
            break;
        *ptr++ = *init++;
    }
    if(*init == '|')
        init++;                 //skip separator

    ptr = pwd;
    while(*init) {              //extract password
        if(*init == '|')
            break;
        *ptr++ = *init++;
    }
    if(*init == '|')
        init++;                 //skip separator

    ptr = ip;
    while(*init) {              //extract IP
        if(*init == '|')
            break;
        *ptr++ = *init++;
    }
    if(*init == '|')
        init++;                 //skip separator

    gsmGetPinStatus(com.prm.gprs.hdlGsm, &pinstat, &nbpin1, &nbpin2, &nbpuk1,
                    &nbpuk2);
    trcFN("gsmGetPinStatus pinstat=%d\n", pinstat);
    if(pinstat == 1) {
        ret = gsmEnterPin(com.prm.gprs.hdlGsm, "", pin);
        CHECK(ret == RET_OK, lblKO);
    }

    ret = gsmGetImsi(com.prm.gprs.hdlGsm, imsi);
    if(ret != RET_OK) {
        gsmReadError(com.prm.gprs.hdlGsm, &err);
        //modem dependent only on WAVECOM modem. Mean: PLEASE WAIT INIT ON COMMAND PROCESSING ON PROGRESS
        CHECK(err == 515, lblKO);
        psyTimerWakeAfter(SYS_TIME_SECOND * 5);//5 seconds
        ret = gsmGetImsi(com.prm.gprs.hdlGsm, imsi);
        //CHECK(ret == RET_OK, lblKO);
    }

    ret=gsmReadNetworkStatus(com.prm.gprs.hdlGsm, &status);
    //Force operator selection only if GSM module is not registered on network
    if((status!=1)||(status!=5)||(ret != RET_OK)){ //1 and 5 registered status
    	ret = gsmOperatorSelection(com.prm.gprs.hdlGsm, 0, 2, "");
    	if(ret != RET_OK){
	    	psyTimerWakeAfter(SYS_TIME_SECOND * 3); //3 seconds
	        ret = gsmOperatorSelection(com.prm.gprs.hdlGsm, 0, 2, "");
	        CHECK(ret == RET_OK, lblKO);
        }
    }
    ret = gsmGetSignalParameters(com.prm.gprs.hdlGsm, &rssi, &ber);
    CHECK(ret == RET_OK, lblKO);
    trcFN("gsmGetSignalParameters rssi=%d\n", rssi);
    trcFN("gsmGetSignalParameters ber=%d\n", ber);

    ret = SetGprsLocal(m_apn);
    CHECK(ret == 1, lblKO);

    ret = netNiOpen(NET_NI_PPP, &com.prm.gprs.hdlNet);
    CHECK(ret == RET_OK, lblKO);
    ret =
        netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE,
                       (void *) buffer);
    CHECK(ret == RET_OK, lblKO);
    for (i = 0; buffer[i].name[0] != '\0'; i++)
        if(!strcmp(buffer[i].name, "MODEM3"))
            break;

    if(buffer[i].name[0] == '\0')
        CHECK(ret == RET_OK, lblKO);
    //channel not found should be checked here!
    memset(&cfg, 0, sizeof(cfg));

    cfg.ppp.fields =
        NI_PPP_F_CHANNEL | NI_PPP_F_PPPCFG | NI_PPP_F_PHONENUMBER |
        NI_PPP_F_LOCALIPADDRESS;
    cfg.ppp.channel = i;
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
    //cfg.ppp.pppCfg.vjMax = 0; //disable
    cfg.ppp.pppCfg.localUsername = usr;
    cfg.ppp.pppCfg.localPassword = pwd;
    cfg.ppp.phoneNumber = phone;

    ret = netNiConfigSet(com.prm.gprs.hdlNet, NET_NI_LEVEL_1, &cfg);
    trcFN("netNiConfigSet ret= %d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    ret = netNiStart(com.prm.gprs.hdlNet, 120 * SYS_TIME_SECOND);
    if(ret!=RET_OK){
    	trcFN("netNiStart Error %d\n", ret);
    	netNiStop(com.prm.gprs.hdlNet);
    	psyTimerWakeAfter(SYS_TIME_SECOND * 3); //3 seconds
    	ret = netNiConfigSet(com.prm.gprs.hdlNet, NET_NI_LEVEL_1, &cfg);
		trcFN("netNiConfigSet ret= %d\n", ret);
		CHECK(ret == RET_OK, lblKO);
    	ret = netNiStart(com.prm.gprs.hdlNet, 120 * SYS_TIME_SECOND);
    	trcFN("netNiStart ret= %d\n", ret);
    	gsmReadError(com.prm.gprs.hdlGsm, &err);
    	trcFN("ReadError %d\n", err);
    	if(err==515){
    		tmrPause(1);
    		ret = netNiConfigSet(com.prm.gprs.hdlNet, NET_NI_LEVEL_1, &cfg);
    		trcFN("netNiConfigSet ret= %d\n", ret);
    		CHECK(ret == RET_OK, lblKO);
    		ret = netNiStart(com.prm.gprs.hdlNet, 120 * SYS_TIME_SECOND);
    		trcFN("netNiStart ret= %d\n", ret);
    	}
    }
    CHECK(ret == RET_OK, lblKO);

    //retrieve negociated local Ip address and dns addresses
    memset(&cfg, 0, sizeof(cfg));
    cfg.ppp.fields = NI_PPP_F_LOCALIPADDRESS | NI_PPP_F_PPPCFG;
    cfg.ppp.pppCfg.fields = NI_PPP_F_DNS1IPADDRESS | NI_PPP_F_DNS2IPADDRESS;
    ret = netNiConfigGet(com.prm.gprs.hdlNet, NET_NI_LEVEL_1, &cfg);
    CHECK(ret == RET_OK, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    gsmReadError(com.prm.gprs.hdlGsm, &err);
    trcFN("ReadError %d\n", err);
    if(com.prm.gprs.hdlNet) {
        netNiStop(com.prm.gprs.hdlNet); //SIC: don't check return code!
        netNiClose(com.prm.gprs.hdlNet);    //SIC: don't check return code!
        com.prm.gprs.hdlNet = 0;
    }
    gsmGprsSetPDPActivate(com.prm.gprs.hdlGsm, 0, 255);
    gsmClose(com.prm.gprs.hdlGsm);
    com.prm.gprs.hdlGsm = 0;
    ret = -1;
  lblEnd:
    trcFN("comSetGprs ret=%d\n", ret);
    return ret;
}

static int Dial(const char *srv) {
    int ret = 0, retErr = 0;
    byte adr[4] = "";           //tcp address NNN.NNN.NNN.NNN

    //byte ladr[4] = "";
    //card dLocalPort;
    card dRemotePort;
    char RemotePort[10] = "";
    byte i;
    char *ptr;
    struct sockaddr_in dst;

    VERIFY(com.chn == chnGprs);
    trcFS("comDialGprsIp srv=%s\n", srv);
    com.chnRole = chnRoleClt;
    memset(adr, 0, 4);
    dRemotePort = 0;
    //dLocalPort = 0;
    i = 0;
    while(*srv) {
        if(*srv == '|')
            break;
        if(*srv != '.') {
            adr[i] *= 10;
            adr[i] += *srv - '0';
        } else
            i++;
        srv++;
    }
    CHECK(*srv == '|', lblKO);
    srv++;

    ptr = RemotePort;
    while(*srv) {               //extract remote Port
        if(*srv == '|')
            break;
        *ptr++ = *srv++;
    }
    if(*srv == '|')
        srv++;                  //skip separator

    ret = dec2num(&dRemotePort, RemotePort, 0);
    tmrPause(1); //why 3? make a unitary test case with calibration

    ret = com.prm.tcp.sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(ret == -1) {
        tmrPause(3);
        goto lblSCKKO;
    }
    //dLocalPort=0;

    CHECK(com.prm.tcp.sck >= 0, lblSCKKO);
    /*
       if(dLocalPort > 0) {
       dst.sin_len = sizeof(dst);
       dst.sin_family = AF_INET;
       dst.sin_port = htons_f((word) dLocalPort);
       dst.sin_addr.s_addr =
       htonl_f(IP_DOT2BIN(ladr[0], ladr[1], ladr[2], ladr[3]));
       for (i = 0; i < 8; i++)
       dst.sin_zero[i] = 0;
       ret = bind(com.prm.tcp.sck, (struct sockaddr *) &dst, sizeof(dst));
       CHECK(ret >= 0, lblKO);
       }
     */
    dst.sin_len = sizeof(struct sockaddr_in);
    dst.sin_family = AF_INET;
    dst.sin_port = htons_f((word) dRemotePort);
    dst.sin_addr.s_addr = htonl_f(IP_DOT2BIN(adr[0], adr[1], adr[2], adr[3]));
    memset(&dst.sin_zero, 0, sizeof(dst.sin_zero));

    ret =
        connect(com.prm.tcp.sck, (struct sockaddr *) &dst,
                sizeof(struct sockaddr_in));
    CHECK(ret >= 0, lblConnect);
    ret = 1;

    goto lblEnd;
  lblSCKKO:
    retErr = socketerror();
    trcFN("Error %d", retErr);
    ret = retErr;
    goto lblEnd;
  lblConnect:
    retErr = socketerror();
    trcFN(" connect Error %d", retErr);
    comHangStart();
    comHangWait();
    ret = retErr;
    goto lblEnd;

  lblKO:
    ret = -1;
  lblEnd:
    trcFN("comDialGprsIp ret=%d\n", ret);
    return ret;
}

static int SendBuf(const byte * msg, word len) {
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
    trcFN("comSendBufIp ret=%d\n", ret);
    return ret;
}

static int Recv(byte * b, int dly) {
    int ret = 0;
    struct timeval tmp;

    VERIFY(b);
    tmp.tv_sec = dly;
    tmp.tv_usec = 0;
    ret = -1;
    switch (com.chnRole) {
      case chnRoleSrv:
          ret = recv(com.prm.tcp.srvSck, b, 1, 0);
          break;
      case chnRoleClt:
          setsockopt(com.prm.tcp.sck, SOL_SOCKET, SO_RCVTIMEO, &tmp,
                     sizeof(tmp));
          ret = read(com.prm.tcp.sck, b, 1);
          break;
      default:
          VERIFY(ret >= 0);
          break;
    }
    return ret;
}

static int HangStart(void) {
    int ret = 0;

    trcS("comHangStartIp\n");
    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 || com.chn == chnPpp
           || com.chn == chnGprs);

    switch (com.chnRole) {
      case chnRoleSrv:
          ret = close(com.prm.tcp.srvSck);
          ret = close(com.prm.tcp.cltSck);
          break;
      case chnRoleClt:
          ret = close(com.prm.tcp.sck);
          break;
    }
    if(ret < 0)
        return -1;

    return 1;
}

static int Stop(void) {
    trcS("comStopGprs\n");
    VERIFY(com.chn == chnGprs);

    if(com.prm.gprs.hdlNet) {
        netNiStop(com.prm.gprs.hdlNet); //SIC: don't check return code!
        netNiClose(com.prm.gprs.hdlNet);    //SIC: don't check return code!
        com.prm.gprs.hdlNet = 0;
    }
    // Disconnect the PDP context to avoid GSM Data problem
    // Not required if you want to do several GPRS without GSM data
    //gsmGprsSetPDPActivate(com.prm.gprs.hdlGsm, 0, 255);//IK
    //tmrPause(3); //TO be verified!
    //gsmHangUp(com.prm.gprs.hdlGsm);
    gsmClose(com.prm.gprs.hdlGsm);
    com.prm.gprs.hdlGsm = 0;
    return 1;
}


int tcjd0066(int pI1, int pI2, char *pS1, char *pS2)  // GPRS Send msg
{
	int ret;
	char *ptr;
	byte dRsp[1024];
	tBuffer bRsp;
	byte dReq[1024];
	tBuffer bReq;
	word idx;
	card len;
	byte b;

  bufInit(&bRsp, dRsp, 1024);
  bufReset(&bRsp);
  bufInit(&bReq, dReq, 1024);
  bufReset(&bReq);            //reset request buffer
  //memset(ptr, 0, 1024 + 1);

  trcS("GPRS: Start\n");
  tcDsp(" Start...");
  ret = Start();  //SIM not initialised yet
  trcFN("Start : ret=%d\n", ret);
  CHECK(ret >= 0, lblKO);
  tcDspPrev(" Start OK");
  tcDsp(" Set...");
  ptr= "1234|orange.fr|danfr|mq36noqt|";
  ret = Set(ptr);
  trcFN("Set : ret=%d\n", ret);
  CHECK(ret >= 0, lblKO);
  tcDspPrev(" Set OK");
  tcDsp(" DialIP ...");
  ret = Dial("82.247.161.69|6789");  //topup
  trcFN("Dial : ret=%d\n", ret);
  CHECK(ret >= 0, lblKO);
  tcDspPrev(" DialIP OK");
  ret = bufApp(&bReq, (byte *) "\x00\x00\x00\x3A\x01\x01\x04\x00\x85\x0B\x03\x35\x56\x00\x05\x03\x6C\x6F\x67\x69\x6E\x00\x1D\x03\x32\x30\x30\x37\x30\x34\x32\x34\x31\x39\x30\x33\x31\x30\x00\x06\x03\x30\x39\x39\x39\x39\x39\x39\x39\x00\x0F\x03\x30\x30\x31\x30\x00\x14\x03\x30\x00\x01 ", 62);    //binairy xml header
  CHECK(ret > 0, lblKO);
  tcDsp(" Send ...");  
  ret = SendBuf(bufPtr(&bReq), bufLen(&bReq));
  trcFN("comSendBuf : ret=%d\n", ret);
  tcDspPrev(" Send OK");  
  len = 0;
  idx = 0;
  tcDsp(" Receive ...");  
  trcS(" Receive length:");
  for (idx = 0; idx < 4; idx++) {
        ret = Recv(&b, 30); //receive characters
        CHECK(ret >= 0, lblKO);
        len *= 0x100;
        len += b;
    }
	trcFN("Rsp length: len=%d\n", len);
	trcS("Receive message:");
    for (idx = 0; idx < len; idx++) {
        ret = Recv(&b, 30); //receive characters
        CHECK(ret >= 0, lblEndRecv);
        bufApp(&bRsp, &b, sizeof(byte));
    }
lblEndRecv:
    trcFN("lblEndRecv: ret=%d\n", ret);
    trcS("recvRsp: ");
    trcBAN(bufPtr(&bRsp), bufLen(&bRsp));
    trcS("\n");
    ret = tcDspPrev(" Receiving done");
    CHECK(ret >= 0, lblKO);
	tmrPause(1);
	goto lblEnd;
lblKO:
    trcErr(ret);
    tcDsp(" KO!");
lblEnd:
    ret = tcDsp(" HangStart ...");
    ret=HangStart();
    trcFN(" HangStart : ret=%d\n", ret);
    ret = tcDsp(" Stop GPRS...");
    ret=Stop();
    trcFN(" Stop : ret=%d\n", ret);
}

