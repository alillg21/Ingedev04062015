/**************************************************************************
                              Property of INGENICO CER
 *************************************************************************/
/*+
 *  PROJECT        :    UTA
 *  MODULE         :    TC
 *  FILEMANE       :    TCIT0000.c
 *
 *  date         author     modifications
 *  30-06-2005   ITR        Creation
 *  	
 *  	
 * ------------------------------------------------------------------------
 *  FUNCTIONS    :    
 *  
 *							**INTERNAL**
 *  				int16  cdbOpen(char *ComchannelName, uint32 *ComHandle);
 *  				int16  cdbClose( uint32 ComHandle);
 *  				int16  cdbChanInit(uint32 comHandle);
 *  				int16  cdbMsgWrite( uint32 ComHandle, uint8 * msg)
 *					void   cdbfprintPut(uint32 ComHandle, uint8 * msg)
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unicapt.h>
#include "sys.h"

//Unitary test case tcit0000.
//Com Debug testing.

#ifdef __DEBUG__
#ifdef __DEBUG_ITR__

/////////////////SYSTEM FUNCTIONS/////////////////////////////////

void cdbOpen(char *ComchannelName, card *ComHandle){	
		comOpen(ComchannelName,ComHandle);	
}


void cdbClose( card ComHandle){	
		comClose (ComHandle);	
}


void cdbChanInit(card comHandle){	
		comUartInit_t			uartInit;
		comProtocolCfg_t		comProtocolCfg;
		comUartProtocolCfg_t	uartCfg;
		comInit_t				comInit;
		// Parameters initialization
		memset(&comInit, 0, sizeof(comInit_t));
		comInit.type = COM_INIT_TYPE_UART;
		comInit.level = COM_INIT_LEVEL_1;
		// ITR code COM_PARAM ComParameter = { {1}, {9600}, {1}, {8}, {'N'} };
		memset(&uartInit, 0, sizeof(comUartInit_t));
		uartInit.speed = COM_BAUD_9600;
		uartInit.parity = COM_PARITY_NONE; // Odd= Impaire, Even=Paire, None= Sans parite 
		uartInit.dataSize = COM_DATASIZE_8;
		uartInit.stopBits = COM_STOP_1;
		uartInit.sBufferSize = 255;
		uartInit.rBufferSize = 255;
		comInit.init.uartInit = &uartInit;
		memset(&comProtocolCfg, 0, sizeof(comProtocolCfg_t));
		comProtocolCfg.type = COM_PROT_CFG_TYPE_UART;
		comProtocolCfg.level = COM_PROT_CFG_LEVEL_1;
		memset(&uartCfg, 0, sizeof(comUartProtocolCfg_t));
		uartCfg.flowControl = COM_FLOW_NONE;
		comProtocolCfg.protCfg.uartCfg = &uartCfg;
		/////////////////////////////////////////////////
		// Initilization of port COM
		/////////////////////////////////////////////////
		// Initialize the channel
		comChanInit(
						comHandle,
						COM_MODIF_PARAM_TEMPORARY,
						&comInit,
						COM_SET_PROTOCOL_TEMPORARY,
						&comProtocolCfg);
		
}


void cdbMsgWrite( card ComHandle, char * msg){	
		byte msgLength=0;
		char cEnd[1];
		memset(cEnd,0,sizeof(cEnd));
		msgLength = strlen(msg);
		comSendMsg (ComHandle,msgLength,msg);
		if(msg[msgLength-1]=='\n')
			comSendMsg (ComHandle,2,"\x0D\x0A");

}


void cdbprintPut(card ComHandle, char* msg){  
		cdbMsgWrite(ComHandle, msg);
}

///////////////// TEST CASE /////////////////////////////////


#endif
#endif

