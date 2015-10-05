/// \file tcik0167.c

#include <string.h>
#include "sys.h"
#include "tst.h"

/** UTA test case for integration of aprcry module.
 * Test Key download
 * Functions testing:
 * cryGetPin
 * crySetPpd //Not implemented for APR
 */
#ifdef __APR__
#include <pinpad.h>
#include <Sec.h>
#include <periphPrinter.h>

void tcik0167(void) {
    int ret;
    byte loc;
    byte blk[8];
    byte acc[16];
    const char *msg = "AMOUNT= 1000 EUR\nPIN:\n\n";

    pp_display_clear(0);
    pp_display_text(0, 1, "tcik0167", PT_ALIGN_LEFT);
    pp_display_update();
    pp_get_key(1);
    xx_print("tcik0167");

    pp_display_clear(1);
    pp_display_text(1, 1, "run tcik0165", PT_ALIGN_LEFT);
    pp_get_key(1);

    tcik0165();

    pp_display_clear(0);
    pp_display_text(0, 1, "tcik0167", PT_ALIGN_LEFT);
    pp_display_update();
    pp_get_key(1);

    pp_display_clear(1);
    pp_display_text(1, 1, "cryStart...", PT_ALIGN_LEFT);
    pp_get_key(1);
    ret = cryStart('m');
    CHECK(ret >= 0, lblKO);

    memcpy(acc, (byte *) "2347890123456741", 16);

    loc = 0x24;
    ret = cryGetPin(msg, acc, loc, blk);    //enter 1234 to obtain a known result
    CHECK(ret >= 0, lblKO);

    pp_display_clear(3);
    pp_display_text(3, 1, "Verify pin blk", PT_ALIGN_LEFT);
    pp_get_key(1);

    pp_display_clear(4);
    // Compare with the expected în block
    if(memcmp(blk, "\x06\xDA\x91\xDF\xF2\x76\xD3\xA8", 8))
        goto lblKO;

    xx_print("key verify OK");

    goto lblEnd;
  lblKO:
    xx_print("tcik0167 KO");
    goto lblEnd;
  lblEnd:
    cryStop('m');
    xx_print("tcik0167 End");

    return;
}
#else
void tcik0167(void) {
}
#endif
