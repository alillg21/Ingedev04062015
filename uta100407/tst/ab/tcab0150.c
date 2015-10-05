/** \file
 * Unitary test case tcab0150.
 * Functions testing:
 * \sa
 *  - dbaExport()
 *  - dbaImport()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0150.c $
 *
 * $Id: tcab0150.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0150(void) {
    int ret;
    byte volDba[256];

    trcS("tcab0150 Beg\n");
    nvmStart();                 //initialise non-volatile memory

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    memset(volDba, 0, 256);
    dbaSetPtr(volDba);          //the buffer volDba will be used for volatile section of data base

    ret = dspLS(1, "Remove...");
    CHECK(ret >= 0, lblKO);
    nvmRemove(3);               //don't check the return code; maybe the file does not exist

    ret = dspLS(2, "Export...");
    CHECK(ret >= 0, lblKO);

    memcpy(volDba, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8);

    ret = dbaExport(3, 0xFF, 0, 1, 'H', "01");
    CHECK(ret > 0, lblKO);
    ret = dbaExport(3, 0xFF, 0, 1, 'D', "1");
    CHECK(ret > 0, lblKO);
    ret = dbaExport(3, 0xFF, 0, 3, 'B', "0123");
    CHECK(ret > 0, lblKO);
    ret = dbaExport(3, 0xFF, 2, 3, 'S', "\x45\x67\x89");
    CHECK(ret > 0, lblKO);
    ret = dbaExport(3, 0xFF, 3, 5, 'b', "6789ABCDEF");
    CHECK(ret > 0, lblKO);

    memset(volDba, 0, 256);

    ret = dspLS(3, "Import...");
    CHECK(ret >= 0, lblKO);

    ret = dbaImport(3);
    CHECK(memcmp(volDba, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8) == 0, lblKO);
    ret = dspLS(3, "Import OK");

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    trcS("tcab0150 End\n");
}
