#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcml0001.
//Testing PPP connection with free.fr

void tcml0001(void){
#ifdef __TEST__
    int ret;
    char *ptr;
        tBuffer buf;
    byte dat[1024];

        
        bufInit(&buf,dat,1024);
        bufReset(&buf);

    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= prtStart(); CHECK(ret>=0,lblKO);    
    
        trcMode(1);

        ptr= "0-0860922000|danfr|mq36noqt|212.27.42.12|110";

    ret= dspLS(0,"utaGclStart"); CHECK(ret>=0,lblKO); 
    ret= dspLS(1,ptr); CHECK(ret>=0,lblKO); 
    ret= utaGclStart(gclPpp,ptr); CHECK(ret>=0,lblKO);

        ret= dspLS(2,"utaGclPppDial..."); CHECK(ret>=0,lblKO); 
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

    ret= dspLS(2,"utaGclPppDial OK"); CHECK(ret>=0,lblKO); 
        
    ret= dspLS(0,"utaGclPppRecv.."); CHECK(ret>=0,lblKO); 
        ret= utaGclRecvBuf(&buf,0,100); 

    ret= dspLS(0,"Receiving done"); CHECK(ret>=0,lblKO);

    ret= dspLS(1,"utaGclPppStop"); CHECK(ret>=0,lblKO); 
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
#endif
}
