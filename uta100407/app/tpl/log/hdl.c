
#include <string.h>
#include "log.h"
#include "tst.h"

//forward declarations
static void hdlSubmenu(word msg1, word msg2);

static void hdlSelect(word MnuItm){
    int ret;
    char buf[lenMnu+1];
    
    traReset();
    ret= mapPutWord(traMnuItm,MnuItm); CHECK(ret>=0,lblKO);
    ret= valOpr(); CHECK(ret>0,lblKO); //validate operator         
    ret= mapGet(MnuItm,buf,lenMnu+1); CHECK(ret>=0,lblKO);
    ret= mapPutStr(traCtx,buf+2); CHECK(ret>=0,lblKO);
    
    switch(MnuItm){
        case mnuCustomer:   stub("CUSTOMER");              break;
        case mnuMerchant:   stub("MERCHANT");              break;
        case mnuAdmin:      stub("ADMIN");                 break;
#ifdef __TEST__
        case mnuTest:       stub("TEST");                  break;
#endif
        default: break;
    }
        goto lblEnd;
lblKO:
    trcErr(ret);
lblEnd:
    return;
}

static void hdlSubmenu(word msg1,word msg2){
    int ret;
    byte sta; //menu state, it is (upper item)*10 + (current item)
    char lvl; //menu level
    char mnu[MNUMAX][dspW + 1]; //the final menu array prepared to mnuSelect
    char *ptr[MNUMAX];          //array of pointers to mnu items
    word idx[MNUMAX];           //the correspondent indices
    byte idxP; //index within ptr array
    word idxM; //take values from msg1..msg2
    char itm[lenMnu+1]; //(menu level)+(access type)+(item name)+(ending zero)
//    byte MnuPop;
        
    memset(ptr,0,sizeof(ptr));  
    ret= mapGet(msg1,itm,lenMnu+1); //it is the menu name
    CHECK(ret>0,lblKO);
    VERIFY(strlen(itm)<=lenMnu+1);
    strcpy(mnu[0],itm+2); //omit menu level and access type
    lvl= itm[0]+1;

    idxP= 0;
    ptr[idxP++]= mnu[0]; //it is the menu name
    ret= nvmHold(0);
    for(idxM= msg1+1; idxM<msg2; idxM++){ //build mnu and ptr arrays
        mapGet(idxM,itm,lenMnu+1); //retrieve menu item
        if(itm[0]!=lvl) continue; //only items of level lvl are included
        VERIFY(strlen(itm)<=lenMnu+1);
        strcpy(mnu[idxP],itm+2); //the first and second characters are menu level and access type, it is not showed
        ptr[idxP]= mnu[idxP]; //fill the pointers array
        idx[idxP]= idxM; //save the index to be returned if this item will be selected
        if(++idxP >= MNUMAX)    //no more than MNUMAX items in menu
            break;              
    }
    nvmRelease(0);
    if(idxP==1) goto lblEnd; //empty menu - nothing to do

    sta = 0;                        //it is started with the current item 1 at the top of the screen
    do{
        ret= mnuSelect((Pchar *)ptr,sta,60); //perform user dialog
        if(ret<=0)                  //timeout or aborted - nothing to do
            break;              
        VERIFY(ret > 0);
        VERIFY(ret < MNUMAX*MNUMAX);
        sta = ret;
        idxM = idx[sta % MNUMAX];   //sta%MNUMAX is the current item selected
        hdlSelect((word)(idxM)); 
//        ret= mapGetByte(appMnuPop,MnuPop); CHECK(ret>0,lblKO);
//        if(MnuPop) break;
    }while(sta);
    goto lblEnd;

lblKO:
    trcErr(ret);
lblEnd:
    return;
}

void hdlMainMenu(void){
    int ret;
    trcS("hdlMainMenu: Beg\n");
    ret= mapPutWord(traMnuItm,mnuMainMenu); CHECK(ret>=0,lblKO);
    hdlSubmenu(mnuMainMenu,mnuEnd);
    trcS(": End\n");
lblKO:;
}

static int isNewSoftware(void){
    int ret;
    char dat[lenCmpDat+1];
    char tim[lenCmpTim+1];

    return 0;
    memset(dat,0,lenCmpDat+1);
    memset(tim,0,lenCmpTim+1);

    ret= mapGet(appCmpDat,dat,lenCmpDat+1);
    CHECK(ret>0,lblKO);
    VERIFY(ret<=lenCmpDat+1);
        
    ret= mapGet(appCmpTim,tim,lenCmpTim+1);
    CHECK(ret>0,lblKO);
    VERIFY(ret<=lenCmpTim+1);
        
    if((strcmp(dat,getCmpDat())==0)&&(strcmp(tim,getCmpTim())==0)) return 0;
    return 1;
lblKO:
    return -1;
}

static byte mapOK= 0; //is mapInit called?
void hdlPowerOn(void){
    int ret;
    char key;
    trcS("hdlPowerOn: Beg\n");
    
    ret= mapInit(); //setup map tables
    CHECK(ret>0,lblKO);
    mapOK= 1;

    traReset();

    ret= isNewSoftware();
    if(ret==0) goto lblEnd;//it is negative if the DFS is not created; it is positive if compile time is not the same

    dspClear();
    dspLS(0,"  NEW SOFTWARE  ");
    dspLS(1,"  MEMORY RESET  ");

    tmrStart(0,3*100);
    kbdStart(1);
    key= 0;
    while(tmrGet(0)){ //3 seconds to cancel
        key= kbdKey();
        if(key!=0) break;
    }
    kbdStop();
    tmrStop(0);

    dspClear();
    if(key==kbdANN) return; //operation cancelled by the user

    dspLS(0,"  MEMORY RESET  ");
    dftReset(keyBeg); //fill database by default values
    mapInit(); //re-initialise map tables
    traReset();
    dspLS(1,"      DONE     ");
    tmrPause(1);

    goto lblEnd;
lblKO:
    trcErr(ret);
    //fatal error, the terminal is not in a working state
lblEnd:
	//activate cache buffers
    mapCache(mnuBeg);
    mapLoad(mnuBeg);

    mapCache(msgBeg);
    mapLoad(msgBeg);

    mapCache(scrBeg);

    trcS("hdlPowerOn: End\n");
}

void hdlBefore(void){
    nvmStart();
    //dspStart();
    prtStart();
    if(mapOK) traReset();
}

void hdlAfter(void){
    dspStop();
    prtStop();
}

int hdlAskMagStripe(const char *buf){
    int ret;
    char trk2[lenTrk2+1];
    VERIFY(buf);
    
    memset(trk2,0,lenTrk2);
    memcpy(trk2,buf+128,lenTrk2);
    
    ret= mapPutStr(traTrk2,trk2);
    CHECK(ret>0,lblKO);

    //validate the track2 acceptance
    //..
        
    ret= mapPutByte(traEntMod,'M'); //means that the card can be treated by the application
    CHECK(ret>0,lblKO);
    
    return 1;
lblKO:
    trcErr(ret);
    return 0;
}

void hdlMagStripe(const char *buf){
    int ret;
    char trk2[lenTrk2+1];
//    byte MnuItm;
    VERIFY(buf);
    
    memset(trk2,0,lenTrk2);
    memcpy(trk2,buf+128,lenTrk2);
    ret= mapPutStr(traTrk2,trk2); CHECK(ret>0,lblKO);    
    ret= mapPutByte(traEntMod,'M'); CHECK(ret>0,lblKO);    
//    ret= mapGetByte(appDftTra,MnuItm); CHECK(ret>0,lblKO);    
//    ret= mapPutByte(traMnuItm,MnuItm); CHECK(ret>0,lblKO);
    
//    pmtPayment();
    return;
lblKO:
    trcErr(ret);
}

/* for EMV applications
void hdlSmartCard(const byte *buf){
    int ret;
    byte aid[1+lenAid];
    byte fsBuf[__FSBUFLEN__];
    byte MnuItm;
    VERIFY(buf);

    ret= isInitialised();
    if(ret!=1) return;
    
    memcpy(fsBuf,buf,__FSBUFLEN__);
    ret= emvSetSelectResponse(aid,fsBuf); CHECK(ret>=0,lblKO);
    emvSetFSbuf(fsBuf); //set the buffer to hold the last APPLICATION select command

    ret= emvMapAidMove(aid);
    CHECK(ret>=0,lblKO);

    ret= mapPutByte(traEntMod,'C');
    CHECK(ret>0,lblKO);

    ret= mapGetByte(appDftTra,MnuItm);
    CHECK(ret>0,lblKO);
    
    ret= mapPutByte(traMnuItm,MnuItm);
    CHECK(ret>0,lblKO);
    
    pmtPayment();
lblKO: return;
}
*/

void hdlWakeUp(void) {
	//wake up event processing
	//This function is called once per minute	
}

int hdlTimer(card id, card * date, card * time) {
	/*
    char hhmmss[6 + 1];
    card hh, mm, ss;

    VERIFY(date);
    VERIFY(time);

    hdlWakeUp();                //perform treatment

    //now re-schedule it to one minute later        
    tim2asc(hhmmss, *time);
    dec2num(&ss, hhmmss + 4, 2);
    VERIFY(ss < 60);
    dec2num(&mm, hhmmss + 2, 2);
    VERIFY(mm < 60);
    dec2num(&hh, hhmmss + 0, 2);
    VERIFY(hh < 24);

    mm++;
    if(mm >= 60) {
        mm = 0;
        hh++;
    }
    if(hh >= 24) {
        hh = 0;
    }
    *date = 0xFFFFFFFFL;

    num2dec(hhmmss + 0, hh, 2);
    num2dec(hhmmss + 2, mm, 2);
    num2dec(hhmmss + 4, ss, 2);

    asc2tim(time, hhmmss);
    */
    return 1;                   //0 means without re-scheduling, 1 means with re-schrduling
}
