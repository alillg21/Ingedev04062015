/** \file
 * Unitary test case tcab0166.
 * \sa
 *  - prnPrint()
 *  - hmiADDisplayText()
 *  - hmiADFont()
 *  - hmiADClearLine()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0166.c $
 *
 * $Id: tcab0166.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <unicapt.h>
#include "message.h"
#include "ssaStd.h"
#include "ssaSec.h"
#include "string.h"

static int testPrn(uint32 hPrn, const char *hdr, const char *nrm,
                   const char *bld) {
    int ret;

    ret = prnPrint(hPrn, (char *) hdr);
    if(ret != RET_OK)
        return -1;
    ret = prnPrint(hPrn, (char *) nrm);
    if(ret != RET_OK)
        return -1;
    ret = prnPrint(hPrn, (char *) bld);
    if(ret != RET_OK)
        return -1;
    return 1;
}

static int testDsp(uint32 hHmi, const ssaFontDescr_t * dsc, const char *hdr,
                   const char *nrm, const char *bld) {
    int ret;

    ret = hmiADClearLine(hHmi, 0);
    if(ret != RET_OK)
        return -1;
    ret = hmiADFont(hHmi, HMI_INTERNAL_FONT_RAM(dsc->hmiNormSlot));
    if(ret != RET_OK)
        return -1;
    ret = hmiADDisplayText(hHmi, 0, 0, (char *) hdr);
    if(ret != RET_OK)
        return -1;

    ret = hmiADClearLine(hHmi, 1);
    if(ret != RET_OK)
        return -1;
    ret = hmiADFont(hHmi, HMI_INTERNAL_FONT_RAM(dsc->hmiNormSlot));
    if(ret != RET_OK)
        return -1;
    ret = hmiADDisplayText(hHmi, 1, 0, (char *) nrm);
    if(ret != RET_OK)
        return -1;

    ret = hmiADClearLine(hHmi, 2);
    if(ret != RET_OK)
        return -1;
    ret = hmiADFont(hHmi, HMI_INTERNAL_FONT_RAM(dsc->hmiBoldSlot));
    if(ret != RET_OK)
        return -1;
    ret = hmiADDisplayText(hHmi, 2, 0, (char *) bld);
    if(ret != RET_OK)
        return -1;
    return 1;
}

static int test35(void) {
    int ret;
    uint32 hSsa, hPrn, hHmi;
    uint8 key;
    ssaFontDescr_t dsc;

    hSsa = 0;
    hPrn = 0;
    hHmi = 0;

    ret = ssaSecOpen(SSA_MERCHANT, &hSsa);
    if(ret != RET_OK)
        goto lblEnd;

    ret = hmiOpen("DEFAULT", &hHmi);
    if(ret != RET_OK)
        goto lblEnd;

    ret = prnOpen(PRN_DEFAULT_CHANNEL, &hPrn, PRN_IMMEDIATE);
    if(ret != RET_OK)
        goto lblEnd;

    ret = ssaSecLafWriteFont(hSsa, TERM, Iso8859_35, &dsc);
    if(ret != RET_OK)
        goto lblEnd;

    ret = ssaSecLafSelectPrnDefaultFont(hSsa, TERM,
                                        HMI_INTERNAL_FONT_RAM(dsc.hmiNormSlot),
                                        HMI_INTERNAL_FONT_RAM(dsc.hmiBoldSlot));

    ret = testPrn(hPrn, "test35:",
                  "NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9",
                  "\x1B\x1A BOLD \xC0\xC1\xC3\xC6\xC7\xC8\xC9");
    if(ret < 0)
        goto lblEnd;

    ret = testDsp(hHmi, &dsc, "test35:",
                  "NORM \xAD\xB1\xB2\xB3\xB4\xB5\xB6",
                  "BOLD \xAD\xB1\xB2\xB3\xB4\xB5\xB6");
    if(ret < 0)
        goto lblEnd;

    hmiKeyWait(hHmi, &key, PSY_INFINITE_TIMEOUT);
  lblEnd:
    if(hHmi)
        hmiClose(hHmi);
    if(hPrn)
        prnClose(hPrn);
    if(hSsa)
        ssaSecClose(hSsa);
    return 0;
}

/** NON UTA test case for font management using SSA 5.41 or later
 * The fonts in CFS are used.
 *
 * This test case is based on the test case tcml0005().
 *
 */
void tcab0166(void) {
    int ret;

    ret = test35();             //arab
    if(ret < 0)
        return;
}
