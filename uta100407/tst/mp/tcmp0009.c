/** \file
 * Unitary test case tcmp0009.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioEnroll()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0009.c $
 *
 * $Id: tcmp0009.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0009.c
//Functions testing biometrics fingerprint enrollment for verification
//Fingerprints are stored in temporary list of reference templates

void tcmp0009(void) {
#ifdef __BIO__
    int ret;
    int timeout = 30;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    ret = bioStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    dspLS(0, " ENROLLMENT FOR");
    dspLS(1, "  VERIFICATION");
    dspLS(2, "  Place finger");
    dspLS(3, "   or CANCEL");

    ret = bioEnroll(0, timeout, NULL, NULL);
    CHECK(ret >= 0, lblKO);

    dspClear();	
    prtS("Enrollment OK");
    dspLS(0, "Enrollment OK");	
    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("Enrollment KO!");
    dspLS(0, "Enrollment KO!");
  lblEnd:    
    tmrPause(2);
    bioStop();
    kbdStop();	
    prtStop();
    dspStop();
#endif	
}


