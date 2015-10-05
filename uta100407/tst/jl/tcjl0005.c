/** \file
 * Unitary test case tcjl0005.
 * Functions testing:
 * \sa
 *  - comGetIMSI()
 */

#include "sys.h"
#include <string.h>
void tcjl0005(void) {

    int ret;                    //*< integer value for return code */
    char imsi[15 + 1];          // length of NO_SERIE
    char pin[16 + 1];

    trcS("tcjl0005 Beg\n");

    //open the associated channel
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    trcS("Get Terminal IMSI\n");
    ret = dspLS(0, "Getting Terminal");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "IMSI");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //get terminal serial number
    comGetIMSI(imsi, pin);
    CHECK(ret >= 0, lblKO);

    //present results
    ret = prtS("Terminal IMSI:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(imsi);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, imsi);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);
    trcBuf(imsi, sizeof(imsi));
    goto lblEnd;

  lblKO:
    ret = dspLS(0, "!OK");
    CHECK(ret >= 0, lblKO);
    trcErr(ret);
  lblEnd:
    //close resource
    prtStop();
    dspStop();
    trcS("tcjl0005 End\n");
}
