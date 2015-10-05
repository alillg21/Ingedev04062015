/** \file
 * Unitary test case tcjl0000.
 * Functions testing:
 * \sa
 *  - getSapSer()
 */

#include "sys.h"
#include <string.h>
void tcjl0000(void) {

    int ret;                    //*< integer value for return code */
    char serialNum[20 + 1];     // length of NO_SERIE
    char sap[16 + 1];
    char dvc;

    trcS("tcjl0000 Beg\n");

    //open the associated channel
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    trcS("Get Terminal SN\n");
    ret = dspLS(0, "Getting Terminal");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Serial Number");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    memset(serialNum, 0, sizeof(serialNum));
    memset(sap, 0, sizeof(sap));
    memset(&dvc, 0, sizeof(dvc));
    //get terminal serial number
    getSapSer(sap, serialNum, dvc);

    //present results
    ret = prtS("Terminal SN:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(serialNum);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, serialNum);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);
    trcBuf(serialNum, sizeof(serialNum));
    goto lblEnd;

  lblKO:
    ret = dspLS(0, "!OK");
    CHECK(ret >= 0, lblKO);
    trcErr(ret);
  lblEnd:
    //close resource
    prtStop();
    dspStop();
    trcS("tcjl0000 End\n");
}
