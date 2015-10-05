#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcab0087.

static void fillBuf(byte idx, tBuffer * buf) {
    switch (idx) {
      case 1:                  //Sampath
          bufApp(buf, (byte *) "\x60\x00\x15\x00\x00", 5);  // TPDU
          bufApp(buf, (byte *) "\x02\x00", 2);  // Message Type
          bufApp(buf, (byte *) "\x30\x20\x05\x80\x20\xC0\x10\x04", 8);  // Bitmap (3,4)(11)(22,24)(25)(35)(41,42)(52)(62)
          bufApp(buf, (byte *) "\x00\x10\x00", 3);  // 3:Procs Code
          bufApp(buf, (byte *) "\x00\x00\x00\x00\x01\x00", 6);  // 4:Amount
          bufApp(buf, (byte *) "\x00\x00\x02", 3);  //11:STAN
          bufApp(buf, (byte *) "\x00\x21", 2);  //22:Point-of-Service Entry Mode
          bufApp(buf, (byte *) "\x00\x15", 2);  //24:Network International Identifier
          bufApp(buf, (byte *) "\x00", 1);  //25: Point-of-Service Condition Code
          bufApp(buf, (byte *) "\x32\x50\x45\x11\x01\x38\x00\x02\x43\x90\x6d\x49\x12\x10\x10\x00\x00", 17); //35:Track 2 Data
          //bufApp(buf,(byte *)"\x36\x36\x31\x33\x30\x37\x34\x38",8); //41: Card Acceptor Terminal Identification
          //bufApp(buf,(byte *)"66130748",8); //41: Card Acceptor Terminal Identification
          bufApp(buf, (byte *) "66130006", 8);  //41: Card Acceptor Terminal Identification
          //bufApp(buf,(byte *)"\x30\x30\x30\x30\x30\x30\x30\x30\x33\x31\x38\x31\x30\x39\x36",15); //42:Card Acceptor Identification Code
          //bufApp(buf,(byte *)"000000003181096",15); //42:Card Acceptor Identification Code
          bufApp(buf, (byte *) "000000003089992", 15);  //42:Card Acceptor Identification Code
          bufApp(buf, (byte *) "\x9a\xc1\xbe\x9d\xd2\x6a\xa5\xa8", 8);  //52: Cardholder PIN Block
          bufApp(buf, (byte *) "\x00\x06\x30\x30\x03", 5);  //62:Private use
          break;
      case 2:                  //BORICA
          bufApp(buf, (byte *)
                 "\x60\x00\x02\x00\x00\x02\x00\x70\x3C\x25\x80\x20\xC0\x80\x04\x16\x49\x05\x15\x00\x00\x03\x00\x32\x00\x00\x00\x00\x00\x00\x00\x14\x01\x00\x00\x01\x09\x03\x39\x06\x06\x06\x12\x01\x00\x00\x22\x00\x02\x00\x37\x49\x05\x15\x00\x00\x03\x00\x32\xD0\x61\x21\x01\x00\x00\x03\x82\x00\x00\x0F\x39\x33\x39\x34\x34\x34\x34\x31\x39\x39\x39\x33\x39\x30\x30\x30\x30\x31\x30\x30\x30\x30\x30\x09\x75\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                 129);
          break;
      case 3:                  //TranzWare
          bufApp(buf, (byte *) "\x60\x00\x01\x80\x00", 5);  // TPDU
          bufApp(buf, (byte *) "\x02\x00", 2);  // Message Type
          bufApp(buf, (byte *) "\x30\x20\x05\x80\x20\xC0\x00\x04", 8);  // Bitmap (3,4)(11)(22,24)(25)(35)(41,42)(62)
          bufApp(buf, (byte *) "\x00\x00\x00", 3);  // 3:Processing Code
          bufApp(buf, (byte *) "\x00\x00\x00\x00\x01\x00", 6);  // 4:Amount
          bufApp(buf, (byte *) "\x00\x00\x01", 3);  //11:STAN
          bufApp(buf, (byte *) "\x00\x10", 2);  //22:Point-of-Service Entry Mode
          bufApp(buf, (byte *) "\x00\x01", 2);  //24:Network International Identifier
          bufApp(buf, (byte *) "\x00", 1);  //25: Point-of-Service Condition Code
          bufApp(buf, (byte *) "\x34\x70\x01\x00\x00\x11\x00\x02\x03\x14\xd0\x80\x91\x26\x19\x12\x18\x69", 18); //35:Track 2 Data
          bufApp(buf, (byte *) "ITCTESP1", 8);  //41: Card Acceptor Terminal Identification
          bufApp(buf, (byte *) "16             ", 15);  //42:Card Acceptor Identification Code
          bufApp(buf, (byte *) "\x00\x06\x30\x30\x30\x30\x30\x31", 8);  //62:Private use
          break;
      default:
          break;
    }
}

static char *getSrv(byte idx) {
    switch (idx) {
      case 1:                  //Sampath
          return "0-0094112303045";
      case 2:                  //BORICA
          return "0-0035929707638";
          break;
      case 3:                  //TRANZWARE +88 02 8957917, +88 02 8957918, +88 02 8957919 or +88 02 8960106. 
          //return "0-0088028957917";
          //return "0-0088028957918";
          return "0-0088028957919";
          break;
      default:
          break;
    }
    return 0;
}

void tcab0087(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];
    byte idx;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    idx = 3;
    fillBuf(idx, &buf);
    ptr = getSrv(idx);

    ret = dspLS(0, "comStartSet");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnHdlc);
    CHECK(ret >= 0, lblKO);
    ret = comSet("8N11200");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, ptr + 5);
    CHECK(ret >= 0, lblKO);
    ret = comDial(ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comSendBuf..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret == bufLen(&buf), lblKO);

    bufReset(&buf);
    ret = dspLS(2, "comRecvBuf...");
    CHECK(ret >= 0, lblKO);
    ret = comRecvBuf(&buf, 0, 30);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    ret = comHangStart();
    ret = comHangWait();
    ret = comStop();
    prtStop();
    dspStop();
}
