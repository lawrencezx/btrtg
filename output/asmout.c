#include "compiler.h"

#include "ofmt.h"
#include "section.h"

static FILE* asmfp;

static void asm_cleanup(void)
{
    if (!asmfp)
        return;
    fclose(asmfp);
    asmfp = NULL;
}

static void asm_init(const char* fname)
{
    if (asmfp)
        asm_cleanup();
    if (!fname || fname[0] == '\0') {
        asmfp = NULL;
        return;
    }
    asmfp = fopen(fname, "w");
    if (!asmfp) {
        printf("Unable to open file %s", fname);
        return;
    }
}

static void asm_out_sec(struct section *sec)
{
    switch (sec->type) {
        case TEXT_SEC:
            fprintf(asmfp, "SECTION .TEXT\n");
            break;
        case DATA_SEC:
            fprintf(asmfp, "SECTION .DATA write\n");
            break;
        case BSS_SEC:
            fprintf(asmfp, "SECTION .BSS write\n");
            break;
        default:
            printf("Unsupported section type\n");
            break;
    }
}

static void asm_out(struct output_data* data)
{
    if (data == NULL)
        return;
    switch (data->type) {
        case OUTPUT_RAWDATA:
            fprintf(asmfp, "%s\n", (const char *)data->buf);
            break;
        case OUTPUT_INSN:
            fprintf(asmfp, "  %s\n", (const char *)data->buf);
            break;
        case OUTPUT_SECTION:
        {
            struct section *sec = (struct section *)data->buf;
            asm_out_sec(sec);
            break;
        }
        default:
            break;
    }
}

static const struct ofmt asm_fmt = {
    .init = asm_init,
    .cleanup = asm_cleanup,
    .output = asm_out
};

const struct ofmt *ofmt = &asm_fmt;
