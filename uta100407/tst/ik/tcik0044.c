#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0044.
//Functions testing with GPRS connection chn TCP plus FTP
//plural connections

void tcik0044(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnGprs);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet...PDP1");
    CHECK(ret >= 0, lblKO);

    ptr = "000|orange.fr|danfr|mq36noqt|";
    //ptr= "5670|corpmts064|mts|064|";
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "DialIP ...");
    ret = comDial("212.27.42.12|110");  //free.fr
    //ret = comDial("82.247.161.69|6789"); //topup France
    CHECK(ret >= 0, lblKO);
    bufApp(&buf,
           (byte *) "\x00\x00\x00\x0A\x01\x01\x04\x00\x85\x05\x03\33\x00\x01",
           14);
    ret = dspLS(0, "utaGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "utaGprsSend..OK");
    bufReset(&buf);
    ret = comRecvBuf(&buf, 0, 100);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "Receiving done");
    CHECK(ret >= 0, lblKO);
    ret = comHangStart();
    trcFN("comHangStart : ret=%d\n", ret);
    ret = comHangWait();
    trcFN("comHangWait : ret=%d\n", ret);
    ret = dspLS(3, "IP connexion stoped");
    CHECK(ret >= 0, lblKO);

    //FTP connection
    ret = dspLS(1, "ftpStart...");
    CHECK(ret >= 0, lblKO);

    ret = ftpStart();
    CHECK(ret > 0, lblKO);

    ret = dspLS(1, "ftpStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpDial..");
    CHECK(ret >= 0, lblKO);

    //ret= ftpDial("192.168.53.50", "blacklist", "download",0);
    //ret = ftpDial("ftp.fr.ingenico.com", "soner", "ve2mi1xo6",0);
    //ret = ftpDial("172.19.49.10", "FPLUS\\ing_test", "Cmtch2006",0);//for info "\" must be "\\"
    ret = ftpDial("82.247.161.69", "ingenico", "fedbo9d7", 0);
    //ret = ftpDial("81.80.165.104", "soner", "ve2mi1xo6",0);
    CHECK(ret > 0, lblKO);

    ret = dspLS(2, "ftpDial OK");
    CHECK(ret >= 0, lblKO);

    ret = ftpCD("Incoming");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpCD Incoming");
    CHECK(ret >= 0, lblKO);

    ret = ftpGet(16, "lalala.txt");
    CHECK(ret > 0, lblKO);

    ret = dspLS(2, "ftpGet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpPut ...");
    CHECK(ret >= 0, lblKO);
    ret = ftpPut(16, "ftpPut.txt");
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ftpPut OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "DialIP ...");
    ret = comDial("82.247.161.69|6789");    //topup France
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "ftpDial OK");
    CHECK(ret >= 0, lblKO);

    bufApp(&buf,
           (byte *) "\x00\x00\x00\x0A\x01\x01\x04\x00\x85\x05\x03\33\x00\x01",
           14);
    ret = dspLS(0, "utaGprsSend..2");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "utaGprsSend..2OK");
    bufReset(&buf);
    ret = comRecvBuf(&buf, 0, 100);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "Receiving done2");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    ftpStop();
    tmrPause(3);
    comStop();
    prtStop();
    dspStop();
}
