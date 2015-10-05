/*
 ----------------------------------------------------------------------
|                                                                      |
|   Project :                 INGEDEV 32 - V 3.X                       |
|                             Millennium Application Wizard            |
|                                                                      |
|   File name :               MSGMENU.C                                |
|                                                                      |
|   Description :             MENU MESSAGES                            |
|                             => Called by the 'MsgDispatch' function. |
|                                                                      |
| Remark : This file is created automaticaly by the Ingenico           |
| development interface (INGEDEV) BUT THE USER CAN INSERT THE          |
| SPECIFIC APPLICATION'S TREATMENT THAT HE WANTS.                      |
|                                                                      |
|   Creation date :           13/12/2001                               |
|                                                                      |
|   Copyright :               INGENICO                                 |
|                                                                      |
 ----------------------------------------------------------------------
*/

#include <unicapt.h>
#include "message.h"
#include "ssaStd.h"
#include "ssaSec.h"
#include "string.h"

///////////////////////////// USER'S COMMENTARY //////////////////////////

//////////////////////////////////////////////////////////////////////////
void OnFarsifonts ( LPCMSG Msg )
{

	char dsp[16];
	char dsp_b[16];
	char prn[24];
	char prn_b[24];
	uint8 key;
	
	uint32 secHandle, prnHandle, hmiHandle;
	int16 status;

	//ssaFontDescr_t  descriptor;
	ssaFontDescr_t descript;
	ssaDisFontDescr_t descriptor ;
	ssaDisFontDescr_t descriptor2 ;

	strcpy(dsp,"TST NORM\xAD\xB1\xB2\xB3\xB4\xB5\xB6");      
	//strcpy(dsp_b,"TST BOLD\xB0\xB1\xB2\xB3\xB4\xB5\xB6");      

	//strcpy(prn,"TST NORM \xAD\xB1\xB2\xB3\xB4\xB5\xB6");
	strcpy(prn,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9");       
	strcpy(prn_b,"\x1B\x1ATST BOLD \xC0\xC1\xC3\xC6\xC7\xC8\xC9");      
	
	ssaSecOpen(SSA_MERCHANT,&secHandle);   
    
	status = ssaSecLafWriteFont(secHandle, TERM, Iso8859_35_b, &descript);
	ssaSecLafWriteFont(secHandle, TERM, Iso8859_35, &descriptor);
	

	ssaSecDisSelectFont(secHandle,Iso8859_35, &descriptor);
	ssaSecLafSelectPrnDefaultFont(secHandle, TERM,HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot),
	HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot));	
 	prnOpen( PRN_DEFAULT_CHANNEL, &prnHandle, PRN_IMMEDIATE );      
    // test the printer  
	prnPrint(prnHandle, prn );  
	prnPrint(prnHandle, prn_b );  

	ssaSecLafSelectPrnDefaultFont(secHandle, TERM,HMI_INTERNAL_FONT_RAM(descript.hmiNormSlot),
	HMI_INTERNAL_FONT_RAM(descript.hmiBoldSlot));
	
	prnPrint(prnHandle, prn );  
	prnPrint(prnHandle, prn_b );        
	
	hmiOpen("DEFAULT", &hmiHandle);
	hmiADClearLine (hmiHandle, 0);
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descript.hmiNormSlot)); 		


    hmiADDisplayText(hmiHandle,0,0, dsp);
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descript.hmiBoldSlot)); 
	
    hmiADClearLine (hmiHandle, 1);
    hmiADDisplayText(hmiHandle,1,0, dsp);
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot)); 		
    hmiADClearLine (hmiHandle, 2);    
    hmiADDisplayText(hmiHandle,2,0, dsp);

	hmiADClearLine (hmiHandle, 3);
	hmiADFont(hmiHandle,HMI_INTERNAL_FONT_1 );
	ssaSecLafSelectPrnDefaultFont(secHandle,TERM, HMI_INTERNAL_FONT_1,HMI_INTERNAL_FONT_1);	

    hmiADDisplayText(hmiHandle,3,0, "HELLO"); 		
	hmiKeyWait(hmiHandle,&key,PSY_INFINITE_TIMEOUT);

	hmiADDisplayText(hmiHandle,5,0, "INTERNAL_FONT_1");
	prnPrint(prnHandle,"INTERNAL_FONT_1");

	hmiADClearLine (hmiHandle, 2);
	hmiADClearLine (hmiHandle, 3);
    hmiADDisplayText(hmiHandle,2,0, "INTERNAL_FONT_1");
		
	
	hmiKeyWait(hmiHandle,&key,PSY_INFINITE_TIMEOUT);

	hmiClose(hmiHandle);
	prnClose(prnHandle); 
	ssaSecClose(secHandle);

	
}

///////////////////////////// USER'S COMMENTARY //////////////////////////

//////////////////////////////////////////////////////////////////////////
void OnCyrillic ( LPCMSG Msg )
{
	char dsp[16];
	char prn[24];
	char prn_b[24];
	uint8 key;
	
	uint32 secHandle, prnHandle, hmiHandle;
	int16 status;

	ssaFontDescr_t  descriptor;
	ssaDisFontDescr_t descript;

	strcpy(dsp,"TST NORM \xB0\xB1\xB2\xB3\xB4\xB5\xB6");      

	strcpy(prn,"TST NORM \xB0\xB1\xB2\xB3\xB4\xB5\xB6");      
	strcpy(prn_b,"\x1B\x1ATST BOLD  \xB0\xB1\xB2\xB3\xB4\xB5\xB6");

	
	ssaSecOpen(SSA_MERCHANT,&secHandle);   
	status = ssaSecLafWriteFont(secHandle, TERM, Iso8859_5, &descriptor);

	ssaSecLafSelectPrnDefaultFont(secHandle, TERM,HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot),
	HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot));

	
 	prnOpen( PRN_DEFAULT_CHANNEL, &prnHandle, PRN_IMMEDIATE );
      
    // test the printer  
	prnPrint(prnHandle, prn );  
	prnPrint(prnHandle, prn_b );  
         
	prnClose(prnHandle); 

	hmiOpen("DEFAULT", &hmiHandle);
	hmiADClearLine (hmiHandle, 0);
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot)); 		
    hmiADClearLine (hmiHandle, 1);

    hmiADDisplayText(hmiHandle,1,0, dsp);

	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot)); 	
    hmiADClearLine (hmiHandle, 2);
    hmiADDisplayText(hmiHandle,2,0, dsp);
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot)); 		
    hmiADClearLine (hmiHandle, 3);
    hmiADDisplayText(hmiHandle,3,0, dsp);

	hmiKeyWait(hmiHandle,&key,PSY_INFINITE_TIMEOUT);

	hmiClose(hmiHandle);
	ssaSecClose(secHandle);
}

///////////////////////////// USER'S COMMENTARY //////////////////////////

//////////////////////////////////////////////////////////////////////////
void OnGrec ( LPCMSG Msg )
{

	char dsp[16];
	char prn[24];
	char prn_b[24];
	uint8 key;
	
	uint32 secHandle, prnHandle, hmiHandle;
	int16 status;

	ssaFontDescr_t  descriptor;

	memset(dsp,0,6);

	/*strcpy(dsp,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9");      
	strcpy(dspb,"TST BOLD \xD0\xD1\xD3\xD6\xD7\xD8\xD9"); */

	
	memcpy(dsp,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9",sizeof("TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9"));
	

	strcpy(prn,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9");      
	strcpy(prn_b,"\x1B\x1ATST BOLD  \xD0\xD1\xD3\xD6\xD7\xD8\xD9");

/*	testFontArray[0].isoFontNb = Iso8859_7;                      
   strcpy(testFontArray[0].printKO,"Iso8859_7");    
   strcpy(testFontArray[0].toDisplay,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9");      
   strcpy(testFontArray[0].toDisplayBold,"TST BOLD \xC0\xC1\xC3\xC6\xC7\xC8\xC9");              
   
   
   strcpy(testFontArray[1].toDisplay,"TST NORM \xD0\xD1\xD3\xD6\xD7\xD8\xD9");      
   strcpy(testFontArray[1].toDisplayBold,"TST BOLD \xD0\xD1\xD3\xD6\xD7\xD8\xD9");              
   
   strcpy(testFontArray[1].toPrint,"TST NORM \xD0\xD1\xD3\xD6\xD7\xD8\xD9");              
   strcpy(testFontArray[1].toPrintBold,"\x1B\x1ATST BOLD \xD0\xD1\xD3\xD6\xD7\xD8\xD9"); 
  */ 
    ssaSecOpen(SSA_MERCHANT,&secHandle);   
	status = ssaSecLafWriteFont(secHandle, TERM, Iso8859_7, &descriptor);

	ssaSecLafSelectPrnDefaultFont(secHandle, TERM,HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot),
	HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot));

	
 	prnOpen( PRN_DEFAULT_CHANNEL, &prnHandle, PRN_IMMEDIATE );
      
    // test the printer  
	prnPrint(prnHandle, prn );  
	prnPrint(prnHandle, prn_b );  
         
	prnClose(prnHandle); 

	hmiOpen("DEFAULT", &hmiHandle);
	hmiADClearLine (hmiHandle, 0);
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot)); 		
    hmiADClearLine (hmiHandle, 1);

    hmiADDisplayText(hmiHandle,1,0, dsp);


	memcpy(dsp,"TST BOLD \xD0\xD1\xD3\xD6\xD7\xD8\xD9",sizeof("TST BOLD \xD0\xD1\xD3\xD6\xD7\xD8\xD9"));
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot)); 	
    hmiADClearLine (hmiHandle, 2);
    hmiADDisplayText(hmiHandle,2,0, dsp);
	memcpy(dsp,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9",sizeof("TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9"));
	
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot)); 		
    hmiADClearLine (hmiHandle, 3);
    hmiADDisplayText(hmiHandle,3,0, dsp);

	hmiKeyWait(hmiHandle,&key,PSY_INFINITE_TIMEOUT);

	hmiClose(hmiHandle);
	ssaSecClose(secHandle);


}
