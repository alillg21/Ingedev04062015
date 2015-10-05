/** \file
 * TELIUM non-volatile memory processing (FFMS)
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlmmem.c $
 *
 * $Id: tlmmem.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include <sdk30.h>
#include "sys.h"

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcMEM))

static const char *root(void) {
    static char utaNNNN[1 + 3 + 4 + 1] = "";    // '/' + "UTA" +"NNNN" + Z
    static word appID;

#ifdef __APPID__
    appID = __APPID__;
#else
    appID = getAppType();
#endif

    trcFN("appID:%d\n", appID);
    if(*utaNNNN == 0)
        sprintf(utaNNNN, "/UTA%04X", appID);
    return utaNNNN;

}

/** Get FFMS file name for a given section.
 * 
 * The FFMS file name is built as "page00NN.nvm" where NN is the section number in hex.
 *
 * Root volume directory precedes the file name
 * \param buf (O) the FFMS file name
 * \param sec (I) nvm section
 * \return no
 * \source sys\\tlmmem.c
 */
void nvmFileName(char *buf, byte sec) {
    VERIFY(buf);
    //sprintf(buf, "%s/page%04x.nvm", root(), sec);
    sprintf(buf, "page%04x.nvm", sec);
}

void nvmPathFileName(char *buf, byte sec) {
    VERIFY(buf);
    sprintf(buf, "%s/page%04x.nvm", root(), sec);
}

static S_FS_FILE *fNvm;         //FFMS file holded
static byte sNvm;               //0xFF if not hold; otherwise section number holded

/** Initialize non-volatile memory internal variables.
 * Should be called before calling any non-volatile memory treatment, normally during POWERON event.
 * \return no
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0005.c
 */
void nvmStart(void) {
    int ret;
    S_FS_PARAM_CREATE cfg;
    unsigned long size;

    trcFS("%s\n", "nvmStart");

    fNvm = 0;
    sNvm = 0xFF;

    cfg.Mode = FS_WRITEONCE;
    ret = FS_mount((char *) root(), &cfg.Mode);
    if(ret != FS_OK) {
        trcS("Create UTA disk\n");
        strcpy(cfg.Label, root() + 1);  //without leading slash
        cfg.AccessMode = FS_WRTMOD;
        cfg.NbFichierMax = 16;
        cfg.IdentZone = FS_WO_ZONE_DATA;
        size = cfg.NbFichierMax * 32768;    //any value can be used
        ret = FS_dskcreate(&cfg, &size);
    }
    CHECK(ret == FS_OK, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    FS_unmount((char *) root());
    trcErr(ret);
    ret = -1;
    goto lblEnd;
  lblEnd:
    return;
}

static S_FS_FILE *nvmOpen(byte sec, const char *atr) {
    S_FS_FILE *fid;             //FFMS file
    char tmp[255];              //to store file name

    if(sNvm == sec)
        return fNvm;
    nvmPathFileName(tmp, sec);  //generate file name
    trcFS("nvmOpen %s ", tmp);
    trcFS("atr= %s\n", atr);
    fid = FS_open(tmp, (char *) atr);
    if(!fid) {
        if(strcmp(atr, "r+") == 0) {
            trcS("Try a instead of r+\n");
            fid = FS_open(tmp, "a");
        }
    }
    CHECK(fid, lblKO);
    return fid;
  lblKO:return 0;
}

static void nvmClose(byte sec, S_FS_FILE * fid) {
    VERIFY(fid);
    if(sNvm == sec)             //it is holded
        return;
    FS_close(fid);
}

/** Remove the FFMS file containing the section sec.
 * \param sec (I) Section number
 * \return 1 if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 */
int nvmRemove(byte sec) {
    int ret;
    char tmp[255];              //to store file name

    trcFN("nvmRemove sec=%d\n", (word) sec);
    nvmPathFileName(tmp, sec);  //generate file name
    ret = FS_unlink(tmp);
    CHECK(ret == 0, lblKO);
    return 1;
  lblKO:return 0;
}

/** Hold section sec opened until nvmRelease() called.
 *
 * The couple of function nvmHold()/nvmRelease() is used to avoid file opening and closing
 * at each data access operation.
 * If section sec is accessed several times sequentially the performance can be improved
 * by calling nvmHold at the beginning and nvmRelease at the end. Other sections should not be accessed in hold state.
 * Opens the related FFMS file without closing it
 * \param sec (I) Section number
 * \return no
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 */
int nvmHold(byte sec) {
    S_FS_FILE *fid;

    trcFN("nvmHold sec=%d\n", (word) sec);
    VERIFY(sNvm == 0xFF);
    fid = nvmOpen(sec, "r+");
    CHECK(fid, lblKO);
    sNvm = sec;
    fNvm = fid;
    return 1;
  lblKO:return 0;
}

/** Close section sec opened by preceding nvmHold() call.
 *
 * The couple of function nvmHold()/nvmRelease() is used to avoid file opening and closing
 * at each data access operation.
 * If section sec is accessed several times sequentially the performance can be improved
 * by calling nvmHold at the beginning and nvmRelease at the end.
 * Other sections should not be accessed in hold state.
 * Closes the related FFMS file
 * \param sec (I) Section number
 * \return no
 * \header sys\\sys.h
*/
void nvmRelease(byte sec) {
    trcFN("nvmRelease sec=%d\n", (word) sec);
    if(sNvm != 0xFF) {
        VERIFY(sec == sNvm);
        FS_close(fNvm);
        sNvm = 0xFF;
        fNvm = 0;
    }
}

/** Save len bytes from buffer buf into section sec starting from the position ofs.
 *  The information is saved into pageNNNN.nvm file
 * \param sec (I) Section number
 * \param buf (I) Pointer to a buffer to be saved
 * \param ofs (I) Starting offset within the section; 0XFFFFFFFFL meaning append to the end of file
 * \param len (I) Number of bytes to be saved
 * \return
 *    - (-eNvm code) if failure.
 *    - len if not.
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0005.c
 */
int nvmSave(byte sec, const void *buf, card ofs, word len) {
    int ret = 0;                //to be returned: number of bytes saved
    S_FS_FILE *fid;             //FFMS file

    VERIFY(buf);

    trcFS("%s: ", "nvmSave");
    trcFN("sec=%d ", sec);
    trcFN("len=%d ", len);
    trcFN("ofs=%d buf=[", ofs);
    trcBuf((byte *) buf, (word) len);
    trcS("]\n");

    fid = 0;
    if(len) {
        fid = nvmOpen(sec, "r+");   //update (r/w) mode 
        CHECK(fid, lblOpen);

        if(ofs == 0xFFFFFFFFL)
            ret = FS_seek(fid, 0, FS_SEEKEND);  //move the end of file
        else
            ret = FS_seek(fid, ofs, FS_SEEKSET);    //seek given offset
        if(ret != FS_OK) {
            CHECK(FS_length(fid) == 0, lblSeek);
            ret = FS_OK;
        }
        VERIFY(ret == FS_OK);

        ret = FS_write((void *) buf, len, 1, fid);  //and save it
        CHECK(ret == 1, lblKO);

        ret = FS_flush(fid);
        trcFN("FS_flush ret=%d\n", ret);
        CHECK(ret >= 0, lblFlush);
    }

    ret = len;
    goto lblEnd;

  lblOpen:
    trcErr(ret);
    return -nvmOpenErr;
  lblSeek:
    trcErr(ret);
    ret = -nvmSeekErr;
    goto lblEnd;
  lblFlush:
  lblKO:
    trcErr(ret);
    ret = -nvmKO;
    goto lblEnd;
  lblEnd:
    if(len)
        nvmClose(sec, fid);
    return ret;                 //close it anyway
}

#ifdef __BLOCKSIZE__
#define BLK __BLOCKSIZE__
#else
#define BLK 512
#endif
/** Fill len bytes by value val into section sec starting from the position ofs.
 *  The information is written into pageNNNN.nvm file
 * \param sec (I) Section number
 * \param val (I) Value to fill
 * \param ofs (I) Starting offset within the section; 0XFFFFFFFFL meaning append to the end of file
 * \param len (I) Number of bytes to be filled
 * \return  
 *    - (-eNvm code) if failure.
 *    - len if not.
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0005.c
 */
int nvmSet(byte sec, unsigned char val, card ofs, card len) {
    int ret = 0;                //to be returned: number of bytes set
    S_FS_FILE *fid;             //FFMS file
    byte tmp[BLK];              //to blocks saved
    card blk;                   //number of bytes in the current block
    card idx;                   //number of bytes rest to save

    trcFS("%s: ", "nvmSet");
    trcFN("sec=%d ", sec);
    trcFN("ofs=%d ", ofs);
    trcFS("val=", "");
    trcBN(&val, 1);
    trcFN("len=%d\n", len);

    fid = nvmOpen(sec, "r+");
    CHECK(fid, lblOpen);

    if(ofs == 0xFFFFFFFFL)
        ret = FS_seek(fid, 0, FS_SEEKEND);  //move the end of file
    else
        ret = FS_seek(fid, ofs, FS_SEEKSET);    //seek given offset
    trcFN("FS_seek ret=%d\n", ret);
    if(ret != FS_OK) {
        CHECK(FS_length(fid) == 0, lblSeek);
        ret = FS_OK;
    }
    VERIFY(ret == FS_OK);

    //ret = FS_write(&val, 1, len, fid); //does not do the job (?!)
    //CHECK(ret==len, lblSeek);

    memset(tmp, val, BLK);      //fill buffer by val
    idx = len;
    while(idx) {                //fill by portions of BLK bytes or less
        blk = BLK;              //the size of portion to save
        if(idx < blk)           //is there enough bytes?
            blk = idx;
        ret = FS_write(tmp, blk, 1, fid);
        CHECK(ret == 1, lblKO);
        VERIFY(idx >= blk);
        idx -= blk;             //calculate the number of bytes rest to fill
    }

    ret = FS_flush(fid);
    trcFN("FS_flush ret=%d\n", ret);
    //CHECK(ret==len%512, lblFlush);
    CHECK(ret >= 0, lblFlush);

    ret = len;
    goto lblEnd;
  lblOpen:
    trcErr(ret);
    return -nvmOpenErr;
  lblSeek:
    trcErr(ret);
    ret = -nvmSeekErr;
    goto lblEnd;
  lblFlush:
  lblKO:
    trcErr(ret);
    ret = -nvmKO;
    goto lblEnd;
  lblEnd:nvmClose(sec, fid);
    return ret;
}

#undef BLK

/** Load len bytes into buffer buf from section sec starting from the position ofs.
 * The information is loaded from pageNNNN.nvm file
 * \param sec (I) Section number
 * \param buf (O) Pointer to a buffer to be loaded
 * \param ofs (I) Starting offset within the section
 * \param len (I) Number of bytes to be saved
 * \return
 *    - (-eNvm code) if failure.
 *    - len if not.
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0005.c
 */
int nvmLoad(byte sec, void *buf, card ofs, word len) {
    int ret = 0;                //to be returned: number of bytes loaded
    S_FS_FILE *fid;             //FFMS file

    VERIFY(buf);

    fid = nvmOpen(sec, "r");
    CHECK(fid, lblOpen);

    ret = FS_seek(fid, ofs, FS_SEEKSET);    //seek given offset
    CHECK(ret == FS_OK, lblSeek);
    ret = FS_read(buf, len, 1, fid);    //load from file
    CHECK(ret == 1, lblKO);

    trcFS("%s: ", "nvmLoad");
    trcFN("sec=%d ", sec);
    trcFN("ofs=%d buf=[", ofs);
    trcBuf((byte *) buf, len);
    trcS("]\n");

    ret = len;
    goto lblEnd;

  lblOpen:
    trcErr(ret);
    return -nvmOpenErr;
  lblSeek:
    trcErr(ret);
    ret = -nvmSeekErr;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -nvmKO;
    goto lblEnd;
  lblEnd:nvmClose(sec, fid);
    return ret;
}

/** Return the size of a section
 * \param sec (I) section number
 * \return size of the section if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcgs0030.c
 */
int nvmSize(byte sec) {
    int ret;
    S_FS_FILE *fid;             //FFMS file

    VERIFY(sNvm == 0xFF);
    ret = 0;
    fid = nvmOpen(sec, "r");
    if(!fid)
        return 0;
    ret = FS_length(fid);
    nvmClose(sec, fid);
    return ret;
}

/** Move a section to an another place (remove it from original place, and overwrite the destination place).
 * \param srcSec (I) Section number of the source
 * \param dstSec (I) Section number of the destination
 * \return non-negative if success
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcgs0030.c
 */
int nvmMove(byte srcSec, byte dstSec) {
    int ret = -1;
    char tmp1[255];             //to store file name
    char tmp2[255];             //to store file name

    VERIFY(srcSec < 0xFE);
    VERIFY(dstSec < 0xFE);

    CHECK(nvmSize(srcSec), lblKO);  //Does it exist?

    nvmPathFileName(tmp1, srcSec);
    nvmPathFileName(tmp2, dstSec);
    nvmRemove(dstSec);
    ret = FS_rename(tmp1, tmp2);
    CHECK(ret == FS_OK, lblKO);
    nvmRemove(srcSec);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
    goto lblEnd;
  lblEnd:
    return ret;
}

#define SEQDIM 16
static S_FS_FILE *seq[SEQDIM];
static byte seqIdx = 0;

/** Open a FFMS file for sequential access (reading or writing)
 * \param file (I) file name
 * \param atr (I) attribute: 'r'= reading; 'w': writing
 * \return file handle that should be used in nvmSeqClose,nvmSeqGet,nvmSeqPut
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0186.c
 */
card nvmSeqOpen(const char *file, char atr) {
    byte idx;
    S_FS_FILE *fid;             //FFMS file
    char buf[256];

    VERIFY(file);
    VERIFY(atr == 'r' || atr == 'w' || atr == 'a');
    VERIFY(seq[seqIdx] == 0);
    sprintf(buf, "%s/%s", root(), file);
    trcFS("nvmSeqOpen %s\n", buf);
    fid = FS_open((char *) buf, (atr == 'r') ? "r" : "r+");
    if(!fid) {
        if(atr != 'r') {
            trcS("Try a instead of r+\n");
            fid = FS_open((char *) buf, "a");
            trcFS("if(!fid) %s\n", buf);
        }
    }
    CHECK(fid, lblKO);
    idx = seqIdx + 1;
    seq[seqIdx++] = fid;
    seqIdx %= SEQDIM;
    return seqIdx;
  lblKO:return 0;

}

/** Close a FFMS file opened by nvmSeqOpen() for sequential access (reading or writing)
 * \param hdl (I) FFMS file handle returned by nvmSeqOpen()
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0186.c
 */
void nvmSeqClose(card hdl) {
    VERIFY(hdl);
    hdl--;
    VERIFY(hdl < SEQDIM);
    VERIFY(seq[hdl]);
    FS_close(seq[hdl]);
    seq[seqIdx] = 0;
}

/** Erase a file.
 * \param file (I) file name
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test 
 */
int nvmSeqRemove(const char *file) {
    char buf[256];
    int ret;

    trcS("nvmSeqRemove Beg");

    sprintf(buf, "%s/%s", root(), file);
    trcFS("nvmSeqRemove %s\n", buf);
    ret = FS_unlink(buf);
    CHECK(ret == 0, lblKO);

    return 1;
  lblKO:
    return 0;
}

/** Set file position
 * \param hdl (I) FFMS file handle returned by nvmSeqOpen()
 * \param pos (I) file position to set
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 */
int nvmSeqPos(card hdl, card pos) {
    int ret;

    VERIFY(hdl);
    hdl--;
    VERIFY(hdl < SEQDIM);
    VERIFY(seq[hdl]);

    ret = FS_seek(seq[hdl], pos, FS_SEEKSET);
    CHECK(ret == FS_OK, lblKO);
    return pos;
  lblKO:
    return -1;
}

/** Read up to len bytes into the buffer buf from the file handle hdl
 * \param hdl (I) FFMS file handle returned by nvmSeqOpen()
 * \param buf (O) destination buffer pointer
 * \param len (I) maximal number of bytes to read
 * \return number of bytes actually read
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0186.c
 */
int nvmSeqGet(card hdl, void *buf, word len) {
    int ret;
    word num;                   //number of bytes retrieved

    VERIFY(buf);
    VERIFY(hdl);
    hdl--;
    VERIFY(hdl < SEQDIM);
    VERIFY(seq[hdl]);

    memset(buf, 0, len);
    num = len;
    ret = FS_read(buf, 1, len, seq[hdl]);
    //CHECK(ret == len, lblKO);     //bytes actually read not necessarily the same as the maximum bytes to read
    VERIFY(num <= len);
    ret = num;

    trcS("nvmSeqGet: ");
    trcBuf((byte *) buf, ret);
    trcS("\n");
    goto lblEnd;
  //lblKO:
    //trcErr(ret);
    //ret = -1;
    //goto lblEnd;
  lblEnd:
    return ret;
}

/** Write len bytes into the file handle hdl from the buffer buf 
 * \param hdl (I) FFMS file handle returned by nvmSeqOpen()
 * \param buf (I) source buffer pointer
 * \param len (I) maximal number of bytes to write
 * \header sys\\sys.h
 * \source sys\\tlmmem.c
 * \test tcab0186.c
 */
int nvmSeqPut(card hdl, const void *buf, word len) {
    int ret;

    VERIFY(buf);
    VERIFY(hdl);
    hdl--;
    VERIFY(hdl < SEQDIM);
    VERIFY(seq[hdl]);

    ret = FS_write((void *) buf, 1, len, seq[hdl]);
    CHECK(ret == len, lblKO);

    trcS("nvmSeqPut: ");
    trcBAN((byte *) buf, len);
    trcS("\n");

    ret = len;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
    goto lblEnd;
  lblEnd:
    return ret;
}
