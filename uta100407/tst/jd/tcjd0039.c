#include <string.h>
#include <unicapt.h>
#include "log.h"
#include "tc.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

#ifdef __CHN_GPRS__
#include <gsm.h>
#endif

//Integration test case tcjd0039
//FTP connection via GPRS
//Application TOP5VRS.

#define CHK if(ret<0) return ret;

typedef struct sComChn {
    byte chn;                   //communications channel used
    byte chnRole;
    union {

#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs
            uint32 hdl;
            int16 sck;
        } gprs;
#endif
#ifdef __CHN_GSM__
        struct {                //chn= chnGsm
            uint32 hdl;
        } gsm;
#endif
    } prm;
} tComChn;
static tComChn com;

static int StopPDP(void) {
#ifdef __CHN_GPRS__
    int ret;

    trcS("StopPDP\n");
    VERIFY(com.chn == chnGprs);

    ret = dspClear();
    ret = dspLS(0, "Stop PDP Ctx...");

    if(com.prm.gprs.hdl) {
        ret = netNiStop(com.prm.gprs.hdl);
        CHECK(ret == RET_OK, lblKO);

        ret = netNiClose(com.prm.gprs.hdl);
        CHECK(ret == RET_OK, lblKO);

        com.prm.gprs.hdl = 0;
    }
    // Disconnect the PDP context to avoid GSM Data problem
    // Not required if you want to do several GPRS without GSM data
    gsmGprsSetPDPActivate(com.prm.gprs.hdl, 0, 255);
    ret = dspLS(0, "Stop PDP Ctx OK");
    //gsmClose(com.prm.gprs.hdl);

    return com.prm.gprs.hdl;
  lblKO:
    return -1;
#else
    return -1;
#endif
}

void tcjd0039(void) {
    int ret;
	char *ptr;

	trcS("GPRS: Start\n");

//GPRS Connection

	tcDsp("comStart...");
    ret = comStart(chnGprs);
    CHECK(ret >= 0, lblKO);
    tcDspPrev("comStart OK");
    tcPrt(NULL);

    ptr= "0000|orange.fr|danfr|mq36noqt|10.10.194.103|21";
    tcDsp("comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    tcDspPrev("comSet OK");
    //ret= prtS("comSet OK");
	//FTP Connection
    /* Create a new FTP client session */

    tcDsp("ftpStart...");
    ret = ftpStart();
    CHECK(ret > 0, lblKO);
    tcDspPrev("ftpStart OK");
    //ret= prtS("ftpStart OK");

    tcDsp("ftpDial..");
    //ftpDial("comtech.gotdns.org", "ingenico", "fedbo9d7");
    //ret= ftpDial("192.168.53.50", "blacklist", "download");
    ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6");
    /*
    if(ftpDial("comtech.gotdns.org", "ingenico", "fedbo9d7")<=0)
    	{
    		ret = dspLS(1, "other ftp conn..");
    		if(ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6")<=0)
    		goto lblKO;
    	}
    	*/
    CHECK(ret > 0, lblKO);
    tcDspPrev("ftpDial OK");
    //tcPrt(NULL);
	
	tcDsp("ftpCD Incoming");
    ret = ftpCD("Incoming");
    CHECK(ret >= 0, lblKO);
    
    tcDsp("ftpGet lalala.txt");
    ret = ftpGet(16, "lalala.txt");
    CHECK(ret > 0, lblKO);
    tcDspPrev("ftpGet OK");
	
	tcDsp("ftpPut ...");
    ret = ftpPut(16, "ftpPut.txt");
    CHECK(ret > 0, lblKO);
    tcDspPrev("ftpPut OK");
		/*
    ret = comStop();
    CHECK(ret >= 0, lblKO);
		*/
    goto lblEnd;

  lblKO:
  	trcErr(ret);
    tcDsp(" KO!");
    goto lblEnd;
	
  lblEnd:
    tcDsp("GPRS: End");
    ret = StopPDP();
}

#endif
