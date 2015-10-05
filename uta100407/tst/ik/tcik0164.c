/// \file tcik0164.c

#include <string.h>
#include "sys.h"

/** UTA test case for integration of aprcry module.
 * Test Key download
 * Functions testing:
 *  cryStart
 *  cryStop
 *  cryLoadTestKey
 *  cryVerify
 */
#ifdef __APR__
#include <pinpad.h>
#include <Sec.h>
#include <periphPrinter.h>
void tcik0164(void) {
    int ret;
    byte crt[4];

    //char buf[3];
    byte key[8];
    byte loc;

    pp_display_clear(0);
    pp_display_text(0, 1, "tcik0164", PT_ALIGN_LEFT);
    pp_display_update();
    pp_get_key(1);

    xx_print("tcik0164");

    pp_display_clear(1);
    pp_display_text(1, 1, "cryStart...", PT_ALIGN_LEFT);
    pp_get_key(1);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    pp_display_clear(2);
    pp_display_text(2, 1, "cryLoadKey...", PT_ALIGN_LEFT);
    pp_get_key(1);

    //strcpy(buf, SEC_KCA); //old implementation
    //loc = buf[2];
    loc = 0x10;                 //same as tcab0021
    ret = cryLoadTestKey(loc);
    CHECK(ret >= 0, lblKO);

    pp_display_clear(3);
    pp_display_text(3, 1, "cryVerify...", PT_ALIGN_LEFT);
    pp_get_key(1);

    memset(crt, 0, sizeof(crt));
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    // Compare with the expected certificate
    if(memcmp(crt, "\x82\xE1\x36", 3))
        goto lblKO;
    xx_print("key verify OK");

    pp_display_clear(2);
    pp_display_text(2, 1, "cryLoadKeyTrans", PT_ALIGN_LEFT);
    pp_get_key(1);

    loc = 0x24;
    memcpy(key, "\xF4\x03\x79\xAB\x9E\x0E\xC5\x33", 8);
    ret = cryLoadDKuDK(loc, 0x10, key, 'g');    //TK, transport key is the same is test key 1111111111111111
    CHECK(ret >= 0, lblKO);

    pp_display_clear(3);
    pp_display_text(3, 1, "cryVerify...", PT_ALIGN_LEFT);
    pp_get_key(1);

    memset(crt, 0, sizeof(crt));
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    // Compare with the expected certificate
    if(memcmp(crt, "\x82\xE1\x36", 3))
        goto lblKO;
    xx_print("key verify OK");

    goto lblEnd;
  lblKO:
    xx_print("tcik0164 KO");
    goto lblEnd;
  lblEnd:
    cryStop('m');
    xx_print("tcik0164 End");

    return;
}
#else
void tcik0164(void) {
}
#endif
