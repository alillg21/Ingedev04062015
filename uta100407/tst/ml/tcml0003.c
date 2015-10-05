#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcml0003.
//Testing ETH connection with free.fr using GCL

void tcml0003(void){
    int ret;
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

    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= prtStart(); CHECK(ret>=0,lblKO);    
    
        //trcMode(1);

        ptr= "10.10.59.68|2000|";

    ret= dspLS(0,"utaGclStart"); CHECK(ret>=0,lblKO); 
    ret= dspLS(1,ptr); CHECK(ret>=0,lblKO); 
    ret= utaGclStart(gclEth,ptr); CHECK(ret>=0,lblKO);

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

        ret= dspLS(2,"utaGclDial..."); CHECK(ret>=0,lblKO); 
    ret= utaGclDial(); CHECK(ret>=0,lblKO);
    ret= dspLS(2,"utaGclDial OK"); CHECK(ret>=0,lblKO); 

        ret= dspLS(0,"utaGclSend.."); CHECK(ret>=0,lblKO);
        ret= utaGclSendBuf(bufPtr(&buf),bufLen(&buf)); CHECK(ret==bufLen(&buf),lblKO);
        
    ret= dspLS(0,"utaGclRecv.."); CHECK(ret>=0,lblKO); 
        ret= utaGclRecvBuf(&buf,0,100); 

    ret= dspLS(0,"Receiving done"); CHECK(ret>=0,lblKO);

    ret= dspLS(1,"utaGclStop"); CHECK(ret>=0,lblKO); 
    ret= utaGclStop(); CHECK(ret>=0,lblKO); 

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
