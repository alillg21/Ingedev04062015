//#include <string.h>
#include <unicapt.h>
//#include "message.h"

#include "gmaDefines.h"
#include "GmaMsg.h"
#include "gmaMsgCon.h"
#include "gmaPgMsg.h"

#include "sys.h"
//#include "log.h"

#ifdef __TEST__
#include "tc.h"
#endif


static void OnPowerOn(amgMsg_t *msg){
    gmaPgMsgInitialize((amgMsg_t *)msg);
    gmaMsgAddPhysicalAppInfo(RET_OK, 1, tcIdleApp(), GMA_USER_APPLICATION);	
    //hdlPowerOn();
	tcPrtDateTime("BOOT");
}


static void OnStartup(amgMsg_t *msg){
    uint16 len;
    uint32 date, time;

    psyDateTimeGet(&date, &time);
    gmaMsgScheduleEvent(0,GMA_EVENT_ACTION_ADD,1,date,time);
    //gmaMsgMediaData(0, 0x0, GMA_MEDIA_MONO_BITMAP, sizeof(image_data),image_data);
        len= strlen(tcIdleMsg());
    gmaMsgMediaData(0,1,GMA_MEDIA_TEXT,(uint16)(len+1),(uchar *)tcIdleMsg());
}


static void OnIntegrityCheck(amgMsg_t *msg){ gmaMsgAddLogicalAppStatus(0xFF,1);}


static void evtMenuTransaction(amgMsg_t *msg){
    gmaStructHeader_t *hdr;
    uint16 len;

    gmaMsgDecode(msg);
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK){
                if(hdr->id!=GMA_STRUCT_TRANS_MENU) continue;
//        hdlMainMenu();
		  selectMain();  // my test menus (in tcapp.c)

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

	for(idx= 0; idx<mag->trackLength; idx++)
	{
		switch(mag->trackNumber)
		{
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
    while(gmaMsgRead((void **)&hdr,&len)==RET_OK)
	{
        if(hdr->id!=GMA_STRUCT_TRANS_MAG_TRACK) continue;
        getTrk(trk,(gmaStructTransMagTrack_t *)hdr);
    }
	
    gmaMsgEndDecode();
    //ret= hdlAskMagStripe((char *)trk);
    ret=0;
    gmaMsgAddQueryResult(0xFF,
		(uint8)((ret>0)?GMA_ACCEPTLEVEL_MEDIUM:GMA_ACCEPTLEVEL_NO_ACCEPT));
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
