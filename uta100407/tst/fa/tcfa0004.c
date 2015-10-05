/** \file
 * Unitary test case tcfa0004
 * Function testing:
 * \sa
 *  - Radix64Encode()
 *  - Radix64Decode()
 */

#include <string.h>
#include "pri.h"

#define CHK CHECK(ret>=0,lblKO)


void tcfa0004(void) {
   	byte encoded[1024]="";
   	byte Decoded[1024]="";	
	int  outDec;
	word outEnc;		

    trcS("tcfa0004 Beg\n");
  dspStart();
  prtStart();
//Example 0- When InputLen<=3, OutputLen = 4:
	prtS("Example 0:");
	
	outEnc = Radix64Encode((byte *)"red", 3, encoded);		
	//Expected Encoded Data.
	if(memcmp(encoded, "cmVk", outEnc) == 0)
		prtS("Encoded Data is Correct");
	else
		prtS("Encoded Data is Wrong");			
		
	outDec = Radix64Decode(encoded, Decoded, &outEnc);
	//Expected Decoded Data.
	if(memcmp(Decoded, "red", outDec) == 0)
		prtS("Decoded Data is Correct");
	else
		prtS("Decoded Data is Wrong");	
				
//Example 1- When InputLen=4,7,... OutputLen = 8,12,... with two padding character at the end
	prtS("Example 1:");
	
	outEnc = Radix64Encode((byte *)"Fire", 4, encoded);		
	//Expected Encoded Data.
	if(memcmp(encoded, "RmlyZQ==", outEnc) == 0)
		prtS("Encoded Data is Correct");
	else
		prtS("Encoded Data is Wrong");			
		
	outDec = Radix64Decode(encoded, Decoded, &outEnc);
	//Expected Decoded Data.
	if(memcmp(Decoded, "Fire", outDec) == 0)
		prtS("Decoded Data is Correct");
	else
		prtS("Decoded Data is Wrong");	
		
//Example 2- When InputLen=5,8 OutputLen = 8,12,... with one padding character ('=') at the end
	prtS("Example 2:");
	
	outEnc = Radix64Encode((byte *)"HELLO", 5, encoded);		
	//Expected Encoded Data.
	if(memcmp(encoded, "SEVMTE8=", outEnc) == 0)
		prtS("Encoded Data is Correct");
	else
		prtS("Encoded Data is Wrong");			
		
	outDec = Radix64Decode((byte *)"SEVMTE8=", Decoded, &outEnc);
	//Expected Decoded Data.
	if(memcmp(Decoded, "HELLO", outDec) == 0)
		prtS("Decoded Data is Correct");
	else
		prtS("Decoded Data is Wrong");	
		
//Example 3- When InputLen=6,9 OutputLen = 8,12,... Without Padding.
	prtS("Example 3:");
	
	outEnc = Radix64Encode((byte *)"HELLOO", 6, encoded);		
	//Expected Encoded Data.
	if(memcmp(encoded, "SEVMTE9P", outEnc) == 0)
		prtS("Encoded Data is Correct");
	else
		prtS("Encoded Data is Wrong");			
		
	outDec = Radix64Decode(encoded, Decoded, &outEnc);
	//Expected Decoded Data.
	if(memcmp(Decoded, "HELLOO", outDec) == 0)
		prtS("Decoded Data is Correct");
	else
		prtS("Decoded Data is Wrong");			
		
//Example 4- When InputLen=13,16,... OutputLen = 20,24,... with two padding characters ('=') at the end
	prtS("Example 4:");
	
	outEnc = Radix64Encode((byte *)"Solution Gulf", 13, encoded);		
	//Expected Encoded Data.
	if(memcmp(encoded, "U29sdXRpb24gR3VsZg==", outEnc) == 0)
		prtS("Encoded Data is Correct");
	else
		prtS("Encoded Data is Wrong");			
		
	outDec = Radix64Decode(encoded, Decoded, &outEnc);
	//Expected Decoded Data.
	if(memcmp(Decoded, "Solution Gulf", outDec) == 0)
		prtS("Decoded Data is Correct");
	else
		prtS("Decoded Data is Wrong");	

    goto lblEnd;
 // lblKO:
 //   dspLS(0, "Error");
  lblEnd:
    prtStop();
    dspStop();
    trcS("tcfa0004 End\n");
}
