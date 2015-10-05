/** \file
 * SYS group components
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/sys.h $
 *
 * $Id: sys.h 2616 2010-04-07 11:16:41Z abarantsev $
 */

#ifndef __SYS_H

/** \defgroup sysgroup SYS System components
 *
 * The system component functions are declared in the header file sys.h in sys directory.
 * They are implemented in the files:
 *  - TBX.C: (ToolBoX) tracing and conversion functions,
 *    contains methods and simple supplementary functions of generic usage.
 *    Part of them is enabled in simulation mode only.
 *  - I32.C: (Ingenico 32 bits) wrappers around U32 functions.
 *    These are the 32-specific implementations of functions declared in sys.h
 *    These are the 16-specific implementations of functions declared in sys.h
 *  - E32.C: (Emv 32 bits) wrappers around EMV Kernel.
 *  - C32.C: (Crypto 32 bits) wrappers around SSA calls.
 *  - CHN.C: (CHaNnel) communication CHaNnel functions for UNICAPT32; deprecated, will be replaced by GCL module
 *  - GCL.C: (Generic Communication Library) communication processing based on GCL library
 *  - MEM.C: (MEMory) non-volatile memory treatment (DFS, SRAM)
 *  - HMI.C: (Human Machine Interface) treatment
 *  - EXT.C: (EXTernal) external interfaces (BCR, MFC, CLS etc)
 *
 * The header file EMV.H contains EMV-related enums, in particular, all the EMV tag definitions
 *
 * The header file DEF.H contains the application contexts.
 *
 * @{
 */

#include "def.h"

/** \weakgroup SYSTYP typedef shortcuts
 * The variables types byte, word and card contain 8, 16 and 32 bits everywhere:
 * under KEIL, ARM, Borland, Visual and GNU
 * @{
 */
#ifndef __BASEARM_DEF_H_INCLUDED__
#ifdef __TELIUM__
#ifndef _BASEARM_H
typedef unsigned char byte;     ///< 0x00..0xFF
typedef unsigned short word;    ///< 0x0000..0xFFFF*
#endif //_BASEARM_H
#else //__TELIUM__
typedef unsigned char byte;     ///< 0x00..0xFF
typedef unsigned short word;    ///< 0x0000..0xFFFF*
#endif //__TELIUM__
#endif //__BASEARM_DEF_H_INCLUDED__
typedef unsigned long card;     ///< 0x00000000..0xFFFFFFFF

/** @} */

/** \weakgroup SYSBWC byte-word-card manipulation macros
 * @{
 */
///Combine two bytes into a word
#define WORDHL(H,L) ((word)((((word)H)<<8)|((word)L)))

///Extract highest byte from a word
#define HBYTE(W) (byte)(((word)W&0xFF00)>>8)

///Extract lowest byte from a word
#define LBYTE(W) (byte)((word)W&0x00FF)

///Combine two words into a card
#define CARDHL(H,L) ((card)((((card)H)<<16)|((card)L)))

///Extract highest word from a card
#define HWORD(C) (word)(((card)C&0xFFFF0000UL)>>16)

///Extract lowest word from a card
#define LWORD(C) (word)((card)C&0x0000FFFFUL)
/** @} */

#define Beep utaBeep

/** \weakgroup SYSERRMAC Elementary error processing macros
 * file to be equal to __FILE__. It should be done to avoid string duplication.
 * @{
 */
/** Check a condition COND. If it is wrong, the error processing is performed, then jump to LBL label.
 * \param CND Condition to be verified
 * \param LBL label to jump to if the condition is wrong
 * \remark The label LBL must point to a set of error processing operations.
 * \header sys\\sys.h
 * \test tcab0000.c
 *
 * The macro is used to check exceptional situations like invalid user input, resource exhausted etc,
 * in the same way as the construction try(){...}catch{}is used.
 * The effect of this macro is the same in simulation and non-simulation mode.
 */
#define CHECK(CND,LBL) {if(!(CND)){errChk(#CND,__FILE__,(word)__LINE__); goto LBL;}}

/** Check a condition CND. If it is wrong, stop execution.
 * \param CND Condition to be verified
 * \remark In case of invalid condition the execution is stopped under simulation without visible effect.
 *         See the trace file DEBUG.TRC to find out the reason.
 *         In real terminal mode the condition is not checked.
 *         The result of bug is inpredictable.
 * \header sys\\sys.h
 * This macro is used to verify pre- and post- conditions of a series of operations in simulation mode only.
 * It checks the condition, and if it is not violated the normal processing continues.
 * Otherwise, the condition violated is traced and the execution is stopped.
 *
 * This macro is targeted to catch software bugs.
 * It is disabled when the application is downloaded in the terminal.
 * The run-time exception processing should be performed using CHECK() macro.
*/
#ifdef __TEST__
#define VERIFY(CND) {if(!(CND)){errVrf(#CND,__FILE__,(word)__LINE__);}}
#else
#define VERIFY(CND) {}
#endif

/** @} */

/** \addtogroup tbxgroup TBX ToolBoX
 * TBX component contains the functions of general usage.
 * @{
*/

/** \weakgroup SYSPTR Pointer shortcuts
 * The following functions return pointers to constant values of given type.
 *
 * For example, bPtr(2) returns a pointer to a byte that contains the value 2.
 * @{
 */
const byte *bPtr(byte val);     ///< return the pointer to a byte containing the value val
const word *wPtr(word val);     ///< return the pointer to a word containing the value val
const card *cPtr(card val);     ///< return the pointer to a card containing the value val
const byte *dPtr(const char *s);    ///< return the pointer to a binary data

/** @} */

/** \weakgroup SYSERRFUN Elementary error processing functions
 * file to be equal to __FILE__. It should be done to avoid string duplication.
 *
 * The error processing functions use the following parameters:
 *  - cond: condition string provided by #C macro
 *  - file: source file name provided by __FILE__ macro
 *  - line: source line number provided by __LINE__ macro
 * @{
*/

void errChk(const char *cond, const char *file, word line); ///<trace error and continue

#ifdef __TEST__
void errVrf(const char *cond, const char *file, word line); ///<trace error and stop
#endif
/** @} */

/** \weakgroup SYSCVTFUN Elementary conversion functions
 * The conversion functions below operate with the following data types:
 *  - num: a numeric value of type card, for example, 1234 or 0x1234
 *  - dec: a string containing only decimal characters, for example, "1234"
 *  - hex: a string containing only hexadecimal characters, for example, "12AB"
 *  - bin: a string containing any characters, for example, "\x12\x34"
 *
 * The return value is the number of bytes processed if OK.
 *
 * In case of error it is zero
 *
 * The destination is always the first parameter
 *
 * The second parameter is the source
 *
 * The last parameter is the length of a string (without ending zero) or
 * the length of a binary buffer.
 *
 * The zero value of len means that it should be calculated as strlen(source)
 * or number of digits in source etc according the situation
 *
 * The argument len is the length of source for the functions ...2num and bin2...
 * dec2num,hex2num,bin2hex,bin2num
 *
 * It is the length of destination for functions num2... and ...2bin
 *  num2dec,num2hex,hex2bin,num2bin
 * @{
*/

byte num2dec(char *dec, card num, byte len);    ///< numeric value num --> decimal string dec of length len
byte dec2num(card * num, const char *dec, byte len);    ///< decimal string dec of length len --> numeric value num
byte num2hex(char *hex, card num, byte len);    ///< numeric value num --> hexadecimal string hex of length len
byte hex2num(card * num, const char *hex, byte len);    ///< hexadecimal string hex of length len --> numeric value num
int bin2hex(char *hex, const byte * bin, int len);  ///< binary buffer bin of length len --> hexadecimal string hex
int hex2bin(byte * bin, const char *hex, int len);  ///< hexadecimal string hex --> binary buffer bin of length len
byte bin2num(card * num, const byte * bin, byte len);   ///< binary buffer bin of length len --> numeric value num using direct byte order
byte num2bin(byte * bin, card num, byte len);   ///< numeric value num --> binary buffer bin of length len using direct byte order

/** @} */

/** \weakgroup SYSBIT Bit operations
 * The leftmost bit of buf has number 1
 *
 * The rightmost bit of the first byte of buf has number 8
 *
 * The leftmost bit of the second byte of buf has number 9
 *
 * etc
 * @{
*/
void bitOn(byte * buf, byte idx);   ///< turn on bit idx in buf
void bitOff(byte * buf, byte idx);  ///< turn off bit idx in buf
byte bitTest(const byte * buf, byte idx);   ///< test the state of bit idx in buf
void tbxParity(byte * buf, word dim, byte par, byte typ);   ///<set parity bits
byte tbxCheckParity(byte * buf, word dim, byte par, byte typ);  ///<check parity bits

/** @} */

/** \weakgroup SYSBUF Safe buffer structure
 * This structure is used to fill a memory in a safe manner.
 * The memory buffer is reserved before calling the constructor bufInit() function.
 * When filling this memory space a checking is performed whether there is enough space available
 * The structure members should be never accessed directly (treated like private class members).
 * A set of functions are provided to perform the operations on a buffer
 *
 *The parameter convention in the functions below:
 * - buf: pointer to sBuffer structure to perform an operation
 * - dat: pointer to data to be copied inside the buffer
 * - len: length of the data to be copied; two values have special meaning:
 *   - len==-1 means: strlen(dat) bytes will be copied
 *   - len==0 means: same as -1 and one more byte set by zero
 *   - pos moved by strlen(dat) bytes in both cases
 * - num: number of bytes to be set (starting from the pos offset)
 *
 * The negative return value means an error (not enough space)
 * otherwise, the current buffer position is returned
 * \test tcab0009.c
 * @{
*/
///buffer descriptor
typedef struct sBuffer {
    byte *ptr;                  ///< buffer containing the data
    word dim;                   ///< number of bytes in the buffer
    word pos;                   ///< current position
} tBuffer;

void bufInit(tBuffer * buf, byte * ptr, word dim);  ///<Initialize a buffer
word bufDim(const tBuffer * buf);   ///<Get buffer dimension
const byte *bufPtr(const tBuffer * buf);    ///<Get buffer pointer
word bufLen(const tBuffer * buf);   ///<Get buffer length (position)
void bufReset(tBuffer * buf);   ///<Fill the buffer by zeroes and reset the current position
int bufSet(tBuffer * buf, byte val, word num);  ///<Set num bytes of buffer to the value val
int bufIns(tBuffer * buf, word ofs, const byte * dat, int len); ///<Insert data at a given offset
int bufOwr(tBuffer * buf, word ofs, const byte * dat, int len); ///<Overwrite data at a given offset
int bufCat(tBuffer * dst, tBuffer * src);   //Append the content of src buffer to dst buffer
int bufDel(tBuffer * buf, word ofs, int len);   ///<Delete data at a given offset
int bufApp(tBuffer * buf, const byte * dat, int len);   ///<Append data to the end of buffer
int bufCpy(tBuffer * buf, const byte * dat, int len);   ///<Reset buffer and copy new data into it
int bufGet(tBuffer * buf, byte * dat);

/** A shortcut for string appending:
 * the third argument is zero (calculated as strlen)
*/
#define bufAppStr(BUF,STR) bufApp(BUF,(byte *)STR,0) /** Shortcut to append a zero-ended string */
/** @} */

/** \weakgroup SYSQUE Queue structure
 * This structure provides a container of type queue.
 * It is widely used to interface with EMV Kernel when transmitting the parameter sets to the EMV Kernel function, creating data object lists etc.
 * The structure members should be never accessed directly.
 * A set of functions are provided to perform the operations on a queue.
 *
 * The negative return values mean error
 * If operation is performed OK, the length of the element is returned
 * The len==0 means that len is to be calculated as strlen(dat)
 * \test tcab0010.c
 * @{
*/
///queue descriptor
typedef struct sQueue {
    tBuffer *buf;               ///< buffer containing the queue elements
    word cur;                   ///< current read position
    word put;                   ///< number of elements put (written)
    word get;                   ///< number of elements to get (written and not read yet)
} tQueue;

void queInit(tQueue * que, tBuffer * buf);  ///<Initialize a queue
int quePut(tQueue * que, const byte * dat, int len);    ///<Put a data element into a queue
int queGet(tQueue * que, byte * dat);   ///<Retrieve a data element from the queue
word queNxt(const tQueue * que);    ///<Get the size of the next data element in the queue
word queLen(const tQueue * que);    ///<Get queue length (number of elements not rtetrieved yet)
void queReset(tQueue * que);    ///<Empty the queue
void queRewind(tQueue * que);   ///<Put the current position to the beginning of the queue
int quePutTag(tQueue * que, card tag);  ///<Put a tag into queue
int queGetTag(tQueue * que, card * tag);    ///<Retrieve a tag from the queue
int quePutTlv(tQueue * que, card tag, word len, byte * val);    ///<Put (Tag-Length-Value) triple into the queue
int queGetTlv(tQueue * que, card * tag, word * len, byte * val);    ///<Retrieve (Tag-Length-Value) from the queue

/** @} */

/** \weakgroup SYSTRC Tracing macros and functions
 * The macros below are not empty in simulation mode only.
 * These functions are implemented in TBX or I32 component,
 * depending on whether it is compiled for Simelite or for real terminal.
 *
 * The tracing is performed into "DEBUG.TRC" file under simelite or into a com port in real terminal
 * @{
 */
byte trcMode(byte mod);         ///<Set trace mode (No,File,Com1,Com2)

#ifdef __TEST__
void _trcS(card opt, const char *str);  ///<Trace a string
void _trcFL(card opt, const char *file, int line);  ///<Trace source file name and line
void _trcFS(card opt, const char *fmt, const char *str);    ///<Trace formatted string
void _trcFN(card opt, const char *fmt, long num);   ///<Trace formatted number
void _trcBuf(card opt, const byte * buf, int len);  ///<Trace binary data of a given length as is
void _trcBN(card opt, const byte * buf, int len);   ///<Trace binary data in HEX of a given length
void _trcAN(card opt, const byte * buf, int len);   ///<Trace ASCII data of a given length
void _trcBAN(card opt, const byte * buf, int len);  ///<Trace data in HEX and ASCII format
void _trcQ(card opt, tQueue * que); ///<Trace a queue
void _trcErr(card opt, long err);   ///<Trace en error value
void _trcDT(card opt, const char *file, int line, const char *str); ///<Trace Date/time stamp and source file name and line

/** Trace date/time stamp, source file name and line into trace file labeled by str string.
 *
 * It is not defined in non-testing mode.
 * \param STR (I) String label to trace.
 * \return no
 * \header sys\\sys.h
 * \source sys\\tbx.c
*/
#define TRCDT(STR) _trcDT(trcOpt,__FILE__,__LINE__,STR) /** Trace Date/time stamp and source file name and line */

void _trcQueDol(card opt, tQueue * que);    ///<Trace a Data Object List (DOL) (used in E32)
void _trcQueTlv(card opt, tQueue * que);    ///<Trace a queue containing TLVs (used in E32)

#define trcS(s) _trcS(trcOpt,s)
#define trcFL(f,l) {} _trcFL(trcOpt,f,l)
#define trcFS(f,s) {} _trcFS(trcOpt,f,s)
#define trcFN(f,n) {} _trcFN(trcOpt,f,n)
#define trcBuf(b,n) {} _trcBuf(trcOpt,b,n)
#define trcBN(b,n) {} _trcBN(trcOpt,b,n)
#define trcAN(b,n) {} _trcAN(trcOpt,b,n)
#define trcBAN(b,n) {} _trcBAN(trcOpt,b,n)
#define trcQ(q) {} _trcQ(trcOpt,q)
#define trcErr(n) {} _trcErr(trcOpt,n)
#define trcQueDol(q) {} _trcQueDol(trcOpt,q)
#define trcQueTlv(q) {} _trcQueTlv(trcOpt,q)
#else
#define trcS(s) {}
#define trcFL(f,l) {}
#define trcFS(f,s) {}
#define trcFN(f,n) {}
#define trcBuf(b,n) {}
#define trcBN(b,n) {}
#define trcAN(b,n) {}
#define trcBAN(b,n) {}
#define trcQ(q) {}
#define trcErr(n) {}
#define TRCDT(s) {}
#define trcQueDol(q) {}
#define trcQueTlv(q) {}
#endif

#ifdef __PMD__
void pmdSet(byte sta);
void pmdLog(const char *str, const char *file, int line);

#define PMDSET(STA) pmdSet(STA)
#define PMDLOG(STR) pmdLog(STR,__FILE__,__LINE__)
#else
#define PMDSET(STA)
#define PMDLOG(STR)
#endif

/** @} */
/** @} */

/** \addtogroup i32group I32 Ingenico
 * I32 component implements the interface with terminal perypherals,
 * and wraps the original Ingedev library functions.
 * @{
*/

/** \weakgroup SYSFUN Various system functions
 * A set of various functions retrieving some system information.
 * @{
 */

typedef struct sFileDetails {
    char fileName[20 + 1];      //change to 20 since OBJECT_FILE_NAME_LEN in object_info_t is 20
    word fCrc;
} tFileDetails;

int getAppVer(char *fileName, const char *appName);
byte getPid(void);              ///< retrieve process identifier
int getAppName(char *name);     ///<retrieve application name
int getDateTime(char *YYMMDDhhmmss);    ///<retrieve system date and time in ASCII format
int setDateTime(const char *YYMMDDhhmmss);  ///<set system date and time to a given value
void dat2asc(char *YYMMDD, card date);  ///<convert internal Ingedev date into ASCII format YYMMDD
void tim2asc(char *hhmmss, card time);  ///<convert internal Ingedev time into ASCII format hhmmss
void asc2dat(card * date, const char *YYMMDD);  ///<convert ASCII date YYMMDD into internal Ingedev format
void asc2tim(card * time, const char *hhmmss);  ///<convert ASCII time hhmmss into internal Ingedev format
card sysRand(void);             ///<get random value
void Beep(void);                ///<make a beep
void Click(void);               ///<make a click
void reboot(void);              ///<reboot the terminal
int getCodeFileList(tFileDetails * fDetails, char *cFile);  ///<retrieves the application file names and its CRCs
int TStoDateTime(int timestamp, char unit, char *YYMMDDhhmmss);
int DateTimetoTS(char unit, char *YYMMDDhhmmss);

/** @} */

/** \weakgroup SYSTMR Timer processing
 * There can be up to tmrN timers
 *
 * The first parameter tmr of the functions below is the timer number (0..tmrN-1)
 *
 * The parameter dly (delay) is the number of centiseconds to wait
 * @{
 */
int tmrStart(byte tmr, int dly);    ///<start the timer tmr for dly centiseconds
int tmrGet(byte tmr);           ///<get the number of centiseconds waitng for timer tmr
void tmrStop(byte tmr);         ///<stop the timer tmr
int getTS(char unit);           ///<returns the number of time units since first call or since 01/01/1970
void tmrSleep(card dly);        ///<sleep for dly seconds

/** Make a pause of sec seconds. The timer 0 is used for it.
 * \param sec (I) Number of seconds to wait
 * \return no
 * \header sys\\sys.h
 * \source sys\\i32.c
 */
void tmrPause(byte sec);        ///<make a pause of sec seconds

/** @} */

/** \weakgroup SYSMAG Magnetic stripe processing
 * @{
 */
int magStart(void);             ///<start the magstripe perypheral
int magStop(void);              ///<stop the magstripe perypheral
int magGet(char *trk1, char *trk2, char *trk3); ///<get magstripe tracks
int getCard(byte * buf, const char *ctl);   ///<process event card input

/** @} */

/** \weakgroup SYSSMC Smart card processing
 * @{
 */
enum eIcc {                     ///<Smart card error processing codes
    iccBeg,                     ///< start sentinel
    iccCardRemoved,             ///< card removed
    iccCardMute,                ///< card mute
    iccCardPb,                  ///< other card problem
    iccDriverPb,                ///< driver problem
    iccReadFailure,             ///< Read error
    iccKO                       ///< unknown error
};

int iccStart(byte rdr);         ///<Open Integrated Circuit Card perypheral
int iccStop(byte rdr);          ///<Close Integrated Circuit Card perypheral
int iccDetect(byte rdr);        ///<Detect card
int iccCommand(byte rdr, const byte * cmd, const byte * dat, byte * rsp);   ///<Send a command and data to a smart card and retrieve the response

/** @} */
/** @} */

/** \addtogroup hmigroup HMI Human-Machine Interface
 * HMI component contains the systems wrappers around printer, dicplay and kayboard peripherals, including canvas
 *  processing.
 * @{
*/

/** \weakgroup SYSPRT Printer processing
 * The printer width for default font is set in the file def.h
 * @{
 */
#define prtW __PRTW__
int prtStart(void);             ///<Start printer perypheral
int prtStop(void);              ///<Stop printer perypheral
int prtCtrl(void);              ///<Control paper presence
int prtS(const char *str);      ///<print a string
int prtES(const char *esc, const char *str);    ///<print a string preceeded by an escape characters sequence
int prtImg(const byte * img, int hgt, int wdt, const char *ctl);    ///<Print an image

/** @} */

/** \weakgroup SYSDSP Display processing
 * The default display width is __DSPW__ and its height __DSPH__ is defined in def.h
 *
 * For I5100 the values are 16 and 4.
 *
 * They should be replaced in application context definidtion in the file def.h
 * if another terminal model is used.
 * @{
 */
#define dspW __DSPW__
#define dspH __DSPH__
#define fntH __FNTH__

byte dspWdt(byte wdt);
byte dspHgt(byte hgt);
int dspStart(void);             ///<Start display perypheral
int dspStop(void);              ///<Stop display perypheral
int dspClear(void);             ///<Clear display
int dspLight(byte val);         ///<Set backlight intensity
int dspLS(int loc, const char *str);    ///<Display the string str at the location loc

#define INV(L) (0x80|(L)) /**< display at line L in inverted mode */
#define BLD(L) (0x40|(L)) /**< display in bold font at line L */
#define RGT(L) (0x20|(L)) /**< display in right-justified string at line L */
#define BIG(L) (0x10|(L)) /**< display in big characters at line L */

int dspXYFS(word x, word y, byte font, const char *str);    ///< display a string at a given position in pixels using given font
int dspImg(word x, word y, word wdt, word hgt, const byte * img);   ///<display an image bitmap

/** @} */

/** \weakgroup SYSCNV Canvas processing
 * The default values for canvas:
 *  - canvas width is cnvW,
 *  - its height is cnvH,
 *  - the depth (the number of bits per pixel) is cnvD
 *  - the palette size (number of colors) is cnvP
 * @{
 */
#define cnvW __CNVW__
#define cnvH __CNVH__
#define cnvD __CNVD__
#define cnvP (1<<__CNVD__)

int cnvStart(void);             ///<Initialize canvas
int cnvStop(void);              ///<release canvas resource
int cnvDraw(void);              ///<draw canvas on the display
int cnvClear(void);             ///<clear canvas
int cnvPixel(word hor, word ver, word clr); ///<put pixel on the canvas
int cnvLine(word x1, word y1, word x2, word y2, word clr);  ///<draw line
int cnvBar(word x1, word y1, word x2, word y2, word clr);   ///<draw rectangle bar
int cnvText(word x, word y, const char *txt, byte fnt, card clr);   ///<draw text
card cnvTextSize(const char *txt, byte fnt);    ///<Compute surrounding rectangle width and height
int cnvTile(word x, word y, word wdt, word hgt, int clr, const byte * ptr); ///<draw a tile/bitmap

/** @} */

/** \weakgroup SYSCLR Color and palette definitions
 * @{
 */
enum ePal {                     ///<RGB palette registers
    //palBeg,
    palBlue,
    palGreen,
    palRed,
    palIntense,
    palEnd
};

#ifdef __UNICAPT__
enum eClr {                     ///<Colors
    clrBlack = 0,               //0000=0
    clrBlue = (1 << palBlue),   //0001=1
    clrGreen = (1 << palGreen), //0010=2
    clrRed = (1 << palRed),     //0100=4
    clrCyan = (clrBlue | clrGreen), //0011=3
    clrMagenta = (clrBlue | clrRed),    //0101=5
    clrYellow = (clrGreen | clrRed),    //0110=6
    clrGray = (clrBlue | clrGreen | clrRed),    //0111=7
    clrINTENSE = (1 << palIntense), //1000=8
    clrGRAY = (clrINTENSE | clrBlack),  //1000=8
    clrBLUE = (clrINTENSE | clrBlue),   //1001=9
    clrGREEN = (clrINTENSE | clrGreen), //1010=A
    clrRED = (clrINTENSE | clrRed), //1100=C
    clrCYAN = (clrINTENSE | clrCyan),   //1011=B
    clrMAGENTA = (clrINTENSE | clrMagenta), //1101=D
    clrYELLOW = (clrINTENSE | clrYellow),   //1110=E
    clrWHITE = (clrINTENSE | clrGray),  //1111=F
    clrEnd
};
#endif

#ifdef __TELIUM__
enum eClr {                     ///<Colors
    clrBlack = 0x0000,
    clrBlue = 0x0007,
    clrGreen = 0x0070,
    clrCyan = 0x0077,
    clrRed = 0x0700,
    clrMagenta = 0x0707,
    clrYellow = 0x0770,
    clrGray = 0x0555,
    clrGRAY = 0x0AAA,
    clrBLUE = 0x000F,
    clrGREEN = 0x00F0,
    clrCYAN = 0x00FF,
    clrRED = 0x0F00,
    clrMAGENTA = 0x0F0F,
    clrYELLOW = 0x0FF0,
    clrWHITE = 0x0FFF,
    clrEnd
};
#endif

///Foreground color
#define clrFgd __FGD__

///Background color
#define clrBgd __BGD__
/** @} */

/** \weakgroup SYSKBD Keyboard processing
 * @{
 */
enum eKbd {                     ///<The key codes
    kbdBeg,                     ///< start sentinel
#ifdef __UNICAPT__
    kbdVAL = 0x3A,              ///< Enter
    kbdANN = 0x3B,              ///< Esc
    kbd00 = 0x3C,               ///< "."
    kbdCOR = 0x3D,              ///< BkSp
    kbdINI = 0x3E,              ///< SYS F9
    kbdFWD = 0x3F,              ///< F6 Paper Feed
    kbdF1 = 0x40,               ///< F1
    kbdF2 = 0x41,               ///< F2
    kbdF3 = 0x42,               ///< F3
    kbdF4 = 0x43,               ///< F4
    kbdF5 = 0x44,               ///< F5
    kbdF6 = 0x45,               ///< F6
    kbdF7 = 0x46,               ///< F7
    kbdF8 = 0x47,               ///< F8
    kbdDN = kbdF1,              ///< Down arrow
    kbdUP = kbdF2,              ///< Up arrow
    kbdMNU = kbdF3,             ///< MENU button
    kbdR = 0x46,                ///< F7
    kbdJST = 'E',               ///< push joystick
    kbdJUP = 'C',               ///< joystick up
    kbdJDN = 'G',               ///< joystick down
    kbdJLF = 'D',               ///< joystick left
    kbdJRG = 'F',               ///< joystick right
#else
    kbdVAL = 0x16,              ///< Enter
    kbdANN = 0x17,              ///< Esc
    kbd00 = 0x3C,               ///< not available
    kbdCOR = 0x18,              ///< BkSp
    kbdINI = 0x28,              ///< F button
    kbdFWD = 0x07,              ///< Paper Feed, not available at application level
    kbdF1 = 0x19,               ///< F1
    kbdF2 = 0x20,               ///< F2
    kbdF3 = 0x21,               ///< F3
    kbdF4 = 0x22,               ///< F4
    kbdF5 = 0x44,               ///< not available
    kbdF6 = 0x45,               ///< not available
    kbdF7 = 0x46,               ///< not available
    kbdF8 = 0x47,               ///< not available
    kbdDN = 0x24,               ///< Down arrow
    kbdUP = 0x23,               ///< Up arrow
    kbdMNU = kbdINI,            ///< MENU button
    kbdR = '.',                 ///< T_POINT
    kbdJST = 0x25,              ///< NAVI_OK
    kbdJUP = kbdF4,             ///< joystick up
    kbdJDN = kbdF3,             ///< joystick down
    kbdJLF = 'D',               ///< not available
    kbdJRG = 'F',               ///< not available
#endif
    kbdEnd
};

int kbdStart(byte fls);         ///<start waiting for a key
int kbdStop(void);              ///<stop waiting for a key
char kbdKey(void);              ///<get a key pressed

/** @} */

/** \weakgroup SYSTFT Touch screen processing
 * @{
 */
int tftStart(void);             ///<start waiting for a touch
int tftStop(void);              ///<stop waiting for a touch
card tftGet(void);              ///<get touch coordinates
char tftState(void);            ///<get touch screen status: 'd'= down; 'u'= up; 0=unknown
card tftLocation(void);         ///<get last location touched detected

/** @} */
/** @} */

/** \addtogroup memgroup MEM memory processing
 * MEM component contains the functions operating with non-volatile (DFS) and internal secured (ISM) memory.
 * @{
*/

/** \weakgroup SYSNVM Non-volatile memory processing
 * The argument naming convention for nvm(non-volatile memory):
 *  - sec: section (file or safe data page number)
 *  - buf: buffer containing data to save or to load
 *  - ofs: offset within the section
 *  - len: number of bytes to save or to load
 *  - val: value to fille a block of memory
 * @{
 */
enum eNvm {                     ///<non-volatile memory error processing codes
    nvmBeg,                     ///< start sentinel
    nvmOpenErr,                 ///< error opening file
    nvmSeekErr,                 ///< offset not found
    nvmKO                       ///< unknown error (read/write)
};

void nvmFileName(char *buf, byte sec);  ///<build DFS file name containing section data
void nvmPathFileName(char *buf, byte sec);  ///<build full path file name containing section data
void nvmStart(void);            ///<Initialize internal non-volatile memory descriptors
int nvmHold(byte sec);          ///<Hold section sec opened until nvmRelease() called.
void nvmRelease(byte sec);      ///<Close section sec opened by preceding nvmHold() call.
int nvmRemove(byte sec);        ///<Remove DFS file containing the section sec
int nvmSize(byte sec);          ///<Return the size of a section
int nvmMove(byte srcSec, byte dstSec);  ///<Move a section to an another section (owerwrite)
int nvmSave(byte sec, const void *buf, card ofs, word len); ///<Save data to nvm section
int nvmSet(byte sec, byte val, card ofs, card len); ///<fill data in nvm section by a given value
int nvmLoad(byte sec, void *buf, card ofs, word len);   ///<get data from nvm section
card nvmSeqOpen(const char *file, char atr);    ///<open a file for sequential access (reading or writing)
void nvmSeqClose(card hdl);     ///<close sequential access file
int nvmSeqPos(card hdl, card pos);  ///<set file position
int nvmSeqGet(card hdl, void *buf, word len);   ///<sequential read
int nvmSeqPut(card hdl, const void *buf, word len); ///<sequential write

/** @} */

/** \weakgroup SYSISM Internal secured RAM memory (ISM) processing
 * The argument naming convention for ram(secured RAM):
 *  - buf: buffer containing data to save or to load
 *  - ofs: offset within the region
 *  - len: number of bytes to save or to load
 *  - val: value to fill a block of memory
 * @{
 */
int ismSave(const void *buf, card ofs, word len);   ///<save data into sram region
int ismSet(byte val, card ofs, card len);   ///<fill data in sram region by a given value
int ismLoad(void *buf, card ofs, word len); ///<retrieve data from sram region

/** @} */
/** @} */

/** \addtogroup extgroup EXT external device processing
 * EXT component contains the functions operating with external devices, of the non INGENICO.
 * @{
 */

/** \weakgroup SYSBAR Barcode reader processing
 * @{
 */

int bcrStart(void);             ///<start the bar code reader's com port
void bcrStop(void);             ///<stop the bar code reader's com port and stop the bar code reader
int bcrGet(char *barCode);      ///<get bar code
int bcrReq(void);               ///<start waiting bar code
int bcrCancel(void);            ///<stop the bar code checking
int bcrRead(char *ctl, byte dly, char *barCode);    ///<read particlar type of bar code

/** @} */

/** \weakgroup SYSCLS contactless reader processing
 * @{
 */
int clsStart(void);             ///<start the contactless reader
void clsStop(void);             ///<stop the contactless reader
int clsBip(byte dly);           ///<play bip function
int clsLED(byte ctl, byte color);   //<LED in VENDING PASS manipulation function
int clsImg(const unsigned char *img, word ImgLen, word Lsize);  ///<show image on mfc reader screen
int clsSound(const byte * sound, word SoundLen);    ///< play melody
int clsText(byte line, char *text); ///<show text
int clsClearDsp(void);          ///< clease Cless display
void clsBacklightLevel(byte level); ///< Backlihght of the contactless reader

typedef enum txnStep {
    IDLE,
    INITIALISED,
    INTERFACE_OPENED,
    TXN_READY,
    PRELIMINARY_TXN_COMPLETE,
    ONLINE_PROC,
    COMPLETE_TXN,
    WAIT_FOR_CARD_REMOVAL,
    CLSCARD_REMOVED,
    FINALISE_TXN,
    ERROR,
    FINISHED
} txnStep_t;

int clsMVTransaction(int state, byte * cmd, word cmdLen, tQueue * rsp); ///< CLESS MV

//int clsDetectStart(void);  ///< asyncronous function initiates waiting of the contactless card event
//void clsDetectStop(void); ///< asyncronous function terminates waiting of the contactless card event

int mfcStart(void);             ///<start the Mifare cards reader
void mfcStop(void);             ///<stop the Mifare cards reader
int mfcDetect(byte dly);        ///<Mifare card detect
int mfcAut(const byte * key, byte AutTyp);  ///<authentication
int mfcLoad(byte * data, byte loc, byte fmt);   ///<data load
int mfcSave(const byte * data, byte loc, byte fmt); ///<data save
int mfcDebit(const byte * data, byte loc);  ///<debit value
int mfcCredit(const byte * data, byte loc); ///<credit value

int bioStart(void);             ///<start the Biometric reader
void bioStop(void);             ///<stop the Biometric reader
int bioEnroll(int loc, int dly, char *uid, char *udt);  ///<save biometric data
char bioVerify(int dly);        ///<search single data for verification
char bioIdentify(int dly, card * uin, char *uid, char *udt);    ///<search single data for identification
int bioClearDB(int loc, int dly);   ///<clear biometrics database
int bioRemove(char *uid, int dly);  ///<remove a record in database
int bioCreateDB(word rec, byte fng, char *fld, int dly);    ///<create a user-defined database
int bioDestroyDB(int dly);      ///<destroy the current database

/** @} */
/** @} */

/** \addtogroup chngroup CHN communication processing
 * CHN component contains the functions operating various communication channels.
 * @{
*/

/** \weakgroup SYSCHN Communication processing
 * @{
 */
enum eChn {                     ///<UTA channels
    chnBeg,                     ///<start sentinel
    chnMdm,                     ///<internal modem
    chnCom1,                    ///<Com1
    chnCom2,                    ///<Com2
    chnCom3,                    ///<Com3
    chnTcp5100,                 ///<tcp connection I5100 with ethernet card
    chnTcp = chnTcp5100,        ///<tcp connection generic
    chnTcp7780,                 ///<tcp connection I7780 bluetooth
    chnPpp,                     ///<PPP
    chnGprs,                    ///<GPRS
    chnHdlc,                    ///<HDLC
    chnUSB,                     ///<USB Slave
    chnUSBMaster,               /// <USB Master
    chnSck,                     /// <Socket
    chnWifi,                    /// <Wifi
    chnSsl5100,                 ///<ssl connection I5100 with ethernet card    
    chnSsl = chnSsl5100,        ///<ssl connection generic
    chnSsl7780,                 ///<ssl connection I7780 bluetooth
    chnEnd                      ///<end sentinel
};

enum eChnRole {                 ///<channel roles
    chnRoleBeg,                 ///<start sentinel
    chnRoleSrv,                 //SERVER
    chnRoleClt,                 //CLIENT
    chnRoleEnd                  ///<end sentinel
};

enum eCom {                     ///<Communication errors
    comBeg,                     ///<start sentinel
    comTimeout,                 ///<timeout
    comAnn,                     ///<user cancel
    comNoCarrier,               ///<NO CARRIER
    comNoDialTone,              ///<NO DIAL TONE
    comBusy,                    ///<BUSY
    comKO                       ///<unclassified error
};

int comStart(byte chn);         ///<Open communication perypheral for a selected channel
int comSet(const char *init);   ///<set channel parameters
int comStop(void);              ///<close communication perypheral
int comDial(const char *srv);   ///<Dial (connect) a server
int comHangStart(void);         ///<Start waiting for modem hang up
int comHangWait(void);          ///<Wait until modem hangs up
int comSend(byte b);            ///<Send a character
int comRecv(byte * b, int dly); ///<Receive a character
int comSendBuf(const byte * msg, word len); ///<Send a string of characters
int comRecvBuf(tBuffer * msg, const byte * trm, byte dly);  ///<Receive a buffer until a terminator character
int comListen(const char *srv); ///<Start listening a port
int comAccept(const char *srv); ///<Accept incoming connection
int comGetIMSI(char *pImsi, const char *pPin);  ///<Get IMSI of GSM SIM
int comGetIMEI(char *pImei);    ///<Get IMEI of GSM modem
int comGetLastError(void);      ///<Gets the last error from the link layer
int comGetDialInfo(void);       ///<Gets the status indicating the state of the connection (disconnected, connecting, connected...)

/** @} */

/** \weakgroup SYSFTP FTP session processing
 * @{
 */
int ftpStart(void);             ///<Start an FTP session
int ftpStop(void);              ///<Close FTP session
int ftpDial(const char *srv, const char *usr, const char *pwd, char mod);   ///<Connect to a remote FTP server
int ftpCD(const char *dir);     ///<Change remote directory
int ftpGet(byte sec, const char *file); ///<Download a remote file as a nvm section
int ftpPut(byte sec, const char *file); ///<Upload a nvm section to a remote file

/** @} */

/** \weakgroup SYSTSK Task & sempaphore management functions
 *
 * There can be only one task and no more than 3 semaphores (indexed as 0,1,2)
 * @{
 */
int tskStart(void (*fun) (card), card arg); ///<Start a secondary task
void tskStop(void);             ///<Stop the secondary task
void tskSync(void);             ///<Synchronize tasks
int tskComCtl(byte ctl);        ///<Transfer control over the com handle among main and secondary tasks.
int semInc(byte idx);           ///<Semaphore acquire
int semDec(byte idx);           ///<Semaphore release

/** @} */
/** @} */

/** \addtogroup gclgroup GCL communication processing
 * GCL component contains the wrappers around the functions implemented in Generic Communication Library.
 * It is an alternative way to CHN module.
 * @{
*/
/** \weakgroup SYSGCL GCL (Generic Communication Library) wrappers.
 *
 * The GCL wrappers will replace the old UTA communication functions.
 *
 * It is implemented for 32 bit only
 * @{
 */
enum eGcl {                     ///<gcl communication modes
    gclBeg,                     ///<start sentinel
    gclHdlc,                    ///< HDLC modem connection
    gclAsyn,                    ///< Asynchronous modem connection
    gclISDN,                    ///< ISDN modem
    gclSer,                     ///< Serial communication
    gclWnb,                     ///< WNB serial connection
    gclPpp,                     ///< PPP over a modem connection
    gclPppCom,                  ///< PPP over a serial port
    gclEth,                     ///< Ethernet connection
    gclGsm,                     ///< GSM data call
    gclPppEth,                  ///< PPP over Ethernet connection
    gclGprs,                    ///< GPRS connection
    gclPppGsm,                  ///< PPP over a GSM connection
    gclSck,                     ///< Socket interface
    gclWifi,                    ///< WIFI interface
    gclEnd                      ///<end sentinel
};

enum eGclSta {                  ///<gcl states
    gclStaBeg,                  ///<start sentinel
    gclStaPreDial,              ///<predial
    gclStaDial,                 ///<dial
    gclStaConnect,              ///<connect
    gclStaLogin,                ///<login
    gclStaFinished,             ///<finished
    gclStaEnd                   ///<end sentinel
};

enum eGclErr {                  ///<gcl errors
    gclErrBeg,                  ///<start sentinel
    gclErrCancel,               ///<user cancel
    gclErrDial,                 ///<dial error
    gclErrBusy,                 ///<BUSY
    gclErrNoDialTone,           ///<NO DIAL TONE
    gclErrNoCarrier,            ///<NO CARRIER
    gclErrNoAnswer,             ///<NO ANSWER
    gclErrLogin,                ///<login error
    gclErrCableRemoved,         ////<Cable Removed Error
    gclErrEnd                   ///<end sentinel
};

int utaGclStart(byte chn, const char *init);    ///<Open a GCL communication channel
int utaGclStop(void);           ///<Close the perypheral
int utaGclDial(void);           ///<start dialing
int utaGclDialWait(void);       ///<wait while dial is done
int utaGclDialInfo(void);       ///<get Dial state, see eGclSta enum
int utaGclDialErr(void);        ///<get Dial error, see eGclErr enum
int utaGclSend(byte b);         ///<send a byte
int utaGclRecv(byte * b, int dly);  ///<receive a byte
int utaGclSendBuf(const byte * msg, word len);  ///<send a message
int utaGclRecvBuf(tBuffer * msg, const byte * trm, byte dly);   ///<receive a message until a terminator
void utaGclSetCommCfg(card timOut); ///<Set the gcl communication timeout

/** @} */
/** @} */

/** \addtogroup pgngroup PGN GMA plugin interface functions
 * PGN component contains the wrappers around the GMA plugin interface functions.
 * @{
*/
enum ePgnConn {
    pgnConnNull,
    pgnConnGsm,
    pgnConnGprs,
    pgnConnEnd
};
int pgnImeiImsi(char *imei, char *imsi);    ///<Get IMEI of GSM modem and/or IMSI of GSM SIM
int pgnSgnLvl(byte * rssi, byte * sta); ///<Get signal level and status
int pgnBatLvl(byte * docked);   ///<Get battery level
int pgnSmfStart(void);          ///<Start maintenance session

#ifdef __TEST__
int pgnConnInfo(byte * connected, unsigned long *localIpAddr, unsigned long *dns1, unsigned long *dns2);    ///<Get connection information
#endif

/** @} */

/** \addtogroup e32group E32 Emv wrappers
 *
 * The EMV functions return non-negative value if there is no error.
 *
 * Otherwise the return code is negative and its absolute value is within the enum eEmvErr.
 * @{
*/
enum eEmvErr {                  ///<EMV errors enum
    eEmvErrBeg,                 ///< start sentinel
    eEmvErrNOK,                 ///< unclassified error
    eEmvErrReselect,
    eEmvErrFallback,            ///< fall back condition encountered
    eEmvErrAppBlock,            ///< application is blocked
    eEmvErrCardBlock,           ///< card is blocked
    eEmvErrEnd                  //end sentinel
};

int cvtQV(tQueue * que, byte * dat);    ///<Convert a queue into BER-TLV format
int cvtVQ(tQueue * que, byte * dat);    ///<Convert BER-TLV data into a queue

/** \weakgroup SYSEMV EMV Kernel wrapper functions
 *
 *  The wrappers perform calls to EMV Kernel functions
 *
 * The functions below use the following queue parameters:
 *  - qVAK contains tag values to be transferred from appli to kernel
 *  - qTAK contains tags requested by appli from kernel for future use
 *  - qVKA contains tag values returned from kernel to appli
 *  - qTKA contains tags requested by kernel from appli for future use
 * @{
 */
int emvInit(tQueue * que);      ///<Wrapper around the function amgSelectionCriteriaAdd
int emvStart(void);             ///<Wrapper around the function amgInitPayment
void emvStop(void);             ///<Wrapper around the functions amgEndPayment and amgEmvCloseContext.
int emvSelect(tQueue * que);    ///<Wrapper around the function amgAskSelection
void emvSetFSbuf(byte * buf);   ///<Set Final Select buffer pointer
word emvSta(void);              ///<return SW1SW2 status bytes
int emvSetSelectResponse(byte * aid, void *buf);
int emvCheckFallback(void);
void emvSelectStatus(word * pbm);
int emvApplicationSelect(void);
int emvFinalSelect(const byte * aid);   ///<Wrapper around the function amgEmvFinalSelect
int emvGetLang(char *lan);      ///<Wrapper around the function amgEmvGetLanguagePreference
int emvContext(tQueue * qVAK, tQueue * qTKA);   ///<Wrapper around the function amgEmvInitContext
int emvPrepare(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA); ///<Wrapper around the function amgEmvTransactionPreparation
int emvAuthenticate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA);    ///<Wrapper around the function amgEmvDataAuthentication
int emvCVMstart(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA);   ///<CV_INITIAL wrapper around amgEmvCardHolderVerification
int emvCVMnext(int opt, tQueue * qTAK, tQueue * qVKA);  ///<generic wrapper around amgEmvCardHolderVerification
int emvCVMoff(tQueue * qTAK, tQueue * qVKA);    ///<CV_OFFLINE wrapper around amgEmvCardHolderVerification
int emvValidate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA);    ///<Wrapper around the function amgEmvTransactionValidation
int emvAnalyse(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA); ///<Wrapper around the function amgEmvActionAnalysis
int emvComplete(byte cry, tQueue * qVAK, tQueue * qTAK, tQueue * qVKA); ///<Wrapper around the function amgEmvTransactionCompletion
void emvSetFallBack(byte state);
byte emvGetFallBack(void);
int emvCheckCardBlocked(void) ;

/** @} */
/** @} */

/** \weakgroup SYSCLSMV MV Kernel wrapper functions
 *
 *  The wrappers perform calls to CLESS MV Kernel functions
 *
 * The functions below use the following queue parameters:
 * @{
 */
int clsMVInit(tQueue * que, int explicit);
int clsMVDetect(void);
int clsMVSelect(void);
int clsMVContext(tQueue * que);
int clsMVFinalSelect(tQueue * que);
int clsMVPayPassPerformTransaction(void);

/** @} */

/** \addtogroup c32group C32 Crypto functions
 *
 * C32 contains the functions working with cryptomodule (via SSA for 32 bit or Dallas for 16 bit)
 *
 * The crypto functions below use the following format to locate a key
 *
 * loc= array*16+slot where aarray is array number, slot is a slot in the array,
 * loc being the location function parameter
 * @{
*/
int cryStart(char mod);         ///<Open the associated channel
int cryStop(char mod);          ///<Close the associated channel
void crySetPpd(const char *cfg);    ///<set default pinpad configuration
int cryEmvPin(const char *msg); ///<Wrapper around the function ssaSecEmvl2PinOfflineReq
int cryLoadTestKey(byte loc);   ///<Inject the key 1111111111111111 into the location loc
int cryVerify(byte loc, byte * crt);    ///<Calculate key certificate
int cryDiag(int *nbOfAreaId, int *tabOfAreaId); ///<Diagnostic secret areas
int cryEraseKeysArray(byte array);  ///<Erase the array of keys
int cryLoadDKuDK(byte loc, byte trn, const byte * key, char prp);   ///<Load double key key of property prp into location loc using a double transport key located at the location trn
int cryDiversifyDKwDK(byte loc, byte div, const byte * key, char prp);  ///<Diversify double key with double key

int cryGetPin(const char *msg, const byte * acc, byte loc, byte * blk); ///<Ask user to enter online PIN
int cryMac(tBuffer * buf, byte loc, byte * mac);    ///<Calculate MAC (Message Authentication Code)
int cryDukptInit(byte loc, byte * SerialNumberKey); ///<Key management DUKPT Initialisation
int cryDukptGetPin(const char *msg1, const char *msg2, const byte * acc, word sec, byte * SerialNumberKey, byte * blk); ///<Pin entry DUKPT : pin process (ANSI X9.24-1997)
int cry3DESDukptInit(byte loc, byte * SerialNumberKey); ///<Key management 3DES DUKPT Initialisation
int cry3DESDukptGetPin(const char *msg1, const char *msg2, const byte * acc, word sec, byte * SerialNumberKey, byte * blk); ///<Pin entry 3DES DUKPT : pin process (ANSI X9.24-2002)
int cryOwfInt(const byte * datLeft, const byte * datRight, byte locSrcKey, byte locDstKey, char prp);   ///<One Way Function Internal
int cryOwfExt(const byte * datLeft, const byte * datRight, byte locSrcKey, byte * resLeft, byte * resRight);    ///<One Way Function External
int cryCalculateSha1(tBuffer * buf, int len, byte * hash);  ///<The SHA (Secure Hash Algorithm)
int crySha1ToPkcs1(byte * hash, word size, byte * pkcs);    ///<PKCS#1 Format
int cryRsaSignData(byte * p, byte * q, byte * d, byte * dataIn, byte * dataOut, word size); ///<RSA CRT Algorithm Encryption
int cry3DESDukptGetPinUpdateKey(const char *msg, const byte * acc,
                                word sec, byte * SerialNumberKey,
                                byte * blk, byte loc);
int cryLoadTK(byte loc, const byte * cardData);
word getPinLen(void);
int cryDukptGetPinTSA(const int SecretArea, const char AlgoType, const word Number, const unsigned int BankId, const char *msg1, const char *msg2, const byte * acc, byte * SerialNumberKey, byte * blk);   ///<KIA-TSA Pin entry DUKPT : pin process (ANSI x99.24-2004

/** \weakgroup SYSINF System information
 * The default display width is dspW and its height is 2
 * @{
 */
int getSapSer(char *sap, char *ser, char dvc);  ///<Get S.A.P. code and Serial number of a device

/** @} */

/** \weakgroup SYSPPD Pinpad processing
 * The default display width is dspW and its height is 2
 * @{
 */
#define ppdW 16
#define ppdH 2

int ppdClear(void);             ///<Clear pinpad screen
int ppdLS(byte loc, const char *str);   ///<Display string str at line loc of the pinpad
int ppdDspKbdKey(char *msg, char *lngIndicator);    ///<Used to display 2 line message into PinPad

/** @} */
/** @} */
/** @} */

#define __SYS_H
#endif
