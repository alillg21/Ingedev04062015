#include "tst.h"
#include "err.h"
#include "log.h"
/*************************************************
Instruction of using thest case tcnl0025
1)ask mohamamd to give you one mldt script to download some binary files on DFS 
2)add ssaBatchDwl library to the ingedev project 
3)replace the name of binary fies which you have in DFS(script) with FR000K002021,FR000K00202Z,FR000K002020 if needed
4)download your application on the terminal
5)run mldt and run script(from mohamamd)
4)run my test case
5)it will transfer files from DFS to CFS
6)bdlCFSLoad is a function which it is calling function in libarary ssaBatchDwl for installing the binary files which 
i send as arguments.All jobs it will done by the function in bdlCFSLoad.These function existes in ssaBatchDwl library

/*************************************************/
#ifdef __TEST__
static code const char *srcFile = __FILE__;

//Integration test case tcnl0025.
//BDL.
//Prerequisite , downloading binary files in DFS

#define CHK CHECK(ret>=0,lblKO)

int16 bdlClbChangeTerminalState(uint32 newState) {
    return RET_OK;
}
int16 bdlCfsRequestFile(char *name) {
    char fileName[PSY_FILENAME_LENGTH + 1];

    memset(fileName, 0, PSY_FILENAME_LENGTH + 1);
    memmove(fileName, name, PSY_FILENAME_LENGTH);

    file = psyFileOpen(fileName, "r");
    if(file == 0) {
        psyFileErrnoGet();
        return ERR_FILE_NOT_AVAILABLE;
    } else {
        psyFileStat(fileName, &fileStat);
        return RET_OK;
    }
}

int16 bdlGetFileBlock(uint8 * pdata, uint32 * pdataLength) {
    if(file == 0) {
        return ERR_FILE_NOT_SET;
    }
    psyFileRead(file, pdata, (uint16 *) pdataLength);
    ofs += *pdataLength;
    if(ofs >= fileStat.stSize) {
        psyFileClose(file);
        ofs = 0;
        file = 0;
        return RET_OK;
    }
    return RET_OK;
}

/************************************************************/
int bdlCFSLoad(tFileList * FileList, int fileNo) {

    int16 ret;
    CfsFileInfo_t stFiles[10];
    byte idx = 0;

    ret = psyFileChDir("c:\\");

    while(idx < fileNo) {
        memmove(stFiles[idx].fileName, FileList[idx].fileName, 12);
        stFiles[idx].fileLength = FileList[idx].fileSize;
        stFiles[idx].rfu = BDL_REP;
        idx++;
    }

    /* Running an update... */
    ret = bdlLogConfig(BDL_START_LOG);
    CHECK(ret >= 0, lblDnl);

    ret = bdlOpenSession(0, 0);
    CHECK(ret >= 0, lblKO);
    dspClear();
    dspLS(1, "BEGIN LOADER");
    dspLS(1, "Loading...");

    ret = bdlCfsStartDownload(stFiles, idx);
    CHECK(ret >= 0, lblKO);

    dspClear();

    ret = bdlCloseSession();
    CHECK(ret >= 0, lblKO);

    ret = bdlLogConfig(BDL_STOP_LOG);
    CHECK(ret >= 0, lblKO);

    /* End of the update. */
    ret = 1;
    goto lblEnd;

  lblKO:
    dspLS(1, "KO..");
    tmrPause(2);
    trcErr(ret);
    return;
  lblEnd:
    trcFN("bdlCFSLoad: ret=%d\n", ret);
    return ret;
}

/***********************************************************************************/

void tcnl0025(void) {
    int ret;
    tFileList FileList[10];
    int FileNum = 3;

    char Filename[12 + 1] = "";

    memmove(stFiles[0].fileName, "FR000K002021", 12);
    stFiles[0].fileLength = 21372;
    stFiles[0].rfu = BDL_REP;

    memmove(stFiles[1].fileName, "FR000K00202Z", 12);
    stFiles[1].fileLength = 380;
    stFiles[1].rfu = BDL_REP;

    memmove(stFiles[2].fileName, "FR000K002020", 12);
    stFiles[2].fileLength = 430;
    stFiles[2].rfu = BDL_REP;

    //this line is not working in simelite
    ret = bdlCFSLoad((tFileList *) & FileList, FileNum);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    dspLS(1, "KO..");
    tmrPause(2);
    trcErr(ret);
    return;
  lblEnd:
    return;
}
#endif
