/** \file
 * Unitary test case for tcgs0034.
 * Functions testing:
 * \sa
 *  - dbaExport()
 */

#include <string.h>
#include "pri.h"
#include "tst.h"


void tcgs0035(void) {
    int ret;
    byte volDba[1024];

    trcS("tcgs0035 Beg\n");

    nvmStart();                 //initialise non-volatile memory

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "dbaSetPtr...");
    CHECK(ret >= 0, lblKO);
    memset(volDba, 0, sizeof(volDba));
    dbaSetPtr(volDba);          //the buffer volDba will be used for volatile section of data base

    ret = dspLS(1, "Remove...");
    CHECK(ret >= 0, lblKO);
    nvmRemove(3);               //don't check the return code; maybe the file does not exist

    ret = dspLS(2, "Export...");
    CHECK(ret >= 0, lblKO);

    volDba[0] = 0x80;
    memset(volDba + 1, 2, volDba[0]);   //length= 128, value= "\x80\x02....
    volDba[volDba[0] + 1] = 0x39;

    ret = dbaExport(3, 0xFF, 0, volDba[0] + 1, 'B', "80020202.....");
    CHECK(ret > 0, lblKO);
    ret = dbaExport(3, 0xFF, volDba[0] + 1, 1, 'b', "39");

    ret = dspLS(2, "OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    trcS("tcgs0035 End\n");
}
