/** \file
 * Dialog functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/pri/dlg.c $
 *
 * $Id: dlg.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include "pri.h"

#undef trcOpt
//static const card trcOpt = TRCOPT & BIT(trcDLG);
#define trcOpt (TRCOPT & BIT(trcDLG))

/** Display a message on the screen and waits for 3 seconds.
 * Can be used as a temporary stub for features that are not implemented yet.
 * \param s (I) Message to be displayed
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
*/
void stub(const char *s) {
    dspClear();
    dspLS(1, "      STUB      ");
    dspLS(2, s);
    Beep();
    tmrPause(3);
}

static byte _dlgClick = 0;      //click after key press?

/** Enable/disable clicking while key pressing in dialogs
 * \param clk (I)
 *  - 0: disable clicking
 *  - 1: enable ckicking
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
*/
void dlgClick(byte clk) {
    _dlgClick = clk;
}

//shortcut for checking the return value
#define CHK CHECK(ret>=0,lblKO)

static int hdrShow(const char *hdr) {   //display menu name
    int ret;
    char tmp[255];

    VERIFY(hdr);

    memset(tmp, 0, 255);
    memcpy(tmp, hdr, dspWdt(0));
    fmtPad(tmp, dspWdt(0), ' ');    //right pad by spaces
    ret = dspLS(BLD(0), tmp);   //display in bold
    CHK;

    return 0;
  lblKO:return -1;
}

 //display current menu state
 // mnu: menu array
 // upp: index of the first menu item to be displayed
 // cur: current menu item infex to be dispalyed in inverted mode
 // dim: number of menu items
static int mnuShow(const Pchar * mnu, byte upp, byte cur, byte dim) {
    int ret;
    char tmp[255];
    byte idx;

    //current menu item should be on the screen
    VERIFY(upp <= cur);
    VERIFY(cur < upp + dim);

    memset(tmp, 0, 255);
    for (idx = 0; idx < dim; idx++) {   //dim lines to display
        memcpy(tmp, mnu[upp + idx], dspWdt(0));
        fmtPad(tmp, dspWdt(0), ' ');    //pad by spaces to have a good menu bar
        ret = dspLS((upp + idx == cur) ? INV(idx + 1) : (idx + 1), tmp);    //the current menu item is inverted
        CHK;
    }
    return 0;
  lblKO:return -1;
}

/** Propose a menu of strings to the user. Allow user to browse the menu and to select an item or cancel.
 *
 * The menu can contain no more than MNUMAX items.
 * The cursor keys can be used to browse the menu.
 * Green key ENTER is used to make the selection, and red key CLEAR is used to abort.
 * The numeric keys '1'..’9’ are used to access directly to a menu item.
 *
 * The state of the menu consists of two parts:
 *  - (sta % MNUMAX) is the current position of the selector
 *  - (sta / MNUMAX) is the number of the upper item displayed on the screen
 * 
 * The initial state is given by the second parameter;
 * the function returns the resulting stated if an item is selected.
 * 
 * Timeout of dly seconds is set.
 * Timer 0 is used.
 * If it is zero, the waiting state is infinite.
 *
 * \param mnu (I) An array of strings containing menu items.
 *        The element number zero is the menu name displayed at the top of the screen.
 *        The last element of this array should be zero pointer
 * \param sta (I) Initial menu state defining the topmost menu item displayed
 *        and the current item selected; if it is zero the first menu item will be at the top
 *        and it will be selected
 * \param dly (I) Timeout in seconds; zero value means infinite loop
 * \pre 
 *    - mnu!=0
 *    - the initial current item should be displayed on the screen
 *    - the number of items in the menu should not be zero
 * \return  
 *    - final state if an item is selected.
 *    - zero if it is cancelled. 
 *    - negative in case of error.
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0025.c
*/
int mnuSelect(const Pchar * mnu, int sta, int dly) {
//mnu: array of strings; mnu[0]= name of menu; other elements - menu items
//sta: menu state, contains upp and cur indexes, see below
//dly: timeout in seconds; zero means infinite
    int ret;                    //return value
    byte dim;                   //the mnu dimension including menu header
    byte upp;                   //number of menu item to be displayed first
    byte cur;                   //number of menu item selected just now
    byte bnd;                   //number of items to be displayed on the screen (band)
    char key;                   //key pressed

    VERIFY(mnu);
    if(!sta)                    //the initial state is not important
        sta = 1 * MNUMAX + 1;

    cur = sta % MNUMAX;
    upp = sta / MNUMAX;
    VERIFY(upp);                //the 0th element if the menu header; so the upper item should be greater than 0
    VERIFY(cur);                //the same concerning the current item
    VERIFY(cur < upp + dspHgt(0));  //the current item should be on the screen

    dim = 0;
    while(mnu[dim]) {           //calculate dim: number of elements in the menu array
        dim++;
        ret = MNUMAX;
        if(dim >= ret)
            break;              //no more than MNUMAX elements are supported
    }
    VERIFY(dim > 1);
    VERIFY(upp < dim);
    VERIFY(cur < dim);

    //calculate bnd: number of items to be displayed on the screen
    bnd = dspHgt(0);            //if there is enough elements it is the screen height
    if(dim < bnd)
        bnd = dim;              //otherwise, it is the number of items in the menu
    bnd--;                      //don't forget that the first element of the mnu array is the header string

    ret = dspClear();
    CHK;
    ret = hdrShow(mnu[0]);
    CHK;                        //display menu name
    while(1) {
        VERIFY(upp <= cur);     //loop condition: current item should be displayed
        VERIFY(cur < upp + bnd);

        ret = mnuShow(mnu, upp, cur, bnd);  //display current menu state
        CHK;

        //wait for user input
        if(dly) {
            ret = tmrStart(0, dly * 100);
            CHK;
        }
        ret = kbdStart(1);
        CHK;
        key = 0;
        while((dly == 0) || tmrGet(0)) {    //timer is checked only if dly is not zero
            key = kbdKey();     //capture a key
            if(key)
                break;
        }
        kbdStop();
        if(tmrGet(0))
            if(_dlgClick)
                Click();
        CHK;
        if(dly)
            tmrStop(0);

        switch (key) {          //analyse the key pressed
          case 0:
              return 0;         //timeout
          case kbdVAL:         //green key ENTER - current menu item selected
          case kbdF6:
              return upp * MNUMAX + cur;    //return the menu state
          case kbdANN:
              return 0;         //red key CLEAR - menu selection aborted
          case kbdDN:          //DOWN arrow key - move cursor down
          case kbdF8:
              cur++;
              if(cur >= dim) {  //wrapping from the end to the beginning if needed
                  cur = 1;
                  upp = 1;
              } else if(upp + bnd == cur)
                  upp++;        //follow upp if cur is at the bottom
              break;
          case kbdUP:          //UP arrow key - move cursor up
          case kbdF4:
              cur--;
              if(!cur) {        //wrapping from the beginning to the end if needed
                  cur = dim - 1;
                  if(upp + bnd <= cur)
                      upp = cur - bnd + 1;
              } else if(cur == upp - 1) //follow upp if cur is at the top
                  upp = cur;
              break;
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':            //numeric key: direct selecting
              key -= '0';
              if(key > dim - 1) {   //if there is not enough menu items - beep
                  Beep();
                  break;
              }
              cur = (byte) key; //adjust screen to put current item on the top
              upp = cur;
              if(upp + bnd > dim)
                  upp = dim - bnd;  //adjust upp to display as many items as possible
              break;
          default:
              Beep();
              break;            //key not accepted
        }
    }
  lblKO:return -1;
}

//copy into dst the last dspWdt(0) characters of src
static int getDspTail(char *dst, const char *src) {
    byte len;

    VERIFY(dst);
    VERIFY(src);

    len = strlen(src);
    if(len <= dspWdt(0)) {
        strcpy(dst, src);
    } else {
        fmtSbs(dst, src, (byte) (len - dspWdt(0)), dspWdt(0));
    }
    return strlen(dst);
}

static void dspLLSS(byte loc, const char *str) {
    byte idx;
    byte len;
    char tmp[255];

    VERIFY(str);

    idx = 0;
    len = strlen(str);
    while((loc & 0x0F) < dspWdt(0)) {   //mask out attributes
        fmtSbs(tmp, str, idx, dspWdt(0));
        dspLS(loc, str + idx);
        loc++;
        idx += dspWdt(0);
        if(idx >= len)
            break;
    }
}

static void dspLLSS2(byte loc, const char *str) {
    byte idx;
    byte len;
    char tmp[255];

    VERIFY(str);

    idx = 0;
    len = strlen(str);
    while((loc & 0x0F) < dspWdt(0)) {   //mask out attributes
        fmtSbs(tmp, str, idx, dspWdt(0));
        dspLS(loc, tmp);
        loc++;
        idx += dspWdt(0);
        if(idx > len || loc >= dspHgt(0))
            break;
    }
}

//enterFmtStr: enter formatted string
// loc: line number where the input string is displayed
// str: string entered. At the beginning it contains initial value. The resulting string is saved here
// fmt: format control character, can take the following values:
//  '*': entering passwords; the characters entered are echoed as '*'
//  '.': entering host IP address; every 4th character would be "."
//  0xFF: echo without formatting
//  0, 2 or 3: echo formatted amount with related exponent 
//len: buffer str size including ending zero
#define MAXLEN 255
static int enterFmtStr(byte loc, char *str, byte fmt, byte len) {
    int idx;
    char key;                   //key pressed
    char buf[MAXLEN + 1];       //contains the current result
    char tmp[MAXLEN + 1];       //contains the formatted string to display
    byte padLen = 16;

    VERIFY(loc < dspHgt(0));    //line number should be on the screen
    VERIFY(str);
    VERIFY(strlen(str) <= (word) (len + 1));    //too long initial strings are not acepted
    VERIFY(len);

    memset(buf, 0, MAXLEN + 1);
    strcpy(buf, str);           //the initial string will be modified only in case of ENTER is pressed
    while(42) {
        switch (fmt) {
          case 0xFF:           //get the last dspWdt(0) characters
              getDspTail(tmp, buf);
              break;
          case ';':            //no formatting, just copy
              strcpy(tmp, buf);
              break;
          case '.':            //TCP-IP Address entering
              memset(tmp, 0, dspWdt(0));
              fmtTcp(tmp, buf, strlen(buf));
              break;
          case '*':            //password entering
              memset(tmp, '*', dspWdt(0));  //the displayed string contains stars
              VERIFY(strlen(buf) <= dspWdt(0));
              tmp[strlen(buf)] = 0;
              break;
          case '%':            //password with leading string eg. PWD:****
              memset(tmp, '*', dspWdt(0));
              memcpy(tmp, str, strlen(str));    //the rest of the displayed string contains stars
              //VERIFY(strlen(buf) <= dspWdt(0));
              if(strlen(buf) < strlen(str))
                  strcpy(buf, str); // don't delete the original string
              tmp[strlen(buf)] = 0;
              break;
          case '=':            //normal entry with leading string eg. Quantity: 1234
              getDspTail(tmp, buf);
              memcpy(tmp, str, strlen(str));    //the rest of the displayed string contains stars
              //VERIFY(strlen(buf) <= dspWdt(0));
              if(strlen(buf) < strlen(str))
                  strcpy(buf, str); // don't delete the original string
              tmp[strlen(buf)] = 0;
              break;
          case '/':
              memset(tmp, 0, dspWdt(0));
              fmtDate(tmp, buf, strlen(buf));
              break;
          case ':':
              memset(tmp, 0, dspWdt(0));
              fmtTime(tmp, buf, strlen(buf));
              break;
          case 0:
          case 2:
          case 3:              //amount entering
              while(strlen(buf) > (byte) (fmt + 1)) {   //discard leading zeroes
                  if(*buf != '0')
                      break;
                  memmove(buf, buf + 1, strlen(buf));
              }
              fmtAmt(tmp, buf, fmt, ",."); //@agmr - BRI
//              fmtAmt(tmp, buf, fmt, ".,");      
              fmtPad(tmp, -padLen, ' ');
              VERIFY(strlen(tmp) <= dspWdt(0));
              break;
          default:
              VERIFY(fmt < 4);
              break;
        }
        dspLLSS(loc, tmp);      //display formatted string

        tmrStart(0, 60 * 100);  //60 seconds timeout for user reaction
        kbdStart(1);
        key = 0;
        while(tmrGet(0)) {
            key = kbdKey();     //retrieve the key pressed if any
            if(key)
                break;          //quit the loop if the key is pressed
        }
        kbdStop();
        if(tmrGet(0))
            if(_dlgClick)
                Click();
        tmrStop(0);
        idx = strlen(buf);      //the current position is always at the end of the string
        switch (key) {
          case 0:
              return 0;         //timeout; the initial string is not modified
          case kbdF1: //@agmr 
          case kbdANN:
              return key;       //aborted; the initial string is not modified  
          case kbdF4: //@agmr 
          case kbdVAL:         //the entering is finished
              VERIFY(strlen(buf) <= len);
              strcpy(str, buf); //now we can change the initial string
              return key;
          case kbdCOR:         //backspace, remove the last character entered
              if(!idx)
                  Beep();       //if nothing to remove, beep
              else
                  buf[--idx] = 0;   //remove it
              break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':            //a valid key typed
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
              if(idx >= len - 1)
                  Beep();       //overflow - beep
              else
                  buf[idx++] = key; //append the character pressed
              break;
          case kbd00:
              if(idx >= len - 2)
                  Beep();       //overflow - beep
              else {
                  buf[idx++] = '0';
                  buf[idx++] = '0';
              }
              break;
          default:
              Beep();
              break;            //invalid key pressed
        }
    }
}

/** 
 * \todo To be documented
*/
//enterFmtStrFarsi: enter formatted string
// loc: line number where the input string is displayed
// str: string entered. At the beginning it contains initial value. The resulting string is saved here
// fmt: format control character, can take the following values:
//  '*': entering passwords; the characters entered are echoed as '*'
//  0xFF: echo without formatting
//  0, 2 or 3: echo formatted amount with related exponent 
//len: buffer str size including ending zero
int enterFmtStrFarsi(byte loc, char *str, byte fmt, byte len) {
    int idx;
    int key;                    //key pressed
    char buf[MAXLEN + 1];       //contains the current result
    char tmp[MAXLEN + 1];       //contains the formatted string to display

    VERIFY(loc < dspHgt(0));    //line number should be on the screen
    VERIFY(str);
    VERIFY(strlen(str) <= (word) (len + 1));    //too long initial strings are not acepted
    VERIFY(len);

    memset(buf, 0, MAXLEN + 1);
    strcpy(buf, str);           //the initial string will be modified only in case of ENTER is pressed
    while(1) {
        switch (fmt) {
          case 0xFF:           //get the last dspWdt(0) characters
              getDspTail(tmp, buf);
              break;
          case '/':            //no formatting, just copy
              strcpy(tmp, buf);
              break;
          case '*':            //password entering
              memset(tmp, '*', dspWdt(0));  //the displayed string contains stars
              VERIFY(strlen(buf) <= dspWdt(0));
              tmp[strlen(buf)] = 0;
              break;
          case 0:
          case 2:
          case 3:              //amount entering
              fmtAmt(tmp, buf, fmt, 0);
              fmtPad(tmp, -dspWdt(0), ' '); //left pad the amount
              VERIFY(strlen(tmp) <= dspWdt(0));
              break;
          default:
              VERIFY(fmt < 4);
              break;
        }
        dspLLSS(loc, tmp);      //display formatted string

        tmrStart(0, 60 * 100);  //60 seconds timeout for user reaction
        kbdStart(1);
        key = 0;
        while(tmrGet(0)) {
            key = kbdKey();     //retrieve the key pressed if any
            if(key)
                break;          //quit the loop if the key is pressed
        }
        kbdStop();
        if(tmrGet(0))
            if(_dlgClick)
                Click();
        tmrStop(0);
        idx = strlen(buf);      //the current position is always at the end of the string

        switch (key) {
          case 0:
              return 0;         //timeout; the initial string is not modified
          case kbdANN:
              return key;       //aborted; the initial string is not modified
          case kbdVAL:         //the entering is finished
              VERIFY(strlen(buf) <= len);
              strcpy(str, buf); //now we can change the initial string
              return key;
          case kbdCOR:         //backspace, remove the last character entered
              if(!idx)
                  Beep();       //if nothing to remove, beep
              else
                  buf[--idx] = 0;   //remove it
              break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':            //a valid key typed
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
              if(idx >= len - 1)
                  Beep();       //overflow - beep
              else {
                  key = key + 80;
                  buf[idx++] = key; //append the character pressed
              }
              break;
          case kbd00:
              if(idx >= len - 2)
                  Beep();       //overflow - beep
              else {
                  buf[idx++] = '0';
                  buf[idx++] = '0';
              }
              break;
          default:
              Beep();
              break;            //invalid key pressed
        }
    }
}

/** Process user input. The string str to be entered is displayed on the line loc of the screen.
 * Up to len-1 characters can be entered.
 * If the length of the input string is greater than screen width only the last dspW characters
 * entered are displayed.
 
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 *
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param str (I) Initial string the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string
 *
 * \pre
 *    - str!=0
 *    - loc<dspH
 *
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN) or 0 in case of timeout.
 *    - negative in case of error
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0026.c
*/
int enterStr(byte loc, char *str, byte len) {
    VERIFY(str);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, str, 0xFF, len);
}

/** Process user input. The string str to be entered is displayed on the line loc of the screen.
 * Up to len-1 characters can be entered.
 * If the length of the input string is greater than screen width only the last dspW characters
 * entered are displayed.
 
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 *
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param str (I) Initial string the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string
 *
 * \pre
 *    - str!=0
 *    - loc<dspH
 *
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN) or 0 in case of timeout.
 *    - negative in case of error
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcts0003.c
*/
int enterStr2(byte loc, char *str, byte len) {
    VERIFY(str);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, str, '=', len);
}

/** Process user input.
 * The string str to be entered is displayed on the line loc of the screen.
 * Up to len-1 characters can be entered.
 * If the length of the input string is greater than screen width the rest of the string is displayed
 * on the length line etc.
 
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 *
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param       str (I) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string
 *
 * \pre
 *    - str!=0
 *    - loc<dspH
 *
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN) or 0 in case of timeout.
 *    - negative in case of error
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0026.c
*/
int enterPhr(byte loc, char *str, byte len) {
    VERIFY(str);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, str, ';', len);
}

/** Process password input.
 * The string str to be entered is displayed on the line loc of the screen.
 * Up to len-1 characters can be entered.
 * 
 * The characters entered are echoed as ‘*’.
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 * 
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param str (I) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string
 * 
 * \pre
 *    - str!=0
 *    - loc<dspH
 *    - len<=dspW+1
 *
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN) or 0 in case of timeout; negative in case of error
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0026.c
*/
int enterPwd(byte loc, char *str, byte len) {
    VERIFY(str);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    VERIFY(len <= dspWdt(0) + 1);
    return enterFmtStr(loc, str, (byte) '*', len);
}

/** Process password input.
 * The string str to be entered is displayed on the line loc of the screen.
 * Up to len-1 characters can be entered.
 * 
 * The characters entered are echoed as ‘*’.
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 * 
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param str (I) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string
 * 
 * \pre
 *    - str!=0
 *    - loc<dspH
 *    - len<=dspW+1
 *
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN) or 0 in case of timeout; negative in case of error
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcts0003.c
*/
int enterPwd2(byte loc, char *str, byte len) {
    VERIFY(str);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    VERIFY(len <= dspWdt(0) + 1);
    return enterFmtStr(loc, str, (byte) '%', len);
}

/** Process amount input.
 * The string amt to be entered is displayed on the line loc of the screen.
 * Up to 12 digits are allowed in amount.
 * After pressing a key the amount is formatted according to the exp parameter and displayed.
 * 
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 *
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param amt (I) Initial string; the result string will be copied here if ENTER was pressed
 * \param exp (I) consists of two parts:
 *   - exp/10 is the maximal amount length (zero is interpreted as 12 for backward compatibility)
 *   - exp%10 is the number of decimals in the amount: 0 or 2 or 3
 *
 * \pre 
 *    - str!=0
 *    - loc<dspH
 *    - exp==0 or exp==2 or exp==3
 *
 * \return
 *    - the last key pressed (kbdVAL or kbdANN).
 *    - 0 in case of timeout.
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0026.c
*/
int enterAmt(byte loc, char *amt, byte exp) {
    byte len;

    VERIFY(amt);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    len = exp / 10;
    if(!len)
        len = 9;               //for compatibility //change by @@AR-SIMAS
    VERIFY(len);
    exp %= 10;
    VERIFY(exp == 0 || exp == 2 || exp == 3);
    return enterFmtStr(loc, amt, exp, (byte) (len + 1));    //maximum amount length = len digits
}

/** Process IP address input.
 * The string buf to be entered is displayed on the line loc of the screen.
 * Up to 12 digits are allowed in address, the dots are displayed but they are not in the buffer
 * After pressing a key the address is formatted and displayed.
 * 
 * Only the digits are accepted.
 * The last character can be erase using the yellow key.
 * The red key CLEAR is used to abort the operation (the initial string value is not changed).
 * The green key ENTER is used to accept the current string.
 *
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param buf (I) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) length of the initial str without null pointer
 *
 * \pre 
 *    - buf!=0
 *    - loc<dspH
 *
 * \return
 *    - the last key pressed (kbdVAL or kbdANN).
 *    - 0 in case of timeout.
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0026.c
*/

int enterTcp(byte loc, char *buf, byte len) {
    VERIFY(buf);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, buf, '.', (byte) (len + 1));    //maximum amount length = len digits
}

/**
 * \todo To be documented
*/
int enterData(byte loc, char *amt, byte len) {  // input from right, with variable length
    VERIFY(amt);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, amt, 0, (byte) (len + 1));
}

/** Process alphabetical input like authorisation code or password input.
 * The string str to be entered is displayed on the line loc of the screen.
 * Up to len-1 characters can be entered.
 *
 * Digits and capital letters are accepted according to fmt.
 * The last character can be erased using the yellow key.
 * The red key CLEAR is used to abort the operation.
 * The green key ENTER is used to accept the current string.
 *
 * After pressing one digit, there is one second waiting to change it to one capital letter
 * by pressing the same digit.
 * If in one second no key is pressed it will wait for the next user input.
 *
 * The last key pressed is returned.
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number
 * \param str (I) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string
 * \param fmt (I) Displaying format of character. It can be '*' for password input or 0xff for clear text.
 *
 * \pre 
 *    - str!=0
 *    - loc<dspH
 *
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN).
 *    - 0 in case of timeout. 
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcnl0007.c
*/
int enterTxt(byte loc, char *str, byte len, byte fmt) {
    int ret;
    int idx = 0;
    int lenStr;
    byte start = 0;
    int letterInd = 0;
    int idxTemp = 0;
    byte key;                   //key pressed
    byte preKey = 0x7D;         //Previous Key
    byte ind;
    char buf[MAXLEN + 1] = "";  //contains the current result
    char tmp[MAXLEN + 1] = "";  //contains the formatted string to display
    char letGroup[26] = "";
    char isBig = 1;             // it is capital; default is capital
    char pwdshow[MAXLEN + 1] = "";  //contains the formatted string to display for Pwd fmt

    const char *letterGrb[] = {
        "0.",
        "1 ",
        "2ABC",
        "3DEF",
        "4GHI",
        "5JKL",
        "6MNO",
        "7PQRS",
        "8TUV",
        "9WXYZ",
        "",
        "",
        "<#-",
        "",
        "",
        "?*,",
    };
    const char *letterLrb[] = {
        "0.",
        "1 !@",
        "2abc",
        "3def",
        "4ghi",
        "5jkl",
        "6mno",
        "7pqrs",
        "8tuv",
        "9wxyz",
        "",
        "",
        "<#-",
        "",
        "",
        "?*,",
        "",
        "",
        "",
        "?*,",
        "",
        "<#-",
    };

    VERIFY(loc < dspH);         //line number should be on the screen
    VERIFY(str);
    VERIFY(strlen(str) <= (word) (len + 1));    //too long initial strings are not acepted
    VERIFY(len);

    memset(buf, 0, MAXLEN + 1);
    strcpy(buf, str);           //the initial string will be modified only in case of ENTER is pressed

    // if str in not null , i can display that
    strcpy(tmp, buf);           //the initial string will be modified only in case of ENTER is pressed
    if(tmp[0] != 0)
        idxTemp = strlen(tmp);
    while(1) {
        switch (fmt) {
          case 0xFF:           //get the last dspW characters   
              //display '_' to wait for the next one
              if((tmp[0] != 0) && tmp[idxTemp] != '_') {
                  tmp[strlen(tmp)] = '_';
                  tmp[strlen(tmp) + 1] = 0;
              }
              break;
          case '*':            //password entering                         
              // SD added
              memset(pwdshow, 0, sizeof(pwdshow));
              ret = strlen(tmp);
              if(tmp[0] == '_' || ret == 0)
                  strcpy(pwdshow, "");
              else {

                  fmtPad(pwdshow, ret, '*');
              }
              //display '_' to wait for first charachter or the next one to be entered
              if(tmp[strlen(tmp) - 1] != '_') {
                  tmp[strlen(tmp)] = '_';
                  if(strlen(pwdshow))
                      pwdshow[strlen(pwdshow)] = '_';
              }
              tmp[strlen(tmp) + 1] = 0;
              pwdshow[strlen(pwdshow) + 1] = 0;
              //  }
              break;
        }

        letterInd = 0;
        if(fmt == '*')
            dspLLSS(loc, pwdshow);  //display formatted string for pwd
        else
            dspLLSS(loc, tmp);  //display formatted string                

        //wait for user input
        ret = tmrStart(0, 60 * 100);
        CHK;                    //one minute timeout
        ret = kbdStart(1);
        CHK;
        key = 0;
        while(tmrGet(0)) {
            key = kbdKey();     //capture a key
            if(key)
                break;
        }
        ret = kbdStop();
        CHK;
        if(tmrGet(0))
            Click();
        tmrStop(0);

        ind = key - '0';
        idx = strlen(buf);
        tmrStart(1, 160);       //1.6 sec
        //the current position is always at the end of the string       
        while(tmrGet(1)) {
            switch (key) {
              case 0:          // if no charachter pressed in 1.2 seconds ,start a timer for next character
                  //timeout; the initial string is not modified
                  tmrStop(1);
                  break;
              case kbdANN:
                  return key;   //aborted; the initial string is not modified
              case kbdVAL:     //the entering is finished
                  VERIFY(strlen(buf) <= len);
                  //save last key pressed
                  buf[idx] = tmp[idxTemp];

                  //for fmt '*' changed last character  before enterinf val key to '*'
                  if((tmp[idxTemp] != '*') && (fmt == '*'))
                      tmp[idxTemp] = '*';

                  //For each Fmt delete '_' if there is from buffer
                  if(buf[strlen(buf) - 1] == '_') {
                      buf[strlen(buf) - 1] = 0;
                      tmp[strlen(tmp) - 1] = 0;
                  }
                  // SD added
                  memset(pwdshow, 0, sizeof(pwdshow));
                  ret = strlen(tmp);
                  if(ret)
                      fmtPad(pwdshow, ret, '*');

                  if(fmt == '*')
                      dspLLSS(loc, pwdshow);
                  else
                      dspLLSS(loc, tmp);

                  strcpy(str, buf); //now we can change the initial string                 
                  return key;
              case kbdCOR:     //backspace, remove the last character entered                 
                  if(!idx) {
                      tmrStop(1);
                      Beep();
                  } else {
                      if(tmp[idxTemp] != '_') {
                          tmp[idxTemp] = '_';   //remove it                     
                          //buf[idx]=0;
                      } else {
                          tmp[idxTemp] = 0; //remove '-' and the previous character
                          idxTemp--;
                          tmp[idxTemp] = 0;
                          buf[idx - 1] = 0;
                          idx--;
                          //buf[idx]=0;//remove last char of buf                          
                      }
                      if(idx < 0)
                          idx = 0;
                      if(idxTemp < 0)
                          idxTemp = 0;
                      preKey = key;
                      dspLS(3, "            ");
                      tmrStop(1);
                  }
                  start = 1;
                  break;
              case kbdF3:
              case kbdUP:      //Add for ICT familly
                  isBig++;
                  if(isBig % 2 == 0)
                      isBig = 0;    //it is small 
                  else
                      isBig = 1;
                  break;
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':        //a valid key typed
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
              case '?':
              case '<':
              case kbdF4:
              case kbdF6:
                  if(idxTemp >= len - 1)
                      Beep();
                  else {

                      dspLLSS((byte) (dspH - 1), "               ");
                      memset(letGroup, 0, sizeof(letGroup));
                      if(isBig) {
                          lenStr = strlen(&letterGrb[ind][0]);
                          memcpy(letGroup, &letterGrb[ind][0], lenStr);
                      } else {  // if menu is pressed , it is ready to get small letter
                          lenStr = strlen(&letterLrb[ind][0]);
                          memcpy(letGroup, &letterLrb[ind][0], lenStr);
                      }

                      fmtPad(letGroup, -15, ' ');
                      dspLLSS((byte) (dspH - 1), letGroup);
                      if(key == preKey) {
                          if((letterInd) >= (lenStr - 1))
                              letterInd = 0;
                          else
                              letterInd++;
                          if(isBig)
                              tmp[idxTemp] = letterGrb[ind][letterInd];
                          else
                              tmp[idxTemp] = letterLrb[ind][letterInd];

                      } else {
                          tmp[idxTemp] = key;
                          letterInd = 0;    //for next time                        
                      }
                      if(fmt == '*') {
                          memset(pwdshow, 0, sizeof(pwdshow));
                          ret = strlen(tmp);
                          if(ret != 1)
                              fmtPad(pwdshow, ret - 1, '*');
                          pwdshow[ret - 1] = tmp[ret - 1];
                          dspLLSS(loc, (char *) pwdshow);
                      } else
                          dspLLSS(loc, (char *) tmp);   //display formatted string tmp    

                      preKey = key;
                  }
                  break;
              default:
                  Beep();
                  break;        //invalid key pressed
            }                   //end switch
            if(start == 1) {
                //ready to get new character from input
                start = 0;
                break;
            }
            ret = kbdStart(1);
            CHK;
            while(tmrGet(1)) {
                key = kbdKey(); //capture a key
                if(key) {
                    ind = key - '0';
                    break;
                }
            }
            ret = kbdStop();
            CHK;
            if(!tmrGet(1))      //it means user has confirmed this key
            {
                preKey = 0x7D;
                tmrStop(1);
                if(idx >= len - 1)
                    Beep();     //overflow - beep
                else {
                    if((tmp[idxTemp] != '_') /*&&(tmp[idxTemp] !='*') */ )
                        buf[idx] = tmp[idxTemp];    //append the character pressed                  
                }
                if((tmp[idxTemp] != 0)
                   && (tmp[idxTemp] != '_') /*&&(tmp[idxTemp] !='*') */ ) {
                    idxTemp++;
                    idx++;
                }
                break;          //next key
            }
        }                       //while
    }                           //while

  lblKO:return -1;
}

static char iccType = 0;

/** Process card input. The application waits for an external event: chip card inserted
 * or magnetic stripe card swiped or a key pressed or 1-minute timeout according to control string
 * ctl settings.
 *
 * The control string contains characters 'c', 'i', 'm', 'k', 'b', or 'h' to indicate what type of input is accepted.
 *
 * If an EMV chip card is inserted the ATR is saved into buf and the value 'c' is returned.
 * If a non-EMV ISO7816 chip card is inserted the ATR is saved into buf and the value 'i' is returned.
 *
 * If a magnetic stripe card is swiped the track1 is saved into buf+0, the track2 into buf+128 
 * and the track3 into buf+256.
 *
 * If a bar code was read with the bar code reader read the bar code as zero terminated string
 * 
 * If an eN-Check 2500 is scanned, the check data is saved into buf and value 'h' is returned.
 * 
 * If a key is pressed it is saved into buf and the value 'k' is returned
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param buf (I) Buffer to capture the input (character or ATR or 3 tracks)
 * \param ctl (I) control string containing 'c','i','k','m','b','h' to indicate what type of input is accepted
 *
 * \pre 
 *    - buf!=0; the size of the buffer should be not less that 128*3
 *    - ctl!=0
 *    - the options 'i' and 'c' are mutually exclusive
 *
 * \return  
 *    - 'k' if a key is pressed.
 *    - 'm' if a card is swiped.
 *    - 'c' if an EMV card is inserted.
 *    - 'i' if an ISO7816 non EMV card is inserted.
 *    - 'b' if a bar code was scanned.
 *    - 'h' if eN-Check 2500 is swiped.
 *    -  0 in case of timeout.
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test 
 *  - tcab0027.c
 *  - tcab023.c
 */
int enterCard(byte * buf, const char *ctl) {
    int ret, mag, icc, kbd;

#ifdef __BCR__
    int bcr;
#endif
#ifdef __CHECKREADER2500__
    int chk;
#endif

    VERIFY(buf);
    VERIFY(ctl);

    memset(buf, 0, 128 * 3);
    mag = -1;
    icc = -1;
    kbd = -1;
    iccType = 0;
#ifdef __BCR__
    bcr = -1;
#endif
#ifdef __CHECKREADER2500__
    chk = -1;
#endif
    ret = tmrStart(0, 60 * 100);
    CHK;
    while(*ctl) {
        switch (*ctl++) {
          case 'm':
              mag = magStart();
              break;
          case 'c':
              icc = iccStart(0);
              iccType = 'c';
              break;
          case 'i':
              icc = iccStart(0x10);
              iccType = 'i';
              break;
          case 'k':
              kbd = 1;
              break;
#ifdef __BCR__
          case 'b':
              bcr = bcrReq();
              break;
#endif
#ifdef __CHECKREADER2500__
          case 'h':
              chk = chkStart("7E11200");
              break;
#endif
          default:
              break;
        }
    }
    ret = kbdStart(1);
    CHK;

    while(tmrGet(0)) {
        if(icc >= 0) {          //we process chip?
            ret = iccCommand(0, (byte *) 0, (byte *) 0, buf);   //try reset

            if(ret == -iccCardRemoved) {    //removed=not inserted
                ret = 0;
            } else if(ret == -iccDriverPb) {    //driver works fine?
                ret = 0;
                icc = -1;       //don't ask for icc any more  
            }
#ifndef __HYBRDR__
            else if(ret < 0) {

                ret = -iccReadFailure;
                goto lblEnd;
            }
#else
            if(ret < 0) {
                removeCard();
            }
#endif
            if(ret > 0) {       //chip card inserted
                ret = iccType;
                break;
            }
        }

        if(mag >= 0) {          //we process magstripe?
            ret = magGet((char *) buf, (char *) buf + 128, (char *) buf + 256);
            CHK;                //is the mag card swiped?
            if(ret) {
                ret = 'm';      //yes, finished
                break;
            }
        }
#ifdef __BCR__
        if(bcr >= 0) {
            ret = bcrGet((char *) buf);
            CHK;
            if(ret) {
                ret = 'b';
                break;
            }
        }
#endif
#ifdef __CHECKREADER2500__
        if(chk >= 0) {
            ret = chkGet((char *) buf);
            CHK;
            if(ret) {
                ret = 'h';
                break;
            }
        }
#endif
        ret = kbdKey();         //now check keyboard
        CHECK(ret != '\xFF', lblKO);    //errors are not acceptable        
        if(ret) {               //key pressed
            switch (ret) {
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                  if(kbd < 0) {
                      Beep();
                      ret = kbdStart(1);
                      CHK;
                      continue;
                  }
                  *buf = ret;   //put it into buffer
                  ret = 'k';
                  break;
              case kbdANN:
                  *buf = ret;   //put it into buffer
                  ret = 'k';
                  goto lblEnd;
              default:
                  Beep();
                  kbdStop();
                  ret = kbdStart(1);
                  CHK;
                  continue;
            }
            break;
        }
    }
    if(!tmrGet(0))
        ret = 0;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    tmrStop(0);
    if(mag >= 0)
        magStop();
    if(icc >= 0)
        iccStop(0);
#ifdef __BCR__
    if(bcr >= 0)
        bcrCancel();
#endif
#ifdef __CHECKREADER2500__
    if(chk >= 0)
        chkStop();
#endif
    kbdStop();
    return ret;
}

/** Wait until the chip card is removed
 * \return N/A
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0027.c
 */
void removeCard(void) {
    int ret;

    trcS("removeCard\n");
    ret = iccStart((iccType == 'c') ? 0 : 0x10);
    if(ret < 0)
        return;

    do {
        ret = iccCommand(0, (byte *) 0, (byte *) 0, (byte *) 0);
    } while(ret > 0);
    iccStop(0);
}

void DetectCard(void) {
    int ret;

    ret = iccStart(0);
    if(ret < 0)
        return;

    do {
        ret = iccDetect(0);
    } while(ret > 0);
    iccStop(0);
}

/** This function is similar to enterTxt. 
 * The parameters of functions and the return values are the same as we can read in the program
 * references. 
 * In the usage are some differences between two functions and some bugs of the enterTxt function
 * are fixed. 
 * The str parameter of enterTxt2 function can be empty string.
 * In case of input of the numbers (0-9) the position will be automatically the next
 * until the length of string will be len  (this is the maximum of the length). 
 * After pressing a key (e.g. 2) then pressing key F1 (down) again and again the abc2ab...
 * or pressing key F2 (up) again and again the ABC2AB... are cyclical repeating.
 * If __TXT_HUN__ or __TXT_BUL__ definded, with F3 key can change the input characters 
 * between Latin1 and Latin2 (hungarian) or Latin1 and Cyrillic (bulgarian) characters
 * The signs are offered to key 1, key PaperFeed and key 00. 
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number of display
 * \param str (IO) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string (maximum length)
 * \param fmt (I) Displaying format of character. It can be '*' for password input or
 *                0xFF for clear text.
 *
 * \pre 
 *    - loc<dspH
 *    - strlen(str)<=(dspH-loc)*dspW+1)
 *    - len>0
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN).
 *    - 0 in case of timeout. 
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcbb0000.c
*/
int enterTxt2(byte loc, char *str, byte len, byte fmt) {
    int ret;                    //return value
    int idx;                    //index of str
    int idc = 0;                //index of the circuit
    int const tmo = 6000;       //time-out over 60 seconds
    char key = 0;               //the actual key
    char last = 0;              //the last pressed key (before the actual)
    int end = 0;                //input end
    int itr = 0;                //interrupt
    byte sln;                   //length of the str
    char buf[255];              //buffer for '*'
    int cyrillic = 0;           //flag for cyrillic characters
    int hungarian = 0;          //flag for cyrillic characters

    const char *letterGrb[] = {
        "0 ",
        "1.,!",
        "2ABC",
        "3DEF",
        "4GHI",
        "5JKL",
        "6MNO",
        "7PQRS",
        "8TUV",
        "9WXYZ",
        "",                     //OK
        "",                     //Esc
        "<#-",                  //00
        "",                     //Backspace
        "",                     //
        "?*;",                  //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    const char *letterGrbHun[] = {
        "0 ",
        "1.,!",
        "2AÁBC",
        "3DEÉF",
        "4GHIÍ",
        "5JKL",
        "6MNOÓÖÕ",
        "7PQRS",
        "8TUÚÜÛV",
        "9WXYZ",
        "",                     //OK
        "",                     //Esc
        "<#-",                  //00
        "",                     //Backspace
        "",                     //
        "?*;",                  //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    const char *letterGrbCyrillic[] = {
        "0 ",
        "1ÌÎÏ",
        "2°±²",
        "3³´E",
        "4¶·¸",
        "5¹º»",
        "6M½¾",
        "7¿ÀÁÂ",
        "8ÃÄÅÆ",
        "9ÇÈÉÊ",
        "",                     //OK
        "",                     //Esc
        "<#-!+",                //00
        "",                     //Backspace
        "",                     //
        ".,?*;",                //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    const char *letterLrb[] = {
        "0 ",
        "1.,!",
        "2abc",
        "3def",
        "4ghi",
        "5jkl",
        "6mno",
        "7pqrs",
        "8tuv",
        "9wxyz",
        "",                     //OK
        "",                     //Esc
        "<#-",                  //00
        "",                     //Backspace
        "",                     //
        "?*;",                  //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    const char *letterLrbHun[] = {
        "0 ",
        "1.,!",
        "2aábc",
        "3deéf",
        "4ghií",
        "5jkl",
        "6mnoóöõ",
        "7pqrs",
        "8tuúüûv",
        "9wxyz",
        "",                     //OK
        "",                     //Esc
        "<#-",                  //00
        "",                     //Backspace
        "",                     //
        "?*;",                  //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    const char *letterLrbCyrillic[] = {
        "0 ",
        "1ìîï",
        "2aÑÒ",
        "3ÓÔe",
        "4Ö×Ø",
        "5ÙÚÛ",
        "6ÜÝÞ",
        "7ßàáâ",
        "8ãäåæ",
        "9çèéê",
        "",                     //OK
        "",                     //Esc
        "<#-!+",                //00
        "",                     //Backspace
        "",                     //
        ".,?*;",                //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    sln = strlen(str);
    idx = sln;

    VERIFY(loc < dspHgt(0));    //line number should be on the screen
    VERIFY(strlen(str) <= (word) (len + 1));    //too long initial strings are not acepted
    VERIFY(len);
    VERIFY(len <= (dspHgt(0) - loc) * dspWdt(0) + 1);   //the len is too big according to loc

    if(fmt == '*') {
        memset(buf, 0, len + 1);
        if(idx > 0) {
            memset(buf, '*', idx - 1);
            buf[idx - 1] = str[idx - 1];
        }
    }

    if(sln == 0) {
        memset(str, 0, len + 1);
    } else {
        if(fmt == '*') {
            buf[idx] = str[idx];
            dspLLSS2(loc, buf);
        } else {
            dspLLSS2(loc, str);
        }
    }

    do {
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);
        ret = tmrStart(0, tmo);
        CHECK(ret >= 0, lblKO);
        key = 0;
        while(tmrGet(0) && !key) {
            key = kbdKey();
            CHECK(key != '\xFF', lblKO);
        }

        kbdStop();
        if(tmrGet(0))
            if(_dlgClick)
                Click();

        if(!tmrGet(0)) {
            goto lblTmo;
        }

        tmrStop(0);

        if(idx < len && key >= '0' && key <= '9') { //numbers
            str[idx] = key;
            if(fmt == '*') {
                buf[idx] = key;
                if(idx > 0)
                    buf[idx - 1] = '*';
            }
            last = key;
            idx++;
            idc = 1;
        } else {
            switch (key) {
              case kbdVAL:     //OK
                  end = 1;
                  if(fmt == '*' && idx > 0)
                      buf[idx - 1] = '*';
                  break;
              case kbdANN:     //Esc
                  itr = 1;
                  break;
              case kbdFWD:     //PaperFeed
              case kbd00:      //00
                  if(idx < len) {
                      if(cyrillic == 0 && hungarian == 0) {
                          str[idx] = letterLrb[key - 48][0];
                      } else {
                          if(hungarian == 1) {
                              str[idx] = letterLrbHun[key - 48][0];
                          } else {
                              str[idx] = letterLrbCyrillic[key - 48][0];
                          }
                      }
                      if(fmt == '*') {
                          if(cyrillic == 0 && hungarian == 0) {
                              buf[idx] = letterLrb[key - 48][0];
                          } else {
                              if(hungarian == 1) {
                                  buf[idx] = letterLrbHun[key - 48][0];
                              } else {
                                  buf[idx] = letterLrbCyrillic[key - 48][0];
                              }
                          }
                          if(idx > 0)
                              buf[idx - 1] = '*';
                      }
                      last = key;
                      idx++;
                      idc = 1;
                  }
                  break;
              case kbdCOR:     //Backspace
                  if(idx > 0) {
                      idx--;
                      str[idx] = 0;
                      if(fmt == '*') {
                          buf[idx] = 0;
                          if(idx > 0)
                              buf[idx - 1] = str[idx - 1];
                      }
                  }
                  break;
              case kbdF1:      //F1 Down, now lower case
                  if((last >= '0' && last <= '9') || last == '<' || last == '?') {
                      idx--;
                      if(cyrillic == 0 && hungarian == 0) {
                          str[idx] = letterLrb[last - 48][idc];
                          buf[idx] = letterLrb[last - 48][idc];
                          if(letterLrb[last - 48][idc + 1] == 0) {
                              idc = 0;
                          } else {
                              idc++;
                          }
                      } else {
                          if(hungarian == 1) {
                              str[idx] = letterLrbHun[last - 48][idc];
                              buf[idx] = letterLrbHun[last - 48][idc];
                              if(letterLrbHun[last - 48][idc + 1] == 0) {
                                  idc = 0;
                              } else {
                                  idc++;
                              }
                          } else {
                              str[idx] = letterLrbCyrillic[last - 48][idc];
                              buf[idx] = letterLrbCyrillic[last - 48][idc];
                              if(letterLrbCyrillic[last - 48][idc + 1] == 0) {
                                  idc = 0;
                              } else {
                                  idc++;
                              }
                          }
                      }
                      idx++;
                  }
                  break;
              case kbdF2:      //F2 Up, now capital letters
                  if((last >= '0' && last <= '9') || last == '<' || last == '?') {
                      idx--;
                      if(cyrillic == 0 && hungarian == 0) {
                          str[idx] = letterGrb[last - 48][idc];
                          buf[idx] = letterGrb[last - 48][idc];
                          if(letterGrb[last - 48][idc + 1] == 0) {
                              idc = 0;
                          } else {
                              idc++;
                          }
                      } else {
                          if(hungarian == 1) {
                              str[idx] = letterGrbHun[last - 48][idc];
                              buf[idx] = letterGrbHun[last - 48][idc];
                              if(letterGrbHun[last - 48][idc + 1] == 0) {
                                  idc = 0;
                              } else {
                                  idc++;
                              }
                          } else {
                              str[idx] = letterGrbCyrillic[last - 48][idc];
                              buf[idx] = letterGrbCyrillic[last - 48][idc];
                              if(letterGrbCyrillic[last - 48][idc + 1] == 0) {
                                  idc = 0;
                              } else {
                                  idc++;
                              }
                          }
                      }
                      idx++;
                  }
                  break;
              case kbdF3:      //F3 Change between Latin1 and Cyrillic or Latin1 and Latin2 (hungarian) characters
#ifdef __TXT_HUN__
                  if(hungarian == 0) {
                      hungarian = 1;
                  } else {
                      hungarian = 0;
                  }
                  idc = 1;
#endif
#ifdef __TXT_BUL__
                  if(cyrillic == 0) {
                      cyrillic = 1;
                  } else {
                      cyrillic = 0;
                  }
                  idc = 1;
#endif
                  break;
            }
        }

        if(fmt == '*') {
            dspLLSS2(loc, buf);
        } else {
            dspLLSS2(loc, str);
        }

    }
    while(!end && !itr);

    if(itr)
        goto lblEnd;
    goto lblEnd;

  lblTmo:                      //Time-out
    tmrStop(0);
    return 0;
  lblKO:                       //KO
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    return -1;
  lblEnd:                      //OK or Esc
    return key;
}

/** This function is similar to enterTxt2. 
 * The parameters of functions and the return values are the same as we can read in the program
 * references. 
 * In the usage are some differences between two functions and some bugs of the enterTxt function
 * are fixed. 
 * The str parameter of enterTxt3 function also can be empty string.
 * In case of input of the numbers (0-9) the position will be automatically the next
 * until the length of string will be len  (this is the maximum of the length). 
 * After pressing the menu key (F3) again and again the 2ABCabc... are cyclical repeating.
 * Press F1 or F2 and you can move with a cursor. If then press a key it will be inserted. 
 * The signs are offered to key 1, key PaperFeed and key 00. 
 * Timeout of 60 seconds is set. Timer 0 is used.
 *
 * \param loc (I) Input line number of display
 * \param str (IO) Initial string; the result string will be copied here if ENTER was pressed
 * \param len (I) Size of destination buffer string (maximum length)
 * \param fmt (I) Displaying format of character. It can be '*' for password input or
 *                0xFF for clear text.
 *
 * \pre 
 *    - loc<dspH
 *    - strlen(str)<=(dspH-loc)*dspW+1)
 *    - len>0
 * \return  
 *    - the last key pressed (kbdVAL or kbdANN).
 *    - 0 in case of timeout. 
 *    - negative in case of error.
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcgs0016.c
*/
int enterTxt3(byte loc, char *str, byte len, byte fmt) {
    int ret;                    //return value
    int idx;                    //index of str
    int idc = 0;                //index of the circuit
    int idr;                    //index of the cursor
    int const tmo = 6000;       //time-out over 60 seconds
    char key = 0;               //the actual key
    char last = 0;              //the last pressed key (before the actual)
    char inv = 0;               //the invisible character
    int end = 0;                //input end
    int itr = 0;                //interrupt
    byte sln;                   //length of the str
    char buf[dspW * dspH + 1];  //buffer for '*'
    int i;

    const char *letterGrb[] = {
        "0 ",
        "1/\\+<>&^",
        "2ABCabc",
        "3DEFdef",
        "4GHIghi",
        "5JKLjkl",
        "6MNOmno",
        "7PQRSpqrs",
        "8TUVtuv",
        "9WXYZwxyz",
        "",                     //OK
        "",                     //Esc
        ".,!<#-",               //00
        "",                     //Backspace
        "",                     //
        "?*;$@|",               //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    sln = strlen(str);
    idx = sln;
    idr = sln;

    VERIFY(loc < dspH);         //line number should be on the screen
    VERIFY(strlen(str) <= (word) (len + 1));    //too long initial strings are not acepted
    VERIFY(len);
    VERIFY(len <= (dspH - loc) * dspW + 1); //the len is too big according to loc

    if(fmt == '*') {
        memset(buf, 0, len + 1);
        if(idx > 0) {
            memset(buf, '*', idx - 1);
            buf[idx - 1] = str[idx - 1];
        }
    }

    if(sln == 0) {
        memset(str, 0, len + 1);
    } else {
        if(fmt == '*') {
            buf[idx] = str[idx];
            dspLLSS2(loc, buf);
        } else {
            dspLLSS2(loc, str);
        }
    }

    do {
        ret = kbdStart(1);
        CHECK(ret >= 0, lblKO);
        ret = tmrStart(0, tmo);
        CHECK(ret >= 0, lblKO);
        key = 0;
        while(tmrGet(0) && !key) {
            key = kbdKey();
            CHECK(key != '\xFF', lblKO);
        }

        ret = kbdStop();
        CHECK(ret >= 0, lblKO);

        if(!tmrGet(0)) {
            goto lblTmo;
        }

        tmrStop(0);

        //new character to the end of string
        if(idx < len && key != kbdF1 && key != kbdF2 && key != kbdF3
           && key != kbdANN && key != kbdCOR && key != kbdVAL) {
            if(idx == idr) {
                str[idx] = key;
                if(fmt == '*') {
                    buf[idx] = key;
                    if(idx > 0)
                        buf[idx - 1] = '*';
                }
                last = key;
                idr++;
                idx++;
                str[idx] = 0;
                buf[idx] = 0;
                idc = 1;
            }
            //insert character to the cursor
            else {
                for (i = idx; i >= idr; i--) {
                    str[i + 1] = str[i];
                    buf[i + 1] = buf[i];
                }
                str[idr] = key;
                if(fmt == '*') {
                    buf[idr] = key;
                    if(idr > 0)
                        buf[idr - 1] = '*';
                }
                last = key;
                idr++;
                idx++;
                idc = 1;
            }
        } else {
            switch (key) {
              case kbdANN:     //Esc
                  itr = 1;
                  break;
              case kbdCOR:     //Backspace
                  if(idx > 0 && idx == idr) {
                      idx--;
                      idr--;
                      str[idx] = 0;
                      if(fmt == '*') {
                          buf[idx] = 0;
                          if(idx > 0)
                              buf[idx - 1] = str[idx - 1];
                      }
                  } else {
                      if(idr < idx && idr > 0) {
                          for (i = idr; i <= idx; i++) {
                              str[i] = str[i + 1];
                              buf[i] = buf[i + 1];
                          }
                          idr--;
                          idx--;
                          str[idr] = '_';
                          buf[idr] = '_';
                          buf[idr - 1] = str[idr - 1];
                      }
                  }
                  break;
              case kbdVAL:     //OK
                  end = 1;
                  if(fmt == '*' && idx > 0)
                      buf[idx - 1] = '*';
                  if(idr < idx) {
                      str[idr] = inv;
                      buf[idr] = inv;
                  }
                  break;
              case kbdF1:      //F1 Down, now back, left step
                  if(idr > 0) {
                      idr--;
                      str[idr + 1] = inv;
                      inv = str[idr];
                      str[idr] = '_';
                      buf[idr] = '_';
                      buf[idr - 1] = str[idr - 1];
                      if(idr + 1 < idx)
                          buf[idr + 1] = '*';
                  }
                  idc = 0;
                  last = 0;
                  break;
              case kbdF2:      //F2 Up, now forward, right step
                  if(idr < idx) {
                      idr++;
                      str[idr - 1] = inv;
                      inv = str[idr];
                      buf[idr - 2] = '*';
                      buf[idr - 1] = str[idr - 1];
                      if(idr < idx) {
                          str[idr] = '_';
                          buf[idr] = '_';
                      }
                  }
                  idc = 0;
                  last = 0;
                  break;
              case kbdF3:      //F3 Menu, at first capital letters, then lower case        
                  if(last != 0) {
                      idx--;
                      str[idr - 1] = letterGrb[last - 48][idc];
                      buf[idr - 1] = letterGrb[last - 48][idc];
                      idx++;
                      if(letterGrb[last - 48][idc + 1] == 0) {
                          idc = 0;
                      } else {
                          idc++;
                      }
                  }
                  break;
            }
        }

        if(fmt == '*') {
            dspLLSS2(loc, buf);
        } else {
            dspLLSS2(loc, str);
        }

    }
    while(!end && !itr);

    if(itr)
        goto lblEnd;
    goto lblEnd;

  lblTmo:                      //Time-out
    tmrStop(0);
    return 0;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    return -1;
  lblEnd:                      //OK or Esc
    return key;
}

static int infoDisplay(tInfo * inf) {
    int ret;
    byte loc;

    VERIFY(inf);
    for (loc = 0; loc < dspHgt(0); loc++) {
        if(!inf->msg[loc][0])
            continue;
        ret = dspLS(loc, inf->msg[loc]);
        CHK;
    }
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static int waitS(int dly) {
    int ret;
    char key;

    if(!dly)
        return 0;
    key = 0;

    ret = kbdStart(1);
    CHK;
    ret = tmrStart(0, dly * 100);
    CHK;
    while(tmrGet(0)) {
        key = kbdKey();
        if(key != 0)
            break;
    }

    ret = key;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
    goto lblEnd;
  lblEnd:
    tmrStop(0);
    kbdStop();
    return ret;
}

static int infoS(tInfo * inf) {
    int ret;

    VERIFY(inf);

    ret = infoDisplay(inf);
    CHK;
    return waitS(inf->dly);
  lblKO:
    trcErr(ret);
    return -1;
}

static int waitB(int dly) {
    int ret;
    char key;

    VERIFY(dly);

    Beep();
    Beep();
    Beep();
    ret = kbdStart(1);
    CHK;
    ret = tmrStart(0, dly * 100);
    CHK;
    ret = tmrStart(1, 100);
    CHK;
    while(1) {
        key = kbdKey();
        if(key != 0)
            break;
        //Click();
        if(tmrGet(0) == 0)
            break;
        if(tmrGet(1) != 0)
            continue;
        tmrStop(1);
        //Beep();
        ret = tmrStart(1, 100);
        CHK;
        kbdStop();
        kbdStart(1);
    }
    tmrStop(1);
    tmrStop(0);
    kbdStop();
    return key;
  lblKO:
    trcErr(ret);
    return -1;
}

static int infoB(tInfo * inf) {
    int ret;

    VERIFY(inf);
    ret = infoDisplay(inf);
    CHK;
    return waitB(inf->dly);
  lblKO:
    trcErr(ret);
    return -1;
}

/** Display an info screen
 * \param inf (I) info screen descriptor
 *
 * Depending on inf->cmd the behaviour is:
 *  - 's': display the four lines on the screen and wait for inf-dly seconds
 *  - 'b': do the same and beep while waiting
 *  - 'i': ignore, don't display nothing
 *
 * If during waiting a key is pressed the function returns the code of the key pressed.
 * \return
 *  - 0 if no key is pressed during inf->dly seconds
 *  - the code of the key pressed if any
 *  - negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\dlg.c
 * \test tcab0028.c
*/
int info(tInfo * inf) {
    VERIFY(inf);
    switch (inf->cmd) {
      case 's':
          return infoS(inf);
      case 'b':
          return infoB(inf);
      case 'i':
      default:
          break;
    }
    VERIFY(inf->cmd == 'i');
    return 0;
}

int enterDate(byte loc, char *buf, byte len) {
    VERIFY(buf);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, buf, '/', (byte) (len + 1));
}

int enterTime(byte loc, char *buf, byte len) {
    VERIFY(buf);
    //loc = dspHgt(0) / 2 - 2 + loc;
    VERIFY(loc < dspHgt(0));
    return enterFmtStr(loc, buf, ':', (byte) (len + 1));
}
