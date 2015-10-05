/** \file
 * Unitary test case tcab0210.
 * Binary XML parser
 * Functions testes:
 * \sa
 *  - bmlParse()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0210.c $
 *
 * $Id: tcab0210.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"

enum bmlTag {
    //ABC tags
    bmlTagGet = 5,
    bmlTagPut,
    bmlTagRun,
    bmlTagData,

    //BUS tags
    bmlTagReq = 5,
    bmlTagRsp,
    bmlTagLst,
    bmlTagRow,
    bmlTagFld,

    bmlTagEnd
};
enum bmlAtr {
    //ABC Atrs
    bmlAtrId = 5,
    bmlAtrFmt,
    bmlAtrLen,
    bmlAtrVal,
    bmlAtrErr,
    bmlAtrCmd,

    //BUS atrs
    bmlBusAtrApp = 5,
    bmlBusAtrId,
    bmlBusAtrDt,
    bmlBusAtrTid,
    bmlBusAtrMid,
    bmlBusAtrSeq,
    bmlBusAtrPan,
    bmlBusAtrAmt,
    bmlBusAtrCr,
    bmlBusAtrDb,
    bmlBusAtrCode,
    bmlBusAtrDsp,
    bmlBusAtrPrt,
    bmlBusAtrSec,
    bmlBusAtrUsage,
    bmlBusAtrSer,
    bmlBusAtrBat,

    bmlAtrEnd
};

/*  
<?xml version="1.0"?>
<get>
  <data id='3'/>
  <data id='17'/>
</get>
*/
static int makeABC(tStream * stm) {
    int ret;
    word dim;

    VERIFY(stm);
    dim = 0;

    ret = stmPut(stm, (byte *) "\x03\x01\x03\x00", 4);
    CHECK(ret == 4, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(bmlTagGet | bmlBitC), 1);
    CHECK(ret == 1, lblKO);
    dim += ret;

    //First line of data
    ret = stmPut(stm, bPtr(bmlTagData | bmlBitA), 1);   //tag without content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(bmlAtrId), 1);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0x03), 1);   //global token STR_I
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, (byte *) "3", 0); //attribute value
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0), 1);  //string terminator
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END
    CHECK(ret == 1, lblKO);
    dim += ret;

    //Second line of data
    ret = stmPut(stm, bPtr(bmlTagData | bmlBitA), 1);   //tag without content with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(bmlAtrId), 1);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0x03), 1);   //global token STR_I
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, (byte *) "17", 0);    //attribute value
    CHECK(ret == 2, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0), 1);  //string terminator
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int bmlHdlTagABC(int tag, byte lvl) {
    int ret;
    const char *ptr;
    char tmp[prtW + 1];

    if(tag < 0)                 //no treatment of the end of the tag
        return 0;

    switch (tag) {
      case bmlTagGet:
          ptr = "get";
          break;
      case bmlTagPut:
          ptr = "put";
          break;
      case bmlTagRun:
          ptr = "run";
          break;
      case bmlTagData:
          ptr = "data";
          break;
      default:
          ptr = "unexpected tag";
          break;
    }
    sprintf(tmp, "tag %s", ptr);
    ret = prtS(tmp);
    return ret;
}

static int bmlHdlAtrABC(byte atr, const char *buf) {
    int ret;
    const char *ptr;
    char tmp[prtW + 1];

    switch (atr) {
      case bmlAtrId:
          ptr = "id";
          break;
      case bmlAtrFmt:
          ptr = "fmt";
          break;
      case bmlAtrLen:
          ptr = "len";
          break;
      case bmlAtrVal:
          ptr = "val";
          break;
      case bmlAtrErr:
          ptr = "err";
          break;
      case bmlAtrCmd:
          ptr = "cmd";
          break;
      default:
          ptr = "unexpected atr";
          break;
    }
    sprintf(tmp, "atr %s= [%s]", ptr, buf);
    ret = prtS(tmp);
    return ret;
}

static int appAtrVal(tStream * stm, byte atr, const char *val) {
    int ret;
    word dim;

    VERIFY(stm);
    dim = 0;

    ret = stmPut(stm, &atr, 1);
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0x03), 1);   //global token STR_I
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, (byte *) val, 0); //attribute value
    CHECK(ret == strlen(val), lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(0), 1);  //string terminator
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static int appFldIdVal(tStream * stm, const char *id, const char *val) {
    int ret;
    word dim;

    VERIFY(stm);
    dim = 0;

    ret = stmPut(stm, bPtr(bmlTagFld | bmlBitA), 1);    //tag without content and with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appAtrVal(stm, bmlAtrId, id);
    CHECK(ret, lblKO);
    dim += ret;

    ret = appAtrVal(stm, bmlAtrVal, val);
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of fld attributes set)
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

/*  
<?xml version="1.0"?>
<lst id="configuration">
    <fld id="maxOpr" val="1000"/> <!-- number of daily operations allowed -->
    <fld id="autoClose" val="2300"/> <!-- automatic day close time -->
    <fld id="site" val="123"/> <!-- site number -->
    <fld id="location" val="Lagos"/> <!-- site location -->
    <lst id="header"> <!-- receipt header -->
        <row>
            <fld id="font" val="bold"/>
            <fld id="line" val="Header line 1"/>
        </row>
        <row>
            <fld id="font" val="normal"/>
            <fld id="line" val="Header line 2"/>
        </row>
        <row>
            <fld id="font" val="normal"/>
            <fld id="line" val="Header line 3"/>
        </row>
    </lst>
    <lst id="footer"> <!-- receipt footer -->
        <row>
            <fld id="font" val="normal"/>
            <fld id="line" val="Thank you"/>
        </row>
        <row>
            <fld id="font" val="normal"/>
            <fld id="line" val="Goodbye"/>
        </row>
    </lst>
</lst>
*/
static int makeBUS(tStream * stm) {
    int ret;
    word dim;

    VERIFY(stm);
    dim = 0;

    ret = stmPut(stm, (byte *) "\x03\x01\x03\x00", 4);
    CHECK(ret == 4, lblKO);
    dim += ret;

    //<lst id="configuration">
    ret = stmPut(stm, bPtr(bmlTagLst | bmlBitC | bmlBitA), 1);  //tag with content and with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appAtrVal(stm, bmlAtrId, "configuration");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of lst attributes set)
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "maxOpr", "1000");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "autoClose", "2300");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "site", "123");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "location", "Lagos");
    CHECK(ret, lblKO);
    dim += ret;

    //<lst id="header">
    ret = stmPut(stm, bPtr(bmlTagLst | bmlBitC | bmlBitA), 1);  //tag with content and with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appAtrVal(stm, bmlAtrId, "header");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of lst attributes set)
    CHECK(ret == 1, lblKO);
    dim += ret;

    //    <row>
    //        <fld id="font" val="bold"/>
    //        <fld id="line" val="Header line 1"/>
    //    </row>    
    ret = stmPut(stm, bPtr(bmlTagRow | bmlBitC), 1);    //tag with content and without attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "font", "bold");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "line", "Header line 1");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of row)
    CHECK(ret == 1, lblKO);
    dim += ret;

    //    <row>
    //        <fld id="font" val="normal"/>
    //        <fld id="line" val="Header line 2"/>
    //    </row>    
    ret = stmPut(stm, bPtr(bmlTagRow | bmlBitC), 1);    //tag with content and without attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "font", "normal");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "line", "Header line 2");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of row)
    CHECK(ret == 1, lblKO);
    dim += ret;

    //    <row>
    //        <fld id="font" val="normal"/>
    //        <fld id="line" val="Header line 3"/>
    //    </row>    
    ret = stmPut(stm, bPtr(bmlTagRow | bmlBitC), 1);    //tag with content and without attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "font", "normal");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "line", "Header line 3");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of row)
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of lst)
    CHECK(ret == 1, lblKO);
    dim += ret;

    //<lst id="footer">
    ret = stmPut(stm, bPtr(bmlTagLst | bmlBitC | bmlBitA), 1);  //tag with content and with attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appAtrVal(stm, bmlAtrId, "footer");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of lst attributes set)
    CHECK(ret == 1, lblKO);
    dim += ret;

    //    <row>
    //        <fld id="font" val="normal"/>
    //        <fld id="line" val="Thank you"/>
    //    </row>    
    ret = stmPut(stm, bPtr(bmlTagRow | bmlBitC), 1);    //tag with content and without attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "font", "normal");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "line", "Thank you");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of row)
    CHECK(ret == 1, lblKO);
    dim += ret;

    //    <row>
    //        <fld id="font" val="normal"/>
    //        <fld id="line" val="Goodbye"/>
    //    </row>    
    ret = stmPut(stm, bPtr(bmlTagRow | bmlBitC), 1);    //tag with content and without attributes
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "font", "normal");
    CHECK(ret, lblKO);
    dim += ret;

    ret = appFldIdVal(stm, "line", "Goodbye");
    CHECK(ret, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of row)
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of lst)
    CHECK(ret == 1, lblKO);
    dim += ret;

    ret = stmPut(stm, bPtr(1), 1);  //global token END (of content of lst)
    CHECK(ret == 1, lblKO);
    dim += ret;

    return dim;
  lblKO:
    return -1;
}

static struct {
    char maxOpr[prtW + 1];
    char autoClose[prtW + 1];
    char site[prtW + 1];
    char location[prtW + 1];
    char headerfont[10][prtW + 1];
    char headerline[10][prtW + 1];
    char footerfont[10][prtW + 1];
    char footerline[10][prtW + 1];
} data;

static void prtBusData(void) {
    char tmp[64];
    byte idx;

    prtS("=== DATA ===:");
    sprintf(tmp, "maxOpr=%s", data.maxOpr);
    prtS(tmp);
    sprintf(tmp, "autoClose=%s", data.autoClose);
    prtS(tmp);
    sprintf(tmp, "site=%s", data.site);
    prtS(tmp);
    sprintf(tmp, "location=%s", data.location);
    prtS(tmp);
    prtS("HEADER FONTS:");
    for (idx = 0; idx < 10; idx++) {
        if(data.headerfont[idx][0] == 0)
            break;
        sprintf(tmp, "hf[%d]=%s", idx, data.headerfont[idx]);
        prtS(tmp);
    }
    prtS("HEADER LINES:");
    for (idx = 0; idx < 10; idx++) {
        if(data.headerline[idx][0] == 0)
            break;
        sprintf(tmp, "hl[%d]=%s", idx, data.headerline[idx]);
        prtS(tmp);
    }
    prtS("FOOTER FONTS:");
    for (idx = 0; idx < 10; idx++) {
        if(data.footerfont[idx][0] == 0)
            break;
        sprintf(tmp, "ff[%d]=%s", idx, data.footerfont[idx]);
        prtS(tmp);
    }
    prtS("FOOTER LINES:");
    for (idx = 0; idx < 10; idx++) {
        if(data.footerline[idx][0] == 0)
            break;
        sprintf(tmp, "fl[%d]=%s", idx, data.footerline[idx]);
        prtS(tmp);
    }
}

enum { busMaxOpr, busAutoClose, busSite, busLocation, busFont, busLine };   //field ids
enum { busCfg, busHeader, busFooter };  //list ids
static struct {                 //current context
    byte tag;                   //current tag
    byte fld;                   //current field
    byte lst;                   //current list
    byte row;                   //current row
} ctx;

static int bmlHdlTagBUS(int tag, byte lvl) {
    int ret;
    const char *ptr;
    char tmp[prtW + 1];

    if(tag > 0) {
        ctx.tag = tag;
        switch (tag) {
          case bmlTagLst:
              ptr = "<lst";
              break;
          case bmlTagRow:
              ptr = "<row";
              break;
          case bmlTagFld:
              ptr = "<fld";
              break;
          default:
              ptr = "<unexpected tag";
              break;
        }
    } else {                    //end of tag
        ctx.tag = 0;
        switch (-tag) {
          case bmlTagLst:
              ptr = "lst>";
              ctx.row = 0;      //reset row counter
              break;
          case bmlTagRow:
              ctx.row++;        //increment row counter when end of tag found
              ptr = "row>";
              break;
          case bmlTagFld:
              ptr = "fld>";
              break;
          default:
              ptr = "unexpected tag>";
              break;
        }
    }
    sprintf(tmp, "tag %s lvl=%d", ptr, lvl);
    ret = prtS(tmp);
    return ret;
}

static void treatId(const char *buf) {
    VERIFY(ctx.tag);
    switch (ctx.tag) {
      case bmlTagLst:
          ctx.lst = 0xFF;
          if(strcmp(buf, "configuration") == 0)
              ctx.lst = busCfg;
          else if(strcmp(buf, "header") == 0)
              ctx.lst = busHeader;
          else if(strcmp(buf, "footer") == 0)
              ctx.lst = busFooter;
          VERIFY(ctx.lst < 0xFF);
          break;
      case bmlTagRow:
          break;
      case bmlTagFld:
          ctx.fld = 0xFF;
          if(strcmp(buf, "maxOpr") == 0)
              ctx.fld = busMaxOpr;
          else if(strcmp(buf, "autoClose") == 0)
              ctx.fld = busAutoClose;
          else if(strcmp(buf, "site") == 0)
              ctx.fld = busSite;
          else if(strcmp(buf, "location") == 0)
              ctx.fld = busLocation;
          else if(strcmp(buf, "font") == 0)
              ctx.fld = busFont;
          else if(strcmp(buf, "line") == 0)
              ctx.fld = busLine;
          VERIFY(ctx.fld < 0xFF);
          break;
      default:
          ctx.tag = 0;
          break;
    }
    VERIFY(ctx.tag);
}

static void treatVal(const char *buf) {
    VERIFY(buf);
    VERIFY(strlen(buf) <= prtW);
    switch (ctx.fld) {
      case busMaxOpr:
          strcpy(data.maxOpr, buf);
          break;
      case busAutoClose:
          strcpy(data.autoClose, buf);
          break;
      case busSite:
          strcpy(data.site, buf);
          break;
      case busLocation:
          strcpy(data.location, buf);
          break;
      case busFont:
          switch (ctx.lst) {
            case busHeader:
                VERIFY(ctx.row < 10);
                strcpy(data.headerfont[ctx.row], buf);
                break;
            case busFooter:
                VERIFY(ctx.row < 10);
                strcpy(data.footerfont[ctx.row], buf);
                break;
            case busCfg:       //impossible
            default:
                buf = 0;
                break;
          }
          break;
      case busLine:
          switch (ctx.lst) {
            case busHeader:
                VERIFY(ctx.row < 10);
                strcpy(data.headerline[ctx.row], buf);
                break;
            case busFooter:
                VERIFY(ctx.row < 10);
                strcpy(data.footerline[ctx.row], buf);
                break;
            case busCfg:       //impossible
            default:
                buf = 0;
                break;
          }
          break;
      default:
          buf = 0;
          break;
    }
    VERIFY(buf);
}

static int bmlHdlAtrBUS(byte atr, const char *buf) {
    int ret;
    const char *ptr;
    char tmp[prtW + 1];

    switch (atr) {
      case bmlAtrId:
          treatId(buf);
          ptr = "id";
          break;
      case bmlAtrVal:
          treatVal(buf);
          ptr = "val";
          break;
      default:
          ptr = "unexpected atr";
          break;
    }
    sprintf(tmp, "atr %s= [%s]", ptr, buf);
    ret = prtS(tmp);
    return ret;
}

void tcab0210(void) {
    int ret;
    byte dat[4096];
    tBuffer buf;
    tStream stm;
    tVmtBml vmt;
    char tmp[64 + 1];

    trcS("tcab0210 Beg\n");

    nvmStart();
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 4096);

    dspLS(0, "ABC make...");
    ret = stmInit(&stm, &buf, 0, 'w');
    CHECK(ret >= 0, lblKO);

    ret = makeABC(&stm);
    CHECK(ret >= 0, lblKO);

    ret = stmClose(&stm);
    CHECK(ret >= 0, lblKO);

    trcS("testABC: buf= ");
    trcBAN(bufPtr(&buf), bufLen(&buf));
    trcS("\n");

    dspLS(1, "ABC parse...");
    ret = stmInit(&stm, &buf, 0, 'r');
    CHECK(ret >= 0, lblKO);

    vmt.bmlHdlHdr = 0;          //no parsing for header
    vmt.bmlHdlTag = bmlHdlTagABC;
    vmt.bmlHdlAtr = bmlHdlAtrABC;
    prtS("=== ABC ===");
    ret = bmlParse(&stm, &vmt);
    CHECK(ret >= 0, lblKO);

    ret = stmClose(&stm);
    CHECK(ret >= 0, lblKO);

    dspLS(2, "ABC Done");
    tmrPause(1);

    dspClear();
    dspLS(0, "BUS make...");
    bufReset(&buf);
    nvmFileName(tmp, 1);
    ret = stmInit(&stm, &buf, tmp, 'w');
    CHECK(ret >= 0, lblKO);

    ret = makeBUS(&stm);
    CHECK(ret >= 0, lblKO);

    ret = stmClose(&stm);
    CHECK(ret >= 0, lblKO);

    trcS("testBUS: buf= ");
    trcBAN(bufPtr(&buf), bufLen(&buf));
    trcS("\n");

    dspLS(1, "BUS parse...");
    ret = stmInit(&stm, &buf, tmp, 'r');
    CHECK(ret >= 0, lblKO);

    prtS("=== BUS ===");
    vmt.bmlHdlHdr = 0;          //no parsing for header
    vmt.bmlHdlTag = bmlHdlTagBUS;
    vmt.bmlHdlAtr = bmlHdlAtrBUS;
    memset(&ctx, 0, sizeof(ctx));
    ret = bmlParse(&stm, &vmt);
    CHECK(ret >= 0, lblKO);

    ret = stmClose(&stm);
    CHECK(ret >= 0, lblKO);

    prtBusData();

    dspLS(2, "BUS Done");

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();
    trcS("tcab0210 End\n");
}
