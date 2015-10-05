#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcab0114.
//Functions testing:
// xmlGetTag
// xmlGetAtr

static void handleAtr(tBuffer atr, tBuffer val, int depth) {
    int i;

    if(!atr.pos || !val.pos)
        return;

    for (i = 0; i <= depth; i++)
        trcS("   ");

    trcFS("atr: %10s    ", (char *) bufPtr(&atr));
    trcFS("val: %10s\n", (char *) bufPtr(&val));
}

static void handleTag(tBuffer tag, int depth) {
    int i;

    if(tag.pos == 0)
        return;
    trcS("\n");
    for (i = 0; i < depth; i++)
        trcS("   ");
    trcFS("tag: %10s\n", (char *) bufPtr(&tag));
}

static void handleData(tBuffer val, int depth) {
    int i;

    if(val.pos == 0)
        return;
    for (i = 0; i < depth; i++)
        trcS("   ");
    trcFS("DATA: %s\n", (char *) bufPtr(&val));
}

static void xmlParse(const tBuffer * xml, int depth) {
    const byte *ptr;
    int ret;
    char tagBuf[100];
    char atrBuf[1000];
    char valBuf[10000];

    tBuffer tag;
    tBuffer atr;
    tBuffer val;

    bufInit(&tag, (byte *) tagBuf, sizeof(tagBuf));
    bufInit(&atr, (byte *) atrBuf, sizeof(atrBuf));
    bufInit(&val, (byte *) valBuf, sizeof(valBuf));

    for (ptr = bufPtr(xml);
         (ret = xmlGetTag(&tag, &atr, &val, (char *) ptr)) > 0; ptr += ret) {
        const byte *ptrAtr;
        int retAtr;
        char _atrBuf[64];
        char _valBuf[64];
        tBuffer _atr;
        tBuffer _val;

        bufInit(&_atr, (byte *) _atrBuf, sizeof(_atrBuf));
        bufInit(&_val, (byte *) _valBuf, sizeof(_valBuf));

        handleTag(tag, depth);

        if(atr.pos) {
            for (ptrAtr = bufPtr(&atr);
                 (retAtr = xmlGetAtr(&_atr, &_val, (char *) ptrAtr)) > 0;
                 ptrAtr += retAtr) {
                handleAtr(_atr, _val, depth);
            }
        }

        if(val.pos && tag.pos)
            xmlParse(&val, depth + 1);
    }
    handleData(val, depth);
}

static void prepare01(tBuffer * xml) {
    VERIFY(xml);
    bufAppStr(xml, "<?xml?>\n");
    bufAppStr(xml, "<ccfReq chn=\"1\" type=\"send\">\n");
    bufAppStr(xml, " <ccf:data fmt=\"asc\">How are you?</ccf:data>\n");
    bufAppStr(xml, "</ccfReq>\n");
}

void tcab0114(void) {
    int ret;
    char dat[50000];
    tBuffer xml;

    bufInit(&xml, (byte *) dat, sizeof(dat));

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "01...");
    CHECK(ret >= 0, lblKO);
    prepare01(&xml);
    trcS("XML input:\n");
    trcS("---\n");
    trcS((char *) bufPtr(&xml));
    trcS("---\n");
    xmlParse(&xml, 0);
    ret = dspLS(0, "01 OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
