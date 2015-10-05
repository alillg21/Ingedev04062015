#include <string.h>
#include "str.h"
#include "dft.h"

#define CHK CHECK(ret>=0,lblKO)

/** Get compile date.
 * \return pointer to the date of compilation of the file dft.c
 * 
 * The function allows to identify the compilation date of the application.
 * It is used together with the function getCmpTim() to detect whether the current version of the data base
 * is created by the current or by the previous version of the application.
 * If the current version of the application and the database are not the same
 * a date base rebuilding is ussually required.
 * 
 * This method is used while treating POWER ON event.
 * \sa
 *  - getCmpTim()
 * \remark
 * The format of compile date depends on the compiler.
 * It is not the same for Microsoft Visual and ARM.
 */
const char *getCmpDat(void){ return __DATE__;}
/** Get compile time.
 * \return pointer to the time of compilation of the file dft.c
 * 
 * The function allows to identify the compilation time of the application.
 * It is used together with the function getCmpDat() to detect whether the current version of the data base
 * is created by the current or by the previous version of the application.
 * If the current version of the application and the database are not the same
 * a date base rebuilding is ussually required.
 * 
 * This method is used while treating POWER ON event.
 * \sa
 *  - getCmpDat()
 */
const char *getCmpTim(void){ return __TIME__;}

static int fillDftApp(void){
    int ret;
    trcS("fillDftApp: Beg\n");

    ret= mapPutStr(appChn,dftAppChn); CHK;
    ret= mapPutStr(appPabx,dftAppPabx); CHK;
    ret= mapPutStr(appPrefix,dftAppPrefix); CHK;
    ret= mapPutStr(appPhone,dftAppPhone); CHK;
    ret= mapPutStr(appMrcPwd,dftAppMrcPwd); CHK;
    ret= mapPutStr(appAdmPwd,dftAppAdmPwd); CHK;
    ret= mapPutByte(appExp,dftAppExp); CHK;

    ret= 1; CHECK(ret>0,lblKO); goto lblEnd;
lblKO: ret= -1;
lblEnd:
    trcFN("fillDftApp: ret=%d\n",ret);
    return ret;
}

static int fillDftMnu(void){
    int ret;
    word key;

    trcS("fillDftMnu: Beg\n");
    mapCache(mnuBeg); //enable cache
    for(key= mnuBeg+1; key<mnuEnd; key++){
        ret= mapPut(key,dftMnu[key-mnuBeg-1],0);
        CHK;
    }

    ret= mapSave(mnuBeg); CHECK(ret>=0,lblKO);
    ret= 1; goto lblEnd;
lblKO: trcErr(ret); ret= -1;
lblEnd:
    mapCache(-mnuBeg); //disable cache
    trcFN("fillDftMnu: ret=%d\n",ret);
    return ret;
}

static int fillDftMsg(void){
    int ret;
    word key;

    trcS("fillDftMsg: Beg\n");
    mapCache(msgBeg); //enable cache
    for(key= msgBeg+1; key<msgEnd; key++){
        ret= mapPut(key,dftMsg[key-msgBeg-1],0);
        CHK;
    }

    ret= mapSave(msgBeg); CHECK(ret>=0,lblKO);
    ret= 1; goto lblEnd;
lblKO: trcErr(ret); ret= -1;
lblEnd:
    mapCache(-msgBeg); //disable cache
    trcFN("fillDftMsg: ret=%d\n",ret);
    return ret;
}

static int fillDftRpt(void){
    int ret;
    word key;

    trcS("fillDftRpt: Beg\n");
    for(key= rptBeg+1; key<rptEnd; key++){
        ret= mapPut(key,dftRpt[key-rptBeg-1],0);
        CHK;
    }

    ret= 1; goto lblEnd;
lblKO: trcErr(ret); ret= -1;
lblEnd:
    trcFN("fillDftRpt: ret=%d\n",ret);
    return ret;
}

static int fillDftScr(void){
    int ret;
    word i;
    trcS("fillDftScr: Beg\n");

    mapCache(scrBeg); //enable cache
    for(i= 0; i<DIM(infBeg,infEnd); i++){
        ret= mapMove(scrBeg,i); CHK;
        ret= mapPut((word)(scrCmd),&dftScr[i]._scrCmd,sizeof(char)); CHK;
        ret= mapPut((word)(scrMsg0),&dftScr[i]._scrMsg0,sizeof(word)); CHK;
        ret= mapPut((word)(scrMsg1),&dftScr[i]._scrMsg1,sizeof(word)); CHK;
        ret= mapPut((word)(scrMsg2),&dftScr[i]._scrMsg2,sizeof(word)); CHK;
        ret= mapPut((word)(scrMsg3),&dftScr[i]._scrMsg3,sizeof(word)); CHK;
        ret= mapPut((word)(scrDly),&dftScr[i]._scrDly,sizeof(byte)); CHK;
        ret= mapSave(scrBeg); CHECK(ret>=0,lblKO);
    }

    ret= 1; CHECK(ret>0,lblKO); goto lblEnd;
lblKO: ret= -1;
lblEnd:
    mapCache(-scrBeg); //disable cache
    trcFN("fillDftScr: ret=%d\n",ret);
    return ret;
}

static int fillDftReg(void){
    int ret;
    trcS("fillDftReg: Beg\n");


    ret= 1; CHECK(ret>0,lblKO); goto lblEnd;
lblKO: ret= -1;
lblEnd:
    trcFN("fillDftReg: ret=%d\n",ret);
    return ret;
}

static void moveSlider(char *sld){
    if(strlen(sld)>=dspW) memset(sld,0,dspW);
    strcat(sld,".");
    dspLS(3,sld);
}

/** Fill the UTA data base by default values.
 * \param  key (I) Starting sentinel of the data structure to be reset to default values.
 * \return
 * - 1 if everything is OK
 * - negative if failure
 * 
 * The default data values are given in uta:dat attributes for fields in single records and table rows.
 * For multirecord fields they are provided in uta:data/uta/instance tags. 
 * If the parameter is keyBeg (zero) all data structures are reset to default values.
 * 
 * It is a time consuming operation.
 * Depending on the number of data elements in the data base it can take up to several minutes.
 * This function is usually called only once just after application download.
 */
int dftReset(word key){
    int ret;
    char sld[dspW+1];
    trcFN("dftReset: key=%d\n",key);

    ret= mapReset(key); CHK;
    if(key==keyBeg){
        memset(sld,0,dspW+1);

        moveSlider(sld); ret= fillDftApp(); CHK;
        moveSlider(sld); ret= fillDftMnu(); CHK;
        moveSlider(sld); ret= fillDftMsg(); CHK;
        moveSlider(sld); ret= fillDftRpt(); CHK;
        moveSlider(sld); ret= fillDftScr(); CHK;
        moveSlider(sld); ret= fillDftReg(); CHK;

        //reset compiler date and time into reserved fields
        ret= mapPutStr(appCmpDat,getCmpDat()); CHK;
        ret= mapPutStr(appCmpTim,getCmpTim()); CHK;
    }else{
        switch(key){
            case appBeg: ret= fillDftApp(); break;
            case mnuBeg: ret= fillDftMnu(); break;
            case msgBeg: ret= fillDftMsg(); break;
            case rptBeg: ret= fillDftRpt(); break;
            case scrBeg: ret= fillDftScr(); break;
            case regBeg: ret= fillDftReg(); break;
            default: ret= -1; VERIFY(ret>=0); break;
        }
        CHK;
    }

    ret= 1;
    goto lblEnd;

lblKO:
    trcErr(ret);
    ret= -1;
lblEnd:
    trcFN("dftReset: ret=%d\n",ret);
    return ret;
}

