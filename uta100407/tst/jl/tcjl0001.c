#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcjl0001.
//Functions testing:
//  getAppName
//  reboot

void testReboot() {
    int ret;                    //*< integer value for return code */

    //close any resource used
    prtStop();
    dspStop();

    ret = dspClear();           //clear display
    CHECK(ret >= 0, lblKO);

    trcS("Reboot\n");
    ret = dspLS(3, "Restarting.. ");
    CHECK(ret >= 0, lblKO);
    ret = prtS("Reboot");
    CHECK(ret >= 0, lblKO);
    tmrPause(5);
    reboot();
    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    prtStop();                  //close resource
    dspStop();
    trcS("tcjl0002 End\n");

}

void tcjl0001(void) {

    int ret;                    //*< integer value for return code */
    char name[20 + 1];

    trcS("tcjl0001 Beg\n");

    //open the associated channel
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    trcS("Get App Info\n");
    ret = dspLS(0, "Getting ");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "appli name");
    CHECK(ret >= 0, lblKO);

    memset(&name, 0, sizeof(name));
    //get apllication name
    ret = getAppName(name);
    CHECK(ret >= 0, lblKO);

    //present results
    ret = prtS("Application name:");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) name);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, (char *) name);  //display
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    //close resource
    prtStop();
    dspStop();
    // test reboot function
    testReboot();
    trcS("tcjl0001 End\n");
}
