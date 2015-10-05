#ifndef __LINE_ENCRYPTION__H__
#define __LINE_ENCRYPTION__H__
#include "log.h"
#include <string.h>
#include "rsa.h" //@agmr - rsa


#define TLE_USER_ID_LEN             8
#define TLE_PWD_LEN                 8
#define TERMINAL_SERIAL_NUM_LEN     8
#define TLE_HEADER_VERSION_LEN      1
#define TLE_KEY_ALGORITHM_LEN       1
#define TLE_AMK_VERSION_LEN         1
#define TLE_REF_NUMBER_LEN          8
#define TLE_KCV                     3
#define TLE_MODULUS_LENGTH_LEN      2
#define TLE_MODULUS_LEN             256
#define TLE_EXPONENT_LENGTH_LEN     2
#define TLE_EXPONENT_LEN            3
#define TLE_ENCRYPTED_UMK_LEN       128
#define TLE_UMK_LEN                 32
#define TLE_USK_LEN                 32


typedef struct 
{
    byte userId[TLE_USER_ID_LEN];
    byte pwd[TLE_PWD_LEN];
    byte terminalSerialNum[TERMINAL_SERIAL_NUM_LEN];
    byte headerVersion[TLE_HEADER_VERSION_LEN];
    byte keyAlgorithm[TLE_KEY_ALGORITHM_LEN];
    byte amkVersion[TLE_AMK_VERSION_LEN];
    byte refNumber[TLE_REF_NUMBER_LEN];
    byte kcv[TLE_KCV];
    byte modulusLength[TLE_MODULUS_LENGTH_LEN];
    byte modulus[TLE_MODULUS_LEN];
    byte exponentLength[TLE_EXPONENT_LENGTH_LEN];
    byte exponent[TLE_EXPONENT_LEN];
    byte encryptedUMK[TLE_ENCRYPTED_UMK_LEN];
    byte umk[TLE_UMK_LEN];
    byte usk[TLE_USK_LEN];
    
}TLE_DATA;



#define STORE_TLE_FIELD 58
#define DERIVATION_DATA_LEN         8 //@@AS0.0.10
#define AES_KEY_LEN                 32 //@@AS0.0.10
extern unsigned char encryptedField[64];
extern unsigned char xsec_derivation_data [DERIVATION_DATA_LEN]; //@@AS0.0.10
extern unsigned char xsec_UMK [AES_KEY_LEN+1]; //@@AS0.0.10
extern unsigned char xsec_USK [AES_KEY_LEN+1]; //@@AS0.0.10
extern unsigned char xsec_TLE [lenKEY+1]; //@@AS-SIMAS

extern byte xsec_encrypt_data (byte *USK, byte *data, word *data_len);
extern void xsec_encrypt_derivation_data (byte *UMK, byte *derivation_data, byte *encrypted_derivation_data);
extern void xsec_generate_derivation_data (byte *data_out);
extern void xsec_generate_USK (byte *UMK, byte *derivation_data, byte *USK);

int shaData(char *data);

void testSHA();
void testRSA();

int checkKey();
//int checkAcqTleKey();
void printRSA_t(RSA_t* t);
void tleProcess(byte needCreateKey);
void keyDownload();

void tleMenu();
int tleGetKey();
int tleKeyDownload();
int tleInputUserPass();
int tleDecryptAndStoreUMK();
int TleTlvProcess(tBuffer * val, byte bit,byte *tlv_buf);
byte isEncryptedField(byte bit);
word xsec_add_tlv (byte *tag, byte tag_len, byte *data, word data_len, byte *tlv_buf);

int GetEncryptedPinBlock(byte *Pin,byte PinLen,byte *mk,byte *aPinKey,byte KeyLength,byte *PAN,byte lenPAN, byte *ePinBlock );//@@SIMAS-SW_PIN


#endif
