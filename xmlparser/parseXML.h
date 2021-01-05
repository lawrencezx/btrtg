#ifndef NASM_PARSEXML_H
#define NASM_PARSEXML_H

void init_tks(void);
void init_tmplts(void);
void parse_tks_file(const char *fname);
void parse_tmplts_file(const char *fname);

#endif
