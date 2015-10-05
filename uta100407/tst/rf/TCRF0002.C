#include "tst.h"
#include "stdlib.h"
//#include "string.h"
//#include "stdio.h"


#include <string.h>
#include <stdio.h>
#include <miltypes.h>
#include <unicapt.h>
#include <psyTLV.h>
 
#include <ssaStd.h>
#include <ssaSec.h>

#include <message.h>

typedef struct
{
   int16    status;                 /* Execution status */
   uint16   length;                 /* Length of returned data */
   uint8    data[50];               /* buffer of data */
} ssaKbdSecRes_t;


typedef struct
{
   uint8    isoFontNb;                             
   char     printOK[17];         
   char     printKO[25];             
   char     toDisplay[17];           
   char     toDisplayBold[17];              
   char     toPrint[17];              
   char     toPrintBold[17];           
   char     pinEntryText[17];            
   char     pinEntryinput[17];        
} testFont_t;


uint32               prnHandle;
   uint32               hmiHandle;
   uint32               secHandle;
   uint8                key;
   uint8                i;
   ssaDisFontDescr_t    descriptor;
   int16                status;
   ssaSecDispKbdInput_t ssaKbdInput;
   ssaKbdSecRes_t       asyncResult;
   uint32               waitRet;
   int16                retCode;
   uint8                performTest;
   //static testFont_t    testFontArray[NB_FONT_TYPE];


void tool_loadTestKey(uint32 secHandle, ssaSecKeyDescription_t  *doubleTrspKey);
void mnutestIdent(void);
void testLanguages(void);

void testLanguages(void)
{
   uint32               prnHandle;
   uint32               hmiHandle;
   uint32               secHandle;
   uint8                key;
   uint8                i;
   ssaDisFontDescr_t    descriptor;
   int16                status;
   ssaSecDispKbdInput_t ssaKbdInput;
   ssaKbdSecRes_t       asyncResult;
   uint32               waitRet;
   int16                retCode;
   uint8                performTest;
   static testFont_t    testFontArray[NB_FONT_TYPE];
    
   testFontArray[0].isoFontNb = Iso8859_1;                      
   strcpy(testFontArray[0].printOK,"Iso8859_1 OK");         
   strcpy(testFontArray[0].printKO,"Iso8859_1 not loaded");    
   strcpy(testFontArray[0].toDisplay,"TST NORM \xDE\xDF");      
   strcpy(testFontArray[0].toDisplayBold,"TST BOLD \xDE\xDF");              
   strcpy(testFontArray[0].toPrint,"TST NORM \xDE\xDF");              
   strcpy(testFontArray[0].toPrintBold,"\x1B\x1ATST BOLD \xDE\xDF");           
   strcpy(testFontArray[0].pinEntryText,"PIN ENTRY \xDE\xDF");            
   strcpy(testFontArray[0].pinEntryinput,"CODE \xDE\xDF:");  
   testFontArray[1].isoFontNb = Iso8859_2;                      
   strcpy(testFontArray[1].printOK,"Iso8859_2 OK");         
   strcpy(testFontArray[1].printKO,"Iso8859_2 not loaded");    
   strcpy(testFontArray[1].toDisplay,"TST NORM \xD0\xD1");      
   strcpy(testFontArray[1].toDisplayBold,"TST BOLD \xD0\xD1");              
   strcpy(testFontArray[1].toPrint,"TST NORM \xD0\xD1");              
   strcpy(testFontArray[1].toPrintBold,"\x1B\x1ATST BOLD \xD0\xD1");           
   strcpy(testFontArray[1].pinEntryText,"PIN ENTRY \xD0\xD1");            
   strcpy(testFontArray[1].pinEntryinput,"CODE \xD0\xD1:");        
   testFontArray[2].isoFontNb = Iso8859_5;                      
   strcpy(testFontArray[2].printOK,"Iso8859_5 OK");         
   strcpy(testFontArray[2].printKO,"Iso8859_5 not loaded");    
   strcpy(testFontArray[2].toDisplay,"TST NORM \xB1\xB3");      
   strcpy(testFontArray[2].toDisplayBold,"TST BOLD \xB1\xB3");              
   strcpy(testFontArray[2].toPrint,"TST NORM \xB1\xB3");              
   strcpy(testFontArray[2].toPrintBold,"\x1B\x1ATST BOLD \xB1\xB3");           
   strcpy(testFontArray[2].pinEntryText,"PIN ENTRY \xB1\xB3");            
   strcpy(testFontArray[2].pinEntryinput,"CODE \xB1\xB3:");
   testFontArray[3].isoFontNb = Iso8859_7;                      
   strcpy(testFontArray[3].printOK,"Iso8859_7 OK");         
   strcpy(testFontArray[3].printKO,"Iso8859_7 not loaded");    
   strcpy(testFontArray[3].toDisplay,"TST NORM \xD0\xD1");      
   strcpy(testFontArray[3].toDisplayBold,"TST BOLD \xD0\xD1");              
   strcpy(testFontArray[3].toPrint,"TST NORM \xD0\xD1");              
   strcpy(testFontArray[3].toPrintBold,"\x1B\x1ATST BOLD \xD0\xD1");           
   strcpy(testFontArray[3].pinEntryText,"PIN ENTRY \xD0\xD1");            
   strcpy(testFontArray[3].pinEntryinput,"CODE \xD0\xD1:");   
   
   testFontArray[4].isoFontNb = Iso8859_8;                      
   strcpy(testFontArray[4].printOK,"Iso8859_8 OK");         
   strcpy(testFontArray[4].printKO,"Iso8859_8 not loaded");    
   strcpy(testFontArray[4].toDisplay,"TST NORM \xE0\xE1\xE2\xE3\xE4\xE5\xE6");      
   strcpy(testFontArray[4].toDisplayBold,"TST BOLD \xE0\xE1\xE2\xE3\xE4\xE5\xE6");              
   strcpy(testFontArray[4].toPrint,"TST NORM \xE0\xE1\xE2\xE3\xE4\xE5\xE6");              
   strcpy(testFontArray[4].toPrintBold,"\x1B\x1ATST BOLD \xE0\xE1\xE2\xE3\xE4\xE5\xE6");  

   strcpy(testFontArray[4].toDisplay,"TST NORM \xE7\xE8\xE9\xEA\xEB\xEC\xED");      
   strcpy(testFontArray[4].toDisplayBold,"TST BOLD \xE7\xE8\xE9\xEA\xEB\xEC\xED");              
   strcpy(testFontArray[4].toPrint,"TST NORM \xE7\xE8\xE9\xEA\xEB\xEC\xED");              
   strcpy(testFontArray[4].toPrintBold,"\x1B\x1ATST BOLD \xE7\xE8\xE9\xEA\xEB\xEC\xED");  	  
      
  // strcpy(testFontArray[4].pinEntryText,"PIN ENTRY \xE0\xE1");            
  // strcpy(testFontArray[4].pinEntryinput,"CODE \xE0\xE1:");    

   testFontArray[5].isoFontNb = Iso8859_9;                      
   strcpy(testFontArray[5].printOK,"Iso8859_9 OK");         
   strcpy(testFontArray[5].printKO,"Iso8859_9 not loaded");    
   strcpy(testFontArray[5].toDisplay,"TST NORM \xDE\xDF");      
   strcpy(testFontArray[5].toDisplayBold,"TST BOLD \xDE\xDF");              
   strcpy(testFontArray[5].toPrint,"TST NORM \xDE\xDF");              
   strcpy(testFontArray[5].toPrintBold,"\x1B\x1ATST BOLD \xDE\xDF");           
   strcpy(testFontArray[5].pinEntryText,"PIN ENTRY \xDE\xDF");            
   strcpy(testFontArray[5].pinEntryinput,"CODE \xDE\xDF:");   
   testFontArray[6].isoFontNb = Iso8859_35;                      
   strcpy(testFontArray[6].printOK,"Iso8859_35 OK");         
   strcpy(testFontArray[6].printKO,"Iso8859_35 not loaded");    
   strcpy(testFontArray[6].toDisplay,"TST NORM \xE0\xE1");      
   strcpy(testFontArray[6].toDisplayBold,"TST BOLD \xE0\xE1");              
   strcpy(testFontArray[6].toPrint,"TST NORM \xE0\xE1");              
   strcpy(testFontArray[6].toPrintBold,"\x1B\x1ATST BOLD \xE0\xE1");           
   strcpy(testFontArray[6].pinEntryText,"PIN ENTRY \xE0\xE1");            
   strcpy(testFontArray[6].pinEntryinput,"CODE \xE0\xE1:"); 
   testFontArray[7].isoFontNb = Iso8859_35_b;                      
   strcpy(testFontArray[7].printOK,"Iso8859_35b OK");         
   strcpy(testFontArray[7].printKO,"Iso8859_35b not loaded");    
   strcpy(testFontArray[7].toDisplay,"TST NORM \xE0\xE1");      
   strcpy(testFontArray[7].toDisplayBold,"TST BOLD \xE0\xE1");              
   strcpy(testFontArray[7].toPrint,"TST NORM \xE0\xE1");              
   strcpy(testFontArray[7].toPrintBold,"\x1B\x1ATST BOLD \xE0\xE1");           
   strcpy(testFontArray[7].pinEntryText,"PIN ENTRY \xE0\xE1");            
   strcpy(testFontArray[7].pinEntryinput,"CODE \xE0\xE1:");  
   
   
   for(i=0; i<NB_FONT_TYPE; i++)
   {
      performTest = FALSE;
      
	   ssaSecOpen(SSA_MERCHANT,&secHandle);
      status = ssaSecDisSelectFont(secHandle, testFontArray[i].isoFontNb, &descriptor);
      
      prnOpen( PRN_DEFAULT_CHANNEL, &prnHandle, PRN_IMMEDIATE );
      prnPrint(prnHandle, "-----------------------" ); 
      if(status == RET_OK)
      {
         prnPrint(prnHandle, testFontArray[i].printOK );
         performTest = TRUE;
      }
      else
      {
         prnPrint(prnHandle, testFontArray[i].printKO ); 
      }
            
      if(performTest == TRUE)
      {
         // test the printer  
         prnPrint(prnHandle, testFontArray[i].toPrint );  
         prnPrint(prnHandle, testFontArray[i].toPrintBold ); 
         
         // test the display
         hmiOpen("DEFAULT", &hmiHandle);
         hmiADClearLine (hmiHandle, 0);
         hmiADDisplayText(hmiHandle,0,0, testFontArray[i].printOK); 
         hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot)); 
         hmiADClearLine (hmiHandle, 1);
         hmiADDisplayText(hmiHandle,1,0, testFontArray[i].toDisplay);
         hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot)); 
         hmiADClearLine (hmiHandle, 2);
         hmiADDisplayText(hmiHandle,2,0, testFontArray[i].toDisplayBold);
         hmiKeyWait (hmiHandle, &key, PSY_INFINITE_TIMEOUT);         
         hmiClose(hmiHandle);      
         
         // test pin entry function      
         ssaKbdInput.lineNbText = 1;
         ssaKbdInput.fontText = SSA_INTERNAL_FONT_RAM(descriptor.hmiNormSlot);
         memcpy(ssaKbdInput.text,testFontArray[i].pinEntryText,sizeof(testFontArray[i].pinEntryText));
         ssaKbdInput.lineNbInput = 2;
         ssaKbdInput.fontInput = SSA_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot);
         memset(ssaKbdInput.textInput,0,sizeof(ssaKbdInput.textInput));
         memcpy(ssaKbdInput.textInput,testFontArray[i].pinEntryinput,sizeof(testFontArray[i].pinEntryinput));
         ssaKbdInput.inputFieldDefinition = 0x00F0;
         ssaKbdInput.direction = 1;
         ssaKbdInput.minimumKeys = 4;
         ssaKbdInput.maximumKeys = 4;
         ssaKbdInput.endKey = 0;
         ssaKbdInput.echoCharacter = '*';
         ssaKbdInput.parameters1 = 0;
         ssaKbdInput.corKeyAction = 0;
         ssaKbdInput.replacementCharacter = 0;
         memset(ssaKbdInput.values,0,4);
         ssaKbdInput.timeOutFirstKey = 5;
         ssaKbdInput.timeOutInterKey = 9;  
         retCode = ssaSecDispKbdInputReq (secHandle, &ssaKbdInput);
         if(retCode == RET_RUNNING)
         {
            // here the application decide to wait on applicative events but on none OS event
            waitRet = psyPeripheralResultWait(  0, PSY_INFINITE_TIMEOUT, PSY_WAIT_ON_EVENT);
         
            if (waitRet & PSY_EVENT_RECEIVED)
               // here means an applicative event occured
               if(waitRet & SSA_WAIT)
               {
                  // here means a SSA event occured
                  ssaSecResultGet (secHandle, sizeof(asyncResult), &asyncResult); 
               }
         }
		
      }   
      
      prnPrint(prnHandle, " " );  
      ssaSecClose(secHandle);
      prnClose(prnHandle);
   }
   
}


static code const char *srcFile= __FILE__;

//Unitary test case tcab0002.
//Functions testing:
//  dspStart
//  dspStop
//  dspClear
//  dspLS

void characters1(){
    int ret;
	int i;
	char pbuf[5];
	char pbuf2[10];
	char pbuf3[10];
	char cchar[2];
    
    ret= dspStart();
    CHECK(ret>=0,lblKO);
    
	for (i=160;i<=180;i++) {
		//pbuf[1]=i;
		itoa(i,cchar,16);
		sprintf(pbuf, "\\x%s", cchar);
		ret= dspLS(0,pbuf);
		CHECK(ret>=0,lblKO);

		sprintf(pbuf2, "Num %c", i);
		ret= dspLS(1,pbuf2);
		CHECK(ret>=0,lblKO);
		
		sprintf(pbuf3, "Num %d", i);
		ret= dspLS(2,pbuf3);
		CHECK(ret>=0,lblKO);

		//tmrPause(1);
		tmrStart(0,20);while(tmrGet(0));tmrStop(0);
	}

	//ret= dspLS(0,"char>> \xa5\xab");
	//CHECK(ret>=0,lblKO);
	//tmrPause(3);

    ret= dspClear();
    CHECK(ret>=0,lblKO);
    tmrPause(1);

    goto lblEnd;    
lblKO:
    trcErr(ret);
    tmrPause(3);
lblEnd:
    dspStop(); //close resource
}


void characters2(){
    int ret;
	int i;
	char pbuf[15];
	char cchar[2];
    
    ret= prtStart(); CHECK(ret>=0,lblKO);
    
	for (i=128;i<=255;i++) {
		itoa(i,cchar,16);
		sprintf(pbuf, "x%s  %d  %c", cchar, i, i);
		ret= prtS(pbuf); CHECK(ret>=0,lblKO);
		}
    goto lblEnd;
lblKO:
    trcErr(ret);
    tmrPause(3);
lblEnd:
    prtStop();
}

void ssa_output(){
	uint8	key;
	uint32	secHandle;
	uint32	hmiHandle;
	uint32	prnHandle;
	int16	status;
	ssaFontDescr_t	descriptor;

	ssaSecOpen(SSA_MERCHANT,&secHandle);
	
	// display
	ssaSecLafWriteFont(secHandle, TERM, Iso8859_2, &descriptor);
	// ?? return status value ?? is 8859-2 exists ??
	hmiOpen("DEFAULT", &hmiHandle);
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiBoldSlot));
	// ?? is there any other font style ?? it isn't independent from 8859-2 ??
	hmiADDisplayText(hmiHandle, 0, 0, "prÛbaszˆveg-\xf5");
	hmiADFont(hmiHandle, HMI_INTERNAL_FONT_RAM(descriptor.hmiNormSlot));
	// ?? how can i change the font size ??
	hmiADDisplayText(hmiHandle, 1, 0, "prÛbaszˆveg-\xf5");
	hmiADDisplayText(hmiHandle, 2, 0, "·ÈÌÛ˙¡…Õ”⁄\xf5\xfb\xd5\xdb");
	hmiADDisplayText(hmiHandle, 3, 0, " - Press a key -");
	key=hmiKeyWait(hmiHandle, &key, PSY_INFINITE_TIMEOUT);
	hmiClose(hmiHandle);
	
	// printer
	status = ssaSecDisSelectFont(secHandle, Iso8859_2, &descriptor);
	prnOpen(PRN_DEFAULT_CHANNEL, &prnHandle, PRN_IMMEDIATE);
	if(status == RET_OK)
	{
		prnPrint(prnHandle, "Iso8859_2 OK" );
		prnPrint(prnHandle, "·ÈÌÛ˙¡…Õ”⁄\xf5\xfb\xd5\xdb");
	}
	else
	{
		prnPrint(prnHandle, "Iso8859_2 not loaded" );
		prnPrint(prnHandle, "·ÈÌÛ˙¡…Õ”⁄\xf5\xfb\xd5\xdb");
	}
	prnClose(prnHandle);

	ssaSecClose(secHandle);
}

void tcrf0002(void){
	//characters1(); // output characters with codes; one per screen
	//characters2(); // writes all special characters to the Printer (without SSA)
	ssa_output(); // writes all spec. chars. to the Display with SSA
	//testLanguages(); // codepage detection and test
}
