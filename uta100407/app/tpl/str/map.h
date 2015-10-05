#ifndef __MAP_H

//this file is included in map.c only
//it is not visible for other source files

#include "str.h"
static tRecord recApp; //application parameters record
static tContainer cntApp;
static word appLen[DIM(appBeg,appEnd)]= {
    lenCmpDat,  //appCmpDat
    lenCmpTim,  //appCmpTim
    lenChn,  //appChn
    lenPabx,  //appPabx
    lenPrefix,  //appPrefix
    lenPhone,  //appPhone
    lenPwd,  //appMrcPwd
    lenPwd,  //appAdmPwd
    sizeof(byte),  //appExp
};
static word appMap[DIM(appBeg,appEnd)];

static tTable tabMnu; //menu tree
static tContainer cntMnu;
static byte mnuCache[lenMnu*DIM(mnuBeg,mnuEnd)
];

static tTable tabMsg; //screen messages
static tContainer cntMsg;
static byte msgCache[dspW*DIM(msgBeg,msgEnd)
];

static tTable tabRpt; //receipt formatting strings
static tContainer cntRpt;
static tRecord recScr; //user screen structure
static tTable tabScr;
static tContainer cntScr;
static word scrLen[DIM(scrBeg,scrEnd)]= {
    sizeof(char),  //scrCmd
    sizeof(word),  //scrMsg0
    sizeof(word),  //scrMsg1
    sizeof(word),  //scrMsg2
    sizeof(word),  //scrMsg3
    sizeof(byte),  //scrDly
};
static word scrMap[DIM(scrBeg,scrEnd)];
static byte scrCache[
    sizeof(char)+ //scrCmd
    sizeof(word)+ //scrMsg0
    sizeof(word)+ //scrMsg1
    sizeof(word)+ //scrMsg2
    sizeof(word)+ //scrMsg3
    sizeof(byte)+ //scrDly
0];

static tRecord recReg; //registry record
static tContainer cntReg;
static word regLen[DIM(regBeg,regEnd)]= {
    sizeof(word),  //regTrxNo
};
static word regMap[DIM(regBeg,regEnd)];

static tRecord recTra; //transaction related data situated in volatile memory
static tContainer cntTra;
static word traLen[DIM(traBeg,traEnd)]= {
    lenDatTim,  //traDatTim
    sizeof(word),  //traMnuItm
    dspW,  //traCtx
    sizeof(char),  //traEntMod
    lenTrk2,  //traTrk2
    lenPan,  //traPan
    lenExpDat,  //traExpDat
    lenAmt,  //traAmt
};
static word traMap[DIM(traBeg,traEnd)];

typedef struct sVol{
    char _traDatTim[lenDatTim];
    word _traMnuItm;
    char _traCtx[dspW];
    char _traEntMod;
    char _traTrk2[lenTrk2];
    char _traPan[lenPan];
    char _traExpDat[lenExpDat];
    char _traAmt[lenAmt];
}tVol;
static tVol vol; //reserve volatile memory


#define __MAP_H
#endif

