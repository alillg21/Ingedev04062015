/*************************************************************************/
/*                                INGENICO                               */
/*-----------------------------------------------------------------------*/
/*     FILE  COMMUNICATION.C                                             */
/*-----------------------------------------------------------------------*/
/*+++******* INCLUDES ************************************************---*/
#include <stdio.h>
#include <string.h>
#include <unicapt.h>
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h> 
#include <LNetSocket.h>


#include <string.h>  // for memset, strcpy, etc

static comInit_t         init;
static comUartInit_t     uartInit;
static comProtocolCfg_t  protocol;
static comUartProtocolCfg_t uartProtocol;

// AT Command to initialize the modem
static char txCMD[30][60]={
	"AT\r",                                     // attention modem!
	"ATE0X1\r",                                 // echo off & simple CONNECT messages
	"AT+crm=1\r",                             // CDMA AT command
    "AT\r",                                     // attention modem!
    "AT$LGPKT=3\r",
	"ATDT #777\r",                                     // attention modem!
	""
};

/*static char txCMD[30][60]={
		"ATE0X1$LGPKT=3\r"                                 // echo off & simple CONNECT messages
};*/



// LNET dial script...
static netScriptCmd_t hwtS_scriptCmd[]= 
{	
	
	{"AT\r",				 "OK", 			 NULL,		 1,	   0,	  60},
	{"atdt#777\r",           "CONNECT",      NULL,      -1,    0,     60},  // CDMA dial string
	{NULL,                   "NO CARRIER",   NULL,      -1,    1,     0},
	{NULL,                   "NO ANSWER",    NULL,      -1,    2,     0},
	{NULL,                   "NO DIAL",      NULL,      -1,    3,     0},
	{NULL,                   "BUSY",         NULL,      -1,    4,     0},
	{NULL,                   NULL,           NULL,      -1,    5,     0},    /* timeout */
};


// POP user login string
static uchar POP_LOGIN_STR[] = { 'u','s','e','r',' ','d','a','n','f','r',0x0d,0x0a };
#define LEN_POP_LOGIN_STR 12

// POP user password string
static uchar POP_PASSWD_STR[] = { 'p','a','s','s',' ','m','q','3','6','n','o','q','t',0x0d,0x0a };
#define LEN_POP_PASSWD_STR 15

// STAT -  status string
static uchar POP_STAT_STR[] = { 's','t','a','t',0x0d,0x0a };
#define LEN_POP_STAT_STR 6

// RETR -  retrieve first email message
static uchar POP_RETR_STR[] = { 'r','e','t','r',' ', '1', 0x0d,0x0a };
#define LEN_POP_RETR_STR 8

// QUIT -  quit string
static uchar POP_QUIT_STR[] = { 'q','u','i','t',0x0d,0x0a };
#define LEN_POP_QUIT_STR 6

// Why do I need to define this?
   typedef struct
   {
      int16    status;
      uint16   length;
      uint8    data[100];
   } comOneResult_t;


// don't use DSR and DCD signals for PPP dial...
// static netScript_t hwtS_script = { hwtS_scriptCmd, NI_MODEM_IN_DSRDCD};
static netScript_t hwtS_script = { hwtS_scriptCmd, 0};




static uint8 	Counter;
		
static  char buffer[1024];
static  uchar rcvData[1024*4];
static  dnsConfig_t			dnsConfig;					// Set and configure DNS parameters  
static 	netIpAddr_t 		dnsAddr;
static 	struct in_addr		inAddr;
static  struct sockaddr_in  destAddr;

// local static printer handle
static uint32  prnHandle;        // print handle
static uint32  comHandle;        // comm handle
static uint32  displayHandle;    // display handle
static comOneResult_t  comResult;// comm result structure

static uint8   key;             // keyboard buffer


// local function protocols
static void  pingTest(char *addr);
static int16 tcpClient(char *szEnderecoIP, char *szPorta);
static long gRecieveBuffer(int16 lFramer,
                           int8  *o_pbyteBuffer,
                           int16 *o_pjub2Size);
static int16 hSocket;


/*************************************************************************/
/*      FUNCTION india1Test                                              */
/*      -----------------                                                */
/*      This function open, initialize, send and receive bytes on COM1.  */
/*************************************************************************/
void tcsd0000(void)
{	
   // The following variables are used by LNET
   uint32           ni;
   netChannelList_t pBuffer[NET_CHANNEL_MAX];
   netNiConfig_t    config;
   int16	        i,stat;
   uint8            keyChar;   // wait for this key
   int16            retCode;    // check return code - now integer
   uint32           pSigValue,pSigDefinition;



/*** STANDARD HMI & PRINTER OPEN ROUTINE ***/
 
   // Open display channel...
   hmiOpen("DEFAULT", &displayHandle);	

   hmiADClearLine (displayHandle, HMI_ALL_LINES);
   hmiADDisplayText(displayHandle, 0,0, "INDIA1 DIAL TEST");

   retCode = prnOpen("DEFAULT", &prnHandle, PRN_IMMEDIATE);

   if (retCode != RET_OK)    
   {
      hmiADDisplayText (displayHandle, 3, 2, "Error Open Printer");
	  hmiKeyWait (displayHandle, &keyChar, PSY_INFINITE_TIMEOUT);
      return;
   }
   else
   {
    //  prnPrint( prnHandle, "Printer Ready");
   }

    // clear scratch buffer	
	memset (buffer, 0, sizeof(buffer));

    //open MODEM channel  (MODEM 3 is the GPRS MODEM)
    retCode = comOpen ("COM1", &comHandle);
	
	if (retCode == RET_OK)
	{
		prnPrint(prnHandle, "COM / MODEM open = OK\n");
	}
	else
	{
		prnPrint(prnHandle, "COM / MODEM open NOK\n");
		return;
	}

	// Specify the type
	init.type = COM_INIT_TYPE_UART;
	init.level = COM_INIT_LEVEL_1;
	init.init.uartInit = &uartInit;

	// Set up connection speed and caracterisis
	uartInit.speed     = COM_BAUD_115200;
 	uartInit.parity    = COM_PARITY_NONE;
 	uartInit.dataSize  = COM_DATASIZE_8;
	uartInit.stopBits  = COM_STOP_1;
 
	// Set up the protocol parameters
	protocol.type = COM_PROT_CFG_TYPE_UART;
	protocol.level = COM_PROT_CFG_LEVEL_1;
	protocol.protCfg.uartCfg = &uartProtocol;

	// Set up the UART parameters
	uartProtocol.flowControl      = COM_FLOW_NONE;

    // inter character timeout of 120 * 10 ms = 2 secs
	uartProtocol.interCharTimeOut = 200; 
	
	// Try to configure the modem with the previous data
    retCode = comChanInit( comHandle, COM_MODIF_PARAM_TEMPORARY, &init, 
	   COM_SET_PROTOCOL_TEMPORARY, &protocol );
	
  	if (1) // (retCode == RET_OK)
  	{
		// The configuration process was successul
		// Send the AT Commands to the modem
		// It stops when the line size is 0
		for ( Counter=0; strlen(txCMD[Counter]); ++Counter )
		{			
			retCode = comGetSignals  (comHandle,&pSigValue,&pSigDefinition ) ;
			if(retCode != RET_OK)
			{
				sprintf(buffer,"%02d",retCode);
				prnPrint(prnHandle, buffer);

			}

            prnPrint(prnHandle, "SIGNALS");

			sprintf(buffer,"%02d",pSigValue);
			prnPrint(prnHandle, buffer);

			sprintf(buffer,"%02d",pSigDefinition);
			prnPrint(prnHandle, buffer);

			// Send the AT commands / Asynchronous without the result
			retCode = comSendMsg(comHandle, strlen(txCMD[Counter]), (uint8 *)txCMD[Counter]);
			
			// Clear the screen
			hmiADClearLine(displayHandle, HMI_ALL_LINES);


            // flush buffer before each sprintf
            memset (buffer, 0, sizeof(buffer));

		    // Display on screen the AT command sent to the Modem. Start at line 0 column 0 
			sprintf(buffer,"SEND AT COMMANDS\r");
			hmiADDisplayText(displayHandle, 0, 0, buffer);

            // flush buffer before each sprintf
            memset (buffer, 0, sizeof(buffer));

            //	with length :	sprintf(buffer, "TX : %s : %d\r\n",txCMD[Counter], strlen(txCMD[Counter]));
			sprintf(buffer, "TX : %s",txCMD[Counter]);

            // display text on 2nd screen line
			hmiADDisplayText(displayHandle, 1, 0, buffer);

            if(retCode != RET_OK)
			{
               // flush buffer before each sprintf
               memset (buffer, 0, sizeof(buffer));
				
				// Sending failed
				sprintf(buffer,"Sending command failed %x", retCode);
                
                // display the error on 4th (bottom) line
				hmiADDisplayText(displayHandle, 3, 0, buffer);

                // print the failure as well
           		prnPrint(prnHandle, buffer);
				// break;	
			}
		
			// Receiving the AT commands execution status from the modem

            // clear result structure first
            memset (&comResult, 0, sizeof(comResult));
			
            retCode = comReceiveMsgWait(comHandle, (uint16)sizeof(comResult.data),NULL,1000,(uint32)sizeof(comResult),&comResult);
 
			if ((retCode != RET_OK) &&
                (retCode != COM_TIMEOUT) &&
                (retCode != COM_INTER_TIMEOUT))
			{
                
                // flush buffer before each sprintf
                memset (buffer, 0, sizeof(buffer));

				switch(retCode)
				{
					case ERR_NOT_OPEN :
						sprintf(buffer, "ERR_NOT_OPEN ");
						break;
					case ERR_INVALID_HANDLE :
						sprintf(buffer, "ERR_INVALID_HANDLE ");
						break;
					case ERR_CONFLICT :
						sprintf(buffer, "ERR_CONFLICT ");
						break;
					case ERR_SYS_RESOURCE_PB :
						sprintf(buffer, "ERR_SYS_RESOURCE_PB ");
						break;
					case ERR_BUSY :
						sprintf(buffer, "ERR_BUSY ");
						break;
					case ERR_PERIPHERAL :
						sprintf(buffer, "ERR_PERIPHERAL ");
						break;
					case ERR_INSUFFICIENT_OUT_BUFFER :
						sprintf(buffer, "ERR_INSUFFICIENT_OUT_BUFFER");
						break;
					case COM_BAD_BUFFERSIZE :
						sprintf(buffer, "COM_BAD_BUFFERSIZE");
						break;
					case COM_OVERFLOW :
						sprintf(buffer, "COM_OVERFLOW");
						break;
					case COM_TIMEOUT :
						sprintf(buffer, "COM_TIMEOUT ");
						break;
					case COM_INTER_TIMEOUT :
						sprintf(buffer, "COM_INTER_TIMEOUT ");
						break;
					case COM_ERR_USB_NO_PREINIT :
						sprintf(buffer, "COM_ERR_USB_NO_PREINIT ");
						break;
					case COM_ERR_USB_NOT_CONNECTED :
						sprintf(buffer, "COM_ERR_USB_NOT_CONNECTED ");
						break;
					case COM_ERR_DCD :
						sprintf(buffer, "COM_ERR_DCD ");
						break;
					default :
						sprintf(buffer, "UNKNOWN RETURN %X", retCode);
				}
				
				// display & print the error and exit
                // display the error on 4th (bottom) line
				hmiADDisplayText(displayHandle, 3, 0, buffer);
           		prnPrint(prnHandle, buffer);
				// break;
			}

            if (retCode < 0)    
            {
#ifdef DEBUG_STATUS

               // flush buffer before each sprintf
               memset (buffer, 0, sizeof(buffer));

   	           sprintf(buffer,"status Receive: %02x",retCode);
   	           prnPrint (prnHandle, buffer);  
#endif
            }
            else   
            {
               
               // flush buffer before each sprintf
               memset (buffer, 0, sizeof(buffer));

               prnLinefeed(prnHandle,1);
               sprintf (buffer, "STATUS : %02x",comResult.status);
               prnPrint (prnHandle, buffer);
               sprintf (buffer, "LENGTH : %02x",comResult.length);
               prnPrint (prnHandle, buffer);
               //sprintf (buffer, "DATA : %c%c%c%c%c%c ",comResult.data[0], comResult.data[1], comResult.data[2], comResult.data[3],comResult.data[4], comResult.data[5]);
               //prnPrint (prnHandle, buffer);
               prnLinefeed(prnHandle,2);
            }


            // flush buffer before each sprintf
            memset (buffer, 0, sizeof(buffer));

            // format the response
            sprintf(buffer, "RX : %s\n",comResult.data);

            // display response on 3rd line
			hmiADDisplayText(displayHandle, 2, 0, buffer);
           		
			// wait for a key press
//			hmiKeyWait(displayHandle, &key, PSY_INFINITE_TIMEOUT);
			hmiKeyWait(displayHandle, &key, 200);
		}
   }   
   else 
   {
   		prnPrint(prnHandle,"INIT COM failed");
   }
		
   // All the AT commands were executed successfully      
   // close COM1
   retCode = comClose( comHandle );
	
   if (retCode != RET_OK)
   {
      prnPrint(prnHandle,"COM Close failed");
   	  return;
   }


   // Now we are trying to establish a PPP connection


   hmiADClearLine (displayHandle, HMI_ALL_LINES);
   hmiADDisplayText(displayHandle, 0,0, "INDIA1 LNET TEST");
   hmiADDisplayText(displayHandle, 1,0, "Starting LNET...");

	// open PPP com handle   
   	netNiOpen(NET_NI_PPP,&ni);

	// 
	netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE, (void*)pBuffer );

    // display channels
	for(i=0;pBuffer[i].name[0]!='\0';i++)
	{
       // use the modem
	    if (!strcmp(pBuffer[i].name,"COM1"))
    	{
        	break;
      	}
    }

    // flush buffer before each sprintf
    memset (buffer, 0, sizeof(buffer));

    // format the response
    sprintf(buffer, "LNET Channel : %s\n",pBuffer[i].name);
    
    prnPrint(prnHandle, buffer);
    
    //channel not found should be checked here!
    if(pBuffer[i].name[0] == '\0')
    {
    	// Channel not found
    	return;	
    }
    
    // Specify the PPP parameters
    memset(&config,0,sizeof(config));

	config.ppp.channel = i;

	config.ppp.fields					= NI_PPP_IN_ECHO|
										  NI_PPP_F_CHANNEL|
										  NI_PPP_F_PPPCFG|
										  NI_PPP_F_SERIALCONFIG|
										  NI_PPP_F_SCRIPTCONFIG;
										  
	config.ppp.serialConfig.speed    	= NET_SERIAL_BAUD_115200;
	config.ppp.serialConfig.parity   	= NET_PARITY_NONE;	
	config.ppp.serialConfig.dataSize	= NET_DATASIZE_8;	
	config.ppp.serialConfig.stopBits 	= NET_STOP_1;
	
	config.ppp.pppCfg.fields         	= NI_PPP_F_LCPFLAGS|
										  NI_PPP_F_IPCPFLAGS|
										  NI_PPP_F_VJMAX|
										  NI_PPP_F_LOCALUSERNAME|
										  NI_PPP_F_LOCALPASSWORD|
										  NI_PPP_F_INFLAGS;
	config.ppp.pppCfg.inFlags			= NI_PPP_IN_DEFAULT_ROUTE;
	config.ppp.pppCfg.lcpFlags       	= NI_PPP_LCP_PAP|
										  NI_PPP_LCP_PCOMP|
										  NI_PPP_LCP_ACOMP;	
	config.ppp.pppCfg.ipcpFlags      	= NI_PPP_IPCP_ACCEPT_LOCAL|
										  NI_PPP_IPCP_ACCEPT_REMOTE|
										  NI_PPP_IPCP_DNS1|
										  NI_PPP_IPCP_DNS2;	config.ppp.pppCfg.vjMax          = 48;

	//config.ppp.pppCfg.localUsername  = "icicipos";//"2238038637"; //icicipos username for CDMA India
	//config.ppp.pppCfg.localPassword  = "icicbank";//"2238038637"; // password for CDMA India
	config.ppp.pppCfg.localUsername  = "4432512268";//"2238038637"; //icicipos username for CDMA India
	config.ppp.pppCfg.localPassword  = "4432512268";//"2238038637"; // password for CDMA India
	
	config.ppp.scriptConfig          = &hwtS_script;

	// Try to configure the PPP
	stat = netNiConfigSet(ni, NET_NI_LEVEL_1, &config);
	
	if (stat == RET_OK )
		prnPrint(prnHandle, "netni config set ok! "); 
	else if ( stat == NET_ERR_INVALID_NI )
		prnPrint(prnHandle, "net err invalid ni! "); 
	else if ( stat == NET_ERR_WRONG_APP )
		prnPrint(prnHandle, "net err wrong app! "); 
	else if ( stat == NET_ERR_NI_IN_WRONG_STATE )
		prnPrint(prnHandle, "net err ni in wrong state! "); 
	else if ( stat == NET_ERR_LEVEL_NOT_SUPPORTED )
		prnPrint(prnHandle, "err internal! "); 
	else if ( stat == NET_ERR_READ_ONLY )
		prnPrint(prnHandle, "err internal! "); 
	else if ( stat == NET_ERR_CHANNEL_NOT_FOUND )
		prnPrint(prnHandle, "err channel not found! "); 
	else if ( stat == ERR_INTERNAL )
		prnPrint(prnHandle, "err internal! "); 
	else if ( stat == NET_ERR_NI_NOT_CONFIGURED )
		prnPrint(prnHandle, "net ni not configured! "); 
	else 
		prnPrint(prnHandle, "net ni others err "); 
	
	//start NI and block until negociation is finished
	stat = netNiStart(ni, 120*SYS_TIME_SECOND); //sd chaned from 120
	
	
	if (stat == RET_OK )
		prnPrint(prnHandle, "LNET PPP is ok! "); 
	else if ( stat == NET_ERR_INVALID_NI )
		prnPrint(prnHandle, "net err invalid ni! "); 
	else if ( stat == NET_ERR_WRONG_APP )
		prnPrint(prnHandle, "net err wrong app! "); 
	else if ( stat == NET_ERR_NI_IN_WRONG_STATE )
		prnPrint(prnHandle, "net err ni in wrong state! "); 
	else if ( stat == ERR_INTERNAL )
		prnPrint(prnHandle, "err internal! "); 
	else if ( stat == NET_ECONNREFUSED )
		prnPrint(prnHandle, "net econnrefused! "); 
	else if ( stat == NET_ETIMEDOUT )
		prnPrint(prnHandle, "net etimedout "); 
	else prnPrint(prnHandle, "err others! ");

   if (stat != RET_OK )
   {	 
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "ERR : stat is: %02d", stat);
		prnPrint(prnHandle, buffer); 

        // Stop the interface
        netNiStop( ni );
   
        // Close the channel
	    netNiClose(ni);
	
    	prnPrint(prnHandle, "End of LNET test");
      
        /*** STANDARD HMI & PRINTER CLOSE ROUTINE ***/
   
        prnClose(prnHandle);

        hmiClose(displayHandle); 	

		return ;
   }
   
   //retrieve negociated local Ip address and dns addresses
   memset(&config,0,sizeof(config));
   config.ppp.fields        = NI_PPP_F_LOCALIPADDRESS|NI_PPP_F_PPPCFG;
   config.ppp.pppCfg.fields = NI_PPP_F_DNS1IPADDRESS|NI_PPP_F_DNS2IPADDRESS;

   if ( netNiConfigGet( ni, NET_NI_LEVEL_1, &config )==RET_OK )
 	  prnPrint(prnHandle, ">LNET config is ok! "); 
	else
 	  prnPrint(prnHandle, ">LNET config is not ok! "); 

   prnLinefeed(prnHandle, 1);
  
  
	// Set DNS server addresses parameters
	dnsConfig.fields		= DNS_FIELD_DOMAINNAME | DNS_FIELD_DNSSERVER1 | DNS_FIELD_DNSSERVER2;
	dnsConfig.domainName	= "defaultDomain";  //"websfr";
	dnsConfig.dnsServer1 = config.ppp.pppCfg.dns1IpAddress;
	dnsConfig.dnsServer2 = config.ppp.pppCfg.dns2IpAddress;

	inAddr.s_addr = config.ppp.pppCfg.dns1IpAddress;
	inet_ntoa(inAddr, buffer);
	prnPrint(prnHandle, buffer);
	
	inAddr.s_addr = config.ppp.pppCfg.dns2IpAddress;
	inet_ntoa(inAddr, buffer);
	prnPrint(prnHandle, buffer);

	retCode = dnsConfigSet(DNS_LEVEL_1, &dnsConfig);
	
	if(retCode != RET_OK)
	{
		prnPrint(prnHandle, "Set DNS failed");	
		prnLinefeed(prnHandle, 1);
	}

	hmiADClearLine (displayHandle, HMI_ALL_LINES);

	if ((i=tcpClient("202.54.135.139", "1705")) < 0) // APAGAR!!!!
	{
	  prnPrint(prnHandle, ">open socket not ok! ");
	  hmiADDisplayText(displayHandle, 1, 0, "OPEN SOCKET FAIL");
	}
	else
	{
	  prnPrint(prnHandle, ">open socket ok! ");
	  hmiADDisplayText(displayHandle, 1, 0, "OPEN SOCKET OK");
	}


    // Stop the interface
    netNiStop( ni );
   
    // Close the channel
	netNiClose(ni);
	
	prnPrint(prnHandle, "End of LNET test");

/*** STANDARD HMI & PRINTER CLOSE ROUTINE ***/
   
    prnClose(prnHandle);

    hmiClose(displayHandle); 	


	return;
}



// Testing socket functionality
static int16 tcpClient(char *szEnderecoIP, char *szPorta)
{
	
	int16 errorCode;
	int i, maxSegSize;
    int16 rcvDataLength;
    struct timeval rxTimeout;

   memset(&destAddr, 0, sizeof(destAddr));

	/* Specify struct sock address */
   destAddr.sin_family = AF_INET;
//   destAddr.sin_port = htons_f (atoi(szPorta));
   destAddr.sin_port = htons((uint16) atoi(szPorta));
   destAddr.sin_addr.s_addr = inet_addr(szEnderecoIP);

   for (i = 0; i < 8; i++)
   {
      destAddr.sin_zero[i] = 0;
   }


	/* Create a socket */
	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (hSocket == -1) 
	{
        errorCode = socketerror();
       // prnPrint(prnHandle,"Error %d", errorCode);
      //  displayMsg("socket:", "Failed!!!", msgbuf, NULL, 0x0100FFFF);
		prnPrint(prnHandle,"socket() error");
		return -1;
	}

    maxSegSize = 1450;
    
    /* The default TCP_MAXSEG is 536 bytes, try at 1450 bytes */
    if (setsockopt(hSocket, SOL_TCP, TCP_MAXSEG, (void *)&maxSegSize, sizeof(maxSegSize)) == 0)
    {
        prnPrint(prnHandle, "setsockopt(TCP_MAXSEG) = OK");
    }    
    else
    {
        prnPrint(prnHandle, "setsockopt(TCP_MAXSEG) = Fail");
    }    

    rxTimeout.tv_sec = 10;
    rxTimeout.tv_usec = 0;
    
    /* The default SO_RCVTIMEO is 60 seconds, try at 10 seconds*/
    if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (void *)&rxTimeout, sizeof(rxTimeout)) == 0)
    {
        prnPrint(prnHandle, "setsockopt(SO_SNDTIMEO) = OK");
    }    
    else
    {
        prnPrint(prnHandle, "setsockopt(SO_SNDTIMEO) = Fail");
    }    



    rxTimeout.tv_sec = 10;
    rxTimeout.tv_usec = 0;
    
    /* The default SO_RCVTIMEO is 60 seconds, try at 10 seconds*/
    if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (void *)&rxTimeout, sizeof(rxTimeout)) == 0)
    {
        prnPrint(prnHandle, "setsockopt(SO_RCVTIMEO) = OK");
    }    
    else
    {
        prnPrint(prnHandle, "setsockopt(SO_RCVTIMEO) = Fail");
    }    

	/* Connect to the server */
	errorCode = connect(hSocket, (struct sockaddr *)&destAddr, sizeof(destAddr));

	//inet_ntoa(destAddr.sin_addr.s_addr, buffer);
	prnPrint(prnHandle, "Destination Address:");
	prnPrint(prnHandle, buffer);


	if (errorCode == -1) 
	{
        //errorCode = socketerror();
       // prnPrint(prnHandle,"Error %d", errorCode);
      //  displayMsg("socket:", "Failed!!!", msgbuf, NULL, 0x0100FFFF);
		prnPrint(prnHandle,"connect() socket error");
		return -1;
	}

    errorCode = write(hSocket, "HELLO", 5);

	if (errorCode == -1)
	{
        //errorCode = socketerror();
       // prnPrint(prnHandle,"Error %d", errorCode);
      //  displayMsg("socket:", "Failed!!!", msgbuf, NULL, 0x0100FFFF);
		prnPrint(prnHandle,"write() socket error");
		return -1;
	}
    else
    {
       prnPrint(prnHandle, "TX : HELLO");
    }



    rcvDataLength = sizeof(rcvData);
    memset(rcvData, 0, sizeof(rcvData));

    prnPrint(prnHandle, "read start");
	errorCode = read(hSocket, (void *)&rcvData[0], rcvDataLength);

    prnPrint(prnHandle, "read done");

	if (errorCode == -1)
	{
		prnPrint(prnHandle,"read() socket error");
		return -1;
	}
    else
    {
       // format the response
       sprintf(buffer, "RX [%d]: %s",errorCode, &rcvData[0]);
    
       prnPrint(prnHandle, buffer);
    }

    /* Close the socket */
    close(hSocket);

	return(0);  // finish OK
}


