#include "SDK30.H"
#include "etat.h"
#include "log.h"
#include "bri.h"

static const char APP_NAME[] = "ISOZZSG";

int Main(unsigned int size, StructPt *data)
{
	NO_SEGMENT No;
	int ret=FCT_OK;
	int hdl= 1;
	char buf[25], buf1[25], buf2[25];
	byte yy, mm, dd, hh, min, ss;
	int tempTime,menit,detik;

	No = ApplicationGetCurrent();
	switch(data->service)
	{
		case GIVE_YOUR_DOMAIN:
		case IS_NAME:
		case IS_STATE:
			hdl= 0;
			break;
	}
	if(hdl)
		hdlBefore();

	switch(data->service)
	{
		case AFTER_RESET :
			trcS("Main: AFTER_RESET\n");
			ret = after_reset(No,NULL,&data->Param.AfterReset.param_out);
			break;

		case MORE_FUNCTION :
			trcS("Main: MORE_FUNCTION\n");
			ret = more_function(No,NULL,NULL);
			break;

		case IS_TIME_FUNCTION :
			
			memset(buf  ,0x00,sizeof(buf));
			memset(buf1,0x00,sizeof(buf1));
			memset(buf2,0x00,sizeof(buf2));
			
			mapGet(acqTID, buf2, 8);
			strcpy(buf, "20");    
			ret = getDateTime(buf + 2); 

			memcpy(buf1,buf,4);
			buf1[4]=0;
			yy = atoi(buf1);
			memcpy(buf1,&buf[4],2);
			buf1[2]=0;
			mm = atoi(buf1);
			memcpy(buf1,&buf[6],2);
			buf1[2]=0;
			dd = atoi(buf1);
			memcpy(buf1,&buf[8],2);
			buf1[2]=0;
			hh = atoi(buf1);
			memcpy(buf1,&buf[10],2);
			buf1[2]=0;
			min = atoi(buf1);
			memcpy(buf1,&buf[12],2);
			buf1[2]=0;
			ss = atoi(buf1);

			if(min >= 60)
			{
				min = 0;
				hh++;
			}
			if(hh >= 24)
				hh = 0;
			if ((atoi(buf2)<10000))
			{
				menit=(atoi(buf2)/60);
				detik=(atoi(buf2)%60);
			}
			else
			{
			tempTime = (atoi(buf2)%10000);
				menit=(tempTime/60);
				detik=(tempTime%60);
			}
			if((hh==1) && (min >= (30+menit)))
				debitLogon(0, 1);
			break;

		case TIME_FUNCTION :
			trcS("Main: TIME_FUNCTION\n");
			ret = time_function(No,NULL,NULL);
			break;

			//case CONSULT :
			//	ret = consult(No,NULL,NULL);
			//      break;

			//case MCALL :
			//	ret = mcall(No,NULL,NULL);
			//      break;

		case STATE :
			ret = state(No,NULL,NULL);
			break;

		case IS_NAME :
			trcS("Main: IS_NAME\n");
			ret = is_name(No,NULL,&data->Param.IsName.param_out);
			break;

		case IS_STATE :
			trcS("Main: IS_STATE\n");
			ret = is_state(No,NULL,&data->Param.IsState.param_out);
			break;
			/*
				case IS_EVOL_PG :
					ret = is_evol_pg(No,NULL,&data->Param.IsEvolPg.param_out);
			        break;
			*/
		case IS_CHANGE_INIT :
			trcS("Main: IS_CHANGE_INIT\n");
			ret = is_change_init(No,NULL,&data->Param.IsChangeInit.param_out);
			break;

	    //case MODIF_PARAM :
		//	ret = modif_param(No, &data->Param.ModifParam.param_in, NULL);
		//	break;

		case IDLE_MESSAGE :
			trcS("Main: IDLE_MESSAGE\n");
			ret = idle_message(No,NULL,NULL);
			break;

		case KEYBOARD_EVENT :
			 ret = keyboard_event(No,&data->Param.KeyboardEvent.param_in,&data->Param.KeyboardEvent.param_out);
			 break;

			//case FILE_RECEIVED :
			//      ret = file_received(No,&data->Param.FileReceived.param_in,NULL);
			//      break;

		case FILE_RECEIVED :
			ret = file_received(No,&data->Param.FileReceived.param_in,NULL);
			break;
			//case IS_DELETE :
			//      ret = is_delete(No,NULL,&data->Param.IsDelete.param_out);
			//      break;

		case DEBIT_NON_EMV :
			trcS("Main: DEBIT_NON_EMV\n");

			//if(adaTransPembelian()) //@agmr @@SIMAS-SWIPE_IDLE - remark this line
			    ret = debit_non_emv(No,&data->Param.DebitNonEmv.param_in,&data->Param.DebitNonEmv.param_out); //@agmr - tutup untuk ptr
			//prtS("DEBIT_NON_EMV");
			break;

		//open by irfan, 10052010
		case GIVE_YOUR_DOMAIN :
			  trcS("Main: GIVE_YOUR_DOMAIN\n");
			  ret = give_your_domain(No,NULL,&data->Param.GiveYourType.param_out);
			  break;
		//
#if 0
		case GIVE_AID:
			trcS("Main: GIVE_AID\n");
			ret = give_aid(No,&data->Param.GiveAid.param_in,&data->Param.GiveAid.param_out);
			EMVFlagSet = 2;
			//prtS("GIVE_AID");
		    	break;

		case IS_FOR_YOU_AFTER :
			trcS("Main: IS_FOR_YOU_AFTER\n");
			if(EMVFlagSet != 1){
				//prtS("EMVFlagSet == 0");
				EMVFallbackFlagSet = 1;
				EMV_ENTRYMODE = 0;
			}
			else{
				//prtS("EMVFlagSet == 1");
				EMVFallbackFlagSet = 0;
				EMVFlagSet = 0;
			}
			ret = is_for_you_after(No,&data->Param.IsForYouAfter.param_in,&data->Param.IsForYouAfter.param_out);
			//prtS("IS_FOR_YOU_AFTER");
			break;
#endif
		case GIVE_AID:
			trcS("Main: GIVE_AID\n");
			ret = give_aid(No,&data->Param.GiveAid.param_in,&data->Param.GiveAid.param_out);
		    	break;

		case IS_FOR_YOU_AFTER :
			trcS("Main: IS_FOR_YOU_AFTER\n");
			ret = is_for_you_after(No,&data->Param.IsForYouAfter.param_in,&data->Param.IsForYouAfter.param_out);
			break;


		case IS_CARD_EMV_FOR_YOU:
			trcS("Main: IS_CARD_EMV_FOR_YOU\n");
			ret = is_card_emv_for_you(No,&data->Param.IsCardEmvForYou.param_in,&data->Param.IsCardEmvForYou.param_out);
			//prtS("IS_CARD_EMV_FOR_YOU");
			break;

		case DEBIT_EMV:
			// tutup  untuk mendisable  deep sale
			trcS("Main: DEBIT_EMV\n");
			ret = debit_emv(No,&data->Param.DebitEmv.param_in,&data->Param.DebitEmv.param_out);//@agmr - tutup untuk ptr
			//prtS("DEBIT_EMV");
			
			break;
		//
		case FALL_BACK:
			 pmtFallback();
		    break;
		#ifdef _USE_ECR
		case MESSAGE_RECEIVED:
			ret = message_received(No,&data->Param.MessageReceived.param_in,NULL);
			break;
		#endif // _USE_ECR
		case TRACK_STREAM:
			//ret = track_stream(No,&data->Param.TrackStream.param_in,&data->Param.TrackStream.param_out);
			//break;
		case IS_CARD_SPECIFIC: // French Health Care Domain
		case TIME_FUNCTION_CHAINE : // French Bank Domain
		case GIVE_INFOS_CX:		// French Bank Domain
		case DEBIT_OVER:
		case AUTO_OVER:
		case IS_ORDER:			// French Health Care Domain
		case ORDER:				// French Health Care Domain
		case IS_SUPPR_PG:		// French Health Care Domain
		case IS_INSTALL_PG:		// French Health Care Domain
		case GET_ORDER:			// French Health Care Domain
		case IS_LIBELLE:		// French Health Care Domain
		case EVOL_CONFIG:		// French Bank Domain
		case GIVE_MONEY:		// French Bank Domain
		case COM_EVENT:
		case MODEM_EVENT:
		#ifndef _USE_ECR
		case MESSAGE_RECEIVED:
		#endif //_USE_ECR
		case CARD_INSIDE:		// French Health Care Domain
		case GIVE_INTERFACE:
		case IS_BIN_CB:			// French Bank Domain
		case SELECT_FUNCTION:		// French Bank Domain
		case SELECT_FUNCTION_EMV:	// French Bank Domain

/* close by irfan, 10052010
		case GIVE_AID:
		case IS_CARD_EMV_FOR_YOU:
		case DEBIT_EMV:
*/
		default:
			trcS("Main: UNKNOWN CASE\n");
			ret = FCT_OK;
			//prtS("OTHERS");
			break;
	}

	if(hdl)
		hdlAfter();
	return ret; //for accepted card must return another answer then FCT_OK
}

void my_custom_message(int size, InfosMSG_CUST *infocust)
{
	if(infocust->num == 256)
	{
		//display custom message
		if(infocust->flag)
		{
			//get key
		}
		infocust->cr_treatment = CUSTOMIZED_MESSAGE;
	}
	else
	{
		infocust->cr_treatment = NO_CUSTOMIZED_MESSAGE;
	}
}

typedef int (*T_SERVICE_FUNCTION)(unsigned int nSize, void*Data);

int give_interface(unsigned short no,void *p1,void *p2)
{
	#ifdef _USE_ECR
	service_desc_t MesServices[26];
	#else
	service_desc_t MesServices[25];
	#endif
	int i ;
	i = 0 ; //service 1
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_NAME;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 2
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = GIVE_YOUR_DOMAIN;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 3
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = MORE_FUNCTION;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 4

	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IDLE_MESSAGE;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 30;
	i++;//service 5

	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = KEYBOARD_EVENT;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 20;
	i++;//service 6
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = AFTER_RESET;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 7
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_STATE;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 8
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_DELETE;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 9
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = DEBIT_NON_EMV;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 10
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_FOR_YOU_AFTER;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 11
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = STATE;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 12
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = MCALL;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 13
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = CONSULT;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 14
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_EVOL_PG;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 15
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_TIME_FUNCTION;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 16
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = TIME_FUNCTION;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 17
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_CHANGE_INIT;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 18
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = MODIF_PARAM;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;//service 19
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = FILE_RECEIVED;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
	/*
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = TRACK_STREAM  ;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
	*/
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = GIVE_AID  ;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IS_CARD_EMV_FOR_YOU  ;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = DEBIT_EMV  ;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = FILE_RECEIVED;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = FALL_BACK;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
#if 1
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = CUSTOMIZE_MESSAGE;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)my_custom_message;
	MesServices[i].priority  = 10;
	i++;
#endif
#ifdef _USE_ECR
	MesServices[i].appli_id  = no; //service 26
	MesServices[i].serv_id   = MESSAGE_RECEIVED;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 10;
	i++;
#endif //_USE_ECR
	ServiceRegister(i,MesServices);

	return FCT_OK;
}

void entry(void)
{
	object_info_t info;

	ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(),&info);
	give_interface(info.application_type, NULL, NULL);

	// Open all the external libraries.

	//paramlib_open();	// TELIUM Manager Library
	//extenslib_open();	// Extension Library
	//svlib_open();		// SV Library (for date management).
	//libgrlib_open();	// Graphic Library
	//entrylib_open();	//
}

//++ @agmr - tambahan dari native
int state(NO_SEGMENT AppliNum, void* pParamIn, void* pParamOut)
{
//	FILE* hPrinter;
	SEGMENT Infos;

	// Get information on the application
	info_seg(AppliNum, &Infos);

	// Open the printer
//	hPrinter = fopen("PRINTER", "w-*");
//	if (hPrinter != NULL)
	{
		// Print
		pprintf("\x1b" "@\x1b" "E%s\n\x1b" "@", APP_NAME);
		pprintf("Version : %s\n", Infos.libelle);
		pprintf("CRC     : %04x\n\n", Infos.crc);
		//pprintf ("Vers. : %s\n",Get_AppliName(AppliNum));
		//pprintf ("CRC   : %04x\n",Get_AppliCrc(AppliNum));
		//pprintf ("Code  : %06x\n",Get_AppliCode(AppliNum));
		//pprintf ("Data  : %06x\n\n\n",Get_AppliData(AppliNum));

		ttestall(PRINTER, 0);
//		fclose(hPrinter);
	}

	return FCT_OK;
}
//-- @agmr - tambahan dari native
