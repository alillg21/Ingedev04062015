
#include <string.h>
#include "sys.h"
#include "tst.h"
#include "gcl.h"

// Unitary test case tcfa0005.
// Functions testing of TCPIP connection with no DHCP:
// TCPIP connection via GCL with static local IP
// parameterized Local IP, Gateway, Subnet Mask, DHCP, Timeout and Solinger
// Removing the ETH cable before sending.
// Testing gclEthSendCableChk() which is for checking the ETH cable is connected to the terminal or not.
static int makeCall(void) {
	
    int ret;
    int sav;
	const char *srv;
	
	char *ptr;
	tBuffer buf;
	byte dat[1024];

	bufInit(&buf,dat,1024);
	bufReset(&buf);
	bufApp(&buf,(byte *)"\x00\x5A",2); //length
	bufApp(&buf,(byte *)"\x60\x00\x01\x00\x00",5); // TPDU
	bufApp(&buf,(byte *)"\x02\x00",2); // Message Type
	bufApp(&buf,(byte *)"\x30\x20\x05\x80\x20\xC0\x00\x04",8); // Bitmap
	bufApp(&buf,(byte *)"\x00\x00\x00",3); // Procs Code
	bufApp(&buf,(byte *)"\x00\x00\x00\x12\x39\x12",6); // Amount
	bufApp(&buf,(byte *)"\x00\x00\x02\x00\x22\x00\x06\x00\x37\x54",10);
	bufApp(&buf,(byte *)"\x37\x71\x36\x00\x00\x00\x20\xD0\x30\x41",10);
	bufApp(&buf,(byte *)"\x01\x00\x00\x05\x49\x00\x00\x1F\x38\x30",10);
	bufApp(&buf,(byte *)"\x30\x30\x30\x30\x30\x31\x32\x30\x30\x31",10);
	bufApp(&buf,(byte *)"\x34\x39\x39\x30\x36\x30\x33\x34\x20\x20",10);
	bufApp(&buf,(byte *)"\x20\x00\x12\x30\x30\x30\x30\x30\x32\x38",10);
	bufApp(&buf,(byte *)"\x31\x39\x39\x39\x39\x00",6);

	
	//Remote IP Address - Remote Port- Mask - Gateway - Local IP Address
	//For Static Connection.
	srv= "192.1.1.253|1000|255.255.255.0|192.1.1.1|192.1.1.166";
	//Remote IP Address - Remote Port
	//For Dynamic Connection.
	//srv= "192.1.1.253|1000";
	
	ret = dspLS(0, "utaGclStart...");
	CHECK(ret >= 0, lblKO);
	
	ret= utaGclStart(gclEth,srv);
	CHECK(ret >= 0, lblKO);

	ret = utaGclDial();
	CHECK(ret >= 0, lblKO); 
	
	sav= 0;
	do{		
		ret= utaGclDialInfo();
		if(sav==ret) continue;
		sav= ret;
		switch(sav){
			case gclStaPreDial:		ptr= "PreDial"; break;
			case gclStaDial:		ptr= "Dial"; break;
			case gclStaConnect:		ptr= "Connect"; break;
			case gclStaLogin:		ptr= "Login"; break; 
			case gclStaFinished:	ptr= "Dial Finished"; break;
				break;
			default:
				ptr= "FAILED";
				CHECK(ret >= 0, lblKO);
		}
		dspLS(0,ptr);
	}while(sav!=gclStaFinished);  
	
	ret = dspLS(0, "CONNECTED");
	CHECK(ret >= 0, lblKO);
	
	tmrStart(1, 1500);

	ret = kbdStart(1); CHECK(ret>=0,lblKO);	
	dspClear();
	
	ret= dspLS(1, "     PLEASE");
	ret= dspLS(2, "REMOVE ETH CABLE");
	ret= dspLS(3, " THEN PRESS OK");	
	
	while(tmrGet(1))
	{
        ret= kbdKey(); //retrieve a key if pressed
        if(ret == kbdVAL) break;
    }  	
	
	tmrStop(1);
	dspClear();	
	ret= dspLS(1,"  utaGclSend.."); CHECK(ret>=0,lblKO);
	tmrPause(3);
	ret= utaGclSendBuf(bufPtr(&buf),bufLen(&buf)); //CHECK(ret==bufLen(&buf),lblKO);	
	
	if(ret == -gclErrCableRemoved)
	{
		dspClear();	
		dspLS(1, " ETHERNET CABLE ");
		dspLS(2, "    REMOVED     ");
		tmrPause(3);
	}

    ret = 1;
    goto lblEnd;

lblKO:
	dspClear();	
	ret = -1;
lblEnd:
	kbdStop();
    tmrStop(1);
	utaGclStop();
    return ret;
}


void tcfa0005(void) {
    int ret;
        
    ret= makeCall(); 
    CHECK(ret >= 0, lblKO);
  
    tmrPause(1);
    goto lblEnd;
    
lblKO:
	trcErr(ret);
	dspLS(0, "KO!");
lblEnd:
	tmrPause(3);
	dspStop();
}

