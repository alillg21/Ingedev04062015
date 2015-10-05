/** \file
 * Unitary test case tcab0003.
 * \sa
 * - kbdStart()
 * - kbdStop()
 * - kbdKey()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0003.c $
 *
 * $Id: tcab0003.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0003(void) {
    int ret;
    char key;                   //the last key pressed
    char buf[dspW + 1];         //buffer to accumulated keys pressed
    int idx;                    //the number of key pressed

    trcS("tcab0003 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Press a key...");
    CHECK(ret >= 0, lblKO);

    idx = 0;
    memset(buf, 0, dspW + 1);
    while(idx < dspW) {
        ret = tmrStart(1, 500);
        CHECK(ret >= 0, lblKO);

        /*! TEST SEQUENCE :
         * \par Step 1: call kbdStart(byte fls)
         * This function start waiting for a key press,
         * should be called when an application is ready to accept a user input.
         * The keys waiting in the buffer are flushed if fls is not zero.
         * - fls  (I) Indicates whether key flush is to be done
         * \n
         * If Touch screen __TFT__ is defined in context header ctx.h
         * this function request to open the keyboard for exclusive use (non-sharable).
         */
        ret = kbdStart(1);      //start keyboard waiting
        CHECK(ret >= 0, lblKO);

        key = 0;
        while(tmrGet(1)) {
            /*!
             * \par Step 2: call kbdKey()
             *  Retrieve a key pressed if any. Return its key code.
             * - Key code according to eKbd enum.
             * - If no key pressed zero value is returned.
             * - In case of error it returns 0xFF.
             */
            key = kbdKey();     //retrieve a key if pressed
            CHECK(key != '\xFF', lblKO);    //error during key waiting
            if(key)
                break;          //quit the loop if a key pressed
        }

        /*!
         * \par Step 3: call kbdStop()
         *  Stop waiting for a key. \n
         * All the keys pressed after calling this function will be ignored.
         */

        ret = kbdStop();        //stop keyboard waiting
        CHECK(ret >= 0, lblKO);
        tmrStop(1);

        buf[idx] = key;
        if(!buf[idx])
            break;              //quit the loop if nothing is pressed during 5 seconds
        if(buf[idx] == kbdANN)
            break;              //quit the loop if red key is pressed
        dspLS(1, buf);
        idx++;
    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:                      //close resource
    kbdStop();
    dspStop();
    trcS("tcab0003 End\n");
}
