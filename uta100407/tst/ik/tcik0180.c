/** \file
 * non-UTA Unitary test case tcik0180 for color implementation for Telium platform.
 */
#include "ctx.h"

#ifdef __COLOR12__
#include <sdk30.h>
#include <Libgr.h>

static unsigned short int screen[320 * 240];

enum test_color {
    clrBgd = 0x00,
    clrRed = ((0x0F << 8) | clrBgd),
    clrGreen = ((clrBgd << 8) | 0xF0),
    clrBleu = (clrBgd | 0x0F),
    clrYellow = (clrGreen | clrRed)
};

static int RGB(int color) {
    return ((((color & 0xF00) << 12) + ((color & 0x0F0) << 8) +
             ((color & 0x00F) << 4)) + 0xFF000000);
}

void tcik0180(void) {
    FILE *hDisplay;
    unsigned int nHeaderStatus;
    unsigned long Wdth, Hght;
    unsigned short int lineX[2 * 240];
    unsigned short int lineY[320 * 2];
    unsigned short int rect[16 * 16];
    int idx;

    // libgrlib_open();            // Graphic Library - deprecated function in 7.1

    // Remove the top banner.
    nHeaderStatus = StateHeader(0);

    InitContexteGraphique(PERIPH_DISPLAY_EXTENDED);
    hDisplay = fopen("DISPLAY", "a");

    Wdth = 0x140;
    Hght = 0xF0;
    memset(screen, clrGreen, (sizeof(screen)));
    _SetScreenToScreen(0, 0, screen, Wdth, Hght);

    for (idx = 0; idx < (sizeof(lineX) / sizeof(lineX[0])); idx++)
        lineX[idx] = clrRed;
    _SetScreenToScreen(60, 0, lineX, 2, 0xF0);

    for (idx = 0; idx < (sizeof(lineY) / sizeof(lineY[0])); idx++)
        lineY[idx] = clrBleu;
    _SetScreenToScreen(0, 60, lineY, 0x140, 2);

    for (idx = 0; idx < (sizeof(rect) / sizeof(rect[0])); idx++)
        rect[idx] = clrYellow;
    _SetScreenToScreen(150, 110, rect, 16, 16);

    _DrawText8859(100, 100, "TCIK0180 GRAPHIC", "_dNORMAL_", _FIXED_WIDTH_,
                  RGB(0xF00), RGB(clrGreen));

    ttestall(KEYBOARD, 500);    //until now nothing is displayed on the screen

    PaintGraphics();
    ttestall(KEYBOARD, 500);

    fclose(hDisplay);
    InitContexteGraphique(PERIPH_DISPLAY);
    StateHeader(nHeaderStatus);
}
#else
void tcik0180(void) {
}
#endif
