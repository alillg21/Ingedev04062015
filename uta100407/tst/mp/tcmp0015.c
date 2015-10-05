/** \file
 * Unitary test case tcmp0015.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioRemove()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0015.c $
 *
 * $Id: tcmp0015.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0015.c
//Functions testing the removal of a record in the database

void tcmp0015(void) {
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
    dspLS(1, "Enroll a Fingerprint");

	//Create the database
	ret = bioCreateDB(100, 1, "0|128|NAME|1|32|PSWORD", timeout);

    //Allow add up to three records
    ret = bioEnroll(1, timeout, "TestRemoval", "Left Hand Finger|Index");
    CHECK(ret >= 0, lblKO);
	
    dspClear(); 
    prtS("Enrollment OK");
	
    ret = bioRemove("TestRemoval", timeout);
    CHECK(ret >= 0, lblKO);

    dspClear(); 
    prtS("Removal OK");
    dspLS(0, "Removal OK");  
    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("Removal KO!");
    dspLS(0, "Removal KO!");
  lblEnd:    
    tmrPause(2);
    bioStop();
    kbdStop();  
    prtStop();
    dspStop();
#endif  
}


