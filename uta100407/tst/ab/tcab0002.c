/** \file
 * Unitary test case tcab0002.
 * Functions testing:
 * - dspStart()
 * - dspStop()
 * - dspClear()
 * - dspLight()
 * - dspLS()
 * - dspXYFS()
 * - dspWdt()
 * - dspHgt()
 * - dspImg()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0002.c $
 *
 * $Id: tcab0002.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "sys.h"
#include "tst.h"

byte logoVisa[] = {
    0xFF, 0xFF, 0x80, 0x00, 0xBF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF,
    0xBF, 0xFF, 0xBF, 0xFF,
    0xBF, 0xFF, 0x80, 0x00, 0x81, 0xC0, 0x81, 0xC1, 0x81, 0xC3, 0x81, 0xC7,
    0x81, 0xCF, 0x81, 0xDE,

    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0xDC, 0xFC, 0xDD, 0xFC, 0xDD, 0x80, 0xBB, 0x80,
    0x3B, 0xE0, 0x7B, 0xF1,

    0xFF, 0xFE, 0x00, 0x02, 0xFF, 0xFA, 0xFF, 0xFA, 0xFF, 0xFA, 0xFF, 0xFA,
    0xFF, 0xFA, 0xFF, 0xFA,
    0xFF, 0xFA, 0x00, 0x02, 0x3C, 0xE2, 0x3D, 0xF2, 0x7C, 0xE2, 0xEC, 0x02,
    0xCC, 0x02, 0xCC, 0x02,

    0x81, 0xFC, 0x81, 0xF8, 0x81, 0xF0, 0x81, 0xE0, 0x81, 0xC0, 0x80, 0x00,
    0xBF, 0xFF, 0xBF, 0xFF,
    0xBF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF, 0x80, 0x00,
    0xFF, 0xFF, 0x00, 0x00,

    0x71, 0xF3, 0x70, 0x77, 0xEC, 0xEF, 0xEF, 0xEE, 0xEF, 0xDC, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0x00,

    0xFC, 0x02, 0xFC, 0x02, 0xFC, 0x02, 0x1C, 0x02, 0x1C, 0x02, 0x00, 0x02,
    0xFF, 0xFA, 0xFF, 0xFA,
    0xFF, 0xFA, 0xFF, 0xFA, 0xFF, 0xFA, 0xFF, 0xFA, 0xFF, 0xFA, 0x00, 0x02,
    0xFF, 0xFE, 0x00, 0x00
};

byte logoMC[] = {
    0x00, 0x00, 0xFF, 0xFF, 0x80, 0x07, 0x80, 0x7F, 0x81, 0xFF, 0x83, 0xFF,
    0x87, 0xFF, 0x87, 0xFF,
    0x9F, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF, 0xF6, 0xFF,
    0xF4, 0x89, 0xE4, 0xEB,

    0x00, 0x00, 0xFF, 0xFF, 0x80, 0x07, 0xF8, 0x38, 0xFC, 0x40, 0xFF, 0x80,
    0xFE, 0x00, 0xFF, 0xC0,
    0xFC, 0x00, 0xFF, 0xE0, 0xF8, 0x00, 0xFF, 0xF0, 0xF0, 0x00, 0xBF, 0xF7,
    0xB1, 0x5E, 0x15, 0x3C,

    0x00, 0x00, 0xFF, 0xFF, 0xFC, 0x01, 0x03, 0x01, 0x00, 0xC1, 0x00, 0x21,
    0x00, 0x11, 0x00, 0x09,
    0x00, 0x05, 0x00, 0x05, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x00, 0x19,
    0x75, 0x19, 0x16, 0x79,

    0xE0, 0x89, 0xEA, 0xAD, 0xCA, 0x89, 0xFF, 0xFF, 0xBF, 0xFF, 0xBF, 0xFF,
    0x9F, 0xFF, 0x9F, 0xFF,
    0x8F, 0xFF, 0x87, 0xFF, 0x83, 0xFF, 0x80, 0xFF, 0x80, 0x3F, 0x80, 0x0F,
    0xFF, 0xFF, 0x00, 0x00,

    0xB1, 0x7C, 0xB7, 0x7E, 0x91, 0x77, 0xFF, 0xF0, 0xF8, 0x00, 0xFF, 0xE0,
    0xFC, 0x00, 0xFF, 0xC0,
    0xFF, 0x00, 0xFF, 0x80, 0xFE, 0x80, 0xFC, 0x60, 0xF0, 0x18, 0xC0, 0x07,
    0xFF, 0xFF, 0x00, 0x00,

    0x74, 0xD9, 0x54, 0xD9, 0x74, 0x69, 0x00, 0x01, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x05, 0x00, 0x05,
    0x00, 0x09, 0x00, 0x11, 0x00, 0x21, 0x00, 0x41, 0x01, 0x81, 0xFE, 0x01,
    0xFF, 0xFF, 0x00, 0x00
};

static int test(byte wdt, byte hgt) {
    int ret;

    /*! \section Test Steps from 3 to 10
     * \par Step 3: call dspWdt(byte wdt) and dspHgt(byte hgt)
     *  dspWdt(byte wdt) : Get/set display screen width. The zero value of argument means just to return the current value of screen width.
     * dspHgt(byte hgt) : Get/set display screen height. The zero value of argument means just to return the current value of screen height.
     */
    dspWdt(wdt);
    dspHgt(hgt);

    /*!
     * \par Step 4: call dspLS(int loc, const char * str)
     *  Display the string str at the location loc. The string is truncated to the default display width.
     * The first argument contains the line number (lowest nibble) and attribute (highest nibble).
     * Zero attribute means normal font, line number begins from zero.
     * Any combination of bold font and inverted mode can be defined using BLD and INV macros.
     */
    ret = dspLS(0, "NORMAL");   //display the string "NORMAL" at the 1st line of the screen
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 5: call dspLight(byte val)
     *  this function set backlight intensity to value between 0 (off) and 100(on at maximum value)
     */
    ret = dspLight(0);          //turn backlight off
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 6: call dspLS(int loc, const char * str)
     *  With loc equal to INV(1), INV(L) return (0x80|(L)) , where L is location at screen (screen line).
     * INV macros is used to set inverted mode
     */
    ret = dspLS(INV(1), "INVERTED");    //display the string "INVERTED" at the 2nd line of the screen in inverted mode
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 7: call dspLS(int loc, const char * str)
     *  With loc equal to BLD(2), BLD(L) return (0x40|(L)) , where L is location at screen (screen line).
     * BLD macros is used to set bold font
     */
    ret = dspLS(RGT(BLD(2)), "BOLD RIGHT"); //display the string "BOLD" at the 3rd line of the screen in bold font
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 8: call dspLight(byte val)
     *  set backlight on with maximum value of intensity.
     */
    ret = dspLight(100);        //turn backlight on
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 9: call dspLS(int loc, const char * str)
     *  With loc equal to INV(BLD(3)), that gives combination of step 4 and 5
     */
    ret = dspLS(INV(BLD(3)), "INVERTED BOLD");  //display the string "INVERTED BOLD" at the 4th line of the screen in bold font in inverted mode
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 10: call dspXYFS(word x, word y, const char * str)
     *  Display the string str at the position (x,y) in pixels .
     * - x   (I) horizontal coordinate (column) in pixels
     * - y   (I) vertical coordinate (row) in pixels
     * - font (I) font number to use
     * - str (I) string to display
     */
    ret = dspXYFS(3, 5, 2, "(3,5)");    //display the string "(3,5)" upper left corner being at at the column 3 row 5 using font 2
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    //display the string "(52,47)" upper left corner being at at the column 52 row 47 using font 3
    ret = dspXYFS(52, 47, 3, "(52,47)");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 11: call dspLS(int loc, const char * str)
     *  With loc equal to BIG(1), that gives displays the characters using *2 multipliers
     */
    ret = dspLS(BIG(1), "BIG"); //display the string "BIG" at the 3th line of the screen
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    return 1;
  lblKO:
    return -1;
}

void tcab0002(void) {
    int ret;
    byte *ptr;
    byte col, row;

    trcS("tcab0002 Beg\n");

    /*! TEST SEQUENCE :
     * \par Step 1: call dspStart()
     *  Start display perypheral.
     * Open the associated channel.
     * Should be called before calling any display/keyboard processing, normally before an event treatment.
     * Inside of this fonction calls
     * - selectFontDsp , not implemented yet
     * - start touch screen if __TFT__ is defined in context header ctx.h.
     */
    ret = dspStart();           //open the associated channel
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2: call dspClear()
     *  This function clear display.
     */
    ret = dspClear();           //clear display
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Steps 3-10: called with screen width = 16 and heigth = 4
     * \ref Test
     */
    ret = test(16, 4);
    CHECK(ret >= 0, lblKO);

    ret = dspClear();           //clear display
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Steps 3-10: called with screen width = 21 and heigth = 6
     * \ref Test
     */
//    ret = test(21, 6);
//    CHECK(ret >= 0, lblKO);

    ret = dspClear();           //clear display
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ptr = logoVisa;
    for (row = 0; row < 2; row++) {
        for (col = 0; col < 3; col++) {
            ret = dspImg(col * 16, row * 16, 16, 16, ptr);
            CHECK(ret >= 0, lblKO);
            ptr += 32;
        }
    }
    tmrPause(3);

    ptr = logoMC;
    for (row = 0; row < 2; row++) {
        for (col = 0; col < 3; col++) {
            ret = dspImg(64 + col * 16, row * 16, 16, 16, ptr);
            CHECK(ret >= 0, lblKO);
            ptr += 32;
        }
    }
    tmrPause(3);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    tmrPause(3);
  lblEnd:
    dspWdt(16);
    dspHgt(4);
    /*!
     * \par Step 11: call dspStop()
     *  Stop display perypheral.
     * Close the associated channel.
     * Should be called before returning to the idle prompt or before giving the control to another application.
     */
    dspStop();                  //close resource
    trcS("tcab0002 End\n");
}
