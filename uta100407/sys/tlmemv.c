/** \file
 * TELIUM EMV DC module wrappers
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/tlmemv.c $
 *
 * $Id: tlmemv.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include "ctx.h"

#ifdef __EMV__
#ifndef _SECDEL_
#define _SECDEL_
#endif
#include <sdk30.h>
#include <etat.h>
#include <Appel.h>
#include "_emvdctag_.h"
//#include "db_util.h"
#include "def_tag.h"
#include "_emvdc_.h"
#include "del.h"
#include "oem.h"
#include "log.h"
#define EMV_PROC_ERROR  11
#include "sys.h"
#include "emv.h"

#include "key.h"

#undef trcOpt
static const card trcOpt = TRCOPT & BIT(trcE32);

static DEL locDEL;
static DEL inpDEL;
static DEL *outDEL;
static StructProcessStep step;
static DEL aids;                // cache for supported terminal AIDS; this cache will be used in emvSelect
static StructProcessStep g_TheProcessStep;
static int nCmdNumber;          //telium apdu dump number

#define mask8 ((byte)0x80)
#define mask6 ((byte)0x20)
#define mask87 ((byte)0xC0)
#define mask54321 ((byte)0x1F)
#define mask7654321 ((byte)0x7F)

static void iniDEL(DEL * del) {
    int i;

    del->current = 0;
    del->count = 0;
    for (i = 0; i < MAX_DATA_ELEMENT; i++) {
        del->list[i].tag = 0;
        del->list[i].length = 0;
        del->list[i].ptValue = &(del->ValBuf[i][0]);
        memclr(&(del->ValBuf[i][0]), MAX_ELT_LENGTH);
    }
}

static void ini_DEL_(_DEL_ * del) {
    int i;

    del->current = 0;
    del->count = 0;
    for (i = 0; i < MAX_DATA_ELEMENT; i++) {
        del->list[i].tag = 0;
        del->list[i].length = 0;
        memclr(del->list[i].ptValue, sizeof(del->list[i].ptValue));
    }
}

static int AddDEL(DEL * del, unsigned long tag, byte * val, int len) {
    int dim;
    void *ptr;

    dim = del->count;
    if(dim >= MAX_DATA_ELEMENT)
        return -1;              //overflow

    del->list[dim].tag = tag;

    if(len > MAX_ELT_LENGTH)
        len = MAX_ELT_LENGTH;
    del->list[dim].length = len;

    if(val != NULL) {
        ptr = &(del->ValBuf[dim][0]);
        memcpy(ptr, val, len);
        del->list[dim].ptValue = ptr;
    }
    del->count++;
    return 0;
}

static int getDELVal(DEL * del, card tag, byte ofs, byte * buf) {
    int idx;
    int num = 0;
    int len;

    VERIFY(del);
    VERIFY(buf);

    for (idx = 0; idx < del->count; idx++) {
        if(del->list[idx].tag != tag)
            continue;
        if(num == ofs)
            break;
        num++;
    }
    len = del->list[idx].length;
    memcpy(buf, del->list[idx].ptValue, len);

    return len;
}

static int add_DEL_(_DEL_ * del, unsigned long tag, byte * val, int len) {
    int dim;
    void *ptr;

    dim = del->count;
    if(dim >= MAX_DATA_ELEMENT)
        return -1;              //overflow

    del->list[dim].tag = tag;
    del->list[dim].length = len;
    ptr = val;
    if(ptr != NULL)
        memcpy(del->list[dim].ptValue, ptr, len);
    del->count++;
    return 0;
}

static void cpyDELinp(DEL * src, _DEL_ * dst) {
    int i, len, dim;

    ini_DEL_(dst);
    if(!src)
        return;

    dst->current = src->current;
    dim = src->count;
    if(dim >= MAX_DATA_ELEMENT)
        dim = MAX_DATA_ELEMENT;
    dst->count = dim;

    for (i = 0; i < dim; i++) {
        dst->list[i].tag = src->list[i].tag;
        len = src->list[i].length;
        if(len >= MAX_ELT_LENGTH)
            len = MAX_ELT_LENGTH;
        dst->list[i].length = len;
        memcpy(dst->list[i].ptValue, &(src->ValBuf[i][0]), len);
    }
}

static void cpyDELout(_DEL_ * src, DEL * dst) {
    int i, len;

    iniDEL(dst);
    dst->current = src->current;
    dst->count = src->count;
    for (i = 0; i < src->count; i++) {
        dst->list[i].tag = src->list[i].tag;
        len = src->list[i].length;
        if(len >= MAX_ELT_LENGTH)
            len = MAX_ELT_LENGTH;
        dst->list[i].length = len;
        memcpy(&(dst->ValBuf[i][0]), src->list[i].ptValue, len);
    }
}

static byte FallBackAllowed;
void emvSetFallBack(byte sta) {
    FallBackAllowed = sta;
    mapPutByte(traFallback,sta);
    trcFN("Fallback set: %d\n", sta);
}

byte emvGetFallBack(void) {
    return FallBackAllowed;
}

static void emvFallBackCheck(int res) {
    switch (res) {
      case TAG_PROCESSING_ERROR:
      case TAG_CARD_ERROR:
      case TAG_CDA_FAILED:
      case TAG_NOT_ACCEPTED:
      case TAG_ICC_DATA_FORMAT_ERROR:
      case TAG_MISSING_MANDATORY_ICC_DATA:
      case TAG_ICC_REDUNDANT_DATA:
      case TAG_MISSING_MANDATORY_TERM_DATA:
      case TAG_INTERNAL_ERROR:
      case TAG_SERVICE_NOT_ALLOWED:
      case TAG_NOT_ENOUGH_MEMORY:
      case TAG_WRONG_DEL_INPUT_PARAMETER:
          emvSetFallBack(1);
          break;

      case TAG_PROCESS_COMPLETED:
      case TAG_VISA_EASY_ENTRY_CARD:
          break;                // don't use fallback for it

          // Fallback forbidden
          // what to do if application blocked?
      case TAG_CARD_BLOCKED:
      case TAG_COND_OF_USE_NOT_SATISFIED:
      case TAG_REMOVED_CARD:
      default:
          emvSetFallBack(0);
          break;
    }
}

static int EMVDC_ServiceCall(const DEL * del, int fun, unsigned short srv) {
    int ret;
    unsigned char pri;          //priority
    byte buf[256];

    if(ServiceGet(96, srv, &pri) != 0)
        goto lblKO;

    if(del != NULL)
        cpyDELinp((DEL *) del, &step.DEL);
    else
        ini_DEL_(&step.DEL);

    step.type_code = 96;
    step.service = srv;
    step.function = fun;
    USR_ServiceCall(96, srv, sizeof(step), &step, &ret);
    cpyDELout(&step.DEL, &locDEL);
    outDEL = &locDEL;

    ret = getDELVal(outDEL, TAG_SERVICE_STATUS_CODE, 0, buf);
    CHECK(ret >= 0, lblKO);
    ret = WORDHL(buf[0], buf[1]);
    emvFallBackCheck(ret);
    return ret;
  lblKO:
    return -1;
}

#define EMVDC_STEP(STEP) EMVDC_ServiceCall(&inpDEL,STEP,SERVICE_EMVDC_PROCESS_STEP)

/* Convert an input queue of (Tag,Value) pairs into SAGEM DEL format
 * \param dat (O) all tags and values in DEL format
 * \param que (I) Input queue containing intermittent T,V,T,V,... pairs
 *   Each tag is a card number.
 *   The values are arbitrary binary data, the first byte being the length of the value.
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 */
static int cvtVD(DEL * del, tQueue * que) {
    int ret;
    card tag;
    word len;
    byte val[512];
    byte frc = 0;               //force online

    VERIFY(que);
    VERIFY(del);

    iniDEL(del);
    ret = 0;

    queRewind(que);

    while(queLen(que)) {
        CHECK(queNxt(que) == sizeof(card), lblKO);
        ret = queGet(que, (byte *) & tag);  //retrieve tag
        VERIFY(ret == sizeof(card));

        memset(val, 0, 512);
        CHECK(queNxt(que) <= 512, lblKO);
        ret = queGet(que, val); //retrieve value
        VERIFY(ret > 0);
        VERIFY(ret <= 512);
        len = ret;              //and it is the length

        //tag force online and hot list for telium is 2 bytes
        if(((tag == tagFrcOnl) || (tag == tagHotLst)) && (len == 1)) {
            if(val[0]) {
                frc = 1;
            }
            val[0] = frc ? 0x80 : 0;
            val[1] = 0;
            len = 2;
        }
        //ignore ingedev tags
        switch (tag) {
          case tagLstRspCod:
          case tagAmtPrv:
          case tagTrmDc1:
          case tagTrmDc2:
          case tagScrRes:
          case tagOnlSta:
              break;
          default:
              AddDEL(del, tag, val, len);
              break;
        }
    }
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Convert DEL encoded data into a queue containing tags and values
 * \param que (O) Output queue containing intermittent T,V,T,V,... pairs
 *   Each tag is a card number.
 *   The values are arbitrary binary data, the first byte being the length of the value.
 * \param del (O) all tags and values in DEL format
 * \param que (I) Input queue containing intermittent T,V,T,V,... pairs
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 */
static int cvtDV(tQueue * que, DEL * del) {
    int ret;
    int idx;
    card tag;
    word len;
    byte *val;

    VERIFY(que);
    VERIFY(del);

    queReset(que);
    idx = 0;
    while(idx < del->count) {
        tag = del->list[idx].tag;
        val = del->list[idx].ptValue;
        len = del->list[idx].length;

        ret = quePut(que, (byte *) & tag, sizeof(card));    //save tag into queue
        CHECK(ret == sizeof(card), lblKO);

        ret = quePut(que, val, (word) len); //save data into queue
        CHECK(ret >= 0, lblKO);

        idx++;
    }
    return queLen(que);
  lblKO:
    trcErr(ret);
    return -1;
}

static int cvtTD(DEL * del, tQueue * que) { // Add tags only from queue to DEL struct
    int ret;
    card tag;

    VERIFY(que);
    VERIFY(del);

    iniDEL(del);
    ret = 0;
    while(queLen(que)) {
        CHECK(queNxt(que) == sizeof(card), lblKO);
        ret = queGet(que, (byte *) & tag);  //retrieve tag
        VERIFY(ret == sizeof(card));
        AddDEL(del, tag, 0, 0);
    }
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static int cvtAQ(tQueue * que, card * arr) {    //convert array to queue
    int idx = 0;
    int ret;
    card tag;

    while(arr[idx]) {
        tag = arr[idx];
        ret = quePut(que, (byte *) & tag, sizeof(card));    //save tag into queue
        CHECK(ret == sizeof(card), lblKO);
        idx++;
    }
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

static int getDCdata(tQueue * inpQueT, tQueue * outQueV, DEL * inpDEL,
                     DEL * outDEL) {
    int ret;

    VERIFY(inpQueT);            //input que of tags
    VERIFY(outQueV);            //output que of values
    VERIFY(inpDEL);             //input DEL
    VERIFY(outDEL);             //output DEL
    iniDEL(inpDEL);

    ret = cvtTD(inpDEL, inpQueT);   //convert tags from queue to DEL
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_ServiceCall(inpDEL, 0, SERVICE_EMVDC_GET_DATA_ELEMENTS);    //request tags and values from kernel
    CHECK(ret >= 0, lblKO);

    ret = cvtDV(outQueV, outDEL);
    CHECK(ret >= 0, lblKO);
    return 1;
  lblKO:
    return -1;
}

static int fndTAGdel(card tag) {    //Search for tag in outDEL
    int i = 0;

    while(i < outDEL->count) {
        if(outDEL->list[i].tag == tag)
            return 1;
        i++;
    }
    return 0;
}

static int fndTAGque(tQueue * que, card tag) {  //// Find tag in Que (Tags + values)
    int ret;
    int found;
    card cur;                   //current tag
    byte buf[256];

    VERIFY(que);
    queRewind(que);
    found = 0;
    while(queLen(que)) {
        VERIFY(queNxt(que) == sizeof(card));
        ret = queGet(que, (byte *) & cur);
        VERIFY(ret == sizeof(card));
        if(cur == tag) {
            found = 1;
            break;
        }

        VERIFY(queNxt(que) <= 256);
        ret = queGet(que, buf); //get value
        VERIFY(ret >= 0);
        VERIFY(ret < 256);
    }
    queRewind(que);

    return found;
}

static int __EMVDC_ServiceCall(unsigned short usAppType,
                               unsigned short usServiceID,
                               StructProcessStep * pProcessStep) {
    unsigned char ucPriority;
    int nResult;

    // Does the service exist ?
    if(ServiceGet(usAppType, usServiceID, &ucPriority) == 0) {
        // Call the service
        if(ServiceCall
           (usAppType, usServiceID, sizeof(StructProcessStep), pProcessStep,
            &nResult) == 0) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        // The service does not exist
        return FALSE;
    }
}

static int __EMVDC_Service(unsigned short usServiceID, int nFunction,
                           StructProcessStep * pProcessStep) {
    int bResult;

    pProcessStep->type_code = 0x60;
    pProcessStep->service = usServiceID;
    pProcessStep->function = nFunction;

    // Call the service
    bResult = __EMVDC_ServiceCall(0x60, usServiceID, pProcessStep);
    if(!bResult) {
        ini_DEL_(&pProcessStep->DEL);
    }

    return bResult;
}

static void getApduValue(int tag, byte * buf) {
    switch (tag) {
      case TAG_APDU_CMD:
          memcpy(buf, g_TheProcessStep.DEL.list[0].ptValue,
                 g_TheProcessStep.DEL.list[0].length);
          break;

      case TAG_APDU_RESPONSE:
          memcpy(buf, g_TheProcessStep.DEL.list[1].ptValue,
                 g_TheProcessStep.DEL.list[1].length);
          break;

      default:
          break;
    }
}

static unsigned int getApduLength(int tag) {
    switch (tag) {
      case TAG_APDU_CMD:
          return g_TheProcessStep.DEL.list[0].length;

      case TAG_APDU_RESPONSE:
          return g_TheProcessStep.DEL.list[1].length;

      default:
          return 0;
    }
}

static byte emvIsConstructed(const byte * tag) {    //is it a constructed tag?
    return ((*tag) & mask6) ? 1 : 0;
}

static int emvGetT(byte * tag, const byte * src) {  //extract tag from BER-TLV encoded buffer
    byte ret;

    *tag = *src;
    ret = 1;
    if(((*tag) & mask54321) != mask54321)
        return 1;               //1-byte tag
    do {                        //process multiple-byte tags
        ret++;
        tag++;
        src++;
        *tag = *src;
    } while((*tag) & mask8);
    VERIFY(ret <= 2);           //only 1-byte and 2-byte tags are supported
    return ret;
}

static int emvGetL(card * len, const byte * src) {  //extract length from BER-TLV encoded buffer
    byte ret;

    if(((*src) & mask8) != mask8) { //the easiest case : 1-byte length
        *len = *src;
        ret = 1;
        goto lblOK;
    }
    ret = (*src) & mask7654321;
    src++;
    *len = *src++;
    ret--;

    if(ret == 0) {              //two-byte length
        ret = 2;
        goto lblOK;
    }
    *len *= 0x100;
    *len += *src++;
    ret--;

    if(ret == 0) {              //three-byte length
        ret = 3;
        goto lblOK;
    }
    *len *= 0x100;
    *len += *src++;
    ret--;

    if(ret == 0) {              //four-byte length
        ret = 4;
        goto lblOK;
    }
    *len *= 0x100;
    *len += *src++;
    ret--;
    if(ret == 0) {              //five-byte length
        ret = 5;
        goto lblOK;
    }

    return -1;                  //very long TLVs are not supported
  lblOK:
    return ret;
}

static int emvGetTLV(byte * tag, card * len, byte * val, const byte * src) {    //retrieve TLV from BER-TLV encoded buffer
    word ret;

    ret = emvGetT(tag, src);    //extract tag
    if(*tag == 0)               //It is the case for ETEC 6.0, Interoper.07 MC
        return -1;
    if(ret > 2)
        return -1;

    ret += emvGetL(len, src + ret); //extract length
    src += ret;

    memcpy(val, src, (word) * len); //extract value
    ret += (word) * len;

    return ret;
}

static int trcEmvDbgReq(const byte * dbg) { //trace request APDUs
    char *cmd;
    int len = 0;

    len = getApduLength(TAG_APDU_CMD);

    trcS("  CLA INS P1 P2 L>: ");
    trcBN(dbg, 5);              //it is the command itself in the binary format

    cmd = "";
    if(memcmp(dbg, "\x00\xA4", 2) == 0)
        cmd = " [APPLICATION SELECT]";
    else if(memcmp(dbg, "\x00\xB2", 2) == 0)
        cmd = " [READ RECORD]";
    else if(memcmp(dbg, "\x80\xA8", 2) == 0)
        cmd = " [GET PROCESSING OPTIONS]";
    else if(memcmp(dbg, "\x80\xCA", 2) == 0)
        cmd = " [GET DATA]";
    else if(memcmp(dbg, "\x80\xAE", 2) == 0)
        cmd = " [GENERATE AC]";
    else if(memcmp(dbg, "\x00\x88", 2) == 0)
        cmd = " [INTERNAL AUTHENTICATE]";
    else if(memcmp(dbg, "\x00\x82", 2) == 0)
        cmd = " [EXTERNAL AUTHENTICATE]";
    else if(memcmp(dbg, "\x00\x84", 2) == 0)
        return -1;              //invalid tag
    trcS(cmd);                  //trace command name
    trcS("\n");

    if(len > 5) {               //if there are data, trace it
        dbg += 5;
        trcS("    ");
        trcBN(dbg, len - 5);    //in binary format
        trcS(" [");
        trcAN(dbg, len - 5);    //and in ASCII format
        trcS("]\n");
    }
    return len + 2;

    return len + 2;
}

static int trcEmvDbgRsp(const byte * dbg) { //trace reponse APDU
    int ret, dim, idx, totalInc;
    byte tag[2];                //EMV tag
    card len;                   //EMV tag length
    byte val[1024];             //EMV tag value
    char tmp[1024];

    //extract debug info portion length
    ret = idx = totalInc = dim = 0;
    dim = getApduLength(TAG_APDU_RESPONSE);
    if(dim < 0)
        return 0;
    idx = dim - 2;

    trcS("  Response<:");
    trcBN(dbg, idx);            //trace raw binary response
    trcS("\n");

    while(idx > 0) {            //parse EMV TLVs from response using BER-TLV coding rules
        memset(tag, 0, 2);
        memset(val, 0, 1024);
        ret = emvGetTLV(tag, &len, val, dbg);   //retrieve (tag,length,value)
        if(ret < 0)
            break;

        memset(tmp, 0, 1024);
        if((int) len > (dim - 4))
            len = (card) (dim - 4);
        sprintf(tmp, "   T=%02X%02X L=%3u V= ", (word) tag[0], (word) tag[1],
                (word) len);
        trcS(tmp);
        trcBN(val, len);        //value in binary fromat

        if(!emvIsConstructed(tag)) {
            trcS(" [");
            trcAN(val, len);    //value in ASCII format
            trcS("]\n");
            if(ret > ((dim - 2) - totalInc))
                ret = ((dim - 2) - totalInc);
            dbg += ret;
            totalInc += ret;
            idx -= ret;
            continue;
        }
        //process constructed tags
        trcS("\n");

        if(idx <= 0)
            break;
        ret = emvGetT(tag, dbg);    //skip tag
        if(ret > ((dim - 2) - totalInc))
            ret = ((dim - 2) - totalInc);
        dbg += ret;
        totalInc += ret;
        idx -= ret;

        if(idx <= 0)
            break;
        ret = emvGetL(&len, dbg);   //skip length
        if(ret < 0)
            break;
        if(ret > ((dim - 2) - totalInc))
            ret = ((dim - 2) - totalInc);
        dbg += ret;
        totalInc += ret;
        idx -= ret;
        //and now continue retrieving values
    }

    memset(tmp, 0, 1024);
    sprintf(tmp, "   SW1SW2=%02X%02X\n", (word) dbg[0], (word) dbg[1]);
    trcS(tmp);

    return dim + 2;
}

static void trcEmvDbg(void) {   //trace debug info buffer after calling a kernel function
    int ret, ret2;
    byte dbg[1024];

    trcS("EMV DBG:\n");
    ini_DEL_(&g_TheProcessStep.DEL);
    ret = ret2 = 0;

    while(ret >= 0) {
        ret =
            __EMVDC_Service(SERVICE_EMVDC_GET_COMMAND, nCmdNumber,
                            &g_TheProcessStep);
        if(getApduLength(TAG_APDU_CMD) == 0)
            break;
        memset(dbg, 0, sizeof dbg);
        getApduValue(TAG_APDU_CMD, dbg);
        ret2 = trcEmvDbgReq(dbg);
        memset(dbg, 0, sizeof dbg);
        CHECK(ret2 >= 0, lblEnd);
        getApduValue(TAG_APDU_RESPONSE, dbg);
        trcEmvDbgRsp(dbg);
      lblEnd:
        nCmdNumber++;
    }
}

/** Emulation of the function amgSelectionCriteriaAdd.
 * Using this function an application informs EMV Kernel that it can accept a number of EMV AIDs.
 * Normally this function should be used only at the POWERON event processing.
 *
 * \param que (I) A queue that contains all the EMV AIDs that are accepted by the terminal application
 * \return
 *    - negative if failure.
 *    - otherwise number of AIDs registered.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0011.c
 */
int emvInit(tQueue * que) {
    int dim;                    //number of AIDs in the queue que
    int idx;                    //loop counter, 0..dim-1
    int ofs;
    int len;                    //length of current AID
    byte arr[512];              //current AID retrieved from the input queue
    byte aid[16 + 1];
    byte tmp[16 + 1];
    byte err[2];

    nCmdNumber = 0;             //reset telium apdu dump every emv init
    VERIFY(que);
    trcS("emvInit\n");

    emvSetFallBack(0);
    dim = queLen(que);          //number of AIDs announced by the terminal application
    VERIFY(dim);
    queRewind(que);
    ofs = 0;
    for (idx = 0; idx < dim; idx++) {
        VERIFY(queNxt(que) <= 16);
        len = queGet(que, aid); //retrieve AID from the input queue
        VERIFY(len > 0);
        VERIFY(len <= 16);
        memcpy(&tmp[1], aid, len);
        tmp[0] = (byte) len;    // add len
        memcpy(&arr[ofs], tmp, len + 1);
        ofs += (len + 1);
    }
    len = ofs;
    iniDEL(&aids);              // in this case we use aids instead of inDEL for caching AIDS
    AddDEL(&aids, TAG_AID_LIST_TERMINAL, arr, len);
    memset(err, 0, 2);
    AddDEL(&aids, TAG_CUST_STATUS_CODE, err, 2);    // Send error code = 0x0000

    trcS("emvInit que:\n");
    trcQ(que);
    queRewind(que);
    return dim;
}

/** Emulation of the function amgInitPayment.
 * This function should be called before starting EMV processing.
 * \return
 *    - negative if failure
 *    - otherwise 1
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0012.c
 */

int emvStart(void) {
    int ret;
    byte tmp[256];

    nCmdNumber = 0;             //reset telium apdu dump every emv init

    trcS("emvStart\n");
    iniDEL(&inpDEL);

    ret = EMVDC_STEP(EMVDC_START);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    iniDEL(&inpDEL);

    memcpy(tmp, "\x01", LEN_USE_PSE);
    AddDEL(&inpDEL, TAG_USE_PSE, tmp, LEN_USE_PSE);

    memcpy(tmp, "\x01", LEN_COMPATIBILITY_2000);
    AddDEL(&inpDEL, TAG_COMPATIBILITY_2000, tmp, LEN_COMPATIBILITY_2000);

    memcpy(tmp, "\x00", 1);
    AddDEL(&inpDEL, TAG_WAIT_AFTER_ATR, tmp, 1);

    memcpy(tmp, "\x00", 1);
    AddDEL(&inpDEL, TAG_PSE_ALGO, tmp, 1);

    ret =
        EMVDC_ServiceCall(&inpDEL, 0, SERVICE_EMVDC_SET_DEFAULT_DATA_ELEMENTS);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

/** Emulation of the function amgEndPayment and amgEmvCloseContext.
 * This function should be called after finishing EMV processing and in case of exceptions.
 * \return no
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0012.c
 */
void emvStop(void) {
    trcS("emvStop\n");
    nCmdNumber = 0;             //reset telium apdu dump
    iniDEL(&inpDEL);
    EMVDC_STEP(EMVDC_STOP);
}

int emvCheckFallback(void) {
    int ret = 0;
    int len = 0;
    int i = 0;
    int fallback = 0;
    byte buf[1024];
    byte PSE_CMD[] =
        { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x31, 0x50, 0x41, 0x59, 0x2E, 0x53,
        0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00
    };

    trcS("emvCheckFallback Beg\n");
    iniDEL(&inpDEL);
    AddDEL(&inpDEL, TAG_NUMBER_OF_MATCHING_APP, NULL, 0);
    AddDEL(&inpDEL, TAG_LAST_APDU_CMD_RESPONSE, NULL, 0);

    ret = EMVDC_ServiceCall(&inpDEL, 0, SERVICE_EMVDC_GET_DATA_ELEMENTS);
    CHECK(ret >= 0, lblKO);

    ret = getDELVal(outDEL, TAG_LAST_APDU_CMD_RESPONSE, 0, buf);
    if(ret == LEN_LAST_APDU_CMD_RESPONSE) {
        if((buf[4] == 0x6A) && (buf[5] == 0x81))
            return 0;
    }

    memset(buf, 0, sizeof buf);
    ret = getDELVal(outDEL, TAG_NUMBER_OF_MATCHING_APP, 0, buf);
    if(buf[0] == 0) {
        trcS("There were no matching applications\n");
        ini_DEL_(&g_TheProcessStep.DEL);
        nCmdNumber = 0;

        while(ret >= 0) {
            ret =
                __EMVDC_Service(SERVICE_EMVDC_GET_COMMAND, nCmdNumber,
                                &g_TheProcessStep);
            CHECK((getApduLength(TAG_APDU_CMD) != 0), lblEnd);

            memset(buf, 0, sizeof buf);
            getApduValue(TAG_APDU_CMD, buf);

            //check if select command and not PSE
            if((buf[0] == 0x00) && (buf[1] == 0xA4)
               && memcmp(buf, PSE_CMD, 20) != 0) {
                i = 0;
                len = getApduLength(TAG_APDU_RESPONSE);
                trcFN("length of APDU response = %d\n", len);

                memset(buf, 0, sizeof buf);
                getApduValue(TAG_APDU_RESPONSE, buf);

                while(i < len) {
                    i++;
                }

                //check if application is blocked
                if((buf[len - 2] == 0x62) && (buf[len - 1] == 0x83)) {
                    trcS("Fallback not Allowed\n");
                    fallback = 0;
                    goto lblEnd;
                } else {
                    trcS("Fallback Allowed\n");
                    fallback = 1;
                }
            }
            nCmdNumber++;
        }
    }

    goto lblEnd;

  lblKO:
    trcErr(ret);
    return 0;
  lblEnd:
    return fallback;
}

/** Emulation of the function amgEmvSelectStatus.
 *  This function will return the status of emvapplication selection
 *  after emvSelect() if emvSelect is returning the value less than or equal 0
 * \param pbm (O) returning SW
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcnl0029.c
 */
void emvSelectStatus(word * pbm) {
    int ret = 0;
    byte buf[256];

    VERIFY(pbm);
    *pbm = 0;

    iniDEL(&inpDEL);
    AddDEL(&inpDEL, TAG_LAST_APDU_CMD_RESPONSE, NULL, 0);
    ret = EMVDC_ServiceCall(&inpDEL, 0, SERVICE_EMVDC_GET_DATA_ELEMENTS);
    CHECK(ret >= 0, lblKO);

    ret = getDELVal(outDEL, TAG_LAST_APDU_CMD_RESPONSE, 0, buf);
    if(ret == LEN_LAST_APDU_CMD_RESPONSE)
        *pbm = (word) (buf[4] << 8) + buf[5];

    trcFN("SW1SW2: %04X\n", *pbm);
  lblKO:
    return;
}

/** Emulation of the function amgAskSelection.
 * Creates a list of AIDs from the card that are accepted by the Kernel (see emvInit function).
 * Returns a queue of (AID, application name,priority indicator) triples sorted according
 * to the priority.
 * This queue should be used by the application to select the first element of the queue,
 * or to organise a menu of AIDs, ask for the confirmation or just perform a final select call,
 * depending on the application capabilities.
 * According the EMV specification, the confirmation should be asked if the highest bit
 * of priority indicator is set.
 * For an idle application manipulating with several EMV applications this wrapper is not suitable:
 * the association between AIDs and PIDs is removed.
 * \param que (O) Pointer to the queue containing the triples (AID, application name, priority indicator). The length of AID is no more than 16 and can be found using queNxt function.
 *    The application name is also no more than 16 characters long.
 *    The priority indicator is always 1-byte long.
 *    The Kernel gives application preferred name if it is contained in the card answer
 *    and if it can be displayed for a given code index table.
 * \return
 *    - the number of AIDs in the queue.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0012.c
 */
int emvSelect(tQueue * que) {
    long int ret;
    byte dim;                   //the number of AIDs5
    byte codIdx;
    byte idx;                   //loop counter, 0..dim-1
    byte buf[1024];

    ret = EMVDC_ServiceCall(&aids, 0, SERVICE_EMVDC_APPLICATION_SELECTION); //use cached ids
    CHECK(ret >= 0, lblKO);

    queReset(que);
    dim = 0;
    if(fndTAGdel(TAG_NUMBER_OF_MATCHING_APP) == 0)
        goto lblEnd;

    ret = getDELVal(outDEL, TAG_NUMBER_OF_MATCHING_APP, 0, &dim);
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < dim; idx++) {
        //get AID from DEL and put it to queue
        ret = getDELVal(outDEL, TAG_AID_ICC, idx, buf);
        CHECK(ret >= 0, lblKO);
        ret = quePut(que, buf, ret);    //put it to queue
        CHECK(ret >= 0, lblKO);

        if(fndTAGdel(TAG_ISSUER_CODE_TABLE_INDEX) == 0)
            goto lblEnd;

        ret = getDELVal(outDEL, TAG_ISSUER_CODE_TABLE_INDEX, 0, &codIdx);
        CHECK(ret >= 0, lblKO);

        if(codIdx == 1)         //Should be (codIdx > 0) but now accept 1 only since on additional terminal capabilities table 1 is only supported
        {
            //get application name from DEL and put it to queue
            ret = getDELVal(outDEL, TAG_APPLICATION_PREFFERRED_NAME, idx, buf);
            CHECK(ret >= 0, lblKO);

            if(ret == 0) {      // if length of application_preferred name == 0
                ret = getDELVal(outDEL, TAG_APPLICATION_LABEL, idx, buf);
                CHECK(ret >= 0, lblKO);
            }
        } else {
            //get application label from DEL and put it to queue
            ret = getDELVal(outDEL, TAG_APPLICATION_LABEL, idx, buf);
            CHECK(ret >= 0, lblKO);
        }
        ret = quePut(que, buf, ret);
        CHECK(ret >= 0, lblKO);

        //get priority indicator from DEL and put it to queue
        ret = getDELVal(outDEL, TAG_APPLI_PRIOR_IND, idx, buf);
        CHECK(ret >= 0, lblKO);
        if(!ret) {              //missing application priority indicator
            *buf = 0;
            ret = 1;
        }
        ret = quePut(que, buf, ret);    //EMV Priority
        CHECK(ret >= 0, lblKO);
    }
    ret = dim;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    trcEmvDbg();
    queRewind(que);
    trcS("que:\n");
    trcQ(que);
    return ret;
}

/** Set Final Select buffer pointer
 * Not used in Telium
 * \return no
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0013.c
 */
void emvSetFSbuf(byte * buf) {
}

/**
 * \todo To be documented
 */
int emvSetSelectResponse(byte * aid, void *buf) {
    _DEL_ *del;
    int i;

    del = (_DEL_ *) buf;
    memset(aid, 0, 1 + 16);
    for (i = 0; i < del->count; i++) {
        if(del->list[i].tag == TAG_GIVE_AID_AID_VALUE) {
            *aid = (byte) del->list[i].length;
            memcpy(aid + 1, del->list[i].ptValue, *aid);
            return *aid;
        }
    }
    return -1;
}

/** Emulation of the function amgEmvFinalSelect.
 * Perform an explicite final APPLICATION SELECT command to before starting the EMV transaction flow.
 * It is the last operation of application selection procedure.
 * The candidate is selected before from the list using a menu or application selection algorithm.
 * \param aid (I) The EMV AID to be selected.
 *   The first byte is the length of AID, it is followed by AID itself.
 *   For example, to select a VSDC application A0 00 00 00 03 10 10 00
 *   the value "\x07\xA0\x00\x00\x00\x03\x10\x10" should be given as an input of this function.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0013.c
 */
int emvFinalSelect(const byte * aid) {
    int ret;

    VERIFY(aid);
    iniDEL(&inpDEL);
    AddDEL(&inpDEL, TAG_AID_LIST_TERMINAL, (byte *) aid, aid[0] + 1);
    ret = EMVDC_STEP(EMVDC_FINAL_SELECTION);
    trcFN("EMVDC_FINAL_SELECTION sta=%04x\n", ret);
    trcS("aid=");
    trcBuf(aid, *aid + 1);
    trcS("\n");
    trcEmvDbg();
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Emulation of the function amgEmvGetLanguagePreference.
 * Retrieve language preference tag (5F2D).
 * \param lan (O) The buffer to accept language preference EMV tag.
 *   It points to a buffer contatining 9 characters (zero-ended 8 character string).
 * \remark
 *   This function can be called after emvFinalSelect function and gives the information
 *   concerning the languages accepted by the customer.
 *   Each two characters in lan string correspond to a language.
 *   For example, enfresde means English, Français, Espagnol and Deutsch.
 *   The first of these languages is preferred for user dialogs.
 * \return
 *    - number of languages if OK.
 *    - negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0013.c
 */
int emvGetLang(char *lan) {
    int len;
    int ret;
    int fnd = 0;

    VERIFY(lan);
    memset(lan, 0, 9);

    iniDEL(&inpDEL);
    ret = AddDEL(&inpDEL, TAG_LANGUAGE_PREFERENCE, NULL, 0);
    CHECK(ret >= 0, lblKO);
    ret = EMVDC_ServiceCall(&inpDEL, 0, SERVICE_EMVDC_GET_DATA_ELEMENTS);
    CHECK(ret >= 0, lblKO);

    fnd = fndTAGdel(TAG_LANGUAGE_PREFERENCE);
    if(fnd) {
        ret = getDELVal(outDEL, TAG_LANGUAGE_PREFERENCE, 0, lan);
        CHECK(ret >= 0, lblKO);
    }
    trcFS("emvGetLang lan=%s\n", lan);
    len = strlen(lan);

    VERIFY(len <= 8);
    switch (len) {
      case 0:
      case 2:
      case 4:
      case 6:
      case 8:
          return len / 2;
      default:
          break;
    }

  lblKO:
    return -1;
}

//Define the length of the buffer to be used between Kernel and application
#define L_BUF 1024

/** Emulation of the function amgEmvInitContext.
 * Initialise context before starting an EMV transaction.
 * A number of tags described in the EMV Kernel documentation should be transmitted to
 * EMV Kernel Transaction module.
 * The Kernel can optionally send a list of tags wanted.
 * The function call is simplified: there is no proprietary tags;
 * if needed a direct call to EMV Kernel should be used.
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
     The list of mandatory and optional tags can be found in EMV Kernel documentation.
     Only Application data are transmitted; Transaction data are not used being obsolete.
 * \param qTKA (I) Pointer to a queue containing tags that Kernel wants to know when the next
 *   Kernel function will be called.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0014.c
 */
int emvContext(tQueue * qVAK, tQueue * qTKA) {
    int ret;
    card tka[] = {
        0
    };

    // additional tags for SAGEM
    ret = fndTAGque(qVAK, tagSchemasKSU);
    if(!ret)
        quePutTlv(qVAK, tagSchemasKSU, 4, (byte *) "\x01\x00\x00\x00");
    ret = fndTAGque(qVAK, tagPinPadOK);
    if(!ret)
        quePutTlv(qVAK, tagPinPadOK, 1, (byte *) "\x01");

    ret = cvtVD(&inpDEL, qVAK);
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_INITIATE_APPLICATION_PROCESSING);
    trcFN("EMVDC_INITIATE_APPLICATION_PROCESSING sta=%04x\n", ret);
    trcEmvDbg();
    trcS("qVAK:\n");
    trcQueTlv(qVAK);

    cvtAQ(qTKA, tka);           // Prepare tags to Application for the next tags required by Kernel
    trcS("qTKA:\t");
    trcQueDol(qTKA);

    //CHECK(ret == TAG_PROCESS_COMPLETED, lblKO); //Bao comment this

    //return 1; // Bao comment this
    goto lblEnd;
  lblKO:
    trcErr(ret);
    return -1;

  //Bao add
  lblEnd:
  	return ret;
}

/** Emulation of the function amgEmvTransactionPreparation.
 * Load the information from the card.
 * At first the Kernel sends GET PROCESSING OPTIONS command to the card.
 * The card answer contains AFL, Application File Locator.
 * It is the list of records in card files to be read.
 * After that the Kernel reads all the information according to this AFL.
 *
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
 *   It is built from the queue qTKA obtained from the previous emvContext call.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain at the end
 *   of the execution of this function.
 *   For example, it can contain tags 5F34 (PAN), 57 (Track2) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \param qTKA (O) Pointer to a queue containing tags that Kernel wants to know when
 *   the next Kernel function will be called.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0015.c
 */
int emvPrepare(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    word sws;
    card tka[] = {
        //TAG_CUST_IS_ODA_PROCESSING,
        //TAG_CUST_DETECT_FAIL_PRIOR_TAA,
        //TAG_CUST_CDA_NEVER_REQUEST_ARQC_1GENAC,
        //TAG_CUST_CDA_NEVER_REQUEST_ONLINE_2GENAC,
        //TAG_TRANSACTION_DATE,
        TAG_ODA_PROCESSING,
        0x9F81A6,               //  TAG_DETECT_FAIL_PRIOR_TAA,
        0x9F81A7,               //  TAG_CDA_NEVER_REQUEST_ARQC_1GENAC,
        0x9F81A8,               //  TAG_CDA_NEVER_REQUEST_ONLINE_2GENAC,
        TAG_CUST_PAN_IN_BLACK_LIST,
        TAG_CERT_AUTH_PUB_KEY_MODULUS,
        TAG_CERT_AUTH_PUB_KEY_EXPONENT,
        0
    };

    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_READ_APPLICATION_DATA);  // Send command to kernel to read data from card
    trcFN("EMVDC_READ_APPLICATION_DATA ret=%d\n", ret);
    trcEmvDbg();
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK((ret == TAG_PROCESS_COMPLETED) || (ret == TAG_VISA_EASY_ENTRY_CARD), lblKO);

    emvSelectStatus(&sws);
    CHECK(sws == 0x9000, lblKO);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = getDCdata(qTAK, qVKA, &inpDEL, outDEL);
    CHECK(ret > 0, lblKO);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    cvtAQ(qTKA, tka);           // Prepare tags to Application for the next tags required by Kernel
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Emulation of the function amgEmvDataAuthentication.
 * The EMV Kernel performs the data authentication and sets the corresponding bits in TVR and TSI.
 * Before calling this function the queue qVAK should be prepared according the qTKA got
 * from the previous emvPrepare call.
 * Normally it contains the tags DF1F (RSA key) and DF7F (Public Key Exponent).
 * They should retrieved from the application table of CAPK public keys according to application AID
 * and public key index retrieved from emvPrepare call.
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
 *   It is built from the queue qTKA obtained from the previous emvPrepare call.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain at the end
 *   of the execution of this function.
 *   For example, it can contain tags 95 (TVR), 9B (TSI) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \param qTKA (O) Pointer to a queue containing tags that Kernel wants to know when the next Kernel
 *   function will be called.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0016.c
 */
int emvAuthenticate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    word sws;
    card tka[] = {              // Tags required for Kernel for the next step
        TAG_MAX_TARG_PERC_FOR_BIASED_RAND_SEL,
        TAG_TARG_PERC_FOR_BIASED_RAND_SEL,
        TAG_TRESH_VALUE_FOR_BIASED_RAND_SEL,
        TAG_TERMINAL_FLOOR_LIMIT,
        TAG_FLOOR_LIMIT_CHECKING,
        TAG_RANDOM_TRANSACTION_SELECT,
        TAG_VELOCITY_CHECKING,
        TAG_TRM_OVERPASS_AIP,
        TAG_CUMUL_AMOUNT,
        0
    };

    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_OFFLINE_DATA_AUTHENTICATION);
    trcFN("EMVDC_OFFLINE_DATA_AUTHENTICATION sta=%04X\n", ret);
    trcEmvDbg();
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    emvSelectStatus(&sws);
    CHECK(sws == 0x9000, lblKO);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = getDCdata(qTAK, qVKA, &inpDEL, outDEL);
    CHECK(ret > 0, lblKO);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    cvtAQ(qTKA, tka);           // Prepare tags to Application for the next tags required by Kernel

    trcS("qTKA:\t");
    trcQueDol(qTKA);
    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int GetCVMDataFromKernel(tQueue * qTAK, tQueue * qVKA, DEL * inpDEL, DEL * outDEL, bool off_pin) {   //is called by CVM processing functions
    int ret;
    byte CVMOutRes[2];          //UNICAPT32 compatibility tag

    //temporary queue for tags (to add Telium-specific tags)
    byte dT[1024];
    tBuffer bT;
    tQueue qT;

    //temporary queue for values (to add Telium-specific values)
    byte dV[2048];
    tBuffer bV;
    tQueue qV;

    card tag;
    word len;
    byte val[256];
    byte idx, idxSignature, idxRemainPin, idxStatusCVP, idxCVMOutRes;

    // Copy queue qTAK -> qT
    bufInit(&bT, dT, sizeof(dT));
    queInit(&qT, &bT);
    bufInit(&bV, dV, 2048);
    queInit(&qV, &bV);

    idx = 0;
    idxSignature = idxRemainPin = idxStatusCVP = idxCVMOutRes = 0;
    queRewind(qTAK);
    while(queLen(qTAK)) {       // Copy all tags, skipping tagCVMOutRes
        idx++;
        ret = queGetTag(qTAK, &tag);
        VERIFY(ret >= 0);
        switch (tag) {          //save indexes of Telium and UNicapt specific tags
          case tagCVMOutRes:
              idxCVMOutRes = idx;
              continue;         //skip it
          case tagSignature:
              idxSignature = idx;
              break;
          case tagRemainPin:
              idxRemainPin = idx;
              break;
          case tagStatusCVP:
              idxStatusCVP = idx;
              break;
          default:
              break;
        }
        ret = quePutTag(&qT, tag);
        VERIFY(ret >= 0);
    }

    //Add Telium specific tags if needed
    if(!idxSignature) {
        ret = quePutTag(&qT, tagSignature);
        VERIFY(ret >= 0);
        idxSignature = 0;
    }

    if(!idxRemainPin) {
        ret = quePutTag(&qT, tagRemainPin);
        VERIFY(ret >= 0);
        idxRemainPin = 0;
    }

    if(!idxStatusCVP) {
        ret = quePutTag(&qT, tagStatusCVP);
        VERIFY(ret >= 0);
        idxStatusCVP = 0;
    }

    trcS("qTAK for Telium CVM:\t");
    trcQueDol(&qT);

    ret = getDCdata(&qT, &qV, inpDEL, outDEL);
    CHECK(ret > 0, lblKO);

    trcS("qVKA for Telium CVM:\n");
    trcQueTlv(&qV);

    // Copy queue qV -> qVKA
    memset(CVMOutRes, 0, 2);
    while(queLen(&qV)) {        //Detect CVMOutRes-filling tags , parse and skip
        ret = queGetTlv(&qV, &tag, &len, val);
        VERIFY(ret >= 0);
        switch (tag) {
          case tagStatusCVP:
              VERIFY(len == 1);
              switch (val[0]) {
                case cvmINPUT_PIN_ON:
                    bitOn(CVMOutRes, cvmOnlinePin);
                    //@@SIMAS-PIN_BYPASS : remark the below line
                    //bitOn(CVMOutRes, cvmEnding); //causes online pin not to be processed
                    break;
                case cvmINPUT_PIN_OFF:
                    if(off_pin)
                        bitOn(CVMOutRes, cvmWrongPin);
                    bitOn(CVMOutRes, cvmOfflinePin);
                    break;
                case cvmINPUT_PIN_OK:
                    bitOn(CVMOutRes, cvmOfflinePinOK);
                    bitOn(CVMOutRes, cvmEnding);
                    break;
                case cvmNO_REMAINING_PIN:  // ZAK_TODO
                    bitOn(CVMOutRes, cvmEnding);
                    break;
                case cvmNO_INPUT:
                    bitOn(CVMOutRes, cvmEnding);
                    break;
                default:       // ZAK_TODO : simulate an error
                    break;
              }
              break;

          case tagRemainPin:
              VERIFY(len == 1);
              if(val[0] == 1)
                  bitOn(CVMOutRes, cvmLastAttempt);
              break;

          case tagSignature:
              VERIFY(len == 1);
              if(val[0] == 1)
                  bitOn(CVMOutRes, cvmSignature);
              //bitOn(CVMOutRes, cvmEnding); //fix for terminal that supports both pin and signature
              break;

          default:
              break;
        }
    }

    queRewind(&qV);
    queReset(qVKA);
    idx = 1;
    while(queLen(&qV)) {        //Detect CVMOutRes-filling tags , parse and skip
        if(idx == idxCVMOutRes) {
            ret = quePutTlv(qVKA, tagCVMOutRes, 2, CVMOutRes);
            VERIFY(ret >= 0);
            idx++;
            continue;
        }
        ret = queGetTlv(&qV, &tag, &len, val);
        VERIFY(ret >= 0);
        switch (tag) {
          case tagSignature:
              if(idx != idxSignature)
                  continue;
          case tagRemainPin:
              if(idx != idxRemainPin)
                  continue;
          case tagStatusCVP:
              if(idx != idxStatusCVP)
                  continue;
          default:
              break;
        }
        ret = quePutTlv(qVKA, tag, len, val);
        VERIFY(ret >= 0);
        idx++;
    }

    //RSP_NO_REMAINING_PIN  = 52,       /**< PIN Try limit Exceeded */
    //RSP_INPUT_PIN_OK      = 55,       /**< PIN entry performed */
    //RSP_PP_HS             = 60,       /**< Pinpad Out of Services */
    //RSP_CANCEL_INPUT      = 61,       /**< Pin entry cancelled */
    //RSP_TO_INPUT          = 62,       /**< Time Out during PIN Entry */
    //RSP_REMOVED_CARD      = 63,       /**< Card Removed */
    //RSP_ERROR_CARD        = 64,       /**< Card Error */
    //RSP_MORE              = 65,       /**< Internal use only */
    // cvmPinRevocationPossible,    ?
    // cvmOfflinePinOK,             ?
    // cvmWrongPin,                 ?
    trcEmvDbg();
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Emulation of the function amgEmvCardHolderVerification with the first parameter CV_INITIAL.
 * Should be called before starting CVM processing.
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
 *   It is built from the queue qTKA obtained from the previous emvAuthenticate call.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain at the end
 *   of the execution of this function.
 *   For example, it can contain tags 95 (TVR), 9B (TSI), DF38(CVMOUT Result), 9F34(CVM Result) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0017.c
 */
int emvCVMstart(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA) {
    int ret;
    word sws;

    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_CARDHOLDER_VERIFICATION_FIRST);
    trcFN("EMVDC_CARDHOLDER_VERIFICATION_FIRST ret=%d\n", ret);
    trcEmvDbg();
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    emvSelectStatus(&sws);
    CHECK(sws == 0x9000, lblKO);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = GetCVMDataFromKernel(qTAK, qVKA, &inpDEL, outDEL, 0);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

/** Emulation of the function amgEmvCardHolderVerification.
 * Normally this function is called inside a loop processing CVMs.
 * The decision to quit or to continue processing is based on the value of the tag DF38 (CVMOUT Result).
 * Offline pin processing is treated using a special emvCVMoff wrapper.
 * \param opt (I) entry option that describes the result of the last CVM. It can take only three values:
 *    -  1: the previous CVM is performed successfully
 *    - -1: the previous CVM failed
 *    -  0: the previous CVM was bypassed
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain at the end
 *   of the execution of this function.
 *   For example, it can contain tags 95 (TVR), 9B (TSI), DF38(CVMOUT Result), 9F34(CVM Result) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 */
int emvCVMnext(int opt, tQueue * qTAK, tQueue * qVKA) {
    int ret;
    word sws;
    byte buf[128];
    word pinDigits;

    iniDEL(&inpDEL);
    if(!opt)                    //pin bypass?
    {
        memcpy(buf, "\x01", 1); //True (1) if bypass required
        AddDEL(&inpDEL, tagBypassPin, buf, 1);
        memcpy(buf, "\x41", 1); //65 = Perform pin bypass
    } else
        memcpy(buf, "\x36", 1); //54 = Perform PIN Entry for online verification
    AddDEL(&inpDEL, tagAutoMate, buf, 1);

    pinDigits = getPinLen();
    memset(buf, 0, 12);
    if(pinDigits > 0) {
        AddDEL(&inpDEL, tagPinCode, buf, pinDigits);
    } else {
        AddDEL(&inpDEL, tagPinCode, buf, 12);
    }

    memcpy(buf, "\x00", 1);
    AddDEL(&inpDEL, tagResPropMeth, buf, 1);

    ret = EMVDC_STEP(EMVDC_CARDHOLDER_VERIFICATION_OTHER);
    trcFN("EMVDC_CARDHOLDER_VERIFICATION_OTHER ret=%d\n", ret);
    trcEmvDbg();
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    emvSelectStatus(&sws);
    CHECK(sws == 0x9000, lblKO);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = GetCVMDataFromKernel(qTAK, qVKA, &inpDEL, outDEL, 0);
    CHECK(ret > 0, lblKO);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Emulation of the function amgEmvCardHolderVerification.
 * This function is called after offline pin processing using cryEmvPin function to inform
 * the EMV Kernel about the result of this operation.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain
 * at the end of the execution of this function.
 * For example, it can contain tags 95 (TVR), 9B (TSI), DF38(CVMOUT Result), 9F34(CVM Result) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *  It is built from the queue qTAK given by the application as a second parameter of this function
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0017.c
 */
int emvCVMoff(tQueue * qTAK, tQueue * qVKA) {
    int ret;
    byte buf[128];
    word pinDigits;

    iniDEL(&inpDEL);

    // if we run this function, we suggest that Offline pin is oK
    memcpy(buf, "\x35", 1);
    AddDEL(&inpDEL, tagAutoMate, buf, 1);

    pinDigits = getPinLen();
    memset(buf, 0, 12);
    if(pinDigits > 0) {
        AddDEL(&inpDEL, tagPinCode, buf, pinDigits);
    } else {
        AddDEL(&inpDEL, tagPinCode, buf, 12);
    }

    memcpy(buf, "\x00", 1);
    AddDEL(&inpDEL, tagResPropMeth, buf, 1);

    ret = EMVDC_STEP(EMVDC_CARDHOLDER_VERIFICATION_OTHER);
    trcFN("EMVDC_CARDHOLDER_VERIFICATION_OTHER sta=%04X\n", ret);
    trcEmvDbg();
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    // if Pin NOK, then try again
    //CHECK(sws == 0x9000, lblKO);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = GetCVMDataFromKernel(qTAK, qVKA, &inpDEL, outDEL, 1);
    CHECK(ret > 0, lblKO);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Emulation of the function amgEmvTransactionValidation.
 * The EMV Kernel performs the terminal risk management and sets the corresponding bits in TVR and TSI.
 * Before calling this function the queue qVAK should be prepared according the qTKA got
 * from the previous EMV Kernel function call.
 * Normally it contains the tag 81 (Amount, Authorised (Binary).
 * Only the EMV Kernel can modify TVR and TSI.
 * However, there are verifications that cannot be performed by Kernel such as Opposition list
 * checking and Transaction forced online.
 * In these cases the application must perform them itself and transmit the information
 * about the results using special tags DF26 and DF1C.
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
 *   It is built from the queue qTKA obtained from the previous EMV Kernel function call.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain
 *   at the end of the execution of this function.
 *   For example, it can contain tags 95 (TVR), 9B (TSI) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \param qTKA (O) Pointer to a queue containing tags that Kernel wants to know when
 *   the next Kernel function will be called.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0018.c
 */
int emvValidate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    word sws;
    card tag;
    char val[256];
    word len;
    byte frc = 0, hotLst = 0;   //force online and hot list
    card tka[] = {
        TAG_EXCEPTION_FILE_RESULT,
        TAG_ICC_EMV_DC_OPTION,
        TAG_SKIP_TAC_IAC_DEFAULT,
        TAG_SKIP_TAC_IAC_ONLINE,
        0
    };

    queRewind(qVAK);
    while(queLen(qVAK)) {       // Find force online tag
        ret = queGetTlv(qVAK, &tag, &len, val);
        if((tag == tagFrcOnl) && (len == 1) && val[0]) {
            frc = 1;
        }
        if((tag == tagHotLst) && (len == 1) && val[0]) {
            hotLst = 1;
        }
    }

    /* TAG_ICC_EMV_DC_OPTION value on 2 bytes :
       - Byte 1:
       - bit 8:    Merchant force online choice
       - bit 7:    Merchant force decline: AAC instead of ARQC in Gen AC
       - bit 6:    Merchant force Offline approved instead of Offline declined
       - Other bits : RFU
       - Byte 2: RFU   */
    val[0] = frc ? 0x80 : 0;
    val[1] = 0;
    ret = quePutTlv(qVAK, TAG_ICC_EMV_DC_OPTION, 2, val);

    /* TAG_EXCEPTION_FILE_RESULT value on 1 byte :
       value on 1 byte :
       - 0 : ICC found in exception file
       - 1 : ICC is not found in exception file */
    val[0] = hotLst ? 0x00 : 0x01;
    ret = quePutTlv(qVAK, TAG_EXCEPTION_FILE_RESULT, 1, val);

    queRewind(qVAK);

    iniDEL(&inpDEL);
    ret = EMVDC_STEP(EMVDC_PROCESSING_RESTRICTIONS);
    trcFN("EMVDC_PROCESSING_RESTRICTIONS sta=%04X\n", ret);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_TERMINAL_RISK_MANAGEMENT);
    trcFN("EMVDC_TERMINAL_RISK_MANAGEMENT ret=%d\n", ret);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    trcS("qVAK:\n");
    trcQueTlv(qVAK);

    emvSelectStatus(&sws);
    CHECK(sws == 0x9000, lblKO);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = getDCdata(qTAK, qVKA, &inpDEL, outDEL);
    CHECK(ret > 0, lblKO);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    cvtAQ(qTKA, tka);
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Emulation of the function amgEmvActionAnalysis.
 * The EMV Kernel performs the card risk management sending GENERATE AC cryptogram to the card.
 * The cryptogram is built using CDOL1.
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
 *   It is built from the queue qTKA obtained from the previous EMV Kernel function call.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain at the end
 *   of the execution of this function. For example, it can contain tags 95 (TVR), 9B (TSI) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \param qTKA (O) Pointer to a queue containing tags that Kernel wants to know when the next
 *   Kernel function will be called.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0019.c
 */
int emvAnalyse(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    word sws;
    int error = 0;

    // tags required for the next step (for emvComplete)
    card tka[] = {
        TAG_AUTHORISATION_RESULT,
        TAG_AUTHORISATION_RESPONSE_CODE,
        TAG_CUST_IS_COMMUNICATION_OK,
        TAG_ICC_EMV_DC_OPTION,
        TAG_ISSUER_SCRIPT_TEMPLATE_1,
        TAG_ISSUER_SCRIPT_TEMPLATE_2,
        0
    };
    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_TERMINAL_ACTION_ANALYSIS);
    trcFN("EMVDC_TERMINAL_ACTION_ANALYSIS sta=%d\n", ret);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    emvSelectStatus(&sws);
    CHECK(sws == 0x9000, lblKO);

    iniDEL(&inpDEL);
    ret = EMVDC_STEP(EMVDC_CARD_ACTION_ANALYSIS);
    trcFN("EMVDC_CARD_ACTION_ANALYSIS ret=%04X\n", ret);
    trcEmvDbg();

    switch (ret) {
      case TAG_TRANSACTION_ACCEPTED:
      case TAG_TRANSACTION_DECLINED:
      case TAG_AUTHORISATION_REQUEST:
      case TAG_REFERRAL_REQUEST:
      case TAG_VISA_EASY_ENTRY_CARD:
      case TAG_CDA_FAILED:
          break;                // nothing to do, everything is OK
      case TAG_SERVICE_NOT_ALLOWED:
      case TAG_PROCESSING_ERROR:
      		error = ret;
           break;
      default:
          goto lblKO;
          break;
    }

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = getDCdata(qTAK, qVKA, &inpDEL, outDEL);
    CHECK(ret > 0, lblKO);

    if(error != 0)
		return error;

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    cvtAQ(qTKA, tka);           // Prepare tags to Application for the next tags required by Kernel
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    ret = 1;

    goto lblEnd;

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

static int addScriptCommand(DEL * mDel, byte mID) {
    int dim = 0, i;
    byte *ptr;
    byte bLen = 0;

    dim = mDel->count;
    if(dim >= MAX_DATA_ELEMENT)
        return -1;              //overflow

    for (i = 0; i < dim; i++) {
        if(mDel->list[i].tag == mID) {
            bLen = mDel->list[i].length;
            mDel->list[dim].tag = TAG_ISSUER_SCRIPT_LIST;
            ptr = &(mDel->ValBuf[dim][0]);
            ptr[0] = mID;
            ptr[1] = bLen;
            memcpy(ptr + 2, mDel->list[i].ptValue, bLen);
            mDel->list[dim].ptValue = ptr;
            mDel->list[dim].length = bLen + 2;
            mDel->count++;
        }
    }
    return 1;
}

/** Emulation of the function amgEmvTransactionCompletion.
 * This function produces some steps:
 * 1) On-Line processing
 * 2) is Issue script 1 is present then ISSUE Script 1 Performed
 * 3) Advice request?
 * 4) Completion processing
 * 5) if Issue script 2 is present then ISSUE Script 2 Performed
 * 6) EMV stop
 *
 * \param cry (I) The type of cryptogram to be generated.
 *   Only two values are acceptable 0x40 to approve and 0x00 to decline
 * \param qVAK (I) Pointer to a queue containing TLVs transmitted from Application to Kernel.
 *   It is built from the queue qTKA obtained from the previous EMV Kernel function call.
 * \param qTAK (O) Pointer to a queue containing tags that Application wants to obtain at the
 *   end of the execution of this function. For example, it can contain tags 95 (TVR), 9B (TSI) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *   It is built from the queue qTAK given by the application as a second parameter of this function
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\tlmemv.c
 * \test tcab0020.c
*/
int emvComplete(byte cry, tQueue * qVAK, tQueue * qTAK, tQueue * qVKA) {
    int ret;

    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_ON_LINE_PROCESSING);
    trcFN("EMVDC_ON_LINE_PROCESSING sta=%04X\n", ret);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    // Check for issuer script 1?
    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = addScriptCommand(&inpDEL, TAG_ISSUER_SCRIPT_TEMPLATE_1);
    CHECK(ret >= 0, lblKO);

    ret = EMVDC_STEP(EMVDC_ISSUER_TO_CARD_SCRIPT_PROCESSING1);
    trcFN("EMVDC_ISSUER_TO_CARD_SCRIPT_PROCESSING1 sta=%04X\n", ret);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret == TAG_PROCESS_COMPLETED, lblKO);

    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);
    ret = EMVDC_STEP(EMVDC_COMPLETION);
    trcFN("EMVDC_COMPLETION ret=%d\n", ret);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret >= 0, lblKO);

    switch (ret) {
      case TAG_TRANSACTION_ACCEPTED:
      case TAG_TRANSACTION_DECLINED:
      case TAG_AUTHORISATION_REQUEST:
      case TAG_REFERRAL_REQUEST:
      case TAG_VISA_EASY_ENTRY_CARD:
      case TAG_CDA_FAILED:
          break;
      default:
          goto lblKO;
    }

    // Check issuer script 2;
    ret = cvtVD(&inpDEL, qVAK); //convert TLVs from queue to DEL format
    CHECK(ret >= 0, lblKO);

    ret = addScriptCommand(&inpDEL, TAG_ISSUER_SCRIPT_TEMPLATE_2);
    CHECK(ret >= 0, lblKO);
    ret = EMVDC_STEP(EMVDC_ISSUER_TO_CARD_SCRIPT_PROCESSING2);
    CHECK(ret >= 0, lblKO);
    trcFN("emvComplete. Script 2 processing sta=%04X\n", ret);
    trcEmvDbg();
    trcS("qVAK:\n");
    trcQueTlv(qVAK);

    trcS("qTAK:\t");
    trcQueDol(qTAK);

    ret = getDCdata(qTAK, qVKA, &inpDEL, outDEL);
    CHECK(ret > 0, lblKO);

    trcS("qVKA:\n");
    trcQueTlv(qVKA);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    //ret = -1;
  lblEnd:
    return ret;
}

int emvGetAids(_DEL_ * sel, uchar id) {
    int len;
    byte *ptr;
    byte num;
    byte dim = 0;
    byte buf[512];

    len = getDELVal(&aids, TAG_AID_LIST_TERMINAL, 0, buf);
    ptr = buf;
    ini_DEL_(sel);

    //appnb = 0x01;
    add_DEL_(sel, 0x9F8D05, &id, 1);
    while(len) {
        num = *ptr;
        if(num + 1 > len)
            return -1;
        add_DEL_(sel, 0x9F8D01, ptr + 1, num);
        //add_DEL_(sel, 0x9F8D06, (byte *) bPtr(0), 1);
        add_DEL_(sel, 0x9F8D06, (byte *) bPtr(1), 1); //@@AS0.0.23 SW change here
        len -= (num + 1);
        ptr += (num + 1);
        dim++;
    }
    add_DEL_(sel, 0x9F8D00, &dim, 1);
    return dim;
}

int emvCheckCardBlocked(void) {
    int ret = 0;
    byte buf[1024];

    trcS("emvCheckCardBlocked Beg\n");
    iniDEL(&inpDEL);
    AddDEL(&inpDEL, TAG_NUMBER_OF_MATCHING_APP, NULL, 0);
    AddDEL(&inpDEL, TAG_LAST_APDU_CMD_RESPONSE, NULL, 0);

    ret = EMVDC_ServiceCall(&inpDEL, 0, SERVICE_EMVDC_GET_DATA_ELEMENTS);
    CHECK(ret >= 0, lblKO);

    ret = getDELVal(outDEL, TAG_LAST_APDU_CMD_RESPONSE, 0, buf);
    if(ret == LEN_LAST_APDU_CMD_RESPONSE) {
        if((buf[4] == 0x6A) && (buf[5] == 0x81))
            return 1;
    }
    goto lblEnd;

  lblKO:
    trcErr(ret);
    return 0;
  lblEnd:
  	return 0;
}
#endif
