/// \file tcNS0031.c
/** Unitary UTA test case : VendingPass LEDs demonstration
 * \sa
 */
#include "ctx.h"
#ifdef __MFC__
#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

void tcns0031(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = clsStart();
    CHECK(ret >= 0, lblKO);

    // turn the backlight on
    clsBacklightLevel(100);

    // Turn on the LED : I 0 0 0
    ret = clsLED(0x1F, 0);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn on the LED : 0 I 0 0
    ret = clsLED(0x2F, 0);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn on the LED : 0 0 I 0
    ret = clsLED(0x4F, 0);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn on the LED : 0 0 0 I
    ret = clsLED(0x8F, 0);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn off all the upper LEDs
    ret = clsLED(0x10, 0);
    CHECK(ret >= 0, lblKO);
    ret = clsLED(0x20, 0);
    CHECK(ret >= 0, lblKO);
    ret = clsLED(0x40, 0);
    CHECK(ret >= 0, lblKO);
    ret = clsLED(0x80, 0);
    CHECK(ret >= 0, lblKO);

    // Turn on the color leds: 'G' for GREEN
    ret = clsLED(0x0F, 'G');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn the GREEN off
    ret = clsLED(0x00, 'G');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn on the color leds: 'Y' for YELLOW
    ret = clsLED(0x0F, 'Y');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn the YELLOW off
    ret = clsLED(0x00, 'Y');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn on the color leds: 'R' for RED
    ret = clsLED(0x0F, 'R');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn the RED off
    ret = clsLED(0x00, 'R');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Blinking leds I 0 0 0 + GREEN
    ret = clsLED(0x15, 'G');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Blinking leds I I 0 0 + YELLOW
    ret = clsLED(0x25, 'Y');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Blinking leds I I I 0 + RED
    ret = clsLED(0x45, 'R');
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Blinking leds I I I I
    ret = clsLED(0xF5, 0);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // Turn EVERYTHING off
    ret = clsLED(0, 0);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    // turn the backlight off
    clsBacklightLevel(0);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(1, "Test Fail");
    goto lblEnd;
  lblEnd:
    prtS("tcns0031 done");
    prtS("--------");
    clsStop();
    dspStop();
    prtStop();

}
#else
void tcns0031(void) {
}
#endif
