#ifndef NASM_BUF2TOKEN_H
#define NASM_BUF2TOKEN_H

int nasm_token_hash(const char *token, struct tokenval *tv);
int buf2token(char* buffer, struct tokenval *tv);

#endif
