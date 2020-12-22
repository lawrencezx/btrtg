#ifndef NASM_SECTION_H
#define NASM_SECTION_H

enum secType {
    TEXT_SEC,
    DATA_SEC,
    BSS_SEC
};

struct section {
    enum secType type;
    int datanum;
    int *dataoffs;
    int *datasizes;
};

void init_text_sec(struct section *text_sec);
void init_data_sec(struct section *data_sec);

#endif
