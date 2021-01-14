#include "compiler.h"

#include <ctype.h>
#include "nasmlib.h"

#define HEX_RADIX 16

static int ascii_xdigit_value(char ch)
{
    if(isdigit(ch))
        return ch - '0';
    else if (isupper(ch) && ch - 'A' < HEX_RADIX)
        return ch - 'A' + 10;
    else if (islower(ch) && ch - 'a' < HEX_RADIX)
        return ch - 'a' + 10;
    return -1;
}  

int hex2dec(const char *hex)
{
    int temp, num = 0;

    /* skip head */
    while (*hex == ' ')
        hex++;
    if (strlen(hex) >= 2 && hex[0] == '0' && (hex[1] == 'X' || hex[1] == 'x'))
        hex += 2;

    while ((temp = ascii_xdigit_value(*hex)) != -1) {
        num = (num * HEX_RADIX) + temp;
        hex++;
    }
    return num;
}

void hex2double(const char *hex, int* num){
    /* skip head */
    while (*hex == ' ')
        hex++;
    if (strlen(hex) >= 2 && hex[0] == '0' && (hex[1] == 'X' || hex[1] == 'x'))
        hex += 2;
    
    char part1[11] = "0x";
    char part2[11] = "0x";
    memcpy(part1+2, hex, 8);
    memcpy(part2+2, hex+8, 8);
    num[1] = hex2dec(part1);
    num[0] = hex2dec(part2);
}

void hex2ldouble(const char *hex, int *num){
    /* skip head */
    while (*hex == ' ')
        hex++;
    if (strlen(hex) >= 2 && hex[0] == '0' && (hex[1] == 'X' || hex[1] == 'x'))
        hex += 2;
    char part1[11] = "0x";
    char part2[11] = "0x";
    char part3[11] = "0x";
    memcpy(part1+2, hex, 8);
    memcpy(part2+2, hex+8, 8);
    memcpy(part3+2, hex+16, 8);
    num[2] = hex2dec(part1);
    num[1] = hex2dec(part2);
    num[0] = hex2dec(part3);
}