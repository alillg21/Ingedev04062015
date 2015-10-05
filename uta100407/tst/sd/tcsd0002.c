#include <string.h>
#include "sys.h"
#include "tst.h"

static code const char *srcFile= __FILE__;

//Unitary test case tcab0078.
//Functions testing for GCL Internal modem communication:
//  utaGclStart
//  utaGclDial
//  utaGclRecv
//  utaGclStop

void tcsd0002(void){
    int ret,sav;
    //int idx;
//    byte b;
    char *ptr;
    char msg[dspW+1];
//	tBuffer buf;
//	byte dat[256];
	byte trm[]= { ('C'|0x80),0};

	
	// ret= dspStart(); CHECK(ret>=0,lblKO);    
 //   ret= prtStart(); CHECK(ret>=0,lblKO);    
	dspClear();
    memset(msg,0,dspW+1);

	ptr= "8N12400|80-02224924991";
	//ptr= "7E11200|18280";
//	ptr= "7E11200|0-0836062424";
    ret= dspLS(0,"utaGclStart"); CHECK(ret>=0,lblKO); 
    ret= dspLS(1,ptr); CHECK(ret>=0,lblKO); 
    ret= utaGclStart(gclHdlc,ptr); CHECK(ret>=0,lblKO);

    ret= dspLS(2,"utaGclDial..."); CHECK(ret>=0,lblKO); 
    ret= utaGclDial(); CHECK(ret>=0,lblKO);

	
	sav= 0;
	
	kbdStart(1);
	do{
		ret= utaGclDialInfo();
		CHECK(ret>=0,lblKO);
		
		ret= kbdKey();
		if(ret){
			kbdStop();
			kbdStart(1);
		}
		if(kbdANN == ret ){
			goto lblKO;        
		}
		
		if(sav==ret) continue;
		sav= ret;
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
	
    ret= dspLS(2,"utaGclDial OK"); CHECK(ret>=0,lblKO);

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
	kbdStop();
    tmrPause(3);
	utaGclStop();
    prtStop();
    dspStop();
}
