#include "sdk30.h"
#include "tst.h"
#include "log.h"

static const unsigned char appname[]    = __IDLEAPP__;
int emvGetAids(_DEL_ *sel, uchar id);

int more_function( NO_SEGMENT no, void *p1, void *p2 ){
  StateHeader(0);
  hdlMainMenu();
  dspClear();
  StateHeader(1);
  return FCT_OK;
}

int idle_message (NO_SEGMENT no,void *p1,void *p2){
	FILE *hDisplay;
	int nFont;
	char tmp[256];

	hDisplay = fopen("DISPLAY","w");
	nFont = GetDefaultFont();

	CreateGraphics( _LARGE_ );
	sprintf(tmp,"\n%s",appname);
	_DrawString( tmp,  0, 20, _OFF_ );
	PaintGraphics();

	SetDefaultFont( nFont );
	fclose( hDisplay );

	return FCT_OK;
}

int is_delete(NO_SEGMENT no,void *paramin,S_DELETE *paramout){
	paramout->deleting=DEL_YES;
	return (FCT_OK);
}

//int after_reset( NO_SEGMENT no, void *p1, S_TRANSOUT *etatseq ){
int after_reset( NO_SEGMENT noappli, void *param_in, S_TRANSOUT *param_out){
	FLAG_CHGT  chgt;
  TYPE_CHGT  type;
  DATE       date;
  S_TRANSOUT paramseq;

	read_date(&date);
	memcpy (&paramseq, param_out, sizeof (paramseq));

	//* check for first run : cold or warm reset ? 
	if (first_init (noappli,&chgt,&type) != FSE_FAILED){
		//if (chgt == 0xFF){   
		    //cold reset                                                  
			//raz_init(noappli);
        //}
    }
	
    hdlPowerOn();

	paramseq.noappli = noappli;
    memcpy (param_out, &paramseq, sizeof (paramseq));
	return (OK);

}

int is_state(NO_SEGMENT no,void *p1,S_ETATOUT *etatseq){
	S_ETATOUT etatout;
	int retour; 
	
	memcpy (&etatout, etatseq, sizeof(etatout));
	etatout.returned_state[etatout.response_number].state.response = REP_OK;
	memcpy (etatseq, &etatout, sizeof(etatout));
	retour = is_name (no, PT_NULL, etatseq);
	return (retour);
}

int is_name(NO_SEGMENT no,void *p1,S_ETATOUT *etatseq)
{
	S_ETATOUT etatout;
	memset(&etatout, 0, sizeof(etatout));
	
	memcpy ((char *)&etatout, (char *)etatseq, sizeof(etatout));
	strcpy((char *)etatout.returned_state[etatout.response_number].appname,(char *)appname);
	etatout.returned_state[etatout.response_number].no_appli = no;
	etatout.response_number++;
	memcpy (etatseq, &etatout, sizeof(etatout));
	return (FCT_OK);
}

int give_your_domain(NO_SEGMENT no,void *p1,S_INITPARAMOUT *param_out)
{
	S_INITPARAMOUT etatout;
	
	memcpy (&etatout, param_out, sizeof(etatout));//|MSK_SWIPE
	etatout.returned_state[etatout.response_number].mask     = MSK_MDP|MSK_TYPE_PPAD|MSK_PINPAD|MSK_STANDARD|MSK_LANGUE|MSK_FRMT_DATE|MSK_DATE;
	etatout.returned_state[etatout.response_number].application_type = TYP_EXPORT;
	etatout.response_number++;
	memcpy (param_out, &etatout, sizeof(etatout));
	return (FCT_OK);
}

int is_change_init(NO_SEGMENT noappli, void *p1, S_ETATOUT *param_out)
{
	S_ETATOUT etatout;
	int       retour;
	memcpy(&etatout, param_out, sizeof(etatout));
	// accept all 
	etatout.returned_state[etatout.response_number].state.mask=0;
	memcpy(param_out,&etatout,sizeof(etatout));
	retour = is_name (noappli, PT_NULL, param_out);
	return(FCT_OK);		 
}

static int getTrk(char *trk,const S_TRANSIN *src){
	const char *ptr;
	char *dst;
	byte len;
	VERIFY(trk);
	VERIFY(src);
	memcpy(trk+0*128,src->track1,sizeof(src->track1));
	
	ptr= src->track2;
	len= 0;
	if(*ptr++!='B')
		return -1;
	dst= trk+1*128;
	while(*ptr!='F'){
		if(len>=128)
			return -1;
		if(*ptr=='D')
			*dst++= '=';
		else
			*dst++= *ptr;
		len++;
		ptr++;
	}
	memcpy(trk+2*128,src->track3,sizeof(src->track3));
	return 128*3;
}

int debit_non_emv(NO_SEGMENT no,S_TRANSIN *param_in,S_TRANSOUT *param_out){
	S_TRANSIN card_in;
	unsigned char buf[128*3]; //trk1 + trk2 + trk3
	int ret;

	memset(buf,0, sizeof(buf));
	memcpy (&card_in, param_in, sizeof(card_in));
	ret= getTrk(buf,&card_in);
//	if(ret>0)
//		hdlMagCard(buf);

	return (FCT_OK);

}

int is_for_you_after(NO_SEGMENT no,S_TRANSIN * param_in, S_CARDOUT * param_out){
	S_TRANSIN card_in;
	int ret;
	int idx;
	unsigned char buf[128*3];//trk1 + trk2 + trk3

	memcpy (&card_in, param_in, sizeof(card_in));

	switch(card_in.support){
		case TRACK2_SUPPORT:
			memset(buf,0, sizeof(buf));
			idx= param_out->response_number;
			ret= getTrk(buf,&card_in);
			if(ret<0){
				param_out->returned_state[idx].cardappnumber = 0;
			}else{
				//ret = hdlAskMagCard(buf);
				param_out->returned_state[idx].cardappnumber = ret;
			}
			
			strcpy(param_out->returned_state[idx].appname, appname);
			param_out->returned_state[idx].cardapp[0].priority = CARD_PRIORITY;
			strcpy(param_out->returned_state[idx].cardapp[0].cardappname, appname);
			param_out->returned_state[idx].no_appli = no;
			param_out->response_number++;
		default:
			break;
	}

	return (FCT_OK);
}

int keyboard_event(NO_SEGMENT noappli,S_KEY *key_in,S_KEY *key_out){
	switch (key_in->keycode)
	{
	case N0: case N1: case N2: case N3: case N4: 
	case N5: case N6: case N7: case N8: case N9: 
	case T_VAL : case T_POINT :
		key_out->keycode = 0;               // Inhibit these keys to Manager for International domain
		break; 
	case F1 : case F2 : case F3 : case F4 : 
	case T_CORR : case T_ANN : case NAVI_CLEAR : case NAVI_OK : 
	case UP : case DOWN :  
	case T_F :                              // do not filter F key and return the same key ! 
		key_out->keycode=key_in->keycode;   // Return the same key value for keys above ! 
		break; 
	default :
		key_out->keycode=key_in->keycode;
		break;
	}

	//hdlKey (key_out->keycode);

	return (FCT_OK);
}
