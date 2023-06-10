#include <rotary/string.h>

/* ========================================================================= */

void * memset(void * dest, uint8 val, uint32 len) {
    uint8 * temp = (uint8*) dest;
    for (; len != 0; len--) *temp++ = val;
    return dest;
}

/* ========================================================================= */

void * memcpy(void * dest, void * src, uint32 size) {
    char * d = dest;
    char * s = src;
    while (size--)
        *d++ = *s++;
    return dest;
}

/* ========================================================================= */

void int_to_str(uint32 num, char * dest)
{
    int i = 0;
    do {
        dest[i++] = num % 10 + '0';
        num /= 10;
    } while (num != 0);
    dest[i] = '\0';

    int j = 0, k = i-1;
    while (j < k) {
        char temp = dest[j];
        dest[j] = dest[k];
        dest[k] = temp;
        j++; k--;
    }
}

/*
void int_to_str(uint64 value, char *buf, uint8 base) {
    static const char *digits = "0123456789ABCDEF";
    char *p = buf;
    uint64 tmp = value;

    do {
        *p++ = digits[tmp % base];
        tmp /= base;
    } while (tmp != 0);

    *p-- = '\0';

    // Reverse the string
    char *start = buf;
    char tmp_char;

    while (start < p) {
        tmp_char = *start;
        *start = *p;
        *p = tmp_char;
        start++;
        p--;
    }
}
*/

/* ========================================================================= */

void int_to_hex_str(uint32 num, char * dest) {
  int i;
  char hex_digits[] = "0123456789abcdef";
  
  for (i = 0; i < 8; i++) {
    dest[7-i] = hex_digits[num & 0xf];
    num >>= 4;
  }
  dest[8] = '\0';
}

/* ========================================================================= */

void fill_buffer(char* dest_buf, uint32* dest_index, char c, int count) {
    for (int i = 0; i < count; i++) {
        dest_buf[(*dest_index)++] = c;
    }
}

/* ========================================================================= */

void sprintf(char *dest_buf, char *format_str, ...) {
    __builtin_va_list list;
    __builtin_va_start(list, format_str);

    uint32 index = 0;
    uint32 processed = 0;
    uint32 dest_index = 0;

    // Iterate until the end of the string
    while (format_str[index] != '\0') {
        if (format_str[index] == '%') {
            char num[32];
            memset(&num, 0, sizeof(num));

            // Copy characters before the format specifier
            while (processed < index) {
                dest_buf[dest_index++] = format_str[processed++];
            }

            index++; // move past '%'

            // Padding
            int padding = 0;
            while(format_str[index] >= '0' && format_str[index] <= '9') {
                padding = padding * 10 + (format_str[index] - '0');
                index++;
            }

            int length = 0;

            switch (format_str[index]) {
                case 's':
                    {
                        char *str_arg = __builtin_va_arg(list, char*);
                        // Padding
                        length = strlen(str_arg);
                        if (padding > length) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - length);
                        }
                        // Concatenate the string argument
                        while (*str_arg != '\0') {
                            dest_buf[dest_index++] = *str_arg++;
                        }
                    }
                    break;
                case 'd':
                    int_to_str(__builtin_va_arg(list, int), (char*)&num);
                    // Padding
                    length = strlen(num);
                    if (padding > length) {
                        fill_buffer(dest_buf, &dest_index, '0', padding - length);
                    }
                    // Concatenate the decimal number
                    char *num_ptr = num;
                    while (*num_ptr != '\0') {
                        dest_buf[dest_index++] = *num_ptr++;
                    }
                    break;
                case 'x':
                    int_to_hex_str(__builtin_va_arg(list, int), num);
                    // Padding
                    length = strlen(num);
                    if (padding > length) {
                        fill_buffer(dest_buf, &dest_index, '0', padding - length);
                    }
                    // Concatenate the hexadecimal number
                    char *hex_ptr = num;
                    while (*hex_ptr != '\0') {
                        dest_buf[dest_index++] = *hex_ptr++;
                    }
                    break;
            }

            processed = index + 1; // index is already moved past the format character
        }
        index++;
    }

    // Copy characters after the last format specifier
    while (processed < index) {
        dest_buf[dest_index++] = format_str[processed++];
    }
    
    dest_buf[dest_index] = '\0'; // Null-terminating the destination buffer
    __builtin_va_end(list);
}

// ------------------------------------------------------------------------- //

int strcmp(char * str1, char * str2) {
    if(strlen(str1) != strlen(str2))
        return 0;

    for(uint32 i = 0; i < strlen(str1); i++ ) {
        if(str1[i] != str2[i]) {
            return 0;
        }
    }

    return 1;
}

// ------------------------------------------------------------------------- //

void strcpy(char * dest_buf, char * src_buf) {
    uint32 i = 0;
    while(*(dest_buf+i) != 0x00) {
        *(dest_buf+i) = src_buf[i];
        i++;
    }
    *(dest_buf + ++i) = 0x00;
}

// ------------------------------------------------------------------------- //

void strncpy(char * dest_buf, char * src_buf, uint32 n) {
    for(uint32 i = 0; i < n; i++) {
        *(dest_buf+i) = src_buf[i];
    }
}

// ------------------------------------------------------------------------- //

char * strcat(char * str1, char * str2) {
    uint32 str1_len = strlen(str1);
    uint32 str2_len = strlen(str2);
    uint32 i;
    for(i = 0; i < str2_len; i++) {
        str1[str1_len+i] = str2[i];
    }
    str1[str1_len+i] = '\0';
    return str1;
}

// ------------------------------------------------------------------------- //

uint32 strlen(char * string) {
    for(uint32 i = 0; i < MAX_STRING_LEN; i++) {
        if(string[i] == '\0')
            return i;
    }
    return -1;
}

// ------------------------------------------------------------------------- //

// https://www.geeksforgeeks.org/implement-itoa/

void reverse(char * str, int length) {
	int start = 0;
	int end = length - 1;
	while(start < end) {
        uchar temp = *(str+start);
		*(str+start) = *(str+end);
        *(str+end) = temp;
		start++;
		end--;
	}
}

void itoa(uint32 num, char* dec_string) {
    int i;
    uint32 divisor = 1000000000;
    int leading_zeroes = 1;
    for (i = 0; i < 10; i++) {
        int digit = num / divisor;
        if (digit != 0 || !leading_zeroes) {
            dec_string[i] = digit + '0';
            leading_zeroes = 0;
        }
        num %= divisor;
        divisor /= 10;
    }
    dec_string[10] = '\0';
}

// ------------------------------------------------------------------------- //

int atoi(char * str) {
    int res = 0;
    for(int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
    return res;
}

// ------------------------------------------------------------------------- //

