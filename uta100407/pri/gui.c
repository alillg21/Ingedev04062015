/** \file
 * Graphical User Interface
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/pri/gui.c $
 *
 * $Id: gui.c 2329 2009-05-06 13:57:36Z abarantsev $
 */

#include "pri.h"
#include <string.h>

#ifdef __CANVAS__
#undef trcOpt
//static const card trcOpt = TRCOPT & BIT(trcGUI);
#define trcOpt (TRCOPT & BIT(trcGUI))

static const tVmtGui _vmt = { 0, 0 };

static byte _guiClick = 0;      //click after key press?

/** Enable/disable clicking while key pressing in gui dialogs
 * \param clk (I)
 *  - 0: disable clicking
 *  - 1: enable ckicking
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\gui.c
*/
void guiClick(byte clk) {
    _guiClick = clk;
}

/** Input constructor: 
 * Build an input structure from a drawing.
 *
 * \param inp (O) Pointer to the input to be initialized
 * \param drw (I) Pointer to the associated drawing
 * \param x   (I) Horizontal coordinate of the drawing
 * \param y   (I) Vertical coordinate of the drawing
 * \param ctl (I) Entry mode control string
 * \param vmt (I) Virtual methods table
 * \param dly (I) dly wait timeout in seconds
 * \pre 
 *  - inp!=0
 *  - drw!=0
 *  - x<cnvW
 *  - y<cnvH
 *  - ctl!=0
 *
 * The control string can contain the following characters:
 *  - 'k','K': wait for keyboard
 *  - 't','T': wait for touch screen
 *  - 'm': wait for magnetic stripe card swipe
 *  - 'c': wait for chip card insertion
 *  - 'b': wait for bar code scanning
 *
 * The uppercase 'K' and 'T' make button clicks.
 *
 * These control characters can be present in the control string in any combination and any order
 *
 * The virtual methods table consists of the following functions:
 *  - poll(idx) is called in the loop while waiting; the idx being the number of call
 *  - hdl(mod,buf) is called when an input is done; mod being the entry mode, buffer contains the input
 *
 * \remark
 * In poll idx wrapps around.
 * If poll returns 0 the waiting loop is stopped.
 * Positive value means continue.
 *
 * \remark
 * hdl('s',buf) can be called to provide the status of the Input
 * In this case buf contains the status.
 * For example, for tSelector it is a cardinal containing the current state of selector
 * \sa guiInputSelect    
 *
 * If no input is done the hdl is called also with mod=0 and empty buf
 *
 * The function hdl can also modify buf; normally it returns mod; however it can be changed also.
 *
 * \return number of shapes in the drawing
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\gui.c
 * \test tcab0160.c
 */
int guiInputInit(tInput * inp, tDrawing * drw, word x, word y, const char *ctl,
                 const tVmtGui * vmt, byte dly) {
    VERIFY(inp);
    VERIFY(drw);
    VERIFY(ctl);
    VERIFY(strlen(ctl) <= sizeof(inp->ctl));
    trcFN("guiInputInit dim=%d\n", drw->dim);

    inp->drw = drw;
    inp->x = x;
    inp->y = y;
    inp->dly = dly;
    strcpy(inp->ctl, ctl);
    if(!vmt)
        vmt = &_vmt;
    inp->vmt = vmt;

    return inp->drw->dim;
}

static int clickButton(tDrawing * drw, char val) {
    int ret;
    word idx, dim;
    tButton *btn;

    VERIFY(drw);
    VERIFY(val);

    dim = drwDim(drw);
    for (idx = 0; idx < dim; idx++) {
        if(drw->shp[idx].key == val) {
            if(drw->shp[idx].typ == shpButton)
                break;
        }
        if(drw->shp[idx].typ == shpDrawing) {
            ret = clickButton(drw->shp[idx].ptr.drw, val);
            if(ret)
                return ret;
        }
    }
    if(idx >= dim)
        return 0;

    btn = drw->shp[idx].ptr.btn;
    ret = btnPush(btn);
    CHECK(ret >= 0, lblKO);
    if(_guiClick)
        Click();
    ret = btnState(btn, 'n');
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

/** Wait for user input
 * \param inp (I) Pointer to the input descriptor
 * \param num (I) number of shapes to redraw
 * \param buf (O) buffer containing the input information depending on the input type
 *
 * \pre 
 *  - inp!=0
 *  - buf!=0
 *
 * The buffer is filled according to the input:
 *  - 'k','K'[keyboard]: only one byte: key pressed
 *  - 't','T'[touch screen]: 3 elements are filled:
 *    - card loc: x and y coordinates of the touch point packed into a card (see guiGetTftLoc)
 *    - tShape *shp: pointer to the touched shape (see guiGetTftShp)
 *    - char key: associated key if any; otherwise zero (see guiGetTftKey)
 *  - 'm'[magnetic stripe card swipe]: 3 tracks read, each one of 128 bytes
 *  - 'c': wait for chip card insertion: ATR of the card, up to 256 bytes
 *  - 'b': wait for bar code scanning: bar code scanned, up to 256 bytes
 *
 * \return input mode used
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\gui.c
 * \test tcab0160.c
 */
int guiInputWait(tInput * inp, byte num, tBuffer * buf) {
    int ret;
    card idx;
    char c;
    const char *ptr;
    int mag = -1;
    int icc = -1;
    int kbd = -1;

#ifdef __BCR__
    int bcr = -1;
#endif
#ifdef __TFT__
    int tft = -1;
#endif
    VERIFY(inp);
    VERIFY(buf);
    trcS("guiInputWait\n");

    bufReset(buf);

    ret = drwDrawingRefresh(inp->drw, num); //only the topmost shape is redrawn
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    if(!inp->dly)
        goto lblEnd;

    ptr = inp->ctl;
    while(*ptr) {
        c = *ptr++;
        switch (c) {
          case 'm':
              mag = magStart();
              if(mag > 0)
                  mag = c;
              else
                  mag = 0;
              break;
          case 'c':
              icc = iccStart(0);
              if(icc > 0)
                  icc = c;
              else
                  icc = 0;
              break;
          case 'k':
          case 'K':
              kbd = kbdStart(1);
              if(kbd > 0)
                  kbd = c;
              else
                  kbd = 0;
              break;
          case 'b':
#ifdef __BCR__
              bcr = bcrReq();
              if(bcr > 0)
                  bcr = c;
              else
                  bcr = 0;
#endif
              break;
          case 't':
          case 'T':
#ifdef __TFT__
              tft = tftStart();
              if(tft > 0)
                  tft = c;
              else
                  tft = 0;
#endif
              break;
          default:
              break;
        }
    }

    ret = tmrStart(0, inp->dly * 100);
    CHECK(ret >= 0, lblKO);
    idx = 0;
    while(tmrGet(0)) {
        if(inp->vmt->poll) {
            ret = inp->vmt->poll(idx++);
            CHECK(ret >= 0, lblKO);
            if(!ret)
                break;
        }
        if(mag > 0) {           //do we process magstripe?
            char tmp[128 * 3];

            VERIFY(bufDim(buf) >= 128 * 3);

            ret = magGet(tmp, tmp + 128, tmp + 256);
            CHECK(ret >= 0, lblKO);
            if(ret) {
                ret = bufApp(buf, (byte *) tmp, 128 * 3);
                CHECK(ret >= 0, lblKO);
                ret = mag;
                goto lblEnd;
            }
        }
        if(icc > 0) {           //do we process chip?
            byte tmp[260];

            ret = iccCommand(0, (byte *) 0, (byte *) 0, tmp);   //try reset
            if(ret == -iccCardRemoved)
                ret = 0;        //removed=inserted
            if(ret == -iccDriverPb) {   //driver works fine?
                ret = 0;
                icc = -1;       //don't ask for icc any more
            } else if(ret < 0) {
                ret = -iccReadFailure;
                goto lblEnd;
            }
            if(ret) {
                ret = bufApp(buf, tmp, ret);
                CHECK(ret >= 0, lblKO);
                ret = icc;      //chip card inserted
                goto lblEnd;
            }
        }
#ifdef __BCR__
        if(bcr > 0) {
            char tmp[256];

            memset(tmp, 0, 256);
            ret = bcrGet(tmp);
            CHECK(ret >= 0, lblKO);
            if(ret) {
                ret = bufApp(buf, (byte *) tmp, ret);
                CHECK(ret >= 0, lblKO);
                ret = bcr;
                goto lblEnd;
            }
        }
#endif
        if(kbd > 0) {
            char k;

            ret = kbdKey();
            CHECK(ret != '\xFF', lblKO);
            if(ret) {
                k = (byte) ret;
                if(kbd == 'K') {
                    ret = clickButton(inp->drw, k);
                    CHECK(ret >= 0, lblKO);
                }
                ret = bufApp(buf, (byte *) & k, sizeof(byte));
                CHECK(ret >= 0, lblKO);
                ret = kbd;
                goto lblEnd;
            }
        }
#ifdef __TFT__
        if(tft > 0) {
            ret = tftGet();
            CHECK(ret >= 0, lblKO);
            if(ret) {
                card loc;
                const tShape *shp;
                char k;

                k = tftState();
                while(k != 'u') {
                    ret = tftGet();
                    CHECK(ret >= 0, lblKO);
                    k = tftState();
                }
                loc = ret;
                k = 0;

                ret = bufApp(buf, (byte *) & loc, sizeof(loc));
                CHECK(ret >= 0, lblKO);

                shp = drwDrawingShape(inp->drw, HWORD(loc), LWORD(loc));
                ret = bufApp(buf, (byte *) & shp, sizeof(shp));
                CHECK(ret >= 0, lblKO);

                if(shp)
                    k = shp->key;

                if(k) {
                    if(tft == 'T') {
                        ret = clickButton(inp->drw, k);
                        CHECK(ret >= 0, lblKO);
                    }
                }
                ret = bufApp(buf, (byte *) & k, sizeof(byte));
                CHECK(ret >= 0, lblKO);

                ret = tft;
                goto lblEnd;
            }
        }
#endif
    }

    ret = 0;                    //exit due timeout
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    if(mag >= 0)
        magStop();
    if(icc >= 0)
        iccStop(0);
#ifdef __BCR__
    if(bcr >= 0)
        bcrCancel();
#endif
#ifdef __TFT__
    if(tft >= 0)
        tftStop();
#endif
    if(kbd >= 0)
        kbdStop();
    tmrStop(0);
    trcFN("guiInputWait ret=%d\n", ret);
    if(ret >= 0 && inp->vmt->hdl) {
        ret = inp->vmt->hdl((char) ret, buf);
    }
    return ret;
}

card guiGetTftLoc(const tBuffer * buf) {
    const byte *ptr;
    card loc;

    VERIFY(buf);
    VERIFY(bufLen(buf) >= sizeof(card));
    ptr = bufPtr(buf);
    memcpy(&loc, ptr, sizeof(card));
    return loc;
}

tShape *guiGetTftShp(const tBuffer * buf) {
    const byte *ptr;
    tShape *shp;

    VERIFY(buf);
    VERIFY(bufLen(buf) >= sizeof(card) + sizeof(tShape *));
    ptr = bufPtr(buf);
    ptr += sizeof(card);        //skip loc
    memcpy(&shp, ptr, sizeof(shp));
    return shp;
}

char guiGetTftKey(const tBuffer * buf) {
    const byte *ptr;
    char key;

    VERIFY(buf);
    VERIFY(bufLen(buf) >= sizeof(card) + sizeof(tShape *) + sizeof(char));
    ptr = bufPtr(buf);
    ptr += sizeof(card);        //skip loc
    ptr += sizeof(tShape *);    //skip shape pointer
    memcpy(&key, ptr, sizeof(key));
    return key;
}

/** Enter data using input form
 *
 * \param inp (I) Pointer to the input descriptor
 * \param cnt (I) Input container
 * \param key (I) Input data element index within the container cnt
 * \param spc (I) Set of special terminator characters (zero terminated)
 * \param maxLen (I) maximal length of input string; if zero, the length of input container is used
 *
 * \remark Only the top shape is refreshed after each key press
 *
 * The dialog is terminated if kbdVAL or kbdANN or one of special characters is pressed.
 * In case of kbdANN the original value is restored in the container.
 *
 * \pre 
 *  - inp!=0; the control string should be base on keyboard or/and touch screen entry modes
 *  - cnt!=0; the container should have cntGet function (it cannot be a combo)
 *  - spc!=0; if there is no special characters to be treated the string is empty
 *
 * \return last key pressed or 0 in case of timeout; negative if failure
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\gui.c
 * \test tcab0161.c
*/
int guiInputDialog(tInput * inp, const tContainer * cnt, word key,
                   const char *spc, byte maxLen) {
    int ret;
    byte mod;
    const char *ptr;
    int idx;
    char k;
    char tmp[256 + 1];          //input buffer
    char sav[256 + 1];          //save the initial value of the input container to restore it in case of cancel
    byte dat[256];
    tBuffer buf;
    byte len;

    VERIFY(inp);
    VERIFY(cnt);
    VERIFY(spc);
    trcS("guiInputDialog\n");

    bufInit(&buf, dat, 256);
    ret = cntGet(cnt, key, tmp, 256 + 1);
    CHECK(ret >= 0, lblKO);
    strcpy(sav, tmp);
    idx = strlen(tmp);
    len = (byte) cntLen(cnt, key);
    if(maxLen && (len > maxLen))
        len = maxLen;
    //VERIFY(len <= 256);
    ret = drwDrawingDraw(inp->drw, inp->x, inp->y);
    CHECK(ret >= 0, lblKO);
    mod = 0;
    while(42) {
        ret = guiInputWait(inp, mod, &buf);
        CHECK(ret >= 0, lblKO);
        if(!ret)
            break;              //timeout
        mod = 0;
        ptr = (char *) bufPtr(&buf);
        switch (ret) {
          case 'k':
          case 'K':
              k = *ptr;
              break;
          case 't':
          case 'T':
              k = guiGetTftKey(&buf);
              break;
          default:
              ret = -1;
              goto lblEnd;
        }
        switch (k) {
          case kbdVAL:         //done
              ret = k;
              goto lblEnd;
          case kbdANN:         //restore the initial value from save buffer
              mod = 1;
              ret = cntPutStr(cnt, key, sav);
              CHECK(ret >= 0, lblKO);
              ret = k;
              goto lblEnd;
          case kbdCOR:         //backspace, remove the last character entered
              if(!idx)
                  Beep();       //if nothing to remove, beep
              else {
                  tmp[--idx] = 0;   //remove it
                  mod = 1;
              }
              break;
          case kbdINI:
          default:
              ptr = spc;
              while(*ptr) {
                  if(*ptr++ == k) {
                      ret = (byte) k;
                      goto lblEnd;
                  }
              }
              if(idx >= len)
                  Beep();       //overflow - beep
              else if(k) {
                  tmp[idx++] = k;   //append the character pressed
                  mod = 1;
              }
              break;
        }
        if(mod) {
            ret = cntPutStr(cnt, key, tmp);
            CHECK(ret >= 0, lblKO);
        }
    }

    ret = 0;                    //exit due timeout
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("guiInputDialog ret=%d\n", ret);
    return ret;
}

/** Selector dialog.
 * \param inp (I) Pointer to the input descriptor
 * \param str (I) list of selector items
 * \param pos (I) Initial selector position WORDHL(top,idx)
 * \pre 
 *  - inp!=0
 *  - str!=0
 *
 * The selector position consists of two bytes:
 *  - top: the index of the item displayed at the top
 *  - idx: the index of the current highlighted item
 *
 * The drawing associated with the inp should contain selector shape.
 *
 * If a slider is associated with the selector it is moved also during the navigation.
 *
 * The keys on the keyboard are treated in the following way:
 *  - kbdVAL,kbdJST: select current item highlighted, return the current menu state
 *  - kbdDN,kbdJDN: move the highlighted item down
 *  - kbdUP,kbdJUP: move the highlighted item up
 *  - kbdJLF: move the top item up (scroll up)
 *  - kbdJRG: move the top item down (scroll down)
 *  - numeric keys: move the selector bar to a related item 
 *
 * The special cases are treated as:
 *  - kbdANN: no selection is assumed; return WORDHL(0xFF,kbdANN);
 *  - In case of timeout the return code is 0xFFFF
 *  - Special return codes can be generated by virtual functions WORDHL(0xFF,...)
 *
 * \return 
 *    - the selector position if an item is selected
 *    - number of items if timeout or aborted
 *    - negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\gui.c
 * \test tcab0176.c
*/
int guiInputSelect(tInput * inp, const Pchar * str, word pos) {
    int ret;
    tSelector *sel;
    tSlider *sld;
    byte idx;                   //current item highlighted
    byte top;                   //index of the topmost item displayed
    byte dim;                   //number of items in the list
    byte num;                   //number of the items displayed
    card loc;
    const byte *ptr;
    char k;
    byte dat[256];
    tBuffer buf;

    VERIFY(inp);
    VERIFY(str);

    sel = 0;
    sld = 0;
    dim = drwDim(inp->drw);     //here dim is the number of shapes in input drawing
    for (idx = 0; idx < dim; idx++) {
        switch (inp->drw->shp[idx].typ) {   //capture the uppermost selector
          case shpSelector:
              sel = inp->drw->shp[idx].ptr.sel;
              break;
          default:
              break;
        }
    }
    VERIFY(sel);
    sld = sel->sld;             //save slider pointer to be restored at the end

    dim = 0;
    while(str[dim])
        dim++;                  //now dim is the number of items in selector
    top = HBYTE(pos);
    idx = LBYTE(pos);
    num = selDim(sel);          //num is the number of rows that can be displayed
    if(num > dim)               //small menu
        num = dim;
    if(dim <= num) {            //disable slider, it is not needed
        sel->sld = 0;
    }
    VERIFY(top <= idx);
    VERIFY(idx < dim);
    VERIFY(top + num - 1 < dim);

    ret = drwDrawingDraw(inp->drw, inp->x, inp->y);
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 256);
    while(42) {
        trcFN("top= %d", top);
        trcFN(" idx== %d\n", idx);
        if(inp->vmt->hdl) {
            bufReset(&buf);
            loc = CARDHL(top, idx);
            bufApp(&buf, (byte *) & loc, sizeof(card));
            ret = inp->vmt->hdl('s', &buf);
            CHECK(ret >= 0, lblKO);
        }
        ret = selPos(sel, str, WORDHL(top, idx));
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        ret = guiInputWait(inp, 0, &buf);
        CHECK(ret >= 0, lblKO);
        if(!ret)
            break;              //timeout
        ptr = bufPtr(&buf);
        switch (ret) {
          case 'k':
          case 'K':
              k = *ptr;
              break;
          case 't':
          case 'T':
              k = guiGetTftKey(&buf);
              if(k)
                  break;
              loc = guiGetTftLoc(&buf);
              pos = selWhere(sel, loc);
              VERIFY(pos <= selDim(sel));
              if(pos < num) {
                  VERIFY(top + pos <= dim);
                  idx = (byte) (top + pos);
                  ret = selPos(sel, str, WORDHL(top, idx));
                  CHECK(ret >= 0, lblKO);
                  ret = cnvDraw();
                  CHECK(ret >= 0, lblKO);
                  tmrStart(0, 5);
                  while(tmrGet(0));
                  tmrStop(0);
                  k = kbdVAL;
                  trcFN("pos= %d", pos);
                  trcFN(" top= %d", top);
                  trcFN(" idx== %d\n", idx);
              }
              break;
          default:
              ret = -1;
              goto lblEnd;
        }
        trcFN("k== %c\n", k);
        switch (k) {
          case kbdVAL:
          case kbdJST:
              ret = WORDHL(top, idx);
              goto lblEnd;
          case kbdANN:
              ret = WORDHL(0xFF, kbdANN);
              goto lblEnd;
          case kbdJLF:
              if(top) {
                  top--;
                  if(top + num <= idx)
                      idx--;
              }
              break;
          case kbdJRG:
              if(top + num < dim) {
                  top++;
                  if(idx < top)
                      idx++;
              }
              break;
          case kbdDN:
          case kbdJDN:
              idx++;
              if(idx >= dim) {
                  idx = 0;
                  top = 0;
              } else if(top + num <= idx)
                  top++;
              break;
          case kbdUP:
          case kbdJUP:
              if(idx) {
                  idx--;
                  if(top > idx)
                      top--;
              } else {
                  idx = dim - 1;
                  top = dim - num;
              }
              break;
          case 0xFF:           //special key
              VERIFY(bufLen(&buf) >= 2);
              ptr = bufPtr(&buf);
              k = ptr[1];       //retrieve substituted value
              ret = WORDHL(0xFF, k);
              goto lblEnd;
          default:
              if(k < '1')
                  break;
              if(k > '9')
                  break;
              k -= '1';
              if(k >= dim)
                  break;
              idx = k;
              if(top + num <= idx)
                  top = idx - num + 1;
              if(top > idx)
                  top = idx;
              VERIFY(top <= idx);
              VERIFY(idx < top + num);
              break;
        }
    }

    ret = 0xFFFF;               //exit due timeout
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    if(inp->vmt->hdl) {         //call status handler (to display current item state)
        bufReset(&buf);
        loc = CARDHL(top, idx);
        bufApp(&buf, (byte *) & loc, sizeof(card));
        inp->vmt->hdl('s', &buf);
    }
    if(sld) {                   //restore slider value if it was disabled
        sel->sld = sld;
    }
    trcFN("guiInputSelect idx=%d\n", LBYTE(ret));
    return ret;
}

/** Browser dialog.
 * \param inp (I) Pointer to the input descriptor
 * \param brw (I) pointer to virtual methods table
 * \param pos (I) Initial selector position CARDHL(top,idx)
 * \pre 
 *  - inp!=0
 *  - brw!=0
 *  - all virtual methods in brw defined
 *
 * The selector position consists of two bytes:
 *  - top: the index of the item displayed at the top
 *  - idx: the index of the current highlighted item
 *
 * The drawing associated with the inp should contain selector shape.
 *
 * If a slider is associated with the selector it is moved also during the navigation.
 * If the number of items fits on the screen the slider is disabled
 *
 * The keys on the keyboard are treated in the following way:
 *  - kbdVAL,kbdJST: select current item highlighted, return the current menu state
 *  - '8',kbdDN,kbdJDN: move the highlighted item down
 *  - '2',kbdUP,kbdJUP: move the highlighted item up
 *  - kbdJLF: move the top item up (scroll up)
 *  - kbdJRG: move the top item down (scroll down)
 *  - '1','0': move to the first item (Home)
 *  - '7',kbd00: move to the last item (End)
 *  - '9': show next page (PgDn)
 *  - '3': show previous page (PgUp)
 *
 * The special cases are treated as:
 *  - kbdANN: no selection is assumed; return CARDHL(0xFFFF,kbdANN);
 *  - In case of timeout the return code is 0xFFFFFFFF
 *  - Special return codes can be generated by virtual functions CARDHL(0xFFFF,...)
 *
 * \return 
 *    - the selector position if an item is selected
 *    - number of items if timeout or aborted
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\gui.c
 * \test tcab0194.c
*/
card guiInputBrowse(tInput * inp, const tVmtBrw * brw, card pos) {
    int ret;
    tSelector *sel;
    tSlider *sld;
    word idx;                   //current item highlighted
    word top;                   //index of the topmost item displayed
    word dim;                   //number of items in the list
    byte num;                   //number of the items displayed
    byte row;                   //row counter
    card loc;
    const byte *ptr;
    char k;
    byte dat[256];
    tBuffer buf;

    VERIFY(inp);
    VERIFY(brw);
    VERIFY(brw->dim);
    VERIFY(brw->getTxt);
    VERIFY(brw->getFnt);

    sel = 0;
    sld = 0;
    dim = drwDim(inp->drw);     //here dim is the number of shapes in input drawing
    for (idx = 0; idx < dim; idx++) {
        switch (inp->drw->shp[idx].typ) {   //capture the uppermost selector
          case shpSelector:
              sel = inp->drw->shp[idx].ptr.sel;
              break;
          default:
              break;
        }
    }
    VERIFY(sel);
    sld = sel->sld;             //save slider pointer to be restored at the end

    dim = brw->dim();           //number of items to browse
    top = HWORD(pos);
    idx = LWORD(pos);

    num = selDim(sel);          //num is the number of rows that can be displayed
    if(num > dim)               //small menu
        num = (byte) dim;
    if(dim <= num)              //disable slider, it is not needed
        sel->sld = 0;
    if(top > idx)
        top = idx;
    VERIFY(top <= idx);
    if(dim) {
        VERIFY(idx < dim);      //dim==idx==0 is OK
        if(top + num > dim)
            if(top)
                top--;
    } else {
        VERIFY(top == 0);
        VERIFY(idx == 0);
    }

    ret = drwDrawingDraw(inp->drw, inp->x, inp->y);
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 256);
    while(42) {
        trcFN("top= %d", top);
        trcFN(" idx== %d\n", idx);

        if(inp->vmt->hdl) {     //call status handler (to display current item state)
            bufReset(&buf);
            loc = CARDHL(top, idx);
            bufApp(&buf, (byte *) & loc, sizeof(card));
            ret = inp->vmt->hdl('s', &buf);
            CHECK(ret >= 0, lblKO);
        }

        for (row = 0; row < dspH; row++) {
            byte fnt;

            fnt = 0;
            if(row < num) {
                VERIFY(top + row < dim);
                ret =
                    brw->getTxt(top + row, sel->itm[row],
                                sizeof(sel->itm[row]));
                CHECK(ret >= 0, lblKO);
                ret = brw->getFnt(top + row);
                CHECK(ret >= 0, lblKO);
                fnt = (byte) ret;
            } else {
                memset(sel->itm[row], 0, sizeof(sel->itm[row]));
            }
            ret = drwTextInit(&sel->txt[row], &sel->cnt[row], NULLKEY, fnt, 0); //color is managed by selector
            CHECK(ret >= 0, lblKO);
        }

        ret = brwPos(sel, CARDHL(top, idx), dim);
        CHECK(ret >= 0, lblKO);
        ret = cnvDraw();
        CHECK(ret >= 0, lblKO);

        ret = guiInputWait(inp, 0, &buf);
        CHECK(ret >= 0, lblKO);
        if(!ret)
            break;              //timeout
        ptr = bufPtr(&buf);
        switch (ret) {
          case 'k':
          case 'K':
              k = *ptr;
              break;
          case 't':
          case 'T':
              k = guiGetTftKey(&buf);
              if(k)
                  break;
              loc = guiGetTftLoc(&buf);
              pos = selWhere(sel, loc);
              VERIFY(pos <= selDim(sel));
              if(pos < num) {
                  VERIFY(top + pos <= dim);
                  idx = (word) (top + pos);
                  brwPos(sel, CARDHL(top, idx), dim);
                  ret = cnvDraw();
                  CHECK(ret >= 0, lblKO);
                  tmrStart(0, 5);
                  while(tmrGet(0));
                  tmrStop(0);
                  k = kbdVAL;
                  trcFN("pos= %d", pos);
                  trcFN(" top= %d", top);
                  trcFN(" idx== %d\n", idx);
              }
              break;
          default:
              ret = -1;
              goto lblEnd;
        }
        trcFN("k== %c\n", k);
        switch (k) {
          case kbdVAL:
          case kbdJST:
              ret = CARDHL(top, idx);
              goto lblEnd;
          case kbdANN:
              ret = CARDHL(0xFFFF, kbdANN);
              goto lblEnd;
          case kbdJLF:
              if(!dim)
                  break;
              if(top) {
                  top--;
                  if(top + num <= idx)
                      idx--;
              }
              break;
          case kbdJRG:
              if(!dim)
                  break;
              if(top + num < dim) {
                  top++;
                  if(idx < top)
                      idx++;
              }
              break;
          case kbdDN:
          case kbdJDN:
          case '8':
              if(!dim)
                  break;
              idx++;
              if(idx >= dim) {
                  idx = 0;
                  top = 0;
              } else if(top + num <= idx)
                  top++;
              break;
          case '9':            //page down
              if(!dim)
                  break;
              idx += num;
              top += num;
              if(idx >= dim) {
                  idx = 0;
                  top = 0;
              } else if(top + num >= dim)
                  top = dim - num;
              break;
          case kbdUP:
          case kbdJUP:
          case '2':
              if(!dim)
                  break;
              if(idx) {
                  idx--;
                  if(top > idx)
                      top--;
              } else {
                  idx = dim - 1;
                  top = dim - num;
              }
              break;
          case '3':            //page up
              if(!dim)
                  break;
              if(idx >= num) {
                  idx -= num;
                  if(top >= num)
                      top -= num;
                  else
                      top = 0;
              } else {
                  idx = dim - 1;
                  top = dim - num;
              }
              break;
          case '0':
          case '1':            //Home
              top = 0;
              idx = 0;
              break;
          case kbd00:
          case '7':            //End
              if(!dim)
                  break;
              top = dim - num;
              idx = dim - 1;
              break;
          case '\xFF':         //special key
              VERIFY(bufLen(&buf) >= 2);
              ptr = bufPtr(&buf);
              k = ptr[1];       //retrieve substituted value
              ret = CARDHL(0xFFFF, k);
              goto lblEnd;
          default:
              break;
        }
    }

    ret = 0xFFFFFFFFL;          //exit due timeout
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    if(inp->vmt->hdl) {         //call status handler (to display current item state)
        bufReset(&buf);
        loc = CARDHL(top, idx);
        bufApp(&buf, (byte *) & loc, sizeof(card));
        inp->vmt->hdl('s', &buf);
    }
    if(sld) {                   //restore slider value if it was disabled
        sel->sld = sld;
    }
    trcFN("guiInputBrowse idx=%d\n", LWORD(ret));
    return ret;
}
#endif
