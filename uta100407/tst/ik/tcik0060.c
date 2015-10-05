#include <string.h>
#include <unicapt.h>
#include "sys.h"
//#include "log.h"
#include "tst.h"
#include "ssaStd.h"
#include "ssaSec.h"

//Unitary test case tcik0060.
//Functions testing:
//ssaSecLafWriteFont
//ssaSecLafSelectPrnDefaultFont

static uint32 hPrt = 0;

void tcik0060(void) {
    int ret;
    uint32 secHandle;           //, prnHandle;
    ssaFontDescr_t descriptor1;
    ssaFontDescr_t descriptor2;
    char tmp[255];

    //open ressources
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    dspLS(0, "prnOpen");
    tmrPause(2);
    ret = prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    CHECK(ret == RET_OK, lblKO);
    dspLS(0, "ssaSecOpen");
    tmrPause(2);
    //hmiOpen("DEFAULT", &hmiHandle);
    ret = ssaSecOpen(SSA_MERCHANT, &secHandle);
    CHECK(ret >= 0, lblKO);
    dspLS(0, "WriteFont");
    tmrPause(2);
    ret = ssaSecLafWriteFont(secHandle, TERM, Iso8859_35_b, &descriptor1);
    CHECK(ret >= 0, lblKO);
    dspLS(0, "SelectFont");
    tmrPause(2);
    ret =
        ssaSecLafSelectPrnDefaultFont(secHandle, TERM,
                                      HMI_INTERNAL_FONT_RAM(descriptor1.
                                                            hmiNormSlot),
                                      HMI_INTERNAL_FONT_RAM(descriptor1.
                                                            hmiBoldSlot));
    CHECK(ret >= 0, lblKO);

    //hmiOpen("DEFAULT", &hmiHandle);
    //ret=ssaSecLafSelectPrnDefaultFont(secHandle,TERM, HMI_INTERNAL_FONT_1,0);
    //CHECK(ret >= 0, lblKO);

    //prtES("\x1C\x1A","EFT TERM READY");
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "Iso8859_35_b");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "\x1B\x17");
    strcat(tmp, "EFT TERM READY");
    dspLS(0, "prnPrint");
    tmrPause(2);
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "\x1B\x17");
    strcat(tmp, "\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);

    /*
       prtES("\x1C\x1A","\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
       prtES("\x1b\x18","EFT TERM READY");
       prtES("\x1b\x18","\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
       prtES("\x1b\x19","EFT TERM READY");
       prtES("\x1b\x19","\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
     */

    dspLS(0, "WriteFont");
    tmrPause(2);
    ret = ssaSecLafWriteFont(secHandle, TERM, Iso8859_35, &descriptor2);
    if(ret == ERR_FONT_ALREADY_PRESENT) {   // another application already loaded the font
        descriptor2.hmiNormSlot = 1;
        descriptor2.hmiBoldSlot = 2;
        ret = 0;
    }
    CHECK(ret >= 0, lblKO);
    dspLS(0, "SelectFont");
    tmrPause(2);
    ret =
        ssaSecLafSelectPrnDefaultFont(secHandle, TERM,
                                      HMI_INTERNAL_FONT_RAM(descriptor2.
                                                            hmiNormSlot),
                                      HMI_INTERNAL_FONT_RAM(descriptor2.
                                                            hmiBoldSlot));
    CHECK(ret >= 0, lblKO);

    dspLS(0, "prnPrint");
    tmrPause(2);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "Iso8859_35");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "\x1B\x17");
    strcat(tmp, "\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "\x1B\x19");
    strcat(tmp, "EFT TERM READY");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);

    ret =
        ssaSecLafSelectPrnDefaultFont(secHandle, TERM, HMI_INTERNAL_FONT_1, 0);
    CHECK(ret >= 0, lblKO);

    memcpy(tmp, "\x00", 256);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "INTERNAL_FONT_1");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "\x1B\x17");
    strcat(tmp, "\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
    memcpy(tmp, "\x00", 256);
    strcpy(tmp, "\x1B\x19");
    strcat(tmp, "EFT TERM READY");
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);

/*
		prtES("\x1C\x1A","EFT TERM READY");
		prtES("\x1C\x1A","\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
		prtES("\x1b\x18","EFT TERM READY");
		prtES("\x1b\x18","\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
		prtES("\x1b\x19","EFT TERM READY");
		prtES("\x1b\x19","\xB1\xFE\xF3\xF5\xE3\xF3\x90 \x96\xF5\x97");
*/
    ssaSecClose(secHandle);
    //hmiClose(hmiHandle);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(2);
  lblEnd:

    ret = prnClose(hPrt);
    dspLS(0, "DONE!");
    dspStop();                  //close resources
    tmrPause(1);
}
