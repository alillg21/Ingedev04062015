/** \file
 * Unitary test case tcab0054.
 * Loading MasterKey 0123456789ABDCDEF into loc 12
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0054.c $
 *
 * $Id: tcab0054.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0054(void) {
    int ret;
    byte loc;
    byte crt[4];
    char tmp[prtW + 1];
    byte key[16];
    byte wrk[8];

//    const byte *wrk= (byte *)"\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD"; //0123456789ABCDEF encrypted by 1111111111111111
    byte *mst;                  //plaintext master key

    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    mst = (byte *) "\x01\x23\x45\x67\x89\xAB\xCD\xEF";
    stdDES(wrk, mst, (byte *) "\x11\x11\x11\x11\x11\x11\x11\x11");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "cryStart...");
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "cryLoadTestKey...");
    CHECK(ret >= 0, lblKO);
    loc = 0x10;                 //array 1 slot 0
    ret = cryLoadTestKey(loc);  //load test transport key 1= 11..11 into location loc
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "cryVerify...");
    CHECK(ret >= 0, lblKO);
    ret = cryVerify(loc, crt);  //crt is the first four bytes of EDE(00..00,T0T1)
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = bin2hex(tmp, &loc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tmp, ": ");
    VERIFY(strlen(tmp) == 4);
    ret = bin2hex(tmp + 4, crt, 4);
    CHECK(ret == 8, lblKO);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(crt, "\x82\xE1\x36\x65", 4) == 0, lblKO);  //it is the certificate of the key 1111111111111111
    tmrPause(1);

    ret = dspLS(0, "cryLoadDKuDK...");
    CHECK(ret >= 0, lblKO);
    memcpy(key, wrk, 8);
    memcpy(key + 8, wrk, 8);    //EDE using duplicated key gives single DES    
    loc = 0x12;
    ret = cryLoadDKuDK(loc, 0x10, key, 'g');    //download master key into loc using transport key located at 0x10
    CHECK(ret >= 0, lblKO);
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    //present results
    ret = bin2hex(tmp, &loc, 1);
    CHECK(ret == 2, lblKO);
    strcat(tmp, ": ");
    VERIFY(strlen(tmp) == 4);
    ret = bin2hex(tmp + 4, crt, 4);
    CHECK(ret == 8, lblKO);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, tmp);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(crt, "\xD5\xD4\x4F\xF7", 4) == 0, lblKO);  //it is the certificate of the key 0123456789ABCDEF

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    cryStop('m');
    prtStop();
    dspStop();
}
