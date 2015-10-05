  /** \file
 * Unitary UTA test case tcfa0006.
 * \sa
 *  - getAppVersion()
 */
 
#ifdef __TELIUM__
#include <sdk30.h>
#else
#include <string.h>
#endif

#include "sys.h"
#include "tst.h"

void tcfa0006(void) {
    int ret;
    char versionRelease[3+1]="";	
    char appName[]="SGM6YWW"; //Application name without version.
    //Family Name: ex. SGM6YWW100
    
    //It will return the app. version "three digits" which is set after the 
    //application family name in Ingedev properties or in the parameter file used for sign the application.    
    ret= getAppVer(versionRelease,appName);
    CHECK(ret >= 0, lblKO);        
    
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    
    ret = dspLS(0, "App. Version is");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, versionRelease);        

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);            
    dspStop();
}
