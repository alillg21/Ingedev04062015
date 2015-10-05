#include <string.h>
#include "sys.h"
#include "tst.h"

#include <LNet.h>
#include <LNetFtp.h>

static code const char *srcFile= __FILE__;

//Unitary test case tcrm0003.
//FTP testing:


// unitary test case for connection to hobex with ppp



void tcrm0004(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[128];

	// create new ftpHandle
	ftpHandle_t ftpHandle;

    bufInit(&buf,dat,128);
    




	//trcMode(1);
	// Start the Display
    ret = dspStart(); 
	ret = dspLS(0, "PPP Connection");	CHECK(ret>=0,lblKO);
	
	

    ret= dspLS(1,"comStart..."); CHECK(ret>=0,lblKO); 
	tmrPause(1);
//    TRCDT("comStart beg");
    TRCDT("comStart beg");
    ret= comStart(chnPpp); CHECK(ret>=0,lblKO);
    TRCDT("comStart end");
//   TRCDT("comStart end");
    ret= dspLS(1,"comStart...OK"); CHECK(ret>=0,lblKO); 
	tmrPause(1);

    ret= dspLS(1,"comSet..."); CHECK(ret>=0,lblKO); 
	tmrPause(1);
//    TRCDT("comSet beg");
    ret= comSet("0,02236225509-terminal-hobex-60"); CHECK(ret>=0,lblKO); //free.fr
//    TRCDT("comSet end");
    ret= dspLS(1,"comSet OK"); CHECK(ret>=0,lblKO); 
	tmrPause(1);

    goto lblEnd;
lblKO:
    //trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    tmrPause(2);
    comStop();
    dspStop();
}
