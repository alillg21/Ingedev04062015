#include <string.h>
#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcik0056.
//Functions testing:
// cryGetPin

void tcik0056(void) {
    int ret;
    byte crt[4];
    char tmp[prtW + 1];
    byte acc[8];
    byte blk[8];
    byte loc;
    byte key[16];

    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    const byte *wrk = (byte *) "\x42\x25\xEA\x8E\x9A\x0B\x5F\x63";

    //open ressources
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "cryLoadTestKey...");
    CHECK(ret >= 0, lblKO);

    loc = 0x10;                 //array 1 slot 0
    ret = cryLoadTestKey(loc);  //load test transport key 1= 11..11 into location loc
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(loc, crt);  //crt is the first four bytes of EDE(00..00,T0T1)
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\x82\xE1\x36\x65", 4) == 0, lblKO);  //it is the certificate of the key 1111111111111111

    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    //start testing
    ret = dspLS(0, "cryLoadDKuDK...");
    CHECK(ret >= 0, lblKO);

    memcpy(key, wrk, 8);
    memcpy(key + 8, wrk, 8);    //EDE using duplicated key gives single DES

    loc = 0x24;
    ret = cryLoadDKuDK(loc, 0x10, key, 'p');    //download pin key into loc using transport key located at 0x10
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\xF7\x53\xC2\x98", 4) == 0, lblKO);  //it is the certificate of the key 6B218F24DE7DC66C

    ret = dspLS(0, "crySetPpd...");
    CHECK(ret >= 0, lblKO);
    //start testing
    dspStop();                  //close channel to give cryptomodule access to HMI

    crySetPpd("01046*");
    memcpy(acc, "\x00\x00\x78\x90\x12\x34\x56\x74", 8);
    ret = cryGetPin("AMOUNT= 1000 USD\nEnter Pin:\n\n", acc, loc, blk); //enter 1234 to obtain a known result
    CHECK(ret > 0, lblKO);

    ret = dspStart();           //now we can open HMI again
    CHECK(ret >= 0, lblKO);

    //present results
    ret = bin2hex(tmp, blk, 8);
    CHECK(ret == 16, lblKO);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, tmp);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(blk, "\x06\xDA\x91\xDF\xF2\x76\xD3\xA8", 8) == 0, lblKO);  //it is the pinblock if the pin is 1234

    tmrPause(1);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    cryStop('m');
    prtStop();
    dspStop();                  //close resources
}
