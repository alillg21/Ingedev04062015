/** \file
 * Unitary test case for sys component: DFS memory processing.
 * \sa
 *  - nvmStart()
 *  - nvmSave()
 *  - nvmSet()
 *  - nvmLoad()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0005.c $
 *
 * $Id: tcab0005.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0005(void) {
    int ret;
    byte bin[256];
    char chr[256];
    word ofs;
    struct {
        byte b;
        word w;
        char s[1];
        card c;
    } dat;

    trcS("tcab0005 Beg\n");

    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    /*! TEST SEQUENCE :
     * \par Step 1: call nvmStart()
     *  This function initialize non-volatile memory internal variables :
     * - DFS file handle if holded otherwise 0
     * - section number holded or 0xFF if not hold.
     * Should be called before calling any non-volatile memory treatment, normally during POWERON event.
     */
    ret = dspLS(0, "nvmStart...");
    CHECK(ret >= 0, lblKO);
    nvmStart();                 //initialise non-volatile memory
    ret = prtS("nvmStart...");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2: call nvmSet(byte sec, unsigned char val, card ofs, card len)
     * This function fill len bytes by value val into section sec starting from the position ofs.
     * - sec  (I) Page number, starts from zero to __NDATS__ for 16-bit or to 9999 for 32-bit
     * - val  (I) Value to fill
     * - ofs  (I) Starting offset within the section; 0XFFFFFFFFL meaning append to the end of file
     * - len  (I) Number of bytes to be filled
     */
    ret = dspLS(1, "nvmSet...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("nvmSet... 32*'X' ofs 0");
    CHECK(ret >= 0, lblKO);
    ret = nvmSet(0, 'X', 0, 32);    //set 32 byte of nvm to 'X'
    CHECK(ret == 32, lblKO);

    /*!
     * \par Step 3: call nvmSave(byte sec, const void * buf, card ofs, word len)
     * This function save len bytes from buffer buf into section sec starting from the position ofs.
     * - sec  (I) Section number, starts from zero to __NDATS__ for 16-bit or to 99 for 32-bit
     * - buf  (I) Pointer to a buffer to be saved
     * - ofs  (I) Starting offset within the section; 0XFFFFFFFFL meaning append to the end of file
     * - len  (I) Number of bytes to be saved
     */
    ret = dspLS(2, "nvmSave...");
    CHECK(ret >= 0, lblKO);
    dat.b = 0xAB;
    dat.w = 0x1234;
    memcpy(dat.s, "A", 1);
    dat.c = 0x1234ABCDL;
    ofs = 0;
    ret = prtS("Data structure:");
    CHECK(ret >= 0, lblKO);
    ret = prtS("b = 0xAB");
    CHECK(ret >= 0, lblKO);
    ret = prtS("w = 0x1234");
    CHECK(ret >= 0, lblKO);
    ret = prtS("s = \"A\"");
    CHECK(ret >= 0, lblKO);
    ret = prtS("c = 0x1234ABCD");
    CHECK(ret >= 0, lblKO);
    ret = prtS("nvmSave... data ofs 0");
    CHECK(ret >= 0, lblKO);

    ret = nvmSave(0, &dat, ofs, sizeof(dat));   //save the structure into nvm
    CHECK(ret == sizeof(dat), lblKO);
    ofs += ret;

    /*!
     * \par Step 4: call nvmLoad(byte sec, void * buf, card ofs, word len)
     * This function load len bytes into buffer buf from section sec starting from the position ofs.
     * - sec  (I) Section number, starts from zero to __NDATS__ for 16-bit or to 99 for 32-bit
     * - buf  (O) Pointer to a buffer to be loaded
     * - ofs  (I) Starting offset within the section
     * - len  (I) Number of bytes to be loaded
     */
    ret = dspLS(3, "nvmLoad...");
    CHECK(ret >= 0, lblKO);
    ret = prtS("nvmLoad... file ofs 0");
    CHECK(ret >= 0, lblKO);
    ret = nvmLoad(0, bin, 0, ofs);  //load the structure from nvm to bin
    CHECK(ret == ofs, lblKO);

    /*!
     * \par Step 5: Print result
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>   <td>Printed lines</td>   <td>In Simulation mode</td>                    <td>In Terminal</td></tr>
     <tr>   <td>line 1</td>            <td>ABC8341241C81110CDAB3412</td>        <td>AB00341241000000CDAB3412</td></tr>
     <tr>   <td>line 2</td>            <td>585858585858585858585858</td>        <td>585858585858585858585858</td></tr>
     </table>
     \endhtmlonly
     * \n
     * The first line printed in Terminal consists of: \n
     * - AB: it is the byte value dat.b= 0xAB
     * - 00: it is an alignement gap between dat.b and dat.w
     * - 3412: it is the word value dat.w= 0x1234
     * - 41: it is the character value dat.s= "A"
     * - 00: it is an alignement gap between dat.s and dat.c
     * - CDAB3412: it is the card value dat.c= 0x1234ABCDL
     * \n Note that the byte order in word and card values is reversed and that the structure is aligned, there are alignment gaps.
     * \n In Simulation mode printed result is diffrent. Alignement gap between written values is not erased by 00, but conserve original value.
     * \n File debug.trc contain :
     * \n prtStart
     * \n nvmStart
     * \n nvmSet: sec=0 ofs=0 val=58 len=32
     * \n nvmSave: sec=0 ofs=0 buf=AB C8 34 12 41 C8 11 10 CD AB 34 12  [??4?A?????4?]
     * \n nvmLoad: sec=0 ofs=0 buf=AB C8 34 12 41 C8 11 10 CD AB 34 12  [??4?A?????4?]
     * \n prtS str=ABC8341241C81110CDAB3412
     * \n nvmLoad: sec=0 ofs=12 buf=58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58 58  [XXXXXXXXXXXXXXXXXXXX]
     * \n prtS str=5858585858585858585858585858585858585858
     * \n prtS str=NVM processing OK
     * \n prtStop
     */

    ret = prtS("Print file");
    CHECK(ret >= 0, lblKO);
    ret = bin2hex(chr, bin, (byte) ofs);
    prtS(chr);

    ret = nvmLoad(0, bin, ofs, (byte) (32 - ofs));  //load the rest of data
    CHECK(ret == 32 - ofs, lblKO);

    ret = bin2hex(chr, bin, (byte) (32 - ofs));
    prtS(chr);

    prtS("NVM processing OK");
    /*!
     * \par Step 6: Memory check
     * After end of test check with Terminal Workshop nvm file page0000.nvm at Terminal memory
     * \image html tcab0005_1.JPG
     */
    goto lblEnd;

  lblKO:
    trcErr(ret);
    prtS("KO!");
  lblEnd:
    dspStop();
    prtStop();
    trcS("tcab0005 End\n");
}
