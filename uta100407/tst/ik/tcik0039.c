#include <string.h>
#include <unicapt.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

#ifdef __CHN_GPRS__
#include <gsm.h>
#endif

//Integration test case tcik0039
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

void tcik0039(void) {
    int ret;
    char *ptr;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();

    trcS("GPRS: Start\n");

//GPRS Connection

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnGprs);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comStart OK");
    ret = prtS("comStart OK");

    ptr = "0000|orange.fr|danfr|mq36noqt|10.10.194.103|21";
    ret = dspLS(0, "comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);
    //ret= prtS("comSet OK");
    //FTP Connection
    /* Create a new FTP client session */

    ret = dspLS(2, "ftpStart...");
    CHECK(ret >= 0, lblKO);
    ret = ftpStart();
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpStart OK");
    CHECK(ret >= 0, lblKO);
    //ret= prtS("ftpStart OK");

    ret = dspLS(1, "ftpDial..");
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
    ret = dspLS(1, "ftpDial OK");
    //CHECK(ret >= 0, lblKO);
    //ret= prtS("ftpDial OK");

    ret = dspLS(2, "ftpCD Incoming");
    ret = ftpCD("Incoming");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpGet lalala.txt");
    ret = ftpGet(16, "lalala.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpGet OK");
    //CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpPut ...");
    ret = ftpPut(16, "ftpPut.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpPut OK");
    //CHECK(ret >= 0, lblKO);
    /*
       ret = comStop();
       CHECK(ret >= 0, lblKO);
     */
    prtStop();
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(3, "KO!");
    goto lblEnd;
  lblEnd:
    tmrPause(3);
    trcS("GPRS: End\n");
    ret = StopPDP();
    prtStop();
    dspStop();
}

#endif
