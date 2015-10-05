#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcje0030.
//Functions testing:
//  Beep
//  Click

void tcje0030(void) {
    
    trcS("tcje0030 Beg\n");

    /*!
     * \par Step 1: call Beep()
     *  Make a beep. \n
     * Make a long sound.
     */
    Beep();   
    tmrPause(1);

    /*!
     * \par Step 2: call Click()
     *  Make a click. \n
     * Make a short sound.
     */
    Click();    
	
    trcS("tcje0030 End\n");
    
}
