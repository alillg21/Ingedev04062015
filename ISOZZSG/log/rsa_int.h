#ifndef RSAINT_H
#define RSAINT_H
/*******************************************************************************
Internal RSA header file
                                  NOTES
Size is counted in number of words based on the word size of the intended platform.
The number will be represented as an array of WORDS.
The array will start with element zero as the LSB.
The storage of a word can be little or big endian depending on intended platform.
The functions below will only access the array as an array of WORDS.
*******************************************************************************/
#include "rsa.h"

/*******************************************************************************
                                  TYPES
*******************************************************************************/
/*define used instead of typdef so that definitions can be built upon other definitions*/
#define int8_t  char
#define int16_t short
#define int32_t long
#define int64_t long long

#define uint8_t  unsigned int8_t 
#define uint16_t unsigned int16_t
#define uint32_t unsigned int32_t
#define uint64_t unsigned int64_t


#if 0
  #define WORD_t  int8_t
  #define DWORD_t int16_t
#else
  #define WORD_t  int16_t
  #define DWORD_t int32_t
//  #define WORD_t  int32_t
//  #define DWORD_t int64_t
#endif



#define uWORD_t  unsigned WORD_t 
#define uDWORD_t unsigned DWORD_t
#define size_t  uint16_t 

#define HighWord(a) ((a)>>(8*sizeof(uWORD_t)))


extern void math_memInit(void *memory,uint16_t size);           /*initialize math memory*/
extern void* math_memAlloc(uint16_t amount);   /*allocate some memory from the math heap*/
extern void math_memFree(void * src);          /*free any math memory declared after *dest*/


/*******************************************************************************
                                  MATH PROTOTYPES
*******************************************************************************/
extern void hex_dump(char *title,void *src,int len);
extern void math_hex_dump(char* title, uWORD_t* src,size_t size);

extern void math_init(void);

extern void math_clr(uWORD_t *dest,size_t size);
extern void math_cpy(uWORD_t *dest,uWORD_t *src,size_t size);
extern WORD_t math_cmp(uWORD_t *src1,uWORD_t *src2,size_t size);

extern uWORD_t math_shl(uWORD_t* dest,size_t size);
extern uWORD_t math_shlx(uWORD_t* dest,size_t shift,size_t size);
extern uWORD_t math_shr(uWORD_t* dest,size_t size);
extern uWORD_t math_shrx(uWORD_t* dest,size_t shift,size_t size);
extern size_t math_msb(uWORD_t *src, size_t size);
extern size_t math_lsb(uWORD_t *src, size_t size);

extern uWORD_t math_add(uWORD_t* dest,uWORD_t *src,size_t size);
extern WORD_t math_sub(uWORD_t* dest,uWORD_t *src,size_t size);

extern uWORD_t math_mulsub(uWORD_t *dest,uWORD_t *src1,uWORD_t src2,size_t size);

extern void math_div(uWORD_t *quotient,uWORD_t *remainder,uWORD_t *denominator,uWORD_t size);
extern void math_mul(uWORD_t *dest,uWORD_t *src,size_t size);

extern void math_modexp(uWORD_t *dest, uWORD_t *exp, uWORD_t *mod, size_t size);
extern void math_modmul(uWORD_t *dest,uWORD_t *src,uWORD_t *mod,size_t size);


extern uWORD_t math_moninv(uWORD_t src);
extern void math_monexp(uWORD_t *dest, uWORD_t *exp, uWORD_t *mod, size_t size);
void math_monmul(uWORD_t *dest,uWORD_t *src,uWORD_t *mod,uWORD_t inv,size_t size);
extern void math_monred(uWORD_t* dest, uWORD_t* mod, uWORD_t m_inv , size_t size );

void math_random_init(void);
extern uint8_t math_random_bit(void);
extern void math_random(uWORD_t *dest,size_t size);

extern uint16_t* math_generatePrimeTable(uint16_t size);
extern uWORD_t *math_generatePrime(size_t wordsize, size_t bitsize);
extern void math_make32(uWORD_t *dest,uint32_t src,size_t size);

extern void math_add16(uWORD_t *dest, uint16_t src,size_t size);
extern uint16_t math_mod16(uWORD_t *src1, uint16_t src2,size_t size);
extern uint32_t math_mod24(uWORD_t *src1, uint32_t src2,size_t size);
extern void math_initCompositeTable(uWORD_t *prime,size_t size, uint16_t* compositeTable, uint16_t* primeTable, size_t primeTableSize);
extern uint8_t math_testComposite(uint16_t offset,uint16_t* compositeTable,uint16_t* primeTable,size_t primeTableSize);
extern uint8_t math_millerRabinTest(uWORD_t *prime, size_t size);

extern void math_modinv(uWORD_t *dest,uWORD_t *src,uWORD_t *mod,size_t size);
extern void math_sqrt(uWORD_t* dest,size_t size);

extern uWORD_t* math_byteToWord(uint8_t *src,size_t word_size,size_t byte_size);
extern void math_wordToByte(uint8_t *dest,uWORD_t *src,size_t wordsize);

#endif
