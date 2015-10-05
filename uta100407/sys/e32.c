/** \file
 * UNICAPT EMV Kernel wrappers 
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/e32.c $
 *
 * $Id: e32.c 2217 2009-03-20 14:52:31Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include "sys.h"
#include "emv.h"

#ifdef __EMV__

#undef trcOpt
#define trcOpt (TRCOPT & BIT(trcE32))
static byte FallBackAllowed;

void emvSetFallBack(byte state) {
    FallBackAllowed = state;
}

byte emvGetFallBack(void) {
    return FallBackAllowed;
}

static void emvFallBackCheck(int res) {
    switch (res) {
      case AMG_ERR_CARD_PROCESSING:
      case AMG_ERR_CARD_ANSWER:
      case AMG_ERR_CARD_DATA:
      case AMG_ERR_CARD_MUTE:
      case AMG_ERR_DUPLICATE_CARD_DATA:
      case AMG_ERR_VALUE_FORMAT:
      case AMG_ERR_MISSING_MANDATORY_CARD_DATA:
      case AMG_ERR_RECORD_NOT_FOUND:
      case AMG_ERR_CHANNEL:
      case AMG_ERR_MEMORY_SPACE:
      case AMG_ERR_HANDLE:
      case ERR_SYS_RESOURCE_PB:
          emvSetFallBack(1);
          break;
      default:
          emvSetFallBack(0);
    }
}

/** Wrapper around the function amgSelectionCriteriaAdd.
 * Using this function an application informs EMV Kernel that it can accept a number of EMV AIDs.
 * Normally this function should be used only at the POWERON event processing.
 *
 * \param que (I) A queue that contains all the EMV AIDs that are accepted by the terminal application
 * \return  
 *    - negative if failure. 
 *    - otherwise number of AIDs registered.
 * \header sys\\sys.h
 * \source sys\\e32.c
 * \test tcab0011.c
*/

int emvInit(tQueue * que) {
    int ret;
    int dim;                    //number of AIDs in the queue que
    int idx;                    //loop counter, 0..dim-1
    int ofs;                    //current offset within the lst structure below
    int len;                    //length of current AID
    char app[16 + 1];           //application name, zero-terminated
    byte aid[16];               //current AID retrieved from the input queue
    struct {                    //this structure contains the parameters for amgSelectionCriteriaAdd
        uint16 pid;             //process identifier
        uint8 app[16];          //application name, not ASCIIZ
        uint8 cpb[2];           //additional terminal capabilities
        amgAid_t tab[64];       //table of AIDs, described in amg.h
    } lst;

    VERIFY(que);
    emvSetFallBack(0);

    memset(&lst, 0, sizeof(lst));
    ofs = 0;                    //accumulates the number of bytes copied into lst

    dim = queLen(que);          //number of AIDs announced by the terminal application
    VERIFY(dim);

    //fill lst structure header before filling the table
    lst.pid = getPid();
    ofs += sizeof(lst.pid);

    ret = getAppName(app);
    VERIFY(ret >= 0);
    memset(lst.app, 0, 16);
    strcpy((char *) lst.app, app);
    ofs += sizeof(lst.app);

    memcpy(lst.cpb, "\x00\x01", 2); //only code table 1 is supported
    ofs += sizeof(lst.cpb);

    for (idx = 0; idx < dim; idx++) {   //now fill the main body of lst
        VERIFY(queNxt(que) <= 16);
        len = queGet(que, aid); //retrieve AID from the input queue
        VERIFY(len > 0);
        VERIFY(len <= 16);

        //the table record consists of AID length, AID itself, priority index and priority indicator
        //the AID and its length is retrievd from the input queue
        lst.tab[idx].aidLength = len;
        ofs += sizeof(lst.tab[idx].aidLength);

        memcpy(lst.tab[idx].aidName, aid, len);
        ofs += sizeof(lst.tab[idx].aidName);

        lst.tab[idx].aidPriorityIndex = 0;  //not used
        ofs += sizeof(lst.tab[idx].aidPriorityIndex);

        lst.tab[idx].aidAppSelIndicator = 1;    //allow partial selection
        ofs += sizeof(lst.tab[idx].aidAppSelIndicator);
    }

    //The parameters are ready. Let us perform the call
    ret = amgSelectionCriteriaAdd(T_AID, (uint16) ofs, &lst);

#ifdef __DEBUG__
    {
        byte verRfu[256];       //the first dummy parameter of EMV kernel functions below; to be ignored
        byte verSel[256];       //version of Select module
        byte verTra[256];       //version of Transaction module

        memset(verRfu, 0, sizeof(verRfu));
        memset(verSel, 0, sizeof(verSel));
        memset(verTra, 0, sizeof(verTra));

        amgEmvSelectGetVersion(verRfu, verSel);
        amgEmvTransacGetVersion(verRfu, verTra);

        //16 bytes: Company name, 16 bytes: library name; 6 bytes: Version number
        verSel[16 + 16 + 6] = 0;
        verTra[16 + 16 + 6] = 0;

        trcFN("emvInit: %d\n", ret);
        trcFS("\tverSel= %s\n", verSel);
        trcFS("\tverTra= %s\n", verTra);
        trcS("emvInit que:\n");
        trcQ(que);
    }
#endif

    CHECK(ret == RET_OK, lblKO);
    queRewind(que);
    return dim;

  lblKO:
    trcErr(ret);
    return -1;
}

/** Wrapper around the function amgInitPayment.
 * This function should be called before starting EMV processing.
 * \return  
 *    - negative if failure 
 *    - otherwise 1
 * \header sys\\sys.h
 * \source sys\\e32.c
 * \test tcab0012.c
*/
int emvStart(void) {
    int16 ret;
    amgMsg_t msg;

    msg.msgType = MSG_SMART_CARD;
    msg.msgLength = 0;
    msg.receiverPid = 0xFFFF;
    msg.callerTaskId = 0xFFFF;

    ret = amgInitPayment("DEFAULT", &msg);
    emvFallBackCheck(ret);
    trcFN("emvStart ret=%d\n", ret);
    CHECK(ret == RET_OK, lblKO);

    return 1;

  lblKO:
    trcErr(ret);
    return -1;
}

/** Wrapper around the function amgEndPayment and amgEmvCloseContext.
 * This function should be called after finishing EMV processing and in case of exceptions.
 * \return no
 * \header sys\\sys.h
 * \source sys\\e32.c 
 * \test tcab0012.c
*/
void emvStop(void) {
    amgEndPayment();
    amgEmvCloseContext();
    trcS("emvStop\n");
}

#define mask8 ((byte)0x80)
#define mask6 ((byte)0x20)
#define mask87 ((byte)0xC0)
#define mask54321 ((byte)0x1F)
#define mask7654321 ((byte)0x7F)

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

#ifdef __DEBUG__
static int trcEmvDbgReq(const byte * dbg) { //trace request APDUs
    char *cmd;
    int len;

    //extract debug info portion length
    len = dbg[0];
    len *= 256;
    len += dbg[1];
    dbg += 2;

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
}

static int trcEmvDbgRsp(const byte * dbg) { //trace reponse APDU
    int ret, dim, idx, totalInc;
    byte tag[2];                //EMV tag
    card len;                   //EMV tag length
    byte val[1024];             //EMV tag value
    char tmp[1024];

    //extract debug info portion length
    dim = dbg[0];
    dim *= 256;
    dim += dbg[1];
    dbg += 2;
    idx = dim - 2;
    totalInc = 0;

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

static int trcEmvDbgCrt(const byte * dbg) { //trace certificate values after authentication
    int len;                    //the debug info portion size
    int ofs;                    //the current offset within the portion
    byte crt;                   //certificate type, see the doc
    char tmp[80];

    ofs = 0;
    crt = *dbg++;               //certificate type
    ofs++;

    //retrieve the debug info portion length from the buffer
    len = dbg[0];
    len *= 256;
    len += dbg[1];
    dbg += 2;
    ofs += len + 2;

    sprintf(tmp, "  Certificate %02X: ", (word) crt);
    trcS(tmp);
    trcBN(dbg, len);
    trcS("\n");
    return ofs;
}

static int trcEmvDbgErr(const byte * dbg) { //trace error info
    trcS("  Error:");
    trcBN(dbg, 3);              //error info consists of source file code and line number
    trcS("\n");
    return 3;
}

static void trcEmvDbg(const byte * dbg) {   //trace debug info buffer after calling a kernel function
    int len;

    trcS("EMV DBG:\n");
    len = 0;
    while(len >= 0) {
        switch (*dbg++) {       //parse according to the information type byte
          case 0x80:           //ICC command
              len = trcEmvDbgReq(dbg);
              break;

          case 0x40:           //ICC answer
              len = trcEmvDbgRsp(dbg);
              break;

          case 0x20:           //ICC certificate
              len = trcEmvDbgCrt(dbg);
              break;

          case 0x01:           //Internal errors
              len = trcEmvDbgErr(dbg);
              break;

          case 0:              //finished, quit the loop
              len = -1;
              break;

          default:             //unexpected value
              trcS("  Unknown format: ");
              trcBN(dbg, 16);
              trcS("\n");
              len = -1;
              break;
        }
        if(len >= 0)
            dbg += len;         //move to the next debug info
    }
}
#else
#define trcEmvDbg(dbg)
#endif

#ifdef __DEBUG__
#define L_DBG 2048
#else
#define L_DBG 0
#endif

/** Wrapper around the function amgEmvSelectStatus.
 *  Return the status of EMV application selection after emvSelect() 
 * if emvSelect is returning the value less than or equal 0
 * \param pbm (O) returning SW           
 * \header sys\\sys.h
 * \source sys\\e32.c
 * \test tcnl0029.c
 */
void emvSelectStatus(word * pbm) {
    int idx;
    word sw = 0;
    amgEmvSelectionDebug_t sta;

    VERIFY(pbm);

    *pbm = 0;
    amgEmvSelectStatus(&sta);
    idx = sta.nbSelect;
    while(idx--) {
        sw = sta.responseSW1SW2[idx].selectSW1SW2;
        if((sw == 0x6283) || (sw == 0x6A81)) {
            *pbm = sw;
            return;
        }
    }
    if(sw == 0x6A82) {          //no candidate in the list
        *pbm = 0xFFFF;
    }
}

/** Wrapper around the function amgAskSelection.
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
 * \source sys\\e32.c
 * \test tcab0012.c
*/
int emvSelect(tQueue * que) {
    int ret;
    byte dim;                   //the number of AIDs
    byte idx;                   //loop counter, 0..dim-1
    byte len;                   //length of AID or of application name
    byte dbg[L_DBG + 1];        //buffer to accept the debug information of the kernel
    amgSelectionResult_t rsp;   //structure containing amgAskSelection response

    VERIFY(que);
    memset(dbg, 0, L_DBG + 1);
    memset(&rsp, 0, sizeof(rsp));
    queReset(que);

    ret = amgAskSelection(&rsp, getPid(), L_DBG, dbg);  //perform the call
    emvFallBackCheck(ret);
    trcFN("emvSelect ret=%d\n", ret);
    trcEmvDbg(dbg);
    CHECK(ret == RET_OK, lblKO);

    dim = (byte) rsp.dataList.emv2List.nbAid;   //number of AIDs in the candidate list
    VERIFY(dim <= 64);
    VERIFY(rsp.selectionType == AMG_EMV2_SELECTION);

    for (idx = 0; idx < dim; idx++) {   //retrieve all the candidates fro mthe list and put the data into the queue
        len = rsp.dataList.emv2List.tbAid[idx].aidLength;
        VERIFY(len <= 16);

        ret = quePut(que, rsp.dataList.emv2List.tbAid[idx].aidName, len);   //EMV AID
        CHECK(ret >= 0, lblKO);

        len = rsp.dataList.emv2List.tbAid[idx].appNameLength;
        VERIFY(len <= 16);

        ret = quePut(que, rsp.dataList.emv2List.tbAid[idx].appName, len);   //Application (preferred) name
        CHECK(ret >= 0, lblKO);

        ret = quePut(que, &rsp.dataList.emv2List.tbAid[idx].priorityIndic, 1);  //priority indicator
        CHECK(ret >= 0, lblKO);
    }
    trcS("que:\n");
    trcQ(que);
    return dim;

  lblKO:
    trcErr(ret);
    switch (ret) {
      case AMG_ERR_CARD_BLOCKED:   // Hassan - Added for Req10 
          ret = -eEmvErrCardBlock;
          break;
      default:
          ret = -1;
    }
    return ret;
}

static byte *fsBuf = 0;

/** Set Final Select buffer pointer
 * It is not a wrapper. It is an artefact of EMV Kernel architecture.
 * A piece of memory of size __FSBUFLEN__  bytes will be used to transmit an information from Selection
 * to Transaction module of EMV Kernel.
 * It contains the response to the last APPLICATION SELECT command.
 * The function should be called just before emvFinalSelect function.
 * The memory will be used during emvContext function call, and it will not be used afterwards.
 * \param buf (I) The pointer to a memory buffer of size >= __FSBUFLEN__ bytes
 *   to accept the result of the last response to APPLICATION SELECT command.
 *   These data are not to be treated inside the application.
 *   It is to be transmitted from EMV Kernel Select module to transaction module.
 * \return no
 * \header sys\\sys.h
 * \source sys\\e32.c
 * \test tcab0013.c 
*/
void emvSetFSbuf(byte * buf) {
    VERIFY(buf);
    fsBuf = buf;
}

/** 
 * \todo To be documented
*/
int emvSetSelectResponse(byte * aid, void *buf) {
    int ret, idx;
    byte tag[2];
    card len;
    byte val[256];
    byte tmp[__FSBUFLEN__];
    byte *ptr;

    VERIFY(aid);
    VERIFY(buf);

    memset(aid, 0, 1 + 16);

    //look for tag 6F
    ptr = (byte *) buf;
    for (idx = 0; idx < 32; idx++)
        if(ptr[idx] == 0x6F)
            break;
    CHECK(idx < 32, lblKO);
    CHECK(idx >= 4, lblKO);

    idx -= 4;
    ptr += idx;
    idx = ptr[2];
    if(!idx)
        idx = ptr[3];
    memcpy(tmp, buf, idx + 4);
    ptr = tmp + 4;
    while(idx > 0) {
        memset(tag, 0, 2);
        ret = emvGetTLV(tag, &len, val, ptr);
        if(ret < 0)
            break;
        if(!emvIsConstructed(tag)) {
            if(*tag == 0x84)
                break;
            ptr += ret;
            idx -= ret;
            continue;
        }
        ret = emvGetT(tag, tmp);
        ptr += ret;
        idx -= ret;
        ret = emvGetL(&len, tmp);
        if(ret < 0)
            break;
        ptr += ret;
        idx -= ret;
    }
    CHECK(idx > 0, lblKO);
    CHECK(*tag == 0x84, lblKO);
    CHECK(len <= 16, lblKO);
    *aid = (byte) len;
    memcpy(aid + 1, val, *aid);
    memcpy(buf, tmp, 260);
    return 0;
  lblKO:return -1;
}

/** Wrapper around the function amgEmvFinalSelect.
 * Perform an explicite final APPLICATION SELECT command to before starting the EMV transaction flow.
 * It is the last operation of application selection procedure.
 * The candidate is selected before from the list using a menu or application selection algorithm.
 * \param aid (I) The EMV AID to be selected.
 *   The first byte is the length of AID, it is followed by AID itself.
 *   For example, to select a VSDC application A0 00 00 00 03 10 10 00
 *   the value "\x07\xA0\x00\x00\x00\x03\x10\x10" should be given as an input of this function.
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source : sys\\e32.c
 * \test tcab0013.c 
*/
int emvFinalSelect(const byte * aid) {
    int ret;
    amgSelectionResult_t res;   //structure containing the input information
    byte dbg[L_DBG + 1];        //buffer to accept the debug information of the kernel

    VERIFY(aid);
    VERIFY(fsBuf);
    memset(dbg, 0, L_DBG + 1);
    memset(&res, 0, sizeof(res));

    //prepare parameters for the amgEmvFinalSelect call
    res.selectionType = AMG_EMV2_SELECTION;
    res.dataList.emv2List.nbAid = 1;
    res.dataList.emv2List.tbAid[0].aidLength = aid[0];  //the first byte is the AID length
    VERIFY(aid[0] <= sizeof(res.dataList.emv2List.tbAid[0].aidName));
    memcpy(res.dataList.emv2List.tbAid[0].aidName, aid + 1, *aid);  //AID itself follows
    res.dataList.emv2List.tbAid[0].termPriority[0] = 1;
    res.dataList.emv2List.tbAid[0].termAppliCandidate[0] = getPid();

    ret = amgEmvFinalSelect("DEFAULT", &res.dataList.emv2List.tbAid[0], &res, fsBuf, L_DBG, dbg);   //call it
    emvFallBackCheck(ret);
    trcFN("emvFinalSelect ret=%d\n", ret);
    trcS("aid=");
    trcBN(aid, *aid + 1);
    trcS("\n");
    trcS("fsBuf=");
    trcBN(fsBuf, 100);
    trcS("\n");
    trcEmvDbg(dbg);
    CHECK(ret == RET_OK, lblKO);

    return 1;
  lblKO:trcErr(ret);
    switch (ret) {
      case AMG_ERR_RESELECT:   // Hassan - Added for Req10 
          ret = -eEmvErrReselect;
          break;
      default:
          ret = -1;
    }
    return ret;
}

/** Wrapper around the function amgEmvGetLanguagePreference.
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
 * \source sys\\e32.c
 * \test tcab0013.c 
*/
int emvGetLang(char *lan) {
    int len;

    VERIFY(lan);
    memset(lan, 0, 9);
    amgEmvGetLanguagePreference((byte *) lan, 9);
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
    return -1;
}

static int emvPutT(tBuffer * buf, card tag) {   //put EMV tag into the buffer in BER-TLV format
    byte idx, len;
    byte tmp[4];

    VERIFY(buf);

    idx = len = 0;
    memset(tmp, 0, 4);

    //calculate the BER-TLV length of tag
    if(tag > 0x000000FF)
        idx++;
    if(tag > 0x0000FFFF)
        idx++;
    if(tag > 0x00FFFFFF)
        idx++;

    while(tag) {
        tmp[idx--] = (byte) (tag & 0xFF);
        tag >>= 8;
        len++;
        VERIFY(len <= 2);       //only 1- or 2- byte lengths are supported
    }
    return bufApp(buf, tmp, len);   //append it to the end of the buffer
}

static int emvPutL(tBuffer * buf, word len) {   //put EMV length into the buffer in BER-TLV format
    byte tmp[2];
    byte dim;

    VERIFY(buf);
    VERIFY(len < 256);          //big length are not supported

    memset(tmp, 0, 2);
    if(len < 128) {             //it is easy to process small lengths
        *tmp = (byte) len;
        dim = 1;
    } else {                    //and a bit more complicated with 2-byte lengths
        tmp[0] = 0x81;
        tmp[1] = (byte) len;
        dim = 2;
    }
    return bufApp(buf, tmp, dim);   //append it to the end of the buffer
}

//Define the length of the buffer to be used between Kernel and application
#define L_BUF 1024

/** Convert an input queue of (Tag,Value) pairs into BER-TLV format
 * \param que (I) Input queue containing intermittent T,V,T,V,... pairs
 *   Each tag is a card number.
 *   The values are arbitrary binary data, the first byte being the length of the value.
 * \param dat (O) The TLV triples according to BER-TLV encoding rules
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\e32.c
*/
int cvtQV(tQueue * que, byte * dat) {
    int ret;
    card tag;
    word len;
    byte val[512];
    tBuffer buf;

    VERIFY(que);
    VERIFY(dat);

    ret = 0;
    bufInit(&buf, dat + 2, L_BUF - 2);  //the first two bytes will contain the length
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

        //ignore telium tags  
        switch (tag) {
          case tagODA:
          case tagDetFaitPriTAA:
          case tagCDANevReqARQ1:
          case tagCDANevReqONL2:
          case tagAuthRes:
          case tagCommOk:
              break;
          default:
              ret = emvPutT(&buf, tag); //save tag  
              CHECK(ret > 0, lblKO);

              ret = emvPutL(&buf, len); //save length  
              CHECK(ret > 0, lblKO);

              ret = bufApp(&buf, val, len); //append value to the end of the buffer  
              CHECK(ret > 0, lblKO);
              break;
        }

    }
    ret = bufLen(&buf);         //the length should be written at the beginning of the buffer
    ret = num2bin(dat, ret, 2);
    VERIFY(ret == 2);
    return ret;
  lblKO:
    trcErr(ret);
    return -1;
}

static card cvtTC(byte * tag) { //convert BER-TLV tag into card format
    card ret;

    ret = *tag++;
    if(!*tag)
        return ret;

    ret *= 256;
    ret += *tag++;
    if(!*tag)
        return ret;

    ret *= 256;
    ret += *tag++;
    if(!*tag)
        return ret;

    ret *= 256;
    ret += *tag++;
    if(!*tag)
        return ret;

    return ret;
}

//convert a queue of tags into BER-TLV format acceptable by EMV Kernel
//The input queue contains tags to be saved into the buffer
static int cvtQT(tQueue * que, byte * dat) {
    int ret;
    card tag;
    tBuffer buf;

    VERIFY(que);
    VERIFY(dat);

    ret = 0;
    bufInit(&buf, dat + 2, L_BUF - 2);  //the first two bytes will contain the length
    while(queLen(que)) {
        CHECK(queNxt(que) == sizeof(card), lblKO);
        ret = queGet(que, (byte *) & tag);  //retrieve tag
        VERIFY(ret == sizeof(card));

        ret = emvPutT(&buf, tag);   //save tag
        CHECK(ret > 0, lblKO);
    }
    ret = bufLen(&buf);         //the length should be written at the beginning of the buffer
    ret = num2bin(dat, ret, 2);
    VERIFY(ret == 2);
    return ret;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Convert BER-TLV encoded data into a queue containing tags and values
 * \param que (O) Output queue containing intermittent T,V,T,V,... pairs
 *   Each tag is a card number.
 *   The values are arbitrary binary data, the first byte being the length of the value.
 * \param dat (I) The TLV triples according to BER-TLV encoding rules
 * \return non-negative value if OK; negative otherwise
 * \header sys\\sys.h
 * \source sys\\e32.c
*/
int cvtVQ(tQueue * que, byte * dat) {
    int ret;
    card dim;                   //the length of the input buffer
    card tag;                   //T part of TLV
    card len;                   //L part of TLV
    byte val[512];              //V part of TLV
    byte tmp[4];                //tag in BER-TLV format

    VERIFY(que);
    VERIFY(dat);

    queReset(que);
    ret = bin2num(&dim, dat, 2);    //retrieve the length of the data in the buffer
    VERIFY(ret == 2);
    VERIFY(dim < 0x10000);
    dat += 2;                   //goto the start of the data

    while(dim) {
        memset(tmp, 0, 4);
        memset(val, 0, 512);
        len = 0;

        ret = emvGetTLV(tmp, &len, val, dat);   //retrieve TLV from input buffer
        CHECK(ret > 0, lblKO);
        CHECK(len <= 0xFFFF, lblKO);
        dim -= ret;             //ret bytes processed
        dat += ret;             //skip the info processed

        tag = cvtTC(tmp);       //convert from BER-TLV into card        
        ret = quePut(que, (byte *) & tag, sizeof(card));    //save tag into queue
        CHECK(ret == sizeof(card), lblKO);

        ret = quePut(que, val, (word) len); //save data into queue
        CHECK(ret > 0, lblKO);
    }
    return queLen(que);
  lblKO:
    trcErr(ret);
    return -1;
}

//convert BER-TLV encoded tags from EMV Kernel into que
static int cvtTQ(tQueue * que, byte * dat) {
    int ret;
    card dim;
    card tag;
    byte tmp[4];

    VERIFY(que);
    VERIFY(dat);

    queReset(que);
    ret = bin2num(&dim, dat, 2);    //retrieve the length of the data in the buffer
    VERIFY(ret == 2);
    VERIFY(dim < 0x10000);
    dat += 2;                   //goto the start of the data

    while(dim) {
        memset(tmp, 0, 4);
        ret = emvGetT(tmp, dat);
        CHECK(ret > 0, lblKO)

            dim -= ret;         //ret bytes processed
        dat += ret;             //skip the info processed

        tag = cvtTC(tmp);       //convert from BER-TLV into card        
        ret = quePut(que, (byte *) & tag, sizeof(card));    //save tag into queue
        CHECK(ret == sizeof(card), lblKO)
    }
    return queLen(que);
  lblKO:
    trcErr(ret);
    return -1;
}

static int fsPatch(void) {      //this function patches the differences in the alignment for various compiler versions
    byte i, dim, j = 0;

    VERIFY(fsBuf);

    dim = 0;
    for (i = 0; i < 4; i++) {   //find length byte
        if(fsBuf[i] == 0)
            continue;
        if(fsBuf[i] == 0x90) {  //skip 90 if it is the first one
            if(!j++)
                continue;
        }
        dim = fsBuf[i];
        break;
    }
    if(!dim)
        return -1;
    fsBuf[2] = 0;
    fsBuf[3] = dim;
    return i;
}

/*
static void dump(const byte *buf, word len){
	char tmp[prtW+1];
	word idx= 0;
	while(len--){
		sprintf(tmp+3*idx++," %02X",*buf++);
		if(idx<8) continue;
	  prtS(tmp);
	  idx= 0;		
  }
  if(idx)
  	prtS(tmp);
}
*/

/** Wrapper around the function amgEmvInitContext.
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
 * \source sys\\e32.c
 * \test tcab0014.c 
*/
int emvContext(tQueue * qVAK, tQueue * qTKA) {
    int ret;
    byte tka[L_BUF];
    byte dApp[L_BUF];           //application data
    byte dTrn[2];               //transaction data, obsolete

    memset(dApp, 0, L_BUF);
    memset(dTrn, 0, 2);

    //prtS("emvContext");
    ret = fsPatch();            //return the index where the length is found
    CHECK(ret >= 0, lblKO);
    CHECK(ret < 4, lblKO);

    ret = cvtQV(qVAK, dApp);    //prepare input Application Data
    CHECK(ret >= 0, lblKO);

//dump(fsBuf,128);
    ret = amgEmvInitContext((byte *) "DEFAULT", //SMCChannel
                            __OWNTAGPRE__,  //OwnTagPrefix
                            fsBuf + 2,  //CardAnswer
                            dApp,   //ApplicationData
                            dTrn,   //TransactionData
                            1,  //NbProprietaryTags
                            (uint8 *) "\x9F\x53\x00\x01\x00\x01\x40",   //ProprietaryTags: tag 9F 53, minlen 00 01, maxlen 00 01, mask 40 (binary from terminal)
                            L_BUF,  //Size_DObjListEMV  
                            tka //DObjListEMV
        );
    //{char tmp[64]; sprintf(tmp,"ret=%d %d",ret,AMG_ERR_CARD_ANSWER); prtS(tmp);}
    emvFallBackCheck(ret);
    trcFN("emvContext ret=%d\n", ret);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtTQ(qTKA, tka);     //convert output data: tags requested by Kernel
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

//add to the beginning of the queue the tag DF42 containing the address of debug buffer
static int emvAskDebugInfo(tQueue * que, byte * dbg) {
#ifdef __DEBUG__
    int ret;
    byte idx;
    int32 adr;
    byte val[7];

    adr = tagDebugInfo;
    ret = quePut(que, (byte *) & adr, sizeof(card));
    if(ret < 0)
        return -1;

    idx = 0;
    val[idx++] = 0xFF;          //all bits enabled in the mask
    num2bin(val + idx, L_DBG, 2);   //buffer length
    idx += 2;
    adr = (uint32) dbg;
    num2bin(val + idx, adr, 4); //address of debug buffer

    ret = quePut(que, val, 7);  //put tag value into the queue
    VERIFY(ret >= 0);
#endif
    return queLen(que);
}

/** Wrapper around the function amgEmvTransactionPreparation.
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
 * \source sys\\e32.c
 * \test tcab0015.c 
*/
int emvPrepare(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    byte dVAK[L_BUF];
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];
    byte dbg[L_DBG + 1];

    memset(dbg, 0, L_DBG + 1);
    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);

    ret = emvAskDebugInfo(qVAK, dbg);   //put debug tag into the queue
    CHECK(ret >= 0, lblKO);

    ret = cvtQV(qVAK, dVAK);    //convert TLVs from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = amgEmvTransactionPreparation(dTAK, dVAK, L_BUF, dVKA, L_BUF, dTKA);
    emvFallBackCheck(ret);
    trcFN("emvPrepare ret=%d\n", ret);
    trcEmvDbg(dbg);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    ret = cvtTQ(qTKA, dTKA);    //convert tags into the queue
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);

    switch (ret) {
      case AMG_ERR_RESELECT:
          ret = -eEmvErrReselect;
          break;
      case AMG_ERR_CARD_ANSWER:
      case AMG_ERR_CARD_MUTE:
      case AMG_ERR_HANDLE:
          ret = -eEmvErrFallback;
          break;
      default:
          ret = -1;
    }
    return ret;
}

/** Wrapper around the function amgEmvDataAuthentication.
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
 * \source sys\\e32.c
 * \test tcab0016.c 
*/
int emvAuthenticate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    byte dVAK[L_BUF];
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];
    byte dbg[L_DBG + 1];

    memset(dbg, 0, L_DBG + 1);
    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);

    ret = emvAskDebugInfo(qVAK, dbg);   //put debug tag into the queue
    CHECK(ret >= 0, lblKO);

    ret = cvtQV(qVAK, dVAK);    //convert TLVs from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = amgEmvDataAuthentication(dTAK, dVAK, L_BUF, dVKA, L_BUF, dTKA);
    emvFallBackCheck(ret);
    trcFN("emvAuthenticate ret=%d\n", ret);
    trcEmvDbg(dbg);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    ret = cvtTQ(qTKA, dTKA);    //convert tags into the queue
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** One of the wrappers around the function amgEmvCardHolderVerification
 * with the first parameter CV_INITIAL.
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
 * \source sys\\e32.c
 * \test tcab0017.c 
*/
int emvCVMstart(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA) {
    int ret;
    byte dVAK[L_BUF];
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];
    byte dbg[L_DBG + 1];

    memset(dbg, 0, L_DBG + 1);
    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);

    ret = emvAskDebugInfo(qVAK, dbg);   //put debug tag into the queue
    CHECK(ret >= 0, lblKO);

    ret = cvtQV(qVAK, dVAK);    //convert TLVs from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret =
        amgEmvCardHolderVerification(CV_INITIAL, dTAK, dVAK, L_BUF, dVKA, L_BUF,
                                     dTKA);
    emvFallBackCheck(ret);
    trcFN("emvCVMstart ret=%d\n", ret);
    trcEmvDbg(dbg);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    VERIFY(dTKA[0] == 0);
    VERIFY(dTKA[1] == 0);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** One of the wrappers around the function amgEmvCardHolderVerification.
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
 * \source sys\\e32.c
*/
int emvCVMnext(int opt, tQueue * qTAK, tQueue * qVKA) {
    int ret;
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];
    amgEMVCVEntryOption_t ent;

    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    switch (opt) {
      case 1:
          ent = CV_OK;
          break;
      case -1:
          ent = CV_KO;
          break;
      default:
          VERIFY(opt == 0);
          ent = CV_BYPASSED;
          break;
    }
    ret = amgEmvCardHolderVerification(ent, dTAK, 0, L_BUF, dVKA, L_BUF, dTKA);

    trcFN("emvCVMnext ret=%d\n", ret);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    VERIFY(dTKA[0] == 0);
    VERIFY(dTKA[1] == 0);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** One of the wrappers around the function amgEmvCardHolderVerification.
 * This function is called after offline pin processing using cryEmvPin function to inform
 * the EMV Kernel about the result of this operation.
 * \param qTAK (I) Pointer to a queue containing tags that Application wants to obtain
 * at the end of the execution of this function.
 * For example, it can contain tags 95 (TVR), 9B (TSI), DF38(CVMOUT Result), 9F34(CVM Result) etc
 * \param qVKA (O) Pointer to a queue containing TLVs transmitted from Kernel to Application.
 *  It is built from the queue qTAK given by the application as a second parameter of this function
 * \return 1 if OK, negative if failure.
 * \header sys\\sys.h
 * \source sys\\e32.c
 * \test tcab0017.c 
*/
int emvCVMoff(tQueue * qTAK, tQueue * qVKA) {
    int ret;
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];

    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret =
        amgEmvCardHolderVerification(CV_OFFLINEPIN, dTAK, 0, L_BUF, dVKA, L_BUF,
                                     dTKA);

    trcFN("emvCVMoff ret=%d\n", ret);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    VERIFY(dTKA[0] == 0);
    VERIFY(dTKA[1] == 0);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Wrapper around the function amgEmvTransactionValidation.
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
 * \source sys\\e32.c
 * \test tcab0018.c 
*/
int emvValidate(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    byte dVAK[L_BUF];
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];
    byte dbg[L_DBG + 1];
    byte lti[3];                //Last Transaction Information

    memset(dbg, 0, L_DBG + 1);
    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);
    memset(lti, 0, 3);          //this parameter is not used in the wrapper

    ret = emvAskDebugInfo(qVAK, dbg);   //put debug tag into the queue
    CHECK(ret >= 0, lblKO);

    ret = cvtQV(qVAK, dVAK);    //convert TLVs from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret =
        amgEmvTransactionValidation(lti, dTAK, dVAK, L_BUF, dVKA, L_BUF, dTKA);
    emvFallBackCheck(ret);
    trcFN("emvValidate ret=%d\n", ret);
    trcEmvDbg(dbg);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    ret = cvtTQ(qTKA, dTKA);    //convert tags into the queue
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/** Wrapper around the function amgEmvActionAnalysis.
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
 * \source sys\\e32.c
 * \test tcab0019.c 
*/
int emvAnalyse(tQueue * qVAK, tQueue * qTAK, tQueue * qVKA, tQueue * qTKA) {
    int ret;
    byte dVAK[L_BUF];
    byte dTAK[L_BUF];
    byte dVKA[L_BUF];
    byte dTKA[L_BUF];
    byte dbg[L_DBG + 1];

    memset(dbg, 0, L_DBG + 1);
    memset(dVKA, 0, L_BUF);
    memset(dTKA, 0, L_BUF);

    ret = emvAskDebugInfo(qVAK, dbg);   //put debug tag into the queue
    CHECK(ret >= 0, lblKO);

    ret = cvtQV(qVAK, dVAK);    //convert TLVs from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = amgEmvActionAnalysis(dTAK, dVAK, L_BUF, dVKA, L_BUF, dTKA);
    emvFallBackCheck(ret);
    trcFN("emvAnalyse ret=%d\n", ret);
    trcEmvDbg(dbg);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    trcS("qTAK:\t");
    trcQueDol(qTAK);
    CHECK(ret == RET_OK, lblKO);

    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    ret = cvtTQ(qTKA, dTKA);    //convert tags into the queue
    trcS("qTKA:\t");
    trcQueDol(qTKA);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    switch (ret) {
      case AMG_ERR_CARD_ANSWER:    // Hassan - Added for Req10 
          ret = -eEmvErrFallback;
          break;
      default:
          ret = -1;
    }
    return ret;
}

/** Wrapper around the function amgEmvTransactionCompletion.
 * The EMV Kernel complete the transaction.
 * For offline transactions it is not needed.
 * If it is online transaction, the authorisation request should be performed just before this step.
 * For successful online transaction EXTERNAL AUTHENTICATE is performed.
 * Anyway, a second cryptogram is generated.
 * If there are scripts to perform they are sent to the card during this call.
 * There is not qTKA parameter: it is the last operation for an EMV transaction flow.
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
 * \source sys\\e32.c
 * \test tcab0020.c 
*/
int emvComplete(byte cry, tQueue * qVAK, tQueue * qTAK, tQueue * qVKA) {
    int ret, sav;
    byte dVAK[L_BUF] = "";
    byte dTAK[L_BUF] = "";
    byte dVKA[L_BUF] = "";
    byte dbg[L_DBG + 1] = "";
    byte lti[L_BUF] = "";       //Last Transaction Information

    memset(dbg, 0, L_DBG + 1);
    memset(dVKA, 0, L_BUF);
    memset(lti, 0, 3);          //this parameter is not used in the wrapper

    ret = emvAskDebugInfo(qVAK, dbg);   //put debug tag into the queue
    CHECK(ret >= 0, lblKO);

    ret = cvtQV(qVAK, dVAK);    //convert TLVs from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    ret = cvtQT(qTAK, dTAK);    //convert tags from queue to BER-TLV format
    CHECK(ret >= 0, lblKO);

    //ret= amgEmvTransactionCompletion(cry,lti,dTAK,dVAK,L_BUF,dVKA);
    ret = amgEmvTransactionCompletion(cry, dVAK, dTAK, lti, L_BUF, dVKA);
    emvFallBackCheck(ret);
    trcFN("emvComplete ret=%d\t", ret);
    trcFN("cry=%02X\n", cry);
    trcEmvDbg(dbg);
    trcS("qVAK:\n");
    trcQueTlv(qVAK);
    trcS("qTAK:\t");
    trcQueDol(qTAK);

    sav = ret;
    ret = cvtVQ(qVKA, dVKA);    //convert TLVs into the queue
    trcS("qVKA:\n");
    trcQueTlv(qVKA);
    CHECK(ret >= 0, lblKO);

    CHECK(sav == RET_OK, lblKO);    //check after setting the VK
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}
#endif
