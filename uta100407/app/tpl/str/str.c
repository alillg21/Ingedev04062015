#include <string.h>
#include "str.h"

int traReset(void){
    int ret;
    char datetime[lenDatTim+1];
    trcS("traReset: Beg\n");

    ret = mapReset(traBeg);
    CHECK(ret > 0, lblKO);

    strcpy(datetime,"20"); //CC
    ret= getDateTime(datetime+2); //CC+YYMMDDhhmmss
    CHECK(ret>0,lblKO);

    ret= mapPut(traDatTim,datetime,lenDatTim);
    CHECK(ret>0,lblKO);
    
    ret= 1;
    goto lblEnd;
lblKO:
    trcErr(ret);
    ret= -1;
lblEnd:
    trcFN("traReset: ret=%d\n",ret);
    return ret;
}
