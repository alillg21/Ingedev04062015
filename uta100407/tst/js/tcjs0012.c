#include <string.h>
#include "sys.h"
#include "tst.h"
#include "GCL.h"
#include "..\GCL_ETH\inc\gcl_wifi.h"

static code const char *srcFile= __FILE__;

typedef struct cfgConnectionConfig_st
{
   struct{
      gclWiFi_t config;
      char ipAddress[17];
      char tcpPort[6];
      char hostName[32];
      char dhcpHostName[32];
      char defaultLocalIpAddress[17];
      char defaultDefGateway[17];
      char defaultDns1IpAddress[17];
      char defaultDns2IpAddress[17];
      char domainName[21];
      char desiredSSID[33];
      char ownSSID[33];
      char key0Value[29];
      char key1Value[29];
      char key2Value[29];
      char key3Value[29];
   } WiFiConfig;

   
}cfgConnectionConfig_t;

cfgConnectionConfig_t connectionConfig;

static int16 connErrorCallBack(gclConfig_t * gcl) {
    gclSetError(0);
    return gclPostMessage(GCL_RETRY);
}

static void cfgWiFiInit(void)
{
/*	#define APN "demo"
	#define STATIC_HOST_ADDRESS "172.17.72.125"
	#define PORT "4000"
	#define GATEWAY "255.255.255.0"
	#define WIFI_SEC_KEY                "918131F3120DC6C48732C69441"
	#define WIFI_IP_LOCAL_ADDR          "172.17.72.149"

	//connectionConfig.WiFiConfig.config.connectionId = 1;
	WiFiConfig.ipAddress = STATIC_HOST_ADDRESS;
	WiFiConfig.tcpPort = PORT;
	WiFiConfig.useDhcp = 0;
    WiFiConfig.defaultLocalIpAddress  = WIFI_IP_LOCAL_ADDR;
	WiFiConfig.defaultDefGateway = GATEWAY;
	WiFiConfig.WiFiMode = NET_PRISM_MODE_INFRASTRUCTURE;
	WiFiConfig.desiredSSID = APN;*/

	//memset(&WiFiConfig,0,sizeof(gclWiFi_t));
	/*WiFiConfig.connectionId = 1;
	WiFiConfig.ipAddress = "172.17.72.125";
	WiFiConfig.tcpPort = "4000";
	WiFiConfig.useDhcp = 0;
    WiFiConfig.defaultLocalIpAddress  = "172.17.72.149";
	WiFiConfig.defaultDefGateway = "255.255.255.0";
	WiFiConfig.WiFiMode = NET_PRISM_MODE_INFRASTRUCTURE;
	WiFiConfig.desiredSSID = "demo";*/
	

	strncpy(connectionConfig.WiFiConfig.defaultDefGateway,"255.255.255.000",15);
	strncpy(connectionConfig.WiFiConfig.defaultLocalIpAddress,"172.17.72.149",15);
	strncpy(connectionConfig.WiFiConfig.desiredSSID,"demo",4);
	strncpy(connectionConfig.WiFiConfig.tcpPort,"4000",4);
	strncpy(connectionConfig.WiFiConfig.ipAddress,"172.17.72.125",15);
	strncpy(connectionConfig.WiFiConfig.defaultDns1IpAddress,"0.0.0.0",7);
	strncpy(connectionConfig.WiFiConfig.defaultDns2IpAddress,"0.0.0.0",7);
	strcpy(connectionConfig.WiFiConfig.dhcpHostName," ");
	strcpy(connectionConfig.WiFiConfig.domainName," ");
	strcpy(connectionConfig.WiFiConfig.hostName," ");
	strcpy(connectionConfig.WiFiConfig.key0Value," ");
	strcpy(connectionConfig.WiFiConfig.key1Value," ");
	strcpy(connectionConfig.WiFiConfig.key2Value," ");
	strcpy(connectionConfig.WiFiConfig.key3Value," ");
	strcpy(connectionConfig.WiFiConfig.ownSSID,"");

	//connectionConfig.WiFiConfig.config.connectionId=1;
	connectionConfig.WiFiConfig.config.useDhcp = 0;
	connectionConfig.WiFiConfig.config.WiFiMode = NET_PRISM_MODE_INFRASTRUCTURE;
	connectionConfig.WiFiConfig.config.defaultDefGateway      = connectionConfig.WiFiConfig.defaultDefGateway;
	connectionConfig.WiFiConfig.config.defaultDns1IpAddress   = connectionConfig.WiFiConfig.defaultDns1IpAddress;
	connectionConfig.WiFiConfig.config.defaultDns2IpAddress   = connectionConfig.WiFiConfig.defaultDns2IpAddress;
	connectionConfig.WiFiConfig.config.defaultLocalIpAddress  = connectionConfig.WiFiConfig.defaultLocalIpAddress;
	connectionConfig.WiFiConfig.config.dhcpHostName           = connectionConfig.WiFiConfig.dhcpHostName;
	connectionConfig.WiFiConfig.config.domainName             = connectionConfig.WiFiConfig.domainName;
	connectionConfig.WiFiConfig.config.ipAddress              = connectionConfig.WiFiConfig.ipAddress;
	connectionConfig.WiFiConfig.config.hostName               = connectionConfig.WiFiConfig.hostName;
	connectionConfig.WiFiConfig.config.tcpPort                = connectionConfig.WiFiConfig.tcpPort;
	connectionConfig.WiFiConfig.config.desiredSSID            = connectionConfig.WiFiConfig.desiredSSID;
	connectionConfig.WiFiConfig.config.ownSSID                = connectionConfig.WiFiConfig.ownSSID;
	connectionConfig.WiFiConfig.config.key0Value              = connectionConfig.WiFiConfig.key0Value;
	connectionConfig.WiFiConfig.config.key1Value              = connectionConfig.WiFiConfig.key1Value;
	connectionConfig.WiFiConfig.config.key2Value              = connectionConfig.WiFiConfig.key2Value;
	connectionConfig.WiFiConfig.config.key3Value              = connectionConfig.WiFiConfig.key3Value;
	connectionConfig.WiFiConfig.config.defaultWepKey = 0xffffffff;
	connectionConfig.WiFiConfig.config.communicationTimeout = 5000;
    connectionConfig.WiFiConfig.config.connectTimeout = 4000;
    connectionConfig.WiFiConfig.config.loginTimeout = 3000;
    connectionConfig.WiFiConfig.config.retries = 1;
	connectionConfig.WiFiConfig.config.wepEncryption = 0;

	//WiFiConfig.key0Value = "918131F3120DC6C48732C69441";


   //connectionConfig.WiFiConfig.config.ownSSID                = connectionConfig.WiFiConfig.ownSSID;
   //WiFiConfig.key0Value              = WIFI_SEC_KEY;
   //connectionConfig.WiFiConfig.config.key1Value              = connectionConfig.WiFiConfig.key1Value;
   //connectionConfig.WiFiConfig.config.key2Value              = connectionConfig.WiFiConfig.key2Value;
   //connectionConfig.WiFiConfig.config.key3Value              = connectionConfig.WiFiConfig.key3Value;
}


static int cfgWiFiSet(void)
{
	int ret;

   gclFunctionList_t funcList;
   ret = 0;

   funcList.predial  = gclWiFiPreDial;
   funcList.dial     = gclWiFiDial;
   funcList.connect  = gclWiFiConnect;
   funcList.error    = connErrorCallBack;
   funcList.retry    = gclConnectRetry;
   funcList.hangup   = gclWiFiHangUp;
   funcList.login    = gclNULLFunction;
   funcList.send     = gclWiFiSend;
   funcList.recv     = gclWiFiReceive;
   gclReset();
   ret = gclWiFiSet(&connectionConfig.WiFiConfig.config, &funcList, 0, NULL);
   return ret;
}


void tcjs0012(void){
    int ret;
    char *ptr;
    tBuffer buf; 
    byte dat[1024];

        
        bufInit(&buf,dat,1024);
        bufReset(&buf);

        
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= prtStart(); CHECK(ret>=0,lblKO);    
    
    //trcMode(1);
	ret= dspLS(0,"GclStart"); CHECK(ret>=0,lblKO);
    cfgWiFiInit();   
	ret= cfgWiFiSet(); CHECK(ret>=0,lblKO);

	ret= dspLS(2,"GclwifiDial..."); CHECK(ret>=0,lblKO); 
    //ret= utaGclDial(); CHECK(ret>=0,lblKO);
	ret = gclStart();
	prtS("bf gclstart");
    CHECK(ret==RET_OK,lblKO);
	
    ret = gclStartConnection();
	prtS("bf gclstartConn");
    CHECK(ret==RET_OK,lblKO);

	if(ret==RET_OK)
	{
		ret = 1;
	}

                do{
                ret= utaGclDialInfo();
                switch(ret){
                        case gclStaPreDial: ptr= "PreDial"; break;
                        case gclStaDial: ptr= "Dial"; break;
                        case gclStaConnect: ptr= "Connect"; break;
                        case gclStaLogin: ptr= "Login"; break;
                        case gclStaFinished: ptr= "Done"; break;
                        default: goto lblKO;
                }
            dspLS(2,ptr);
        }while(ret!=gclStaFinished);
    ret= dspLS(2,"utaGclWifiDial OK"); CHECK(ret>=0,lblKO); 

    /*ret= dspLS(0,"utaGclStart"); CHECK(ret>=0,lblKO); 
    ret= dspLS(1,ptr); CHECK(ret>=0,lblKO); 
    ret= utaGclStart(gclWifi,ptr); CHECK(ret>=0,lblKO);

        ret= dspLS(2,"utaGclwifiDial..."); CHECK(ret>=0,lblKO); 
    ret= utaGclDial(); CHECK(ret>=0,lblKO);

                do{
                ret= utaGclDialInfo();
                switch(ret){
                        case gclStaPreDial: ptr= "PreDial"; break;
                        case gclStaDial: ptr= "Dial"; break;
                        case gclStaConnect: ptr= "Connect"; break;
                        case gclStaLogin: ptr= "Login"; break;
                        case gclStaFinished: ptr= "Done"; break;
                        default: goto lblKO;
                }
            dspLS(2,ptr);
        }while(ret!=gclStaFinished);
    ret= dspLS(2,"utaGclWifiDial OK"); CHECK(ret>=0,lblKO); */
        
    /*ret= dspLS(0,"utaGclwifiRecv.."); CHECK(ret>=0,lblKO); 
        ret= utaGclRecvBuf(&buf,0,100); 
        ret= prtS(buf.ptr);CHECK(ret>=0,lblKO); 

    ret= dspLS(0,"Receiving done"); CHECK(ret>=0,lblKO);*/

    /* bufReset(&buf);
     bufAppStr(&buf,(byte *)"lalala\x0D\x0A");
     ret= prtS(buf.ptr);CHECK(ret>=0,lblKO);       

         ret= dspLS(0,"utaGclGprsSend.."); CHECK(ret>=0,lblKO);
         ret= utaGclSendBuf(bufPtr(&buf),bufLen(&buf));
     CHECK(ret==bufLen(&buf),lblKO);

        //ret= utaGclRecvBuf(&buf,0,100); 
        //ret= prtS(buf.ptr);CHECK(ret>=0,lblKO); 

        //ret= dspLS(0,"Receiving done"); CHECK(ret>=0,lblKO);

    ret= dspLS(3,"utaGclGprsStop"); CHECK(ret>=0,lblKO); 
    ret= utaGclStop(); CHECK(ret>=0,lblKO); */

    goto lblEnd;
lblKO:
   trcErr(ret);
        ret= utaGclDialErr();
        switch(ret){
                case gclErrCancel: ptr= "User cancelled"; break;
                case gclErrDial: ptr= "Dial error"; break;
                case gclErrBusy: ptr= "BUSY"; break;
                case gclErrNoDialTone: ptr= "NO DIAL TONE"; break;
                case gclErrNoCarrier: ptr= "NO CARRIER"; break;
                case gclErrNoAnswer: ptr= "NO ANSWER"; break;
                case gclErrLogin: ptr= "LOGIN PBM"; break;
                default: ptr= "KO!"; break;
        }
    dspLS(0,ptr);
lblEnd:
    tmrPause(3);
    utaGclStop();
    prtStop();
    dspStop();
}