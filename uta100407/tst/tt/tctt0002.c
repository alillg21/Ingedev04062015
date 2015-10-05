#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tctt0002.
//Parsing response of Dalily Download (Domestic & Range BINs)

#define CHK CHECK(ret>=0,lblKO)
static int prepareBlock1(tBuffer *rsp){
    int ret;

        ret= bufApp(rsp,(byte *)"3030303032",0); CHK; //BINVersion
        ret= bufApp(rsp,(byte *)"303033",0); CHK; //NumberOfBIN
        ret= bufApp(rsp,(byte *)"3030333139",0); CHK; //RangeBINVersion
        ret= bufApp(rsp,(byte *)"303032",0); CHK; //NumberOfRangeBIN
        ret= bufApp(rsp,(byte *)"31",0); CHK; //UpdatedParam
        ret= bufApp(rsp,(byte *)"30",0); CHK; //DailyLoopNo
        ret= bufApp(rsp,(byte *)"3030303030",0); CHK; //DailyLoopPos
        ret= bufApp(rsp,(byte *)"03",0); CHK; //FillUnknownField
        ret= bufApp(rsp,(byte *)"20202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020",0); CHK; //FillBlankField

    return bufLen(rsp);
        
lblKO:
    trcErr(ret);
        return -1;

}


static int prepareBlock2(tBuffer *rsp){
    int ret;

//      ret= bufApp(rsp,(byte *)"00",0); CHK; //StartBinNoInBlock
        ret= bufApp(rsp,(byte *)"2F3D49",0); CHK; //Bin1 ;2D3745
        ret= bufApp(rsp,(byte *)"13FF03",0); CHK; //BinInfo1
        ret= bufApp(rsp,(byte *)"311556",0); CHK; //Bin2
        ret= bufApp(rsp,(byte *)"15FD01",0); CHK; //BinInfo2
        ret= bufApp(rsp,(byte *)"3C1060",0); CHK; //Bin3
        ret= bufApp(rsp,(byte *)"13DF02",0); CHK; //BinInfo3
        ret= bufApp(rsp,(byte *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",0); CHK; //FillerFF  
    
    return bufLen(rsp);
        
lblKO:
    trcErr(ret);
        return -1;
}

static int prepareBlock3(tBuffer *rsp){
    int ret;

        ret= bufApp(rsp,(byte *)"40404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040",0); CHK; //FinalBlock
    
    return bufLen(rsp);
        
lblKO:
    trcErr(ret);
        return -1;
}

static int prepareBlock4(tBuffer *rsp){
    int ret;

//      ret= bufApp(rsp,(byte *)"00",0); CHK; //StartBINNoInBlock
        
        ret= bufApp(rsp,(byte *)"2F3D46",0); CHK; //LowBIN1//310C24
        ret= bufApp(rsp,(byte *)"2F4D49",0); CHK; //HighBIN1//310D35
        ret= bufApp(rsp,(byte *)"33",0); CHK; //CardType
        ret= bufApp(rsp,(byte *)"07",0); CHK; //HostID
        ret= bufApp(rsp,(byte *)"FFFF",0); CHK; //TxnTypesFFFF

        ret= bufApp(rsp,(byte *)"361060",0); CHK; //LowBIN2
        ret= bufApp(rsp,(byte *)"3D1060",0); CHK; //HighBIN2
        ret= bufApp(rsp,(byte *)"36",0); CHK; //CardType
        ret= bufApp(rsp,(byte *)"02",0); CHK; //HostID
        ret= bufApp(rsp,(byte *)"0F00",0); CHK; //TxnTypes

        ret= bufApp(rsp,(byte *)"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",0); CHK; //FillZeroField
              
    return bufLen(rsp);
        
lblKO:
    trcErr(ret);
        return -1;
}


static int prepareBlock5(tBuffer *rsp){
    int ret;

        ret= bufApp(rsp,(byte *)"40404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040",0); CHK; //FinalBlock
      
    return bufLen(rsp);
        
lblKO:
    trcErr(ret);
        return -1;
}


static int printRspDnlDaily(void){
    int ret;
        word idx;
        card DmsBinCnt;
        card RngBinCnt;
        byte i;
    char buf[256],temp[256];
        char CardType= 0; //AB stub
        char IssRow;
        word wrd;
        char chr;

        /*
        //      char cc[128],bb[128];
                bb[0]='\0';
                for (i=0;i<3;++i)
                {       
                        ret= bin2num(&num,&buf[i],1);
                        ret= num2dec(cc,num,2);
                        strcat(bb,cc);
                }
                ret= prtS(bb); CHK;
        */

    ret= prtS("DmsBinVer:"); CHK;
    ret= mapGet(appDmsBinVerNo,buf,256); CHK;   
    ret= prtS(buf); CHK;
  
        ret= prtS("GlbDmsBinCnt:"); CHK;
    ret= mapGet(glbDmsBinCnt,buf,256); CHK;     
    ret= prtS(buf); CHK;
        ret= dec2num(&DmsBinCnt,buf,4);

        ret= prtS("RngBinVer:"); CHK;
    ret= mapGet(appRngBinVerNo,buf,256); CHK;   
    ret= prtS(buf); CHK;

        ret= prtS("GlbRngBinCnt:"); CHK;
    ret= mapGet(glbRngBinCnt,buf,256); CHK;     
    ret= prtS(buf); CHK;
        ret= dec2num(&RngBinCnt,buf,4);

        for (idx=0;idx<DmsBinCnt;idx++)
        {
                strcpy(buf,"idx=");
        num2dec(buf+4,idx,3);
        strcat(buf,":");
        ret= prtS(buf);CHECK(ret>=0,lblKO);

                ret= mapMove(dmsBinBeg+1,idx);
                ret= prtS("dmsBin:"); CHK;
                ret= mapGet(dmsBin,buf,256); CHK;

        //      ret= sprintf(temp,"%02d%02d%02d",buf[0],buf[1],buf[2]);
                ret= bin2hex((char *)temp,(byte *)buf,3);
                ret= prtS(temp); CHK;

                ret= prtS("Issuer Row:"); CHK;
                ret= mapGetByte(dmsIssRow,IssRow); CHK;
                num2hex(temp,IssRow,2*sizeof(byte));
                ret= prtS(temp); CHK;
        
                for (i=0;i<dimIss;i++)
                {
                        ret= mapMove(issBeg+1,i);
                        ret= mapGetByte(issId,chr); CHECK(ret>0,lblKO);
                        if(!chr) break; // what to do??

                        if (chr == CardType) 
                        {
                                ret= prtS("CardTypePermissions:"); CHK;
                                ret= mapGetWord(dmsPer,wrd); CHK;
                                num2hex(temp,wrd,2*sizeof(word));
                                ret= prtS(temp); CHK;
                                break;
                        }
                }
                
        }


        for (idx=0;idx<RngBinCnt;idx++)
        {
                strcpy(buf,"idx=");
        num2dec(buf+4,idx,3);
        strcat(buf,":");
        ret= prtS(buf);CHECK(ret>=0,lblKO);

                ret= mapMove(binBeg+1,idx);
                ret= prtS("LowBin:"); CHK;
                ret= mapGet(binLowRange,buf,256); CHK;

        //      ret= sprintf(temp,"%02d%02d%02d",buf[0],buf[1],buf[2]);
                ret= bin2hex((char *)temp,(byte *)buf,3);
                ret= prtS(temp); CHK;

                ret= prtS("HighBin:"); CHK;
                ret= mapGet(binHighRange,buf,256); CHK;

        //      ret= sprintf(temp,"%02d%02d%02d",buf[0],buf[1],buf[2]);
                ret= bin2hex((char *)temp,(byte *)buf,3);
                ret= prtS(temp); CHK;

                ret= prtS("Issuer Row:"); CHK;
                ret= mapGetByte(binIssRow,IssRow); CHK;
                num2hex(temp,IssRow,2*sizeof(byte));
                ret= prtS(temp); CHK;
        
                for (i=0;i<dimIss;i++)
                {
                        ret= mapMove(issBeg+1,i);
                        ret= mapGetByte(issId,chr); CHECK(ret>0,lblKO);
                        if(!chr) break; // what to do??

                        if (chr == CardType) 
                        {
                                ret= prtS("CardTypePermissions:"); CHK;
                                ret= mapGetWord(binPer,wrd); CHK;
                                num2hex(temp,wrd,2*sizeof(word));
                                ret= prtS(temp); CHK;
                                break;
                        }
                }
                
        }

        return 1;
lblKO:
    trcErr(ret);
        return -1;      
}

void tctt0002(void){
    int ret;
    byte dRsp[256];
    tBuffer bRsp;
        byte buf[256+1];

    ret= dspClear(); CHECK(ret>=0,lblKO);
    
        ret= dspLS(0,"prepareBlock1..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
        ret= prepareBlock1(&bRsp); CHECK(ret>=0,lblKO);     

        ret= hex2bin(buf,(char *)bufPtr(&bRsp),(byte)(bufLen(&bRsp)/2));  

    ret= dspLS(1,"parse01..."); CHECK(ret>=0,lblKO);    
        ret= rspDnlDailyB1(buf,(word)strlen((char *)buf)); CHECK(ret>=0,lblKO);

//--------------------------------------------------------------------

        ret= dspLS(0,"prepareBlock2..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
        buf[0]='\0';
        ret= prepareBlock2(&bRsp); CHECK(ret>=0,lblKO);     

        ret= hex2bin(buf,(char *)bufPtr(&bRsp),(byte)(bufLen(&bRsp)/2));  

    ret= dspLS(1,"parse02..."); CHECK(ret>=0,lblKO);    
        ret= rspDnlDailyB2(buf,256); CHECK(ret>=0,lblKO);

//--------------------------------------------------------------------

        ret= dspLS(0,"prepareBlock3..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
        buf[0]='\0';
        ret= prepareBlock3(&bRsp); CHECK(ret>=0,lblKO);     

        ret= hex2bin(buf,(char *)bufPtr(&bRsp),(byte)(bufLen(&bRsp)/2));  

    ret= dspLS(1,"parse03..."); CHECK(ret>=0,lblKO);    
        ret= rspDnlDailyB3(buf,256); CHECK(ret>=0,lblKO);

//--------------------------------------------------------------------

        ret= dspLS(0,"prepareBlock4..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
        buf[0]='\0';
        ret= prepareBlock4(&bRsp); CHECK(ret>=0,lblKO);     

        ret= hex2bin(buf,(char *)bufPtr(&bRsp),(byte)(bufLen(&bRsp)/2));  

    ret= dspLS(1,"parse04..."); CHECK(ret>=0,lblKO);    
        ret= rspDnlDailyB4(buf,256); CHECK(ret>=0,lblKO);

//--------------------------------------------------------------------

        ret= dspLS(0,"prepareBlock5..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
        buf[0]='\0';
        ret= prepareBlock5(&bRsp); CHECK(ret>=0,lblKO);     

        ret= hex2bin(buf,(char *)bufPtr(&bRsp),(byte)(bufLen(&bRsp)/2));  

    ret= dspLS(1,"parse05..."); CHECK(ret>=0,lblKO);    
        ret= rspDnlDailyB5(buf,256); CHECK(ret>=0,lblKO);

//--------------------------------------------------------------------
        mapPutByte(regDnlPerformed ,1);
    ret= printRspDnlDaily(); CHECK(ret>=0,lblKO);

    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrStop(1);
   // tmrPause(3);
}
#endif

