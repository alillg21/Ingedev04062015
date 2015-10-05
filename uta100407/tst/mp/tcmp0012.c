/** \file
 * Unitary test case tcmp0012.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioCreateDB()
 *  - bioEnroll()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0012.c $
 *
 * $Id: tcmp0012.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0012.c
//Functions testing biometrics fingerprint enrollment for identification
//Fingerprints are stored in database

void tcmp0012(void) {
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
    dspLS(1, " IDENTIFICATION");
    dspLS(2, "  Place finger");
    dspLS(3, "   or CANCEL");
	
	//Create the database
	ret = bioCreateDB(100, 1, "0|128|NAME|1|32|PSWORD", timeout);

    //Allow add up to three records
    ret = bioEnroll(1, timeout, "TempRecord1", "First Finger|Index");
    if (ret < 0) {
        ret = bioEnroll(1, timeout, "TempRecord2", "Second Finger|Middle");
    }
    if (ret < 0) {
        ret = bioEnroll(1, timeout, "TempRecord3", "Third Finger|Thumb");
    }
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


