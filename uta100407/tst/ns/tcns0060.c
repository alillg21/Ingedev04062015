
#include <string.h>
#include "sys.h"

#define BLK 248
void tcns0060(void) {
    int ret;
    int len;
    char *ptr=0;
    byte rsp[260];
    byte blk = BLK; //block size
    byte tmp[260];
    int n1;/// N1 (see p 119) - number of event records to read
    card n6;// activity structure size
    word ofs=0;/// offset for reading large files by blocks
    byte cmd[5+1]; /// 5 bytes of command + 1 byte of expected length

    ret = iccStart(0x10);          //Open associated channel and start waiting for a card 7816
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 60 * 100);    //60 seconds timeout
    CHECK(ret >= 0, lblKO);
    dspClear();
    
    len = 0;
    while(tmrGet(0)) {
    	dspLS(0, "insert card");
        ret = iccCommand(0, 0, 0, rsp);
        //look for a result : if a card is inserted, capture ATR
        if(ret == -iccCardRemoved || ret == 0)
            continue;
        CHECK(ret >= 0, lblKO);
        len = ret;
        break;
    }

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

    memset(rsp,0,sizeof(rsp));
    		/*
            * call iccCommand(byte rdr, const byte * cmd, const byte * dat, byte * rsp)
            * This function sends the command cmd followed by data dat to the card and captures the result into rsp.
            * Command consists of 5 bytes:
            * TYP: the type of command, can take values in the range 0..4:
            *  0 without any protocol
            *  1 no input, no output
            *  2 no input, some output
            *  3 some input, no ouput
            *  4 some input, some output
            * CLA: class byte
            * INS: instruction byte
            * P1: parameter 1 byte
            * P2: parameter 2 byte The data dat consists of the length of data and the data itself.
            * Lc: Length of data expected            
            */
    
// READ CardIccIdentification file (AID = 0002), size 25 bytes
    dspLS(0, "CardIccIdentification");   
	    // select file (INS = 0xA4) with ID = \x00\x02, TYP=0x03 -  some input, no ouput             
	    // here P1 = 0x02 means selection of elementary file by ID within the current directory
	    //      P2 = 0x0C means no response expected
	    //                           TYP CLA INS P1  P2  Lc              Len    ID
	    ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x00\x02", rsp);
	    // Check response code is 2 bytes long and is status OK : 0x90 0x00
	    CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	    
	   // Read block (INS = 0xB0) of length Lc = 0x19 (=25=sizeof(CardIccIdentification) see P.119 of spec)
	   // from the file which has been just selected 
	   // TYP=0x02 -  no input, some output
	   len = (int)(0x19);
	            //                   TYP CLA INS P1  P2  Lc             Len 
	   ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\x19", (byte *)"\x19", rsp);
	   // response code should be (len+2), 2 last bytes should be: 0x90 0x00
	   CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
   
// READ CardChipIdentification file (ID = 0005), size 8 bytes
	   dspLS(0, "CardChipIdentification");  
	   // select file (INS = 0xA4) with ID = \x00\x05, TYP=0x03 -  some input, no ouput
	   // here P1 = 0x02 means selection of elementary file by ID within the current directory
	   //      P2 = 0x0C means no response expected   
	               //               TYP CLA INS P1  P2  Lc              Len    ID
	   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x00\x05", rsp);
	   // Check response code is 2 bytes long and is status OK : 0x90 0x00
	   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	   
	  // Read block (INS = 0xB0) of length Lc = 0x08 (=8=sizeof(CardChipIdentification) see P.119 of spec)
	  // from the file which has been just selected 
	  // TYP=0x02 -  no input, some output
	  len = (int)(0x08);
	           //                   TYP CLA INS P1  P2  Lc             Len 
	  ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\x08", (byte *)"\x08", rsp);
	  // response code should be (len+2), 2 last bytes should be: 0x90 0x00
	  CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
	  
// SELECT TACHO directory file (AID = TACHO) (change directory)
	  dspLS(0, "SELECT TACHO");
	  // select file (INS = 0xA4) with AID = 0xFF\TACHO, TYP=0x03 -  some input, no ouput             
	  // here P1 = 0x04 means selection of elementary file by AID (by name) within the current directory
	  //      P2 = 0x0C means no response expected
	  //                            TYP CLA INS P1  P2  Lc              Len    ID  
	  ret = iccCommand(0,(byte *) "\x03\x00\xA4\x04\x0C\x06", (byte *)"\x06\xFFTACHO", rsp);
	  // Check response code is 2 bytes long and is status OK : 0x90 0x00
	  CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
     
// READ ApplicationIdentification file (ID = 0501), size 10 bytes
	  dspLS(0, "ApplicationIdentification");
	   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x01", rsp);
	   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   

	   len = (int)(0x0A);
	   ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\x0A", (byte *)"\x0A", rsp);
	   CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
	   
	   // get the nuber of events records
	   n1 = (int)rsp[3];
	   bin2num(&n6,rsp+5,2);
	   

// READ CardCertificate file (ID = C100), size 194 bytes
	   dspLS(0, "CardCertificate");
	   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\xC1\x00", rsp);
	   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	
	   len = (int)(0xC2);
	   ret = iccCommand(0,(byte *) "\x02\x00\xB0\x00\x00\xC2", (byte *)"\xC2", rsp);
	   CHECK(ret == len+2 && rsp[len] == 0x90 && rsp[len + 1] == 0x00, lblKO);
  
	   
 // READ Events file (ID = 0502), size = 6 * 24 * n1 bytes
	   dspLS(0, "Events");
	   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x02", rsp);
	   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	
	   len = 6 * 24 * n1;
	   ofs = 0;
	   blk = BLK;
	   while(len) {
	          if(len < blk)
	              blk = (byte) len;

	          memset(tmp, 0, sizeof(tmp));   	          
	          memcpy(cmd,"\x02\x00\xB0",3);	          
	          num2bin(cmd+3,ofs,2);
	          cmd[5] = (byte) blk;
	          
	    	  // here P1,P2 offset	          
	          ret = iccCommand(0,cmd, cmd+5, rsp);
	          CHECK(ret == blk+2 && rsp[blk] == 0x90 && rsp[blk + 1] == 0x00, lblKO);	   
	          
	          ofs += blk;
	          len -= blk;
       }
	   
   // READ Activity file (ID = 0504), size = 2 + 2 + n6 bytes
  	   dspLS(0, "Activity");
  	   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x04", rsp);
  	   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
  	
  	   len = 2 + 2 + n6;
  	   ofs = 0;
  	   blk = BLK;
  	   while(len) {
  	          if(len < blk)
  	              blk = (byte) len;

  	          memset(tmp, 0, sizeof(tmp));   	          
	          memcpy(cmd,"\x02\x00\xB0",3);	          
	          num2bin(cmd+3,ofs,2);
	          cmd[5] = (byte) blk;
  	          
  	          ret = iccCommand(0,cmd, cmd+5, rsp);
  	          CHECK(ret == blk+2 && rsp[blk] == 0x90 && rsp[blk + 1] == 0x00, lblKO);	   
  	          
  	          ofs += blk;
  	          len -= blk;
         }	     	   
  	   
 // PERFORM HASH of EF 
 	  dspLS(0, "PERFORM HASH");
 	  // INS = 0x2A - perform security operation, 0x90 - perform hash, TYP=0x01 -  no input, no output             
 	  // here P1 = 0x90 - perform hash
 	  //      P2 = 0x00 means hash the currently selected EF
 	  //                    TYP CLA INS P1  P2  Lc              Len    ID   	
 	 ret = iccCommand(0,(byte *) "\x01\x80\x2A\x90\x00\x00", (byte *)"\x00", rsp);
   	 CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
  	 
// COMPUTE SIGNATURE
 	  dspLS(0, "Signature");
 	  // INS = 0x2A - perform security operation, 0x9E - compute signature, TYP=0x02 -   no input, some output           
 	  // here P1 = 0x9E - compute signature
 	  //      P2 = 0x9A - data field in the card to be signed (hash of the EF)
 	  // Lc = 0x80 = 128 - expected length of digital signature
 	  //                           TYP CLA INS P1  P2  Lc              Len    ID   	
 	 ret = iccCommand(0,(byte *) "\x02\x00\x2A\x9E\x9A\x80", (byte *)"\x80", rsp);
   	 CHECK(ret == 128+2 && rsp[128] == 0x90 && rsp[128+1] == 0x00, lblKO);   
   	 
   	 
   	 
// WRITE DATA TO THE FILE
   	 		dspLS(0, "WRITE");
   	 	// select file CardDownload (0x050E)	
   		   ret = iccCommand(0,(byte *) "\x03\x00\xA4\x02\x0C\x02", (byte *)"\x02\x05\x0E", rsp);
   		   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
   	 
   		// INS = 0xD6 - write data, 0x9E - compute signature, TYP=0x02 -   no input, some output           
   		 	  // here P1,P2 - offset within the file
   		   	  // Lc - length of the data to write
   		 	  //  Data : "\x49\x9D\xA6\x7D"            
   		   ret = iccCommand(0,(byte *) "\x03\x00\xD6\x00\x00\x04", (byte *)"\x04\x49\x9D\xA6\x7D", rsp);
   		   CHECK(ret == 2 && rsp[0] == 0x90 && rsp[1] == 0x00, lblKO);   
	   
	 dspLS(0, "done ok");
	 dspLS(1, "remove card");
	 tmrPause(3);
	   
	   goto lblEnd;
  
  lblKO:
    trcErr(ret);
    dspLS(0, ptr);
  lblEnd:
     iccStop(0);
}
