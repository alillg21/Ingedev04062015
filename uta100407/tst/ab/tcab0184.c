/** \file
 * Unitary nonUTA test case tcab0184.
 * hmiOpDrawText test to show font processing anomaly:
 * the font 2 is displayed in different way under simelite and in the terminal
 * Test is done for I5100
 * \sa
 *  - hmiGraphicBppSet()
 *  - hmiOpInitGC()
 *  - hmiOpSetFont()
 *  - hmiOpSetForeground()
 *  - hmiOpSetBackground()
 *  - hmiGraphicDataWrite()
 *  - hmiOpDrawText()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0184.c $
 *
 * $Id: tcab0184.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>             // for memset, strcpy, etc
#include <unicapt.h>

static Canvas *cnv = 0;
static GC ctxOpt;
static uint32 hHmi = 0;

#define cnvW 128
#define cnvH 64
#define cnvD 1

static void cnvStart(void) {
    static Canvas _cnv;
    static char bmp[(cnvW * cnvH) / 8];
    Dimen wdt, hgt;
    uint8 pal, bpp;

    wdt = cnvW;
    hgt = cnvH;
    pal = 2;
    bpp = HMI_BPP_1;
    cnv = &_cnv;
    hmiOpDefineCanvas(cnv, wdt, hgt, pal, (Bitmap *) bmp);
    hmiGraphicBppSet(hHmi, (hmiBpp_t) bpp);
    hmiOpInitGC(&ctxOpt);
}

static void cnvText(uint16 x, uint16 y, const char *txt, uint8 fnt, uint8 clr) {
    uint8 len;

    hmiOpSetFont(&ctxOpt, fnt);
    hmiOpSetForeground(&ctxOpt, clr);
    len = strlen(txt);
    hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
}

void tcab0184(void) {
    uint8 key;

    hmiOpen("DEFAULT", &hHmi);
    cnvStart();
    hmiOpSetBackground(&ctxOpt, 0);
    hmiOpClearCanvas(cnv, &ctxOpt);
    cnvText(16, 16, "Arnaud LUCIEN", 2, 1);
    hmiGraphicDataWrite(hHmi, cnv->bm, 0, 0, cnv->w, cnv->h);
    hmiKeyWait(hHmi, &key, PSY_INFINITE_TIMEOUT);
    hmiClose(hHmi);
}
