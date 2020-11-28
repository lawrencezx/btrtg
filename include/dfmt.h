#ifndef TEST_DFMT_H
#define TEST_DFMT_H

struct dfmt {
    void (*init)(const char* fname);

    void (*cleanup)(void);

    void (*print)(const char *format, ...);
};

extern const struct dfmt *dfmt;

#endif
