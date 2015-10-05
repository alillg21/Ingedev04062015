/// \file tcNS0059.c
/** Unitary UTA test case : SMC Special Card READ
 * \sa
 */
#include "ctx.h"
#include <sdk30.h>
#include "sys.h"
#include "tst.h"

//the size of block to read write is limited to 248, see oem.h, SV_input_command and SV_output_command
#define OEM_BLK 248
void tcns0059(void) {
	  FILE *hSmc= 0;
    int ret = 0;
    int len;
    unsigned char  rsp[OEM_BLK]=""; // 
    unsigned char  blk; //block size
    int n1;/// N1 (see p 119) - number of event records to read
    card n6;// activity structure size
    word ofs=0;/// offset for reading large files by blocks
    HISTORIC atr;
    COMMAND_CAM cmd;
    unsigned char command[5]="";
    unsigned char data[OEM_BLK]="";// telium constraint  
    
        
    hSmc = fopen("CAM0", "rw");
    //if(hSmc == 0) {
    //      hSmc = stdperif("CAM0", 0);
    //}
    CHECK(hSmc, lblKO);
    
    ret = tmrStart(0, 30 * 100);    //30 seconds timeout
    CHECK(ret >= 0, lblKO);
    dspClear();
   
    while(tmrGet(0)) {
    	dspLS(0, "insert card");
    
        ret = SV_power_on(hSmc, &atr);
        
        //look for a result : if a card is inserted, capture ATR
        switch(ret){	       
	        case 1://! - 1 if card is absent ( returned only on CAM0, CAM1 ) 
	        case 7: //! - 7 if card has been removed 
	        	continue;	        
	        case 0://! ok	  
	        	break;
	        default:
	        	//! - 2 if card has not been processed 
	        	//! - 3 if card does not reply to reset 
	        	//! - 4 if VDC voltage fault detected 
	        	//! - 5 if VPP problem detected 
	        	//! - 6 if card communication fault detected 
	        	goto lblKO;
        } ;        
        //ATR may be    
        //! - Manufacturer's identifier 
        //! - Mask number 
        //! - Lock word 
        //! - Status word 1 
        //! - Status word 2 
        break;        
    }

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

// READ CardIccIdentification file (AID = 0002), size 25 bytes
    dspLS(0, "CardIccIdentification");   
	    // select file (INS = 0xA4) with ID = \x00\x02        
	    // here P1 = 0x02 means selection of elementary file by ID within the current directory
	    //      P2 = 0x0C means no response expected
   
        memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
        memcpy(data,(byte *)"\x00\x02",2);        
        cmd.data = data;
        cmd.header = command;
              
        ret = SV_input_command(hSmc, &cmd );
        CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);       

	   // Read block (INS = 0xB0) of length Lc = 0x19 (=25=sizeof(CardIccIdentification) see P.119 of spec)
	   // from the file which has been just selected 
	   // no input, some output
        
       memcpy(command,(byte *)"\x00\xB0\x00\x00\x19",5);   
       cmd.data = rsp;
       cmd.header = command;
               
       ret = SV_output_command(hSmc, &cmd);
       CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);
        
      
/// READ CardChipIdentification file (ID = 0005), size 8 bytes
   dspLS(0, "CardChipIdentification");  
   
   // select file (INS = 0xA4) with ID = \x00\x05
   // here P1 = 0x02 means selection of elementary file by ID within the current directory
   //      P2 = 0x0C means no response expected   
     memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
     memcpy(data,(byte *)"\x00\x05",2);        
     cmd.data = data;
     cmd.header = command;
           
     ret = SV_input_command(hSmc, &cmd );
     CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 ==0x00, lblKO);       

     // Read block (INS = 0xB0) of length Lc = 0x08 (=8=sizeof(CardChipIdentification) see P.119 of spec)
     //	  // from the file which has been just selected 
          
     memcpy(command,(byte *)"\x00\xB0\x00\x00\x08",5);   
     cmd.data = rsp;
     cmd.header = command;
            
     ret = SV_output_command(hSmc, &cmd);
     CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);
     
  
// SELECT TACHO directory file (AID = TACHO) (change directory)
	  dspLS(0, "SELECT TACHO");
	  // select file (INS = 0xA4) with AID = 0xFF\TACHO, TYP=0x03 -  some input, no ouput             
	  // here P1 = 0x04 means selection of elementary file by AID (by name) within the current directory
	  //      P2 = 0x0C means no response expected
     
	  memcpy(command,(byte *)"\x00\xA4\x04\x0C\x06",5);        
      memcpy(data,(byte *)"\xFFTACHO",6);        
      cmd.data = data;
      cmd.header = command;
            
      ret = SV_input_command(hSmc, &cmd );
      CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);       
     

// READ ApplicationIdentification file (ID = 0501), size 10 bytes
	  dspLS(0, "ApplicationIdentification");
	  
		memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
		memcpy(data,(byte *)"\x05\x01",2);        
		cmd.data = data;
		cmd.header = command;
		ret = SV_input_command(hSmc, &cmd );
		CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);       
		
		memcpy(command,(byte *)"\x00\xB0\x00\x00\x0A",5);   
		cmd.data = rsp;
		cmd.header = command;
		       
		ret = SV_output_command(hSmc, &cmd);
		CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);

	   
	   // get the number of events records
	   n1 = (int)rsp[3];
	   bin2num(&n6,rsp+5,2);
	   

// READ CardCertificate file (ID = C100), size 194 bytes
	   dspLS(0, "CardCertificate");
	   
		memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
		memcpy(data,(byte *)"\xC1\x00",2);        
		cmd.data = data;
		cmd.header = command;
		ret = SV_input_command(hSmc, &cmd );
		CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);       
		
		memcpy(command,(byte *)"\x00\xB0\x00\x00\xC2",5);   
		cmd.data = rsp;
		cmd.header = command;
		       
		ret = SV_output_command(hSmc, &cmd);
		CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);


 // READ Events file (ID = 0502), size = 6 * 24 * n1 bytes
	   dspLS(0, "Events");
	   
		memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
		memcpy(data,(byte *)"\x05\x02",2);        
		cmd.data = data;
		cmd.header = command;
		ret = SV_input_command(hSmc, &cmd );
		CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);       
		
   		
	   len = 6 * 24 * n1;
	   ofs = 0;
	   blk = OEM_BLK;
	   while(len) {		   
   	          if(len < blk)
   	              blk = (byte) len;

   	          memcpy(command,"\x00\xB0",2);	          
   	          num2bin(command+2,ofs,2);
   	          command[4] = (byte) blk;
   	       // here P1,P2 offset	 
   	          
   	          cmd.data = rsp;
   	          cmd.header = command;
   	          
   	          ret = SV_output_command(hSmc, &cmd);
   	          CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);
   	          
   	          
 	          ofs += blk;
 	          len -= blk;
        }
   	
	    	
	   
   // READ Activity file (ID = 0504), size = 2 + 2 + n6 bytes
  	   dspLS(0, "Activity");
  	   
   		memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
 		memcpy(data,(byte *)"\x05\x04",2);        
 		cmd.data = data;
 		cmd.header = command;
 		ret = SV_input_command(hSmc, &cmd );
 		CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);       
 		
    		
 	   len = 2 + 2 + n6;
 	   ofs = 0;
 	   blk = OEM_BLK;
 	   while(len) {		   
    	          if(len < blk)
    	              blk = (byte) len;

    	          memcpy(command,"\x00\xB0",2);	          
    	          num2bin(command+2,ofs,2);
    	          command[4] = (byte) blk;
    	       // here P1,P2 offset	 
    	          
    	          cmd.data = rsp;
    	          cmd.header = command;
    	          
    	          ret = SV_output_command(hSmc, &cmd);
    	          CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);
    	          
    	          
  	          ofs += blk;
  	          len -= blk;
         }
  	   
 // PERFORM HASH of EF 
 	  dspLS(0, "PERFORM HASH");
 	  // INS = 0x2A - perform security operation, 0x90 - perform hash, TYP=0x01 -  no input, no output             
 	  // here P1 = 0x90 - perform hash
 	  //      P2 = 0x00 means hash the currently selected EF
 	  
 	memcpy(command,(byte *)"\x80\x2A\x90\x00\x00",5);        
	memcpy(data,(byte *)"\x00",1);        
	cmd.data = data;
	cmd.header = command;
	ret = SV_input_command(hSmc, &cmd );
	CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);   
 	 
// COMPUTE SIGNATURE
 	  dspLS(0, "Signature");
 	  // INS = 0x2A - perform security operation, 0x9E - compute signature, TYP=0x02 -   no input, some output           
 	  // here P1 = 0x9E - compute signature
 	  //      P2 = 0x9A - data field in the card to be signed (hash of the EF)
 	  // Lc = 0x80 = 128 - expected length of digital signature
 	  
 	 memcpy(command,(byte *)"\x00\x2A\x9E\x9A\x80",5);   
	 cmd.data = rsp;
	 cmd.header = command;
 	 		       
	 ret = SV_output_command(hSmc, &cmd);
	 CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00,lblKO);
 	  
   	 
// WRITE DATA TO THE FILE
   	 		dspLS(0, "WRITE");
   	 		
   	 	//  select file CardDownload (0x050E)	
   	 	 memcpy(command,(byte *)"\x00\xA4\x02\x0C\x02",5);        
 	     memcpy(data,(byte *)"\x05\x0E",2);        
 	     cmd.data = data;
 	     cmd.header = command;
 	           
 	     ret = SV_input_command(hSmc, &cmd );
 	     CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 ==0x00, lblKO);       

//   		// INS = 0xD6 - write data, 0x9E - compute signature,         
//   		 	  // here P1,P2 - offset within the file
//   		   	  // Lc - length of the data to write
//   		 	  //  Data : "\x49\x9D\xA6\x7D"            
 	     
 	    memcpy(command,(byte *)"\x00\xD6\x00\x00\x04",5);        
    	memcpy(data,(byte *)"\x49\x9D\xA6\x7D",4);        
    	cmd.data = data;
    	cmd.header = command;
    	ret = SV_input_command(hSmc, &cmd );
    	CHECK(ret == 0 && cmd.statusword1 == 0x90 && cmd.statusword2 == 0x00, lblKO);   
 	     
	 dspLS(0, "done ok");
	 dspLS(1, "remove card");
	 tmrPause(3);
	   
	   goto lblEnd;
  
  lblKO:
    trcErr(ret);
  lblEnd:
  	if(hSmc)
      fclose(hSmc);
}
