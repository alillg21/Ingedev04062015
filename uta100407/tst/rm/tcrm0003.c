#include <string.h>
#include "sys.h"
#include "tst.h"

#include <LNet.h>
#include <LNetFtp.h>

//Unitary test case tcrm0003.
//FTP testing:

void tcrm0003(void) {

    int ret;

	// create new ftpHandle
	ftpHandle_t ftpHandle;

	// Start the Display
    ret= dspStart(); 
	CHECK(ret>=0,lblKO);

    ret= dspLS(0,"comStart..."); CHECK(ret>=0,lblKO); 
    ret= comStart(chnTcp5100); CHECK(ret>=0,lblKO);
    ret= comSet(""); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"comSet OK"); CHECK(ret>=0,lblKO); 

	/* Create a new FTP client session */
    ret= dspLS(1,"ftpOpen..."); CHECK(ret>=0,lblKO); 
	ret = ftpOpen( &ftpHandle );
	CHECK(ret==0,lblKO);
    ret= dspLS(0,"ftpOpen OK"); CHECK(ret>=0,lblKO); 

    ret= dspLS(2,"ftpConnect.."); CHECK(ret>=0,lblKO); 	
	// open ftp connection
	ret= ftpConnect( ftpHandle, "192.168.53.50", 
                            "blacklist", "download",
			    FTP_PASSIVE, 0);
	CHECK(ret==0,lblKO);
    ret= dspLS(2,"ftpConnect..OK"); CHECK(ret>=0,lblKO); 	

	// change directory
	ret = ftpChDir (ftpHandle, "58");
    ret= dspLS(2,"DIRECTORY CHANGED OK"); CHECK(ret>=0,lblKO); 

	// retrieve file
    ret = ftpRetrieve( ftpHandle, "file1", "delta.txt");
	CHECK(ret==0,lblKO);
    
	ret= dspLS(2,"FILE RETRIEVED"); CHECK(ret>=0,lblKO); 

    tmrPause(3);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
     /* Close FTP connection */
     ftpDisconnect(ftpHandle);

	  /* End FTP session */
	ftpClose(ftpHandle);
    tmrPause(2);
    dspStop();	
}

