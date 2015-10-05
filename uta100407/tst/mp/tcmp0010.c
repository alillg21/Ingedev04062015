/** \file
 * Unitary test case tcmp0010.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioVerify()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0010.c $
 *
 * $Id: tcmp0010.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0010.c
//Functions testing for verifying a specific fingerprint in the list of
//reference templates

void tcmp0010(void) {
#ifdef __BIO__
    int ret;
    int timeout = 30;
    char res;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    ret = bioStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    dspLS(0, "     VERIFY");
    dspLS(1, "  Place finger");
    dspLS(2, "   or CANCEL");

    res = bioVerify(timeout);
    prtS("Verification Done");

    dspClear();	
    dspLS(0, "Verification OK");

    switch (res) {
        case 1:
            prtS("Same Finger");
            dspLS(1, "Same Finger");			
            break;
        case 0:
            prtS("Diff Finger");
            dspLS(1, "Diff Finger");			
            break;
        default:
            prtS("Timeout");
            dspLS(1, "Timeout");		
            break;
    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("Verification KO!");
    dspLS(0, "Verification KO!");
  lblEnd:    
    tmrPause(2);  	
    bioStop();
    kbdStop();	
    prtStop();
    dspStop();
#endif	
}


