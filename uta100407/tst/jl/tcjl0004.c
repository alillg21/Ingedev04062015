/** \file
 * Unitary test case tcjl0004.
 * Functions testing:
 * \sa
 *  - getCodeFileList()
 */

#include "sys.h"
#include <string.h>
void tcjl0004(void) {
    int ret;
    tFileDetails fileDetails;
    char crc[6 + 1];

    trcS("tcjl0004 Beg\n");

    //open the associated channel
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    trcS("Get Terminal File List\n");
    ret = dspLS(0, "Getting Terminal");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "File List");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    getCodeFileList(&fileDetails, crc);
    CHECK(ret >= 0, lblKO);

    //present results
    num2hex(crc, fileDetails.fCrc, 4);
    ret = prtS("File Name:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(fileDetails.fileName);
    CHECK(ret >= 0, lblKO);
    ret = prtS("CRC:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(crc);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, fileDetails.fileName);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, crc);
    CHECK(ret >= 0, lblKO);
    tmrPause(5);

    trcBuf(fileDetails.fileName, sizeof(fileDetails.fileName));
    trcBuf(crc, sizeof(fileDetails.fCrc));
    goto lblEnd;

  lblKO:
    ret = dspLS(0, "!OK");
    CHECK(ret >= 0, lblKO);
    trcErr(ret);
  lblEnd:
    //close resource
    prtStop();
    dspStop();
    trcS("tcjl0004 End\n");
}
