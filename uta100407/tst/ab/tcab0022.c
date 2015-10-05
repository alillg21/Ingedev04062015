/** \file
 * Unitary test case tcab0022.
 * Functions testing:
 * \sa
 *  - cryLoadDKuDK()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0022.c $
 *
 * $Id: tcab0022.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

void tcab0022(void) {
    int ret;
    byte crt[4];
    char tmp[prtW + 1];
    byte key[16];
    byte loc;
    byte len;

    //WK, working key is 6B218F24DE7DC66C
    //TK, transport key is the test key 1111111111111111
    //WC, working key encrypted by transport key is 4225EA8E9A0B5F63
    const byte *wrk = (byte *) "\x42\x25\xEA\x8E\x9A\x0B\x5F\x63";

#ifdef __ICT220__
    len = 3;
#else
    len = 4;
#endif

    trcS("tcab0022 Beg\n");
    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    tcab0021();                 //test key downloading

    //open resources
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    //verify that the transport key 1111111111111111 is downloaded at offset 1008
#ifdef __TELIUM__
    loc = 1008 / 8;             //1024-16
#endif
#ifdef __UNICAPT__
    loc = 0x10;
#endif
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\x82\xE1\x36\x65", len) == 0, lblKO);    //it is the certificate of the key 1111111111111111

    tmrPause(1);

    //start testing
    ret = dspLS(0, "cryLoadDKuDK...");
    CHECK(ret >= 0, lblKO);

    memcpy(key, wrk, 8);
    memcpy(key + 8, wrk, 8);    //EDE using duplicated key gives single DES

#ifdef __TELIUM__
    loc = 8 / 8;                //FOR ISO9564 PIN block calculation PINKey location must be at offset =8;
    ret = cryLoadDKuDK(loc, (1008 / 8), key, 'p');  //download pin key into loc using transport key located at 1008
    CHECK(ret >= 0, lblKO);
#endif
#ifdef __UNICAPT__
    loc = 0x24;
    ret = cryLoadDKuDK(loc, 0x10, key, 'p');    //download pin key into loc using transport key located at 0x10
#endif

    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    sprintf(tmp, "%04X: ", loc);
    bin2hex(tmp + strlen(tmp), crt, 4);

    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, tmp);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\xF7\x53\xC2\x98", len) == 0, lblKO);    //it is the certificate of the key 6B218F24DE7DC66C

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
    trcS("tcab0022 End\n");
}
