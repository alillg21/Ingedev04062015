#include <string.h>
#include "sys.h"
#include "tst.h"

//Functions testing for GCL HDLC communication:
//  utaGclStart
//  utaGclDial
//  utaGclSendBuf
//  utaGclRecvBuf
//  utaGclStop

static void fillBuf(byte idx, tBuffer *buf){
	switch(idx){
		case 1: //Sampath
			bufApp(buf,(byte *)"\x60\x00\x15\x00\x00",5); // TPDU
			bufApp(buf,(byte *)"\x02\x00",2); // Message Type
			bufApp(buf,(byte *)"\x30\x20\x05\x80\x20\xC0\x10\x04",8); // Bitmap
			bufApp(buf,(byte *)"\x00\x10\x00",3); // Procs Code
			bufApp(buf,(byte *)"\x00\x00\x00\x00\x01\x00",6); // Amount
			bufApp(buf,(byte *)"\x00\x00\x02\x00\x21\x00\x15\x00\x32\x50",10);
			bufApp(buf,(byte *)"\x45\x11\x01\x38\x00\x02\x43\x90\x6d\x49",10);
			bufApp(buf,(byte *)"\x12\x10\x10\x00\x00\x36\x36\x31\x33\x30",10);
			bufApp(buf,(byte *)"\x37\x34\x38\x30\x30\x30\x30\x30\x30\x30",10);
			bufApp(buf,(byte *)"\x30\x33\x31\x38\x31\x30\x39\x36\x9a\xc1",10);
			bufApp(buf,(byte *)"\xbe\x9d\xd2\x6a\xa5\xa8\x00\x06\x30\x30",10);
			bufApp(buf,(byte *)"\x03",1);
		break;
		case 2: //BORICA
			bufApp(buf,(byte *)"\x60\x00\x02\x00\x00\x02\x00\x70\x3C\x25\x80\x20\xC0\x80\x04\x16\x49\x05\x15\x00\x00\x03\x00\x32\x00\x00\x00\x00\x00\x00\x00\x14\x01\x00\x00\x01\x09\x03\x39\x06\x06\x06\x12\x01\x00\x00\x22\x00\x02\x00\x37\x49\x05\x15\x00\x00\x03\x00\x32\xD0\x61\x21\x01\x00\x00\x03\x82\x00\x00\x0F\x39\x33\x39\x34\x34\x34\x34\x31\x39\x39\x39\x33\x39\x30\x30\x30\x30\x31\x30\x30\x30\x30\x30\x09\x75\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",129);
		break;
		default:
			break;
	}
}

static char *getSrv(byte idx){
	switch(idx){
		case 1: //Sampath
			return "8N11200|0-0094112303045";
		case 2: //BORICA
			return "8N11200|0-0035929707638";
		break;
		default:
			break;
	}
	return 0;
}

void tcml0000(void){   
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];
    byte idx;

    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= prtStart(); CHECK(ret>=0,lblKO);    

    bufInit(&buf,dat,1024);
    bufReset(&buf);

	idx= 2;
	fillBuf(idx,&buf);
	ptr= getSrv(idx);

    ret= dspLS(0,"utaGclStart"); CHECK(ret>=0,lblKO); 
    ret= dspLS(1,ptr); CHECK(ret>=0,lblKO); 
    ret= utaGclStart(gclHdlc,ptr); CHECK(ret>=0,lblKO);
    
    ret= dspLS(2,"utaGclHdlcDial..."); CHECK(ret>=0,lblKO); 
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


    ret= dspLS(2,"utaGclHdlcDial OK"); CHECK(ret>=0,lblKO); 


	ret= dspLS(0,"utaGclHdlcSend.."); CHECK(ret>=0,lblKO);
	ret= utaGclSendBuf(bufPtr(&buf),bufLen(&buf)); CHECK(ret==bufLen(&buf),lblKO);
	
    ret= dspLS(0,"utaGclHdlcRecv.."); CHECK(ret>=0,lblKO); 
	ret= utaGclRecvBuf(&buf,0,100);	

    ret= dspLS(0,"Receiving done"); CHECK(ret>=0,lblKO);

    ret= dspLS(1,"utaGclHdlcStop"); CHECK(ret>=0,lblKO); 
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
