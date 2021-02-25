#include "service.h"

int main(void)
{
    start_server();
}

void start_server(void)
{
    int a = 100;
    printf("printf: a = %d\n", a);
    printk("[PASS]\t c server started - OK.\n");
    halt();
}

void halt(void)
{
    printk("[PASS]\t System halting - OK.\n");
    __asm__ __volatile__("hlt;");
}

int str_cpy(char *dst, const char *src)
{
    int len = 0;
    while ((*dst++ = *src++) != '\0')
        len++;
    return len;
}

int int2str(char *dst, int val)
{
    int cnt, i = 0, tmp = val;

    while(tmp /= 10)
          i++;
    cnt = i + 1;

    do {
         dst[i--] = (val % 10) + '0';
    } while(val /= 10);

    return cnt;
}

/**
 * printf - format and print data without stdlib
 * @fmt: format
 * @...: contents
 * supported format:
 *  %d, %c, %s
 */
void printf(const char *fmt, ...)
{
    char str_buf[MAX_PRINTF_STR_SIZE];
    char *c = str_buf;
    va_list ap;
    int int_v;
    char char_v;
    char *str_v;
    va_start(ap,fmt);
    for (const char *p = fmt; *p; p++) {
        if(*p != '%') {
          *c++ = *p;
          continue;
        }
        switch (*++p) {
        case 'd':
            int_v = va_arg(ap, int);
            c += int2str(c, int_v);
            break;
        case 'c':
            char_v = va_arg(ap, char);
            *c++ = char_v;
            break;
        case 's':
            str_v = va_arg(ap, char *);
            c += str_cpy(c, str_v);
            break;
        default:
            *c++ = *p;
            break;
        }
    }
    printk(str_buf);
}

int printk(const char *str)
{
    char *ptr = (char *)str;
    while (*ptr)
        swrite(*(ptr++));
    return ptr - str;
}

void swrite(char c)
{
    while ((inb(0x3FD) & 0x20) != 0x20)
        ;
    outb(0x3F8, c);
}

uint8_t inb(int port)
{
    int ret;

    __asm__ __volatile__ ("xorl %eax, %eax");
    __asm__ __volatile__ ("inb %%dx, %%al" \
            : "=a" (ret) \
            : "d"(port));

    return ret;
}

void outb(int port, uint8_t data)
{
    __asm__ __volatile__ ("outb %%al, %%dx" :: "a"(data), "d"(port));
}
