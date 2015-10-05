#include <string.h>
#include <stdio.h>
#include <unicapt.h>
#include <hmi.h>
#include "sys.h"

#define RSP_LEN 1024
typedef struct {
    int16 sta;
    uint16 len;
    char buf[RSP_LEN];
} tResponse;

void tcns0062(void) {
    int ret;
    int len;
    uint32 hSmc;
    unsigned char rsp[260];
    unsigned char blk = 0xFF;  
    unsigned char tmp[260];//block size
    int n1; // N1 (see p 119) - number of event records to read
    unsigned long int n6;// activity structure size
    unsigned long int ofs=0;/// offset for reading large files by blocks
    unsigned char ofsB[4]; 
    
    smcCommand_t com;
    union smcCommandIn_t cit;
    tResponse res;

   dspStart();
   ret = smcOpen("ICC0", &hSmc);
   CHECK(ret == RET_OK, lblKO);

   ret = tmrStart(0, 60 * 100);    //60 seconds timeout
   CHECK(ret >= 0, lblKO);
   dspClear();
    
    len = 0;
    while(tmrGet(0)) {
    	dspLS(0, "insert card");
    	
    	memset(&com, 0, sizeof(com));
    	memset(&cit, 0, sizeof(cit));
    	
        cit.newPowerCommand.cardStandards = SMC_ISO_MAX_LIMIT;
        cit.newPowerCommand.GRClassByte00 = SMC_CLA_00;
        cit.newPowerCommand.preferredProtocol = SMC_DEFAULT_PROTOCOL;
        
    	com.commandIn = &cit;
    	
    	do{
    		ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
    	}while(ret == SMC_ERR_NO_ICC);
	    CHECK(ret == RET_RUNNING, lblKO);
	    
	    do {//waiting for result
	            ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
        } 
	    while(ret == RET_RUNNING);
	    
	    if(ret != RET_OK)
	    			goto lblKO;

       switch (res.sta) {
         case RET_OK:             //main branch: no problem detected
             ret = res.len;
             if(rsp)//if response anticipated
                 memcpy(rsp, res.buf, ret);
             break;
         case SMC_CARD_REMOVAL:
             ret = -iccCardRemoved;
             break;
         case SMC_CARD_MUTE:
             ret = -iccCardMute;
             break;
         default:                 //there are too many other error types
             ret = -iccCardPb;
             break;
       }

        if(ret == -iccCardRemoved || ret == 0)
            continue;
        CHECK(ret >= 0, lblKO);
        len = ret;
        break;
    }

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

    
    		
// READ CardIccIdentification file (AID = 0002), size 25 bytes
    dspLS(0, "CardIccIdentification");   
    
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));
	
	// select  00 02
	//                         TYP CLA INS P1  P2  Lc              Len    ID
	// iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x00\x02", rsp);
	com.commandType = SMC_ASYNC_COMMAND;
    cit.asyncCommand.asyncCommandType = 0x03; /// TYP
    cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x000;
    cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
    cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
    cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x00\x02";
    com.commandIn = &cit;
    
    ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
    CHECK(ret == RET_RUNNING, lblKO);
    
    do {//waiting for result
            ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
    } 
    while(ret == RET_RUNNING);
    
    if(ret != RET_OK) 
    	goto lblKO;

    if(res.sta == RET_OK) {
         ret = res.len;
         memcpy(rsp, res.buf, ret);
    }
    else goto lblKO;
    
    CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
    
    
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));
	
	// read block of length 0x19  00 02
	//                              TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\x19", (byte *)"\x19", rsp);
	len = (int)(0x19);
	com.commandType = SMC_ASYNC_COMMAND;
    cit.asyncCommand.asyncCommandType =                    0x02; 
    cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xB0;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x19;
    cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
    com.commandIn = &cit;
    
    ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
    CHECK(ret == RET_RUNNING, lblKO);
    
    do {//waiting for result
            ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
    } 
    while(ret == RET_RUNNING);
    
    if(ret != RET_OK) 
    	goto lblKO;

    if(res.sta == RET_OK) {
         ret = res.len;
         memcpy(rsp, res.buf, ret);
    }
    else goto lblKO;
   
    CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
    
     
    
    // READ CardChipIdentification file (ID = 0005), size 8 bytes
    dspLS(0, "CardChipIdentification");  
    
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));
	
	// select file (INS = 0xA4) with ID = \x00\x05, TYP=0x03 -  some input, no ouput
	//                            TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x00\x05", rsp);
	com.commandType = SMC_ASYNC_COMMAND;
    cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
    cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
    cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
    cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x00\x05";
    com.commandIn = &cit;
    
    ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
    CHECK(ret == RET_RUNNING, lblKO);
    
    do {//waiting for result
            ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
    } 
    while(ret == RET_RUNNING);
    
    if(ret != RET_OK) 
    	goto lblKO;

    if(res.sta == RET_OK) {
         ret = res.len;
         memcpy(rsp, res.buf, ret);
    }
    else goto lblKO;
    
    CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
    
    
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));
	
	// // Read block (INS = 0xB0) of length Lc = 0x08 (=8=sizeof(CardChipIdentification) see P.119 of spec)
	//                              TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\x08", (byte *)"\x08", rsp);
	len = (int)(0x08);
	com.commandType = SMC_ASYNC_COMMAND;
    cit.asyncCommand.asyncCommandType =                    0x02; 
    cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xB0;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x08;
    cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
    com.commandIn = &cit;
    
    ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
    CHECK(ret == RET_RUNNING, lblKO);
    
    do {//waiting for result
            ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
    } 
    while(ret == RET_RUNNING);
    
    if(ret != RET_OK) 
    	goto lblKO;

    if(res.sta == RET_OK) {
         ret = res.len;
         memcpy(rsp, res.buf, ret);
    }
    else goto lblKO;
   
    CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
    
    
    
//// SELECT TACHO directory file (AID = TACHO) (change directory)
	  dspLS(0, "SELECT TACHO");
    
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
   	memset(&cit, 0, sizeof(cit));
   	
   	// // select file (INS = 0xA4) with AID = 0xFF\TACHO, TYP=0x03 -  some input, no ouput        
   	//                            TYP CLA INS P1  P2  Lc              Len    ID
   	//ret = iccCommand(0,(byte *) "\x03\x00\xA4\x04\x0C\x06", (byte *)"\x06\xFFTACHO", rsp);
   com.commandType = SMC_ASYNC_COMMAND;
   cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
   cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x04;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
   cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x06;
   cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\xFFTACHO";
   com.commandIn = &cit;
   
   ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
   CHECK(ret == RET_RUNNING, lblKO);
   
   do {//waiting for result
           ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
   } 
   while(ret == RET_RUNNING);
   
   if(ret != RET_OK) 
   	goto lblKO;

   if(res.sta == RET_OK) {
        ret = res.len;
        memcpy(rsp, res.buf, ret);
   }
   else goto lblKO;
   
   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
    

   // READ ApplicationIdentification file (ID = 0501), size 10 bytes
   dspLS(0, "ApplicationIdentification");  
   
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));

	//                            TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x01", rsp);
	com.commandType = SMC_ASYNC_COMMAND;
   cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
   cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
   cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
   cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x05\x01";
   com.commandIn = &cit;
   
   ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
   CHECK(ret == RET_RUNNING, lblKO);
   
   do {//waiting for result
           ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
   } 
   while(ret == RET_RUNNING);
   
   if(ret != RET_OK) 
   	goto lblKO;

   if(res.sta == RET_OK) {
        ret = res.len;
        memcpy(rsp, res.buf, ret);
   }
   else goto lblKO;
   
   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
   
   
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));

	//                              TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\x0A", (byte *)"\x0A", rsp);
	len = (int)(0x0A);
	com.commandType = SMC_ASYNC_COMMAND;
   cit.asyncCommand.asyncCommandType =                    0x02; 
   cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xB0;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x0A;
   cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
   com.commandIn = &cit;
   
   ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
   CHECK(ret == RET_RUNNING, lblKO);
   
   do {//waiting for result
           ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
   } 
   while(ret == RET_RUNNING);
   
   if(ret != RET_OK) 
   	goto lblKO;

   if(res.sta == RET_OK) {
        ret = res.len;
        memcpy(rsp, res.buf, ret);
   }
   else goto lblKO;
  
   CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
   
   // get the number of events records
   n1 = (int)rsp[3];
   bin2num(&n6,rsp+5,2);

   // READ CardCertificate file (ID = C100), size 194 bytes
   dspLS(0, "CardCertificate");  
   
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));

	//                             TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\xC1\x00", rsp);
   com.commandType = SMC_ASYNC_COMMAND;
   cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
   cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
   cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
   cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\xC1\x00";
   com.commandIn = &cit;
   
   ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
   CHECK(ret == RET_RUNNING, lblKO);
   
   do {//waiting for result
           ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
   } 
   while(ret == RET_RUNNING);
   
   if(ret != RET_OK) 
   	goto lblKO;

   if(res.sta == RET_OK) {
        ret = res.len;
        memcpy(rsp, res.buf, ret);
   }
   else goto lblKO;
   
   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
   
   
    memset(rsp,0,sizeof(rsp));
    memset(&com, 0, sizeof(com));
	memset(&cit, 0, sizeof(cit));

	//                              TYP CLA INS P1  P2  Lc              Len    ID
	//ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\xC2", (byte *)"\xC2", rsp);
	len = (int)(0xC2);
	com.commandType = SMC_ASYNC_COMMAND;
   cit.asyncCommand.asyncCommandType =                    0x02; 
   cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xB0;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x00;
   cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0xC2;
   cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
   com.commandIn = &cit;
   
   ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
   CHECK(ret == RET_RUNNING, lblKO);
   
   do {//waiting for result
           ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
   } 
   while(ret == RET_RUNNING);
   
   if(ret != RET_OK) 
   	goto lblKO;

   if(res.sta == RET_OK) {
        ret = res.len;
        memcpy(rsp, res.buf, ret);
   }
   else goto lblKO;
  
   CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);

   
// // READ Events file (ID = 0502), size = 6 * 24 * n1 bytes
	   dspLS(0, "Events");
    
     memset(rsp,0,sizeof(rsp));
     memset(&com, 0, sizeof(com));
 	 memset(&cit, 0, sizeof(cit));

 	//                             TYP CLA INS P1  P2  Lc              Len    ID
 	//ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x02", rsp);
    com.commandType = SMC_ASYNC_COMMAND;
    cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
    cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
    cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
    cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
    cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
    cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x05\x02";
    com.commandIn = &cit;
    
    ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
    CHECK(ret == RET_RUNNING, lblKO);
    
    do {//waiting for result
            ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
    } 
    while(ret == RET_RUNNING);
    
    if(ret != RET_OK) 
    	goto lblKO;

    if(res.sta == RET_OK) {
         ret = res.len;
         memcpy(rsp, res.buf, ret);
    }
    else goto lblKO;
    
    CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
    
	
	   len = 6 * 24 * n1;
	   ofs = 0;
	   blk = 0xFF;
	   while(len) {
	          if(len < blk)
	              blk = (byte) len;

                 memset(tmp, 0, sizeof(tmp));   	          
                 num2bin(ofsB,ofs,2);
	          
	             memset(rsp,0,sizeof(rsp));
	             memset(&com, 0, sizeof(com));
	             memset(&cit, 0, sizeof(cit));

	          	 com.commandType = SMC_ASYNC_COMMAND;
	             cit.asyncCommand.asyncCommandType =                    0x02; 
	             cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
	             cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xB0;
	             cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   ofsB[0];
	             cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   ofsB[1];
	             cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   blk;
	             cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
	             com.commandIn = &cit;
	             
	             ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
	             CHECK(ret == RET_RUNNING, lblKO);
	             
	             do {//waiting for result
	                     ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
	             } 
	             while(ret == RET_RUNNING);
	             
	             if(ret != RET_OK) 
	             	goto lblKO;

	             if(res.sta == RET_OK) {
	                  ret = res.len;
	                  memcpy(rsp, res.buf, ret);
	             }
	             else goto lblKO;
	            
	             CHECK(ret == blk+2 && rsp[blk] == 0x90 && rsp[blk + 1] == 0x00, lblKO);	   
	          
	          ofs += blk;
	          len -= blk;
       }
	          
	          
//	   
//   // READ Activity file (ID = 0504), size = 2 + 2 + n6 bytes
  	   dspLS(0, "Activity");

	        memset(rsp,0,sizeof(rsp));
	        memset(&com, 0, sizeof(com));
	    	 memset(&cit, 0, sizeof(cit));

	    	//                             TYP CLA INS P1  P2  Lc              Len    ID
	    	//ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x02", rsp);
	       com.commandType = SMC_ASYNC_COMMAND;
	       cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
	       cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
	       cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
	       cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
	       cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
	       cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
	       cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x05\x04";
	       com.commandIn = &cit;
	       
	       ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
	       CHECK(ret == RET_RUNNING, lblKO);
	       
	       do {//waiting for result
	               ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
	       } 
	       while(ret == RET_RUNNING);
	       
	       if(ret != RET_OK) 
	       	goto lblKO;

	       if(res.sta == RET_OK) {
	            ret = res.len;
	            memcpy(rsp, res.buf, ret);
	       }
	       else goto lblKO;
	       
	       CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	       
     	   len = 2 + 2 + n6;
     	   ofs = 0;
     	   blk = 0xFF;
     	   
	   	   while(len) {
	   	          if(len < blk)
	   	              blk = (byte) len;

	                    memset(tmp, 0, sizeof(tmp));   	          
	                    num2bin(ofsB,ofs,2);
	   	          
	   	             memset(rsp,0,sizeof(rsp));
	   	             memset(&com, 0, sizeof(com));
	   	             memset(&cit, 0, sizeof(cit));

	   	          	 com.commandType = SMC_ASYNC_COMMAND;
	   	             cit.asyncCommand.asyncCommandType =                    0x02; 
	   	             cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
	   	             cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xB0;
	   	             cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   ofsB[0];
	   	             cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   ofsB[1];
	   	             cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   blk;
	   	             cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
	   	             com.commandIn = &cit;
	   	             
	   	             ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
	   	             CHECK(ret == RET_RUNNING, lblKO);
	   	             
	   	             do {//waiting for result
	   	                     ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
	   	             } 
	   	             while(ret == RET_RUNNING);
	   	             
	   	             if(ret != RET_OK) 
	   	             	goto lblKO;

	   	             if(res.sta == RET_OK) {
	   	                  ret = res.len;
	   	                  memcpy(rsp, res.buf, ret);
	   	             }
	   	             else goto lblKO;
	   	            
	   	             CHECK(ret == blk+2 && rsp[blk] == 0x90 && rsp[blk + 1] == 0x00, lblKO);	   
	   	          
	   	          ofs += blk;
	   	          len -= blk;
	          }

// // PERFORM HASH of EF 
 	  dspLS(0, "PERFORM HASH");
// 	  // INS = 0x2A - perform security operation, 0x90 - perform hash, TYP=0x01 -  no input, no output             
// 	  // here P1 = 0x90 - perform hash
// 	  //      P2 = 0x00 means hash the currently selected EF

	  memset(rsp,0,sizeof(rsp));
      memset(&com, 0, sizeof(com));
      memset(&cit, 0, sizeof(cit));

    	//                              TYP CLA INS P1  P2  Lc              Len    ID
    	//ret = iccCommand(0,(byte *) "\x01\x80\x2A\x90\x00\x00", (byte *)"\x00", rsp);
       com.commandType = SMC_ASYNC_COMMAND;
       cit.asyncCommand.asyncCommandType =                    0x01; /// TYP
       cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x80;
       cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0x2A;
       cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x90;
       cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x00;
       cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x00;
       cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
       com.commandIn = &cit;
       
       ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
       CHECK(ret == RET_RUNNING, lblKO);
       
       do {//waiting for result
               ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
       } 
       while(ret == RET_RUNNING);
       
       if(ret != RET_OK) 
       	goto lblKO;

       if(res.sta == RET_OK) {
            ret = res.len;
            memcpy(rsp, res.buf, ret);
       }
       else goto lblKO;
       
       CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	   	   
//// COMPUTE SIGNATURE
 	  dspLS(0, "Signature");
// 	  // INS = 0x2A - perform security operation, 0x9E - compute signature, TYP=0x02 -   no input, some output           
// 	  // here P1 = 0x9E - compute signature
// 	  //      P2 = 0x9A - data field in the card to be signed (hash of the EF)
// 	  // Lc = 0x80 = 128 - expected length of digital signature
       
//                                      TYP CLA INS P1  P2  Lc              Len    ID
       	//ret = iccCommand(0,(byte *) "\x02\x00\x2A\x9E\x9A\x80", (byte *)"\x80", rsp);
       	com.commandType = SMC_ASYNC_COMMAND;
          cit.asyncCommand.asyncCommandType =                    0x02; 
          cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
          cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0x2A;
          cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x9E;
          cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x9A;
          cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x80;
          cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = 0;
          com.commandIn = &cit;
          
          ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
          CHECK(ret == RET_RUNNING, lblKO);
          
          do {//waiting for result
                  ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
          } 
          while(ret == RET_RUNNING);
          
          if(ret != RET_OK) 
          	goto lblKO;

          if(res.sta == RET_OK) {
               ret = res.len;
               memcpy(rsp, res.buf, ret);
          }
          else goto lblKO;
         
          CHECK(ret == 128+2 && rsp[128] == 0x90 && rsp[128+1] == 0x00, lblKO);   
          
          
//// WRITE DATA TO THE FILE
   	 		dspLS(0, "WRITE");
//   	 	// select file CardDownload (0x050E)	
//   		   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x0E", rsp);
   	 	      memset(rsp,0,sizeof(rsp));
   	 	      memset(&com, 0, sizeof(com));
   	 	      memset(&cit, 0, sizeof(cit));

   	 	       com.commandType = SMC_ASYNC_COMMAND;
   	 	       cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
   	 	       cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
   	 	       cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xA4;
   	 	       cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x02;
   	 	       cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x0C;
   	 	       cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x02;
   	 	       cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x05\x0E";;
   	 	       com.commandIn = &cit;
   	 	       
   	 	       ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
   	 	       CHECK(ret == RET_RUNNING, lblKO);
   	 	       
   	 	       do {//waiting for result
   	 	               ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
   	 	       } 
   	 	       while(ret == RET_RUNNING);
   	 	       
   	 	       if(ret != RET_OK) 
   	 	       	goto lblKO;

   	 	       if(res.sta == RET_OK) {
   	 	            ret = res.len;
   	 	            memcpy(rsp, res.buf, ret);
   	 	       }
   	 	       else goto lblKO;
   	 	       
   	 	       CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
   	 	       
//   		// INS = 0xD6 - write data, 0x9E - compute signature, TYP=0x02 -   no input, some output           
//   		 	  // here P1,P2 - offset within the file
//   		   	  // Lc - length of the data to write
//   		 	  //  Data : "\x49\x9D\xA6\x7D"            
//   		   ret = iccCommand(0,(byte *) "\x03\x00\xD6\x00\x00\x04", (byte *)"\x04\x49\x9D\xA6\x7D", rsp);
//   		   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
//	   
   	 	  memset(rsp,0,sizeof(rsp));
 	      memset(&com, 0, sizeof(com));
 	      memset(&cit, 0, sizeof(cit));

 	       com.commandType = SMC_ASYNC_COMMAND;
 	       cit.asyncCommand.asyncCommandType =                    0x03; /// TYP
 	       cit.asyncCommand.asyncCommand.asyncCommandType3.CLA =  0x00;
 	       cit.asyncCommand.asyncCommand.asyncCommandType3.INS =  0xD6;
 	       cit.asyncCommand.asyncCommand.asyncCommandType3.P1 =   0x00;
 	       cit.asyncCommand.asyncCommand.asyncCommandType3.P2 =   0x00;
 	       cit.asyncCommand.asyncCommand.asyncCommandType3.Lc =   0x04;
 	       cit.asyncCommand.asyncCommand.asyncCommandType4.ptIn = (uint8 *) "\x49\x9D\xA6\x7D";;
 	       com.commandIn = &cit;
 	       
 	       ret = smcCommandReq(hSmc, &com);   //here the command is sent to the card
 	       CHECK(ret == RET_RUNNING, lblKO);
 	       
 	       do {//waiting for result
 	               ret = smcResultGet(hSmc, SMC_OTHERS, sizeof(res), &res);
 	       } 
 	       while(ret == RET_RUNNING);
 	       
 	       if(ret != RET_OK) 
 	       	goto lblKO;

 	       if(res.sta == RET_OK) {
 	            ret = res.len;
 	            memcpy(rsp, res.buf, ret);
 	       }
 	       else goto lblKO;
 	       
 	       CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
 	       
 	       
	 dspLS(0, "done ok");
	 dspLS(1, "remove card");
	 tmrPause(3);
	   
	   goto lblEnd;
  
  lblKO:
    trcErr(ret);
  lblEnd:
      smcClose(hSmc);
      dspStop();
}
