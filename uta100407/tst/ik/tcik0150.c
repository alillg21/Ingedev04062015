/** \file 
 * Unitary test case for sys component
 * \sa
 *  - cryDiag()
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

void tcik0150(void) {
    int ret;
    int idx;
    int NbSecArea;
    int TabSecArea[8];
    char tmp[prtW + 1];

    memset(tmp, 0, prtW + 1);

    /*! TEST OF SECRET AREA
     * Number of secret areas is limited in Sagem terminals
     * This function is usefull to diagnostic eventual overflow
     * And also to check if a correct secret area was created
     */
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "cryDiag...");
    CHECK(ret >= 0, lblKO);

    ret = cryDiag(&NbSecArea, TabSecArea);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "cryDiag OK");
    CHECK(ret >= 0, lblKO);

    prtS("Secret areas:");

    for (idx = 0; idx < NbSecArea; idx++) {

        sprintf(tmp, "%01X: ", idx);
        num2hex(tmp + strlen(tmp), TabSecArea[idx], 2 * sizeof(int));

        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);
    }

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
