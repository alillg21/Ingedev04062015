#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;


//Recovery testing.

#define CHK CHECK(ret>=0,lblKO)

void tcmt0009(void){
    int ret;

    ret= dspClear(); CHECK(ret>=0,lblKO);
 
    ret= dspLS(1,"admRecovery..."); CHECK(ret>=0,lblKO);    
    admRecovery(); CHECK(ret>=0,lblKO);

    goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
}
#endif
