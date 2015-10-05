/** \file
 * Unitary test case tcab0194.
 * GUI testing:
 * \sa
 *  - guiInputBrowse()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0194.c $
 *
 * $Id: tcab0194.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "pri.h"
#include "tst.h"
#include <string.h>

#ifdef __CANVAS__

static Pchar str[] = {
    "01. BORDEAUX",
    "02. CABERNET",
    "03. SAUVIGNON",
    "04. CHARDONNAY",
    "05. MERLOT",
    "06. PINOT NOIR",
    "07. SYRAH",
    "08. FRONSAC",
    "09. MEDOC",
    "10. PAUILLAC",
    "11. POMEROL",
    "12. SAUTERNES",
    "13. MUSCAT",
    "14. RIESLING",
    "15. SYLVANER",
    "16. ARMAGNAC",
    "17. BROUILLY",
    "18. MORGON",
    "19. BOURGOGNE",
    "20. ALIGOTE",
    "21. CHABLIS",
    "22. GIVRY",
    "23. RULLY",
    "24. MEURSAULT",
    "25. POMMARD",
    "26. CHAMPAGNE",
    "27. COGNAC",
    "28. POUILLY",
    "29. TAVEL",
    "30. LIRAC",
    "31. CASSIS",
    "32. CAHORS",
    "33. MADIRAN",
    "34. GAILLAC",
    "35. BERGERAC",
    "36. DURAS",
    "37. ANJOU",
    "38. GAMAY",
    "39. SAUMUR",
    "40. REUILLY",
    "41. CHINON",
    "42. CHEVERNY",
    "43. TOURAINE",
    "44. FITOU",
    "45. LIMOUX",
    "46. MERLOT",
    0
};

static int brwDim(void) {       ///<return the number of items to browse
    int dim = 0;

    while(str[dim])
        dim++;
    trcFN("brwDim: %d\n", dim);
    return dim;
}

static int brwGetTxt(word idx, char *buf, byte len) {   ///<get the row content
    memset(buf, 0, len);
    VERIFY(strlen(str[idx]) < len)
        strcpy(buf, str[idx]);
    trcFS("brwGetTxt: %s\n", buf);
    return strlen(buf);
}

static int brwGetFnt(word idx) {    ///<get the row text font
    return idx % 3;
}

static int test(void) {         //browsing selector with slider
    int ret;
    const char *ptr;
    tBar bgd;
    tBox foc;
    tBar beg, end;
    tSlider sld;
    tSelector sel;
    tShape shp[2 + 1];
    card rfp[2];
    tDrawing drw;
    tInput inp;
    tVmtBrw brw;

    byte wdt, hgt, rowHgt, idx;
    word clrSelBgd = clrBgd;
    word clrSelFoc = clrFgd;

    wdt = (byte) ((cnvW / dspW - 2) * dspW);
    hgt = (byte) ((cnvH / dspH - 1) * dspH);
    rowHgt = (cnvH / dspH);

    memset(shp, 0, sizeof(shp));
    memset(rfp, 0, sizeof(rfp));

    ret = cnvClear();
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&beg, 8, 8, clrFgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&end, 8, 8, clrBgd);
    CHECK(ret >= 0, lblKO);

    ret = drwBarInit(&bgd, wdt, hgt, clrSelBgd);
    CHECK(ret >= 0, lblKO);
    ret = drwBoxInit(&foc, wdt, rowHgt, clrSelFoc);
    CHECK(ret >= 0, lblKO);

    idx = 0;
    ret = drwSelectorInit(&sel, &bgd, &foc, &sld, 0);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpSelector, &sel);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(2, 2);

    idx++;
    ret = drwSliderInit(&sld, &beg, &end, 0, hgt - 8);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp[idx], shpSlider, &sld);
    CHECK(ret >= 0, lblKO);
    rfp[idx] = CARDHL(2 + wdt + 2, 2);

    ret = drwDrawingInit(&drw, shp, rfp);
    CHECK(ret >= 0, lblKO);
    ret = guiInputInit(&inp, &drw, 0, 0, "KT", 0, 60);
    CHECK(ret >= 0, lblKO);

    brw.dim = brwDim;
    brw.getTxt = brwGetTxt;
    brw.getFnt = brwGetFnt;
    ret = guiInputBrowse(&inp, &brw, CARDHL(0, 0));
    if(HWORD(ret) == 0xFFFF)
        ptr = "Aborted";
    else
        ptr = str[LWORD(ret)];
    ret = prtS(ptr);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0194(void) {
    int ret;

    trcS("tcab0194 Beg\n");

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    RUN(test);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    tmrPause(1);
    cnvStop();
    prtStop();
    dspStop();
    trcS("tcab0194 End\n");
}
#else
void tcab0194(void) {
}
#endif
