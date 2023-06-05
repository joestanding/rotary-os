/* ========================================================================= */
/* Kernel String and Memory Functions                                        */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/debug.h>

#define MAX_STRING_LEN 512

/* ========================================================================= */

void   int_to_str(uint32 num, char * dest);
void   int_to_hex_str(uint32 num, char * dest);

void * memset(void * dest, uint8 val, uint32 len);
void * memcpy(void * dest, void * src, uint32 size);

int    strcmp(char * str1, char * str2);
void   strncpy(char * dest_buf, char * src_buf, uint32 n);
char * strcat(char * str1, char * str2);
void   sprintf(char * dest_buf, char * format_str, ...);
uint32 strlen(char * string);
void   itoa(uint32 num, char * dec_string);
int    atoi(char * str);

/* ========================================================================= */
