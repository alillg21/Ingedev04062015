/** \file
 * Unitary test case tcmp0013.
 * Functions testing:
 * \sa
 *  - bioStart()
 *  - bioStop()
 *  - bioIdentify()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/mp/tcmp0013.c $
 *
 * $Id: tcmp0013.c 2534 2010-01-06 10:35:08Z abarantsev $
 */
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif
#include "sys.h"
#include "tst.h"

//Unitary test case tcmp0013.c
//Functions testing for identifying a specific fingerprint from the database

void tcmp0013(void) {
#ifdef __BIO__
    int ret;
    int timeout = 30;
    card index;
    char res;
	char userID[32];
	char data[64];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = kbdStart(1);
    CHECK(ret >= 0, lblKO);

    ret = bioStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    dspLS(0, "    IDENTIFY");
    dspLS(1, "  Place finger");
    dspLS(2, "   or CANCEL");

    res = bioIdentify(timeout, &index, userID, data);
    prtS("Identification Done");
	trcFN("Index = %d", index);
	trcFS("User ID = %s", userID);
	trcFS("Data String = %s", data);

    dspClear(); 
    dspLS(0, "Identify OK");

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
    dspClear(); 
    prtS("Identify KO!");
    dspLS(0, "Identify KO!");
  lblEnd:    
    tmrPause(2);    
    bioStop();
    kbdStop();  
    prtStop();
    dspStop();
#endif  
}


