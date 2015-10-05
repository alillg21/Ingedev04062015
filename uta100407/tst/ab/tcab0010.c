/** \file
 * Unitary test case for sys component: Structure sQueue testing.
 * \sa
 *  - queInit()
 *  - queLen()
 *  - quePut()
 *  - queNxt()
 *  - queGet()
 *  - queRewind()
 *  - queReset()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0010.c $
 *
 * $Id: tcab0010.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0010(void) {
    int ret;
    tBuffer buf;

    /*! TEST SEQUENCE :
     * \par Step 1: tQueue declaration
     * sQueue structure contains :
     * - tBuffer * buf, buffer containing the queue elements
     * - word cur, current read position
     * - word put , number of elements put (written)
     * - word get , number of elements to get (written and not read yet)
     */
    tQueue que;
    byte dat[1024];
    byte get[prtW + 1];

    trcS("tcab0010 Beg\n");

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    dspLS(0, "bufInit");
    prtS("bufInit... Size 1024");
    bufInit(&buf, dat, 1024);   //buffer initialisation

    /*!
     * \par Step 2: call queInit(tQueue * que, tBuffer * buf )
     * This function initialize a queue. Initialize a queue que linked to the buffer buf.
     * The buffer will be used to store all the elements of the queue and their lengths.
     */
    dspLS(0, "queInit");
    prtS("queInit... link to buf");
    queInit(&que, &buf);        //queue initialisation

    /*!
     * \par Step 3: call queLen(const tQueue * que)
     * This function get queue length (number of elements not rtetrieved yet).
     * Return the number of elements in the queue that are saved but not retrieved yet.
     */
    CHECK(queLen(&que) == 0, lblKO);
    tmrPause(1);

    dspLS(1, "quePut");
    /*!
     * \par Step 4: call quePut(const tQueue * que, const byte * dat, int len)
     * This function put a data element into a queue.
     * Put an element of len bytes pointed to by dat in the queue que.
     * The data are copied into the queue.
     * The last parameter len can have a special value len==0;
     * it means that the number of bytes should be calculated as strlen(dat).
     * Number of put elements is incremented
     */
    prtS("quePut... \"ONE\"");
    ret = quePut(&que, (byte *) "ONE", 0);  //the first item included "ONE"
    CHECK(ret == 3, lblKO);
    CHECK(queLen(&que) == 1, lblKO);

    prtS("quePut... \"TWO\"");
    ret = quePut(&que, (byte *) "TWO", 0);  //"TWO" is the second one
    CHECK(ret == 3, lblKO);
    CHECK(queLen(&que) == 2, lblKO);

    prtS("quePut... \"THREE\"");
    ret = quePut(&que, (byte *) "THREE", 0);    //add one more item
    CHECK(ret == 5, lblKO);
    CHECK(queLen(&que) == 3, lblKO);

    trcS("que:\n");
    trcQ(&que);
    tmrPause(1);

    dspLS(2, "queGet");
    /*!
     * \par Step 5: call queNxt(const tQueue * que)
     * This function get the size of the next data element in the queue.
     * Get the number of bytes in the next element of queue to be retrieved.
     */
    CHECK(queNxt(&que) == 3, lblKO);    //the length of first item "ONE" is 3

    /*!
     * \par Step 6: call queGet(tQueue * que, byte * dat)
     * This function retrieve a data element from the queue.
     * Retrieve an element from the queue que into the buffer pointed by dat.
     * It is assumed that there is enought space in the destination buffer.
     * The size of the date retrieved from the que following the FIFO principle -First In First Out.
     */
    prtS("queGet...");
    ret = queGet(&que, get);    //retrieve the first item
    CHECK(ret == 3, lblKO);
    CHECK(queLen(&que) == 2, lblKO);
    get[ret] = 0;
    prtS((char *) get);
    CHECK(strcmp((char *) get, "ONE") == 0, lblKO);

    prtS("queGet...");
    CHECK(queNxt(&que) == 3, lblKO);    //the length of next item "TWO" is 3
    ret = queGet(&que, get);    //retrieve the second item
    CHECK(ret == 3, lblKO);
    CHECK(queLen(&que) == 1, lblKO);
    get[ret] = 0;
    prtS((char *) get);
    CHECK(strcmp((char *) get, "TWO") == 0, lblKO);

    prtS("queGet...");
    CHECK(queNxt(&que) == 5, lblKO);    //the length of next item "THREE" is 5
    ret = queGet(&que, get);    //retrieve the third item
    CHECK(ret == 5, lblKO);
    CHECK(queLen(&que) == 0, lblKO);
    get[ret] = 0;
    prtS((char *) get);
    CHECK(strcmp((char *) get, "THREE") == 0, lblKO);
    tmrPause(1);

    dspLS(3, "queRewind");
    /*!
     * \par Step 7: call queRewind(tQueue * que)
     * This function put the current position to the beginning of the queue.
     * Move the reading head of the queue to the begining.
     * After calling this function the next element to read will be the first element saved
     */
    prtS("queRewind...");
    queRewind(&que);
    CHECK(queNxt(&que) == 3, lblKO);    //the length of first item "ONE" is 3
    prtS("queGet...");
    ret = queGet(&que, get);    //retrieve the first item
    CHECK(ret == 3, lblKO);
    CHECK(queLen(&que) == 2, lblKO);
    get[ret] = 0;
    prtS((char *) get);
    CHECK(strcmp((char *) get, "ONE") == 0, lblKO);

    dspLS(0, "queReset");
    /*!
     * \par Step 8: call queReset(tQueue * que)
     * This function empty the queue que.
     */
    prtS("queReset...");
    queReset(&que);
    CHECK(queLen(&que) == 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();                  //close resources
    trcS("tcab0010 End\n");
}
