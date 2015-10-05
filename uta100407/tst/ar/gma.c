//#include <string.h>
#include <unicapt.h>
//#include "message.h"

#include "gmaDefines.h"
#include "GmaMsg.h"
#include "gmaMsgCon.h"
#include "gmaPgMsg.h"

#include "tst.h"
#include "sys.h"
//#include "log.h"

#define __IDLEMSG__ "UTA 2.3\ntest cases\nAmaury"
#define __IDLEAPP__ "tcar32"

static void OnPowerOn(amgMsg_t *msg){
    gmaPgMsgInitialize((amgMsg_t *)msg);
    gmaMsgAddPhysicalAppInfo(RET_OK,1,__IDLEAPP__,GMA_USER_APPLICATION);
    //hdlPowerOn();
}

static void OnStartup(amgMsg_t *msg){
    uint16 len;
    uint32 date, time;

    psyDateTimeGet(&date, &time);
    gmaMsgScheduleEvent(0,GMA_EVENT_ACTION_ADD,1,date,time);
    //gmaMsgMediaData(0, 0x0, GMA_MEDIA_MONO_BITMAP, sizeof(image_data),image_data);
        len= strlen(__IDLEMSG__);
    gmaMsgMediaData(0,1,GMA_MEDIA_TEXT,(uint16)(len+1),(uchar *)__IDLEMSG__);
}

static void OnIntegrityCheck(amgMsg_t *msg){ gmaMsgAddLogicalAppStatus(0xFF,1);}

static uint32 hHmi= 0;

#define RSP_LEN 1024
typedef struct{ int16 sta; uint16 len; char buf[RSP_LEN];}tResponse;
static int getKey(void){
    int ret;
    tResponse rsp;
    ret= hmiKeyReq(hHmi);
    while(ret==RET_RUNNING) ret= hmiResultGet(hHmi,sizeof(rsp),&rsp);
    hmiCancel(hHmi);
    if(ret==RET_OK) ret= rsp.buf[0]; else ret= -1;
    return ret;
}

static int confirm(char *msg){
    int ret;

	(void) hmiADClearLine(hHmi, HMI_ALL_LINES);
    (void) hmiADDisplayText(hHmi, 2, 0, "Confirm selection");
    (void) hmiADDisplayText(hHmi, 3, 0, msg);
	
    ret= getKey();
    hmiClose(hHmi);
    hHmi= 0;
    return ret;
}

static void selectMain(void)
{
	int loop = 1;

	while (loop)
	{
		int ret;

		hmiOpen("DEFAULT",&hHmi);
		hmiADDisplayText(hHmi, 0, 0, "tcar32");
		hmiADDisplayText(hHmi, 1, 0, "Select [1-9]");
		hmiADDisplayText(hHmi, 2, 0, " 1: LoadArticle");
		hmiADDisplayText(hHmi, 3, 0, " 2: SaleFinish");
		hmiADDisplayText(hHmi, 4, 0, " 3: SaleClose");
		hmiADDisplayText(hHmi, 5, 0, " 4: ProgramTax");
		hmiADDisplayText(hHmi, 6, 0, " 5: LoadListItem");
		hmiADDisplayText(hHmi, 7, 0, " 6: OnLineSale");
		hmiADDisplayText(hHmi, 8, 0, " 7: HandShake");
		hmiADDisplayText(hHmi, 9, 0, " 8: OnLineVoid");
		hmiADDisplayText(hHmi,10, 0, " 9: FinishSale");
		ret = getKey();
		
		switch (ret)
		{
			case '1': if(confirm(" 1: LoadArticle")==0x3A) tcar0000(); break;
			case '2': if(confirm(" 2: SaleFinish")==0x3A) tcar0001(); break;
			case '3': if(confirm(" 3: SaleClose")==0x3A) tcar0002(); break;
			case '4': if(confirm(" 4: ProgramTax")==0x3A) tcar0003(); break;
			case '5': if(confirm(" 5: LoadListItem")==0x3A) tcar0004(); break;
			case '6': if(confirm(" 6: OnLineSale")==0x3A) tcar0005(); break;
			case '7': if(confirm(" 7: HandShake")==0x3A) tcar0006(); break;
			case '8': if(confirm(" 8: OnLineVoid")==0x3A) tcar0007(); break;
			case '9': if(confirm(" 9: FinishSale")==0x3A) tcar0008(); break;
			default : loop = 0; break;
		}

		if (loop)
		{
			// Pause to read the test result message
			tmrPause(3);
		}
		
		if (hHmi) hmiClose(hHmi);
		hHmi = 0;
	}
}


static void evtMenuTransaction(amgMsg_t *msg){
    gmaStructHeader_t *hdr;
    uint16 len;

    gmaMsgDecode(msg);
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK){
                if(hdr->id!=GMA_STRUCT_TRANS_MENU) continue;
//        hdlMainMenu();
		  selectMain();

    }
    gmaMsgEndDecode();
    gmaMsgAddReturnCode(0);
}

static void getTrk(char *trk,gmaStructTransMagTrack_t *mag){
    int idx,ofs;
    if(mag->trackNumber==0) return; //normally, it is impossible
    if(mag->trackNumber>3) return; //normally, it is impossible
    if(mag->trackLength>128) return; //normally, it is impossible
    ofs= 128*(mag->trackNumber-1);
    memcpy(trk+ofs,mag->magTrack,mag->trackLength);
    for(idx= 0; idx<mag->trackLength; idx++){
	switch(mag->trackNumber){
	    case 1: trk[ofs+idx]+= 0x20; break;
	    case 2: trk[ofs+idx]|= 0x30; break;
	    case 3: trk[ofs+idx]+= 0x30; break;
	    default: break;
	}
    }
}

static void evtMagCardQuery(amgMsg_t *msg){
    int ret;
    gmaStructHeader_t *hdr;
    uint16 len;
    byte trk[128*3];

    memset(trk,0,128*3);
    gmaMsgDecode(msg);
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK){
        if(hdr->id!=GMA_STRUCT_TRANS_MAG_TRACK) continue;
        getTrk(trk,(gmaStructTransMagTrack_t *)hdr);
    }
    gmaMsgEndDecode();
    //ret= hdlAskMagStripe((char *)trk);
    ret=0;
    gmaMsgAddQueryResult(0xFF,(uint8)((ret>0)?GMA_ACCEPTLEVEL_MEDIUM:GMA_ACCEPTLEVEL_NO_ACCEPT));
}

static void evtMagCardTransaction(amgMsg_t *msg){
    gmaStructHeader_t *hdr;
    uint16 len;
    byte trk[128*3];

    memset(trk,0,128*3);
    gmaMsgDecode(msg);
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK){
        if(hdr->id!=GMA_STRUCT_TRANS_MAG_TRACK) continue;
        getTrk(trk,(gmaStructTransMagTrack_t *)hdr);
    }
    gmaMsgEndDecode();
    //hdlMagStripe((char *)trk);
    gmaMsgAddReturnCode(0);
}

static void evtSmartCardTransaction(amgMsg_t *msg){
    gmaStructHeader_t *hdr;
    uint16 len;
    byte buf[260];

    memset(buf,0,260);
    gmaMsgDecode(msg);
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK){
        if(hdr->id==GMA_STRUCT_TRANS_SMC_EMV){
            gmaStructTransSmcEmv_t *emv= (gmaStructTransSmcEmv_t *)hdr;
            if(emv->responseLength>260) continue; //impossible
            memcpy(buf,emv->response,emv->responseLength);
        }
    }
    gmaMsgEndDecode();
//    ret= hdlSmartCard(buf)); //uncomment it for EMV applications
    gmaMsgAddReturnCode(0);
}

static void OnTransactionQuery(amgMsg_t *msg){
    uint16 len;
    int16 ret;
    gmaStructHeader_t *hdr;
    gmaStructTransType_t *typ;

    gmaMsgDecode(msg);
    while(42){
        ret= gmaMsgRead((void **)&hdr,&len);
        if(hdr->id == GMA_STRUCT_TRANS_TYPE) break;
        if(ret!= RET_OK) break; //error
    }
    gmaMsgEndDecode();

    typ= (gmaStructTransType_t *)hdr;
    switch(typ->transType){
        case GMA_TRANS_MAG_CARD: evtMagCardQuery(msg); break;
        default: break;
    }
}

static void OnTransaction(amgMsg_t *msg){
    uint16 len;
    int16 ret;
    gmaStructHeader_t *hdr;
    gmaStructTransType_t *typ;

    gmaMsgDecode(msg);
    while(42){
        ret= gmaMsgRead((void **)&hdr,&len);
        if(ret!=RET_OK) return; //error
        if(hdr->id==GMA_STRUCT_TRANS_TYPE) break;
    }
    gmaMsgEndDecode();

    typ= (gmaStructTransType_t *)hdr;
    switch(typ->transType){
        case GMA_TRANS_MENU: evtMenuTransaction(msg); return;
        case GMA_TRANS_MAG_CARD: evtMagCardTransaction(msg); break;
        case GMA_TRANS_SMART_CARD: evtSmartCardTransaction(msg); break;
        default: break;
    }
}

static void OnEventOccur(amgMsg_t *msg){
    gmaStructEventOccur_t *evt;
    gmaStructHeader_t *hdr;
    uint16 len;
    uint32 date, time;
    int ret;

    evt= 0;
    gmaMsgDecode(msg);
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK){
        if(hdr->id!=GMA_STRUCT_EVENT_OCCUR) continue;
        evt= (gmaStructEventOccur_t *)hdr;
        break;
    }
    gmaMsgEndDecode();
    if(!evt) return;
    psyDateTimeGet(&date, &time);
    //ret= hdlTimer(evt->eventId,&date,&time);
    ret= 0;
    if(ret) gmaMsgScheduleEvent(0,GMA_EVENT_ACTION_ADD,1,date,time);
    gmaMsgAddReturnCode(1);
}

void HandleMessage(amgMsg_t *Msg){
    amgMsg_t rsp;
    memset(&rsp,0,sizeof(rsp));
    gmaMsgEncode(&rsp,Msg->msgType);

    //hdlBefore((byte)(Msg->msgType==MSG_POWER_ON));
    switch(Msg->msgType){
        case MSG_POWER_ON: OnPowerOn(Msg); break;
        case GMA_MSG_POWER_ON: OnPowerOn(Msg); break;
        case GMA_MSG_TRANSACTION: OnTransaction(Msg); break;
        case GMA_MSG_TRANSACTION_QUERY: OnTransactionQuery(Msg); break;
        case GMA_MSG_EVENT_OCCUR: OnEventOccur(Msg); break;
        case GMA_MSG_INIT_INTEGRITY_CHECK: OnIntegrityCheck(Msg); break;
        case GMA_MSG_STARTUP: OnStartup(Msg); break;
        default: break;
    }
    //hdlAfter();
    //gmaMsgEndEncode();
    gmaMsgSendId(Msg->callerTaskId);
}

void OnMsgPowerOn(amgMsg_t *Msg){ OnPowerOn(Msg);}
