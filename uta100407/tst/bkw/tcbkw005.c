/** \file
 * Unitary non UTA test case for ...
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bkw/tcbkw005.c $
 *
 * $Id: tcbkw005.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include "SDK30.H"
#include "pp30_def.h"
#include "SEC_interface.h"
#include "schVar_def.h"
#include "tlvVar_def.h"
#include "tst.h"

extern int ManageMenu( const char *szTitle, int nItems, const char* Items[] );

#define C_NB_PARTS  3             // 1-pincode entries 2-card reader access 3-ciphering functions 
                                  // 4-keyboard entries (future) 5-display functions (future)
#define MAX_CHOICE  2
//#define NUMBER_OF_ITEMS(a) (sizeof(a)/sizeof((a)[0]))

#define ECHO_PIN   '*'

// Secret area identification
// ==========================
#define VAR_ID	           0x00         // Var Id from your RSA signing card
#define CARD_NB            0x0003C009   // Serial number from RSA signing card T10
#define AREA_ID	           0x00002030   // Secret area identification from 00002030 to 3FFF2F30

/*
#define VAR_ID   	       0x00         // Var Id from your RSA signing card
#define CARD_NB            0x0003C00F   // Serial number from RSA signing card S17
#define AREA_ID  	       0x00002030   // Secret area identification from 00002030 to 3FFF2F30
*/

// Key(s) Location                      // Location into secred area 
// ===============
#define ROOT_KEY_LOC	   126*8        // Root Key                   126 slot => 1008 position bit)
#define ISO9564PIN_KEY_LOC   1*8        // Iso9564Pin Key               1 slot =>    8 position bit)
#define MAC_KEY_LOC         11*8        // Mac Key                     11 slot =>   88 position bit)

// Bank(s) Id                            // Bank identification from 80000200 to FFFF0200
// ==========                            // related to a key loaded into secret area (2 last bytes)
#define BANK_ROOT_ID 		0x800002FC   // 126*8 => 128*2*4 => 256*4 => 0xFC*4
#define BANK_ISO9564_ID     0x80000202   // 1*8   => 1*2*4   => 2*4   => 0x02*4  
#define BANK_MAC_ID         0x80000216   // 11*8  => 11*2*4  => 22*4  => 0x16*4 

//****************************************************************************
//      PRIVATE TYPES                                                       
//****************************************************************************
    /* */

//****************************************************************************
//      PRIVATE DATA                                                        
//****************************************************************************
// SKMT2 T_DES (CBC mode)
// RootKey => (PartialKey1=213F2F5633B199A111A7F8BDEC6D6FBB) deciphered by CKey (PartialKey2=0123456789ABCDEFFEDCBA9876543210)  
const byte zRootKeyPlainText[] = "\x11\x11\x11\x11\x11\x11\x11\x11""\x22\x22\x22\x22\x22\x22\x22\x22";

const byte zIso9564PinKey[] = "\x40\xB3\x9B\xA8\x55\xE0\x41\x16";      // Iso9564PinKey ciphered by Root key => unciphered is 6B218F24DE7DC66C 
const byte zMacKey[] = "\x27\xB6\xA9\x4E\x50\xD4\xFA\x45";             // MacKey ciphered by Root key => unciphered is 0923456789ABCDEF

static const char *tzMenuChoice[]=
{
	"TERMINAL",
	"PINPAD"
};


static const char *tzMenuScheme[] =
{
	"Set Secure Config.",	
	"Get Secure Config.",
    "Create Secret Area",
	"Free Secret Area",
	"Load Iso9564 Key",
	"Iso9564 Pin Entry",
	"Load Mac Key",
	"Mac Calculation",
	0
};

static int iPpdBooster, iTermBooster;

//****************************************************************************
//                     int ChkPpdConfig (void)                            
//  This function checks the pinpad configuration from the Manager.      
//  This function has no parameters.
//  This function has return value.
//    >=0 : Right configuration (No pinpad=0 or pinpad present=1)
//     <0 : Wrong configuration (Wrong pinpad or pinpad missing but required=-1)                                
//****************************************************************************

static int ChkPpdConfig(void)
{
	// Local variables 
    // ***************
    char tcReference[17+1];
	int iRet;
		
    // Check pinpad configuration from Manager
    // ***************************************
    iRet = PSQ_Is_pinpad();
	if (iRet != 0)                                              // Manager asks for a pinpad
	{
		memset(tcReference, 0, sizeof(tcReference));
	    iRet = IsPPSConnected(); CHECK(iRet==RET_PP_OK, lblKO); // Check if pinpad is connected
		iRet=1;                                                 // Pinpad is connected
#ifndef WIN32
	    PPS_SendTerminalRefRequestCommand(tcReference);         // Get pinpad reference (Bug ret is always 0)    
		switch (PSQ_Pinpad_Value())
		{
		case PP30_PPAD:                                         // Pinpad characters no reader 					
			CHECK (memcmp(&tcReference[8], "PP30 ", 5)==0, lblKO);
			break;  
		case PP30S_PPAD:                                        // Pinpad characters no reader 
			CHECK (memcmp(&tcReference[8], "PP30S", 5)==0, lblKO);
			break; 
		case PPC30_PPAD:                                        // Pinpad graphic no reader 
			CHECK (memcmp(&tcReference[8], "PPC30", 5)==0, lblKO);
			break; 
		case PPR30_PPAD:                                        // Pinpad graphic reader 
			CHECK (memcmp(&tcReference[8], "PPR30", 5)==0, lblKO);
			break; 
		case P30_PPAD:                                          // Pinpad graphic reader  
			CHECK (memcmp(&tcReference[8], "P30  ", 5)==0, lblKO);
			break; 
		default:                                                // Pinpad unknown  
			iRet=-1;
			break; 
		}
#endif		
	}

	goto lblEnd;                                                // Right pinpad configuration (No pinpad or pinpad present)
	    	
	// Errors treatment 
	// ****************   
lblKO:                                                                 
    putchar('\x1B');                                            // Clear screen
    printf ("Pinpad Required\n"                                 // Pinpad required (pinpad disconnected or missing)
	   	    "Check Ppd Config");
    pprintf("\x1b""E""     Pinpad problem!!!\n\n" "\x1b""F");
    pprintf("------------------------");
    pprintf("Pinpad disconnected\n");
	pprintf("=> plug-it again\n");
    pprintf("---------- OR ----------");
    pprintf("Wrong pinpad connected\n");
	pprintf("=> Check MANAGER config\n");
    pprintf("---------- OR ----------");
    pprintf("Pinpad required\n");
	pprintf("=> Check MANAGER config\n");
    pprintf("------------------------");
    pprintf("\n\n\n\n\n\n");
	ttestall(PRINTER, 2*100);                                   // Wait until everything printed
    iRet=-1;                              
lblEnd: 
	return iRet;
}

//****************************************************************************
//                     int GetPpdDisplay (void)                            
//  This function returns the type of pinpad's display.      
//  This function has no parameters.
//  This function has return value.
//     2 : Pinpad with graphical display
//     1 : Pinpad with character display
//     0 : No pinpad connected
//    -1 : Processing error                                             
//****************************************************************************

int GetPpdDisplay(void) 
{ 
	// Local variables 
    // ***************
    int iDisplay;
    int iRet;
    
    // Check if pinpad declared from Manager and connected to the terminal
    // *******************************************************************
    iRet = PSQ_Is_pinpad(); CHECK(iRet!=0, lblKO);
	iRet = IsPPSConnected(); CHECK(iRet==RET_PP_OK, lblKO); 
		
	// Retrieve the type of display
	// ****************************
	iRet = PSQ_Pinpad_Value(); 
	switch (iRet)
	{
	case PP30_PPAD:          // Pinpad characters no reader 
	case PP30S_PPAD:         // Pinpad characters no reader
		iDisplay=1;          // Display = characters
        break;
	case PPC30_PPAD:         // Pinpad graphic with no reader
	case PPR30_PPAD:         // Pinpad graphic reader
	case P30_PPAD:           // Pinpad graphic reader
		iDisplay=2;          // Display = graphical
		break;
	default:
		iDisplay=-1;         // Pinpad unknown
		break;
	}

	goto lblEnd;
	
	// Errors treatment 
    // ****************    
lblKO:
    iDisplay=0;              // No pinpad connected
lblEnd: 
    return iDisplay;
}

//****************************************************************************
//                     int IsPinOnPpd (void)                            
//  This function checks if the Pin entry is done on terminal or on Pinpad.      
//  This function has no parameters.
//  This function has return value.
//     2 : Pin entry on PinPad with graphical display
//     1 : Pin entry on pinpad with character display
//     0 : Pin entry on terminal
//    -1 : Processing error                                              
//****************************************************************************

static int IsPinOnPpd(void)
{
	// Local variables 
    // ***************
	T_SEC_CONFIG txConfParts[C_NB_PARTS];
	int iRet, iNbParts;
	
	// Get the configuration of the security component
	// ***********************************************
	iRet = SEC_GetConfig(&iNbParts, txConfParts);  // Get number and parameters of secure parts
	CHECK(iRet==OK, lblKO);
	
	// Analyse C_SEC_PINCODE secure part
	// *********************************	
	iRet = GetPpdDisplay(); CHECK(iRet>=0, lblKO); // Pin entry on pinpad or terminal?
    if (iRet != 0)                                 // Pinpad connected
    	if (memcmp(txConfParts[0].ptszBoosterPeripheral, IAPP_USB_NAME, strlen(IAPP_USB_NAME)) != 0)
    		iRet=0;                                // Pin entry redirected to terminal 

	goto lblEnd;
	
	// Errors treatment
	// ****************
lblKO:
	iRet=-1;
lblEnd:
    return iRet;
}

//****************************************************************************
//         int PinEntry (const char *pcMsg1, const char *pcMsg2, 
//                       const char *pcMsg1Idle, const char *pcMsg2Idle, 
//                       byte ucTimeOut)                            
//  This function manages the pin entry.      
//       SEC_PinEntryInit : To initialise the parameters for the secure 
//                          Pin entry code
//       SEC_PinEntry : To manage the secure pincode entry
//  This function has parameters.
//    pcMsg1 (I-) : Message to display on first line
//    pcMsg2 (I-) : Message to display on second line
//    pcMsg1Idle (I-) : Idle message on first line
//    pcMsg2Idle (I-) : Idle message on second line
//    ucTimeOut (I-) : TimeOut (sec) Max 255 sec => 4,25 min
//  This function has return value.
//    >0 : Pin entry done 
//     0 : Timeout/Cancel
//    <0 : Pin entry failed                                              
//****************************************************************************

static int PinEntry(const char *pcMsg1, const char *pcMsg2, 
		            const char *pcMsg1Idle, const char *pcMsg2Idle, byte ucTimeOut)
{
	// Local variables 
    // ***************
	doubleword uiEvent=0, uiEveWait;
	char tcPin[16+1];
	int iContinue=TRUE;
	bool bExitPin=FALSE, bKeyEntry=FALSE, bEvent=FALSE;
	byte ucKey;	
	int iPpd, iPinSize, iRet;
	T_SEC_ENTRYCONF xEntryCfg;
	
	// Prompt messages on terminal or PinPad
	// *************************************
    iPpd = IsPinOnPpd(); CHECK(iPpd>=0, lblKO);
    switch (iPpd)
    {
    case 0:                                                                                    // *** Pin on terminal
    	_clrscr();                                                                             // Clear graphic screen
    	_DrawLine(0, 16, 127, 16, _ON_);
    	_DrawLine(0, 17, 127, 17, _ON_);
		_DrawExtendedString(0, 22, (char*) pcMsg1, _OFF_, _MEDIUM_, _NORMALE_);                // First message on terminal
		_DrawExtendedString(0, 35, (char*) pcMsg2, _OFF_, _MEDIUM_, _NORMALE_);                // Second message on terminal
		_DrawLine(0, 49, 127, 49, _ON_);
		_DrawLine(0, 50, 127, 50, _ON_);		
		PaintGraphics();                                                               
    	uiEvent=0;                                                                             // Wait for timeout
    	break;
    case 1:                                                                                    // *** Pin on pinpad with characters display
        font_size (8,16); putchar('\x1B');                                                     // Normal font size
    	iRet = printf("Pin Entry\n""On PinPad..."); CHECK(iRet>=0, lblKO);                     // Display messages on terminal
    	PPS_firstline();
    	PPS_Display((char*) pcMsg1);                                                           // First message on pinpad  
    	PPS_newline();
    	PPS_Display((char*) pcMsg2);                                                           // Second message on pinpad     	
    	uiEvent=KEYBOARD;                                                                      // Wait for EFT keyboard to cancel pin entry on pinpad
    	break;
    case 2:                                                                                    // *** Pin on pinpad with graphical display
        font_size (8,16); putchar('\x1B');                                                     // Normal font size
    	iRet = printf("Pin Entry\n""On PinPad..."); CHECK(iRet>=0, lblKO);                     // Display messages on terminal
    	InitContext(PERIPH_PPR);                                                               // Switch to graphic display pinpad
		_DrawExtendedString(0, 22, (char*) pcMsg1, _OFF_, _MEDIUM_, _NORMALE_);                // First message on pinpad
		_DrawExtendedString(0, 35, (char*) pcMsg2, _OFF_, _MEDIUM_, _NORMALE_);                // Second message on pinpad
		PaintGraphics();                                                               
    	uiEvent=KEYBOARD;                                                                      // Wait for EFT keyboard to cancel pin entry on pinpad
    	break;	
    default:
    	break;
    }

	// Pin entry code initialisation
	// *****************************
    while (!bExitPin)
    {
    	xEntryCfg.ucEchoChar = ECHO_PIN;                                                       // '*' echo character
    	xEntryCfg.ucMinDigits = 4;                                                             // Min pin entry 4 digits
    	xEntryCfg.ucMaxDigits = 4;                                                             // Max pin entry 4 digits
    	if (ucTimeOut < 60)
    	{
    		xEntryCfg.iFirstCharTimeOut = ucTimeOut*1000;                                      // Wait for first digit < 60s
    		ucTimeOut=0;
    	}
    	else
    	{
    		xEntryCfg.iFirstCharTimeOut = 60*1000;                                             // Wait for first digit 60s (Max supported)
    		ucTimeOut -= 60;
    	}
    	xEntryCfg.iInterCharTimeOut = 10*1000;                                                 // Wait for next digits 10s
	    // Secure part relative to the Pin entry function    
	    iRet = SEC_PinEntryInit (&xEntryCfg, C_SEC_PINCODE); CHECK(iRet==OK, lblKO);           // C_SEC_PINCODE secure part recommended to pin entry (PCI/PED)

	    // Pin entry code management
	    // *************************
	    iPinSize=0;
        strcpy (tcPin, pcMsg2);  
    	uiEveWait=uiEvent;              // Init ttestall (from SEC_PinEntry) with the right peripheral events        
        while (!bExitPin) 
	    {
        	iRet = SEC_PinEntry (&uiEveWait, &ucKey, &iContinue); 
		
		    if (iRet == OK)
		    {
		    	if (ucKey == ECHO_PIN)
		    	{
				    // Enter pin in progress
				    // =====================
		    		if (iPinSize < MAX_PIN_CODE_SIZE)
		    		{
		    			bKeyEntry=TRUE;
		    			tcPin[strlen(pcMsg2)+iPinSize] = ucKey;
					    tcPin[strlen(pcMsg2)+iPinSize+1] = 0;
					    switch (iPpd)
				        {
				        case 0:                                                                // *** Pin on terminal
				        	DrawExtendedString(0, 35, tcPin, _OFF_, _MEDIUM_, _NORMALE_);      // Display pin code entry
				    	    break;
				        case 1:                                                                // *** Pin on pinpad with characters display
				        	PPS_clearline(); PPS_Display(tcPin); 				               // Display pin code entry	
				        	break;
				        case 2:                                                                // *** Pin on pinpad with graphical display
						    DrawExtendedString(0, 35, tcPin, _OFF_, _MEDIUM_, _NORMALE_);      // Display pin code entry				    	
				    	    break;	
				        default:
				    	    break;
				        }
					    iPinSize++;
					}
		        	uiEveWait=uiEvent; // Re-init ttestall (from SEC_PinEntry) with the right peripheral events
			    }
			    else
			    {
				    // Pin confirmation
				    // ================				
				    switch (ucKey) 
				    {
				    case 0x00:                   // Response to stop pin entry by receiving an event 
						if(uiEveWait & KEYBOARD)     // Key event from terminal
						{
				    		getchar();               // Exit on terminal key pressed
    						_DrawExtendedString(0, 22, "EVENT           ", _OFF_, _MEDIUM_, _NORMALE_);                
    						_DrawExtendedString(0, 35, "KEY PRESSED     ", _OFF_, _MEDIUM_, _NORMALE_);                
    						PaintGraphics();
    						ttestall(0, 2*100);
    						bEvent=TRUE;
						}
						iRet=0;
				    	break; 				    
				    case 0x01:   iRet=0; break;
				    case T_VAL:  iRet=1; break; // Valid key from Pinpad or Terminal when enter pin => iContinue=FALSE if >= Min pin entry
				    case T_ANN:  iRet=0; break; // Cancel key from Pinpad or Terminal when enter pin => iContinue=FALSE
				    case T_CORR:                // Correction from Pinpad or Terminal when enter pin => iContinue=TRUE
				    	if (iPinSize != 0)
					    {
				    		iPinSize--;
						    tcPin[strlen(pcMsg2)+iPinSize] = ' ';
						    tcPin[strlen(pcMsg2)+iPinSize+1] = 0;
						    switch (iPpd)
					        {
					        case 0:                                                            // *** Pin on terminal
					        	DrawExtendedString(0, 35, tcPin, _OFF_, _MEDIUM_, _NORMALE_);  // Pin entry code correction				    	
					    	    break;
					        case 1:                                                            // *** Pin on pinpad with characters display
					    	    PPS_clearline(); PPS_Display(tcPin); 				           // Pin entry code correction	
					    	    break;
					        case 2:                                                            // *** Pin on pinpad with graphical display
							    DrawExtendedString(0, 35, tcPin, _OFF_, _MEDIUM_, _NORMALE_);  // Pin entry code correction				    	
					    	    break;	
					        default:
					    	    break;
					        }
					    }
                        break; 
				    default:             break; 
				    }
				}
		    }
		    else if (iRet == ERR_TIMEOUT)                                                      // Pin entry on pinpad and canceled by terminal
			    bExitPin=TRUE;
		    else                                                                               // Pin entry on pinpad already in progress
		    {
			    iRet=-1;
			    bExitPin=TRUE;
		    }
		
		    // Pin exit process
		    // ================
		    if (iContinue  == FALSE)                                                           // The Pin entry is already stopped
		    	bExitPin=TRUE;                                                                 // exit immediatly
		    else if (bExitPin == TRUE)                                                         // The Pin entry is not stopped
		    {
			    iContinue=FALSE;                                                               // To stop the Pin entry
			    bExitPin=FALSE;                                                                // loop one more time before exit
		    }		
	    }
        
    	if ((ucTimeOut != 0) && (bKeyEntry == FALSE) && (bEvent == FALSE))                          // No Key entry, loop until full timeout expired 
    		bExitPin=FALSE;
    }	
    goto lblEnd;
    
	// Errors treatment 
    // ****************	
lblKO:
    iRet=-1;                                                                                   // Pin entry failed
lblEnd:   
    switch (iPpd)
    {
    case 0:                                                                                    // *** Pin on terminal
        font_size (8,16); putchar('\x1B');                                                     // Normal font size
    	break;
    case 1:                                                                                    // *** Pin on pinpad with characters display
    	PPS_firstline();
    	PPS_Display((char*) pcMsg1Idle);                                                       // Idle message 
    	PPS_newline();
    	PPS_Display((char*) pcMsg2Idle);                                                       // Second message on pinpad     	
    	break;
    case 2:                                                                                    // *** Pin on pinpad with graphical display
		_DrawExtendedString(0, 22, (char*) pcMsg1Idle, _OFF_, _MEDIUM_, _NORMALE_);            // Idle message
		_DrawExtendedString(0, 35, (char*) pcMsg2Idle, _OFF_, _MEDIUM_, _NORMALE_);          
		PaintGraphics(); 
        InitContext(PERIPH_DISPLAY);                                                           // Switch to graphic display terminal
    	break;	
    default:
    	break;
    }

	return iRet;			
}

//****************************************************************************
//                          void SetConfig(void)                            
//  This function sets the configuration of the security component for the 
//  current application.      
//       SET_SetConfig() : To configure the security component.
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void SetConfig(void)
{
	// Local variables 
    // ***************
    T_SEC_CONFIG txConfParts[C_NB_PARTS];
	int i, iNbParts, iDefItem; 
	char *pcTitle;
	int iRet;

	// Change font size
	// ================
    font_size (8,16);                                                         // Normal font size
	putchar('\x1B');                                                          // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll);              // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);                            // Pinpad problem? => check config from Manager

	// Get the configuration parts on the security component
	// *****************************************************
	iRet = SEC_GetConfig(&iNbParts, txConfParts); CHECK(iRet==OK, lblKO);     // Get configuration parts by default    
    for (i=0; i<iNbParts; i++)
	{
		if ( memcmp (txConfParts[i].ptszBoosterPeripheral, IAPP_USB_NAME, strlen(IAPP_USB_NAME)) == 0)
			iPpdBooster = txConfParts[i].cBoosterType;                        // Pinpad booster type 1/2
		else
			iTermBooster = txConfParts[i].cBoosterType;                       // Terminal booster type 1/2/3
	}
    
	// Set the configuration parts on the security component
	// *****************************************************
	for (i=0; i<C_NB_PARTS; i++)
	{

		iRet = ChkPpdConfig(); 
		if (iRet > 0)                                                         // Pinpad declared in Manager and connected?
		{
			switch (txConfParts [i].SecureType)                               // Yes, Prompt for switching some secure parts 
			{
			case C_SEC_PINCODE  : pcTitle = "PIN ENTRY ?";      break;        // Pin on terminal or pinpad? 
			case C_SEC_CARD     : pcTitle = "CARD READER ?";    break;        // Card on terminal or pinpad?
			case C_SEC_CIPHERING: pcTitle = "DATA CIPHERING ?"; break;        // Ciphering on terminal or pinpad?
			default:              pcTitle = "SECURE TYPE ?";    break;
			}
				
			switch (txConfParts [i].SecureType)                               // Select the secure parts to switch
			{
			case C_SEC_PINCODE :
			case C_SEC_CARD :
			case C_SEC_CIPHERING :
				if (memcmp (txConfParts[i].ptszBoosterPeripheral, IAPP_USB_NAME, strlen(IAPP_USB_NAME)) == 0)
					iDefItem=1;
				else
					iDefItem=0;
				iRet = ManageMenu((const char*) pcTitle, 2, tzMenuChoice); 
				switch (iRet) 
				{   
				case 0:
					txConfParts[i].cBoosterType = (char) iTermBooster;        // Booster type 1/2/3
					txConfParts[i].ptszBoosterPeripheral = IAPP_DEF_NAME;     // DEF_NAME internal booster 
                    break; 
				case 1:
					txConfParts[i].cBoosterType = (char) iPpdBooster;         // Booster type 1/2
					txConfParts[i].ptszBoosterPeripheral = IAPP_USB_NAME;     // USB_NAME external booster 
                    break;
				default:
					goto lblEnd;                                              // Cancel/Timeout
				}
				break;	
			default:
				goto lblKO;                                                   // Processing error
			}
		}
	}
	
	iRet = SEC_SetConfig (C_NB_PARTS, txConfParts); CHECK(iRet==OK, lblKO);   // Set configuration parts
    font_size (8,16);                                                         // Normal font size
	putchar('\x1B');                                                          // Clear screen
	gotoxy(0,0); iRet = printf("Set Security"); CHECK(iRet>=0, lblKO);        // Display message
	gotoxy(1,0);  iRet = printf("Config. done..."); CHECK(iRet>=0, lblKO);    // Display message
	ttestall(0, 5*100); 
	
	goto lblEnd;
	
	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                                          // Clear screen
	printf("Missing Sec DLL ");                                               // Security DLL missing
	ttestall(0, 5*100);                                                       // Delay 5s 
	goto lblEnd;	
lblKO:
	putchar('\x1B');                                                          // Clear screen
	printf("Processing Error");                                               // Oops! error
    ttestall(0, 5*100);                                                       // Delay 5s 
lblEnd:
    return;
}

//****************************************************************************
//                          void GetConfig(void)                            
//  This function gets the configuration of the security component for the 
//  current application.      
//       SET_GetConfig() : Configuration of the security component.
//       SEC_listSecureId() : List Ids of loaded schemes and secret areas
//                            in a secure part.
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void GetConfig(void)
{
	// Local variables 
    // ***************
	T_SEC_CONFIG txConfParts[C_NB_PARTS];
	int iNbParts, iNbId, tiList[10];
	char cBooster;
	int i, i1, iRet;

	// Change font size
	// ================
    font_size (8,16);                                              // Normal font size
	putchar('\x1B');                                               // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll);   // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);                 // Pinpad problem? => check config from Manager
	iRet = printf("Info Security..."); CHECK(iRet>=0, lblKO);      // Display message

	// Get the configuration of the security component
	// ===============================================
    iRet = SEC_GetConfig(&iNbParts, txConfParts); CHECK(iRet==OK, lblKO);
    iRet = pprintf("\x1b""E""      Security Config.\n\n"           // Print security parts
		           "\x1b""F"); CHECK(iRet>=0, lblKO);          
	iRet = pprintf ("Nbr secure parts: %d\n", iNbParts); CHECK(iRet>=0, lblKO);
    for (i=0; i<iNbParts; i++)
	{
		switch (txConfParts[i].SecureType)
		{
		case C_SEC_PINCODE:   iRet = pprintf("\n-PIN ENTRY: ");   break;  
		case C_SEC_CARD:      iRet = pprintf("\n-CARD READER: "); break;
		case C_SEC_CIPHERING: iRet = pprintf("\n-CIPHERING: ");   break;
		default:              iRet = pprintf("\n-SecurePart: %d ", txConfParts[i].SecureType); break; 
		}
		CHECK(iRet>=0, lblKO);                                     // Get device type Pinpad or Terminal
		if (memcmp (txConfParts[i].ptszBoosterPeripheral, IAPP_USB_NAME, strlen(IAPP_USB_NAME)) == 0)
		{
			iRet = pprintf("PINPAD"); CHECK(iRet>=0, lblKO);
		}
		else
		{
			iRet = pprintf("TERMINAL"); CHECK(iRet>=0, lblKO);
		}
		cBooster = txConfParts[i].cBoosterType;                    // Get booster type (B1, B2, B3)     
		switch (cBooster)         
		{
		case C_SEC_BL1: pprintf("\nBooster Type=1\n"); CHECK(iRet>=0, lblKO); break;
		case C_SEC_BL2: pprintf("\nBooster Type=2\n"); CHECK(iRet>=0, lblKO); break;
		default:        pprintf("\nBooster Type=3\n"); CHECK(iRet>=0, lblKO); break;
		}
		if (txConfParts[i].cbGestResid == TRUE)                   // Get schemes process (resident or not)
		{
			iRet = pprintf("Resident schemes enable\n"); CHECK(iRet>=0, lblKO);
		}
		else
		{
			iRet = pprintf("Resident schemes disable\n"); CHECK(iRet>=0, lblKO);
		}

		// Get the list of Ids of loaded schemes and secret areas in a secure part
		// =======================================================================
		memset (tiList, 0, sizeof(tiList)); iNbId=0;
		iRet = SEC_listSecureId (txConfParts[i].SecureType, &iNbId, tiList); CHECK(iRet==OK, lblKO);
		iRet = pprintf ("Secret Area Ids:\n"); CHECK(iRet>=0, lblKO);
		for (i1=0; i1<iNbId; i1++)                               // Get list of secret areas
		{
			iRet = pprintf ("%08x\n", tiList[i1]); CHECK(iRet>=0, lblKO);
		}
	}

	iRet = pprintf("\n\n\n\n\n\n"); CHECK(iRet>=0, lblKO);

	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                             // Clear screen
	printf("Missing Sec DLL ");                                  // Security DLL missing
	ttestall(0, 5*100);                                          // Delay 5s 
    goto lblEnd;
lblKO:
	putchar('\x1B');                                             // Clear screen
	printf("Processing Error");                                  // Oops! error
    ttestall(0, 3*100);                                          // Delay 3s 
lblEnd:
	ttestall(PRINTER, 2*100);   
}

//****************************************************************************
//                          void CreateSecret(void)                            
//  This function creates a secret area and loads the Root key.      
//       SEC_CreateSecretArea() : To create a secret area
//       SEC_LoadKey() : To load a key in secret area
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void CreateSecret(void)
{
	// Local variables 
    // ***************
	byte tucChkSum[3];
	T_SEC_DATAKEY_ID xRootKey;
	doubleword uiLen;
	int iRet;

	// Initialisations
	// ***************
    font_size (8,16);                                             // Normal font size
	putchar('\x1B');                                              // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll);  // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);                // Pinpad problem? => check config from Manager   
	iRet = printf("CreateSecretArea"); CHECK(iRet>=0, lblKO);     // Display message
	
    // Create Secret Area
	// ******************
	iRet = SEC_CreateSecretArea (C_SEC_PINCODE,                   // C_SEC_PINCODE secure part recommended (PCI/PED)
		  		                 (SEG_ID) AREA_ID,                // Secret area identification
								 CARD_NB,                         // Serial number from you RSA signing card
								 VAR_ID);                         // Var Id from your RSA signing card 
	CHECK(iRet==OK, lblKO);
	
	// Load Root Key
	// *************
	xRootKey.iSecretArea = AREA_ID;                               // Secret area identification
	xRootKey.cAlgoType   = TLV_TYPE_KTDES;                        // This ROOT key is a TDES Key 
	xRootKey.usNumber    = ROOT_KEY_LOC;                          // ROOT key location inside the secret area
	xRootKey.uiBankId    = BANK_ROOT_ID;                          // Bank id related to this ROOT key

    iRet = SEC_LoadKey (C_SEC_PINCODE,                            // C_SEC_PINCODE secure part recommended (PCI/PED)   
					    NULL, 
	                    &xRootKey,                                // ROOT key parameters
	                    (unsigned char*) zRootKeyPlainText,       // The ROOT key value 
	                    CIPHERING_KEY);                           // ROOT key usage : to cipher PIN key or MAC key 
	CHECK(iRet==OK, lblRootKO);
    
    // Root Key CheckSum
    // *****************    
    iRet = SEC_KeyVerify (C_SEC_PINCODE,                          // C_SEC_PINCODE secure part recommended (PCI/PED)                           
    		              &xRootKey,                              // ROOT key parameters
    		              tucChkSum,                              // ROOT key checksum
    		              &uiLen);                                // Length checksum

    gotoxy(1,0);
	iRet = printf("Area done...\n"                                // Display message
	              "CheckSum: %02X%02X%02X", tucChkSum[0], tucChkSum[1], tucChkSum[2]);              
    CHECK(iRet>=0, lblKO);
	ttestall(0, 5*100);                                           // Delay 5s 

	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                              // Clear screen
	printf("Missing Sec DLL ");                                   // Security DLL missing
	ttestall(0, 5*100);                                           // Delay 5s 
	goto lblEnd;
lblRootKO:
    putchar('\x1B');                                              // Clear screen
    printf("Load Root Key KO");                                   // Load Root Key failed 
    ttestall(0, 5*100);                                           // Delay 5s
    goto lblEnd;
lblKO:
	putchar('\x1B');                                              // Clear screen
	printf("Processing Error");                                   // Oops! error
    ttestall(0, 5*100);                                           // Delay 5s 
lblEnd:
    return;
}

//****************************************************************************
//                          void FreeSecret(void)                            
//  This function is used to kill the secret area.      
//       SEC_FreeSecret() : To free a secret data or the whole secret area.
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void FreeSecret(void)
{
	// Local variables 
    // ***************
	T_SEC_DATAKEY_ID xEraseKey;
	int iRet;

	// Initialisations
	// ***************
    font_size (8,16);                                            // Normal font size
	putchar('\x1B');                                             // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll); // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);               // Pinpad problem? => check config from Manager    
	iRet = printf("FreeSecretArea"); CHECK(iRet>=0, lblKO);      // Display message

	// Erase Secret Area
	// *****************
	xEraseKey.iSecretArea = AREA_ID;                             // Secret area identification
	xEraseKey.cAlgoType = 0;
	xEraseKey.usNumber = 0;
	xEraseKey.uiBankId = 0;
    iRet = SEC_FreeSecret (C_SEC_PINCODE,                        // C_SEC_PINCODE secure part recommended (PCI/PED)     
    		               &xEraseKey);                          // Secret area to erase
    CHECK(iRet==OK, lblKO);

    gotoxy(1,0);
	iRet = printf("Area free..."); CHECK(iRet>=0, lblKO);        // Display message
	ttestall(0, 3*100);                                          // Delay 3s 

	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                             // Clear screen
	printf("Missing Sec DLL ");                                  // Security DLL missing
	ttestall(0, 5*100);                                          // Delay 5s 
	goto lblEnd;
lblKO:
	putchar('\x1B');                                             // Clear screen
	printf("Processing Error");                                  // Oops! error
    ttestall(0, 5*100);                                          // Delay 5s 
lblEnd:
    return;
}

//****************************************************************************
//                   void LoadIso9564Key(void)                            
//  This function loads Iso9564 pin key inside the secret area by using the 
//  Root key. 
//       SEC_LoadKey() : To load a key in secret area
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void LoadIso9564Key(void)
{
	// Local variables 
    // ***************
	byte tucChkSum[3];
	T_SEC_DATAKEY_ID xRootKey, xPinKey;
	doubleword uiLen;
	int iRet;

	// Initialisations
	// ===============
    font_size (8,16);                                            // Normal font size
	putchar('\x1B');                                             // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll); // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);               // Pinpad problem? => check config from Manager   
	iRet = printf("LoadPinKey"); CHECK(iRet>=0, lblKO);          // Display message

	// Load Pin Key using Root Key
	// ***************************
	xRootKey.iSecretArea = AREA_ID;                              // Secret area identification
	xRootKey.cAlgoType   = TLV_TYPE_KTDES;                       // This ROOT key is a TDES Key 
	xRootKey.usNumber    = ROOT_KEY_LOC;                         // ROOT key location inside the secret area
	xRootKey.uiBankId    = BANK_ROOT_ID;                         // Bank id related to this ROOT key

	xPinKey.iSecretArea = AREA_ID;                               // Secret area identification
	xPinKey.cAlgoType   = TLV_TYPE_KDES;                         // This PIN key is a DES Key 
	xPinKey.usNumber    = ISO9564PIN_KEY_LOC;                    // PIN key location inside the secret area
	xPinKey.uiBankId    = BANK_ISO9564_ID;                       // Bank id related to this PIN key

	iRet = SEC_LoadKey (C_SEC_PINCODE,                           // C_SEC_PINCODE secure part recommended for loading ISO9564Pin key (PCI/PED) 
					    &xRootKey,                               // ROOT key parameters
					    &xPinKey,                                // PIN key parameters
					    (unsigned char*) zIso9564PinKey,         // The ISO9564 PIN key value
					    CIPHERING_PIN);                          // PIN key usage : to cipher PIN entry 
	CHECK(iRet==OK, lblKO);

    // Pin Key CheckSum
    // ****************		
    iRet = SEC_KeyVerify (C_SEC_PINCODE,                          // C_SEC_PINCODE secure part recommended (PCI/PED)                           
    		              &xPinKey,                               // PIN key parameters
    		              tucChkSum,                              // PIN key checksum
    		              &uiLen);                                // Length checksum
    gotoxy(1,0);
	iRet = printf("Key loaded...\n"                               // Display message
	              "CheckSum: %02X%02X%02X", tucChkSum[0], tucChkSum[1], tucChkSum[2]);              
    CHECK(iRet>=0, lblKO);
    ttestall(0, 3*100);                                           // Delay 3s 

	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                             // Clear screen
	printf("Missing Sec DLL ");                                  // Security DLL missing
	ttestall(0, 5*100);                                          // Delay 5s 
	goto lblEnd;	
lblKO:
	putchar('\x1B');                                             // Clear screen
	printf("Processing Error");                                  // Oops! error
    ttestall(0, 5*100);                                          // Delay 5s 
lblEnd:
    return;
}

//****************************************************************************
//                          void LoadMacKey(void)                            
//  This function loads the Mac key inside the secret area by using the Root key.
//       SEC_LoadKey() : To load a key in secret area
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void LoadMacKey(void)
{
	// Local variables 
    // ***************
	byte tucChkSum[3];
	T_SEC_DATAKEY_ID xRootKey, xMacKey;
	doubleword uiLen;
	int iRet;

	// Initialisations
	// ***************
    font_size (8,16);                                            // Normal font size
	putchar('\x1B');                                             // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll); // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);               // Pinpad problem? => check config from Manager
	iRet = printf("LoadMacKey"); CHECK(iRet>=0, lblKO);          // Display message

	// Load Mac Key using Root Key
	// ***************************
	xRootKey.iSecretArea = AREA_ID;                              // Secret area identification
	xRootKey.cAlgoType   = TLV_TYPE_KTDES;                       // This ROOT key is a TDES Key 
	xRootKey.usNumber    = ROOT_KEY_LOC;                         // ROOT key location inside the secret area
	xRootKey.uiBankId    = BANK_ROOT_ID;                         // Bank id related to this ROOT key

	xMacKey.iSecretArea = AREA_ID;                               // Secret area identification
	xMacKey.cAlgoType   = TLV_TYPE_KDES;                         // This MAC key is a DES Key 
	xMacKey.usNumber    = MAC_KEY_LOC;                           // MAC key location inside the secret area
	xMacKey.uiBankId    = BANK_MAC_ID;                           // Bank id related to this MAC key

	iRet = SEC_LoadKey (C_SEC_CIPHERING,                         // C_SEC_CIPHERING secure part chosen without any recommendation to load MAC key     
					    &xRootKey,                               // ROOT key parameters
					    &xMacKey,                                // MAC key parameters
					    (unsigned char*) zMacKey,                // The MAC key value
					    CIPHERING_DATA);                         // MAC key usage : to cipher Data Message
	CHECK(iRet==OK, lblKO);

    // MAC Key CheckSum
    // ****************	
    iRet = SEC_KeyVerify (C_SEC_PINCODE,                          // C_SEC_PINCODE secure part recommended (PCI/PED)                           
    		              &xMacKey,                               // MAC key parameters
    		              tucChkSum,                              // MAC key checksum
    		              &uiLen);                                // Length checksum
    gotoxy(1,0);
	iRet = printf("Key loaded...\n"                               // Display message
	              "CheckSum: %02X%02X%02X", tucChkSum[0], tucChkSum[1], tucChkSum[2]);              
    CHECK(iRet>=0, lblKO);
    ttestall(0, 5*100);                                           // Delay 5s 

	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                             // Clear screen
	printf("Missing Sec DLL ");                                  // Security DLL missing
	ttestall(0, 5*100);                                          // Delay 5s 
	goto lblEnd;
lblKO:
	putchar('\x1B');                                             // Clear screen
	printf("Processing Error");                                  // Oops! error
    ttestall(0, 3*100);                                          // Delay 3s 
lblEnd:
    return;
}

//****************************************************************************
//        int Iso9564PinCiphering(const byte *pucAcc, byte *pucBlk)                            
//  This function enciphers the Pin entry by using the PIN key loaded in the
//  secret area to calculate the Pinblock.       
//       SEC_Iso9564() : ISO 9564 Pin cyphering  (ANSI X9.8)
//  This function has parameters.
//    pucAcc (I-) : Account number
//    pucBlk (-O) : Pinblock
//  This function has return value.
//    =0 : Done
//    !0 : Failed
//****************************************************************************

static int Iso9564PinCiphering(const byte *pucAcc, byte *pucBlk)
{
	// Local variables 
    // ***************
	doubleword uiLengthOut;
	T_SEC_DATAKEY_ID xKey;
	int iRet;

    // ISO9564 Pin ciphering
	// *********************
	xKey.iSecretArea = AREA_ID;                   // Secret area identification
	xKey.cAlgoType = TLV_TYPE_KDES;               // PIN key is a DES Key 
	xKey.usNumber = ISO9564PIN_KEY_LOC;           // PIN key location inside the secret area 
	xKey.uiBankId = BANK_ISO9564_ID;              // Bank id related to this PIN key 

	iRet = SEC_Iso9564 (C_SEC_PINCODE,            // C_SEC_PINCODE secure part recommended to cipher/decipher Iso9564Pin entry (PCI/PED)
			            &xKey,                    // PIN key parameters
			            ISO9564_F0_DES,           // Ciphering format           
			            (unsigned char *) pucAcc, // Account number (last 12 digits without luhn)
					    pucBlk,                   // Pinblock result 
					    &uiLengthOut);
	
	return iRet;
}

//****************************************************************************
//                     void Iso9564PinEntry(void)                            
//  This function manages the Pin entry (online) to get the Pinblock.      
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void Iso9564PinEntry(void)
{
	// Local variables 
    // ***************	
	char *pcMsg1, *pcMsg2, *pcMsg1Idle, *pcMsg2Idle;
	byte tucAcc[8];
    byte tucBlk[8];
	int i, iRet;

	// Initialisations
	// ***************
    font_size (8,16);                                                  // Normal font size
	putchar('\x1B');                                                   // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll);       // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);                     // Pinpad problem? => check config from Manager

    // Enter PIN
	// *********
	pcMsg1 = "AMT     1000 EUR";
	pcMsg2 = "PIN: ";
	pcMsg1Idle = "    WELCOME     ";
	pcMsg2Idle = "                ";
	iRet = PinEntry (pcMsg1, pcMsg2, pcMsg1Idle, pcMsg2Idle, 90);      // Pin entry with timeout 90sec
	CHECK(iRet!=0, lblEnd); CHECK(iRet>=0, lblKO);
	
	// PIN ciphering to get pinblock
	// *****************************
	memcpy(tucAcc, "\x00\x00\x78\x90\x12\x34\x56\x74", 8);
	iRet = Iso9564PinCiphering(tucAcc, tucBlk); CHECK(iRet==OK, lblKO);

	// Print and display pinblock
	// **************************
    iRet = pprintf("\x1b""E""       Pin Entry Demo\n\n"                // Print demo in progress
		           "\x1b""F"); CHECK(iRet>=0, lblKO);      
    iRet = pprintf("Account Number:\n"); CHECK(iRet>=0, lblKO);
	for (i=0; i<8; i++)                                                // Print account number
	{
		iRet = pprintf ("%02x ", tucAcc[i]); CHECK(iRet>=0, lblKO);
	}
    iRet = pprintf("\nPin Key:\n"                                      // Print pin key
	               "6B 21 8F 24 DE 7D C6 6C\n"); CHECK(iRet>=0, lblKO);
    iRet = pprintf("Pinblock:\n"); CHECK(iRet>=0, lblKO);
	for (i=0; i<8; i++)                                                // Print pinblock
	{
		iRet = pprintf ("%02X ", tucBlk[i]); CHECK(iRet>=0, lblKO);
	}

	iRet = pprintf("\n\n\n\n\n\n\n"); CHECK(iRet>=0, lblKO);

	putchar('\x1B');                                                   // Clear screen
	gotoxy(0,0); iRet = printf("PinBlock:"); CHECK(iRet>=0, lblKO);  
	gotoxy(1,0);
	for (i=0; i<8; i++)
	{
		iRet = printf ("%02X", tucBlk[i]); CHECK(iRet>=0, lblKO);      // Display pinblock
	}

	ttestall(0, 5*100);                                                // Delay 5s
	
	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	putchar('\x1B');                                                   // Clear screen
	printf("Missing Sec DLL ");                                        // Security DLL missing
	ttestall(0, 5*100);                                                // Delay 5s 
	goto lblEnd;
lblKO:
	putchar('\x1B');                                                   // Clear screen
	printf("Processing Error");                                        // Oops! error
    ttestall(0, 5*100);                                                // Delay 5s 
lblEnd:  
	ttestall(PRINTER, 10);   
}

//****************************************************************************
//                       void MacCalculation(void)                            
//  This function manages the Mac calculation on a message.      
//  This function has no parameters.    
//  This function has no return value.                                      
//****************************************************************************

void MacCalculation(void)
{
	// Local variables 
    // ***************	
    T_SEC_DATAKEY_ID xMacKey;
	const unsigned char tucMsg[24] = "\x11\x11\x11\x11\x11\x11\x11\x11"
		                             "\x22\x22\x22\x22\x22\x22\x22\x22"
							    	 "\x33\x33\x33\x33\x33\x33\x33\x33";
    byte tucMac[8];
	int i, iRet;

	// Initialisations
	// ***************
    font_size (8,16);                                                // Normal font size
	putchar('\x1B');                                                 // Clear screen

    iRet = DLLExist("SECURITY"); CHECK(iRet==TRUE, lblNoSecDll);     // Security DLL loaded?
	iRet = ChkPpdConfig(); CHECK(iRet>=0, lblEnd);                   // Pinpad problem? => check config from Manager
	iRet = printf("Mac Compute..."); CHECK(iRet>=0, lblKO);          // Display message

    // Mac calculation
	// ***************
	xMacKey.iSecretArea = AREA_ID;                                   // Secret area identification
	xMacKey.cAlgoType = TLV_TYPE_KDES;                               // MAC key is a DES Key 
	xMacKey.usNumber = MAC_KEY_LOC;                                  // MAC key location inside the secret area
	xMacKey.uiBankId = BANK_MAC_ID;                                  // Bank id related to this MAC key 

	iRet = SEC_ComputeMAC (&xMacKey,                                 // MAC key parameters                                  
			               (unsigned char *) tucMsg,                 // Message to compute
			               sizeof(tucMsg),
		 				   NULL, 
		 				   tucMac);                                  // MAC result
	CHECK(iRet==0, lblKO)

	// Print and display Mac
	// *********************
    iRet = pprintf("\x1b""E""        MAC Demo\n\n"                   // Print demo in progress
		           "\x1b""F"); CHECK(iRet>=0, lblKO);      
    iRet = pprintf("Data message:\n"
		           "11 11 11 11 11 11 11 11\n"
				   "22 22 22 22 22 22 22 22\n"
				   "33 33 33 33 33 33 33 33\n"); CHECK(iRet>=0, lblKO);
	iRet = pprintf("MAC Key:\n"
		           "09 23 45 67 89 AB CD EF\n"); CHECK(iRet>=0, lblKO);
    iRet = pprintf("Mac calculation:\n"); CHECK(iRet>=0, lblKO);
	for (i=0; i<8; i++)
	{
		iRet = pprintf ("%02X ", tucMac[i]); CHECK(iRet>=0, lblKO);
	}

	iRet = pprintf("\n\n\n\n\n\n\n"); CHECK(iRet>=0, lblKO);

	gotoxy(0,0); iRet = printf("\x1B""Mac:"); CHECK(iRet>=0, lblKO); // Display MAC
	gotoxy(1,0);
	for (i=0; i<8; i++)
	{
		iRet = printf ("%02X", tucMac[i]); CHECK(iRet>=0, lblKO);
	}

	ttestall(0, 5*100);                                              // Delay 5s
	
	goto lblEnd;

	// Errors treatment 
    // ****************
lblNoSecDll:
	printf("Missing Sec DLL ");                                      // Security DLL missing
    ttestall(0, 5*100);                                              // Delay 5s 
	goto lblEnd;
lblKO:
	putchar('\x1B');                                                 // Clear screen
	printf("Processing Error");                                      // Oops! error
    ttestall(0, 5*100);                                              // Delay 5s 
lblEnd:
	ttestall(PRINTER, 10);   
}

void tcbkw005(void)
{
	switch (ManageMenu("Schemes", 0, tzMenuScheme))
	{
	case 0:  SetConfig();            break; // Set security configuration
	case 1:  GetConfig();            break; // Get security configuration
	case 2:  CreateSecret();         break; // Create a secret area
	case 3:  FreeSecret();           break; // Erase a secret area
	case 4:  LoadIso9564Key();       break; // Load Iso9564 key
	case 5:  Iso9564PinEntry();      break; // Enter pin and pinblock calculation
	case 6:  LoadMacKey();           break; // Load Mac key 
	case 7:  MacCalculation();       break; // Mac calculation
	default: 				         break; // Abort key pressed
	}

}













