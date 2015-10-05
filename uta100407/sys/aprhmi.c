/** \file
 * HMI functions for APR environment
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/aprhmi.c $
 *
 * $Id: aprhmi.c 2429 2009-08-13 10:18:12Z mpamintuan $
 */

#include "sys.h"
#ifdef __HMIAPR__

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include <pinpad.h>
#include <periphPrinter.h>
#include "tlv.h"
#include "tagPeriph.h"
#include <periph.h>

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcHMI))

/** Open the associated channel.
 * Should be called before calling any printing, normally before an event treatment.
 * Not needed for APR, empty
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0000.c
 */
int prtStart(void) {
    int ret;

    trcFS("%s\n", "prtStart");
    ret = 1;
    return ret;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control to another application.
 * Not needed for APR, empty
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0000.c
 */
int prtStop(void) {
    trcFS("%s\n", "prtStop");
    return 0;
}

/** Control paper presence for associated channel.
 * Should be called before printing
 * \return
 *    - negative if failure.
 * 	  - zero if paper is absent
 *    - positive if paper is present or if no information available.
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test 
 */
int prtCtrl(void) {
    int ret = 1;

    //to be tested, 'no paper' is not available in simulation mode
    tlv_t tlv;
    uint8 tlvBuffer[1024];
    uint8 paperAvail = 0;
    uint8 paperDetect = 0;

    //uint8 const* paperAvail = "\xDF\x8A\x3A" ; //TAG_PERIPH_PRINTER_PAPER_DETECT_AVAIL uint8 (True/False) [ True ]
    //uint8 const* paperDetect = "\xDF\x8A\x3B" ; //TAG_PERIPH_PRINTER_PAPER_DETECTED uint8 (True/False) [True]
    //uint8 const* paperLow = "\xDF\x8A\x3C" ; //TAG_PERIPH_PRINTER_PAPER_LEVEL_LOW uint8 (True/False) [False]

    trcFS("%s\n", "prtCtrl");
    tlv_init(&tlv, tlvBuffer, sizeof(tlvBuffer));
    ret = xx_printer_status(&tlv);

    CHECK(ret >= 0, lblKO);

    if(tlv_tag_search(&tlv, TAG_PERIPH_PRINTER_PAPER_DETECT_AVAIL) == 0)
        tlv_get_valueBytes(&tlv, 0, &paperAvail, sizeof(paperAvail));

    if(!paperAvail)
        goto lblEnd;

    if(tlv_tag_search(&tlv, TAG_PERIPH_PRINTER_PAPER_DETECTED) == 0)
        tlv_get_valueBytes(&tlv, 0, &paperDetect, sizeof(paperDetect));

    if(!paperDetect)
        goto lblNoPaper;

    goto lblEnd;

  lblNoPaper:
    trcFN("prtCtrl ret=%d\n", ret);
    ret = 0;
    trcFN("prtCtrl ret=%d\n", ret);
    goto lblEnd;
  lblKO:
    trcFN("prtCtrl ret=%d\n", ret);
    ret = -1;
  lblEnd:
    trcFN("prtCtrl ret=%d\n", ret);
    return ret;

}

/** Print string str.
 * \param str (I) String to print.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0000.c
 */
int prtS(const char *str) {
    int ret;
    char buf[prtW + 1];

    VERIFY(str);
    trcFS("prtS str=%s\n", str);

    memset(buf, 0, prtW + 1);
    if(strlen(str) <= prtW)
        strcpy(buf, str);
    else
        memcpy(buf, str, prtW);

    ret = xx_print(buf);
    CHECK(ret == TRUE, lblKO);

    return strlen(buf);
  lblKO:
    return -1;
}

/** Print string str preceded by a sequence of escape characters
 * \param esc (I) escape control characters
 * \param str (I) String to print.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0000.c
 */
int prtES(const char *esc, const char *str) {
    int ret;
    char tmp[255];

    VERIFY(str);
    VERIFY(esc);
    trcFS("prtES str=%s\n", str);

    strcpy(tmp, esc);
    strcat(tmp, str);
    ret = xx_print(tmp);
    CHECK(ret == TRUE, lblKO);
    return strlen(tmp);
  lblKO:return -1;
}

/** Print an image
 * \param img (I) image bitmap
 * \param hgt (I) image height in pixels
 * \param wdt (I) image width in bytes
 * \param ctl (I) control string, consists of two characters:
 *              - 'l'|'c'|'r': left or center or right justify
 *              - '1'|'2'|'4'|'8'|: multiplier factor
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0000.c
 */
int prtImg(const byte * img, int hgt, int wdt, const char *ctl) {
    //int ret;
    uint32 flg;

    trcS("prtImg\n");
    VERIFY(img);

    switch (ctl[0]) {
      case 'l':
          flg = PRN_AT_LEFT;
          break;
      case 'r':
          flg = PRN_AT_RIGHT;
          break;
      default:
          VERIFY(ctl[0] == 'c');
          flg = PRN_AT_CENTER;
          break;
    }
    switch (ctl[1]) {
      case '1':
          flg |= PRN_MULT1;
          break;
      case '2':
          flg |= PRN_MULT2;
          break;
      case '4':
          flg |= PRN_MULT4;
          break;
      default:
          VERIFY(ctl[1] == '8');
          flg |= PRN_MULT8;
          break;
    }

    xx_print_graphic(wdt * 8, hgt, 0, flg, img);
    //CHECK(ret==0,lblKO);
    return 1;
    //lblKO:
    //  trcErr(ret);
    //    return -1;
}

/** Open the associated channel.
 * Should be called before calling any display processing,
 * normally before an event treatment.
 * Not needed for APR, empty
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 */
int dspStart(void) {
    int ret;

    trcFS("%s\n", "dspStart");
    ret = 1;
    return ret;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control to another application.
 * Not needed for APR, empty
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 */
int dspStop(void) {
    int ret;

    trcFS("%s\n", "dspStop");
    ret = 1;
    return ret;
}

/** Clear display.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 */
int dspClear(void) {
    //int ret;
    trcFS("%s\n", "dspClear");
    pp_display_clear(0);
    pp_display_update();
    return 0;
//  lblKO:
//    return -1;
}

/** Set backlight intensity
 * \param val the backlight intensity (0=Off, 100= On)
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 */
int dspLight(byte val) {
    if(val == 0) {
        trcFS("%s\n", "dspLight OFF");
        pp_light_off();
    } else {
        trcFS("%s\n", "dspLight ON");
        pp_light_on();
    }
    return 0;
}

/** Display the string str at the location loc.
 * The string is truncated to the default display width.
 *
 * The first argument contains the line number (lowest nibble) and attribute (highest nibble).
 * Zero attribute means normal font.
 * Any combination of bold font and inverted mode can be defined using BLD and INV macros.
 * \param loc (I) Attribute and line; the macros INV, BLD, RGT and BIG can be applied to manage attributes
 * \param str (I) string to display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 */
int dspLS(int loc, const char *str) {
    int ret;
    char buf[255];
    word atr = 0;

    VERIFY(str);
    VERIFY((loc & 0x0F) < dspHgt(0));
    trcFN("dspLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    memcpy(buf, str, 255);
    buf[dspWdt(0)] = 0;         //truncate the string if it is too long
    if(loc & 0x20) {            //right justified
        byte len;

        len = (byte) strlen(buf);
        if(len < dspWdt(0)) {
            memmove(buf + dspWdt(0) - len, buf, len);
            memset(buf, ' ', dspWdt(0) - len);
        }
    }

    if(loc & 0x40)
        atr |= PT_BOLD_ON;

    if(loc & 0x10) {            //BIG: apply multipliers
        loc &= 0x0F;
        pp_display_clear(0);
        pp_display_text(loc + 2, 0, buf,
                        PT_DOUBLE_HEIGHT_ON | PT_DOUBLE_WIDTH_ON | PT_CLEAR);
        pp_display_update();
        ret = strlen(buf);
        goto lblEnd;
    }

    if(loc & 0x80) {            //reversed mode
        atr |= PT_INVERSE_ON;
    }

    loc &= 0x0F;
    pp_display_text(loc + 1, 0, buf, atr | PT_CLEAR);
    pp_display_update();
    ret = strlen(buf);
    goto lblEnd;
//  lblKO:
//    trcErr(ret);
//    return -1;
  lblEnd:
    return ret;
}

/** Display the string str at the position (x,y) in pixels
 * \param x (I) horizontal coordinate (column) in pixels
 * \param y (I) vertical coordinate (row) in pixels
 * \param font (I) font number to use
 * \param str (I) string to display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspXYFS(word x, word y, byte font, const char *str) {
    //to be implemented
    return strlen(str);
}

/** Display an image bitmap img of width wdt and height hgt at the position (x,y) in pixels
 * \param x (I) horizontal coordinate (column) in pixels
 * \param y (I) vertical coordinate (row) in pixels
 * \param wdt (I) image width in pixels
 * \param hgt (I) image height in pixels
 * \param img (I) image to display
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspImg(word x, word y, word wdt, word hgt, const byte * img) {
    int ret;

    trcFN("dspImg x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", wdt);
    trcFN(" hgt=%d\n", hgt);
    VERIFY(img);

    ret = pp_display_graphic(x, y, wdt, hgt, img);
    CHECK(ret == 0, lblKO);
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static byte _dspWdt = dspW;
static byte _dspHgt = dspH;

/** Get/set display screen width.
 * The zero value of argument means just to return the current value of screen width.
 * \param wdt new display width if not zero
 * \return previous screen width
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 * The width modification is not implemented and not recommended to be used
 */
byte dspWdt(byte wdt) {
    byte tmp = _dspWdt;

    //if(wdt)
    //    _dspWdt = wdt;
    return tmp;
}

/** Get/set display screen height.
 * The zero value of argument means just to return the current value of screen height.
 * \param hgt new display height if not zero
 * \return previous screen height
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0002.c
 * The height modification is not implemented and not recommended to be used
 */
byte dspHgt(byte hgt) {
    byte tmp = _dspHgt;

    //if(hgt)
    //_dspHgt = hgt;
    return tmp;
}

/** Start waiting for a key.
 * This function should be called when an application is ready to accept a user input.
 * The keys waiting in the buffer are flushed if fls is not zero.
 * \param fls (I) Indicates whether key flush is to be done
 * Not needed for APR, empty
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0003.c
 */
int kbdStart(byte fls) {
    //int ret;
    trcFS("%s\n", "kbdStart");
    return 1;
    //lblKO:return -1;
}

/** Stop waiting for a key. All the keys pressed after calling this function will be ignored.
 * \return non-negative value if OK; negative otherwise
 * Not needed for APR, empty
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 * \test tcab0003.c
 */
int kbdStop(void) {
    int ret;

    trcFS("%s\n", "kbdStop");
    ret = 0;
    goto lblEnd;
    //lblKO:ret = -1;
  lblEnd:
    return ret;
}

/** Retrieve a key pressed if any. Return its key code.
 * \return
 *    - Key code according to eKbd enum.
 *    - If no key pressed zero value is returned.
 *    - In case of error it returns 0xFF.
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0003.c
 */
char kbdKey(void) {
    int ret;

    ret = pp_get_key(1);
    CHECK(ret >= 0, lblKO);

    if(!ret) {
        trcS("kbdKey: TimeOut!\n");
        goto lblEnd;
    }

    switch (ret) {
      case VK_F1:
          ret = kbdF1;
          break;
      case VK_F2:
          ret = kbdF2;
          break;
      case VK_F3:
          ret = kbdF3;
          break;
      case VK_F4:
          ret = kbdF4;
          break;
      case VK_F5:
          ret = kbdF5;
          break;
      case VK_CHQ:
          break;
      case VK_SAV:
          break;
      case VK_CR:
          break;
      case VK_FEED:
          ret = kbdFWD;
          break;
      case VK_SYSTEM:
          break;
      case VK_UP:
          ret = kbdF2;
          break;
      case VK_DOWN:
          ret = kbdF1;
          break;
      case VK_FUNCTION:
          ret = kbdINI;
          break;
      case VK_CLEAR:
          ret = kbdCOR;
          break;
      case VK_ENTER:
          ret = kbdVAL;
          break;
      case VK_CANCEL:
          ret = kbdANN;
          break;
      case VK_00:
          ret = kbd00;
          break;
      case VK_F6:
          ret = kbdF6;
          break;
      case VK_F7:
          ret = kbdF7;
          break;
      case VK_F8:
          ret = kbdF8;
          break;
      case VK_F9:
          break;
      case VK_F10:
          break;
      case VK_F11:
          break;
      case VK_F12:
          break;
      case VK_0:
          break;
      case VK_1:
          break;
      case VK_2:
          break;
      case VK_3:
          break;
      case VK_4:
          break;
      case VK_5:
          break;
      case VK_6:
          break;
      case VK_7:
          break;
      case VK_8:
          break;
      case VK_9:
          break;
      default:
          break;
    }
    if(ret) {
        trcFS("%s: ", "kbdKey");
        trcFN("[%c]\n", ret);
    }
    goto lblEnd;

  lblKO:
    ret = 0xFF;
    trcS("kbdKey: KO!");
    goto lblEnd;
  lblEnd:
    return ret;
}

/** Make a beep.
 * \return no
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 */
void utaBeep(void) {
    trcFS("%s\n", "Beep");
    pp_sound(3, HMI_BEEP_CLICK, HMI_BEEP_MIDTONE);
    //to be implemented
}

/** Make a click.
 * \return no
 * \header sys\\sys.h
 * \source sys\\aprhmi.c
 */
void Click(void) {
    trcFS("%s\n", "Click");
    pp_beep(1);
}
#endif
