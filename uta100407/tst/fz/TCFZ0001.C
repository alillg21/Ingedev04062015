#include <string.h>
#include "stdio.h"
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcfz0001.
//schedule process

#define CHK CHECK(ret>=0,lblKO)

extern tTask tasks[3];

static int prepare00(void){
    int ret;
        byte bByte;
        
                ret= mapMove(taskBeg+1,0);
                ret= mapPutByte(taskTypeID,0); CHK;
                ret= mapPutByte(taskStatus,0); CHK;
                ret= mapPutStr(taskTime,"1010"); CHK;
                //ret= mapPutStr(taskDate,"050412"); CHK;               

                ret= mapMove(taskBeg+1,1);
                ret= mapPutByte(taskTypeID,1); CHK;
                ret= mapPutByte(taskStatus,0); CHK;
                ret= mapPutStr(taskTime,"1035"); CHK;
                //ret= mapPutStr(taskDate,"050412"); CHK;               

                ret= mapMove(taskBeg+1,2);
                ret= mapPutByte(taskTypeID,2); CHK;
                ret= mapPutByte(taskStatus,0); CHK;
                ret= mapPutStr(taskTime,"0125"); CHK;
                ret= mapPutStr(taskDate,"050912"); CHK;         

                //sets terminal permission for transfer batch
                ret= mapMove(perBeg+1,1); CHK;
                bByte = 0x01;
                ret = mapPutByte(perBatchTrf,bByte); CHK;

                return 1;    
lblKO:
    trcErr(ret);
 return -1;
}


void tcfz0001(void){
    int ret,i;
        char TskTime[4+1]="";
        char TskDate[10+1]="";
        byte TskStatus;
        char buf[256+1];
        char dt[20+1];
        char tm[20+1];
        int status =0;

    ret= dspClear(); CHECK(ret>=0,lblKO);
    
    ret= dspLS(0,"prepare00..."); CHECK(ret>=0,lblKO);    
    ret= prepare00(); CHECK(ret>=0,lblKO);
        tmrPause(1);
         
        for(i=0;i<3;++i)
        {

                memset(TskTime,0,4+1);
                memset(TskDate,0,10+1);

                ret=  mapMove(taskBeg+1,(byte)i);
                ret = mapGetByte(taskStatus,TskStatus); CHECK(ret>0,lblKO);
                ret = mapGet(taskDate,TskDate,lenTaskDate); CHECK(ret>0,lblKO);
                ret = mapGet(taskTime,TskTime,lenTaskTime); CHECK(ret>0,lblKO);
                strcat(TskDate,TskTime);
                
                prtS("status:"); CHK;
                num2dec(buf,TskStatus,0);
                prtS(buf); CHK;
                prtS("date        time:"); CHK;
                sprintf(tm,"%.2s%.2s%.2s      %.2s%.2s",TskDate,TskDate+2,TskDate+4,TskDate+6,TskDate+8); CHK;
                prtS(tm); CHK;
                prtS("++++++++++++++++"); CHK;
        }

        prtS("---------------------"); CHK;
        prtS("AFTER SORTING"); CHK;
    ret= dspLS(1,"processSchedule ..."); CHECK(ret>=0,lblKO); 


        while(1){
                ret= processSchedule(); 
                CHECK(ret>=0,lblKO);
                status=1;

        }
                
        for(i=0;i<3;++i)
                {
                        prtS("date        time:"); CHK;
                        sprintf(dt,"%.2s%.2s%.2s      %.2s%.2s",tasks[i].dateTime,tasks[i].dateTime+2,tasks[i].dateTime+4,tasks[i].dateTime+6,tasks[i].dateTime+8); CHK;
                        prtS(dt); CHK;
                        prtS("++++++++++++++++"); CHK;
                }

    goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrStop(1);
}
#endif
