#include <string.h>

#include "sys.h"
#include "tst.h"
#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcap0010.
//Security: Calculating MAC
//Application sp2P5.

/*
KSN= FFFF98789A12345678A0
BDK= 0123456789ABCDEFFEDCBA9876543210 
Dervive IK(Initial Key) from BDK 3-DES= FFFF98789A12345678A00123456789ABCDEFFEDCBA9876543210
Calculate key from ksn 'FFFF98789A12345678A0' and initial key 'D48265DA5D233E0FD9672D20F1672AA3'
Current key calculated to be 'C898F43F1E0B5647672C54F8C1CDFCBF'
PIN key calculated to be     'C898F43F1E0B56B8672C54F8C1CDFC40'
MAC key calculated to be     'C898F43F1E0BA947672C54F8C1CD03BF'


PAN '4407958642097539' is Luhn-10-stripped and left-padded with zeros to give '0000795864209753'
Clear PIN block '041234FFFFFFFFFF'
XOR'd PIN block '04124DA79BDF68AC'
Encoding PIN value '1234' with PAN '4407958642097539' and key-l 'C898F43F1E0B56B8', key-r '672C54F8C1CDFC40' [encode-mode is ISO format 0, 2 Key 3-DES]
[Pack PIN] desmode-3-desEnc([clearpinblock]04124DA79BDF68AC, [key A]C898F43F1E0B56B8)=[tmp1]-8DF58C269E46E7DC
[Pack PIN] desmode-3-desDec([tmp1]8DF58C269E46E7DC,     [key B]672C54F8C1CDFC40)=[tmp2]-1A5A6D96D7A34918
[Pack PIN] desmode-3-desEnc([tmp2]1A5A6D96D7A34918,     [key A]C898F43F1E0B56B8)=[encpinblock]-3F0DCD484692C73E


MAC input data: [313130303732333030373431323843323941303534343037393538363432303937353339303030303030303030303030303030303030303330303030303730323130313535373333FFFF98789A12345678A036303980]
Key-A [C898F43F1E0BA947] -- keyB [672C54F8C1CD03BF]
Padded data = '313130303732333030373431323843323941303534343037393538363432303937353339303030303030303030303030303030303030303330303030303730323130313535373333FFFF98789A12345678A0363039800000'

desEnc(AD4D47D163796E47, [A]-C898F43F1E0BA947) = ABAAA3C5DE37608F [final MAC]
MAC output data: [ABAAA3C5FFFFFFFF]
24798	'15:57:34.421	'< Adding message MAC: [ABAAA3C5FFFFFFFF]


[Dervive IK from BDK 3-DES] Calculate initial key from ksn 'FFFF98789A12345678A0' and bdk '0123456789ABCDEFFEDCBA9876543210'
[Dervive IK from BDK 3-DES] Initial key is 'FFFF98789A12345678A00123456789ABCDEFFEDCBA9876543210'
[Calc 3DES DUKPT Key] Calculate key from ksn 'FFFF98789A12345678A0' and initial key 'D48265DA5D233E0FD9672D20F1672AA3'
[Calc 3DES DUKPT Key] Current key calculated to be 'C898F43F1E0B5647672C54F8C1CDFCBF'
[Calc 3DES DUKPT Key] PIN key calculated to be     'C898F43F1E0B56B8672C54F8C1CDFC40'
[Calc 3DES DUKPT Key] MAC key calculated to be     'C898F43F1E0BA947672C54F8C1CD03BF'
Key-A [C898F43F1E0BA947] -- keyB [672C54F8C1CD03BF]
[3DES MAC] Padded data = '313131303732333030303131304543323841303534343037393538363432303937353339303030303030303030303030303030303030303330303030303730323130313535373333303030FFFF98789A12345678A03630398000000000000000'
MAC input data: [313131303732333030303131304543323841303534343037393538363432303937353339303030303030303030303030303030303030303330303030303730323130313535373333303030FFFF98789A12345678A036303980]
Validated MAC [76882C3AFFFFFFFF]
*/

int InitDUKPT(void)
{
    int ret;
	//byte locMK= 0x24;
	byte locMK= 0x12;
	//byte keySN[10]="\xFF\xFF\x12\x34\x56\x78\x21\x00\x00\x01"; //KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
	//byte keySN[10]="\xFF\xFF\x12\x34\x56\x78\x21\x00\x00\x00"; //KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
	byte keySN[10]="";//="\xFF\xFF\x98\x76\x54\x32\x10\xE0\x00\x00"; //KeySerialNumber 10 bytes long (KET SET ID,TRMS ID,TRANSACTION COUNTER)
	memcpy(keySN,"\xFF\xFF\x98\x76\x54\x32\x10\xE0\x00\x02",10);	
	ret= cry3DESDukptInit(locMK,keySN); CHECK(ret>=0,lblKo);

	return 1;    
	goto lblEnd;

lblKo: 
	prtS("Error in InitDUKPT");         
    goto lblEnd;
lblEnd:    
    return ret;
}

void tcap0010(void){
 int ret;
 //char Pan[16+1]="4407958642097539";
 char Pan[16+1]="4012345678909";
 //byte acc[8]="\x00\x00\x79\x58\x64\x20\x97\x53"; //account extracted from pan
 byte acc[8]="";//"\x00\x00\x40\x12\x34\x56\x78\x90"; //account extracted from pan
 byte blk[8+1]=""; //pinblock calculated by cryptomodule
 char msg1[dspW+1]=" "; 
 char msg2[dspW+1]="PIN:";
 char TraPan[16+1]="";
 byte loc= 0x06; //Mac key location
 //byte loc= 0x08; //Mac key location
 byte keySN[10]="";
 char tmp[prtW+1];
 byte crt[8]="";
 
    memcpy(acc, "\x00\x00\x40\x12\x34\x56\x78\x90", 8);
 	prtStop();
 	prtStart();

 	tctt0015(); 	
 	
    memset(blk,0,8);
    dspStop(); //close channel to give cryptomodule access to HMI
    cryStop('m');
    ret= cryStart('m'); CHECK(ret>=0,lblKo);
    
	ret= InitDUKPT(); CHECK(ret>=0,lblEnd);
			
	ret= cry3DESDukptGetPin_UpdateMacKey(msg1, msg2, acc, 120, keySN, blk,loc);	
	CHECK(ret>=0,lblKo);
		
	//print Pinblock 
	ret= prtS("PinBlock:");
	ret= bin2hex(tmp,blk,8);  
    ret= prtS(tmp); 

	//print macKey CheckSum
	ret= prtS("MacKey CheckSum:");
	ret= cryVerify(loc,crt);
	ret= bin2hex(tmp,crt,8);      
	ret= prtS(tmp);
	
    ret= dspStart(); CHECK(ret>=0,lblKo); //now we can open HMI again
		
	ret=1;
    goto lblEnd;

lblKo: 
	prtS("Error in tcap0010");         
    goto lblEnd;
lblEnd:
	if (ret!=1) prtS("Error!");
}

#endif
