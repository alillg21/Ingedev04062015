/// \file tcik0165.c

#include <string.h>
#include "sys.h"
#include "tst.h"

/** UTA test case for integration of aprcry module.
 * Test Key download
 * Functions testing:
 * cryLoadDKuDK
 */
#ifdef __APR__
#include <pinpad.h>
#include <Sec.h>
#include <periphPrinter.h>

void tcik0165(void) {
    int ret;
    byte loc;
    byte crt[4];

    //char buf[3];
    byte key[8];

    pp_display_clear(0);
    pp_display_text(0, 1, "tcik0165", PT_ALIGN_LEFT);
    pp_display_update();
    pp_get_key(1);
    xx_print("tcik0165");

    pp_display_clear(1);
    pp_display_text(1, 1, "run tcik0164", PT_ALIGN_LEFT);
    pp_get_key(1);

    tcik0164();

    pp_display_clear(0);
    pp_display_text(0, 1, "tcik0165", PT_ALIGN_LEFT);
    pp_display_update();
    pp_get_key(1);

    pp_display_clear(1);
    pp_display_text(1, 1, "cryStart...", PT_ALIGN_LEFT);
    pp_get_key(1);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    pp_display_clear(2);
    pp_display_text(2, 1, "cryLoadDUKPT Key...", PT_ALIGN_LEFT);
    pp_get_key(1);

    //strcpy(buf, SEC_KPE); //old implementation
    //loc = buf[2];
    loc = 0x26;
    memcpy(key, "\x42\x25\xEA\x8E\x9A\x0B\x5F\x63", 8);
    ret = cryLoadDKuDK(loc, 0x24, key, 'p');
    CHECK(ret >= 0, lblKO);

    pp_display_clear(3);
    pp_display_text(3, 1, "cryVerify...", PT_ALIGN_LEFT);
    pp_get_key(1);

    memset(crt, 0, sizeof(crt));
    ret = cryVerify(loc, crt);
    CHECK(ret >= 0, lblKO);

    // Compare with the expected certificate
    if(memcmp(crt, "\xF7\x53\xC2", 3))
        goto lblKO;
    xx_print("key verify OK");

    goto lblEnd;
  lblKO:
    xx_print("tcik0165 KO");
    goto lblEnd;
  lblEnd:
    cryStop('m');
    xx_print("tcik0165 End");

    return;
}
#else
void tcik0165(void) {
}
#endif
