/** \file tcab0188.c
 * Unitary test case for LZW compression
 * http://marknelson.us/attachments/lzw-data-compression/lzw.c
 * It is the next step after tcab0187.
 * The unicapt32 calls are replaced by UTA sys functions.
 * malloc is removed
 * Other modifications are done to simplify the code and be closer to UTA style.
 * To run this test case a file tcab0188.src should be in DFS of the application
 * It will be compressed into tcab0188.lzw
 * Then it will be decompressed to tcab0188.dst
 * If the file tcab0188.src is the same as tcab0187.src
 * then the file tcab0188.lzw should be the same as tcab0187.lzw
 * and tcab0188.dst should be the same as tcab0187.dst
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0188.c $
 *
 * $Id: tcab0188.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include <sys.h>

/** The macro __LZW_BITS__ affects several constants used in LZW algorythm.
 * It can take valuse 12 or 13 or 14.
 */
#ifdef WIN32
#define __LZW_BITS__ 12
#else
#define __LZW_BITS__ 10
#endif
#define LZW_HASHING_SHIFT (__LZW_BITS__-8)
#define LZW_MAX_VALUE ((1 << __LZW_BITS__) - 1)
#define LZW_MAX_CODE LZW_MAX_VALUE - 1

//The string table size needs to be a prime number that is somewhat larger than 2**__LZW_BITS__.
#if __LZW_BITS__ == 14
#define LZW_TABLE_SIZE 18041
#endif
#if __LZW_BITS__ == 13
#define LZW_TABLE_SIZE 9029
#endif
#if __LZW_BITS__ == 12
#define LZW_TABLE_SIZE 5021
//Below are for U32. This must be lower because the memory available in the terminal during Real Mode is small ( 4MB / 8MB )
#endif
#if __LZW_BITS__ == 11
#define LZW_TABLE_SIZE 2297
#endif
#if __LZW_BITS__ <= 10
#define LZW_TABLE_SIZE 1523
#endif

//__LZW_BITS__ Benchmark for a file of size 588434 bytes the compressed file has the size:
// - 12: 222553
// - 13: 218374
// - 14: 213099
//Thus, the compression rate gain is very small relative to memory resources required.

static short int *val;          //code value
static word *prf;               //prefix codes
static byte *app;               //appended chars
static byte *stk;               //decoded string

static card getCode(card src) {
    card ret;
    static int bitNum = 0;
    static card bitBuf = 0L;
    byte b;

    while(bitNum <= 24) {
        nvmSeqGet(src, &b, 1);
        bitBuf |= (card) b << (24 - bitNum);
        bitNum += 8;
    }
    ret = bitBuf >> (32 - __LZW_BITS__);
    bitBuf <<= __LZW_BITS__;
    bitNum -= __LZW_BITS__;
    return ret;
}

static void putCode(card dst, word code) {
    static int bitNum = 0;
    static card bitBuf = 0L;
    byte b;

    bitBuf |= (card) code << (32 - __LZW_BITS__ - bitNum);
    bitNum += __LZW_BITS__;
    while(bitNum >= 8) {
        b = bitBuf >> 24;
        nvmSeqPut(dst, &b, 1);
        bitBuf <<= 8;
        bitNum -= 8;
    }
}

static byte *getStr(byte * buf, word cod) {
    int idx = 0;

    while(cod > 255) {
        *buf++ = app[cod];
        cod = prf[cod];
        CHECK(idx++ < LZW_MAX_CODE, lblKO);
    }
    *buf = cod;
    return buf;
  lblKO:
    return 0;
}

static int find(int hshPrf, word hshChr) {
    int idx;
    int ofs;

    idx = (hshChr << LZW_HASHING_SHIFT) ^ hshPrf;

    ofs = 1;
    if(idx)
        ofs = LZW_TABLE_SIZE - idx;
    while(1) {
        if(val[idx] == -1)
            break;
        if(prf[idx] == hshPrf && app[idx] == hshChr)
            break;
        idx -= ofs;
        if(idx < 0)
            idx += LZW_TABLE_SIZE;
    }

    return idx;
}

static void compress(card src, card dst) {
    byte chr;
    word nxt, cod, idx;

    nxt = 256;                  //Next code is the next available string code
    for (idx = 0; idx < LZW_TABLE_SIZE; idx++)
        val[idx] = (-1);        //Clear out the string table before starting
    nvmSeqGet(src, &chr, 1);
    cod = chr;

    while(nvmSeqGet(src, &chr, 1) >= 0) {   //run until all of the input has been exhausted

        idx = find(cod, chr);   //See if the string is in the table

        if(val[idx] != -1) {
            cod = val[idx];     //if yes, get the code value
            continue;
        }
        //otherwise, try to add it
        if(nxt <= LZW_MAX_CODE) {   //stop adding codes to the table after all of the possible codes have been defined
            val[idx] = nxt++;
            prf[idx] = cod;
            app[idx] = chr;
        }

        putCode(dst, cod);      //When a string is found that is not in the table output the last string after adding the new one
        cod = chr;

    }

    putCode(dst, cod);          // Output the last code
    putCode(dst, LZW_MAX_VALUE);    //Output the end of buffer code
    putCode(dst, 0);            //This code flushes the output buffer
}

static void expand(card src, card dst) {
    word nxt, cod, old;
    byte chr;
    byte *str;

    nxt = 256;                  //This is the next available code to define
    chr = getCode(src);         //Read in the first code
    old = chr;                  //initialize the first character
    nvmSeqPut(dst, &chr, 1);    //send the first character to output

    //Read in characters from the LZW file until the special code used to inidicate the end of the data
    while((cod = getCode(src)) != (LZW_MAX_VALUE)) {
        //Check for the special STRING+CHARACTER+STRING+CHARACTER+STRING
        //case which generates an undefined code.
        //Process it by decoding the last code, and adding a single character 
        //to the end of the decode string.
        if(cod >= nxt) {
            *stk = chr;
            str = getStr(stk + 1, old);
        } else {                // Otherwise we do a straight decode of the new code.
            str = getStr(stk, cod);
        }
        CHECK(str, lblKO);

        chr = *str;
        while(str >= stk)       //output the decoded string in reverse order
            nvmSeqPut(dst, str--, 1);
        if(nxt <= LZW_MAX_CODE) {   //if possible, add a new code to the string table
            prf[nxt] = old;
            app[nxt] = chr;
            nxt++;
        }
        old = cod;
    }
    return;
  lblKO:
    trcS(" *** expand error");
}

static card prtTS(const char *msg, card beg) {
    char tmp[prtW + 1];
    char buf[prtW + 1];
    card ts;

    strcpy(buf, msg);
    ts = getTS(1);
    ts -= beg;
    num2dec(tmp, ts, 0);
    strcat(buf, tmp);
    prtS(buf);
    return ts + beg;
}

void tcab0188(void) {
    int ret;
    short int _val[LZW_TABLE_SIZE];
    word _prf[LZW_TABLE_SIZE];
    byte _app[LZW_TABLE_SIZE];
    byte _stk[4000];
    card src = 0;
    card dst = 0;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    val = _val;
    prf = _prf;
    app = (byte *) _app;
    stk = (byte *) _stk;

    prtTS("compress beg ", 0);

    src = nvmSeqOpen("tcab0188.src", 'r');
    CHECK(src, lblKO);
    dst = nvmSeqOpen("tcab0188.lzw", 'w');
    CHECK(dst, lblKO);
    compress(src, dst);
    nvmSeqClose(dst);
    nvmSeqClose(src);
    prtTS("compress end ", 0);
    prtTS("expand beg ", 0);
    src = nvmSeqOpen("tcab0188.lzw", 'r');
    dst = nvmSeqOpen("tcab0188.dst", 'w');
    expand(src, dst);
    prtTS("expand end ", 0);

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    if(src)
        nvmSeqClose(src);
    if(dst)
        nvmSeqClose(dst);
    dspStop();
    prtStop();
}
