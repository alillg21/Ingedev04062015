/** \file tcab0187.c
 * Unitary test case for LZW compression
 * http://marknelson.us/attachments/lzw-data-compression/lzw.c
 * Minimal changes are made to implement it under UNICAPT32
 * To run this test case a file tcab0187.src should be in DFS of the application
 * It will be compressed into tcab0187.lzw
 * Then it will be decompressed to tcab0187.dst
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0187.c $
 *
 * $Id: tcab0187.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <unicapt.h>

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BITS 12                 /* Setting the number of bits to 12, 13 */
#define HASHING_SHIFT (BITS-8)  /* or 14 affects several constants.    */
#define MAX_VALUE (1 << BITS) - 1   /* Note that MS-DOS machines need to   */
#define MAX_CODE MAX_VALUE - 1  /* compile their code in large model if */
                                  /* 14 bits are selected.               */
#if BITS == 14
#define TABLE_SIZE 18041        /* The string table size needs to be a */
#endif                          /* prime number that is somewhat larger */
#if BITS == 13                  /* than 2**BITS.                       */
#define TABLE_SIZE 9029
#endif
#if BITS <= 12
#define TABLE_SIZE 5021
#endif

void *malloc();

static int *code_value;         /* This is the code value array        */
static unsigned int *prefix_code;   /* This array holds the prefix codes   */
static unsigned char *append_character; /* This array holds the appended chars */
static unsigned char decode_stack[4000];    /* This array holds the decoded string */

/*
 * Forward declarations
 */
/*
void compress(FILE *input,FILE *output);
void expand(FILE *input,FILE *output);
int find_match(int hash_prefix,unsigned int hash_character);
void output_code(FILE *output,unsigned int code);
unsigned int input_code(FILE *input);
unsigned char *decode_string(unsigned char *buffer,unsigned int code);
*/
/*
 * Original declarations are replaced by the declarations below to adapt it to UNICAPT32
 */
static void compress(uint32 input, uint32 output);
static void expand(uint32 input, uint32 output);
static int find_match(int hash_prefix, unsigned int hash_character);
static void output_code(uint32 output, unsigned int code);
static unsigned int input_code(uint32 input);
static unsigned char *decode_string(unsigned char *buffer, unsigned int code);

/********************************************************************
**
** This program gets a file name from the command line.  It compresses the
** file, placing its output in a file named test.lzw.  It then expands
** test.lzw into test.out.  Test.out should then be an exact duplicate of
** the input file.
**
*************************************************************************/

/*
** This is the compression routine.  The code should be a fairly close
** match to the algorithm accompanying the article.
**
*/

static void compress(uint32 input, uint32 output) {
    unsigned int next_code;
    unsigned int character;
    unsigned int string_code;
    unsigned int index;
    int i;
    uint8 b;
    uint16 n;

    next_code = 256;            /* Next code is the next available string code */
    for (i = 0; i < TABLE_SIZE; i++)    /* Clear out the string table before starting */
        code_value[i] = (-1);

    i = 0;
    printf("Compressing...\n");
    //string_code=getc(input);    /* Get the first code                         */  
    n = 1;
    psyFileRead(input, &b, &n);
    string_code = b;
/*
** This is the main loop where it all happens.  This loop runs util all of
** the input has been exhausted.  Note that it stops adding codes to the
** table after all of the possible codes have been defined.
*/
    //while ((character=getc(input)) != (unsigned)EOF)
    while(psyFileRead(input, &b, &n) >= 0) {
        character = b;
        if(++i == 1000) {       /* Print a * every 1000    *//* input characters.  This */
            i = 0;              /* is just a pacifier.     */
            printf("*");
        }
        index = find_match(string_code, character); /* See if the string is in */
        if(code_value[index] != -1) /* the table.  If it is,   */
            string_code = code_value[index];    /* get the code value.  If */
        else {                  /* the string is not in the */
            /* table, try to add it.   */
            if(next_code <= MAX_CODE) {
                code_value[index] = next_code++;
                prefix_code[index] = string_code;
                append_character[index] = character;
            }
            output_code(output, string_code);   /* When a string is found  */
            string_code = character;    /* that is not in the table */
        }                       /* I output the last string */
    }                           /* after adding the new one */
/*
** End of the main loop.
*/
    output_code(output, string_code);   /* Output the last code               */
    output_code(output, MAX_VALUE); /* Output the end of buffer code      */
    output_code(output, 0);     /* This code flushes the output buffer */
}

/*
** This is the hashing routine.  It tries to find a match for the prefix+char
** string in the string table.  If it finds it, the index is returned.  If
** the string is not found, the first available index in the string table is
** returned instead.
*/

static int find_match(int hash_prefix, unsigned int hash_character) {
    int index;
    int offset;

    index = (hash_character << HASHING_SHIFT) ^ hash_prefix;
    if(index == 0)
        offset = 1;
    else
        offset = TABLE_SIZE - index;
    while(1) {
        if(code_value[index] == -1)
            return (index);
        if(prefix_code[index] == hash_prefix &&
           append_character[index] == hash_character)
            return (index);
        index -= offset;
        if(index < 0)
            index += TABLE_SIZE;
    }
}

/*
**  This is the expansion routine.  It takes an LZW format file, and expands
**  it to an output file.  The code here should be a fairly close match to
**  the algorithm in the accompanying article.
*/

static void expand(uint32 input, uint32 output) {
    unsigned int next_code;
    unsigned int new_code;
    unsigned int old_code;
    int character;
    int counter;
    unsigned char *string;
    char b;

    next_code = 256;            /* This is the next available code to define */
    counter = 0;                /* Counter is used as a pacifier.            */
    printf("Expanding...\n");

    old_code = input_code(input);   /* Read in the first code, initialize the */
    character = old_code;       /* character variable, and send the first */
    //putc(old_code,output);       /* code to the output file                */
    b = old_code;
    psyFileWrite(output, &b, 1);
/*
**  This is the main expansion loop.  It reads in characters from the LZW file
**  until it sees the special code used to inidicate the end of the data.
*/
    while((new_code = input_code(input)) != (MAX_VALUE)) {
        if(++counter == 1000) { /* This section of code prints out     *//* an asterisk every 1000 characters   */
            counter = 0;        /* It is just a pacifier.              */
            printf("*");
        }
/*
** This code checks for the special STRING+CHARACTER+STRING+CHARACTER+STRING
** case which generates an undefined code.  It handles it by decoding
** the last code, and adding a single character to the end of the decode string.
*/
        if(new_code >= next_code) {
            *decode_stack = character;
            string = decode_string(decode_stack + 1, old_code);
        }
/*
** Otherwise we do a straight decode of the new code.
*/
        else
            string = decode_string(decode_stack, new_code);
/*
** Now we output the decoded string in reverse order.
*/
        character = *string;
        while(string >= decode_stack) {
            //putc(*string--,output);
            b = (*string--);
            psyFileWrite(output, &b, 1);
        }
/*
** Finally, if possible, add a new code to the string table.
*/
        if(next_code <= MAX_CODE) {
            prefix_code[next_code] = old_code;
            append_character[next_code] = character;
            next_code++;
        }
        old_code = new_code;
    }
}

/*
** This routine simply decodes a string from the string table, storing
** it in a buffer.  The buffer can then be output in reverse order by
** the expansion program.
*/

static unsigned char *decode_string(unsigned char *buffer, unsigned int code) {
    int i;

    i = 0;
    while(code > 255) {
        *buffer++ = append_character[code];
        code = prefix_code[code];
        if(i++ >= MAX_CODE) {
            printf("Fatal error during code expansion.\n");
            exit(-3);
        }
    }
    *buffer = code;
    return (buffer);
}

/*
** The following two routines are used to output variable length
** codes.  They are written strictly for clarity, and are not
** particularyl efficient.
*/

static unsigned int input_code(uint32 input) {
    unsigned int return_value;
    static int input_bit_count = 0;
    static unsigned long input_bit_buffer = 0L;
    uint8 b;
    uint16 n;

    while(input_bit_count <= 24) {
        n = 1;
        psyFileRead(input, &b, &n);
        input_bit_buffer |=
//        (unsigned long) getc(input) << (24-input_bit_count);
            (unsigned long) b << (24 - input_bit_count);
        input_bit_count += 8;
    }
    return_value = input_bit_buffer >> (32 - BITS);
    input_bit_buffer <<= BITS;
    input_bit_count -= BITS;
    return (return_value);
}

static void output_code(uint32 output, unsigned int code) {
    static int output_bit_count = 0;
    static unsigned long output_bit_buffer = 0L;
    uint8 b;

    output_bit_buffer |= (unsigned long) code << (32 - BITS - output_bit_count);
    output_bit_count += BITS;
    while(output_bit_count >= 8) {
        b = output_bit_buffer >> 24;
        psyFileWrite(output, &b, 1);
        output_bit_buffer <<= 8;
        output_bit_count -= 8;
    }
}
#endif

void tcab0187(void) {
#ifdef WIN32
    uint32 src = 0;
    uint32 dst = 0;

    code_value = (int *) malloc(TABLE_SIZE * sizeof(int));
    prefix_code = (unsigned int *) malloc(TABLE_SIZE * sizeof(unsigned int));
    append_character =
        (unsigned char *) malloc(TABLE_SIZE * sizeof(unsigned char));

    src = psyFileOpen("tcab0187.src", "r");
    dst = psyFileOpen("tcab0187.lzw", "w");

    compress(src, dst);
    psyFileClose(dst);
    psyFileClose(src);
    free(code_value);

    src = psyFileOpen("tcab0187.lzw", "r");
    dst = psyFileOpen("tcab0187.dst", "w");

    expand(src, dst);

    if(src)
        psyFileClose(src);
    if(dst)
        psyFileClose(dst);

    free(prefix_code);
    free(append_character);
#endif
}
