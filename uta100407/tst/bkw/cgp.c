/** \file
 * Supporting functions for interfacing to CGUI
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/cgp.c $
 *
 * $Id: cgu.c 2566 2010-02-03 14:33:58Z bkwok $
 */


#include "SDK30.H"
#include "WGUI.h"
#include "WGUI_Plugin.h"
#include "sys.h"
#include "tst.h"
#include "cgp.h"
#include "SEC_interface.h"
#include "schVar_def.h"         // scheme includes
#include "tlvVar_def.h"
#include "cgu.h"


#ifdef __cplusplus
extern "C"
{
#endif
	
	
	//init secret area, signature card and var numbers
	//#define SST_S17 //profile S17
	//#define SST_DEBUG //profile T10
	#define SAT_DEBUG//exemple of profile T10 ICT familly
	#ifdef SAT_DEBUG
	#define VAR_NUMBER_SIGN		0x9A
	#define CARD_NUMBER_SIGN	0x0003C00E
	#define ID_SCR_XXX_BANK		0x009A2030
	#endif

	#ifdef SST_DEBUG
	#define VAR_NUMBER_SIGN		0xFA
	#define CARD_NUMBER_SIGN	0x0003C009
	#define ID_SCR_XXX_BANK		0x00FA2030
	#endif

	#ifdef SST_S17
	#define VAR_NUMBER_SIGN		0x9A
	#define CARD_NUMBER_SIGN	0x0003C039
	#define ID_SCR_XXX_BANK		0x009A2030
	#endif

	FILE *pKbd;
	FILE *pMse;
	CGUI_CONTEXT_STRUCT   CGUIContext;


static int show_pin_page(WGUI_BROWSER_HANDLE browser, char *str){
	int ret;
	char page[256];
	
	memset(page,0,sizeof(page));
	
	strcpy(page,"<html>");
	strcat(page,"<body onkeypress='app://action=0'>");
	strcat(page,"<h1>ENTER PIN</h1>");
	strcat(page,"<h1>");
	strcat(page,str);
	strcat(page,"</h1>");
	strcat(page,"</body>");
	strcat(page,"</html>");
	
	ret= WGUI_Browser_WriteHtmlAscii(browser, page,1);        
	
	return ret;
}

int InitSecurity()
{
	int ret;
	T_SEC_PARTTYPE zone = C_SEC_PINCODE;
	unsigned long loc = 8;
//	T_SEC_DATAKEY_ID rootKey;
	const byte *key = (byte *) "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11"; //clear key only for mockup terminals
	int usage= CIPHERING_PIN;
	T_SEC_DATAKEY_ID pinKey;

	
	ret = SEClib_Open();
	if (ret != 0)
		goto lblEnd;

	ret = SEC_isSecretArea(zone, (SEG_ID) ID_SCR_XXX_BANK);
	if (ret != OK) { //Secret Area is not created
		ret = SEC_CreateSecretArea( //give permission access for a secret area in a BL1 booster; If the SecureType has been configured to BL2 booster, no treatment is done.
				zone, //secureType Secure part to use for high level functions relative to the pincode entries
				(SEG_ID) ID_SCR_XXX_BANK, //Secret Area Id
				CARD_NUMBER_SIGN, //RSA card number
				VAR_NUMBER_SIGN //VAR Id number
				);
		if (ret != OK)
			goto lblEnd;
	}

//Root Key for non-mockup terminal must be loaded with SKMT2	
//	rootKey.iSecretArea = ID_SCR_XXX_BANK;
//	rootKey.cAlgoType = TLV_TYPE_KTDES;
//	rootKey.usNumber = 1008;
//	rootKey.uiBankId = 0x80000200 + ((1008) * 2)/8;

	zone = C_SEC_CIPHERING; //C_SEC_PINCODE;
	pinKey.iSecretArea = ID_SCR_XXX_BANK;
	pinKey.cAlgoType = TLV_TYPE_KTDES;
	pinKey.usNumber = loc * 8;
	pinKey.uiBankId = 0x80000200 + loc * 2;

	ret = SEC_LoadKey(zone, NULL, &pinKey, (unsigned char *) key, usage);
	if (ret != OK)
		goto lblEnd;

	ret = OK;
	goto lblEnd;
	lblKO: ret = -1;
	lblEnd: 
	return ret;
}
/** Handle Pin Entry Page
 * \param canvas (I) Canvas created by CGUI
 * \param browser (I) Browser created by CGUI
 * \param form (I) Pointer to a Form_Handle_t structure containing form properties
 * \sa Header: cgu.h
 * \sa Source: cgu.c
*/
int HandlePinEntryPage(WGUI_CANVAS_HANDLE canvas, WGUI_BROWSER_HANDLE browser, Form_Handle_t * form)
{
	unsigned int evt;
	int ret = 0;
	T_SEC_ENTRYCONF cfg;
	int ctl; //control parameter: to continue or not the PIN entry
	char key;
	byte idx = 0;
	char buf[16 + 1];
	char message[16+1];
	char msg[] = "CODE:";
	
	if (InitSecurity() != OK)
	{
		goto lblKO;
	}

	cfg.ucEchoLine = 2;
	cfg.ucEchoColumn = 0;
	cfg.ucEchoChar = '*';
	cfg.ucMinDigits = 4;
	cfg.ucMaxDigits = 12;
	cfg.iFirstCharTimeOut = 1000 * 60;
	cfg.iInterCharTimeOut = 1000 * 60;

	ret = SEC_PinEntryInit( //initialise the parameters for the secure pincode entry
			&cfg, //Pincode entry configuration
			C_SEC_PINCODE //(expected value for PIN offline processing by old versions of EMV-DC is C_SEC_CARD
			);
	if (ret != OK)
		goto lblKO;

	evt = KEYBOARD;
	ctl = 1;
	idx = 0;
	
	memset(message,0,sizeof(message));
	memset(buf, 0, sizeof(buf));
	strcpy(message, msg);
	show_pin_page(browser, message);
	
    // Write page to the display
//    WGUI_Browser_WriteHtmlAscii(browser, form->html_code, 1);

	// Wait page submission
	while (ctl) 
	{
		ret = SEC_PinEntry( //manage the secure pincode entry
				&evt, //Events to wait during the pin entry/Received events
				&key, //Last input key or 0
				&ctl //Input : FALSE to stop the pincode entry treatment; TRUE to continue the pincode entry; Output : TRUE if the pincode Entry is halted
				);
		if (ret != OK)
			goto lblKO;
		
		if (key == cfg.ucEchoChar) {
			if (idx < MAX_PIN_CODE_SIZE) {
				buf[idx++] = key;
				strcpy(message, msg);
				strcat(message+strlen(msg), buf);
				show_pin_page(browser, message);
			} else
				ctl = 0;
		} else {
			switch (key) {
			case 0x01: //CANCEL Response
			case T_ANN: // CANCEL Key
				ctl = 0;
				ret = 0;
				goto lblEnd;
				//break;
			case T_VAL:
				ctl = 0;
				break;
			case T_CORR:
				if (idx != 0) {
					buf[--idx] = 0;
					strcpy(message, msg);
					strcat(message+strlen(msg), buf);
					//strcat(message+strlen(msg)+strlen(buf), "      ");
					show_pin_page(browser, message);
					break;
					default:
					if (ret < 0)
						goto lblKO;
					break;
				}
			}
		}
		
		if (form->enable_msr)
		{
			if (ProcessSwipeEvent())
			{
				break;
			}
		}
	}
	
	//DisplayPinBlock();
	

	
	ret = OK;
	goto lblEnd;
	lblKO: ret = -1;
	lblEnd: 
	return ret;
}

int DisplayPinBlock()
{
	byte acc[8];
	byte blk[8];
	int len;
	T_SEC_DATAKEY_ID pinKey;
	int ret = 0;
	T_SEC_PARTTYPE zone = C_SEC_PINCODE;

	memcpy(acc, "\x00\x00\x78\x90\x12\x34\x56\x74", 8);
	
	ret = SEC_Iso9564(          //ISO9564 Pin ciphering (ANSI X9.8)
	           zone, //Secure part used to store the CipherKey
	           &pinKey,   //Key Id of the ciphering key
	           ISO9564_F0_TDES,   //ISO9564 format
	           (unsigned char *) acc, //PAN value (8 bytes)
	           blk,   //Output Data buffer
	           &len   //Length in bytes of the Output Data buffer
	  );
	if(ret != OK) 
		goto lblEnd;
	
//	ret = show_pin_block(blk,len);
	
	ret = OK;
	goto lblEnd;
	lblKO: ret = -1;
	lblEnd: 
	return ret;
}

#ifdef __cplusplus
}
#endif
