/** \file
 * Unitary test case tcjl0000.
 * Functions testing:
 * \sa
 *  - comGetIMSI()
 */

#include "sys.h"
#include <string.h>
void tcjl0002(void) {

    int ret;                    //*< integer value for return code */
    char Imsi[15 + 1];
    char pin[16 + 1];

    trcS("tcjl0002 Beg\n");

    //open the associated channel
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    trcS("Get IMSI \n");
    ret = dspLS(0, "Getting Terminal");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "IMSI");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    memset(&pin, 0, sizeof(pin));
    //get International Mobile Subscriber Identity
    ret = comGetIMSI(Imsi, pin);
    CHECK(ret >= 0, lblKO);

    //present results
    ret = prtS("Terminal IMSI:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(Imsi);
    CHECK(ret >= 0, lblKO);
    goto lblEnd;

  lblKO:
    ret = dspLS(0, "!OK");
    CHECK(ret >= 0, lblKO);
    trcErr(ret);
  lblEnd:
    //close resource
    prtStop();
    dspStop();
    trcS("tcjl0002 End\n");
}
