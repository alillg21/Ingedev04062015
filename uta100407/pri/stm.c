/** \file
 * Stream processing
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/pri/stm.c $
 *
 * $Id: stm.c 2217 2009-03-20 14:52:31Z abarantsev $
 */

#include <stdio.h>
#include <string.h>
#include "pri.h"

#undef trcOpt
//static const card trcOpt = TRCOPT & BIT(trcSTM);
#define trcOpt (TRCOPT & BIT(trcSTM))

static int stmLoad(tStream * stm) {
    int ret;
    byte *ptr;

    VERIFY(stm->mod == 'r');
    if(!stm->hdl) {             //RAM stream
        ret = 0;
        goto lblEnd;
    }
    ptr = (byte *) bufPtr(stm->buf);
    ret = nvmSeqGet(stm->hdl, ptr, bufDim(stm->buf));
    CHECK(ret >= 0, lblKO);
    VERIFY(ret <= bufDim(stm->buf));
    stm->buf->pos = ret;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("stmLoad ret=%d\n", ret);
    return ret;

}

/** Stream constructor: 
 * \param stm (O) Pointer to the stream descriptor to be initialized
 * \param buf (I) stream buffer
 * \param file (I) I/O file name
 * \param mod (I) access mode: 'r'= read, 'w'= write
 * \pre 
 *  - stm!=0
 *  - buf!=0
 * \return
 *   - file handle for file stream
 *   - zero for RAM stream
 *   - negative in case of error
 *
 * Two types of stream are implemented:
 *  - file stream
 *  - RAM stream
 *
 * If argument 3 (file) is zero, it is RAM stream; otherwise it is RAM stream
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0143.c
 */
int stmInit(tStream * stm, tBuffer * buf, const char *file, char mod) {
    int ret;

    VERIFY(stm);
    VERIFY(buf);
    VERIFY(mod == 'r' || mod == 'w');
    trcFS("stmInit file=%s\n", file ? file : "RAM");

    memset(stm, 0, sizeof(*stm));
    stm->buf = buf;
    stm->mod = mod;
    if(file) {
        stm->hdl = nvmSeqOpen(file, mod);
        CHECK(stm->hdl, lblKO);
    } else
        stm->hdl = 0;

    if(mod == 'r') {
        //ret= stmLoad(stm);
        //CHECK(ret >= 0, lblKO);
    }
    ret = stm->hdl;

    goto lblEnd;

  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    return ret;
}

/** Flush the buffer if needed and close stream
 * \param stm (O) Pointer to the stream descriptor
 * \pre 
 *  - stm!=0
 * \return non-negative if OK; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0143.c
 */
int stmClose(tStream * stm) {
    int ret;

    VERIFY(stm);
    trcS("stmClose\n");
    ret = 0;
    if(stm->hdl) {              //file stream
        if(stm->mod == 'w') {
            if(bufLen(stm->buf)) {
                ret = nvmSeqPut(stm->hdl, bufPtr(stm->buf), bufLen(stm->buf));
                CHECK(ret >= 0, lblKO);
            }
        }
        bufReset(stm->buf);
        nvmSeqClose(stm->hdl);
    }
    memset(stm, 0, sizeof(*stm));
    return ret;
  lblKO:
    return -1;
}

/** Get data from input stream:
 * \param stm (M) pointer to the stream descriptor
 * \param ptr (O) pointer to the destination buffer 
 * \param len (I) length of data reserved in the destination buffer 
 * \param trm (I) terminator characters
 * \pre 
 *  - stm!=0
 *  - ptr!=0
 * 
 * \remark negative len value means that 0 is also considered as terminator character
 *
 * The bytes are retrieved from the stream to the destination buffer until one of terminator characters is encountered.
 * The number of bytes is limited to len.
 *
 * \return number of bytes retrieved; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0143.c
 */
int stmGet(tStream * stm, byte * ptr, int len, const byte * trm) {
    int ret;
    word num, idx, ofs, dim;
    byte z;

    VERIFY(stm);
    VERIFY(ptr);
    VERIFY(stm->mod == 'r');

    z = 0;
    if(len < 0) {
        z = 1;
        len = -len;
    }
    dim = 0;
    while(len) {
        if(bufLen(stm->buf) == 0) {
            ret = stmLoad(stm); //get next portion into stream buffer
            if(ret <= 0)
                break;
        }
        ofs = (word) (stm->pos % bufDim(stm->buf)); //offset within the buffer
        num = len;
        if(num > bufLen(stm->buf) - ofs)
            num = bufLen(stm->buf) - ofs;
        if(!num)                //all bytes retrieved
            break;
        if(!trm) {
            memcpy(ptr, bufPtr(stm->buf) + ofs, num);
        } else {
            for (idx = 0; idx < num; idx++) {
                const byte *tmp;

                tmp = (byte *) bufPtr(stm->buf);
                ptr[idx] = tmp[ofs + idx];

                for (tmp = trm; *tmp; tmp++) {
                    if(ptr[idx] == *tmp)
                        break;
                }
                if(*tmp) {
                    VERIFY(ptr[idx] == *tmp);
                    num = idx + 1;
                    len = num;
                    break;
                }
                if(!z)
                    continue;
                if(ptr[idx])
                    continue;
                num = idx + 1;
                len = num;
                break;
            }
        }
        VERIFY(num <= len);
        dim += num;
        len -= num;
        stm->pos += num;
        if(stm->pos % bufDim(stm->buf) == 0)
            bufReset(stm->buf);
        ptr += num;
        if(!len)
            break;
    }
    trcS("stmGet: ");
    trcBAN(ptr - dim, dim);
    trcS("\n");
    ret = dim;
    return ret;
}

/** Put data into output stream:
 * \param stm (M) pointer to the stream descriptor
 * \param ptr (I) pointer to the source buffer 
 * \param len (I) length of data in the source buffer 
 * \pre 
 *  - stm!=0
 *  - ptr!=0
 *
 * The bytes are put into the stream form the destination buffer.
 *
 * \return number of bytes retrieved; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0143.c
 */
int stmPut(tStream * stm, const byte * ptr, word len) {
    int ret;
    word dim, num;

    VERIFY(stm);
    VERIFY(ptr);

    if(!len)
        len = strlen((char *) ptr);

    trcS("stmPut: ");
    trcBAN(ptr, len);
    trcS("\n");

    dim = 0;
    while(len) {
        num = len;
        VERIFY(bufDim(stm->buf) >= bufLen(stm->buf));
        if(bufDim(stm->buf) <= bufLen(stm->buf)) {  //buffer overflow
            CHECK(stm->hdl, lblKO);
            ret = nvmSeqPut(stm->hdl, bufPtr(stm->buf), bufDim(stm->buf));
            CHECK(ret == bufDim(stm->buf), lblKO);
            bufReset(stm->buf);
        }
        if(num > bufDim(stm->buf) - bufLen(stm->buf)) {
            num = bufDim(stm->buf) - bufLen(stm->buf);
        }
        ret = bufApp(stm->buf, ptr, num);
        VERIFY(num <= len);
        len -= num;
        ptr += num;
        dim += num;
    }
    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    return ret;
}

#ifndef __LZW_BITS__
#define __LZW_BITS__ 12
#endif

//The string table size needs to be a prime number that is somewhat larger than 2**__LZW_BITS__.
#if __LZW_BITS__ == 14
#define LZW_TABLE_SIZE 18041
#endif
#if __LZW_BITS__ == 13
#define LZW_TABLE_SIZE 9029
#endif
#if __LZW_BITS__ <= 12
#define LZW_TABLE_SIZE 5021
#endif
#define LZW_HASHING_SHIFT (__LZW_BITS__-8)
#define LZW_MAX_VALUE ((1 << __LZW_BITS__) - 1)
#define LZW_MAX_CODE LZW_MAX_VALUE - 1

static short int *lzwVal;       //code value
static word *lzwPrf;            //prefix codes
static byte *lzwApp;            //appended chars
static byte *lzwStk;            //decoded string
static int lzwBitNum;
static card lzwBitBuf;

static int lzwPutCode(tStream * dst, word code) {
    int ret;
    byte b;
    word dim;

    lzwBitBuf |= (card) code << (32 - __LZW_BITS__ - lzwBitNum);
    lzwBitNum += __LZW_BITS__;
    dim = 0;
    while(lzwBitNum >= 8) {
        b = (byte) (lzwBitBuf >> 24);
        ret = stmPut(dst, &b, 1);
        CHECK(ret == 1, lblKO);
        lzwBitBuf <<= 8;
        lzwBitNum -= 8;
        dim++;
    }
    return dim;
  lblKO:
    return -1;
}

static int lzwFind(int hshPrf, word hshChr) {
    int idx;
    int ofs;

    idx = (hshChr << LZW_HASHING_SHIFT) ^ hshPrf;
    ofs = 1;
    VERIFY(idx <= LZW_TABLE_SIZE);
    if(idx)
        ofs = LZW_TABLE_SIZE - idx;
    while(1) {
        VERIFY(idx <= LZW_TABLE_SIZE);
        if(lzwVal[idx] == -1)
            break;
        if(lzwPrf[idx] == hshPrf && lzwApp[idx] == hshChr)
            break;
        idx -= ofs;
        if(idx < 0)
            idx += LZW_TABLE_SIZE;
    }
    return idx;
}

/** Compress src stream into dst stream using LZW algorythm
 * \param src (M) pointer to the source stream descriptor
 * \param dst (M) pointer to the destination stream descriptor
 * \pre 
 *  - src!=0
 *  - dst!=0
 *
 * The LZW algorythm is used as described by Mark NELSON:
 * http://marknelson.us/attachments/lzw-data-compression/lzw.c
 *
 * \return number of bytes written into dst; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0189.c
 */
int lzwCompress(tStream * src, tStream * dst) {
    int ret;
    byte chr;
    word nxt, cod, idx;
    card dim = 0;
    short int _val[LZW_TABLE_SIZE];
    word _prf[LZW_TABLE_SIZE];
    byte _app[LZW_TABLE_SIZE];

    VERIFY(src);
    VERIFY(dst);

    memset(_val, 0, sizeof(_val));
    memset(_prf, 0, sizeof(_prf));
    memset(_app, 0, sizeof(_app));
    //memset(_stk,0,sizeof(_stk));
    lzwVal = _val;
    lzwPrf = _prf;
    lzwApp = _app;
    lzwBitNum = 0;
    lzwBitBuf = 0;

    nxt = 256;                  //Next code is the next available string code
    for (idx = 0; idx < LZW_TABLE_SIZE; idx++)
        lzwVal[idx] = -1;       //Clear out the string table before starting
    ret = stmGet(src, &chr, 1, 0);
    CHECK(ret == 1, lblKO);
    cod = chr;
    dim = 0;
    while(1) {                  //run until all of the input has been exhausted
        ret = stmGet(src, &chr, 1, 0);
        if(ret != 1)
            break;

        idx = lzwFind(cod, chr);    //See if the string is in the table
        if(lzwVal[idx] != -1) {
            cod = lzwVal[idx];  //if yes, get the code value
            continue;
        }
        //otherwise, try to add it
        if(nxt <= LZW_MAX_CODE) {   //stop adding codes to the table after all of the possible codes have been defined
            lzwVal[idx] = nxt++;
            lzwPrf[idx] = cod;
            lzwApp[idx] = chr;
        }
        ret = lzwPutCode(dst, cod); //When a string is found that is not in the table output the last string after adding the new one
        CHECK(ret > 0, lblKO);
        dim += ret;
        cod = chr;
    }

    ret = lzwPutCode(dst, cod); // Output the last code
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = lzwPutCode(dst, LZW_MAX_VALUE);   //Output the end of buffer code
    CHECK(ret > 0, lblKO);
    dim += ret;

    ret = lzwPutCode(dst, 0);   //This code flushes the output buffer
    CHECK(ret > 0, lblKO);
    dim += ret;
    ret = dim;
    goto lblEnd;

  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("lzwCompress ret=%d\n", dim);
    return dim;
}

static int lzwGetCode(tStream * src) {
    card ret;
    byte b;

    VERIFY(src);

    while(lzwBitNum <= 24) {
        ret = stmGet(src, &b, 1, 0);
        CHECK(ret == 1, lblKO);
        lzwBitBuf |= (card) b << (24 - lzwBitNum);
        lzwBitNum += 8;
    }
    ret = lzwBitBuf >> (32 - __LZW_BITS__);
    lzwBitBuf <<= __LZW_BITS__;
    lzwBitNum -= __LZW_BITS__;
    return ret;
  lblKO:
    return -1;
}

static byte *lzwGetStr(byte * buf, word cod) {
    int idx = 0;

    VERIFY(buf);
    while(cod > 255) {
        *buf++ = lzwApp[cod];
        cod = lzwPrf[cod];
        CHECK(idx++ < LZW_MAX_CODE, lblKO);
    }
    *buf = (byte) cod;
    return buf;
  lblKO:
    return 0;
}

/** Decompress src stream into dst stream using LZW algorythm
 * \param src (M) pointer to the source stream descriptor
 * \param dst (M) pointer to the destination stream descriptor
 * \pre 
 *  - src!=0
 *  - dst!=0
 *
 * The LZW algorythm is used as described by Mark NELSON:
 * http://marknelson.us/attachments/lzw-data-compression/lzw.c
 *
 * \return number of bytes written into dst; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0189.c
 */
int lzwExpand(tStream * src, tStream * dst) {
    int ret;
    word nxt, cod, old;
    byte chr;
    byte *str;

    //short int _val[LZW_TABLE_SIZE];
    word _prf[LZW_TABLE_SIZE];
    byte _app[LZW_TABLE_SIZE];
    byte _stk[4000];
    card dim = 0;

    VERIFY(src);
    VERIFY(dst);

    //memset(_val,0,sizeof(_val));
    memset(_prf, 0, sizeof(_prf));
    memset(_app, 0, sizeof(_app));
    memset(_stk, 0, sizeof(_stk));
    //lzwVal= _val;
    lzwPrf = _prf;
    lzwApp = _app;
    lzwStk = _stk;
    lzwBitNum = 0;
    lzwBitBuf = 0;

    nxt = 256;                  //This is the next available code to define
    ret = lzwGetCode(src);      //Read in the first code
    CHECK(ret >= 0, lblKO);
    VERIFY(ret < 256);
    chr = ret;
    old = chr;                  //initialize the first character
    ret = stmPut(dst, &chr, 1); //send the first character to output
    CHECK(ret == 1, lblKO);

    dim = 1;
    //Read in characters from the LZW file until the special code used to inidicate the end of the data
    while(1) {
        ret = lzwGetCode(src);
        if(ret < 0)
            break;
        cod = ret;
        if(cod == LZW_MAX_VALUE)
            break;

        //Check for the special STRING+CHARACTER+STRING+CHARACTER+STRING
        //case which generates an undefined code.
        //Process it by decoding the last code, and adding a single character 
        //to the end of the decode string.
        if(cod >= nxt) {
            *lzwStk = chr;
            str = lzwGetStr(lzwStk + 1, old);
        } else {                // Otherwise we do a straight decode of the new code.
            str = lzwGetStr(lzwStk, cod);
        }
        CHECK(str, lblKO);

        chr = *str;
        while(str >= lzwStk) {  //output the decoded string in reverse order
            ret = stmPut(dst, str--, 1);
            CHECK(ret == 1, lblKO);
            dim++;
        }
        if(nxt <= LZW_MAX_CODE) {   //if possible, add a new code to the string table
            lzwPrf[nxt] = old;
            lzwApp[nxt] = chr;
            nxt++;
        }
        old = cod;
    }
    ret = dim;
    goto lblEnd;

  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    trcFN("lzwExpand ret=%d\n", dim);
    return dim;
}

/** Start building a binary XML stream
 * \param stm (M) Pointer to the output stream descriptor
 * \pre 
 *  - stm!=0
 * \return number of bytes appended; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0209.c
 */
int bmlPutHdr(tStream * stm) {
    int ret;

    VERIFY(stm);
    trcS("bmlPutHdr\n");
    ret = stmPut(stm, (byte *) "\x03\x01\x03\x00", 4);
    CHECK(ret == 4, lblKO);
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Append the input argument tag to the output stream
 * \param stm (M) Pointer to the output stream descriptor
 * \param tag (I) tag to add (optionally ORed with bmlBitA and/or with bmlBitC)
 * \pre 
 *  - stm!=0
 * \return number of bytes appended; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0209.c
 */
int bmlPutTagBeg(tStream * stm, byte tag) {
    int ret;

    VERIFY(stm);
    trcFN("bmlPutTagBeg: tag=%02X\n", tag);
    ret = stmPut(stm, &tag, 1);
    CHECK(ret == 1, lblKO);
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Append to the output stream:
 *  - atr code
 *  - 03 (STR_I) global token
 *  - val string
 *  - 00 string terminator
 * \param stm (M) Pointer to the output stream descriptor
 * \param atr (I) attribute code
 * \param val (I) attribute value
 * \pre 
 *  - stm!=0
 * \return number of bytes appended; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0209.c
 */
int bmlPutAtr(tStream * stm, byte atr, const char *val) {
    int ret;
    word dim;

    VERIFY(stm);
    trcFN("bmlPutAtr: atr=%02X ", atr);
    trcFS("val=[%s]\n", val);
    dim = 0;

    ret = stmPut(stm, &atr, 1);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0x03), 1);   //global token STR_I
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, (byte *) val, strlen(val) + 1);   //ending zero is appended also here
    CHECK(ret == strlen(val) + 1, lblKO);
    dim += ret;

    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** The behaviour of the function depends on the values of bmlBitA and bmlBitC bits:
 *  - if bmlBitC is ON (content is present) the global token 01 (END) is appended to the output stream
 *  - otherwise the function does nothing
 * \param stm (M) Pointer to the output stream descriptor
 * \param tag (I) tag code
 * \pre 
 *  - stm!=0
 * \return number of bytes appended; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0209.c
 */
int bmlPutTagEnd(tStream * stm, byte tag) {
    int ret;

    VERIFY(stm);
    trcFN("bmlPutTagEnd: tag=%02X\n", tag);
    ret = 0;
    if(tag & bmlBitC) {
        ret = stmPut(stm, bPtr(1), 1);  //global token END
        CHECK(ret == 1, lblKO);
    }
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** The behaviour of the function depends on the values of bmlBitA and bmlBitC bits:
 *  - if bmlBitA is ON (attribute list is present) the global token 01 (END) is appended to the output stream
 *  - otherwise the function does nothing
 * \param stm (M) Pointer to the output stream descriptor
 * \param tag (I) englobing tag code
 * \pre 
 *  - stm!=0
 * \return number of bytes appended; negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\stm.c
 * \test tcab0209.c
 */
int bmlPutAtrEnd(tStream * stm, byte tag) {
    int ret;

    VERIFY(stm);
    trcFN("bmlPutAtrEnd: atr=%02X\n", tag);
    ret = 0;
    if(tag & bmlBitA) {
        ret = stmPut(stm, bPtr(1), 1);  //global token END
        CHECK(ret == 1, lblKO);
    }
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int bmlGetAtr(tStream * stm, tVmtBml * vmt) {
    int ret;
    word dim;
    byte tok, atr;
    enum { BUFLEN = 256 };
    char buf[BUFLEN + 1];

    VERIFY(stm);
    VERIFY(vmt);

    trcS("bmlGetAtr Beg\n");
    dim = 0;
    while(1) {
        ret = stmGet(stm, &atr, 1, 0);
        CHECK(ret == 1, lblKO);
        dim += ret;

        if(atr == 0x01)         //global END token
            break;

        ret = stmGet(stm, &tok, 1, 0);
        CHECK(ret == 1, lblKO);
        CHECK(tok == 0x03, lblKO);  //STR_I global token
        dim += ret;

        ret = stmGet(stm, (byte *) buf, -BUFLEN, (byte *) "");
        CHECK(ret > 0, lblKO);
        CHECK(ret <= BUFLEN, lblKO);
        dim += ret;

        trcFN("  atr=%02X ", atr);
        trcFS("val=[%s]\n", buf);
        if(vmt->bmlHdlAtr) {
            ret = vmt->bmlHdlAtr(atr, buf);
            CHECK(ret >= 0, lblKO);
        }
    }

    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("bmlGetAtr ret=%d\n", ret);
    return ret;
}

static int bmlGetTag(tStream * stm, tVmtBml * vmt) {
    int ret;
    word dim;
    byte tag;
    static byte lvl = 0;

    VERIFY(stm);
    VERIFY(vmt);

    dim = 0;
    trcS("bmlGetTag Beg\n");
    while(1) {
        ret = stmGet(stm, &tag, 1, 0);
        CHECK(ret == 1, lblKO);
        dim += ret;

        trcFN("  tag=%02X\n", tag);

        if(tag == 0x01)         //global END token
            break;

        if(vmt->bmlHdlTag) {
            ret = vmt->bmlHdlTag(tag & 0x3F, lvl);
            CHECK(ret >= 0, lblKO);
        }

        if(tag & bmlBitA) {     //attributes are present
            ret = bmlGetAtr(stm, vmt);  //process attributes
            CHECK(ret >= 0, lblKO);
            dim += ret;
        }

        if(tag & bmlBitC) {     //content is not empty
            lvl++;
            ret = bmlGetTag(stm, vmt);  //process tags
            lvl--;
            CHECK(ret >= 0, lblKO);
            dim += ret;
        }
        if(vmt->bmlHdlTag) {
            ret = vmt->bmlHdlTag(-(tag & 0x3F), lvl);
            CHECK(ret >= 0, lblKO);
        }
        if(!lvl)
            break;
    }

    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("bmlGetTag ret=%d\n", ret);
    return ret;
}

int bmlParse(tStream * stm, tVmtBml * vmt) {
    int ret;
    word dim = 0;
    byte buf[4];

    VERIFY(stm);
    VERIFY(vmt);

    trcS("bmlParse Beg\n");

    ret = stmGet(stm, buf, 4, 0);
    CHECK(ret == 4, lblKO);
    dim += ret;

    if(vmt->bmlHdlHdr) {
        ret = vmt->bmlHdlHdr(buf);
        CHECK(ret >= 0, lblKO);
    }

    ret = bmlGetTag(stm, vmt);
    CHECK(ret >= 0, lblKO);
    dim += ret;

    ret = dim;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    trcFN("bmlParse ret=%d\n", ret);
    return ret;
}
