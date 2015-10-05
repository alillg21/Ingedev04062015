#include <string.h>
#include "str.h"

static int getScr(word key, tInfo * dst, word len) {
    int ret;
    byte idx;
    word fld;
    word msg;

    VERIFY(dst);
    VERIFY(len == sizeof(*dst));

    memset(dst, 0, sizeof(tInfo));

    ret = mapMove(scrBeg, key);
    CHECK(ret == key,lblKO);

    ret= mapLoad(scrBeg);		//load it into the cache buffer
    CHECK(ret>=0,lblKO);

    ret = mapGetByte(scrCmd, dst->cmd);
    CHECK(ret == 1,lblKO);

    if(dst->cmd == 'i')
        goto lblEnd;            //ignore user screen

    fld = scrMsg0;
    for (idx = 0; idx < dspH; idx++) {
	 if (fld == scrDly)
	 	break;
        ret = mapGetWord(fld++, msg);
        CHECK(ret == sizeof(word),lblKO);

        ret = mapGet(msg, dst->msg[idx], dspW + 1);
        CHECK(ret <= dspW + 1,lblKO);
    }

    ret = mapGetByte(scrDly, dst->dly);
    CHECK(ret == 1,lblKO);

    goto lblEnd;
  lblKO:
  lblEnd:
    return sizeof(*dst);
}

int usrInfo(word key){
    int ret;
    tInfo inf;
    VERIFY(isSorted(infBeg,key,infEnd));
        VERIFY(key);
    
    trcS("usrInfo: Beg\n");
    ret= getScr((word)(key-1),&inf,sizeof(inf));
    CHECK(ret==sizeof(inf),lblKO);
    
    ret= info(&inf);
    CHECK(ret>=0,lblKO);

    goto lblEnd;
lblKO:
lblEnd:
    trcFN("usrInfo: ret=%d\n",ret);
    return ret;
}
