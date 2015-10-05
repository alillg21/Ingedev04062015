#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0016.
//Parsing response.

#define CHK CHECK(ret>=0,lblKO)

#define MAXMERNO     6
#define MAXCARDTYPE  8
static int idx = 0;

/***********************************************************************************/
static int prepare01(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "00", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "3030303530303430", 0);
    CHK;                        //TermianlID
    ret = bufApp(rsp, (byte *) "393939393939393939202020", 0);
    CHK;                        //MerchantID1
    ret = bufApp(rsp, (byte *) "393939393939393939202020", 0);
    CHK;                        //MerchantID2
    ret = bufApp(rsp, (byte *) "393939393939393939202020", 0);
    CHK;                        //MerchantID3
    ret = bufApp(rsp, (byte *) "202020202020202020202020", 0);
    CHK;                        //MerchantID4
    ret = bufApp(rsp, (byte *) "202020202020202020202020", 0);
    CHK;                        //MerchantID5
    ret = bufApp(rsp, (byte *) "383030303230303031202020", 0);
    CHK;                        //MerchantID6
    ret = bufApp(rsp, (byte *) "30303031", 0);
    CHK;                        //StoreNumber
    ret = bufApp(rsp, (byte *) "30303031", 0);
    CHK;                        //TerminalNo
    ret = bufApp(rsp, (byte *) "33353031", 0);
    CHK;                        //MerCategory
    //ret= bufApp(rsp,(byte *)"30303030",0); CHK; //MerCategory
    ret =
        bufApp(rsp,
               (byte *) "4E42414420434152442043454E545245205448414C45532020",
               0);
    CHK;                        //MerName
    ret = bufApp(rsp, (byte *) "502E42", 0);
    CHK;                        //MerAdd1Part1

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare02(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "01", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "2E204E4F2020342020202020202020202020202020", 0);
    CHK;                        //Merchant Add1
    ret =
        bufApp(rsp, (byte *) "414255204448414249202020202020202020202020202020",
               0);
    CHK;                        //Merchant Add2
    ret =
        bufApp(rsp, (byte *) "554145202020202020202020202020202020202020202020",
               0);
    CHK;                        //Merchant Add3
    ret = bufApp(rsp, (byte *) "30303030", 0);
    CHK;                        //Merchant Location
    ret = bufApp(rsp, (byte *) "20204142552044484142492020", 0);
    CHK;                        //Merchant City
    ret = bufApp(rsp, (byte *) "4145", 0);
    CHK;                        //Merchant State
    ret = bufApp(rsp, (byte *) "3132333435", 0);
    CHK;                        //Merchant Zip Code
    ret = bufApp(rsp, (byte *) "5050505020", 0);
    CHK;                        //Merchant Security Code
    ret = bufApp(rsp, (byte *) "34303031343837202020202020202020", 0);
    CHK;                        //host 1 Primary Tel
    ret = bufApp(rsp, (byte *) "343030313438", 0);
    CHK;                        //host 1 Alt Tel
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare03(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "02", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "37202020202020202020", 0);
    CHK;                        //host 1 Alt Tel
    ret = bufApp(rsp, (byte *) "34303031343837202020202020202020", 0);
    CHK;                        //host 2 pri Tel
    ret = bufApp(rsp, (byte *) "34303031343837202020202020202020", 0);
    CHK;                        //host 2 Alt Tel
    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //host 3 Primary Tel
    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //host 3 Alt Tel
    //ret= bufApp(rsp,(byte *)"35373339333734202020202020202020",0); CHK; //host 2 pri Tel
    //ret= bufApp(rsp,(byte *)"35373339333734202020202020202020",0); CHK; //host 2 pri Tel
    //ret= bufApp(rsp,(byte *)"35373234393031202020202020202020",0); CHK; //host 2 pri Tel
    //ret= bufApp(rsp,(byte *)"35373234393031202020202020202020",0); CHK; //host 2 pri Tel

    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //host 4 Primary Tel
    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //host 4 Alt Tel

    //ret= bufApp(rsp,(byte *)"34303031343837202020202020202020",0); CHK; //host 4 Primary Tel
    //ret= bufApp(rsp,(byte *)"34303031343837202020202020202020",0); CHK; //host 4 Alt Tel
    ret = bufApp(rsp, (byte *) "3430303134383720202020202020", 0);
    CHK;                        //host 5 Primary Tel

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare04(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "03", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "2020", 0);
    CHK;                        //host 5 primary
    ret = bufApp(rsp, (byte *) "34303031343837202020202020202020", 0);
    CHK;                        //host 5 Sec Tel
//      ret= bufApp(rsp,(byte *)"34303031343837202020202020202020",0); CHK; //host 6 Primary Tel
    //  ret= bufApp(rsp,(byte *)"34303031343837202020202020202020",0); CHK; //host 6 Alt Tel
    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //host 6 Primary Tel
    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //host 6 Alt Tel

    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //MDC Primary Tel 
    ret = bufApp(rsp, (byte *) "35373339333734202020202020202020", 0);
    CHK;                        //MDC Sec  Tel 
    ret = bufApp(rsp, (byte *) "54303031343837202020202020202020", 0);
    CHK;                        //Alt MDC Primary 
    ret = bufApp(rsp, (byte *) "55303031343837202020202020202020", 0);
    CHK;                        //Alt MDC Sec 
    ret = bufApp(rsp, (byte *) "573030313438", 0);
    CHK;                        //Param pri Tel part1

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare05(tBuffer * rsp) {
    int ret;

    //new

    ret = bufApp(rsp, (byte *) "04", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "32342020202020202020", 0);
    CHK;                        //Param pri Tel Part2
    ret = bufApp(rsp, (byte *) "38303031343837202020202020202020", 0);
    CHK;                        //Param Sec Tel
    ret = bufApp(rsp, (byte *) "39383030313438372020202020202020", 0);
    CHK;                        //XBul Pri Tel     
    ret = bufApp(rsp, (byte *) "40303031343837202020202020202020", 0);
    CHK;                        //XBul Sec Tel
    ret = bufApp(rsp, (byte *) "31303031343837202020202020202020", 0);
    CHK;                        //Referal Te
    ret = bufApp(rsp, (byte *) "07", 0);
    CHK;                        //XBul DAy        
    ret = bufApp(rsp, (byte *) "50", 0);
    CHK;                        //Def Txn
    ret = bufApp(rsp, (byte *) "9F1F0000", 0);
    CHK;                        //Txn Type//9F1F0000
    ret = bufApp(rsp, (byte *) "00000000", 0);
    CHK;                        //Txn Pass
    ret =
        bufApp(rsp,
               (byte *)
               "64000000C80000002c01000084030000E803000090010000F401000058020000",
               0);
    CHK;                        //Floor Limit
    ret = bufApp(rsp, (byte *) "0C0A", 0);
    CHK;                        //MDC Time
    ret = bufApp(rsp, (byte *) "0C28", 0);
    CHK;                        //Bul Time
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare06(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "05", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "0C29", 0);
    CHK;                        //Daily Time
    ret = bufApp(rsp, (byte *) "4D", 0);
    CHK;                        //Comm Yype
    ret = bufApp(rsp, (byte *) "54", 0);
    CHK;                        //Dial Type
    ret = bufApp(rsp, (byte *) "00", 0);
    CHK;                        //Wait For Dial
    ret = bufApp(rsp, (byte *) "00", 0);
    CHK;                        //PAbX Ok
    ret = bufApp(rsp, (byte *) "0900", 0);
    CHK;                        //PabX No
    ret = bufApp(rsp, (byte *) "2C01", 0);
    CHK;                        //Timer A
    ret = bufApp(rsp, (byte *) "B80B", 0);
    CHK;                        //Timer C
    ret = bufApp(rsp, (byte *) "31", 0);
    CHK;                        //POS OK
    ret = bufApp(rsp, (byte *) "02", 0);
    CHK;                        //Printer Status
    ret = bufApp(rsp, (byte *) "00", 0);
    CHK;                        //Pin Pad Status
    ret = bufApp(rsp, (byte *) "00", 0);
    CHK;                        //BarCode Status
    ret = bufApp(rsp, (byte *) "18", 0);
    CHK;                        //Misc Flag
    ret = bufApp(rsp, (byte *) "03", 0);
    CHK;                        //BF
    ret = bufApp(rsp, (byte *) "373539363432", 0);
    CHK;                        //User Passwprd
    ret = bufApp(rsp, (byte *) "9F860100", 0);
    CHK;                        //Key Entry Limit
    ret = bufApp(rsp, (byte *) "09", 0);
    CHK;                        //Max Amount Digit        
    ret = bufSet(rsp, '0', 122);
    CHK;                        //Slip Msg?????????????
    ret = bufSet(rsp, '0', 18);
    CHK;                        //Password ?????????????
    ret = bufApp(rsp, (byte *) "09", 0);
    CHK;                        //Max Consequitive Txn
    ret = bufApp(rsp, (byte *) "09", 0);
    CHK;                        //Max txn
    ret = bufApp(rsp, (byte *) "6805010A14", 0);
    CHK;                        //Dl Time 
    ret = bufApp(rsp, (byte *) "3132333435363720202020202020", 0);
    CHK;                        //DL Pri Tel part1      
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare17(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "06", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "2020", 0);
    CHK;                        //Dl pri Tel Part2

    ret = bufApp(rsp, (byte *) "36393820202020202020202020202020", 0);
    CHK;                        //Dl Sec Tel 
    ret = bufApp(rsp, (byte *) "20202020202020202020", 0);
    CHK;                        //Site No Not Used
    ret = bufApp(rsp, (byte *) "2020", 0);
    CHK;                        //Term No Not Used
    ret = bufApp(rsp, (byte *) "39303030", 0);
    CHK;                        //User No Not Used
    ret = bufApp(rsp, (byte *) "22", 0);
    CHK;                        // ask N digin of PAN
    ret = bufApp(rsp, (byte *) "202020202020", 0);
    CHK;                        //Pinpad serial No
    ret = bufApp(rsp, (byte *) "01", 0);
    CHK;                        //Ask CVV
    ret = bufApp(rsp, (byte *) "6300", 0);
    CHK;                        //Tip Percent
    ret = bufApp(rsp, (byte *) "FFFF0000", 0);
    CHK;                        //Threshold value
    ret = bufApp(rsp, (byte *) "0E00", 0);
    CHK;                        //Threshold Duration(day)
    ret = bufApp(rsp, (byte *) "0900", 0);
    CHK;                        //# Of transaction
    ret =
        bufApp(rsp, (byte *) "4E6F6F736896E22020202020202020202020202020202020",
               0);
    CHK;                        //first copy 1
    ret =
        bufApp(rsp, (byte *) "666572657368746520202020202020202020202020202020",
               0);
    CHK;                        //first copy 2
    ret = bufApp(rsp, (byte *) "6C6F746664617220202020202020202020202020", 0);
    CHK;                        //Second copy 1
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepare18(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "07", 0);
    CHK;                        //Block No
    ret = bufApp(rsp, (byte *) "20202020", 0);
    CHK;                        //Second copy 1 part 2
    ret =
        bufApp(rsp, (byte *) "7A6162686968746520202020202020202020202020202020",
               0);
    CHK;                        //Second copy 2
    ret = bufApp(rsp, (byte *) "393939393939393939202020", 0);
    CHK;                        //Merchant ID
    ret =
        bufApp(rsp,
               (byte *)
               "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
               0);
    CHK;                        //Reserved    
    ret = bufApp(rsp, (byte *) "6903100A1F1003", 0);
    CHK;                        //Cur Time
    ret = bufApp(rsp, (byte *) "E59C831FC0B95B30E59C831FC0B95B30", 0);
    CHK;                        //PinBlock Key 
    ret = bufApp(rsp, (byte *) "1388f500", 0);
    CHK;                        //Block Checksum 
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepareHeader(tBuffer * rsp) {
    int ret;

    ret = bufApp(rsp, (byte *) "50415353574F5244", 0);
    CHK;                        //password expression
    ret = bufApp(rsp, (byte *) "3030303530303430", 0);
    CHK;                        //Terminal ID
    ret = bufApp(rsp, (byte *) "303034", 0);
    CHK;                        //Number of users     
    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int prepareBlock1(tBuffer * rsp) {
    int ret;

//      ret= bufApp(rsp,(byte *)"00",0); CHK; //BlockNo
    ret = bufApp(rsp, (byte *) "416D6972202020202020", 0);
    CHK;                        //OperName1
    ret = bufApp(rsp, (byte *) "343536202020", 0);
    CHK;                        //OperPass1
    ret = bufApp(rsp, (byte *) "FFFFFFFF", 0);
    CHK;                        //OperPer1

    ret = bufApp(rsp, (byte *) "496D72616E2020202020", 0);
    CHK;                        //OperName2
    ret = bufApp(rsp, (byte *) "613162202020", 0);
    CHK;                        //OperPass2
    ret = bufApp(rsp, (byte *) "83000000", 0);
    CHK;                        //OperPer2

    ret = bufApp(rsp, (byte *) "496c6972202020202020", 0);
    CHK;                        //OperName1
    ret = bufApp(rsp, (byte *) "373839202020", 0);
    CHK;                        //OperPass1
    ret = bufApp(rsp, (byte *) "05000100", 0);
    CHK;                        //OperPer1

    ret = bufApp(rsp, (byte *) "416D6972202020202020", 0);
    CHK;                        //OperName1
    ret = bufApp(rsp, (byte *) "343434202020", 0);
    CHK;                        //OperPass1
    ret = bufApp(rsp, (byte *) "05000100", 0);
    CHK;                        //OperPer1

    ret =
        bufApp(rsp,
               (byte *)
               "00000000000000000000000000000000000000000000000000000000000000000000000000000000",
               0);
    CHK;                        //FillZeroField

    return bufLen(rsp);

  lblKO:
    trcErr(ret);
    return -1;

}

/***********************************************************************************/
static int printRspDnl01(void) {
    int ret;

    //int idx;
//      byte num;
    char buf[256];

    ret = prtS("Block No:");
    CHK;
    ret = prtS("00");
    CHK;

    ret = prtS("TerminalID:");
    CHK;
    ret = mapGet(appTID, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    for (idx = 1; idx <= MAXMERNO; idx++) {
        ret = mapMove(acqBeg + 1, (word) idx);
        //ret= mapGetByte(acqId,num); CHECK(ret>0,lblKO);
        //if(!num) break;

        strcpy(buf, "idx=");
        num2dec(buf + 4, idx, 3);
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
    prtS("erer");               //CHK;       
    ret = prtS(buf);            //CHK;

    ret = prtS("MerchantName:");
    CHK;
    ret = mapGet(appMerName, buf, 256);
    prtS("erer");
    CHK;
    ret = prtS(buf);            //CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl02(void) {
    int ret;
    char buf[256];

    //int idx;

    ret = prtS("Block No:");
    CHK;
    ret = prtS("01");           //CHK;

    ret = prtS("MercahantAdd1:");
    CHK;
    ret = mapGet(appMerchantAdd1, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("MercahantAdd2:");
    CHK;
    ret = mapGet(appMerchantAdd2, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    ret = prtS("MercahantAdd3:");
    CHK;
    ret = mapGet(appMerchantAdd3, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("MercahantLocationCode:");
    CHK;
    ret = mapGet(appMerLocationCode, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("MercahantCityName:");
    CHK;
    ret = mapGet(appMerCityName, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("MercahantStateCode:");
    CHK;
    ret = mapGet(appMerStateCode, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    ret = prtS("MercahantZipCode:");
    CHK;
    ret = mapGet(appMerZipCode, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    idx = 1;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); // CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl03(void) {
    int ret;
    char buf[256];

    //int idx;

    ret = prtS("Block No:");    // CHK;            
    ret = prtS("02");           //CHK;

    ret = prtS("acqSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    //host 2
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); //CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    ret = prtS("acqSecPhone:"); // CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;
    //host 3
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); // CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("acqSecPhone:"); //CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    //host 4
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); //CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("acqSecPhone:"); // CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

/*      
        for(idx=1 ; idx<=4; idx++){//2 - 4 host

        ret= mapMove(acqBeg+1,(word)idx);        
        
                strcpy(buf,"idx=");
        num2dec(buf+4,idx,3);
        strcat(buf,":");
        ret= prtS(buf);CHECK(ret>=0,lblKO);
    
                if(idx == MAXMERNO+1)
                        {ret= prtS("MDCPriPhone:"); CHK;}
                else
                        ret= prtS("acqPriPhone:"); CHK;
                ret= mapGet(acqPriPhone,buf,256); CHK;  
                ret= prtS(buf); CHK;

                if(idx == MAXMERNO+1)
                        {ret= prtS("MDCSecPhone:"); CHK;}
                else
                        ret= prtS("acqSecPhone:"); CHK;
                ret= mapGet(acqSecPhone,buf,256); CHK;  
                ret= prtS(buf); CHK;
    }*/

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl04(void) {
    int ret;
    char buf[256];

    ret = prtS("Block No:");
    CHK;
    ret = prtS("03");           //CHK;

    //HOST 5
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); // CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    ret = prtS("acqSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    //HOST 6
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); // CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("acqSecPhone:"); // CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;
    //MDC1
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); // CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("acqSecPhone:"); //CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    //MDC2
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); //CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    ret = prtS("acqSecPhone:"); //CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl05(void) {
    int ret;
    char buf[256];
    card num;
    int j;

    ret = prtS("Block No:");
    CHK;
    ret = prtS("04");           // CHK;
    //PARAM 
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:");
    CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("acqSecPhone:"); //CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

//Bul
    idx++;
    strcpy(buf, "idx=");
    num2dec(buf + 4, idx, 3);
    strcat(buf, ":");
    ret = prtS(buf);            //CHECK(ret>=0,lblKO);
    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("acqPriPhone:"); //CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    ret = prtS("acqSecPhone:"); // CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;
//referal
    for (j = 0; j < MAXMERNO; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(issBeg + 1, (word) j);
        ret = prtS("Referall Phone:");  //CHK;
        ret = mapGet(issRefPhone, buf, 256);
        CHK;
        ret = prtS(buf);        // CHK;
    }

/*      ret= prtS("BulDay:"); CHK;
    ret= mapGet(taskBulDay,buf,256); CHK;               
    ret= prtS(buf); CHK;
*/
    ret = prtS("defTxn:");      //CHK;
    ret = mapGetByte(appDefTxnTyp, buf[0]);
    CHK;
    buf[1] = 0;
    ret = prtS(buf);            //CHK;

    ret = prtS("Terminal Supported Txn:");  // CHK;
    ret = mapGetCard(appTPmis, num);
    CHK;
    num2hex(buf, num, 8);
    ret = prtS(buf);            // CHK;

    ret = prtS("Txn Password Required:");   //CHK;
    ret = mapGetCard(appPassReq, num);
    CHK;
    num2hex(buf, num, 8);
    ret = prtS(buf);            // CHK;

    for (j = 1; j <= MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j, 3);
        strcat(buf, ":");
        ret = prtS(buf);        //CHECK(ret>=0,lblKO);

        ret = prtS("FloorLimit:");  // CHK;
        mapMove(issBeg + 1, (word) j);
        ret = mapGetCard(issFloorLimit, num);
        CHK;
        num2dec(buf, num, 4);
        ret = prtS(buf);        // CHK;

    }

    ret = prtS("MDC Time:");    //CHK;
    ret = mapMove(taskBeg + 1, taskBatchUpl);
    ret = mapGet(taskTime, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;
/*
        ret= prtS("BulTime:"); CHK;
    ret= mapGet(BulTime,buf,256); CHK;          
    ret= prtS(buf); CHK;*/

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl06(void) {
    int ret;
    char buf[256];
    char bufTemp;
    word num;
    card cnum;
    int j;

    ret = prtS("Block No:");    //CHK;            
    ret = prtS("05");           // CHK;

    ret = prtS("Daily Time:");  // CHK;
    ret = mapMove(taskBeg + 1, taskParamDnl);
    ret = mapGet(taskTime, buf, 256);
    CHK;
    ret = prtS(buf);            //CHK;

    ret = prtS("CommType:");    // CHK;
    ret = mapGetByte(appCommTyp, bufTemp);
    CHK;
    if(bufTemp == 0x00)
        *buf = '0';
    else {
        *buf = bufTemp;

    }
    *(buf + 1) = 0;
    ret = prtS(buf);            // CHK;

    ret = prtS("DialType:");    //CHK;
    ret = mapGetByte(appDialTyp, bufTemp);
    CHK;
    if(bufTemp == 0)
        num2dec(buf, bufTemp, 0);
    else {
        *buf = bufTemp;
        *(buf + 1) = 0;
    }
    ret = prtS(buf);            //CHK;

    ret = prtS("WaitForDialTone:"); //CHK;
    ret = mapGetByte(appWaitForDialTone, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 2);
    ret = prtS(buf);            //CHK;

    ret = prtS("PABXStatus:");  //CHK;
    ret = mapGetByte(appPABXStatus, bufTemp);
    CHK;
    if(bufTemp == 0x00)
        *buf = '0';
    else
        num2dec(buf, bufTemp, 1);

    *(buf + 1) = 0;
    ret = prtS(buf);            // CHK;    

    ret = prtS("PABXNo:");      //CHK;
    ret = mapGetWord(appPABXNo, num);
    CHK;
    num2dec(buf, num, 2);
    ret = prtS(buf);            //CHK;

    ret = prtS("TimerA:");      // CHK;
    ret = mapGetWord(appTimerA, num);
    CHK;
    num2dec(buf, num, 0);

    ret = prtS(buf);            //CHK;

    ret = prtS("TimerC:");      // CHK;
    ret = mapGetWord(appTimerC, num);
    CHK;
    num2dec(buf, num, 0);

    ret = prtS(buf);            // CHK;

    ret = prtS("PosStatus:");   //CHK;
    ret = mapGetByte(appPosStatus, bufTemp);
    CHK;
    if(bufTemp == 0x00)
        *buf = '0';
    else
        *buf = bufTemp;
    *(buf + 1) = 0;
    ret = prtS(buf);            //CHK;

    ret = prtS("PrinterStatus:");   // CHK;
    ret = mapGetByte(appPrnStatus, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 0);
    ret = prtS(buf);            //CHK;

    ret = prtS("PinPadStatus:");    //CHK;
    ret = mapGetByte(appPinPadStatus, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 0);
    ret = prtS(buf);            //CHK;

    ret = prtS("BarCodeStatus:");   //CHK;
    ret = mapGetByte(appBarCodeRdrStatus, bufTemp);
    CHK;
    num2dec(buf, bufTemp, 0);
    ret = prtS(buf);            //CHK;

    ret = prtS("MiscFlag:");    //CHK;
    ret = mapGetByte(appMisFlag, bufTemp);
    CHK;
    num2hex(buf, bufTemp, 0);
    ret = prtS(buf);            //CHK;

    ret = prtS("Blockin Factor:");  // CHK;
    ret = mapGetWord(appBchBlcNum, num);
    CHK;
    num2hex(buf, num, 0);
    ret = prtS(buf);            //CHK;

    ret = prtS("Supervisor Password:"); //CHK;
    ret = mapGet(appSprPass, buf, 256);
    CHK;
    ret = prtS(buf);            // CHK;

    for (j = 0; j < dimTxnType; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(tPerBeg + 1, (word) j);
        ret = prtS("KeyEntrLmt:");  // CHK;
        ret = mapGetCard(tPerKeyEntryLimit, cnum);
        CHK;
        num2dec(buf, cnum, 0);
        cnum = 0;
        ret = prtS(buf);        // CHK;
    }

    for (j = 0; j < MAXCARDTYPE; j++) {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j + 1, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(tPerBeg + 1, (word) j);
        ret = prtS("MaxAmountDigit:");  // CHK;
        ret = mapGetByte(tPerMaxAmountDigits, bufTemp); // CHK;              
        num2hex(buf, bufTemp, 0);
        ret = prtS(buf);        //CHK;
    }

    ret = prtS("appSlogan:");   // CHK;
    ret = mapGet(appSlogan, buf, 256);  // CHK;                
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
        ret = mapGetByte(issMaxConsTxns, bufTemp);  //CHK;   
        num2dec(buf, bufTemp, 0);
        num = 0;
        ret = prtS(buf);        // CHK;

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

    ret = prtS("Download Time:");
    CHK;
    ret = mapMove(taskBeg + 1, taskProgDnl);
    ret = mapGet(taskTime, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    ret = prtS("Download Date:");
    CHK;
    ret = mapGet(taskDate, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    strcpy(buf, "idx=");
    num2dec(buf + 4, idx + 1, 3);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/

static int printRspDnl17(void) {
    int ret;
    int j;
    char buf[256];
    byte bufTemp[256];
    word num;
    card bCard;

    ret = prtS("Block No:");
    CHK;
    ret = prtS("06");
    CHK;

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("DownloadSecPhone:");
    CHK;
    ret = mapGet(acqPriPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    idx++;

    ret = mapMove(acqBeg + 1, (word) idx);
    ret = prtS("DownloadSecPhone:");
    CHK;
    ret = mapGet(acqSecPhone, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;
    idx++;

    for (j = 0; j < dimIss; j++)    //Now is 8 but it depends on txn numbers
    {
        strcpy(buf, "idx=");
        num2dec(buf + 4, j, 3);
        strcat(buf, ":");
        ret = prtS(buf);
        CHECK(ret >= 0, lblKO);

        mapMove(issBeg + 1, (word) j);
        ret = prtS("LstNDigit:");
        CHK;
        ret = mapGetByte(issAskLastNdigit, bufTemp);
        CHK;
        num2hex(buf, bufTemp[0], 1);
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
        ret = mapGetByte(tPerAskCVV2, bufTemp);
        CHK;
        num2hex(buf, bufTemp[0], 1);
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

    //appThreshHoldValue    
    ret = mapGetCard(appThreshHoldValue, bCard);
    CHK;
    ret = num2dec(buf, bCard, 0);
    CHK;
    printS("SecondTrailorSlogan:", buf);
    CHK;

    //appThresholdDuration    
    ret = mapGetCard(appThresholdDuration, num);
    CHK;
    ret = num2hex(buf, num, 2 * sizeof(word));
    printS("SecondTrailorSlogan:", buf);
    CHK;

    //appThresholdNumberTransaction    
    ret = mapGetCard(appThresholdNumberTransaction, num);
    CHK;
    ret = num2hex(buf, num, 2 * sizeof(word));
    printS("SecondTrailorSlogan:", buf);
    CHK;

    //appCustSlogan1    
    ret = mapGet(appFirstHeaderSlogan, buf, 256);
    CHK;
    printS("FirstHeaderSlogan:", buf);
    CHK;

    //appCustSlogan2    
    ret = mapGet(appFirstTrailorSlogan, buf, 256);
    CHK;
    printS("FirstTrailorSlogan:", buf);
    CHK;

    //appMerSlogan1    
    ret = mapGet(appSecondHeaderSlogan, buf, 256);
    CHK;
    printS("SecondHeaderSlogan:", buf);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl18(void) {
    int ret;
    char buf[256];
    byte bufTemp[256];

    ret = prtS("Block No:");
    CHK;
    ret = prtS("08");
    CHK;

//appMerSlogan2    
    ret = mapGet(appSecondTrailorSlogan, buf, 256);
    CHK;
    printS("SecondTrailorSlogan:", buf);
    CHK;

    //appTerminalMerID    
    ret = mapGet(appTerminalMerID, buf, 256);
    CHK;
    printS("TerminalMerID:", buf);
    CHK;

    ret = prtS("PINPadWorKey:");
    CHK;
    ret = mapGet(appPinPadWorkKey, bufTemp, 256);
    CHK;
    bin2hex(buf, bufTemp, 16);
    ret = prtS(buf);
    CHK;

    ret = prtS("CurrentDateTime:");
    CHK;
    ret = mapGet(appCurrentDateTime, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl07(void) {
    int ret;
    char buf[256];

    ret = prtS("Terminal ID:");
    CHK;
    ret = mapGet(glbTermID, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    ret = prtS("Number Of users:");
    CHK;
    //ret= mapGet(regNoUserPermission,buf,256); CHK;    
    ret = mapGet(glbNoUserPermission, buf, 256);
    CHK;
    ret = prtS(buf);
    CHK;

    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
static int printRspDnl08(void) {
    int ret;
    char buf[256];
    card crd;
    int i = 0;

    for (i = 0; i < 4; i++) {
        mapMove(operBeg + 1, (word) i);
        ret = prtS("OperName:");
        CHK;
        ret = mapGet(operName, buf, 256);
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
        ret = mapGetCard(operPermissions, crd);
        CHK;
        num2hex(buf, crd, 2 * sizeof(card));
        ret = prtS(buf);
        CHK;

    }
    return 1;
  lblKO:
    trcErr(ret);
    return -1;
}

/***********************************************************************************/
void tcnl0016(void) {
    int ret;
    byte dRsp[256];
    tBuffer bRsp;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "prepare01...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);

    ret = prepare01(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse01...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl01(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl01();
    CHECK(ret >= 0, lblKO);

    //Block 2       
    ret = dspLS(0, "prepare02...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare02(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse02...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl02(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl02();
    CHECK(ret >= 0, lblKO);

    //Block 3
    ret = dspLS(0, "prepare03...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare03(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse03...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl03(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl03();
    CHECK(ret >= 0, lblKO);

    //Block 4
    ret = dspLS(0, "prepare04...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare04(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse04...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl04(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl04();
    CHECK(ret >= 0, lblKO);

    //Block 5
    ret = dspLS(0, "prepare05...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare05(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse05...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl05(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl05();
    CHECK(ret >= 0, lblKO);

    //Block 6
    ret = dspLS(0, "prepare06...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare06(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse06...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl06(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl06();
    CHECK(ret >= 0, lblKO);

    //Block 7
    ret = dspLS(0, "prepare07...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare17(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse07...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl17(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl17();
    CHECK(ret >= 0, lblKO);

    //Block 8
    ret = dspLS(0, "prepare08...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepare18(&bRsp);
    CHECK(ret >= 0, lblKO);
    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse08...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl18(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    ret = printRspDnl18();
    CHECK(ret >= 0, lblKO);

    //User Permission
    ret = dspLS(0, "prepareHeaderOperator...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepareHeader(&bRsp);
    CHECK(ret >= 0, lblKO);

    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "Header...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl07(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);
    ret = printRspDnl07();
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "prepareBlockOperator...");
    CHECK(ret >= 0, lblKO);
    bufInit(&bRsp, dRsp, 256);
    ret = prepareBlock1(&bRsp);
    CHECK(ret >= 0, lblKO);

    ret = hex2bin(bRsp.ptr, (char *) bRsp.ptr, (byte) ret);

    ret = dspLS(1, "parse01...");
    CHECK(ret >= 0, lblKO);
    ret = rspDnl08(bufPtr(&bRsp), (byte) (bufLen(&bRsp) / 2));
    CHECK(ret >= 0, lblKO);

    mapPutByte(regDnlPerformed, 1);
    ret = printRspDnl08();
    CHECK(ret >= 0, lblKO);

    ret = loadWorkKey();

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrStop(1);
    tmrPause(3);
}
#endif
