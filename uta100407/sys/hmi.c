/** \file
 * UNICAPT HMI (Human-Machine Interface) implementation
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/hmi.c $
 *
 * $Id: hmi.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "sys.h"
#ifdef __HMISTD__

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include <hmi.h>

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcHMI))

#ifdef __FONT__
static ssaFontDescr_t ssaFntDsc;

static int lafStart(void) {
    int ret;
    static int laf = 0;

    if(!laf) {
        ret = ssaSecLafWriteFont(ssaHandle(), TERM, __FONT__, &ssaFntDsc);
        trcFN("lafStart:ssaFntDsc=%d\n", ssaFntDsc.hmiBoldSlot);
        trcFN(" %d\n", ssaFntDsc.hmiNormSlot);
        trcFN("lafStart:ssaSecLafWriteFont=%d\n", ret);
        if(ret == ERR_FONT_ALREADY_PRESENT) {   // another application already loaded the font
            trcS("lafStart:ERR_FONT_ALREADY_PRESENT\n");
#ifdef __FARSI__
            ssaFntDsc.hmiNormSlot = 1;
            ssaFntDsc.hmiBoldSlot = 1;
#else
            ssaFntDsc.hmiNormSlot = 1;
            ssaFntDsc.hmiBoldSlot = 2;
#endif
            laf++;
            return 0;
        }
        CHECK(ret == RET_OK, lblKO);
        CHECK(ssaFntDsc.hmiNormSlot, lblKO);
        CHECK(ssaFntDsc.hmiBoldSlot, lblKO);
        laf++;

    }

    return 1;
  lblKO:
    return -1;
}

static int selectFontDsp(void) {
    int ret;

    if(!ssaStarted()) {
        ret = cryStart('M');
        CHECK(ret >= 0, lblKO);
    }
    VERIFY(ssaStarted());

    ret = lafStart();
    CHECK(ret >= 0, lblKO);
    CHECK(ret > 0, lblOK);      // another application already loaded the font

    ret = ssaSecLafSelectDispDefaultFont(ssaHandle(), TERM,
                                         HMI_INTERNAL_FONT_RAM(ssaFntDsc.
                                                               hmiNormSlot));
    CHECK(ret == RET_OK, lblKO);
  lblOK:
    return 1;
  lblKO:
    return -1;
}

static int selectFontPrt(void) {
    int ret;

    if(!ssaStarted()) {
        ret = cryStart('M');
        CHECK(ret >= 0, lblKO);
    }
    VERIFY(ssaStarted());

    ret = lafStart();
    CHECK(ret >= 0, lblKO);
    CHECK(ret > 0, lblOK);      // another application already loaded the font

    ret = ssaSecLafSelectPrnDefaultFont(ssaHandle(), TERM,
                                        HMI_INTERNAL_FONT_RAM(ssaFntDsc.
                                                              hmiNormSlot),
                                        HMI_INTERNAL_FONT_RAM(ssaFntDsc.
                                                              hmiBoldSlot));
    CHECK(ret == RET_OK, lblKO);

  lblOK:
    return 1;
  lblKO:
    return -1;
}
#else
static int selectFontDsp(void) {
    return 0;
}
static int selectFontPrt(void) {
    return 0;
}
#endif

static uint32 hPrt = 0;

/** Open the associated channel.
 * Should be called before calling any printing, normally before an event treatment.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0000.c
 */
int prtStart(void) {
    int ret;

    trcFS("%s\n", "prtStart");
    ret = selectFontPrt();
    trcFN("selectFontPrt: %d\n", ret);
    CHECK(ret >= 0, lblKO);

    ret = prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:return -1;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control
 * to another application.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0000.c
 */
int prtStop(void) {
    int ret;

    trcFS("%s\n", "prtStop");
    ret = prnClose(hPrt);
    trcFN("prnClose: %d\n", ret);
    //CHECK(ret==RET_OK,lblKO);

    hPrt = 0;
    return hPrt;
    //lblKO:
    //    return -1;
}

/** Control paper presence for associated channel.
 * Should be called before printing
 * \return
 *    - negative if failure.
 * 	  - zero if paper is absent
 *    - positif if paper is present or if no information available.
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcik0071.c
 */
int prtCtrl(void) {
    int ret;
    prnControlReq_t PtReq;
    prnControlRes_t PtRes;

    trcFS("%s\n", "prtCtrl");
    PtReq.type = PRN_CTRL_STATUS_PAPER;
    PtRes.type = PRN_CTRL_STATUS_PAPER;
    ret = prnControl("DEFAULT", &PtReq, &PtRes);
    CHECK(ret == RET_OK, lblKO);
    switch (PtRes.Res.StatusPaper) {
      case PRN_CTRL_PAPER_DETECT:
          break;
      case PRN_CTRL_PAPER_NOT_DETECT:
          return 0;
      case PRN_CTRL_PAPER_DETECT_UNAVAILABLE:
      default:
          break;
    }
    return 1;
  lblKO:return -1;
}

/** Print string str.
 * The string is truncated to the default printer width.
 * Unicapt32 control characters are acceptable.
 * The size control character of normal size is "\\x1b" in UTA (it doesn’t exist in unicapt32).
 * For example in PRT_42COLUMN_SIZE mode the "\\x1b\\x1b""String" will be printed in normal size.
 * \param str (I) String to print.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcgs0005.c
 */
int prtS(const char *str) {
    int ret;
    int idx;
    char preChar;
    byte maxLength = prtW;      //maximum number of printable character
    byte numberNorm = 0;        //number of normal characters (only they are printed)
    char buf[48 + 1 + 40];      //40 control character enabled in half size mode

    VERIFY(str);
    trcFS("prtS str=%s\n", str);

    memcpy(buf, str, sizeof(buf));

    //first two characters must be the size control character
    if(buf[0] == 0x1b && buf[1] >= 0x1b && buf[1] <= 0x21 && strlen(buf) > 1) {
        switch (buf[1]) {
          case 0x1c:
          case 0x1e:
              maxLength = 12;
              break;            //double size
          case 0x1f:
              maxLength = 6;
              break;            //quarddouble size
          case 0x20:
              maxLength = 48;
              break;            //half size (48 columns)
          case 0x21:
              maxLength = 42;
              break;            //42 columns
          case 0x1d:
              maxLength = 24;
              break;            //24 columns
          case 0x1b:           //normal size (it may be needed when the default isn't normal)
          case 0x03:           //not processed on unicapt 
              maxLength = 24;
              for (idx = 0; idx < sizeof(buf) - 2; idx++)
                  buf[idx] = buf[idx + 2];
              break;
        }
    } else {
        switch (prtW) {
          case 6:
              preChar = 0x1f;
              break;
          case 12:
              preChar = 0x1e;
              break;
          case 24:
              preChar = 0;
              break;
          case 42:
              preChar = 0x21;
              break;
          case 48:
              preChar = 0x20;
              break;
          default:
              goto lblKO;
              //unexpected printing default size
        }
        maxLength = prtW;
        if(preChar) {
            for (idx = sizeof(buf) - 1; idx >= 2; idx--)
                buf[idx] = buf[idx - 2];
            buf[0] = 0x1b;
            buf[1] = preChar;
        }
    }

    for (idx = 0; numberNorm < maxLength && buf[idx] != 0 && idx < sizeof(buf);
         idx++) {
        if(buf[idx] < 0x17 || buf[idx] > 0x21)
            numberNorm++;
        // 0x20, 0x21 and 0x22 can be control and normal character as well
        if(buf[idx] == 0x20 || buf[idx] == 0x21 || buf[idx] == 0x22) {
            if(idx != 0) {
                if(buf[idx - 1] != 0x1b)
                    numberNorm++;
            } else
                numberNorm++;
        }
    }

    memset(buf + idx, 0, sizeof(buf) - idx);
    memset(buf + sizeof(buf) - 1, 0, 1);    //turncate the last character always

    ret = prnPrint(hPrt, buf);
    CHECK(ret == RET_OK, lblKO);

    return strlen(buf);
  lblKO:return -1;
}

/** Print string str preceded by a sequence of escape characters
 * \param esc (I) escape control characters
 * \param str (I) String to print.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\hmi.c
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
    ret = prnPrint(hPrt, tmp);
    CHECK(ret == RET_OK, lblKO);
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
 * \source sys\\hmi.c
 * \test tcab0000.c
 */
int prtImg(const byte * img, int hgt, int wdt, const char *ctl) {
    int ret;
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

    ret = prnImagePrint(hPrt, (uint8 *) img, hgt, wdt, 0, flg);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:trcErr(ret);
    return -1;
}

static uint32 hHmi = 0;
static byte _dspWdt = dspW;
static byte _dspHgt = dspH;

#ifdef __TFT__
static uint32 hKbd = 0;
#else
#define hKbd hHmi
#endif

/** Get/set display screen width.
 * The zero value of argument means just to return the current value of screen width.
 * \param wdt new display width if not zero
 * \return previous screen width
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
byte dspWdt(byte wdt) {
    byte tmp = _dspWdt;

    if(wdt)
        _dspWdt = wdt;
    return tmp;
}

/** Get/set display screen height.
 * The zero value of argument means just to return the current value of screen height.
 * \param hgt new display height if not zero
 * \return previous screen height
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
byte dspHgt(byte hgt) {
    byte tmp = _dspHgt;

    if(hgt)
        _dspHgt = hgt;
    return tmp;
}

static byte hmiStarted = 0;

/** Open the associated channel.
 * Should be called before calling any display/keyboard processing,
 * normally before an event treatment.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspStart(void) {
    int ret;

    trcFS("%s\n", "dspStart");

    ret = selectFontDsp();
    CHECK(ret >= 0, lblKO);

#ifdef __TFT__
    ret = hmiOpenShared("DEFAULT", &hHmi, HMI_CH_DISPLAY | HMI_CH_TOUCH, 0);
    CHECK(ret == RET_OK, lblKO);
    ret = hmiOpenShared("DEFAULT", &hKbd, HMI_CH_KEYBOARD, 0);
#else
    ret = hmiOpen("DEFAULT", &hHmi);
#endif
    CHECK(ret == RET_OK, lblKO);

    //dspLight(100);
    hmiStarted = 1;
    return hHmi;
  lblKO:trcErr(ret);
    return -1;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control
 * to another application.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspStop(void) {
    int ret;

    trcFS("%s\n", "dspStop");

    ret = hmiClose(hHmi);
#ifdef __TFT__
    ret = hmiClose(hKbd);
#endif
    //CHECK(ret==RET_OK,lblKO);

    hHmi = 0;
    hmiStarted = 0;
    return hHmi;
    //lblKO:
    //    return -1;
}

/** Clear display.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspClear(void) {
    int ret;

    trcFS("%s\n", "dspClear");

    if(!hmiStarted) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }

    ret = hmiADClearLine(hHmi, HMI_ALL_LINES);
    CHECK(ret == RET_OK, lblKO);

    return hHmi;
  lblKO:return -1;
}

/** Set backlight intensity
 * \param val the backlight intensity (0=Off, 100= On)
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspLight(byte val) {
    int ret;
    hmiOutputConfig_t cfg;

    trcFN("dspLight val=%d\n", val);

    //read the configuration to modify it
    ret = hmiOutputConfigRead(hHmi, &cfg);
    CHECK(ret == RET_OK, lblKO);

    //turn on backlighting
    cfg.backlight = val;
    cfg.fields = HMI_OUTPUT_CONFIG_BACKLIGHT;
    ret = hmiOutputConfigWrite(hHmi, &cfg);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:trcErr(ret);
    return -1;
}

#ifdef __FONT__
static byte isUpper(char mod) {
    if(mod < 'A')
        return 0;
    if(mod > 'Z')
        return 0;
    return 1;
}
#endif

static void *normalFont(void) {
#ifdef __FONT__
    if(!isUpper((char) ssaStarted()))
        return HMI_INTERNAL_FONT_1;
    return HMI_INTERNAL_FONT_RAM(ssaFntDsc.hmiNormSlot);
#else
    //if(dspHgt(0) > 4)
    //    return HMI_INTERNAL_FONT_3;
    return HMI_INTERNAL_FONT_1;
#endif
}

static void *boldFont(void) {
#ifdef __FONT__
    if(!isUpper((char) ssaStarted()))
        return HMI_INTERNAL_FONT_2;
    return HMI_INTERNAL_FONT_RAM(ssaFntDsc.hmiBoldSlot);
#else
    //if(dspHgt(0) > 4)
    //    return HMI_INTERNAL_FONT_5;
    return HMI_INTERNAL_FONT_2;
#endif
}

static void *Font(byte font) {
#ifdef __FONT__
    card tmp = font;

    if(!isUpper((char) ssaStarted()))
        return (void *) tmp;
    return HMI_INTERNAL_FONT_RAM((void *) tmp);
#else
    //if(dspHgt(0) > 4)
    //    return HMI_INTERNAL_FONT_3;
    return (void *) ((card) font);
#endif
}

static int setFont(void *ptr) {
#ifdef __FONT__
    //return ssaSecLafSelectDispDefaultFont(ssaHandle(), TERM, ptr);
    return hmiADFont(hHmi, ptr);
#else
    return hmiADFont(hHmi, ptr);
#endif
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
 * \source sys\\hmi.c
 * \test tcab0002.c
 */
int dspLS(int loc, const char *str) {
    int ret;
    char buf[255];

    VERIFY(str);
    VERIFY((loc & 0x0F) < dspHgt(0));
    trcFN("dspLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    if(!hmiStarted) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }

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
#ifdef __TFT__
    if(loc & 0x80) {
        loc &= 0x7F;
        loc |= 0x40;
    }
#endif
    if(loc & 0x40) {
        ret = setFont(boldFont());
    } else
        ret = setFont(normalFont());
    CHECK(ret == RET_OK, lblKO);

    if(loc & 0x10) {            //BIG: apply multipliers
        ret = hmiADMultiplierSet(hHmi, 2, 2);
        CHECK(ret == RET_OK, lblKO);
    }

    if(loc & 0x80) {            //reversed mode
        hmiOutputConfig_t cfg;

        //read the configuration to modify it
        ret = hmiOutputConfigRead(hHmi, &cfg);
        CHECK(ret == RET_OK, lblKO);

        //configure reversed mode
        ret = OpSetForeground(cfg.wgraphicContext, clrBgd);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
        ret = OpSetBackground(cfg.wgraphicContext, clrFgd);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);

        //clear line and display text
        loc &= 0x0F;
        ret = hmiADClearLine(hHmi, (uint16) loc);
        CHECK(ret == RET_OK, lblKO);
        ret = hmiADDisplayText(hHmi, loc, 0, buf);
        CHECK(ret == RET_OK, lblKO);

        //restore default normal mode
        ret = OpSetForeground(cfg.wgraphicContext, clrFgd);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
        ret = OpSetBackground(cfg.wgraphicContext, clrBgd);
        CHECK(ret == OPTIC_NO_ERROR, lblKO);
    } else {
        loc &= 0x0F;
        //just clear line and display text
        ret = hmiADClearLine(hHmi, (uint16) loc);
        CHECK(ret == RET_OK, lblKO);
        ret = hmiADDisplayText(hHmi, loc, 0, buf);
        CHECK(ret == RET_OK, lblKO);
    }

    //restore normal font
    ret = hmiADFont(hHmi, HMI_INTERNAL_FONT_1);
    CHECK(ret == RET_OK, lblKO);

    ret = strlen(buf);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;
  lblEnd:if(loc & 0x10) {      //BIG: restore multipliers
        hmiADMultiplierSet(hHmi, 1, 1);
    }

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
    int ret;

    trcFN("dspXYS X=%d", x);
    trcFN(" y=%d ", y);
    trcFS(" str=%s\n", str);

    if(!hmiStarted) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }

    ret = hmiADFont(hHmi, Font(font));
    CHECK(ret == RET_OK, lblKO);

    ret = hmiADDisplayText(hHmi, y | HMI_IN_PIXELS, x | HMI_IN_PIXELS,
                           (char *) str);
    CHECK(ret == RET_OK, lblKO);

    ret = strlen(str);
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    hmiADFont(hHmi, normalFont());
    return ret;
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

    VERIFY(img);
    ret = hmiGraphicBLTWrite(hHmi,  // handle: (I) handle of the channel
                             x, //dst_x: (I) Destination x axis pixel location. 
                             y, //dst_y: (I) Destination y axis pixel location
                             (byte *) img,  //src_ptr: (I) Pointer to the start of the source bitmap
                             wdt,   //src_width: (I) Width of total source bitmap in pixels
                             hgt,   //src_height: (I) Height of total source bitmap in pixels
                             0, //src_x: (I) X position within bitmap that is to be BLT to the destination
                             0, //src_y: (I) Y position within bitmap that is to be BLT to the destination
                             wdt,   //cpy_width: (I) Width from src_x that is to be copied to destination
                             hgt,   //cpy_height: (I) Height from src_y that is to be copied to destination
                             HMI_BLT_LOG_OP_MAX,    //log_operator: (I) Operation that is to be performed on the source pixel and destination pixel pior to writing to the destination
                             TRUE   //update (I) An indication to write through to the display or not. TRUE indicates write through to the display, while FALSE indicates that only a write to the canvas should occur.
        );
    CHECK(ret == RET_OK, lblKO);
    return 1;
  lblKO:
    return -1;
}

/** Start waiting for a key.
 * This function should be called when an application is ready to accept a user input.
 * The keys waiting in the buffer are flushed if fls is not zero.
 * \param fls (I) Indicates whether key flush is to be done
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0003.c
 */
int kbdStart(byte fls) {
    int ret;

    trcFS("%s\n", "kbdStart");

    //#ifdef __TFT__
    //    ret = hmiOpenShared("DEFAULT", &hKbd, HMI_CH_KEYBOARD, 0);
    //    CHECK(ret == RET_OK, lblKO);
    //#endif

    ret = hmiKeyReq(hKbd);
    CHECK(ret == RET_RUNNING, lblKO);
    if(fls) {
        while(1) {              //flush input buffer
            ret = kbdKey();
            if(!ret)
                break;
            if(ret == 0xFF)
                break;
            if(ret == -1)
                break;
            ret = hmiKeyReq(hKbd);
            CHECK(ret == RET_RUNNING, lblKO);
        }
    }

    return 1;
  lblKO:return -1;
}

/** Stop waiting for a key. All the keys pressed after calling this function will be ignored.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0003.c
 */
int kbdStop(void) {
    int ret;

    trcFS("%s\n", "kbdStop");

    ret = hmiCancel(hKbd);
    CHECK(ret == RET_OK, lblKO);

    ret = 0;
    goto lblEnd;
  lblKO:ret = -1;
  lblEnd:
    //#ifdef __TFT__
    //    hmiClose(hKbd);
    //#endif
    return ret;
}

#define RSP_LEN 1024
typedef struct {
    int16 sta;
    uint16 len;
    char buf[RSP_LEN];
} tResponse;

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
    tResponse rsp;

    memset(&rsp, 0, sizeof(rsp));
    ret = hmiResultGet(hKbd, sizeof(rsp), &rsp);
    switch (ret) {
      case RET_RUNNING:
          ret = 0;
          break;                //nothing is pressed until now
      case RET_OK:
          ret = rsp.buf[0];
          break;                //a key is pressed, return it
      default:
          ret = '\xFF';
          break;                //an error encountered
    }
    if(ret) {
        trcFS("%s: ", "kbdKey");
        trcFN("[%c]\n", ret);
    }
    return ret;
}

static card tftLoc = 0xFFFFFFFFL;
static char tftSta;
card tftLocation(void) {
    return tftLoc;
}
char tftState(void) {
    return tftSta;
}

/** Start waiting for a touch.
 * This function should be called when an application is ready to accept a user input.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0138.c
 */
int tftStart(void) {
    int ret;
    hmiTouchConfig_t cfg;

    trcFS("%s\n", "tftStart");

    //hmiTouchFlush(hHmi);

    ret = hmiTouchConfigRead(hHmi, &cfg);
    CHECK(ret == RET_OK, lblKO);

    cfg.touchMode = HMI_TOUCH_MODE_CONT;
    cfg.samplesPerSecond = 10;
    ret = hmiTouchConfigWrite(hHmi, &cfg);
    //    CHECK(ret == RET_OK, lblKO); returns always HMI_ERR_READ_ONLY (one of elements cannot be written)

    ret = hmiTouchAutoCoordReq(hHmi);
    //ret = hmiTouchCoordReq(hHmi);
    CHECK(ret == RET_RUNNING, lblKO);

    return 1;
  lblKO:return -1;
}

/** Stop waiting for a touch.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0138.c
 */
int tftStop(void) {
    int ret;

    trcFS("%s\n", "tftStop");

    ret = hmiTouchAutoCoordStop(hHmi);
    CHECK(ret == RET_OK, lblKO);

    return 0;
  lblKO:return -1;
}

/** Retrieve a touch coordinates if any.
 * \return
 *  - x and y coordinates packed to card
 *  - If no touch zero value is returned.
 *  - In case of error it returns 0xFFFFFFFF.
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0138.c
 */
card tftGet(void) {
    int ret;
    hmiTouchCoordResult_t rsp;

    memset((void *) &rsp, 0, sizeof(hmiTouchCoordResult_t));
    tftSta = 0;
    ret = hmiResultGet(hHmi, sizeof(hmiTouchCoordResult_t), (void *) &rsp);
    switch (ret) {
      case RET_RUNNING:
          ret = 0;
          break;                //no touch until now
      case RET_OK:
          if(rsp.eventType == HMI_TOUCH_EV_DOWN) {
              trcS("d\n");
              tftSta = 'd';
              tftLoc = CARDHL(rsp.xCoord, rsp.yCoord);
#ifdef WIN32
              //            tmrSleep(10);
#else
              //            tmrSleep(1);
#endif
          } else if(rsp.eventType == HMI_TOUCH_EV_UP) {
              trcS("u\n");
              tftSta = 'u';
          }
          ret = tftLoc;

          break;                //screen is touched, return the coordinates
      default:
          ret = -1;
          break;                //an error encountered
    }
    if(ret) {
        trcFS("%s: ", "tftGet");
        trcFN("[%08x]\n", ret);
    }
    return (card) ret;
}

/** Make a beep.
 * \return no
 * \header sys\\sys.h
 * \source sys\\hmi.c
 */
void utaBeep(void) {
    int ret;

    trcFS("%s\n", "Beep");
    ret = hmiBeep(hHmi, HMI_BEEP_SHORT, HMI_BEEP_MIDTONE);
    CHECK(ret == RET_OK, lblKO);
  lblKO:;
}

/** Make a click.
 * \return no
 * \header sys\\sys.h
 * \source sys\\hmi.c
 */
void Click(void) {
    int ret;

    trcFS("%s\n", "Click");
    ret = hmiBeep(hHmi, HMI_BEEP_CLICK, HMI_BEEP_MIDTONE);
    CHECK(ret == RET_OK, lblKO);
  lblKO:;
}

#ifdef __CANVAS__
static Canvas *cnv = 0;
static GC ctxOpt;               //graphic context

/** Initialize graphics screen and the canvas.
 *
 * Should be called after dspStart() and before any canvas function
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvStart(void) {
    int ret = 0;
    static Canvas _cnv;

#if (__CNVD__==1)
    static byte bmp[(cnvW * cnvH) / 8];
#endif
#if (__CNVD__==8)
    static byte bmp[cnvW * cnvH];
#endif
    OpticError err;

    //hmiOutputConfig_t cfg;
    Dimen wdt, hgt;
    byte pal, bpp;

    trcFS("%s\n", "cnvStart");

    if(!hmiStarted) {
        ret = dspStart();
        CHECK(ret >= 0, lblKO);
    }
    //hmiOutputConfigRead(hHmi,&cfg);
    wdt = cnvW;                 //(Dimen)cfg.wwidth
    hgt = cnvH;                 //(Dimen)cfg.wheight
    //pal = 1<<cnvD;                                //number of colors
    pal = 0;
    bpp = HMI_BPP_MAX;
    switch (cnvD) {             //derive palette size from color depth
      case 1:
          pal = 2;
          bpp = HMI_BPP_1;
          break;                //black & white
      case 8:
          pal = 255;
          bpp = HMI_BPP_8;
          break;                //8 bpp
      default:
          break;                //others are not supported
    }
    VERIFY(pal);
    //err= hmiOpCreateCanvas(&cnv,wdt,hgt,dpt); //does not work in terminal (?!)
    if(!cnv) {
        cnv = &_cnv;
        err = hmiOpDefineCanvas(cnv, wdt, hgt, pal, (Bitmap *) bmp);
        CHECK(err == OPTIC_NO_ERROR, lblKO);
    }

    ret = hmiGraphicBppSet(hHmi, (hmiBpp_t) bpp);
    CHECK(ret == RET_OK, lblKO);

    hmiOpInitGC(&ctxOpt);

    return 1;
  lblKO:
    trcErr(ret);
    return 0;
}

/** Release memory reserved for canvas.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvStop(void) {
    //OpticError err;
    //VERIFY(cnv);

    memset(&ctxOpt, 0, sizeof(ctxOpt));

    //err= hmiOpDisposeCanvas(cnv); //does not work (?!)
    //CHECK(err==OPTIC_NO_ERROR,lblKO);

    trcFS("%s\n", "cnvStop");
    return 0;
    //lblKO:
    //    return -1;
}

/** Draw canvas on the screen.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvDraw(void) {
    int ret;

    VERIFY(cnv);

    ret = hmiGraphicDataWrite(hHmi, cnv->bm, 0, 0, cnv->w, cnv->h);
    CHECK(ret == RET_OK, lblKO);
    return 0;
  lblKO:
    return -1;
}

/** Clear canvas (not visible until cnvDraw is called).
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvClear(void) {
    OpticError err;

    VERIFY(cnv);

    err = hmiOpSetBackground(&ctxOpt, clrBgd);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpClearCanvas(cnv, &ctxOpt);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    return 0;
  lblKO:
    return -1;
}

/** Put pixel of a given color on the canvas.
 * \param hor (I) Horizontal coordinate (top->bottom)
 * \param ver (I) Vertical coordinate (left->right)
 * \param clr (I) Pixel color
 * \pre
 *  - hor<cnvW
 *  - ver<cnvH
 *  - clr<cnvP
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvPixel(word hor, word ver, word clr) {
    OpticError err;

    //      int ret;

    //trcS("cnvPixel");
    //trcFN(" hor= %d",hor);
    //trcFN(" ver= %d",ver);
    //trcS("\n");

    VERIFY(hor < cnvW);
    VERIFY(ver < cnvH);
    //VERIFY(clr < cnvP);
    VERIFY(cnv);

    err = hmiOpSetForeground(&ctxOpt, clr);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpDrawPoint(cnv, &ctxOpt, hor, ver);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    return 0;
  lblKO:
    return -1;
}

/** Draw a line of a given color on the canvas.
 * \param x1 (I) Horizontal coordinate (top->bottom) of the first end
 * \param y1 (I) Vertical coordinate (left->right) of the first end
 * \param x2 (I) Horizontal coordinate (top->bottom) of the second end
 * \param y2 (I) Vertical coordinate (left->right) of the second end
 * \param clr (I) Line color
 * \pre
 *  - x1<cnvW
 *  - y1<cnvH
 *  - x2<cnvW
 *  - y2<cnvH
 *  - clr<cnvP
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvLine(word x1, word y1, word x2, word y2, word clr) {
    OpticError err;

    trcFN("cnvLine x1=%d", x1);
    trcFN(" y1=%d", y1);
    trcFN(" x2=%d", x2);
    trcFN(" y2=%d\n", y2);

    VERIFY(x1 < cnvW);
    VERIFY(y1 < cnvH);
    VERIFY(x2 < cnvW);
    VERIFY(y2 < cnvH);
    //VERIFY(clr < cnvP);
    VERIFY(cnv);

    err = hmiOpSetForeground(&ctxOpt, clr);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpDrawLine(cnv, &ctxOpt, x1, y1, x2, y2);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    return 0;
  lblKO:
    return -1;
}

/** Draw a filled rectangle of a given color on the canvas.
 * \param x1 (I) Horizontal coordinate (top->bottom) of one corner
 * \param y1 (I) Vertical coordinate (left->right) of one corner
 * \param x2 (I) Horizontal coordinate (top->bottom) of the opposite corner
 * \param y2 (I) Vertical coordinate (left->right) of the opposite corner
 * \param clr (I) Rectangle fill color
 * \pre
 *  - x1<cnvW
 *  - y1<cnvH
 *  - x2<cnvW
 *  - y2<cnvH
 *  - clr<cnvP
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvBar(word x1, word y1, word x2, word y2, word clr) {
    OpticError err;
    word x, y, w, h;

    trcFN("cnvBar x1=%d", x1);
    trcFN(" y1=%d", y1);
    trcFN(" x2=%d", x2);
    trcFN(" y2=%d\n", y2);

    VERIFY(x1 < cnvW);
    VERIFY(y1 < cnvH);
    VERIFY(x2 < cnvW);
    VERIFY(y2 < cnvH);
    //VERIFY(clr < cnvP);
    VERIFY(cnv);

    err = hmiOpSetForeground(&ctxOpt, clr);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    x = x1;
    if(x2 < x1) {
        x = x2;
        w = x1 - x2;
    } else
        w = x2 - x1;
    w++;

    y = y1;
    if(y2 < y1) {
        y = y2;
        h = y1 - y2;
    } else
        h = y2 - y1;
    h++;

    hmiOpSetRectType(&ctxOpt, RectTypeFilled);

    err = hmiOpDrawRect(cnv, &ctxOpt, x, y, w, h);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    return 0;
  lblKO:
    trcErr(err);
    return -1;
}

/** Draw a text on the canvas.
 * \param x (I) Horizontal coordinate (top->bottom)
 * \param y (I) Vertical coordinate (left->right)
 * \param txt (I) text to draw
 * \param fnt (I) font identifier
 * \param clr (I) foreground color
 * \pre
 *  - x<cnvW
 *  - y<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvText(word x, word y, const char *txt, byte fnt, card clr) {
    OpticError err;
    word len;

    VERIFY(x < cnvW);
    VERIFY(y < cnvH);
    VERIFY(txt);
    VERIFY(cnv);
    //VERIFY(clr < cnvP);

    if(*txt == 0)
        return 0;

    //err = hmiOpSetTextDirection(&ctxOpt, TextDirDefault);
    //CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpSetFont(&ctxOpt, fnt);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpSetForeground(&ctxOpt, clr);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    len = strlen(txt);
    err = hmiOpDrawText(cnv, &ctxOpt, x, y, (char *) txt, len);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    return 0;
  lblKO:
    return -1;
}

/** Computes the size of a rectangle required to completely enclose the specified text string
 * \param txt (I) text to draw
 * \param fnt (I) font identifier
 * \return card value, left word being the width of the rectangle, right word being its height
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
card cnvTextSize(const char *txt, byte fnt) {
    OpticError err;
    TextExtent ext;

    VERIFY(txt);
    VERIFY(cnv);

    if(*txt == 0)
        return 0;

    err = hmiOpSetFont(&ctxOpt, fnt);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    err = hmiOpTextExtent(&ctxOpt, (char *) txt, (short) strlen(txt), &ext);
    CHECK(err == OPTIC_NO_ERROR, lblKO);

    return CARDHL(ext.w, ext.h);
  lblKO:
    return 0;
}

/** Draw a tile/bitmap to the canvas.
 *
 * Tile is the array of bytes, each byte being a pixel color.
 *
 * Bitmap is a set of bits, each bit corresponding to a pixel.
 * If the bitmap width is not a multiple of 8 the tail of the last byte of the line is ignored.
 *
 * \param x (I) Horizontal coordinate (top->bottom) of the upper left corner
 * \param y (I) Vertical coordinate (left->right) of the upper left corner
 * \param wdt (I) bitmap width
 * \param hgt (I) bitmap height
 * \param ptr (I) pointer to tile | bitmap
 * \param clr (I) color:
 *   - if negative, ptr points to bitmap, and clr is the color of all non-zero bits
 *   - if positive, ptr is treated as a tile, and (-clr+1) as base color;
 * \pre
 *  - x<cnvW
 *  - y<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\hmi.c
 * \test tcab0117.c
 */
int cnvTile(word x, word y, word wdt, word hgt, int clr, const byte * ptr) {
    OpticError err;
    int ret;
    byte h, v;

    trcFN("cnvTile x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", wdt);
    trcFN(" hgt=%d\n", hgt);

    VERIFY(x < cnvW);
    VERIFY(y < cnvH);
    VERIFY(cnv);
    VERIFY(ptr);

    if(clr < 0) {
        err = hmiOpSetTileBaseColor(&ctxOpt, (Color) (-clr + 1));
        CHECK(err == OPTIC_NO_ERROR, lblKO);

        err = hmiOpDrawTile(cnv, &ctxOpt, x, y, wdt, hgt, (Color *) ptr);
        CHECK(err == OPTIC_NO_ERROR, lblKO);
    } else {
        for (v = 1; v <= hgt; v++) {
            for (h = 1; h <= wdt; h++) {
                if(!bitTest(ptr, h)) {
                    //trcS(".");
                    continue;
                }
                ret =
                    cnvPixel((word) (x + h - 1), (word) (y + v - 1),
                             (byte) clr);
                CHECK(ret >= 0, lblKO);
                //trcS("*");
            }
            ptr += (wdt + 7) / 8;
            //trcS("\n");
        }
    }

    return 0;
  lblKO:
    return -1;
}
#endif

#endif
