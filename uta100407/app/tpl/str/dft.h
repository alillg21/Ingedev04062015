#ifndef __DFT_H

//this file is included in dft.c only
//it is not visible for other source files

#define dftAppChn "H8N12400"
#define dftAppPabx "0"
#define dftAppPrefix "00"
#define dftAppPhone "123456789"
#define dftAppMrcPwd "0000"
#define dftAppAdmPwd "0000"
#define dftAppExp 2

static char *dftMnu[DIM(mnuBeg,mnuEnd)]={
    "0 APP TPL         ", //mnuMainMenu
    "1 CUSTOMER>       ", //mnuCustomer
    "1 MERCHANT>       ", //mnuMerchant
    "1aADMIN>          ", //mnuAdmin
    __mnuTEST__, //mnuTest
    "2 Request         ", //mnuTestReq
};

static char *dftMsg[DIM(msgBeg,msgEnd)]={
    "                ", //msgSpaces
    " Are you sure?  ", //msgAreYouSure
    "  Memory reset  ", //msgMemoryReset
    "Processing error", //msgProcessingError
    "Invalid PIN code", //msgWrongPin
    " ENTER PASWORD: ", //msgPwdInput
    "   Dialing...   ", //msgDialing
    " Please Wait... ", //msgWait
    "    Sending...  ", //msgSend
    "   Receiving... ", //msgReceive
    "ENTER AMOUNT:   ", //msgEnterAmount
    "ENTER CARD:     ", //msgEnterCard
    "", //msgEmpty
};

static char *dftRpt[DIM(rptBeg,rptEnd)]={
    "                        ", //rptSpaces
    "", //rptEmpty
};

typedef struct sScrRecord{
    char _scrCmd;
    word _scrMsg0;
    word _scrMsg1;
    word _scrMsg2;
    word _scrMsg3;
    byte _scrDly;
}tScrRecord;

static tScrRecord dftScr[DIM(infBeg,infEnd)]={
    {'b',msgSpaces,msgMemoryReset,msgAreYouSure,msgSpaces,5}, //infConfirmReset
    {'b',msgEmpty,msgProcessingError,msgSpaces,msgSpaces,5}, //infProcessingError
    {'b',msgEmpty,msgWrongPin,msgSpaces,msgSpaces,5}, //infWrongPin
    {'s',msgSpaces,msgPwdInput,msgSpaces,msgSpaces,0}, //infPwdInput
    {'s',traCtx,msgDialing,msgWait,msgSpaces,0}, //infDialing
    {'s',traCtx,msgEmpty,msgEmpty,msgSend,0}, //infSendReq
    {'s',traCtx,msgEmpty,msgEmpty,msgReceive,0}, //infRecvRsp
    {'s',traCtx,msgEnterAmount,msgSpaces,msgSpaces,0}, //infAmtInput
    {'s',traCtx,msgEnterCard,msgSpaces,msgSpaces,0}, //infCardInput
};


#define __DFT_H
#endif

