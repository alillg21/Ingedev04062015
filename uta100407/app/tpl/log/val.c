#include "string.h"
#include "log.h"

int valOpr(void){
    int ret;
    word MnuItm;
    char buf[lenMnu+1];
    word key;
    char Pwd1[lenPwd+1];
    char Pwd2[lenPwd+1];

    ret= mapGetWord(traMnuItm,MnuItm); CHECK(ret>=0,lblKO);
    ret= mapGet(MnuItm,buf,lenMnu+1); CHECK(ret>=0,lblKO);
    ret= 1;
    switch(buf[1]){ //access type
        case 'm': key= appMrcPwd; break;
        case 'a': key= appAdmPwd; break;
        default: goto lblEnd;
    }
    
    ret= dspClear(); CHECK(ret>=0,lblKO);
    memset(Pwd1,0,lenPwd+1);
    memset(Pwd2,0,lenPwd+1);

    ret= usrInfo(infPwdInput); CHECK(ret>=0,lblKO);    
    ret= enterPwd(2,Pwd1,lenPwd+1); CHECK(ret>=0,lblKO);
    CHECK(ret==kbdVAL,lblAbort);
    ret= mapGet(key,Pwd2,lenPwd+1); CHECK(ret>=0,lblKO);
    CHECK(strcmp(Pwd1,Pwd2)==0,lblBadPwd);

    ret= 1;
    goto lblEnd;
lblBadPwd:
    trcErr(ret);    
    usrInfo(infWrongPin);
    ret= 0;
    goto lblEnd;
lblKO:
    trcErr(ret);    
    trcErr(ret);    
    usrInfo(infProcessingError);
    ret= 0;
    goto lblEnd;
lblAbort:
    ret= 0;
    goto lblEnd;
lblEnd:
    return ret;
}
