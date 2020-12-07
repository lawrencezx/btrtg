#ifndef NASM_BUF2TOKEN_H
#define NASM_BUF2TOKEN_H

void set_token_bufptr(char *ptr);
char *get_token_bufptr(void);
char *get_token_cbufptr(void);
int nasm_token_hash(const char *token, struct tokenval *tv);
int get_token(struct tokenval *tv);

#endif
