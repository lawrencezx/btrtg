#include "compiler.h"


#include "dfmt.h"
#include "error.h"

static FILE* debugfp;

static void debug_cleanup(void)
{
    if (!debugfp)
        return;
    fclose(debugfp);
    debugfp = NULL;
}

static void debug_init(const char* fname)
{
    if (debugfp)
        debug_cleanup();
    if (!fname || fname[0] == '\0') {
        debugfp = NULL;
        return;
    }
    debugfp = fopen(fname, "w");
    if (!debugfp) {
        nasm_fatal("Unable to open file %s", fname);
        return;
    }
}

static void debug_out(const char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef DEBUG_MODE
    vfprintf(stderr, format, args);
#else
    vfprintf(debugfp, format, args);
#endif
    va_end(args);
}

static const struct dfmt debug_fmt = {
    .init = debug_init,
    .cleanup = debug_cleanup,
    .print = debug_out
};

const struct dfmt *dfmt = &debug_fmt;
