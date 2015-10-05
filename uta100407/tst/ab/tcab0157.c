#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcab0157.
//Constructing binary XML files.

#define CHK if(ret<0) return ret
#define NVM 15
enum {
    bmlSWITCH_PAGE = 0x00,
    bmlEND = 0x01,
    bmlENTITY = 0x02,
    bmlSTR_I = 0x03,
    bmlLITERAL = 0x04
};
enum eToken {                   //Token fields
    tokBeg,
    tokReq = 5,                 //5
    tokLst,                     //6
    tokArt,                     //7
    tokRsp,                     //8
    tokTot,                     //9
    tokRow,                     //10
    tokFld,                     //11
    tokFile,                    //12
    tokDone,                    //13
    tokItm,                     //14
    tokInt,                     //15
    tokTax,                     //16
    tokBak,                     //17
    tokBox,                     //18
    tokEnd
};

enum eAtr {                     //Attribute fields
    atrBeg,
    atrId = 5,                  //05
    atrTid,                     //06
    atrEcr,                     //07
    atrBar,                     //08
    atrDsc,                     //09
    atrTax,                     //0A
    atrApp,                     //0B
    atrCode,                    //0C
    atrDate,                    //0D
    atrTime,                    //0E
    atrMid,                     //0F
    atrBat,                     //10
    atrSeq,                     //11
    atrPhone,                   //12
    atrAmt,                     //13
    atrAut,                     //14
    atrMac,                     //15
    atrNum,                     //16
    atrVal,                     //17
    atrFtp,                     //18
    atrDir,                     //19
    atrLogin,                   //1A
    atrPwd,                     //1B
    atrQty,                     //1C
    atrDt,                      //1D
    atrScope,                   //1E
    atrPri,                     //1F
    atrVhc,                     //20
    atrIss,                     //21
    atrReason,                  //22
    atrDst,                     //23
    atrVersion,                 //24
    atrEnd
};

#define bitAtr 0
#define bitTag 1
#define mskAtr (0x80 >> bitAtr)
#define mskTag (0x80 >> bitTag)

static int putByte(tBuffer * buf, byte b) {
    if(buf)
        return bufApp(buf, &b, sizeof(byte));
    return nvmSave(NVM, &b, 0xFFFFFFFFL, sizeof(byte));
}

static int putStr(tBuffer * buf, const char *str) {
    if(buf)
        return bufApp(buf, (byte *) str, strlen(str));
    return nvmSave(NVM, str, 0xFFFFFFFFL, (word) strlen(str));
}

int buildStr(tBuffer * buf, const char *str) {
    int ret;

    VERIFY(str);

    ret = putByte(buf, bmlSTR_I);
    CHK;

    ret = putStr(buf, str);
    CHK;

    ret = putByte(buf, 0);
    CHK;

    return 1 + strlen(str) + 1;
}

static struct {
    byte tag;
    byte atr;
} sta;                          //current tag and attribute

//forward declarations
int buildAtrId(tBuffer * buf);
int buildAtrTid(tBuffer * buf);
int buildAtrEcr(tBuffer * buf);
int buildAtrBar(tBuffer * buf);
int buildAtrDsc(tBuffer * buf);
int buildAtrTax(tBuffer * buf);
int buildAtrApp(tBuffer * buf);
int buildAtrCode(tBuffer * buf);
int buildAtrDate(tBuffer * buf);
int buildAtrTime(tBuffer * buf);
int buildAtrMid(tBuffer * buf);
int buildAtrBat(tBuffer * buf);
int buildAtrSeq(tBuffer * buf);
int buildAtrPhone(tBuffer * buf);
int buildAtrAmt(tBuffer * buf);
int buildAtrAut(tBuffer * buf);
int buildAtrMac(tBuffer * buf);
int buildAtrNum(tBuffer * buf);
int buildAtrVal(tBuffer * buf);
int buildAtrFtp(tBuffer * buf);
int buildAtrDir(tBuffer * buf);
int buildAtrLogin(tBuffer * buf);
int buildAtrPwd(tBuffer * buf);
int buildAtrQty(tBuffer * buf);
int buildAtrDt(tBuffer * buf);
int buildAtrScope(tBuffer * buf);
int buildAtrPri(tBuffer * buf);
int buildAtrVhc(tBuffer * buf);
int buildAtrIss(tBuffer * buf);
int buildAtrReason(tBuffer * buf);
int buildAtrDst(tBuffer * buf);
int buildAtrVersion(tBuffer * buf);
static int buildAtr(tBuffer * buf, const byte * ctl) {
    int ret;
    int idx;

    VERIFY(ctl);

    idx = 0;
    while(*ctl) {
        ret = putByte(buf, *ctl);
        CHK;
        sta.atr = *ctl;         //current attribute
        switch (sta.atr) {
          case atrId:
              ret = buildAtrId(buf);
              break;
          case atrTid:
              ret = buildAtrTid(buf);
              break;
          case atrEcr:
              ret = buildAtrEcr(buf);
              break;
          case atrBar:
              ret = buildAtrBar(buf);
              break;
          case atrDsc:
              ret = buildAtrDsc(buf);
              break;
          case atrTax:
              ret = buildAtrTax(buf);
              break;
          case atrApp:
              ret = buildAtrApp(buf);
              break;
          case atrCode:
              ret = buildAtrCode(buf);
              break;
          case atrDate:
              ret = buildAtrDate(buf);
              break;
          case atrTime:
              ret = buildAtrTime(buf);
              break;
          case atrMid:
              ret = buildAtrMid(buf);
              break;
          case atrBat:
              ret = buildAtrBat(buf);
              break;
          case atrSeq:
              ret = buildAtrSeq(buf);
              break;
          case atrPhone:
              ret = buildAtrPhone(buf);
              break;
          case atrAmt:
              ret = buildAtrAmt(buf);
              break;
          case atrAut:
              ret = buildAtrAut(buf);
              break;
          case atrMac:
              ret = buildAtrMac(buf);
              break;
          case atrNum:
              ret = buildAtrNum(buf);
              break;
          case atrVal:
              ret = buildAtrVal(buf);
              break;
          case atrFtp:
              ret = buildAtrFtp(buf);
              break;
          case atrDir:
              ret = buildAtrDir(buf);
              break;
          case atrLogin:
              ret = buildAtrLogin(buf);
              break;
          case atrPwd:
              ret = buildAtrPwd(buf);
              break;
          case atrQty:
              ret = buildAtrQty(buf);
              break;
          case atrDt:
              ret = buildAtrDt(buf);
              break;
          case atrScope:
              ret = buildAtrScope(buf);
              break;
          case atrPri:
              ret = buildAtrPri(buf);
              break;
          case atrVhc:
              ret = buildAtrVhc(buf);
              break;
          case atrIss:
              ret = buildAtrIss(buf);
              break;
          case atrReason:
              ret = buildAtrReason(buf);
              break;
          case atrDst:
              ret = buildAtrDst(buf);
              break;
          case atrVersion:
              ret = buildAtrVersion(buf);
              break;
          default:
              VERIFY(*ctl == bmlEND);
              break;
        }
        CHK;
        ctl++;
        idx++;
    }
    return idx;
}

//forward declarations
int buildTagReq(tBuffer * buf);
int buildTagLst(tBuffer * buf);
int buildTagArt(tBuffer * buf);
int buildTagRsp(tBuffer * buf);
int buildTagTot(tBuffer * buf);
int buildTagRow(tBuffer * buf);
int buildTagFld(tBuffer * buf);
int buildTagFile(tBuffer * buf);
int buildTagDone(tBuffer * buf);
int buildTagItm(tBuffer * buf);
int buildTagInt(tBuffer * buf);
int buildTagTax(tBuffer * buf);
int buildTagBak(tBuffer * buf);
int buildTagBox(tBuffer * buf);
static int buildTag(tBuffer * buf, const byte * ctl) {
    int ret;
    int idx;

    VERIFY(ctl);

    idx = 0;
    while(*ctl) {
        ret = putByte(buf, *ctl);
        CHK;
        sta.tag = (*ctl) & 0x3F;    //current tag
        switch (sta.tag) {
          case tokReq:
              ret = buildTagReq(buf);
              break;
          case tokLst:
              ret = buildTagLst(buf);
              break;
          case tokArt:
              ret = buildTagArt(buf);
              break;
          case tokRsp:
              ret = buildTagRsp(buf);
              break;
          case tokTot:
              ret = buildTagTot(buf);
              break;
          case tokRow:
              ret = buildTagRow(buf);
              break;
          case tokFld:
              ret = buildTagFld(buf);
              break;
          case tokFile:
              ret = buildTagFile(buf);
              break;
          case tokDone:
              ret = buildTagDone(buf);
              break;
          case tokItm:
              ret = buildTagItm(buf);
              break;
          case tokInt:
              ret = buildTagInt(buf);
              break;
          case tokTax:
              ret = buildTagTax(buf);
              break;
          case tokBak:
              ret = buildTagBak(buf);
              break;
          case tokBox:
              ret = buildTagBox(buf);
              break;
          default:
              VERIFY(*ctl == bmlEND);
              break;
        }
        CHK;
        ctl += ret;             //if zero is returned it means repeat to append a tag
        idx++;
    }
    return idx;
}

int buildTagReq(tBuffer * buf) {
/*
<?xml version="1.0"?>
<req app="5V" id="recharge"
    dt="20070102120600"
    tid="00000001"
    mid="1234"
    bat="003"
    seq="123"
    phone="381641243145"
    amt="1000.00"
    aut="1"
/>
*/
    const byte ctl[] = {
        atrApp,
        atrId,
        atrDt,
        atrTid,
        atrMid,
        atrBat,
        atrSeq,
        atrPhone,
        atrAmt,
        atrAut,
        bmlEND,
        0
    };
    int ret;

    ret = buildAtr(buf, ctl);
    CHK;

    return 1;
}

//These functions replace the data base cursor movement and data retrieval
static word lstPos;
void setLstPos(word pos) {
    lstPos = pos;
}
int getLstPos(void) {
    return lstPos;
}
int getLstDim(void) {
    return 2;
}
const char *getValId(void) {
    const char *val[] = {
        "recharge",
        "recharge",
        0
    };
    VERIFY(getLstPos() < getLstDim());
    return val[getLstPos()];
}
const char *getValDt(void) {
    const char *val[] = {
        "20070102120600",
        "20070102120700",
        0
    };
    VERIFY(getLstPos() < getLstDim());
    return val[getLstPos()];
}
const char *getValSeq(void) {
    const char *val[] = {
        "1",
        "2",
        0
    };
    VERIFY(getLstPos() < getLstDim());
    return val[getLstPos()];
}
const char *getValPhone(void) {
    const char *val[] = {
        "381641243145",
        "381641243145",
        0
    };
    VERIFY(getLstPos() < getLstDim());
    return val[getLstPos()];
}
const char *getValAmt(void) {
    const char *val[] = {
        "1000.00",
        "1000.00",
        0
    };
    VERIFY(getLstPos() < getLstDim());
    return val[getLstPos()];
}
const char *getValNum(void) {
    const char *val[] = {
        "88800000000001",
        "88800000000002",
        0
    };
    VERIFY(getLstPos() < getLstDim());
    return val[getLstPos()];
}

int buildTagRow(tBuffer * buf) {
/*
    <row id="recharge"
        dt="20070102120600"
        seq="1"
    	phone="381641243145"
    	amt="1000.00"
        num="88800000000001"
    />
*/
    const byte ctl[] = {
        atrId,
        atrDt,
        atrSeq,
        atrPhone,
        atrAmt,
        atrNum,
        bmlEND,
        0
    };
    int ret;
    word pos;

    ret = buildAtr(buf, ctl);
    CHK;

    pos = getLstPos();          //get current cursor position
    pos++;                      //move to the next row
    if(pos >= getLstDim())      //if it is a last record go to the next tag
        return 1;
    setLstPos(pos);

    return 0;                   //zero return value means generate one more row
}

int buildTagLst(tBuffer * buf) {
    int ret;

/*
<?xml version="1.0"?>
<lst app="5V" id="batchUpload"
    dt="20070102230000"
    tid="00000001"
    mid="1234"
    bat="003">
    <row id="recharge"
        dt="20070102120600"
        seq="1"
    	phone="381641243145"
    	amt="1000.00"
        num="88800000000001"
    />
    <row id="recharge"
        dt="20070102120700"
        seq="2"
    	phone="381641243145"
    	amt="1000.00"
        num="88800000000002"
    />
</lst>
*/
    const byte ctl[] = {
        atrApp,
        atrId,
        atrDt,
        atrTid,
        atrMid,
        atrBat,
        bmlEND,
        0
    };

    ret = buildAtr(buf, ctl);
    CHK;

    setLstPos(0);               //move the cursor at the beginning of the table

    if(getLstDim() == 0)        //if the batch is empty skip the tag row
        return 2;

    return 1;
}

//Filling attribute values.
//It simulates data retrieval from the data base
int buildAtrApp(tBuffer * buf) {
    return buildStr(buf, "5V");
}
int buildAtrId(tBuffer * buf) {
    VERIFY(sta.atr == atrId);
    switch (sta.tag) {
      case tokReq:
          return buildStr(buf, "recharge");
      case tokRow:
          return buildStr(buf, getValId());
      case tokLst:
          return buildStr(buf, "batchUpload");
      default:
          break;
    }
    return -1;
}
int buildAtrDt(tBuffer * buf) {
    VERIFY(sta.atr == atrDt);
    switch (sta.tag) {
      case tokReq:
          return buildStr(buf, "20070102120600");
      case tokRow:
          return buildStr(buf, getValDt());
      case tokLst:
          return buildStr(buf, "20070102230000");
      default:
          break;
    }
    return -1;
}
int buildAtrTid(tBuffer * buf) {
    return buildStr(buf, "00000001");
}
int buildAtrMid(tBuffer * buf) {
    return buildStr(buf, "1234");
}
int buildAtrBat(tBuffer * buf) {
    VERIFY(sta.atr == atrBat);
    switch (sta.tag) {
      case tokReq:
          return buildStr(buf, "003");
      case tokLst:
          return buildStr(buf, "003");
      default:
          break;
    }
    return -1;
}
int buildAtrSeq(tBuffer * buf) {
    VERIFY(sta.atr == atrSeq);
    switch (sta.tag) {
      case tokReq:
          return buildStr(buf, "123");
      case tokRow:
          return buildStr(buf, getValSeq());
      default:
          break;
    }
    return -1;
}
int buildAtrPhone(tBuffer * buf) {
    VERIFY(sta.atr == atrPhone);
    switch (sta.tag) {
      case tokReq:
          return buildStr(buf, "381641243145");
      case tokRow:
          return buildStr(buf, getValPhone());
      default:
          break;
    }
    return -1;
}
int buildAtrAmt(tBuffer * buf) {
    VERIFY(sta.atr == atrAmt);
    switch (sta.tag) {
      case tokReq:
          return buildStr(buf, "1000.00");
      case tokRow:
          return buildStr(buf, getValAmt());
      default:
          break;
    }
    return -1;
}
int buildAtrAut(tBuffer * buf) {
    return buildStr(buf, "2");
}
int buildAtrNum(tBuffer * buf) {
    VERIFY(sta.atr == atrNum);
    switch (sta.tag) {
      case tokRow:
          return buildStr(buf, getValNum());
      default:
          break;
    }
    return -1;
}

//Not implemented:
int buildTagArt(tBuffer * buf) {
    return -1;
}
int buildTagRsp(tBuffer * buf) {
    return -1;
}
int buildTagTot(tBuffer * buf) {
    return -1;
}
int buildTagFld(tBuffer * buf) {
    return -1;
}
int buildTagFile(tBuffer * buf) {
    return -1;
}
int buildTagDone(tBuffer * buf) {
    return -1;
}
int buildTagItm(tBuffer * buf) {
    return -1;
}
int buildTagInt(tBuffer * buf) {
    return -1;
}
int buildTagTax(tBuffer * buf) {
    return -1;
}
int buildTagBak(tBuffer * buf) {
    return -1;
}
int buildTagBox(tBuffer * buf) {
    return -1;
}

int buildAtrEcr(tBuffer * buf) {
    return -1;
}
int buildAtrBar(tBuffer * buf) {
    return -1;
}
int buildAtrDsc(tBuffer * buf) {
    return -1;
}
int buildAtrTax(tBuffer * buf) {
    return -1;
}
int buildAtrCode(tBuffer * buf) {
    return -1;
}
int buildAtrDate(tBuffer * buf) {
    return -1;
}
int buildAtrTime(tBuffer * buf) {
    return -1;
}
int buildAtrMac(tBuffer * buf) {
    return -1;
}
int buildAtrVal(tBuffer * buf) {
    return -1;
}
int buildAtrFtp(tBuffer * buf) {
    return -1;
}
int buildAtrDir(tBuffer * buf) {
    return -1;
}
int buildAtrLogin(tBuffer * buf) {
    return -1;
}
int buildAtrPwd(tBuffer * buf) {
    return -1;
}
int buildAtrQty(tBuffer * buf) {
    return -1;
}
int buildAtrScope(tBuffer * buf) {
    return -1;
}
int buildAtrPri(tBuffer * buf) {
    return -1;
}
int buildAtrVhc(tBuffer * buf) {
    return -1;
}
int buildAtrIss(tBuffer * buf) {
    return -1;
}
int buildAtrReason(tBuffer * buf) {
    return -1;
}
int buildAtrDst(tBuffer * buf) {
    return -1;
}
int buildAtrVersion(tBuffer * buf) {
    return -1;
}

int buildHdr(tBuffer * buf) {   //Build file header
    int ret;

    if(!buf)                    //means that the data should be written into a file
        nvmRemove(NVM);
    ret = putByte(buf, 0x01);   //WBXML version 1.1
    CHK;
    ret = putByte(buf, 0x01);   //Unknown public identifier
    CHK;
    ret = putByte(buf, 0x04);   //charset= ISO8859-1
    CHK;
    ret = putByte(buf, 0x00);   //string table length
    CHK;
    return 4;
}

int testReq(void) {
/* Build recharge request and write it into a buffer
<?xml version="1.0"?>
<req app="5V" id="recharge"
    dt="20070102120600"
    tid="00000001"
    mid="1234"
    bat="003"
    seq="123"
    phone="381641243145"
    amt="1000.00"
    aut="1"
/>
*/
    int ret;
    const byte ctl[] = {
        (byte) (tokReq | mskAtr),
        0
    };
    tBuffer buf;
    byte dat[1024];

    bufInit(&buf, dat, 1024);

    ret = buildHdr(&buf);
    CHK;
    ret = buildTag(&buf, ctl);
    CHK;

    trcS("testReq: ");
    trcBAN(bufPtr(&buf), bufLen(&buf));
    trcS("\n");
    return bufLen(&buf);
}

int testLst(void) {
/* Build the batch upload list and write it into a file.
<?xml version="1.0"?>
<lst app="5V" id="batchUpload"
    dt="20070102230000"
    tid="00000001"
    mid="1234"
    bat="003">
    <row id="recharge"
        dt="20070102120600"
        seq="1"
    	phone="381641243145"
    	amt="1000.00"
        num="88800000000001"
    />
    <row id="recharge"
        dt="20070102120700"
        seq="2"
    	phone="381641243145"
    	amt="1000.00"
        num="88800000000002"
    />
</lst>
*/
    int ret;
    const byte ctl[] = {
        (byte) (tokLst | mskTag | mskAtr),  //the root tag
        (byte) (tokRow | mskAtr),   //the tags of next level
        0
    };

    ret = buildHdr(0);
    CHK;
    ret = buildTag(0, ctl);
    CHK;

    return ret;
}

void tcab0157(void) {
    int ret;

    trcS("tcab0157 Beg\n");

    dspClear();

    dspLS(0, "Req...");
    ret = testReq();
    CHECK(ret >= 0, lblKO);

    dspLS(1, "Lst...");
    ret = testLst();
    CHECK(ret >= 0, lblKO);

    dspLS(3, "Done.");
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    trcS("tcab0157 End\n");
}
