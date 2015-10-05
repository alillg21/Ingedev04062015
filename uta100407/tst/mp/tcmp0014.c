/** \file
 * Unitary test case tcmp0014.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioDestroyDB()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0014.c $
 *
 * $Id: tcmp0014.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0014.c
//Functions testing the removal of database

void tcmp0014(void) {
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
    dspLS(1, " Removing DBase");

    ret = bioDestroyDB(timeout);
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


