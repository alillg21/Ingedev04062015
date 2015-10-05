/** \file
 * Trace functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/trc.c $
 *
 * $Id: trc.c 2217 2009-03-20 14:52:31Z abarantsev $
 */

#include "ctx.h"

#ifdef __TELIUM__
#define TRCID (trace_sap_id_t)0xF000
#include <sdk30.h>
#ifndef __DEBUG__
#define __DEBUG__
#endif
#endif

#include "sys.h"

#ifdef __UNICAPT__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unicapt.h>
#endif

#ifdef __TEST__
#ifdef __DEBUG__
static byte trcMod = 0xFF;

/** Select tracing mode.
 * \param  mod (I) Tracing mode
 *         - 0   : no tracing.
 *         - 0xFF: trace into DEBUG.TRC file, can be activated only under simelite.
 *         - 1,2 : trace into COM port COM1 or COM2, can be activated only in real terminal.
 * \return original tracing mode
 * \test tcab0072.c
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
byte trcMode(byte mod) {
    byte sav = trcMod;

    if(mod)
        VERIFY(mod == 0xFF);
    trcMod = mod;
    return sav;
}

/** Put string str into the trace file.
 *
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param str (I) String to trace
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcS(card opt, const char *str) {
    static byte i = 0;          //indicates whether this function is called for the first time

#ifdef __TELIUM__
#ifdef WIN32
    FILE *f;
    static const char *dbg = __DEBUG_TRC_PATH__;
#endif
#endif

    if(!opt)
        return;
    if(!trcMod)
        return;

#ifdef _ING_SIMULATION
#ifdef __UNICAPT__
    printf(str);
    fflush(0);
#endif
#endif

#ifdef __TELIUM__
#ifdef WIN32
#undef	fopen
#undef	fprintf
#undef	fclose
    if(dbg) {
        f = fopen(dbg, i ? "at" : "wt");    //rewrite if called for the first time
        if(f) {
            fprintf(f, str);
            fclose(f);
        }
    }
#define	fopen	eft_fopen
#define fprintf  eft_fprintf
#define	fclose	eft_fclose
#else
    trace(TRCID, strlen(str), str);
#endif

#endif

    i = 1;                      //next time it will not be zero
}

void errVrf(const char *cond, const char *file, word line) {
#ifdef __PMD__
    pmdLog(cond, file, line);
#endif
    _trcFS(0xFFFFFFFFL, "Fatal error, COND: %s, ", cond);
    _trcFL(0xFFFFFFFFL, file, line);
#ifdef WIN32
    exit(0);
#endif
}
#else
static byte trcMod = 0;
static uint32 hTrc = 0;
byte trcMode(byte mod) {
    comUartInit_t uIni;
    comInit_t cIni;
    byte sav = trcMod;

    switch (mod) {              //no error checking. It is tracing!
      case 1:
          comOpen("COM1", &hTrc);
          break;
      case 2:
          comOpen("COM2", &hTrc);
          break;
      default:
          VERIFY(!mod);
          comClose(hTrc);
          hTrc = 0;
          break;
    }
    trcMod = mod;
    if(!mod)
        return sav;

    memset(&uIni, 0, sizeof(uIni));
    memset(&cIni, 0, sizeof(cIni));

    uIni.speed = COM_BAUD_9600;
    uIni.parity = COM_PARITY_NONE;
    uIni.dataSize = COM_DATASIZE_8;
    uIni.stopBits = COM_STOP_1;
    uIni.rBufferSize = 0;
    uIni.sBufferSize = 0;

    cIni.type = COM_INIT_TYPE_UART;
    cIni.level = COM_INIT_LEVEL_1;
    cIni.init.uartInit = &uIni;
    comSetPortParameters(hTrc, COM_MODIF_PARAM_TEMPORARY, &cIni);
    return sav;
}

void _trcS(card opt, const char *str) {
    int idx;

    if(!trcMod)
        return;

    idx = strlen(str);
    comSendMsgWait(hTrc, idx, (byte *) str, 60 * 100);
    if(str[idx - 1] != '\n')
        return;
    comSendMsgWait(hTrc, 1, (byte *) "\r", 60 * 100);
}

void errVrf(const char *cond, const char *file, word line) {
    _trcFS(0xFFFFFFFFL, "Fatal error, COND: %s, ", cond);
    _trcFL(0xFFFFFFFFL, file, line);
}
#endif

/** Put string str formatted by sprintf function using fmt string into the trace file.
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param  fmt (I) Format string for sprintf function.
 * \param  str (I) String to trace.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcFS(card opt, const char *fmt, const char *str) {
    char buf[2048];

    sprintf(buf, fmt, str);
    _trcS(opt, buf);
}

/** Put a numeric value num formatted by sprintf function using fmt string into the trace file.
 *
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param fmt (I) Format string for sprintf function.
 * \param num (I) Number to trace.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcFN(card opt, const char *fmt, long num) {
    char buf[1024];

    sprintf(buf, fmt, num);
    _trcS(opt, buf);
}

/** Trace a binary buffer of length len. Each byte is traced as is.
 *
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param buf (I) Pointer to the buffer to trace.
 * \param len (I) Number of bytes to be traced.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 * \remark End of line is not traced. It should be added separately if needed.
 */
void _trcBuf(card opt, const byte * buf, int len) {
#ifdef __TELIUM__
    char tmp[2048];
    int idx;

    if(len > 512)
        len = 512;
    *tmp = 0;
    for (idx = 0; idx < len; idx++) {
        sprintf(tmp + strlen(tmp), "%02X ", (word) buf[idx]);
        //_trcS(opt, tmp);
    }
    //trace(TRCID, len, tmp);
    _trcS(opt, tmp);
#else
    _trcBN(opt, buf, len);
#endif
}

/** Trace a binary buffer of length len. Each byte is traced as a hex value.
 * The bytes are separated by spaces.
 *
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param buf (I) Pointer to the buffer to trace.
 * \param len (I) Number of bytes to be traced.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 * \remark End of line is not traced. It should be added separately if needed.
 */
void _trcBN(card opt, const byte * buf, int len) {
    int idx;
    char tmp[4];

    for (idx = 0; idx < len; idx++) {
        sprintf(tmp, "%02X ", (word) buf[idx]);
        _trcS(opt, tmp);
    }
}

/** Trace a binary buffer of length len. Each byte is traced as an ascii value.
 * The values less than 0x20 and greater than 0x7F are replaced by question marks ‘?’.
 *
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param buf (I) Pointer to the buffer to trace.
 * \param len (I) Number of bytes to be traced.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcAN(card opt, const byte * buf, int len) {
    int idx;
    char val[3];
    char tmp[8192];

    *tmp = 0;
    memset(tmp, 0, sizeof tmp);
    for (idx = 0; idx < len; idx++) {
        memset(val, 0, sizeof val);
        if(buf[idx] == '%')
            val[1] = '%';
        if(buf[idx] < 0x20)
            val[0] = '?';
        else if(buf[idx] > 0x7F)
            val[0] = '?';
        else
            val[0] = buf[idx];
        strcat(tmp, val);
    }
    _trcS(opt, tmp);
}

/** Trace a binary buffer of length len in binary and ascii format.
 * It is a shortcut to call trcBN  and trcAN.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param buf (I) Pointer to the buffer to trace.
 * \param len (I) Number of bytes to be traced.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcBAN(card opt, const byte * buf, int len) {
    _trcBN(opt, (byte *) buf, len);
    _trcS(opt, " [");
    _trcAN(opt, buf, len);
    _trcS(opt, "]");
}

/** Trace a queue. Each queue element is traced on a separate line.
 * Each byte is traced as a hex value. The bytes are separated by spaces.
 * At the end of line ASCII value is traced in brackets.
 * Non-ASCII characters are replaced by question marks '?'.
 *
 * It is defined as an empty macro in non- testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param que (I) Pointer to queue to trace
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcQ(card opt, tQueue * que) {
#define L_BUF 1024
    int ret;
    word dim;
    word len;
    byte val[L_BUF];

    queRewind(que);
    dim = queLen(que);
    while(dim--) {
        VERIFY(queNxt(que) <= L_BUF);
        ret = queGet(que, val);
        VERIFY(ret >= 0);
        len = (word) ret;

        _trcS(opt, "\t");
        _trcBuf(opt, val, len);
        _trcS(opt, "[");
        _trcAN(opt, val, len);
        _trcS(opt, "]\n");
    }
    queRewind(que);
#undef L_BUF
}

/** Trace an error code.
 * It is defined as an empty macro in non- testing mode.
 * A simple call of trcFN() is performed here.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param err (I) Number to trace.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcErr(card opt, long err) {
    _trcFN(opt, "!error code: %ld\n", err);
}

/** Trace date/time stamp into trace file labeled by str string.
 * The source file name and source line are traced also.
 * Usually it is called by TRCDT macro.
 *
 * It is not defined in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param file (I) Source file name.
 * \param line (I) Source line.
 * \param str (I) String label to trace.
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
*/
void _trcDT(card opt, const char *file, int line, const char *str) {
    char dt[12 + 1];

    getDateTime(dt);
    _trcFS(opt, "DTS[%s]:\t", str);
    _trcFS(opt, "{%s}", dt);
    _trcFN(opt, "\t[%d]", line);
    _trcFS(opt, "%s\n", file);
}

/** Put string source file name and line into the trace file.
 *
 * It is defined as an empty macro in non-testing mode.
 * \param opt (I) Trace option; if zero tracing will not be done
 * \param file (I) Source file name, usually srcFile pointing to __FILE__ macro
 * \param line (I) Source line, usually __LINE__ macro
 * \return no
 * \header sys\\sys.h
 * \source sys\\trc.c
 */
void _trcFL(card opt, const char *file, int line) { //trace source file and line
    _trcFS(opt, "FILE: %s ", file);
    _trcFN(opt, "LINE: %u\n", line);
}

/** Trace a Data Object List (DOL)
* \param opt (I) Trace option; if zero tracing will not be done
* \param que (I) A queue containing a list of tags.
*   Each tag is a card number.
* \return no
* \header sys\\sys.h
* \source sys\\trc.c
* \remark It is not empty only if tracing is enabled
*/
void _trcQueDol(card opt, tQueue * que) {
    int ret;
    word dim;
    card tag;
    char tmp[64];

    queRewind(que);
    dim = queLen(que);
    while(dim--) {
        VERIFY(queNxt(que) == sizeof(card));
        ret = queGet(que, (byte *) & tag);
        VERIFY(ret == sizeof(card));

        sprintf(tmp, "%X ", (int) tag);
        _trcS(opt, tmp);
    }
    _trcS(opt, "\n");
    queRewind(que);
}

/** Trace a queue containing tags and their values.
* \param opt (I) Trace option; if zero tracing will not be done
* \param que (I) Queue containing intermittent T,V,T,V,... pairs
*   Each tag is a card number.
*   The values are arbitrary binary data, the first byte being the length of the value.
* \return no
* \header sys\\sys.h
* \source sys\\trc.c
* \remark It is not empty only if tracing is enabled
*/
void _trcQueTlv(card opt, tQueue * que) {
    int ret;
    word dim;
    card tag;
    char tmp[64];
    byte val[1024];

    queRewind(que);
    dim = queLen(que);
    VERIFY(dim % 2 == 0);
    dim /= 2;
    while(dim--) {
        VERIFY(queNxt(que) == sizeof(card));
        ret = queGet(que, (byte *) & tag);
        VERIFY(ret == sizeof(card));

        sprintf(tmp, "\t%X: ", (int) tag);
        _trcS(opt, tmp);

        VERIFY(queNxt(que) <= 1024);
        ret = queGet(que, val);
        VERIFY(ret >= 0);

        _trcBuf(opt, val, ret);
        _trcS(opt, "[");
        _trcAN(opt, val, ret);
        _trcS(opt, "]\n");
    }
    queRewind(que);
}

#endif

void errChk(const char *cond, const char *file, word line) {
    trcFS("errChk: %s, ", cond);
    trcFL(file, line);
#ifdef __PMD__
    pmdLog(cond, file, line);
#endif
}

#ifdef __PMD__
//Post Mortem Dump processing
static byte pmd = 0;
void pmdSet(byte sta) {
    pmd = sta;
}
void pmdLog(const char *str, const char *file, int line) {
    int ret;
    char buf[256 + 1];
    char tmp[12 + 1];
    byte idx;
    uint32 hdl;
    uint16 len;

    idx = strlen(file);
    while(idx--) {
        if(file[idx] == '/' || file[idx] == '\\') {
            idx++;
            break;
        }
    }

    if(!pmd)
        return;
    memset(buf, 0, sizeof(buf));
    getDateTime(tmp);
    ret = getTS(0);
    sprintf(buf, "\n(%s %d)%s[%d]: %s", tmp, ret, file + idx, line, str);

    hdl = psyFileOpen("pmd", "a+");
    ret = psyFileSeek(hdl, 0, PSY_FILE_SEEK_SET);
    len = 1;
    ret = psyFileRead(hdl, &idx, &len);
    if(ret != RET_OK || len != 1) {
        idx = 0;
        psyFileWrite(hdl, &idx, 1);
    }

    psyFileSeek(hdl, 1 + idx * 256, PSY_FILE_SEEK_SET);
    psyFileWrite(hdl, buf, 256);

    idx++;
    psyFileSeek(hdl, 0, PSY_FILE_SEEK_SET);
    psyFileWrite(hdl, &idx, 1);
    psyFileClose(hdl);
}
#endif
