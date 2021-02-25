#ifndef NASM_SERVICE_H
#define NASM_SERVICE_H

typedef unsigned char uint8_t;
typedef char * va_list;

#define MAX_PRINTF_STR_SIZE 10

#define __va_rounded_size(TYPE) ((sizeof(TYPE)+sizeof(int)-1)&~(sizeof(int)-1))
#define va_start(AP,LASTARG) (AP=((char *)&(LASTARG)+__va_rounded_size(LASTARG)))
#define va_end(AP)
#define va_arg(AP,TYPE) \
    (AP+=__va_rounded_size(TYPE) , *((TYPE *)(AP-__va_rounded_size(TYPE))))

void printf(const char *fmt, ...);
int int2str(char *dst, int val);
void start_server(void);
void halt(void);
int printk(const char *str);
void swrite(char c);
uint8_t inb(int port);
void outb(int port, uint8_t data);

#endif
