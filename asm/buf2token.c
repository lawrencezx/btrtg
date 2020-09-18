#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "error.h"
#include "buf2token.h"
#include "insns.h"

static char *buf_copy(const char *p, int len)
{
    char *text;

    text = nasm_malloc(len + 1);
    memcpy(text, p, len);
    text[len] = '\0';

    return text;
}

int buf2token(char* buffer, struct tokenval *tv)
{
    const char *r;

    /* we have a token; either an id, a number or a char */
    if (nasm_isidstart(*buffer)) {
        int token_type;

        r = buffer++;
        while (nasm_isidchar(*buffer))
            buffer++;

        tv->t_charptr = buf_copy(r, buffer - r < IDLEN_MAX ?
                                     buffer - r : IDLEN_MAX - 1);

        token_type = nasm_token_hash(tv->t_charptr, tv);
        if (likely(!(tv->t_flag & TFLAG_BRC))) {
            return token_type;
        } else {
            return tv->t_type = TOKEN_ID;
        }
    } else if (nasm_isnumstart(*buffer)) {  /* now we've got a number */
        bool rn_error;

        tv->t_integer = readnum(buffer, &rn_error);
        if (rn_error) {
            return tv->t_type = TOKEN_ERRNUM;
        }
        tv->t_charptr = NULL;
        return tv->t_type = TOKEN_NUM;
    } else
        return tv->t_type = (uint8_t)(*buffer);
}
