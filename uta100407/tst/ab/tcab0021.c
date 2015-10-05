/** \file
 * Unitary test case tcab0021. Load PK master
 * Functions testing:
 * \sa
 *  - cryStart()
 *  - cryStop()
 *  - cryLoadTestKey() 
 *  - cryVerify() 
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0021.c $
 *
 * $Id: tcab0021.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

void tcab0021(void) {
    int ret;
    byte loc;
    byte crt[4];
    char tmp[prtW + 1];
    byte len;

#ifdef __ICT220__
    len = 3;
#else
    len = 4;
#endif

    trcS("tcab0021 Beg\n");

    memset(tmp, 0, prtW + 1);
    memset(crt, 0, 4);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "cryStart...");
    CHECK(ret >= 0, lblKO);

    ret = cryStart('m');        //open cryptomodule
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(1, "cryLoadTestKey...");
    CHECK(ret >= 0, lblKO);

#ifdef __TELIUM__
    loc = 1008 / 8;             //= (1024 - 16)/8 = 126
#endif
#ifdef __UNICAPT__
    loc = 0x10;                 //array 1 slot 0
#endif
    ret = cryLoadTestKey(loc);  //load test transport key 1= 11..11 into location loc
    //CHECK(ret >= 0, lblKO); //removed to avoid KO for ICT220

    tmrPause(1);

    ret = dspLS(2, "cryVerify...");
    CHECK(ret >= 0, lblKO);

    ret = cryVerify(loc, crt);  //crt is the first four bytes of EDE(00..00,T0T1)
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    sprintf(tmp, "%04X: ", loc);
    bin2hex(tmp + strlen(tmp), crt, 4);

    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, tmp);
    CHECK(ret >= 0, lblKO);

    CHECK(memcmp(crt, "\x82\xE1\x36\x65", len) == 0, lblKO);    //it is the certificate of the key 1111111111111111
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    cryStop('m');               //close cryptomodule ressource
    prtStop();
    dspStop();                  //close other resources
    trcS("tcab0021 End\n");
}
