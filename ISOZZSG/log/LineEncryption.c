#include "sdk30.h"
#include "Libgr.h"
#include "sys.h"
#include "log.h"
#include "lineEncryption.h"
#include "rsa.h"
#include "stdlib.h" //@@AS0.0.10
#include "AES.h" //@@AS0.0.10

#include "crypto_def.h"
#include "Telium_crypto.h"
#include "bri.h"

#include "rsagen.h" //@@AS0.0.28
//#include "prsa.h" //@agmr

#ifdef LINE_ENCRYPTION

extern int genRSAKeyPair(int keyBitSize,int exponent,unsigned char *pubMod,int *pubModLen,unsigned char *privExp,int *privExpLen,unsigned char *pubExp,int *pubExpLen) ;

#define CHK if(ret<0) return ret;

unsigned char encryptedField[64] = {2,14,35,48,0};
unsigned char xsec_derivation_data [DERIVATION_DATA_LEN]; //@@AS0.0.10
unsigned char xsec_UMK [AES_KEY_LEN+1]; //@@AS0.0.10
unsigned char xsec_USK [AES_KEY_LEN+1]; //@@AS0.0.10
unsigned char xsec_TLE [16+1]; //@@AS-SIMAS


int RSA_padding_check_PKCS1_type_2 (unsigned char *to, int tlen,unsigned char *from, int flen)
{
	int i,j;
	unsigned char *p;

	p=from;

	if (*(++p) != 02)
	{
		//RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_BLOCK_TYPE_IS_NOT_02);
		return(-1);
	}

	/* scan over padding data */
	j=flen-1; /* one for type. */
	for (i=0; i<j; i++)
	if (*(p++) == 0) break;

	if (i == j)
	{

		//RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_NULL_BEFORE_BLOCK_MISSING);
		return(-2);
	}

	if (i < 8)
	{
		//RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_BAD_PAD_BYTE_COUNT);
		return(-3);
	}
	i++; /* Skip over the '\0' */
	j-=i;
	if (j > tlen)
	{
		//RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,RSA_R_DATA_TOO_LARGE);
		return(-4);
	}
	memcpy(to,p,(unsigned int)j);

	return(j);
}


int getField60(tBuffer *req)
{
    byte txnType;
    byte buf[512];
    byte data[256];
    int ret;
    byte *ptr;
    char Aid[lenAID + 1];
    byte Fallback;

    ptr = buf;
    MAPGETBYTE(traTxnType,txnType,lblKO);
    memset(buf,0,sizeof(buf));

    MAPGET(emvAid, Aid, lblKO);
     switch(txnType)
    {
        case trtTLEKeyDownload:

            *ptr++ = 128/0x100;
            *ptr++ = 128%0x100;

            memset(data,0,sizeof(data));
            mapGet(appPublicKeyModulus,data,128);
            memcpy(ptr,data,128);
            ptr+=128;

            *ptr++ = 3/0x100;
            *ptr++ = 3%0x100;

            memset(data,0,sizeof(data));
            mapGet(appPublicKeyExponent,data,3);
            memcpy(ptr,data,3);
            ptr+=3;
            break;

	default:
		MAPGET(emvAid, Aid, lblKO);
		if(memcmp(&Aid[1],  "\xA0\x00\x00\x03\x33",  5)==0)
		{
			//data 1-4
			memcpy(ptr,"0000",4);
            		ptr+=4;

			//data 5
			memcpy(ptr,"0",1);
            		ptr++;

			MAPGETBYTE(traField22,Fallback,lblKO);
			//data 6
			memcpy(ptr,"6",1);
            		ptr++;

			MAPGETBYTE(traFallback,Fallback,lblKO);
			if(Fallback == 1)
			{
				//data 7
				memcpy(ptr,"2",1);
            			ptr++;
			}
			else
			{
				//data 7
				memcpy(ptr,"0",1);
            			ptr++;
			}

			//data 8
			memcpy(ptr,"0",1);
            		ptr++;

			//data 9-10
			memcpy(ptr,"00",2);
            		ptr+=2;

			//data 11-14
			memcpy(ptr,"0000",4);
            		ptr+=2;

		}
		break;
    }

	

    ret = bufApp(req, (byte *) buf,ptr-buf);
    CHK;
    return bufLen(req);

lblKO:
    return -1;
}

//@@AS-SIMAS - start
void MACX9_19( byte *key,byte * dataIn, word inLen ,byte * mac)
{
    word i;
    byte next_vector[8],encResult[8];
    byte num_of_blks;
    byte k1[16],k2[16];

	num_of_blks = inLen / 8;
	memcpy(k1,key,8);
	memcpy(&k1[8],key,8);
	memcpy(k2,key,16);

	/*    Clear out the MAC    */
	memset( mac, 0, 8 );
    /*    Generate the mab    */
    for ( ; num_of_blks > 0; num_of_blks--, dataIn += 8 )
    {

        /*
        ** MAC'ING
        ** The current feed back vector will be XORed with the
        ** data, then the data (after encryption) becomes the next
        ** feed back vector.
        */
        memcpy( next_vector, dataIn, 8 );
        for ( i = 0; i < 8; i++)
            next_vector[i] ^= mac[i];
		if (num_of_blks>1)
			stdEDE(encResult,next_vector, k1);
		else
			stdEDE(encResult,next_vector, k2);
		memcpy(next_vector,encResult,8);
#ifdef _DEBUG_TLE
		hex_dump_char("--NEXT VECTOR--",next_vector,8);
#endif //_DEBUG_TLE
        /*    Update feedback vector    */
        memcpy( mac, next_vector,8);
	}
	/* store for chaining next time */
	memset(mac+4,0,4);
}


void calcMAC(byte *key,byte *dataIn,word inLen,byte *dataOut)
{
	byte digest[24];

	memset(digest,0x00,sizeof(digest));
	ShaCompute(dataIn,inLen,digest);

#ifdef _DEBUG_TLE
	hex_dump_char("--SHA1-HASH--",digest,24);
#endif //_DEBUG_TLE

	MACX9_19(key,digest,24 ,dataOut);
}

//@@AS-SIMAS - end


int shaData(char *data)
{
    char buf[120];
    char tid[20];
    char pwd[20];
    char fld13[20];
    char fld12[20];
    char userId[20];
    int ret;
    byte digest[20];

    memset(userId,0,sizeof(userId));
    ret = mapGet(traTLEUserId,userId,sizeof(userId));

    memset(pwd,0,sizeof(pwd));
    ret = mapGet(traTLEPwd,pwd,sizeof(pwd));

    memset(tid,0,sizeof(tid));
    ret = mapGet(acqTID, tid, sizeof(tid));

    memset(fld13,0,sizeof(fld13));
    memset(fld12,0,sizeof(fld12));
    ret = mapGet(traDatTim, buf, sizeof(buf));
#ifdef _DEBUG_TLE
    hex_dump("traDatTim",buf,15);
#endif //_DEBUG_TLE
    memcpy(fld13,buf+4,4); //@@AS0.0.10
    memcpy(fld12,buf+8,6); //@@AS0.0.10

    memset(buf,0,sizeof(buf));
    strcpy(buf,userId);
    strcat(buf,pwd);
    strcat(buf,tid);
    strcat(buf,fld13);
    strcat(buf,fld12);

    ShaCompute(buf,strlen(buf),digest);
#ifdef _DEBUG_TLE
	hex_dump_char("--SHA1-HASH SDK--",digest,20);
#endif //_DEBUG_TLE
	sprintf(data,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			digest[0],digest[1],digest[2],digest[3],digest[4],digest[5],digest[6],digest[7],digest[8],digest[9],
			digest[10],digest[11],digest[12],digest[13],digest[14],digest[15],digest[16],digest[17],digest[18],digest[19]);

#ifdef _DEBUG_TLE
{
    char tmp[50];

    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,data,20);
    prtS(tmp);

    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,data+20,20);
    prtS(tmp);
}
#endif //_DEBUG_TLE

	return 1;
}

#if 0
void testSHA()
{
    char buf[120];
    SHA1Context sha;


    strcpy(buf,"1234567890");
    SHA1Reset(&sha);
    SHA1Input(&sha, buf, strlen(buf));

    if (!SHA1Result(&sha))
    {
        prtS("SHA ERROR");
    }

    sprintf(buf,"%08lX",sha.Message_Digest[0]);
    prtS(buf);
//    strcpy(data,buf);

    sprintf(buf,"%08lX",sha.Message_Digest[1]);
    prtS(buf);
//    strcat(data,buf);

    sprintf(buf,"%08lX",sha.Message_Digest[2]);
    prtS(buf);
//    strcat(data,buf);

    sprintf(buf,"%08lX",sha.Message_Digest[3]);
    prtS(buf);
//    strcat(data,buf);

    sprintf(buf,"%08lX",sha.Message_Digest[4]);
    prtS(buf);
//    strcat(data,buf);

    /*
{
    char tmp[50];

    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,data,20);
    prtS(tmp);

    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,data+20,20);
    prtS(tmp);
}
*/
}
#endif
void generate_random_data (byte *buffer, word length)
{
    word           i;

    for (i = 0; i < length; i++)
        *(buffer + i) = (byte) (rand () % 256);
}

#if 0
void testRSA()
{
//typedef struct
//{
//  unsigned short Option;
//  unsigned short KeySize;    /*size of key in bits*/
//  void* Modulus;             /*modulus represented in bytes with MSB first*/
//  void* PublicExponent;
//  void* PrivateExponent;     /*set this and following fields to null if not present or required*/
//  void* Prime1;              /*primes used to make up the modulus (used for CRT)*/
//  void* Prime2;
//  void* InvPrime1ModPrime2;  /*inverse of prime 1 mod prime 2*/
//} RSA_t;

	byte rsaBuf[10000];
    int ret;
    RSA_t key;
    byte modulus[128] =
    {
        0x9b,0x1f,0xc4,0x6c,0x36,0x79,0xca,0xcf,0xe5,0xa1,0x9b,0x63,0x1c,0xe9,0xc0,
        0x2a,0xeb,0x17,0x25,0x99,0xd1,0xd0,0x00,0xfa,0x90,0x86,0x78,0x53,0xa3,0x14,
        0x79,0x69,0x00,0x11,0xe6,0xd9,0x42,0xf0,0x43,0xe5,0x98,0xef,0x6a,0xdd,0x22,
        0xfa,0xe9,0x5a,0x37,0x83,0x64,0xf0,0xb8,0xb3,0xe4,0x54,0xb4,0xe5,0x28,0xa6,
        0x0e,0x8a,0x6b,0x37,0x47,0x96,0x16,0x22,0x51,0x8e,0x8f,0x15,0x27,0x55,0x4c,
        0x01,0xc0,0x6a,0xdc,0x4f,0xd6,0xd4,0xd9,0x6d,0x8e,0xc2,0x69,0x01,0xb6,0x02,
        0x49,0x93,0x40,0xeb,0xf8,0x9d,0x31,0xbe,0x95,0x1e,0x53,0x77,0xc9,0xda,0xb7,
        0x9b,0xb8,0x58,0x2d,0x02,0xd9,0x6a,0x66,0xbf,0x20,0xa6,0x54,0xc0,0x0f,0x99,
        0x3e,0x08,0xcc,0xd4,0x0b,0x5a,0x13,0xe1
    };
    byte PublicExponent[3]={0x01,0x00,0x01};

    key.Option = RSA_EXPONENT_805;
    key.KeySize = 1024;

    key.Modulus = modulus;
    key.PublicExponent = PublicExponent;
    key.PrivateExponent = NULL;
    key.Prime1 = NULL;
    key.Prime2 = NULL;
    key.InvPrime1ModPrime2 = NULL;
//    key.PrivateExponent = PrivateExponent;
//    key.Prime1 = Prime1;
//    key.Prime2 = Prime2;
//    key.InvPrime1ModPrime2 = Inv;
    memset(modulus,0,sizeof(modulus));
    memset(PublicExponent,0,sizeof(PublicExponent));
    memset(rsaBuf,0,sizeof(rsaBuf));
    //ret=SEC_RandomData(sizeof(rsaBuf),rsaBuf);
    //generate_random_data(rsaBuf,sizeof(rsaBuf));

    RSA_init((void *)rsaBuf,sizeof(rsaBuf));
#ifdef _DEBUG_TLE
    printRSA_t(&key);
#endif //_DEBUG_TLE
    ret = RSA_Make(&key);
#ifdef _DEBUG_TLE
    sprintf(buf,"RSA_Make = %i",ret);
    prtS(buf);
    printRSA_t(&key);
#endif //_DEBUG_TLE

    ret = RSA_TestKey(&key);
#ifdef _DEBUG_TLE
    sprintf(buf,"RSA_TestKey = %i",ret);
    prtS(buf);
#endif //_DEBUG_TLE
/*
    memset(dest,0,sizeof(dest));
    memset(src,0,sizeof(src));

    ret = RSA_Encrypt(dest, src, &key);
    sprintf(buf,"RSA_Encrypt = %i",ret);
    prtS(buf);

    ret = RSA_Decrypt(dest, src, &key);
    sprintf(buf,"RSA_Decrypt = %i",ret);
    prtS(buf);
*/
}
#endif
#ifdef _DEBUG_TLE
void printRSA_t(RSA_t* t)
{
//typedef struct
//{
//  unsigned short Option;
//  unsigned short KeySize;    /*size of key in bits*/
//  void* Modulus;             /*modulus represented in bytes with MSB first*/
//  void* PublicExponent;
//  void* PrivateExponent;     /*set this and following fields to null if not present or required*/
//  void* Prime1;              /*primes used to make up the modulus (used for CRT)*/
//  void* Prime2;
//  void* InvPrime1ModPrime2;  /*inverse of prime 1 mod prime 2*/
//} RSA_t;
    int len;

    byte buf[20];
    prtS("**** RSA_t ****");
    prtS("Option :");
    sprintf(buf,"%i",t->Option);
    prtS(buf);

    prtS("KeySize :");
    sprintf(buf,"%i",t->KeySize);
    prtS(buf);


    prtS("Modulus :");
    ShowData(t->Modulus,t->KeySize/8,0,0,8);


    if(t->Option == RSA_EXPONENT_3)
        len = 1;
    if(t->Option == RSA_EXPONENT_805)
        len = 2;
    if(t->Option == RSA_EXPONENT_10001)
        len = 3;

    prtS("PublicExponent :");
    ShowData(t->PublicExponent,len,0,0,8);

    prtS("PrivateExponent :");
    if(t->PrivateExponent != NULL)
        ShowData(t->PrivateExponent,t->KeySize/8,0,0,8);

    prtS("Prime1 :");
    if(t->Prime1 != NULL)
        ShowData(t->Prime1,10,0,8,8);

    prtS("Prime2 :");
    if(t->Prime2 != NULL)
        ShowData(t->Prime2,10,0,1,10);

    prtS("InvPrime1ModPrime2 :");
    if(t->InvPrime1ModPrime2 != NULL)
        ShowData(t->InvPrime1ModPrime2,10,0,1,10);
}
#endif

int checkKey()
{
/*** hanya memeriksa apakah sudah pernah create key atau belum ***/    
    int ret;
    char buf[256];

    memset(buf,0,sizeof(buf));
    ret = mapGet(appPublicKeyModulus,buf,lenPublicKeyModulus);
    CHECK(ret>=0,lblKO);
    if(memcmp(buf,__DFTMODULUS__,lenPublicKeyModulus) == 0)
        return 0;
        
    memset(buf,0,sizeof(buf));
    ret = mapGet(appPrivateKeyModulus,buf,lenPrivateKeyModulus);
    CHECK(ret>=0,lblKO);
    if(memcmp(buf,__DFTMODULUS__,lenPrivateKeyModulus) == 0)
        return 0;
    return 1;
lblKO:
    return -1;         
}

//int checkAcqTleKey(byte displayMsg)
//{
///*** memeriksa apakah acquirer sudah pernah download key atau belum
//     mapMove(acqBeg,acqId) diluar fungsi ini
//***/    
//    int ret;
//    char buf[256];
//
//#ifndef LINE_ENCRYPTION
//    return 1;
//#endif
//
//    memset(buf,0,sizeof(buf));
//    ret = mapGet(acqUMK,buf,lenPublicKeyModulus);
//    CHECK(ret>=0,lblKO);
//    if(memcmp(buf,__DFTUMK__,lenUMK) == 0)
//    {
//        if(displayMsg)
//        {
//            usrInfo(infTleKeyEmpty);
//        }
//        return 0;
//    }
//
//    return 1;
//lblKO:
//    return -1;         
//}

void tleMenu()
{
	char mnu[MNUMAX][dspW + 1]; //the final menu array prepared to mnuSelect
	char *ptr[MNUMAX];          //array of pointers to mnu items
    int sta=0;  
    int ret;

    strcpy(mnu[0],"DOWNLOAD KEY");
    strcpy(mnu[1],"Download key");
    strcpy(mnu[2],"Create & Dwnld");
    
    memset(ptr,0,sizeof(ptr));
    ptr[0] = mnu[0];
    ptr[1] = mnu[1];
    ptr[2] = mnu[2];

	ret = mnuSelect((Pchar *) ptr, sta, 60);    //perform user dialog
	
	CHECK(ret >= 0, lblEnd);

	if(!ret)
	{                  // Timeout or aborted - nothing to do
		goto lblEnd;
	}

	ret %= MNUMAX;              // Extract the item number selected from menu state
//	ret--;                      // Decrement it since zero item is menu title
	
	if(ret == 1)
	{
	    tleProcess(0);
	    goto lblEnd;
	}
	if(ret == 2)
	{
	    tleProcess(1);
	}

lblEnd:
    return;
}

void tleProcess(byte needCreateKey)
{
    int ret,i,awal,akhir;
    ACQ_T acq[dimAcq];
    byte jmlTid;
            
    if(needCreateKey || checkKey() <=0 )
    {
        if(tleGetKey() <= 0)
            return;
    }

    if(tleInputUserPass() <=0)
        return;
                    
    memset(acq,0,sizeof(acq));

    ret = selectTerminalId(acq,&jmlTid);
    if(ret <= 0)
    {
         prtS("PROSES TLE GAGAL");
         return;
    }
    if(ret == 1) //semua TID
    {
        awal = 0;
        akhir = jmlTid;
    }
    else
    {
        awal = ret-2;
        akhir = awal+1;
    }
    for(i=awal;i<akhir;i++)
    {
        char buf[50];
        char tid[9];
        
        mapMove(acqBeg,acq[i].acqIdx-1);
        memset(tid,0,sizeof(tid));
        mapGet(acqTID,tid,8);
        sprintf(buf,"  TID %s",tid);
        dspLS(2,buf);
        
        if(tleKeyDownload() < 0)
        {
//            return;
            sprintf(buf,"TLE TID %s GAGAL",tid);
            prtS(buf);
            continue;
        }
        if(tleDecryptAndStoreUMK() < 0)
        {
//            return;
            sprintf(buf,"TLE TID %s GAGAL",tid);
            prtS(buf);
            continue;
        }
        sprintf(buf,"TLE TID %s SUKSES",tid);
        prtS(buf);
    }
      
    rptReceipt(rloFeed);
    return;
}

int insertChip() 
{
    int ret = 0,icc, kbd;
    int sta;
    char buf[256];
    
    memset(buf,0,sizeof(buf));
//    icc = -1;
//    kbd = -1;
    icc = iccStart(0x00);
    kbd = kbdStart(1);
    while(1) {
        //Joshua _c for ISO3
        sta = ttestall(KEYBOARD|CAM0 , 30 * 100);  // Wait for an event
        CHECK(sta != 0, lblEnd);    // Exit if no event occured after 30s        
        if(iccDetect(0x00) == 1)
        {
            ret = 'c';
            goto lblEnd;
        }

        if(kbd >= 0) {          //we process keyboard?
            ret = kbdKey();
            CHECK(ret != '\xFF', lblKO);    //errors are not acceptable        
            if(ret) {           //key pressed
                switch (ret) {
                  case kbdANN:
                      *buf = ret;   //put it into buffer
                      ret = 'k';
                      goto lblEnd;
                  default:
                      utaBeep();
                      kbdStop();
                      ret = kbdStart(1);
                      CHECK(ret, lblKO);
                      continue;
                }
                goto lblEnd;
            }
        }
    }

  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    if(icc >= 0)
        iccStop(0);
    kbdStop();
    return ret;
}


#ifdef NOT_USED //@@AS0.0.28
int tleGetKey()
{
    int ret;
#ifdef _DEBUG_TLE
    char buf[50];
#endif
    byte cmd[50];
    byte resp[1024];
    byte data[50];

    memset(resp,0,sizeof(resp));

    dspClear();
    dspLS(2,"   Insert Card");
    
    ret = insertChip();
    if(ret != 'c')
        return 0;    
    
    ret = iccStart(0x10);
#ifdef _DEBUG_TLE
    sprintf(buf,"iccStart = %i",ret);
    prtS(buf);
#endif //_DEBUG_TLE
     
//    while(1)
//    {
//        if(iccDetect(0x00) == 1)
//            break;
//    }

    if(iccDetect(0x00) == 0)
        return 0;    

    memset(cmd,0,sizeof(cmd));
    memset(resp,0,sizeof(resp));

    //POWER ON
    ret = iccCommand(0x00,0x00,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc POWER ON = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE

    //SELECT
    memset(cmd,0,sizeof(cmd));
    memset(resp,0,sizeof(resp));
    memset(data,0,sizeof(data));
    memcpy(cmd,"\x03\x00\xA4\x04\x00\x07",6);
    memcpy(data,"\x07\xA0\x4E\x59\x52\x41\x20\x21",8);
    ret = iccCommand(0x00,cmd,data,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc SELECT = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE
    if(ret == 2 && resp[0] == 0x6c)
    {
        cmd[5] = resp[1];
        ret = iccCommand(0x00,cmd,data,resp);
#ifdef _DEBUG_TLE_1
        sprintf(buf,"icc SELECT 2= %i",ret);
        prtS(buf);
        if(ret >=0)
            ShowData(resp,ret,0,0,8);
#endif
    }

    //CLEAR RSA
    memcpy(cmd,"\x02\x20\x22\x00\x00\x00",6);
    ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc CLEAR = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE

    //GENERATE KEY PAIR
    memset(cmd,0,sizeof(cmd));
    memset(resp,0,sizeof(resp));
    memcpy(cmd,"\x02\x20\x20\x01\x01\x00",6);
    ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc KEY PAIR = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE

    //0x00 0x00 Get public key - modulus
    memset(cmd,0,sizeof(cmd));
    memset(resp,0,sizeof(resp));
    memcpy(cmd,"\x02\x20\x24\x00\x00\x00",6);
    ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc BMODULUS = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE
    if(ret == 2 && resp[0] == 0x6c)
    {
        cmd[5] = resp[1];
        ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
        sprintf(buf,"icc BMODULUS 2= %i",ret);
        prtS(buf);
        if(ret >=0)
            ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE
        mapPut(appPublicKeyModulus,resp,128);
    }

    //0x00 0x01 Get public key - exponent
    memcpy(cmd,"\x02\x20\x24\x00\x01\x00",6);
    ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc BEXPONEN = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif
    if(ret == 2 && resp[0] == 0x6c)
    {
        cmd[5] = resp[1];
        ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
        sprintf(buf,"icc BEXPONEN 2= %i",ret);
        prtS(buf);
        if(ret >=0)
            ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE
        mapPut(appPublicKeyExponent,resp,ret-2);
    }

    //0x01 0x05 Get private key - modulus
    memcpy(cmd,"\x02\x20\x24\x01\x05\x00",6);
    ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc VMODULUS = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif //_DEBUG_TLE
    if(ret == 2 && resp[0] == 0x6c)
    {
        cmd[5] = resp[1];
        ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
        sprintf(buf,"icc VMODULUS = %i",ret);
        prtS(buf);
        if(ret >=0)
            ShowData(resp,ret,0,0,8);
#endif
        mapPut(appPrivateKeyModulus,resp,128);
    }

    //0x01 0x06 Get private key - exponent
    memcpy(cmd,"\x02\x20\x24\x01\x06\x00",6);
    ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"icc VEXPONEN = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif
    if(ret == 2 && resp[0] == 0x6c)
    {
        cmd[5] = resp[1];
        ret = iccCommand(0x00,cmd,NULL,resp);
#ifdef _DEBUG_TLE_1
        sprintf(buf,"icc VEXPONEN 2 = %i",ret);
        prtS(buf);
        if(ret >=0)
            ShowData(resp,ret,0,0,8);
#endif
        mapPut(appPrivateKeyExponent,resp,128);
    }

    ret = iccStop(0x00);
#ifdef _DEBUG_TLE_1
    sprintf(buf,"iccStop = %i",ret);
    prtS(buf);
    if(ret >=0)
        ShowData(resp,ret,0,0,8);
#endif
return 1;
}
#else //NOT_USED
int tleGetKey() //@@AS0.0.28
{
    int ret;
    unsigned char pubMod[129],privExp[129],pubExp[10];
    int pubModLen,privExpLen,pubExpLen;
#ifdef _DEBUG_TLE
    char buf[50];
#endif


    dspClear();
    dspLS(2,"Please Wait...");


    ret=genRSAKeyPair(1024,65537,pubMod,&pubModLen,privExp,&privExpLen,pubExp,&pubExpLen);
    if (ret!=0)
    	return -1;

#ifdef _DEBUG_TLE_1
    prtS("pubMod");
    if(ret >=0)
        ShowData(pubMod,pubModLen,0,0,8);
#endif //_DEBUG_TLE
    mapPut(appPublicKeyModulus,pubMod,pubModLen);

#ifdef _DEBUG_TLE_1
    prtS("pubExp");
    if(ret >=0)
        ShowData(pubExp,pubExpLen,0,0,8);
#endif //_DEBUG_TLE
    mapPut(appPublicKeyExponent,pubExp,pubExpLen);

    mapPut(appPrivateKeyModulus,pubMod,pubModLen);

#ifdef _DEBUG_TLE_1
    prtS("privExp");
    if(ret >=0)
        ShowData(privExp,privExpLen,0,0,8);
#endif //_DEBUG_TLE
    mapPut(appPrivateKeyExponent,privExp,privExpLen);

return 1;
}
#endif //NOT_USED

int tleInputUserPass()
{
    int ret;
//    char traName[dspW + 1];
    char user[10];
    char pwd[10];
        
    memset(user,0,sizeof(user));
    while (strlen(user) <8)  {
		dspClear();
		dspLS(0,"KEY DOWNLOAD");
		dspLS(1,"User:");
		ret = enterPhr(2,user,9);
		if(ret == kbdANN || ret == 0)
			goto lblBatal;
		CHECK(ret>=0,lblKO);
		if (strlen(user) <8) {
			usrInfo(infNotAccepted);
			memset(user,0,sizeof(user));
		}
    }
    MAPPUTSTR(traTLEUserId,user,lblKO);

    dspClear();
    memset(pwd,0,sizeof(pwd));
    dspLS(0,"KEY DOWNLOAD");
    dspLS(1,"Password:");
    ret = enterPwd2(2,pwd,sizeof(pwd));
    if(ret == kbdANN || ret == 0)
		goto lblBatal;
	CHECK(ret>=0,lblEnd);
    MAPPUTSTR(traTLEPwd,pwd,lblKO);  
    return 1;
lblBatal:
    return 0;
lblKO:  
    return -1;
lblEnd:
	return 1;
}

int tleKeyDownload()
{
    int ret;
    char traName[dspW + 1];
//    char user[10];
//    char pwd[10];

    trcS("tleKeyDownload Beg\n");

    mapMove(rqsBeg, (word) (trtTLEKeyDownload - 1));
    MAPGET(rqsDisp, traName, lblKO);
    trcFS("traName: %s\n", traName);

    MAPPUTBYTE(regLocType,'T',lblKO);
    MAPPUTWORD(traMnuItm, mnuTLEKeyDownload, lblKO);
    MAPPUTSTR(traTransType, traName, lblKO);
    MAPPUTSTR(traTypeName, traName, lblKO);
    MAPPUTBYTE(traTxnType, trtTLEKeyDownload, lblKO);

    ret = onlSession();

    CHECK(ret > 0, lblKO);

    goto lblEnd;

lblKO:
    ret = -1;
lblEnd:
	//usrInfo(infRemoveCard); //@@AS0.0.10
    //removeCard(); //@@AS0.0.10
    return ret;
    trcS("tleKeyDownload End\n");
}

int decryptRSA(byte *modulus,byte *pubExponent,byte *privExponent,byte *dataIn,byte *dataOut,int *outLen) {
	byte rsaBuf[10000];
	int ret;
	RSA_t key;
	byte dest[128];
#ifdef _DEBUG_TLE
	byte buf[50];
#endif
#ifdef DEBUG_TLE_GAGAL
    prtS("decryptRSA1");
#endif
	memset(rsaBuf,0,sizeof(rsaBuf));
	RSA_init((void *)rsaBuf,sizeof(rsaBuf));

	key.Option = RSA_EXPONENT_10001;
	key.KeySize = 1024;

	key.Modulus = modulus;
	key.PublicExponent = pubExponent;
	key.PrivateExponent = privExponent;
	key.Prime1 = NULL;
	key.Prime2 = NULL;
	key.InvPrime1ModPrime2 = NULL;

#ifdef _DEBUG_TLE
	sprintf(buf,"+++RSA_init+++");prtS(buf);
	printRSA_t(&key);
	sprintf(buf,"---RSA_init---");prtS(buf);
#endif
#ifdef DEBUG_TLE_GAGAL
    prtS("decryptRSA2");
#endif
	ret=RSA_Decrypt(dest,dataIn,&key);
	if (ret==0) {
#ifdef _DEBUG_TLE
		sprintf(buf,"RSA_Decrypt=%d",ret);prtS(buf);
		hex_dump_char("DataIn",dataIn,128);
		hex_dump_char("DataOut",dest,128);
#endif
#ifdef DEBUG_TLE_GAGAL
		{
			char buf[50];
			sprintf(buf,"RSA_Decrypt=%d",ret);prtS(buf);
			hex_dump_char("DataIn",dataIn,128);
			hex_dump_char("DataOut",dest,128);
		}
#endif
		*outLen = RSA_padding_check_PKCS1_type_2 (dataOut, *outLen,dest,sizeof(dest));
		if (*outLen>0) {
#ifdef _DEBUG_TLE
			sprintf(buf,"plainLen=%d",*outLen);prtS(buf);
			hex_dump_char("plainData",dataOut,*outLen);
#endif
			return (0);
		}
		else {
#ifdef _DEBUG_TLE
			sprintf(buf,"paddingCheck=%d",*outLen);prtS(buf);
#endif
#ifdef DEBUG_TLE_GAGAL
		{
			char buf[50];
            sprintf(buf,"paddingCheck=%d",*outLen);prtS(buf);
		}
#endif
			return (-1);
		}
	}
	else return (-1);
}


int tleDecryptAndStoreUMK()
{
    //byte rsaBuf[10000];
	//byte *rsaBuf; //@@AS0.0.20
    RSA_t key;
    byte modulus[128];
    byte PublicExponent[3];
    byte PrivateExponent[128];
    byte src[150];
    byte buf[150];
    char tid[9],currTid[9];
    int len;
    int ret;
    int i;

#ifdef DEBUG_TLE_GAGAL
    prtS("tleDecrypt1");
#endif

    memset(tid,0,sizeof(tid));
    MAPGET(acqTID,tid,lblKO);
    mapGet(traEncryptedUMK,src,128);
    ret = mapGet(appPublicKeyExponent,PublicExponent,3);

    if(ret == 1)
        key.Option = RSA_EXPONENT_3;
    else if(ret == 2)
        key.Option = RSA_EXPONENT_805;
    else if(ret == 3)
        key.Option = RSA_EXPONENT_10001;
    else goto lblKO;

    ret = mapGet(appPublicKeyModulus,modulus,128);
    if(ret < 0)
        goto lblKO;
    ret = mapGet(appPrivateKeyExponent,PrivateExponent,128);
    if(ret < 0)
        goto lblKO;
#ifdef DEBUG_TLE_GAGAL
    prtS("tleDecrypt2");
#endif
    len=128;
    ret=decryptRSA(modulus,PublicExponent,PrivateExponent,src,buf,&len);

    if (ret != 0)
    	goto lblKO;
#ifdef DEBUG_TLE_GAGAL
    prtS("tleDecrypt3");
#endif
    //free (rsaBuf); //@@AS0.0.20
#ifdef _DEBUG_TLE
    ShowData(buf,len,0,0,8);
#endif //_DEBUG_TLE

    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        memset(currTid,0,sizeof(currTid));
        MAPGET(acqTID,currTid,lblKO);
        if(memcmp(tid,currTid,8) == 0)
        {
            ret=mapPut(acqUMK,buf,len);
            CHK;
        }
    }
#ifdef _DEBUG_TLE
{
    for(i=0;i<dimAcq;i++)
    {
        mapMove(acqBeg,i);
        memset(currTid,0,sizeof(currTid));
        MAPGET(acqTID,currTid,lblKO);
        memset(buf,0,sizeof(buf));
        MAPGET(acqUMK,buf,lblKO);
        
        prtS(currTid);
        ShowData(buf,32,0,1,15);
    }
}
#endif
    goto lblEnd;
  lblKO:
    ret = -1;
    return (ret);
  lblEnd:
//    ret=mapPut(appUMK,buf,len);
    return 1;
    trcS("tleKeyDownload End\n");
}


void keyDownload()
{
    tleKeyDownload();
    tleDecryptAndStoreUMK();
}

byte isEncryptedField(byte bit){
	byte i=0;
	byte ret=0;
	while(1)
	{
		if(encryptedField[i] == 0)
			break;

		if(bit != encryptedField[i++])
			continue;
		else {
			ret=1;
			break;
		}

	}
	return(ret);
}



#if 0
int TleTlvProcess(tBuffer * val, byte bit,byte *tlv_buf)
{
    int i;
    int len;
    int ret;
    word wres;
    byte tag[2];

    trcS("TLEEncryptProcess Beg\n");

    if(bit == STORE_TLE_FIELD)
    {
        return 0;
    }

    while(1)
    {
        if(encryptedField[i] == 0)
            break;

        if(bit != encryptedField[i++])
            continue;

        tag[0] = 0xDF;
        tag[1] = bit;


        //wres = xsec_add_tlv (tag, 2, bufPtr(val),bufLen(val), byte *tlv_buf)
        ret = tlvAdd(tag,TleBuf,bufPtr(val),bufLen(val));
        if(ret == 0)
            goto lblKO;

#ifdef DEBUG_TLV
{
    hex_dump("FIELD:",&bit,1);
    hex_dump("TLV :",(void*)bufPtr(&TleBuf),(int)bufLen(&TleBuf));
}
#endif
        //field yang akan di encrypt di set nilainya menjadi 0
        bufReset(val);
        bufApp(val,0,1);
        break;
    }
    ret = 1;
    goto lblEnd;
lblKO:
    ret = -1;
lblEnd:
    trcS("TLEEncryptProcess End\n");
    return ret;
}
#endif //#if 0
//@@AS0.0.10
word xsec_add_tlv (byte *tag, byte tag_len, byte *data, word data_len, byte *tlv_buf)
{
    byte *ptr, num_byte_of_lenght;
    ptr = tlv_buf;
    // tag
    memmove (ptr, tag, tag_len);
    ptr += tag_len;
    // length
    if (data_len < 128)
    {
        num_byte_of_lenght = 1;
        *ptr = (byte) data_len;
        ptr++;
    }
    else if (data_len < 256)
    {
        num_byte_of_lenght = 2;
        *ptr = 0x81;
        ptr++;
        *ptr = (byte) data_len;
        ptr++;
    }
    else // if (data_len < (256 * 256))
    {
        num_byte_of_lenght = 3;
        *ptr = 0x82;
        ptr++;
        *ptr = (byte) (data_len / 256);
        ptr++;
        *ptr = (byte) (data_len % 256);
        ptr++;
    }
    // data
    memmove (ptr, data, data_len);

    return (tag_len + num_byte_of_lenght + data_len);
}
//@@AS0.0.10
void xsec_generate_derivation_data (byte *data_out)
{
    int         i;

    //memmove (data_out, "12345678", 8);  // test
    for (i = 0; i < DERIVATION_DATA_LEN; i++)
        *(data_out + i) = (byte) (rand () % 256);
}
//@@AS0.0.10
void xsec_generate_USK (byte *UMK, byte *derivation_data, byte *USK)
{
    byte           i, r_data [16];

	AESLoadKey ((unsigned char *) UMK, 256);
	// USK-L
	memmove (r_data, derivation_data, DERIVATION_DATA_LEN);
	memmove (r_data + 8, derivation_data, DERIVATION_DATA_LEN);
	for (i=0; i<DERIVATION_DATA_LEN; i++)
		r_data [8 + i] ^= 0xFF;
	AESEncrypt ((unsigned char *) r_data, 16, (unsigned char *) USK);
	// USK-R
	memmove (r_data, derivation_data, DERIVATION_DATA_LEN);
	memmove (r_data + 8, derivation_data, DERIVATION_DATA_LEN);
	for (i=0; i<DERIVATION_DATA_LEN; i++)
		r_data [i] ^= 0xFF;
	AESEncrypt ((unsigned char *) r_data, 16, (unsigned char *) USK + 16);
}

void xsec_encrypt_derivation_data (byte *UMK, byte *derivation_data, byte *encrypted_derivation_data)
{
    byte           i, data [16];

	AESLoadKey ((unsigned char *) UMK, 256);
	memset (data, 0x00, sizeof (data));
	memmove (data, derivation_data, DERIVATION_DATA_LEN);
	for (i=0; i<DERIVATION_DATA_LEN; i++)
		data [i] ^= 0x0F;
	AESEncrypt ((unsigned char *) data, 16, (unsigned char *) encrypted_derivation_data);
}

/*-----------------------------------------------------------------------
 * Name:           EncryptCBC
 * Description:    Do TDES CBC Encryption
 *
 * Parameters:     byte* pKey , pointer to the key buffer
 							  byte* pData, pointer of data to be encrypted
 							  word nLen, length of data
 							  byte* pResult, Encrypted data
 * Return Value:   None.
 * Notes:
 *-----------------------------------------------------------------------*/
void EncryptCBC(byte* pKey, byte* pData, word nLen, byte* pResult)
{
	byte IV[8],i;
	word ofs=0;

	memset(IV,0x00,sizeof(IV));
	while (nLen != ofs) {
		for(i=0;i<8;i++)
			pData[i+ofs] ^= IV[i];
		stdEDE(pResult+ofs, pData+ofs, pKey);
		memcpy(IV,pResult+ofs,8);
		ofs += 8;
	}
}

/*-----------------------------------------------------------------------
 * Name:           DecryptCBC
 * Description:    Do TDES CBC Encryption
 *
 * Parameters:     byte* pKey , pointer to the key buffer
 							  byte* pData, pointer of data to be encrypted
 							  word nLen, length of data
 							  byte* pResult, Encrypted data
 * Return Value:   None.
 * Notes:
 *-----------------------------------------------------------------------*/
void DecryptCBC(byte* pKey, byte* pData, word nLen, byte* pResult)
{
	byte IV[8],i;
	word ofs=0;

	memset(IV,0x00,sizeof(IV));
	while (nLen != ofs) {
		stdDED(pResult+ofs, pData+ofs, pKey);
		for(i=0;i<8;i++)
			pResult[i+ofs] ^= IV[i];
		memcpy(IV,pData+ofs,8);
		ofs += 8;
	}
}



byte xsec_encrypt_data (byte *tle_key, byte *data, word *data_len)
{
    byte           *buf;
    byte           /*len,*/padding_len = 0;
    word len;

    len = *data_len;

#ifdef _DEBUG_TLE
	hex_dump_char("--ENC-DATA-IN--",data,len);
#endif


    buf = (byte*) umalloc(len*2 + 20);

    if (! buf)
        return 0;
    memmove (buf, data, len);
    //
    *(buf + len) = 0x80;   // ended with 0x80
    len++;
    *(buf + len) = 0x00;   // padd with 0x00
    len++;
    // padding

	if (len % 8)
	{
		// padded with 0x00
		padding_len = 8 - (len % 8);
		memset (buf + len, 0x00, padding_len);
		len += padding_len;
	}

	//convert to hex
	memcpy(data,buf,len);
	bin2hex(buf, data,len);

    // encrypt
	EncryptCBC(tle_key,buf,len*2,data);
    *data_len = len*2;

#ifdef _DEBUG_TLE
	hex_dump_char("--ENC-RESULT--",data,len);
#endif


    //free (buf);
    ufree(buf);
    return 1;
}


//@@SIMAS-SW_PIN - start
void Make_Pin_Block(
byte *pin,	/*  input - PIN number (bcd, left justified padded with f's) */
byte PinLen,
byte *pan,	/*  input - Primary Account Number (including  check digit), BCD right justified padded with zero */
word pan_len,/* input - length of the PAN in digits		*/
byte *Result
)
{
    int count;
    byte pan_block[8], local_pan[10], pan_byte = 0;
    word num_pan_bytes;
    word num_pin_bytes;
	byte PinBlock[8];

	num_pin_bytes = ((PinLen % 2) == 0) ? (PinLen / 2) : ((PinLen / 2) + 1);

	/* Stick in the control and pin length fields    */
	PinBlock[0] = 0x00;
	PinBlock[0] |= (byte)PinLen;

	memcpy(&PinBlock[1], pin, num_pin_bytes);
	memset(&PinBlock[num_pin_bytes + 1], 0xFF, 8 - num_pin_bytes);
	/* if odd number of digits need to fill lower nibble of last pin byte */
	if ((PinLen % 2) != 0)
	{
		PinBlock[num_pin_bytes] |= 0x0f;
	}

	/* create PAN block */
	num_pan_bytes = ((pan_len % 2) == 0) ? (pan_len /2) : ((pan_len / 2) + 1);
	memcpy(local_pan, pan, num_pan_bytes);

	/* want 12 right-most digits of PAN and leave off check digit */
	memset(pan_block, 0x00, 8);
	if ((pan_len % 2) == 0)
	{
		/* need to shift right by half byte (we never use last PAN digit) */
		for (count = 0; count < (signed int)num_pan_bytes; count++)
		{
			pan_byte = (local_pan[num_pan_bytes - count] >> 4) & 0x0f;
			pan_byte |= (local_pan[num_pan_bytes - count - 1] << 4) & 0xf0;
			local_pan[num_pan_bytes - count] = pan_byte;
		}
		local_pan[0] = (local_pan[0] >> 4) & 0x0f;
	}
	else
	{
		num_pan_bytes -= 1;
	}

	for (count = 0; count < 6; count++)
	{
		pan_block[7 - count] = local_pan[(num_pan_bytes - 1) - count];
	}

    /* XOR with the PAN block    */
    for (count = 0; count < 8; count++ )
        PinBlock[count] ^= pan_block[count];
    memcpy(Result,PinBlock,sizeof(PinBlock));
}


//-----------------------------------------------------------------------------
//  GetEncryptedPinBlock		For Using security processor to generate Enc. Pin Block
//
//  Parameters:     PIN(I)			pointer to the clear PIN(in BCD), padded with F
//			  		PinLen(I)		Length of the clear PIN
//			  		mk(I)			pointer Terminal Master Key
//			  		aPinKey(I)		pointer to the PIN Working Key
//		 	  		KeyLength(I)	the length of the key, the possible value is 8 or 16
//		 	  		PAN(I)			pointer to card number(PAN in BCD),  padded with F
//			  		lenPAN(I)		length of the PAN
//			  		ePinBlock(O)	pointer to the encrypted Pin block(8 bytes)
//
//  Returns:        0			successful Pin Block generated 8 bytes returned in [ePinBlock]
//						non-zero for Pin Block generation failed
//-----------------------------------------------------------------------------
int GetEncryptedPinBlock(byte *Pin,byte PinLen,byte *mk,byte *aPinKey,byte KeyLength,byte *PAN,byte lenPAN, byte *ePinBlock )
{
	byte	MasterKey[16];
	byte	WorkData[16];
	byte	WorkKey[16];
	byte	PinKey[16];
    byte	PinBlock[8];
    int     nRc;


	//Check the key length
	if ((KeyLength!=8) && (KeyLength!=16))
		return (-1);

	if (KeyLength==8) {
		memcpy(MasterKey,mk,8);
		memcpy(&MasterKey[8],mk,8);
		memcpy(WorkKey,aPinKey,8);
		memcpy(&WorkKey[8],aPinKey,8);
		stdDED(PinKey,WorkKey,MasterKey);
	}
	else {
		memcpy(MasterKey,mk,16);
		memcpy(WorkKey,aPinKey,16);
		stdDED(PinKey,WorkKey,MasterKey);
		stdDED(&PinKey[8],&WorkKey[8],MasterKey);
	}


    // remove the trailing 0xff
    while (PAN[lenPAN - 1]== 0xff)
    {
        lenPAN--;
    }
    if((PAN[lenPAN - 1] & 0x0f) == 0x0f)
    {
        lenPAN = (lenPAN*2) -1;
    }else
    {
        lenPAN*= 2;
    }
    Make_Pin_Block(Pin,PinLen,PAN,lenPAN,PinBlock);

    stdEDE(WorkData,PinBlock,PinKey);

     memcpy(ePinBlock,WorkData,8);
	return (0);
}
//@@SIMAS-SW_PIN - end


#if 0
int tlvAdd(byte* tag, tBuffer *tlvBuf, byte *data, int dataLen)
{
/*
    Hanya bisa maks 3 byte length, tag 2 byte
*/
    byte buf[8];
    int i, len, tmp, ret;
    byte *ptr

    //tag
    bufApp(tlvBuf,tag,2);

    //len
    if(dataLen <= 0x7F)
    {
        buf[0] = (byte)dataLen;
        len = 1;
    }
    else if(dataLen <= 0xFF)
    {
        buf[0] = 0x81;
        buf[1] = (byte)dataLen;
        len = 2;
    }
    else if(dataLen <= 0xFFFF)
    {
        buf[0] = 0x82;
        buf[1] = dataLen/256;
        buf[2] = dataLen%256;
        len = 3;
    }
    else
        return -1;


    if(bufApp(tlvBuf,buf,1) < 0)
        return -1;

    //data
    if(bufApp(tlvBuf,data,dataLen) < 0)
        return -1;

    return 2+len+dataLen;;
}
#endif //#if 0

#endif //LINE_ENCRYPTION
