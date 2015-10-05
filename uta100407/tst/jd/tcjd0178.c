#include <string.h>
#include "sys.h"
#include "tc.h"

static code const char *srcFile= __FILE__;

//Unitary test case tcjd0178.
//Testing gclSck and FTP

int tcjd0178(int pI1, int pI2, char *pS1, char *pS2)
{
    int ret;
    byte tmp[256];
    const char *ptr;

    ret= tcDsp(" utaGclStart");
	ret= utaGclStart(gclSck,""); CHECK(ret>=0,lblKO);

	tcDsp("ftpStart...");
    ret = ftpStart();
    CHECK(ret > 0, lblKO);
    tcDspPrev("ftpStart OK");
    
	tcDsp("ftpDial..");
    ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6");
    CHECK(ret > 0, lblKO);
    tcDspPrev("ftpDial OK");
	
    ret = ftpCD("Incoming");
    tcDsp("ftpCD Incoming");

    ret = ftpCD("utatest");
    tcDsp("ftpCD utatest");

    nvmRemove(0x10);
    ret = ftpGet(0x10, "test.txt");
    CHECK(ret > 0, lblKO);
    tcDsp("ftpGet OK");

    ret = getDateTime(tmp); CHECK(ret >= 0, lblKO);
	strcat(tmp,": tcjd0178");
	ret= nvmSave(0x10,tmp,0,(word)strlen(tmp)); CHECK(ret >= 0, lblKO);

    ret = ftpPut(0x10, "ftpPut.txt");
    CHECK(ret > 0, lblKO);
    tcDsp("ftpPut OK");

    goto lblEnd;
	
lblKO:
    trcErr(ret);
    ret= utaGclDialErr();
	
	switch(ret)
	{
		case gclErrCancel: ptr= " User cancelled"; break;
		case gclErrDial: ptr= " Dial error"; break;
		case gclErrBusy: ptr= " BUSY"; break;
		case gclErrNoDialTone: ptr= " NO DIAL TONE"; break;
		case gclErrNoCarrier: ptr= " NO CARRIER"; break;
		case gclErrNoAnswer: ptr= " NO ANSWER"; break;
		case gclErrLogin: ptr= " LOGIN PBM"; break;
		default: ptr= " KO!"; break;
	}
	  
    tcDsp(ptr);
	
lblEnd:
    utaGclStop();
}

