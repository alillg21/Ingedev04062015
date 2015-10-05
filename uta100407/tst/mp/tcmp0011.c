/** \file
 * Unitary test case tcmp0011.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioClearDB()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0011.c $
 *
 * $Id: tcmp0011.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0011.c
//Functions testing the removal of the list of reference templates of biometrics
//fingerprint and the database

void tcmp0011(void) {
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
    dspLS(0, " Removing  List");
    dspLS(1, "  of Reference");
    dspLS(2, "    Template");
    tmrPause(1);	

    ret = bioClearDB(0, timeout);
    CHECK(ret >= 0, lblKO);

    dspClear(); 
    prtS("Removal OK");
    dspLS(0, "Removal OK");  
    tmrPause(1);	

    dspClear();
    dspLS(0, " Removing  List");
    dspLS(1, " from Database");
    tmrPause(1);	

    ret = bioClearDB(1, timeout);
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


