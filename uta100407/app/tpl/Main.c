#include "SDK30.H"
#include "etat.h"
#include "log.h"

int Main(unsigned int size, StructPt *data){
	NO_SEGMENT No;
	int ret=FCT_OK;
	int hdl= 1;

	No = ApplicationGetCurrent();
	switch(data->service){
		case GIVE_YOUR_DOMAIN:
		case IS_NAME:
		case IS_STATE:
			hdl= 0;
			break;
	}
	if(hdl) hdlBefore();

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
		
	break;

	//case TIME_FUNCTION :
  //      ret = time_function(No,NULL,NULL);
	//	break;

	//case CONSULT :
	//	ret = consult(No,NULL,NULL);
  //      break;

	//case MCALL :
	//	ret = mcall(No,NULL,NULL);
  //      break;

	case STATE :
		//ret = state(No,NULL,NULL);
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
        //      ret = modif_param(No, &data->Param.ModifParam.param_in, NULL);
	   //	break;

	case IDLE_MESSAGE :
        trcS("Main: IDLE_MESSAGE\n");
        ret = idle_message(No,NULL,NULL);
        break;

	//case KEYBOARD_EVENT :
  //      ret = keyboard_event(No,&data->Param.KeyboardEvent.param_in,&data->Param.KeyboardEvent.param_out);
	//	break;

	//case FILE_RECEIVED :
  //      ret = file_received(No,&data->Param.FileReceived.param_in,NULL);
  //      break;

	case IS_FOR_YOU_AFTER :
		trcS("Main: IS_FOR_YOU_AFTER\n");
		ret = is_for_you_after(No,&data->Param.IsForYouAfter.param_in,&data->Param.IsForYouAfter.param_out);
        break;

	//case IS_DELETE :
  //      ret = is_delete(No,NULL,&data->Param.IsDelete.param_out);
  //      break;

	case DEBIT_NON_EMV :
		trcS("Main: DEBIT_NON_EMV\n");
		ret = debit_non_emv(No,&data->Param.DebitNonEmv.param_in,&data->Param.DebitNonEmv.param_out);
        break;
/*
	case GIVE_YOUR_DOMAIN :
        trcS("Main: GIVE_YOUR_DOMAIN\n");
		ret = give_your_domain(No,NULL,&data->Param.GiveYourType.param_out);
        break;
*/

	case TRACK_STREAM :
		//ret = track_stream(No,&data->Param.TrackStream.param_in,&data->Param.TrackStream.param_out);
		//break;
	case IS_CARD_SPECIFIC: // French Health Care Domain	
	case TIME_FUNCTION_CHAINE : // French Bank Domain
	case GIVE_INFOS_CX:		// French Bank Domain
	case FALL_BACK:
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
	case MESSAGE_RECEIVED:
	case CARD_INSIDE:		// French Health Care Domain
	case GIVE_INTERFACE:
	case IS_BIN_CB:			// French Bank Domain
	case SELECT_FUNCTION:		// French Bank Domain
	case SELECT_FUNCTION_EMV:	// French Bank Domain

	case GIVE_AID:
	case IS_CARD_EMV_FOR_YOU:
	case DEBIT_EMV:

	default:
		trcS("Main: UNKNOWN CASE\n");
		ret = FCT_OK;
        break;
	}

	if(hdl) hdlAfter();
	return ret; //for accepted card must return another answer then FCT_OK
}

typedef int (*T_SERVICE_FUNCTION)(unsigned int nSize, void*Data);

int give_interface(unsigned short no,void *p1,void *p2)
{
	service_desc_t MesServices[24];
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
	/*
	MesServices[i].appli_id  = no;
	MesServices[i].serv_id   = IDLE_MESSAGE;
	MesServices[i].sap       = (T_SERVICE_FUNCTION)Main;
	MesServices[i].priority  = 30;
	i++;//service 5
	*/
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
/*
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
*/

	ServiceRegister(i,MesServices);

	return FCT_OK;
}

void entry(void){
	object_info_t info;

	ObjectGetInfo(OBJECT_TYPE_APPLI, ApplicationGetCurrent(),&info);
	give_interface(info.application_type, NULL, NULL);

	// Open all the external libraries.

	paramlib_open();	// TELIUM Manager Library
	extenslib_open();	// Extension Library
	svlib_open();		// SV Library (for date management).
	libgrlib_open();	// Graphic Library
	entrylib_open();	//
}

