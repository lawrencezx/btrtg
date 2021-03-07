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

int64_t hex2declong(const char *hex)
{
    int64_t temp, num = 0;

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

void str2bcd(const char *hex, uint32_t bcd[3]){
    /* skip head */
    for(int i = 0; i < 3; i++){
        bcd[i] = 0;
    }
    while (*hex == ' ')
        hex++;
    int sign =  1;
    if(*hex == '-'){
        sign = -1;
        hex ++;
    }  
    char *bcd_ptr = (char *)bcd;
    char *end = (char *)hex + strlen(hex) - 1;
    while(*end == ' ' || *end =='\0')
        end --;
    char temp = 0;
    for(int i = 0; end - i >= hex; i++){
        char *c = end - i;
        if(i%2 == 0){
            temp |= *c - '0';
        }else{
            temp |= (*c - '0')<<4;
            *bcd_ptr = temp;
            bcd_ptr ++;
            temp = 0;
        }
    }
    *bcd_ptr = temp;
    ((char *)bcd)[9] = sign == 1 ? 0x00 : 0x80;
}