/** \file
 * Unitary test case tcab0061.
 * Functions testing:
 * \sa
 *  - cryMac()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0061.c $
 *
 * $Id: tcab0061.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

void tcab0061(void) {
    int ret;
    byte crt[4];
    char tmp[prtW + 1];
    byte key[16];
    byte loc, locTK;
    byte dat[256];
    tBuffer buf;
    byte mac[8];
    byte len;

    //WK, working key is 7968FBD9B5673145
    //TK, transport key is the test key 1111111111111111
    //WC, working key encrypted by transport key is 537F97B111487D71
    const byte *wrk = (byte *) "\x53\x7F\x97\xB1\x11\x48\x7D\x71";

#ifdef __ICT220__
    len = 3;
#else
    len = 4;
#endif

    trcS("tcab0061 Beg\n");
    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    tcab0021();                 //test key downloading

    //open ressources
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

#ifdef __TELIUM__
    //verify that the transport key 1111111111111111 is downloaded at loc = 1024; 
    loc = 1008 / 8;             //offset 1024 - length 16
#endif
#ifdef __UNICAPT__
    loc = 0x10;
#endif
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\x82\xE1\x36\x65", len) == 0, lblKO);    //it is the certificate of the key 1111111111111111

    //start testing
    ret = dspLS(0, "cryLoadDKuDK...");
    CHECK(ret >= 0, lblKO);

    memcpy(key, wrk, 8);
    memcpy(key + 8, wrk, 8);    //EDE using duplicated key gives single DES

#ifdef __TELIUM__
    loc = 24 / 8;
    locTK = 1008 / 8;
#endif
#ifdef __UNICAPT__
    loc = 0x26;
    locTK = 0x10;
#endif
    ret = cryLoadDKuDK(loc, locTK, key, 'm');   //download mac key into loc using transport key located at 0x10
    CHECK(ret >= 0, lblKO);

    crt[0] = 'm';
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    //present results
    sprintf(tmp, "%04X: ", loc);
    bin2hex(tmp + strlen(tmp), crt, 4);

    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, tmp);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    CHECK(memcmp(crt, "\x43\x08\x03\xF6", len) == 0, lblKO);    //it is the certificate of the key 7968FBD9B5673145

    //Prepare a string for SPDH MAC calculation
    bufInit(&buf, dat, 256);    //buffer initialisation
    bufReset(&buf);
    ret = bufApp(&buf, (byte *) "04", 0);
    CHECK(ret > 0, lblKO);      //transmission number
    ret = bufApp(&buf, (byte *) "TERMEMV2        ", 0);
    CHECK(ret > 0, lblKO);      //terminal ID
    ret = bufApp(&buf, (byte *) "00", 0);
    CHECK(ret > 0, lblKO);      //transaction code
    ret = bufApp(&buf, (byte *) "1111", 0);
    CHECK(ret > 0, lblKO);      //amount 1 (fid B)
    ret = bufApp(&buf, (byte *) ";4475209900514014=97091261310375600000?", 0);
    CHECK(ret > 0, lblKO);      //track 2 (fid q)
    while(bufLen(&buf) % 8) {
        ret = bufApp(&buf, (byte *) "\x00", 1);
        CHECK(ret > 0, lblKO);
    }

    ret = cryMac(&buf, loc, mac);   //and now test it
    CHECK(ret == 8, lblKO);

    //present results
    strcpy(tmp, "mac: ");
    VERIFY(strlen(tmp) == 5);
    ret = bin2hex(tmp + 5, mac, 4);
    CHECK(ret == 8, lblKO);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, tmp);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(mac, "\xD8\xEB\x98\x16", 4) == 0, lblKO);  //it is the MAC

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
    trcS("tcab0061 End\n");
}
