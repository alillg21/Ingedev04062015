/** \file
 * Unitary test case for Hebrew font display and printing
 * \sa
 *  - prtStart()
 *  - prtStop()
 *  - prtS()
 *  - prtES()
 *  - dspClear()
 *  - dspLS()
 *  - dspStop()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/jl/tcjl0008.c $
 *
 * 
 */
#include <string.h>
#include "sys.h"
#include "tst.h"

void tcjl0008(void) {
    int ret;                    //*< integer value for return code */
	const char *helloWorld = "Hello world";
	const char *hebrewText1 = "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef";
	const char *hebrewText2 = "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa";

    /*!
     * \par Step 1: call prtStart() & dspClear()
     * Make sure font to be used is set in ctx.h using __FONT__ macro
     * For Hebrew define __FONT__ Iso8859_8
     * Inside of this function printer and display fonts are selected in selectFontPrt() and selectFontDsp().
     * otherwise both functions return 0.
     * Depending on SSA version >=541 you need just to download appropriate CFS font file.
     * Or for SSA version<=517 appropriate SSA.
     */

    trcS("tcjl0008 Beg\n");
    ret = dspClear();           //clear display
    CHECK(ret >= 0, lblKO);

    ret = prtStart();           //open the associated channel
    CHECK(ret >= 0, lblKO);	

	prtS("== test2: =============");

	prtS(helloWorld);
	prtS(hebrewText1);
	prtS(hebrewText2);

	dspLS(0, helloWorld);
	dspLS(1, hebrewText1);
	tmrPause(5);
	
	prtES("\x1b\x1a", helloWorld);
	prtES("\x1b\x1a", hebrewText1);
	prtES("\x1b\x1a", hebrewText2);

	ret = dspLS(BLD(2), helloWorld);
    CHECK(ret >= 0, lblKO);
	ret = dspLS(BLD(3), hebrewText1);
    CHECK(ret >= 0, lblKO);

	tmrPause(5);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    goto lblEnd;
  lblEnd:
    //close resource
    prtStop();
    dspStop();
    trcS("tcjl0008 End\n");
}
