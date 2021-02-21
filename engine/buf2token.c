#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "error.h"
#include "buf2token.h"
#include "insns.h"

static char token_buf[128];

/*
 * Standard scanner routine used by parser.c and some output
 * formats. It keeps a succession of temporary-storage strings in
 * stdscan_tempstorage, which can be cleared using stdscan_reset.
 */
static char *token_bufptr = NULL;
static char **token_tempstorage = NULL;
static int token_tempsize = 0, token_templen = 0;
#define TOKEN_TEMP_DELTA 256

void set_token_bufptr(char *ptr)
{
    token_bufptr = ptr;
}

char *get_token_bufptr(void)
{
    return token_bufptr;
}

char *get_token_buf(void)
{
    return (char *)token_buf;
}

char *get_token_cbufptr(void)
{
    memset(token_buf, ' ', 2);
    memset(token_buf + 2, '\0', sizeof(token_buf) - 2);
    token_bufptr = (char *)token_buf + 2;
    return token_bufptr;
}

static void token_pop(void)
{
    nasm_free(token_tempstorage[--token_templen]);
}

void token_reset(void)
{
    while (token_templen > 0)
        token_pop();
}

/*
 * Unimportant cleanup is done to avoid confusing people who are trying
 * to debug real memory leaks
 */
void token_cleanup(void)
{
    token_reset();
    nasm_free(token_tempstorage);
}

static char *buf_copy(const char *p, int len)
{
    char *text;

    text = nasm_malloc(len + 1);
    memcpy(text, p, len);
    text[len] = '\0';

    if (token_templen >= token_tempsize) {
        token_tempsize += TOKEN_TEMP_DELTA;
        token_tempstorage = nasm_realloc(token_tempstorage,
                                           token_tempsize *
                                           sizeof(char *));
    }
    token_tempstorage[token_templen++] = text;

    return text;
}

int get_token(struct tokenval *tv)
{
#ifdef DEBUG_MODE
    fprintf(stderr, "[token_bufptr: 0x%llx]: %s\n", (long long unsigned int)token_bufptr, token_bufptr);
#endif
    const char *r;

    token_bufptr = nasm_skip_spaces(token_bufptr);
    if (!*token_bufptr)
        return tv->t_type = TOKEN_EOS;

    /* we have a token; either an id, a number or a char */
    if (nasm_isidstart(*token_bufptr)) {
        int token_type;

        r = token_bufptr++;
        while (nasm_isidchar(*token_bufptr))
            token_bufptr++;

        tv->t_charptr = buf_copy(r, token_bufptr - r < IDLEN_MAX ?
                                     token_bufptr - r : IDLEN_MAX - 1);

        token_type = nasm_token_hash(tv->t_charptr, tv);
        if (likely(!(tv->t_flag & TFLAG_BRC))) {
            return token_type;
        } else {
            return tv->t_type = TOKEN_ID;
        }
    } else if (nasm_isnumstart(*token_bufptr)) {  /* now we've got a number */
        bool rn_error;
        char c;

        r = token_bufptr;

        for (;;) {
            c = *token_bufptr++;

            if (nasm_isnumchar(c))
                ; /* just advance */
            else
                break;
        }
        token_bufptr--;       /* Point to first character beyond number */

//        if (is_float) {
//        } else {
            r = buf_copy(r, token_bufptr - r);
            tv->t_integer = readnum(r, &rn_error);
            if (rn_error) {
                /* some malformation occurred */
                return tv->t_type = TOKEN_ERRNUM;
            }
            tv->t_charptr = NULL;
            return tv->t_type = TOKEN_NUM;
//        }
    } else if (*token_bufptr == ';' || 
            *token_bufptr == '\n') {
        /* a comment has happened - stay */
        return tv->t_type = TOKEN_EOS;
    } else
        return tv->t_type = (uint8_t)(*token_bufptr++);
}
