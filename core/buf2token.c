#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "error.h"
#include "buf2token.h"
#include "insns.h"

static char token_buf[128];
static char *token_bufptr;

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
    memset(token_buf + 2, '\n', sizeof(token_buf) - 2);
    token_bufptr = (char *)token_buf + 2;
    return token_bufptr;
}

static char *buf_copy(const char *p, int len)
{
    char *text;

    text = nasm_malloc(len + 1);
    memcpy(text, p, len);
    text[len] = '\0';

    return text;
}

int get_token(struct tokenval *tv)
{
#ifdef DEBUG_MODE
    fprintf(stderr, "[token_bufptr]: %s\n", token_bufptr);
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

        nasm_free(tv->t_charptr);
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
            nasm_free((void *)r);
            if (rn_error) {
                /* some malformation occurred */
                return tv->t_type = TOKEN_ERRNUM;
            }
            tv->t_charptr = NULL;
            return tv->t_type = TOKEN_NUM;
//        }
    } else if (*token_bufptr == ';') {
        /* a comment has happened - stay */
        return tv->t_type = TOKEN_EOS;
    } else
        return tv->t_type = (uint8_t)(*token_bufptr++);
}
