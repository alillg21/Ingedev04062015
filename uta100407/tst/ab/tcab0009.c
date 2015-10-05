/** \file
 *  Unitary test case for sys component: Structure sBuffer testing.
 * \sa
 *  - bufInit()
 *  - bufDim()
 *  - bufLen()
 *  - bufSet()
 *  - bufApp()
 *  - bufIns()
 *  - bufCpy()
 *  - bufReset()
 *  - bufOwr()
 *  - bufCat()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0009.c $
 *
 * $Id: tcab0009.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0009(void) {
    int ret;

    /*! TEST SEQUENCE :
     * \par Step 1: tBuffer declaration
     * sBuffer structure contains :
     * - byte * ptr, buffer containing the data
     * - word dim, number of bytes in the buffer
     * - word pos, current position
     */
    tBuffer buf;
    byte dat[1024];
    tBuffer src;
    byte srcDat[1024];

    trcS("tcab0009 Beg\n");

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "bufInit");
    CHECK(ret >= 0, lblKO);
    ret = prtS("bufInit... Size 1024");

    /*!
     * \par Step 2: call bufInit(tBuffer * buf, byte * ptr, word dim)
     * This function initialize a buffer buf of size dim;
     * the memory space pointed to by ptr should be reserved before calling this function.
     * This method should be called before using a buffer.
     */
    bufInit(&buf, dat, 1024);   //buffer initialisation

    /*!
     * \par Step 3: call bufDim(const tBuffer * buf)
     * This function get buffer dimension.
     * Return buffer dimension,i.e. the maximum number of bytes reserved in buffer memory space.
     * This value is defined during buffer initialisation.
     */
    CHECK(bufDim(&buf) == 1024, lblKO);
    /*!
     * \par Step 4: call bufLen(const tBuffer * buf)
     * This function get buffer length (position).
     * Return the number of bytes already filled.
     * The buffer length is updated by bufSet, bufIns, bufApp, bufCpy functions and can be set to zero by bufReset function.
     */
    CHECK(bufLen(&buf) == 0, lblKO);

    tmrPause(1);

    ret = dspLS(1, "bufSet");
    CHECK(ret >= 0, lblKO);
    ret = prtS("bufSet... 3*'A'");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 5: call bufSet(tBuffer * buf, byte val, word num)
     * This function set num bytes of buffer to the value val.
     * Set num bytes of the reserved memory space to the value val starting from the current position.
     * The current position is moved by num bytes.
     */
    ret = bufSet(&buf, 'A', 3); //set 3 bytes starting from the current position to the values 'A'
    CHECK(ret == 3, lblKO);
    CHECK(bufLen(&buf) == 3, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "AAA") == 0, lblKO);

    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    ret = prtS("bufSet... 2*'B'");
    CHECK(ret >= 0, lblKO);
    ret = bufSet(&buf, 'B', 2); //append 2 'B' bytes
    CHECK(ret == 5, lblKO);
    CHECK(bufLen(&buf) == 5, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "AAABB") == 0, lblKO);

    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(2, "bufApp");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 6: call bufApp(tBuffer * buf,const byte * dat, int len)
     * This function append data to the end of buffer.
     * Append len bytes to the end of the buffer and copy the content of dat into it.
     * The current position is moved by len bytes.
     * \n This function is a special case of bufIns()
     * \n In case of len equal to zero, ending zero will be written at the end
     */
    ret = bufApp(&buf, (byte *) "123", 0);  //append "123"; zero will be written at the end
    CHECK(ret == 8, lblKO);
    CHECK(bufLen(&buf) == 8, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "AAABB123") == 0, lblKO);
    ret = prtS("bufApp... \"123\"");
    CHECK(ret >= 0, lblKO);

    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    ret = prtS("bufApp... \"45\"");
    CHECK(ret >= 0, lblKO);
    ret = bufApp(&buf, (byte *) "45", 2);   //append "45"; the ending zero is not written
    CHECK(ret == 10, lblKO);
    CHECK(bufLen(&buf) == 10, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "AAABB12345") == 0, lblKO);

    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(3, "bufIns");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 7: call bufIns(tBuffer * buf, word ofs, const byte * dat, int len)
     * This function insert data at a given offset.
     * Insert len bytes before the position ofs and copy the content of dat into it.
     * The current position is moved by len bytes.
     * If len = -1, len equal to length of inserted data will be calculated and the current position will be moved by len bytes
     */
    ret = bufIns(&buf, 0, (byte *) "abc", -1);  //insert "abc" at the beginning
    CHECK(ret == 13, lblKO);
    CHECK(bufLen(&buf) == 13, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "abcAAABB12345") == 0, lblKO);
    ret = prtS("bufIns... \"abc\" -1");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    ret = bufIns(&buf, 3, (byte *) "def", 3);   //insert "def" at offset 3
    CHECK(ret == 16, lblKO);
    CHECK(bufLen(&buf) == 16, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "abcdefAAABB12345") == 0, lblKO);
    ret = prtS("bufIns... \"def\" ofs 3");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(0, "bufCpy");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 8: call bufCpy(tBuffer * buf, const byte * dat, int len)
     * This function reset buffer and copy new data into it.
     * Just call bufReset and bufApp functions.
     */
    ret = bufCpy(&buf, (byte *) "ABCD", 0); //copy "ABCD" into buffer
    CHECK(ret == 4, lblKO);
    CHECK(bufLen(&buf) == 4, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "ABCD") == 0, lblKO);
    ret = prtS("bufCpy... \"ABCD\"");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspLS(1, "bufReset");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 9: call bufReset(tBuffer * buf)
     * This function fill the buffer by zeroes and reset the current position.
     * Fill the reserved memory space by zeroes and reset the current position.
     */
    bufReset(&buf);             //fill by zeroes; reset current position
    CHECK(bufDim(&buf) == 1024, lblKO);
    CHECK(bufLen(&buf) == 0, lblKO);
    ret = prtS("bufReset...");
    tmrPause(1);

    ret = dspLS(2, "bufSet");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 10: call bufSet() with wrong parameters
     * Here the length is bigger then buffer.
     */
    trcS("Provoking an exception\n");

    ret = bufSet(&buf, 'A', 1025);  //provoke an exception
    CHECK(ret < 0, lblKO);
    ret = prtS("bufSet... 1025*'A'");
    ret = prtS("buffer overflow");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    bufReset(&buf);             // fill with zeroes ; reset current position
    ret = prtS("bufReset...");

    ret = dspLS(3, "bufCpy");
    CHECK(ret >= 0, lblKO);

    ret = bufCpy(&buf, (byte *) "0123456789", 0);   // Copy "0123456789" into buf at offset 0
    CHECK(ret == 10, lblKO);
    CHECK(bufLen(&buf) == 10, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "0123456789") == 0, lblKO);
    ret = prtS("bufCpy... \"0123456789\"");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "bufOwr");

    /*!
     * \par Step 11: call bufOwr()
     * This function overwrites data at a given offset.
     * Write len bytes at the position ofs, overwriting any existing data.
     * The current position is updated when written bytes go beyond the original position.
     * \n Call bufIns function inside.
     */
    ret = bufOwr(&buf, 4, (byte *) "AB", 2);    // Insert "AB" at offset 4, overwriting "45"
    CHECK(ret == 10, lblKO);    // buf length remains 10
    CHECK(bufLen(&buf) == 10, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "0123AB6789") == 0, lblKO);
    ret = prtS("bufOwr... \"AB\" ofs 4");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    ret = bufOwr(&buf, 8, (byte *) "CDEF", 4);  // Insert "CDEF" at offset 8, overwriting "89"
    CHECK(ret == 12, lblKO);    // buf length is updated to 12
    CHECK(bufLen(&buf) == 12, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "0123AB67CDEF") == 0, lblKO);
    CHECK(ret >= 0, lblKO);
    ret = prtS("bufOwr... \"CDEF\" ofs 8");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    bufReset(&buf);             // fill with zeroes ; reset current position
    ret = prtS("bufReset...");

    ret = dspLS(1, "bufCat");

    /*!
     * \par Step 12: call bufCat()
     * This function adds the content of buffer src into buffer buf.
     * Writes bufLen(&src) bytes at the end of buf
     * \n (bufIns function is called inside).
     */
    bufInit(&src, srcDat, 1024);    //source buffer initialisation
    ret = bufApp(&src, (byte *) "defg", 4);
    CHECK(ret == 4, lblKO);
    ret = bufApp(&buf, (byte *) "ABC", 3);
    CHECK(ret == 3, lblKO);
    ret = bufCat(&buf, &src);
    CHECK(ret == 7, lblKO);
    CHECK(strcmp((char *) bufPtr(&buf), "ABCdefg") == 0, lblKO);
    ret = prtS("bufcat... \"ABC\" \"defg\"");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
    trcS("tcab0009 End\n");
}
