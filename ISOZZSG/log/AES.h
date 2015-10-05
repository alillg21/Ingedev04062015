#ifndef AES_H
#define AES_H

void AESKeyExpansion (void);
void AESCipher (void);
void AESInvCipher (void);
int AESLoadKey (unsigned char *key, int key_length);
void AESEncrypt (unsigned char *data_in, unsigned int data_in_len, unsigned char *data_out);
void AESDecrypt (unsigned char *data_in, unsigned int data_in_len, unsigned char *data_out);
//void AESTestEncrypt (void);
//void AESTest (void);

extern unsigned char AESDataIn[16], AESDataOut[16];
extern unsigned char AESKey[32];

#endif
