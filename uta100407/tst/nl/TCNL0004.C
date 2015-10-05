#include <string.h>
//#include <stdio.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tccnl0004
//Online call testing.
static int idx = 0;

#define MAXMERNO     6
#define MAXCARDTYPE  8
#define CHK CHECK(ret>=0,lblKO)

int printDataParamDnl(void) {
    int ret;
    char buf[256];
    word j;
    word num;
    card cnum;
    char bufTemp;
    char Temp[256];
    card DmsBinCnt;
    card RngBinCnt;
    card UserCnt;

//      char CardType= 0; //AB stub
    char IssRow;
    word wrd;
    char chr;

    ret = prtS("TerminalID:");
    CHK;
    ret = mapGet(appTID, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    for (idx = 0; idx < MAXMERNO; idx++) {
        ret = mapMove(acqBeg + 1, (word) idx);

        strcpy(buf, "idx=");
        num2dec(buf + 4, idx + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        ret = prtS("MerchantID:");
        CHK;
        ret = mapGet(acqMerchantNo, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;

        strcpy(buf, "\n");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);
    }
    ret = prtS("StoreNo:");
    CHK;
    ret = mapGet(appStoreNo, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("TerminalNo:");
    CHK;
    ret = mapGet(appTermNo, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MerchantCategory:");
    CHK;
    ret = mapGet(appMerCategoryCode, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MerchantName:");
    CHK;
    ret = mapGet(appMerName, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("Block No:");
    CHK;
    ret = prtS("01");
    CHK;

    ret = prtS("MercahantAdd1:");
    CHK;
    ret = mapGet(appMerchantAdd1, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MercahantAdd2:");
    CHK;
    ret = mapGet(appMerchantAdd2, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MercahantAdd3:");
    CHK;
    ret = mapGet(appMerchantAdd3, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MercahantLocationCode:");
    CHK;
    ret = mapGet(appMerLocationCode, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MercahantCityName:");
    CHK;
    ret = mapGet(appMerCityName, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MercahantStateCode:");
    CHK;
    ret = mapGet(appMerStateCode, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("MercahantZipCode:");
    CHK;
    ret = mapGet(appMerZipCode, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    idx = 0;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx + 1, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:");
    CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("acqSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx + 1, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS("Block No:");
    CHK;
    ret = prtS("02");
    CHK;

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:");
    CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("acqSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    idx++;
    for (idx = idx; idx < MAXMERNO + 1; idx++) {    //2 More For MDC Pri and Sec

        ret = mapMove(acqBeg + 1, (word) idx);

        strcpy(buf, "idx=");
        num2dec(buf + 4, idx + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        if(idx == MAXMERNO) {
            ret = prtS("MDCPriPhone:");
            CHK;
        } else
            ret = prtS("acqPriPhone:");
        CHK;
        ret = mapGet(acqPriPhone, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;

        if(idx == MAXMERNO) {
            ret = prtS("MDCSecPhone:");
            CHK;
        } else
            ret = prtS("acqSecPhone:");
        CHK;
        ret = mapGet(acqSecPhone, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;
    }

    strcpy(buf, "idx=");
    num2dec(buf + 4, idx + 1, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS("Block No:");
    CHK;
    ret = prtS("03");
    CHK;

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("AltMdcPriPhone:");
    CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("AltMdcSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    idx++;

    for (j = 0; j < 2; j++) {   // Pri and Sec Param Tel And Bul Tel

        ret = mapMove(acqBeg + 1, (word) idx);

        strcpy(buf, "idx=");
        num2dec(buf + 4, idx + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        if(j == 0) {
            ret = prtS("ParamPriPhone:");
            CHK;
        } else
            ret = prtS("BulPriPhone:");
        CHK;
        ret = mapGet(acqPriPhone, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;

        if(j == 0) {
            ret = prtS("ParamSecPhone:");
            CHK;
        } else
            ret = prtS("BulSecPhone:");
        CHK;
        ret = mapGet(acqSecPhone, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;
        idx++;
    }
    for (j = 0; j < MAXMERNO; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(issBeg + 1, (word) j);
        ret = prtS("Referall Phone:");
        CHK;
        ret = mapGet(issRefPhone, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;
    }

    ret = prtS("defTxn:");
    CHK;
    ret = mapGetByte(appDefTxnTyp, buf[0]);
    CHK;
    buf[1] = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("Terminal Supported Txn:");
    CHK;
    ret = mapGetCard(appTPmis, num);
    CHK;
    num2hex(buf, num, 0);
    ret = prtS(buf);
    CHK;

    ret = prtS("Txn Password Required:");
    CHK;
    ret = mapGetCard(appPassReq, num);
    CHK;
    num2hex(buf, num, 0);
    ret = prtS(buf);
    CHK;

    for (j = 0; j < MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        ret = prtS("FloorLimit:");
        CHK;
        mapMove(issBeg + 1, (word) j);
        ret = mapGetCard(issFloorLimit, num);
        CHK;
        num2dec(buf, num, 0);
        ret = prtS(buf);
        CHK;

    }

    /*ret= prtS("MDC Time:"); CHK;
       ret= mapGet(MDC,buf,256); CHK;              
       ret= prtS(buf); CHK;

       ret= prtS("BulTime:"); CHK;
       ret= mapGet(BulTime,buf,256); CHK;          
       ret= prtS(buf); CHK; */

    ret = prtS("Block No:");
    CHK;
    ret = prtS("04");
    CHK;

    /*ret= prtS("Daily Time:"); CHK;
       ret= mapGet(Daily,buf,256); CHK;            
       ret= prtS(buf); CHK; */

    ret = prtS("CommType:");
    CHK;
    ret = mapGetByte(appCommTyp, bufTemp);
    CHK;
    if(bufTemp == 0x00)
        *buf = '0';
    else {
        *buf = bufTemp;

    }
    *(buf + 1) = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("DialType:");
    CHK;
    ret = mapGetByte(appDialTyp, bufTemp);
    CHK;
    if(bufTemp == 0)
        num2dec(buf, bufTemp, 0);
    else {
        *buf = bufTemp;
        *(buf + 1) = 0;
    }
    ret = prtS(buf);
    CHK;

    ret = prtS("WaitForDialTone:");
    CHK;
    ret = mapGetByte(appWaitForDialTone, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 2);
    ret = prtS(buf);
    CHK;

    ret = prtS("PABXStatus:");
    CHK;
    ret = mapGetByte(appPABXStatus, bufTemp);
    CHK;
    if(bufTemp == 0x00)
        *buf = '0';
    else
        *buf = bufTemp;
    *(buf + 1) = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("PABXNo:");
    CHK;
    ret = mapGetWord(appPABXNo, num);
    CHK;
    num2dec(buf, num, 2);
    ret = prtS(buf);
    CHK;

    ret = prtS("TimerA:");
    CHK;
    ret = mapGetWord(appTimerA, num);
    CHK;
    num2dec(buf, num, 0);
    ret = prtS(buf);
    CHK;

    ret = prtS("TimerC:");
    CHK;
    ret = mapGetWord(appTimerC, num);
    CHK;
    num2dec(buf, num, 0);
    ret = prtS(buf);
    CHK;

    ret = prtS("PosStatus:");
    CHK;
    ret = mapGetByte(appPosStatus, bufTemp);
    CHK;
    if(bufTemp == 0x00)
        *buf = '0';
    else
        *buf = bufTemp;
    *(buf + 1) = 0;
    ret = prtS(buf);
    CHK;

    ret = prtS("PrinterStatus:");
    CHK;
    ret = mapGetByte(appPrnStatus, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 2);
    ret = prtS(buf);
    CHK;

    ret = prtS("PinPadStatus:");
    CHK;
    ret = mapGetByte(appPinPadStatus, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 2);
    ret = prtS(buf);
    CHK;

    ret = prtS("BarCodeStatus:");
    CHK;
    ret = mapGetByte(appBarCodeRdrStatus, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 2);
    ret = prtS(buf);
    CHK;

    ret = prtS("MiscFlag:");
    CHK;
    ret = mapGetByte(appMisFlag, bufTemp);
    CHK;
    num2hex(buf, bufTemp, 2);
    ret = prtS(buf);
    CHK;

    ret = prtS("Blockin Factor:");
    CHK;
    ret = mapGetWord(appBchBlcNum, num);
    CHK;
    num2hex(buf, num, 0);
    ret = prtS(buf);
    CHK;

    ret = prtS("Supervisor Password:");
    CHK;
    ret = mapGet(appSprPass, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    for (j = 0; j < MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(tPerBeg + 1, (word) j);
        ret = prtS("KeyEntrLmt:");
        CHK;
        ret = mapGetCard(tPerKeyEntryLimit, cnum);
        CHK;
        num2dec(buf, cnum, 0);
        cnum = 0;
        ret = prtS(buf);
        CHK;
    }

    for (j = 0; j < MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(tPerBeg + 1, (word) j);
        ret = prtS("MaxAmountDigit:");
        CHK;
        ret = mapGetByte(tPerMaxAmountDigits, bufTemp);
        CHK;
        num2hex(buf, bufTemp, 0);
        ret = prtS(buf);
        CHK;
    }

    ret = prtS("appSlogan:");
    CHK;
    ret = mapGet(appSlogan, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    for (j = 0; j < MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(issBeg + 1, (word) j);
        ret = prtS("MaxConTxn:");
        CHK;
        ret = mapGetByte(issMaxConsTxns, bufTemp);
        CHK;
        num2dec(buf, bufTemp, 0);
        num = 0;
        ret = prtS(buf);
        CHK;

    }

    for (j = 0; j < MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(issBeg + 1, (word) j);
        ret = prtS("MaxTxns:");
        CHK;
        ret = mapGetByte(issMaxTxns, bufTemp);
        CHK;
        num2dec(buf, bufTemp, 0);
        num = 0;
        ret = prtS(buf);
        CHK;
    }

/*      
        //ret= prtS("Download Time:"); CHK;
    //ret= mapGet(Daily,buf,256); CHK;          
    //ret= prtS(buf); CHK;
*/

    strcpy(buf, "idx=");
    num2dec(buf + 4, idx + 1, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("DownloadPriPhone:");
    CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    ret = prtS("Block No:");
    CHK;
    ret = prtS("05");
    CHK;

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("DownloadSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    idx++;

    for (j = 0; j < 8; j++)     //Now is 8 but it depends on txn numbers
    {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(tPerBeg + 1, (word) j);
        ret = prtS("LstNDigit:");
        CHK;
        ret = mapGetByte(tPerAskLastNDigit, Temp);
        CHK;
        num2hex(buf, Temp[0], 1);
        ret = prtS(buf);
        CHK;
    }

    ret = prtS("PINPadSerial:");
    CHK;
    ret = mapGet(appPinPadSerialNo, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    for (j = 0; j < 8; j++)     //Now is 8 but it depends on txn numbers
    {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(tPerBeg + 1, (word) j);
        ret = prtS("AskCVV2:");
        CHK;
        ret = mapGetByte(tPerAskCVV2, Temp);
        CHK;
        num2hex(buf, Temp[0], 1);
        ret = prtS(buf);
        CHK;
    }

    ret = prtS("TipPercent:");
    CHK;
    ret = mapGetWord(appTipPer, num);
    CHK;
    num2dec(buf, num, 0);
    ret = prtS(buf);
    CHK;

    //Print Reserved
    //Print CurTime

    ret = prtS("PINPadWorKey:");
    CHK;
    ret = mapGet(appPinPadWorkKey, Temp, 256);
    CHK;
    bin2hex(buf, (byte *) Temp, 8);
    ret = prtS(buf);
    CHK;

    ret = prtS("Number of Operator:");
    CHK;
    ret = mapGet(regNoUserPermission, buf, 256);
    CHK;
    prtS(buf);
    ret = dec2num(&UserCnt, buf, 4);

    //Password Downloaded
    for (j = 0; j < UserCnt; j++)   //Now is 8 but it depends on user numbers
    {

        mapMove(operBeg + 1, (word) j);
        ret = mapGet(operName, buf, 256);
        CHK;

        strcpy(Temp, "idx=");
        num2dec(Temp + 4, j + 1, 3);
        strcat(Temp, ":");
        ret = prtS(Temp);
        CHECK(ret >= 0, lblKO);

        ret = prtS("OperName:");
        CHK;
        ret = prtS(buf);
        CHK;

        ret = prtS("OperPassword:");
        CHK;
        ret = mapGet(operPassword, buf, 256);
        CHK;
        ret = prtS(buf);
        CHK;

        ret = prtS("OperPermissions:");
        CHK;
        ret = mapGetCard(operPermissions, cnum);
        CHK;
        num2hex(buf, cnum, 2 * sizeof(card));
        ret = prtS(buf);
        CHK;

    }

    ret = prtS("DmsBinVer:");
    CHK;
    ret = mapGet(appDmsBinVerNo, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("regDmsBinCnt:");
    CHK;
    ret = mapGet(regDmsBinCnt, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    ret = dec2num(&DmsBinCnt, buf, 4);

    ret = prtS("RngBinVer:");
    CHK;
    ret = mapGet(appRngBinVerNo, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("regRngBinCnt:");
    CHK;
    ret = mapGet(regRngBinCnt, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    ret = dec2num(&RngBinCnt, buf, 4);

    for (idx = 0; idx < (int) DmsBinCnt; idx++) //DmsBinCnt
    {
        strcpy(buf, "idx=");
        num2dec(buf + 4, idx, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        ret = mapMove(dmsBinBeg + 1, (byte) idx);
        CHK;
        ret = prtS("dmsBin:");
        CHK;
        ret = mapGet(dmsBin, buf, 256);
        CHK;

        //ret= sprintf(Temp,"%02d%02d%02d",buf[0],buf[1],buf[2]);
        ret = bin2hex(Temp, (byte *) buf, 3);
        ret = prtS(Temp);

        ret = prtS("Issuer Row:");
        ret = mapGetByte(dmsIssRow, IssRow);
        CHK;
        num2hex(Temp, IssRow, 2 * sizeof(byte));
        ret = prtS(Temp);

        ret = prtS("DomesticPermissions:");
        ret = mapGetWord(dmsPer, wrd);
        CHK;
        num2hex(Temp, wrd, 2 * sizeof(word));
        ret = prtS(Temp);

        //      ret= mapMove(issBeg+1,IssRow);CHK;//map move on dmsIssRow
        prtS("Domestic Acq Row:");
        ret = mapGetByte(dmsAcqRow, chr);
        CHECK(ret > 0, lblKO);
        num2hex(Temp, chr, 2 * sizeof(byte));
        ret = prtS(Temp);

    }

    for (idx = 0; idx < (int) RngBinCnt; idx++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, idx, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        ret = mapMove(binBeg + 1, (byte) idx);
        ret = prtS("LowBin:");
        CHK;
        ret = mapGet(binLowRange, buf, 256);
        CHK;

        //ret= sprintf(Temp,"%02d%02d%02d",buf[0],buf[1],buf[2]);
        bin2hex(Temp, (byte *) buf, lenRangeBorder / 2);
        ret = prtS(Temp);
        CHK;

        ret = prtS("HighBin:");
        CHK;
        ret = mapGet(binHighRange, buf, 256);
        CHK;

        //      ret= sprintf(Temp,"%02d%02d%02d",buf[0],buf[1],buf[2]);
        bin2hex(Temp, (byte *) buf, lenRangeBorder / 2);
        ret = prtS(Temp);
        CHK;

        ret = prtS("Issuer Row:");
        CHK;
        ret = mapGetByte(binIssRow, IssRow);
        CHK;
        num2hex(Temp, IssRow, 2 * sizeof(byte));
        ret = prtS(Temp);
        CHK;

        ret = prtS("RangePermissions:");
        ret = mapGetWord(binPer, wrd);
        CHK;
        num2hex(Temp, wrd, 2 * sizeof(word));
        ret = prtS(Temp);

        //      ret= mapMove(issBeg+1,IssRow);CHK;//map move on binIssRow
        prtS("Bin Acq Row:");
        ret = mapGetByte(binAcqRow, chr);
        CHECK(ret > 0, lblKO);
        num2hex(Temp, chr, 2 * sizeof(byte));
        ret = prtS(Temp);

    }

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcnl0004(void) {
    int ret;
    tBuffer bReq;
    byte dReq[256] = "";

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "prepare01...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bReq, dReq, 256);

    ret = dspLS(1, "onlParamDnl...");
    CHECK(ret >= 0, lblKO);
    ret = onlParamDl(PARAM);
    CHECK(ret >= 0, lblKO);
    //ret= dspLS(0,"printData..."); 

    //save in db
    ret = saveDB(PARAM);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "onlBINDnl...");
    CHECK(ret >= 0, lblKO);
    ret = onlParamDl(BINPARAM);
    CHECK(ret >= 0, lblKO);

    //save in db
    ret = saveDB(BINPARAM);
    CHECK(ret >= 0, lblKO);
    mapPutByte(regDnlPerformed, 1);

    ret = printDataParamDnl();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
}
#endif
