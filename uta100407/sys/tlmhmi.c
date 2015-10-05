/** \file
 * TELIUM HMI implementation
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlmhmi.c $
 *
 * $Id: tlmhmi.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "sdk30.h"
#include "Libgr.h"
#include "sys.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcHMI))

static FILE *hPrt = 0;
static byte prtWait = 0;

static int selectFont(void) {
    const char *fnt = __FONT__;
    static char *dft = 0;

    if(dft) {
        DefCurrentFont((char *) dft);
        return 0;               //should be called only once
    }
    trcFS("selectFont: %s\n", fnt);
    dft = LoadFont((char *) fnt);
    CHECK(dft, lblKO);
    DefCurrentFont((char *) dft);

    return 1;
  lblKO:
    return -1;
}

/** Open the associated channel.
 * Should be called before calling any printing, normally before an event treatment.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0000.c
 */
int prtStart(void) {
    int ret;

    trcS("prtStart\n");

    hPrt = fopen("PRINTER", "w-*");
    CHECK(hPrt, lblKO);
    prtWait = 0;

    ret = selectFont();
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control to another application.
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0000.c
 */
int prtStop(void) {
    int ret;

    trcFS("%s", "prtStop\n");
    if(hPrt) {
#ifndef __FONT__
        if(prtWait)
            ttestall(PRINTER, 0);
#endif
        ret = fclose(hPrt);
        //CHECK(ret==RET_OK,lblKO);
        hPrt = 0;
    }
    prtWait = 0;

    return 1;
}

/** Control paper presence for associated channel.
 * Should be called before printing
 * \return
 *    - negative if failure.
 * 	  - zero if paper is absent
 *    - positive if paper is present or if no information available.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcjl0000.c
 */
int prtCtrl(void) {
    unsigned char sta = 0;

    trcFS("%s", "prtCtrl\n");
    VERIFY(hPrt);
    if(prtWait) {
        ttestall(PRINTER, 0);
        prtWait = 0;
    }
    status(hPrt, &sta);
    if(sta & PAPER_OUT)
        return -1;
    return 1;

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
    char buf[prtW + 1 + 1];     //one more character for line break at the end
    char esc[prtW + 1];
    byte idx;

    VERIFY(str);
    trcFS("prtS str=%s\n", str);

    memset(esc, 0, sizeof(esc));
    idx = 0;
    while(*str == 0x1B) {       //strings started with esc characters are treated via prtES
        memcpy(esc + idx, str, 2);
        idx += 2;
        str += 2;
    }
    if(*esc)
        return prtES(esc, str);

    memset(buf, 0, prtW + 1);
    if(strlen(str) <= prtW)
        strcpy(buf, str);
    else
        memcpy(buf, str, prtW);
    strcat(buf, "\n");

    ret = defprinterpattern('\xFF', (char *) NULL);
    ret = pprintf8859(buf, _OFF_, _pNORMAL_, _FIXED_WIDTH_);
    CHECK(ret >= 0, lblKO);

    prtWait = 0;
    return strlen(buf);
  lblKO:
    return -1;
}

/** Print string str preceded by a sequence of escape characters
 * \param esc (I) escape control characters
 * \param str (I) String to print.
 *
 * The following esc characters are supported:
 *  - 0x1A: BOLD (24 columns)
 *  - 0x1D: HIGH (24 columns)
 *  - 0x1E: BIG (12 columns)
 *  - 0x20: CONDENSED (48 columns)
 *  - 0x2A: MEDIUM (16 columns)
 *  - 0x22: REVERSED
 *  - 0x17: CENTERED
 *  - 0x18: RIGHT JUSTIFIED
 *
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0000.c
 */
int prtES(const char *esc, const char *str) {
    int ret;
    int len;
    char *fnt;
    char *tmpFnt;
    byte rev = 0;               //reversed?
    char adj = 0;               //'r': right justified, 'c': centered
    byte wdt = prtW;            //number of characters per line
    byte tmpWdt = prtW;
    byte idx;
    char tmp[prtW * 2 + 1 + 1];
    char buf[prtW * 2 + 1 + 1];
    byte fntChanged = 0;
    int fmtWidth = _FIXED_WIDTH_;
    static int k=5;
    int i, spaceMinus;


    VERIFY(str);
    trcFS("prtES str=%s\n", str);

    tmpFnt = "_pNORMAL_";
    fnt = "_pNORMAL_";

    wdt = prtW;
    if(esc) {
        idx = 0;
        while(esc[idx]) {
            VERIFY(esc[idx] == 0x1B);
            switch (esc[++idx]) {
              case 0x1A:       //BOLD
                  tmpFnt = "_pBOLD_";
                  tmpWdt = prtW;
                  break;
              case 0x1D:       //2 X HEIGHT
                  tmpFnt = "_pHIGH_";
                  tmpWdt = prtW;
                  break;
              case 0x1E:       //2 X SIZE
                  tmpFnt = "_pBIG_";
                  tmpWdt = prtW / 2;
                  break;
              case 0x20:       //48 columns
                  tmpFnt = "_pCONDENSED_";
                  tmpWdt = prtW * 2;
                  break;
              case 0x2A:       //16 columns
                  tmpFnt = "_pMEDIUM_";
                  tmpWdt = (prtW * 2) / 3;
                  break;
              case 0x22:       //REVERSE
                  rev = 1;
                  break;
              case 0x17:       //CENTERED
                  adj = 'c';
                  break;
              case 0x18:       //right justified
                  adj = 'r';
                  break;
              case 0x03:       //adhesive (no gaps between letters)
                  fmtWidth = _PROP_WIDTH_;
                  wdt = prtW * 2;
                  break;
              default:
                  break;
            };
            idx++;

            /* Since Ingenico terminal was supporting two escape characters at the same line (Bold & Condensed),
             * And in Sagem, bold with 48 columns is not supported since you can only combine 1 alignment 
             * Type with 1 font format with 1 for reversed format or not.
             * So this condition will handle only one font format and skip the other one which is for Ingenico.
             */
            if((memcmp(tmpFnt, "_pNORMAL_", 9) != 0) && (fntChanged != 1)) {
                fnt = tmpFnt;   //copy the new font format in fnt pointer which will use for printing this line.
                wdt = tmpWdt;   //copy the new number of characters per this line.
                fntChanged = 1;
            }
        }
    }

    memset(tmp, 0, sizeof(tmp));
    VERIFY(wdt < sizeof(tmp));
    memset(buf, 0, wdt);
    memset(tmp, ' ', wdt);
    if(strlen(str) >= wdt) {    //string too long; truncate it
        memcpy(tmp, str, wdt);
    } else {
        switch (adj) {
          case 'r':            //justify right
              idx = (wdt - strlen(str));
              memset(buf, ' ', idx);
              ret = defprinterpattern('\xFF', (char *) NULL);
              ret = pprintf8859(buf, _OFF_, fnt, fmtWidth);
              CHECK(ret >= 0, lblKO);
              strcpy(tmp, str);
              break;
          case 'c':            //center
              idx = (wdt - strlen(str)) / 2;
              memset(buf, ' ', idx);
              ret = defprinterpattern('\xFF', (char *) NULL);
              ret = pprintf8859(buf, _OFF_, fnt, fmtWidth);
              CHECK(ret >= 0, lblKO);
  		strcpy(tmp, buf);
		strcat(tmp, str);
		//strcpy(tmp, str); //ori
              break;
          default:
              strncpy(tmp, str, wdt);
              break;
        }
    }

    len = strlen(tmp);
    if(tmp[len] != 0x0D && tmp[len] != 0x0A)
        strcat(tmp, "\n");
#if 0
    ret = defprinterpattern('\xFF', (char *) NULL);
    ret = pprintf8859(tmp, rev ? _ON_ : _OFF_, fnt, fmtWidth);
    CHECK(ret >= 0, lblKO);
#else
    InitPrinterBitmap(512);
    i = GetExtendedPoliceSize(fnt);

   if(memcmp(esc, "\x1b\x13", 2) ==  0)
   	spaceMinus = 40;
   else
   	spaceMinus = 37;
	
    ret = _pprintf8859XY(0,(k*i)-(k*spaceMinus),tmp, rev ? _ON_ : _OFF_,fnt,_FIXED_WIDTH_);
    CHECK(ret >= 0, lblKO);

    k++;
    if(k>7)k=5;
    PaintPrinterGraphics();
#endif

    prtWait = 0;
    return strlen(tmp);
  lblKO:
    return -1;
}

/** Print an image
 * \param img (I) image bitmap
 * \param hgt (I) image height
 * \param wdt (I) image width
 * \param ctl (I) control string, consists of two characters:
 *              - 'l'|'c'|'r': left or center or right justify
 *              - '1'|'2'|'4'|'8'|: multiplier factor
 * \return
 *    - negative if failure.
 *    - non negative if OK.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0000.c
 */
int prtImg(const byte * img, int hgt, int wdt, const char *ctl) {
    int ret;
    char buf[2 + 1];

    trcS("prtImg\n");
    VERIFY(img);

    ret = defprinterpattern('\xFF', (char *) img);
    CHECK(ret == 0, lblKO);
    strcpy(buf, "\xFF\n");
    ret = fprintf(hPrt, buf);
    CHECK(ret, lblKO);

    ttestall(PRINTER, 0);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static FILE *hDsp = 0;
static byte dspHdr = 1;
static byte _dspWdt = dspW;
static byte _dspHgt = dspH;
static byte fontHgt = fntH;

/** Open the associated channel.
 * Should be called before calling any display processing,
 * normally before an event treatment.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0002.c
 */
int dspStart(void) {
    int ret;

    trcFS("%s\n", "dspStart");
    dspHdr = StateHeader(0);

    if(!hDsp)
        hDsp = fopen("DISPLAY", "w");
    CHECK(hDsp, lblKO);

    CreateGraphics(fontHgt);
    InitContext(_PERIPH_DISPLAY);

    ret = selectFont();
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Close the associated channel.
 * Should be called before returning to the idle prompt or before giving the control to another application.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0002.c
 */
int dspStop(void) {
    trcFS("%s\n", "dspStop");
    if(hDsp) {
        fclose(hDsp);
        StateHeader(dspHdr);
    }
    hDsp = 0;
    dspHdr = 0;
    return 0;
}

/** Clear display.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0002.c
 */
int dspClear(void) {
    int ret;

    trcFS("%s\n", "dspClear");
    if(!hDsp) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }
    VERIFY(hDsp);

    _clrscr();
    PaintGraphics();
    return 0;
  lblKO:
    return -1;
}

/** Set backlight intensity
 * \param val the backlight intensity (0=Off, 100= On)
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * On Telium it is managed by OS, so it is not implemented
 * \test tcab0002.c
 */
int dspLight(byte val) {
    int ret;

    trcFN("dspLight val= %d\n", val);
    if(!hDsp) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }
    VERIFY(hDsp);
#ifndef WIN32
    if(val)
        StartRetroEclairage(4, 0x0001, 0xFFFF);
    else
        StopRetroEclairage();
#endif
    return 1;
  lblKO:
    return -1;
}

enum eFntDsp {
    fntDspNormal,
    fntDspBold,
    fntDspBig,
    fntDspEnd
};
static const char *fntDsp[fntDspEnd + 1] = {
    "_dNORMAL_",
    "_dBOLD_",
    "_dBIG_",
    0
};

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
 * \source sys\\tlmhmi.c
 * \test tcab0002.c
 */
int dspLS(int loc, const char *str) {
    int ret;
    byte len;
    byte rev;
    char buf[255];
    const char *fnt = fntDsp[fntDspNormal];

    VERIFY(str);
    VERIFY((loc & 0x0F) < dspHgt(0));

    memset(buf, 0, sizeof(buf));

    trcFN("dspLS loc=%02X ", loc);
    trcFS("str=%s\n", str);

    if(!hDsp) {
        ret = dspStart();
        CHECK(ret, lblKO);
    }
    VERIFY(hDsp);

    strncpy(buf, str, dspWdt(0));   //truncate the string if it is too long
    len = (byte) strlen(buf);
    if(len < dspWdt(0)) {
        byte gap = dspWdt(0) - len;

        if(loc & 0x20) {        //right justified
            memmove(buf + gap, buf, len);
            memset(buf, ' ', gap);
        } else {
            memset(buf + len, ' ', gap);
        }
    }

    if(loc & 0x40)              //bold font
        fnt = fntDsp[fntDspBold];

	fontHgt = fntH;
    //fontHgt = 8;
    if(loc & 0x10) {            //big font
        fnt = fntDsp[fntDspBig];
        fontHgt = 21;
    }

    rev = loc & 0x80;
    loc &= 0x0F;
    DrawExtendedString8859(0, loc * fontHgt, (char *) buf, rev ? _ON_ : _OFF_,
                           (char *) fnt, _FIXED_WIDTH_);

    return strlen(buf);
    goto lblEnd;
  lblKO:
    return -1;
  lblEnd:

    return 1;
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
    char *ptr = 0;
    byte rev;

    VERIFY(str);
    trcFN("dspXYS X=%d", x);
    trcFN(" y=%d ", y);
    trcFS(" str=%s\n", str);

    rev = font & 0x80 ? _ON_ : _OFF_;

    switch (font) {
      case 1:
          ptr = "_dBOLD_";
          break;
      case 2:
          ptr = "_dBIG_";
          break;
      default:
          ptr = "_dNORMAL_";
          break;
    }
    DrawExtendedString8859(x, y, (char *) str, rev, ptr, _FIXED_WIDTH_);
    return strlen(str);
}

/** Get/set display screen width.
 * The zero value of argument means just to return the current value of screen width.
 * \param wdt new display width if not zero
 * \return previous screen width
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
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
 * \source sys\\tlmhmi.c
 * \test tcab0002.c
 * The height modification is not implemented and not recommended to be used
 */
byte dspHgt(byte hgt) {
    byte tmp = _dspHgt;

    if(hgt)
        _dspHgt = hgt;
    return tmp;
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
    byte h, v;

    trcFN("dspImg x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", wdt);
    trcFN(" hgt=%d\n", hgt);
    VERIFY(img);
    for (v = 1; v <= hgt; v++) {
        for (h = 1; h <= wdt; h++) {
            if(!bitTest(img, h)) {
                //trcS(".");
                continue;
            }
            _SetPixel((word) (x + h - 1), (word) (y + v - 1), 1);
            //trcS("*");
        }
        img += (wdt + 7) / 8;
        //trcS("\n");
    }
    PaintGraphics();
    return 1;
}

static FILE *hKbd = 0;

/** Start waiting for a key.
 * This function should be called when an application is ready to accept a user input.
 * The keys waiting in the buffer are flushed if fls is not zero.
 * \param fls (I) Indicates whether key flush is to be done
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0003.c
 */
int kbdStart(byte fls) {
    trcFS("%s\n", "kbdStart");

    if(!hKbd) {
        hKbd = fopen("KEYBOARD", "r");
        CHECK(hKbd, lblKO);
    }

    if(fls)                     // Flush Input Buffer
        reset_buf(hKbd, _receive_id);

    return 1;

  lblKO:
    return -1;
}

/** Stop waiting for a key. All the keys pressed after calling this function will be ignored.
 * \return 0
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0003.c
*/
int kbdStop(void) {
    trcFS("%s\n", "kbdStop");
    if(hKbd)
        fclose(hKbd);
    hKbd = 0;
    return 0;
}

/** Retrieve a key pressed if any. Return its key code.
 * \return
 *    - Key code according to eKbd enum.
 *    - If no key pressed zero value is returned.
 *    - In case of error it returns 0xFF.
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0003.c
 */
char kbdKey(void) {
    int ret;
    byte key = 0;

    ret = ttestall(KEYBOARD, 1);    // wait for the first event

    if(ret & KEYBOARD) {
        key = getchar();
        trcFS("%s: ", "kbdKey");
        trcFN("hex= [%02x] ", key);
        trcFN("chr= [%c]\n", key);
    }
    return key;
}

#ifdef __CANVAS__
/** Initialize graphics screen and the canvas.
 * Should be called before any canvas function.
 * In Telium it is the same as dspStart
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvStart(void) {
    trcFS("%s\n", "cnvStart");
#if __CNVD__ > 1
    InitContexteGraphique(PERIPH_DISPLAY_EXTENDED);
    if(!hDsp)
        hDsp = fopen("DISPLAY", "a");
    CHECK(hDsp, lblKO);
#else
    if(!hDsp)
        return dspStart();
#endif
    return 0;
#if __CNVD__ > 1
  lblKO:
    return -1;
#endif
}

/** Release memory reserved for canvas.
 * In Telium it is the same as dspStop
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvStop(void) {
    trcFS("%s\n", "cnvStop");
#if __CNVD__ > 1
    InitContexteGraphique(PERIPH_DISPLAY);
#endif
    if(!hDsp)
        return dspStop();
    return 0;
}

/** Draw canvas on the screen.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvDraw(void) {
    PaintGraphics();
    return 1;
}

/** Clear canvas (not visible until cnvDraw is called).
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvClear(void) {
#if __CNVD__ > 1
    //memset(_scr,0,sizeof(_scr));
    //_SetScreenToScreen(0,0,_scr,cnvW,cnvH);
    cnvBar(0, 0, cnvW - 1, cnvH - 1, clrBgd);
#else
    _clrscr();
#endif
    return 1;
}

#define FFR0G0B0(RGB) ((((((card)RGB)&0xF00) << 12) + ((((card)RGB)&0x0F0) << 8) + ((((card)RGB)&0x00F) << 4))+0xFF000000L)

/** Put pixel of a given color on the canvas.
 * \param hor (I) Horizontal coordinate (left->right)
 * \param ver (I) Vertical coordinate (top->bottom)
 * \param clr (I) Pixel color
 * \pre
 *  - hor<cnvW
 *  - ver<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvPixel(word hor, word ver, word clr) {
    //trcS("cnvPixel");
    //trcFN(" hor= %d",hor);
    //trcFN(" ver= %d",ver);
    //trcS("\n");
    VERIFY(hor < cnvW);
    VERIFY(ver < cnvH);
#if __CNVD__ == 1
    _SetPixel(hor, ver, clr);   //starting with underscore: don't redraw the screen
#else
    //_SetScreenToScreen(hor,ver,&clr,1,1); //starting with underscore: don't redraw the screen
    _Pixel(hor, ver, FFR0G0B0(clr));    //starting with underscore: don't redraw the screen
#endif
    return 1;
}

/** Draw a line of a given color on the canvas.
 * \param x1 (I) Horizontal coordinate (left->right) of the first end
 * \param y1 (I) Vertical coordinate (top->bottom) of the first end
 * \param x2 (I) Horizontal coordinate (left->right) of the second end
 * \param y2 (I) Vertical coordinate (top->bottom) of the second end
 * \param clr (I) Line color
 * \pre
 *  - x1<cnvW
 *  - y1<cnvH
 *  - x2<cnvW
 *  - y2<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvLine(word x1, word y1, word x2, word y2, word clr) {
    VERIFY(x1 < cnvW);
    VERIFY(y1 < cnvH);
    VERIFY(x2 < cnvW);
    VERIFY(y2 < cnvH);
#if __CNVD__ == 1
    _DrawLine(x1, y1, x2, y2, clr); //starting with underscore: don't redraw the screen
#else
    _Line(x1, y1, x2, y2, FFR0G0B0(clr));   //starting with underscore: don't redraw the screen
#endif
    return 1;
}

/** Draw a filled rectangle of a given color on the canvas.
 * \param x1 (I) Horizontal coordinate (left->right) of one corner
 * \param y1 (I) Vertical coordinate (top->bottom) of one corner
 * \param x2 (I) Horizontal coordinate (left->right) of the opposite corner
 * \param y2 (I) Vertical coordinate (top->bottom) of the opposite corner
 * \param clr (I) Rectangle fill color
 * \pre
 *  - x1<cnvW
 *  - y1<cnvH
 *  - x2<cnvW
 *  - y2<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvBar(word x1, word y1, word x2, word y2, word clr) {
    word x, y;

    //word *ptr= 0;

    trcFN("cnvBar x1=%d", x1);
    trcFN(" y1=%d", y1);
    trcFN(" x2=%d", x2);
    trcFN(" y2=%d\n", y2);
    VERIFY(x1 < cnvW);
    VERIFY(y1 < cnvH);
    VERIFY(x2 < cnvW);
    VERIFY(y2 < cnvH);

    x = x1;
    if(x2 < x1) {
        x = x2;
        x2 = x1;
        x1 = x;
    }

    y = y1;
    if(y2 < y1) {
        y = y2;
        y2 = y1;
        y1 = y;
    }
#if __CNVD__ == 1
    _DrawRect(x1, y1, x2, y2, 0, _OFF_, clr);   //starting with underscore: don't redraw the screen
#else
    _Area(x1, y1, x2, y2, FFR0G0B0(clr));   //starting with underscore: don't redraw the screen
    /*
       ptr= _scr;
       for(x= x1; x<=x2; x++){
       for(y= y1; y<=y2; y++){
       *ptr++= clr;
       }
       }
       VERIFY(x1 <= x2);
       VERIFY(y1 <= y2);
       _SetScreenToScreen(x1,y1,_scr,x2-x1+1,y2-y1+1); //starting with underscore: don't redraw the screen
     */
#endif
    return 1;
}

/** Draw a text on the canvas.
 * \param x (I) Horizontal coordinate (left->right)
 * \param y (I) Vertical coordinate (top->bottom)
 * \param txt (I) text to draw
 * \param fnt (I) font identifier
 * \param clr (I) CARDHL(background color,foreground color)
 * \pre
 *  - x<cnvW
 *  - y<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvText(word x, word y, const char *txt, byte fnt, card clr) {
    word h;

    VERIFY(x < cnvW);
    VERIFY(y < cnvH);
    VERIFY(txt);

    if(*txt == 0)
        return 0;

    h = LWORD(cnvTextSize(txt, fnt));
    if(y < h)
        h = y;
    y -= h;

#if __CNVD__ > 1
    _DrawText8859(x, y, (char *) txt, (char *) fntDsp[fnt], _FIXED_WIDTH_, FFR0G0B0(LWORD(clr)),    //foreground
                  FFR0G0B0(HWORD(clr))  //background
        );
#else
    //_DrawExtendedString(x, y, (char *) txt, clr ? 0 : 1, fnt, _PROPORTIONNEL_); //TBD: use downloaded fonts Before: _FIXED_WIDTH_
    _DrawExtendedString8859(x, y,
                            (char *) txt,
                            clr ? _ON_ : _OFF_,
                            (char *) fntDsp[fnt], _FIXED_WIDTH_);
#endif

    return 0;
}

/** Computes the size of a rectangle required to completely enclose the specified text string
 * \param txt (I) text to draw
 * \param fnt (I) font identifier
 * \return card value, left word being the width of the rectangle, right word being its height
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
card cnvTextSize(const char *txt, byte fnt) {
    word wdt, hgt;

    VERIFY(txt);
    VERIFY(fnt < fntDspEnd);

    if(*txt == 0)
        return 0;

    //wdt = _SizeofExtendedString((char *) txt, fnt, _NORMALE_);
    switch (fnt) {
      case fntDspNormal:
          wdt = 8;
          hgt = 16;
          break;
      case fntDspBold:
          wdt = 8;
          hgt = 16;
          break;
      case fntDspBig:
          wdt = 16;
          hgt = 32;
          break;
      default:
          hgt = wdt = 0;
          break;
    }
    VERIFY(hgt);
    wdt *= strlen(txt);
    //hgt = GetPoliceSize();

    return CARDHL(wdt, hgt);
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
 * \param clr (I) color
 * \pre
 *  - x<cnvW
 *  - y<cnvH
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmhmi.c
 * \test tcab0117.c
 */
int cnvTile(word x, word y, word wdt, word hgt, int clr, const byte * ptr) {
    int ret;
    byte h, v;
    word *cur;
    static word scr[cnvW * cnvH];

    trcFN("cnvTile x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", wdt);
    trcFN(" hgt=%d\n", hgt);

    VERIFY(x < cnvW);
    VERIFY(y < cnvH);
    VERIFY(ptr);

    cur = scr;
    if(clr < 0) {
#if __CNVD__ >1
        word pal[] = {
            0x0000,             //clrBlack
            0x0007,             //clrBlue
            0x0070,             //clrGreen
            0x0077,             //clrCyan
            0x0700,             //clrRed
            0x0707,             //clrMagenta
            0x0770,             //clrYellow
            0x0555,             //clrGray
            0x0AAA,             //clrGRAY
            0x000F,             //clrBLUE
            0x00F0,             //clrGREEN
            0x00FF,             //clrCYAN
            0x0F00,             //clrRED
            0x0F0F,             //clrMAGENTA
            0x0FF0,             //clrYELLOW
            0x0FFF,             //clrWHITE
            0
        };
        for (v = 1; v <= hgt; v++) {
            for (h = 1; h <= wdt; h++) {
                *cur++ = pal[(*ptr++) % 16];
            }
        }
        _SetScreenToScreen(x, y, scr, wdt, hgt);
#else
        VERIFY(clr >= 0);
#endif
    } else {
        for (v = 1; v <= hgt; v++) {
            for (h = 1; h <= wdt; h++) {
                if(!bitTest(ptr, h)) {
                    //trcS(".");
                    continue;
                }
                ret =
                    cnvPixel((word) (x + h - 1), (word) (y + v - 1),
                             (card) clr);
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
