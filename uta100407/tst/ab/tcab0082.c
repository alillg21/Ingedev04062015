#include <string.h>
#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcab0082.
//Functions testing for GCL HDLC communication with SL
//  utaGclStart
//  utaGclDial
//  utaGclSendBuf
//  utaGclRecvBuf
//  utaGclStop

void tcab0086(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    bufInit(&buf, dat, 1024);
    bufReset(&buf);
    //bufApp(&buf,(byte *)"\x00\x5A",2); //length
    //bufApp(&buf,(byte *)"\x60\x00\x15\x00\x00",5); // TPDU
    //bufApp(&buf,(byte *)"\x02\x00",2); // Message Type
    //bufApp(&buf,(byte *)"\x30\x20\x05\x80\x20\xC0\x00\x04",8); // Bitmap
    //bufApp(&buf,(byte *)"\x00\x00\x00",3); // Procs Code
    //bufApp(&buf,(byte *)"\x00\x00\x00\x12\x39\x12",6); // Amount
    //bufApp(&buf,(byte *)"\x00\x00\x02\x00\x22\x00\x06\x00\x37\x54",10);
    //bufApp(&buf,(byte *)"\x37\x71\x36\x00\x00\x00\x20\xD0\x30\x41",10);
    //bufApp(&buf,(byte *)"\x01\x00\x00\x05\x49\x00\x00\x1F\x38\x30",10);
    //bufApp(&buf,(byte *)"\x30\x30\x30\x30\x30\x31\x32\x30\x30\x31",10);
    //bufApp(&buf,(byte *)"\x34\x39\x39\x30\x36\x30\x33\x34\x20\x20",10);
    //bufApp(&buf,(byte *)"\x20\x00\x12\x30\x30\x30\x30\x30\x32\x38",10);
    //bufApp(&buf,(byte *)"\x31\x39\x39\x39\x39\x00",6);

    bufApp(&buf, (byte *) "\x60\x00\x15\x00\x00", 5);   // TPDU
    bufApp(&buf, (byte *) "\x02\x00", 2);   // Message Type
    bufApp(&buf, (byte *) "\x30\x20\x05\x80\x20\xC0\x10\x04", 8);   // Bitmap
    bufApp(&buf, (byte *) "\x00\x10\x00", 3);   // Procs Code
    bufApp(&buf, (byte *) "\x00\x00\x00\x00\x10\x00", 6);   // Amount
    bufApp(&buf, (byte *) "\x00\x00\x02\x00\x21\x00\x15\x00\x32\x50", 10);
    bufApp(&buf, (byte *) "\x45\x11\x01\x38\x00\x02\x43\x90\x6d\x49", 10);
    bufApp(&buf, (byte *) "\x12\x10\x10\x00\x00\x36\x36\x31\x33\x30", 10);
    bufApp(&buf, (byte *) "\x37\x34\x38\x30\x30\x30\x30\x30\x30\x30", 10);
    bufApp(&buf, (byte *) "\x30\x33\x31\x38\x31\x30\x39\x36\x9a\xc1", 10);
    bufApp(&buf, (byte *) "\xbe\x9d\xd2\x6a\xa5\xa8\x00\x06\x30\x30", 10);
    bufApp(&buf, (byte *) "\x03", 1);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ptr = "8N11200|0-00-94112303045";
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclHdlc, ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclHdlcDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "utaGclHdlcDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclHdlcSend..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret == bufLen(&buf), lblKO);

    ret = dspLS(0, "utaGclHdlcRecv..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclRecvBuf(&buf, 0, 100);

    ret = dspLS(0, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "utaGclHdlcStop");
    CHECK(ret >= 0, lblKO);
    ret = utaGclStop();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    utaGclStop();
    prtStop();
    dspStop();
}
