/** \file
 * Unitary test case tcml0005.
 * Function testing
 * \sa
 *  - ssaSecLafWriteFont()
 *  - ssaSecLafSelectPrnDefaultFont()
 */

#include <unicapt.h>
#include "message.h"
#include "ssaStd.h"
#include "ssaSec.h"
#include "string.h"

void tcml0005(void){
	char dsp[16];
	char prn[24];
	char prn_b[24];
	uint8 key;
	
	uint32 secHandle, prnHandle, hmiHandle;
	int16 status;

	//ssaFontDescr_t  descriptor;
	ssaFontDescr_t descript;
	ssaFontDescr_t descriptor ;

	strcpy(dsp,"TST NORM\xAD\xB1\xB2\xB3\xB4\xB5\xB6");      
	//strcpy(dsp_b,"TST BOLD\xB0\xB1\xB2\xB3\xB4\xB5\xB6");      

	//strcpy(prn,"TST NORM \xAD\xB1\xB2\xB3\xB4\xB5\xB6");
	strcpy(prn,"TST NORM \xC0\xC1\xC3\xC6\xC7\xC8\xC9");       
	strcpy(prn_b,"\x1B\x1ATST BOLD \xC0\xC1\xC3\xC6\xC7\xC8\xC9");      
	
	ssaSecOpen(SSA_MERCHANT,&secHandle);   
    
	status = ssaSecLafWriteFont(secHandle, TERM, Iso8859_35_b, &descript);
	ssaSecLafWriteFont(secHandle, TERM, Iso8859_35, &descriptor);

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
