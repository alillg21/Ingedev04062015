#include <string.h>
#include <stdio.h>

#include "pri.h"
#include "tst.h"
#ifdef __TEST__

void tcts0002(void){// testing hash functions
	byte hash2;
	byte cardno2[10];
	char tmp[30];

	prtStart();

	//equivalence testing
	prtS(" ");
	prtS("Card binary:");
	prtS("67609999070005001F");
	memcpy(cardno2,"\x67\x60\x99\x99\x07\x00\x05\x00\x1F",9);
	hash2= (byte)hashJerkinsBin(cardno2,9);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	prtS(" ");
	prtS("Card binary:");
	prtS("67609999070005001FFF");
	memcpy(cardno2,"\x67\x60\x99\x99\x07\x00\x05\x00\x1F\xFF",10);
	hash2= (byte)hashJerkinsBin(cardno2,10);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	prtS(" ");
	prtS("Card binary:");
	prtS("67609999070005001F+trash");
	memcpy(cardno2,"\x67\x60\x99\x99\x07\x00\x05\x00\x1F\xAB\xCD",11);
	hash2= (byte)hashJerkinsBin(cardno2,9);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	// be careful not equal

	prtS(" ");
	prtS("4905150000070053");
	memset(cardno2,0,sizeof(cardno2));
	memcpy(cardno2,"\x49\x05\x15\x00\x00\x07\x00\x53",8);
	hash2= (byte)hashJerkinsBin(cardno2,8);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);
	
	prtS(" ");
	prtS("Not same as");
	prtS("4905150000070053FFFF");
	memset(cardno2,0,sizeof(cardno2));
	memcpy(cardno2,"\x49\x05\x15\x00\x00\x07\x00\x53\xFF\xFF",10);
	hash2= (byte)hashJerkinsBin(cardno2,10);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	// testing the diversibility of increasing pan-s

	prtS(" ");
	prtS("4905150000070064");
	memset(cardno2,0,sizeof(cardno2));
	memcpy(cardno2,"\x49\x05\x15\x00\x00\x07\x00\x64",8);
	hash2= (byte)hashJerkinsBin(cardno2,8);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	prtS(" ");
	prtS("4905150000070075");
	memset(cardno2,0,sizeof(cardno2));
	memcpy(cardno2,"\x49\x05\x15\x00\x00\x07\x00\x75",8);
	hash2= (byte)hashJerkinsBin(cardno2,8);
	bin2hex(tmp,&hash2,1);
	tmp[2]=0;
	prtS(tmp);

	prtStop();
}

#endif
