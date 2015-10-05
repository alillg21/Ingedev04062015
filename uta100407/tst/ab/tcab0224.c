/** \file
 * Unitary test case for chinese fonts (simhei)
 *
 * http://rishida.net/scripts/uniview/conversion.php
 * http://en.wikipedia.org/wiki/Lion-Eating_Poet_in_the_Stone_Den
 * 
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0224.c $
 *
 * $Id: tcab0224.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <sdk30.h>
#include "sys.h"
#include "tst.h"

#define DSP(L,S,I) DrawExtendedStringUnicode(0,L*16,(char *)S,I,"_dSIMHEI_",_FIXED_WIDTH_);
#define PRT(S) pprintfUnicode((char *)S, _OFF_,"_pSIMHEI_", _FIXED_WIDTH_); pprintfUnicode((char *)crlf, _OFF_,"_pSIMHEI_", _FIXED_WIDTH_)
void tcab0224(void) {
    int ret;
    const char *simhei = "/SYSTEM/SIMHEI.SGN";
    unsigned char *fnt;
    FILE *hDsp = 0;
    FILE *hPrt = 0;

    const char *uta1 = "\x90\x1A\x75\x28\x7D\x42\x7A\xEF\x61\xC9\x75\x28\x00";
    const char *uta2 =
        "\x90\x1A\x75\x28\x7D\x42\x7A\xEF\x6A\x5F\x61\xC9\x75\x28\x7A\x0B\x5F\x0F\x00";
    const char *shi0 =
        "\x30\x0A\x65\xBD\x6C\x0F\x98\xDF\x73\x45\x53\xF2\x30\x0B\x00";
    const char *shi1 =
        "\x77\xF3\x5B\xA4\x8A\x69\x58\xEB\x65\xBD\x6C\x0F\x00\x2C\x55\xDC\x73\x45\x00\x2C\x8A\x93\x98\xDF\x53\x41\x73\x45\x30\x02\x00";
    const char *shi2 =
        "\x6C\x0F\x66\x42\x66\x42\x90\x69\x5E\x02\x89\x96\x73\x45\x30\x02\x00";
    const char *shi3 =
        "\x53\x41\x66\x42\xFF\x0C\x90\x69\x53\x41\x73\x45\x90\x69\x5E\x02\x30\x02\x00";
    const char *shi4 =
        "\x66\x2F\x66\x42\x00\x2C\x90\x69\x65\xBD\x6C\x0F\x90\x69\x5E\x02\x30\x02\x00";
    const char *shi5 =
        "\x6C\x0F\x89\x96\x66\x2F\x53\x41\x73\x45\xFF\x0C\x60\x43\x77\xE2\x52\xE2\xFF\x0C\x4F\x7F\x66\x2F\x53\x41\x73\x45\x90\x1D\x4E\x16\x30\x02\x00";
    const char *shi6 =
        "\x6C\x0F\x62\xFE\x66\x2F\x53\x41\x73\x45\x5C\x4D\x00\x2C\x90\x69\x77\xF3\x5B\xA4\x30\x02\x00";
    const char *shi7 =
        "\x77\xF3\x5B\xA4\x6F\xD5\xFF\x0C\x6C\x0F\x4F\x7F\x4F\x8D\x62\xED\x77\xF3\x5B\xA4\x30\x02\x00";
    const char *shi8 =
        "\x77\xF3\x5B\xA4\x62\xED\xFF\x0C\x6C\x0F\x59\xCB\x8A\x66\x98\xDF\x66\x2F\x53\x41\x73\x45\x30\x02\x00";
    const char *shi9 =
        "\x98\xDF\x66\x42\xFF\x0C\x59\xCB\x8B\x58\x66\x2F\x53\x41\x73\x45\xFF\x0C\x5B\xE6\x53\x41\x77\xF3\x73\x45\x5C\x4D\x30\x02\x00";
    const char *shiA = "\x8A\x66\x91\xCB\x66\x2F\x4E\x8B\x30\x02";
    const char *crlf = "\x00\x0A\x00";

    fnt = LoadFont((char *) simhei);
    CHECK(fnt, lblKO);

    hPrt = fopen("PRINTER", "w-");
    CHECK(hPrt, lblKO);

    pprintf("test simhei\n");

    hDsp = fopen("DISPLAY", "w");
    CHECK(hDsp, lblKO);
    _clrscr();

    DefCurrentFont(fnt);

    ret = DSP(1, uta1, _OFF_);
    CHECK(ret >= 0, lblKO);
    ret = DSP(2, uta2, _ON_);
    CHECK(ret >= 0, lblKO);

    PRT(shi0);
    PRT(shi1);
    PRT(shi2);
    PRT(shi3);
    PRT(shi4);
    PRT(shi5);
    PRT(shi6);
    PRT(shi7);
    PRT(shi8);
    PRT(shi9);
    PRT(shiA);

    ttestall(0, 300);
    goto lblEnd;
  lblKO:
    ;
  lblEnd:
    fclose(hPrt);
    fclose(hDsp);
}
