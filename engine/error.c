/* ----------------------------------------------------------------------- *
 *
 *   Copyright 1996-2019 The NASM Authors - All Rights Reserved
 *   See the file AUTHORS included with the NASM distribution for
 *   the specific copyright holders.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----------------------------------------------------------------------- */

/*
 * error.c - error message handling routines for the assembler
 */

#include "compiler.h"


#include "nasmlib.h"
#include "error.h"
#include "srcfile.h"

/* Common function body */
#define nasm_do_error(_sev,_flags)				\
	va_list ap;						\
        va_start(ap, fmt);					\
	if ((_sev) >= ERR_CRITICAL)				\
		nasm_verror_critical((_sev)|(_flags), fmt, ap);	\
	else							\
		nasm_verror((_sev)|(_flags), fmt, ap);		\
	va_end(ap);						\
	if ((_sev) >= ERR_FATAL)                                \
		abort();


void nasm_error(errflags severity, const char *fmt, ...)
{
	nasm_do_error(severity & ERR_MASK, severity & ~ERR_MASK);
}

#define nasm_err_helpers(_type, _name, _sev)				\
_type nasm_ ## _name ## f (errflags flags, const char *fmt, ...)	\
{									\
	nasm_do_error(_sev, flags);					\
}									\
_type nasm_ ## _name (const char *fmt, ...)				\
{									\
	nasm_do_error(_sev, 0);						\
}

nasm_err_helpers(void,       listmsg,  ERR_LISTMSG)
nasm_err_helpers(void,       debug,    ERR_DEBUG)
nasm_err_helpers(void,       info,     ERR_INFO)
nasm_err_helpers(void,       nonfatal, ERR_NONFATAL)
nasm_err_helpers(fatal_func, fatal,    ERR_FATAL)
nasm_err_helpers(fatal_func, critical, ERR_CRITICAL)
nasm_err_helpers(fatal_func, panic,    ERR_PANIC)

/*
 * Strongly discourage warnings without level by require flags on warnings.
 * This means nasm_warn() is the equivalent of the -f variants of the
 * other ones.
 */
void nasm_warn(errflags flags, const char *fmt, ...)
{
	nasm_do_error(ERR_WARNING, flags);
}

fatal_func nasm_panic_from_macro(const char *file, int line)
{
	nasm_panic("internal error at %s:%d\n", file, line);
}

fatal_func nasm_assert_failed(const char *file, int line, const char *msg)
{
	nasm_panic("assertion %s failed at %s:%d", msg, file, line);
}


/*
 * Warning stack management. Note that there is an implicit "push"
 * after the command line has been parsed, but this particular push
 * cannot be popped.
 */
struct warning_stack {
	struct warning_stack *next;
	uint8_t state[sizeof warning_state];
};
static struct warning_stack *warning_stack, *warning_state_init;

/* Push the warning status onto the warning stack */
void push_warnings(void)
{
	struct warning_stack *ws;

	ws = nasm_malloc(sizeof *ws);
	memcpy(ws->state, warning_state, sizeof warning_state);
	ws->next = warning_stack;
	warning_stack = ws;
}

/* Pop the warning status off the warning stack */
void pop_warnings(void)
{
	struct warning_stack *ws = warning_stack;

	memcpy(warning_state, ws->state, sizeof warning_state);
	if (!ws->next) {
		/*!
		 *!warn-stack-empty [on] warning stack empty
		 *!  a [WARNING POP] directive was executed when
		 *!  the warning stack is empty. This is treated
		 *!  as a [WARNING *all] directive.
		 */
		nasm_warn(WARN_WARN_STACK_EMPTY, "warning stack empty");
	} else {
		warning_stack = ws->next;
		nasm_free(ws);
	}
}

/* Call after the command line is parsed, but before the first pass */
void init_warnings(void)
{
	push_warnings();
	warning_state_init = warning_stack;
}


/* Call after each pass */
void reset_warnings(void)
{
	struct warning_stack *ws = warning_stack;

	/* Unwind the warning stack. We do NOT delete the last entry! */
	while (ws->next) {
		struct warning_stack *wst = ws;
		ws = ws->next;
		nasm_free(wst);
	}
	warning_stack = ws;
	memcpy(warning_state, ws->state, sizeof warning_state);
}

/*
 * This is called when processing a -w or -W option, or a warning directive.
 * Returns on if if the action was successful.
 *
 * Special pseudo-warnings:
 *
 *!other [on] any warning not specifially mentioned above
 *!  specifies any warning not included in any specific warning class.
 *
 *!all [all] all possible warnings
 *!  is an group alias for \e{all} warning classes.  Thus, \c{-w+all}
 *!  enables all available warnings, and \c{-w-all} disables warnings
 *!  entirely (since NASM 2.13).
 */
bool set_warning_status(const char *value)
{
	enum warn_action { WID_OFF, WID_ON, WID_RESET };
	enum warn_action action;
        const struct warning_alias *wa;
        size_t vlen;
	bool ok = false;
	uint8_t mask;

	value = nasm_skip_spaces(value);

	switch (*value) {
	case '-':
		action = WID_OFF;
		value++;
		break;
	case '+':
		action = WID_ON;
		value++;
		break;
	case '*':
		action = WID_RESET;
		value++;
		break;
	case 'N':
	case 'n':
		if (!nasm_strnicmp(value, "no-", 3)) {
			action = WID_OFF;
			value += 3;
			break;
		} else if (!nasm_stricmp(value, "none")) {
			action = WID_OFF;
			value = NULL;
			break;
		}
		/* else fall through */
	default:
		action = WID_ON;
		break;
	}

	mask = WARN_ST_ENABLED;

	if (value && !nasm_strnicmp(value, "error", 5)) {
		switch (value[5]) {
		case '=':
			mask = WARN_ST_ERROR;
			value += 6;
			break;
		case '\0':
			mask = WARN_ST_ERROR;
			value = NULL;
			break;
		default:
			/* Just an accidental prefix? */
			break;
		}
	}

	if (value && !nasm_stricmp(value, "all"))
		value = NULL;

        vlen = value ? strlen(value) : 0;

	/*
         * This is inefficient, but it shouldn't matter.
         * Note: warning_alias[0] is "all".
         */
	for (wa = warning_alias+1;
             wa < &warning_alias[NUM_WARNING_ALIAS]; wa++) {
            enum warn_index i = wa->warning;

            if (value) {
                char sep;

                if (nasm_strnicmp(value, wa->name, vlen))
                    continue;   /* Not a prefix */

                sep = wa->name[vlen];
                if (sep != '\0' && sep != '-')
                    continue;   /* Not a valid prefix */
            }

            ok = true; /* At least one action taken */
            switch (action) {
            case WID_OFF:
                warning_state[i] &= ~mask;
                break;
            case WID_ON:
                warning_state[i] |= mask;
                break;
            case WID_RESET:
                warning_state[i] &= ~mask;
                warning_state[i] |= warning_state_init->state[i] & mask;
                break;
            }
        }

        if (!ok && value) {
            /*!
             *!unknown-warning [off] unknown warning in -W/-w or warning directive
             *!  warns about a \c{-w} or \c{-W} option or a \c{[WARNING]} directive
             *!  that contains an unknown warning name or is otherwise not possible to process.
             */
            nasm_warn(WARN_UNKNOWN_WARNING, "unknown warning name: %s", value);
	}

	return ok;
}

static bool skip_this_pass(errflags severity);

struct error_format {
    const char *beforeline;     /* Before line number, if present */
    const char *afterline;      /* After line number, if present */
    const char *beforemsg;      /* Before actual message */
};

static const struct error_format errfmt_gnu  = { ":", "",  ": "  };
static const struct error_format *errfmt = &errfmt_gnu;
static struct nasm_errhold *errhold_stack;

#ifndef ABORT_ON_PANIC
# define ABORT_ON_PANIC 0
#endif
static bool abort_on_panic = ABORT_ON_PANIC;

/**
 * get warning index; 0 if this is non-suppressible.
 */
static size_t warn_index(errflags severity)
{
    size_t index;

    if ((severity & ERR_MASK) >= ERR_FATAL)
        return 0;               /* Fatal errors are never suppressible */

    /* Warnings MUST HAVE a warning category specifier! */
    nasm_assert((severity & (ERR_MASK|WARN_MASK)) != ERR_WARNING);

    index = WARN_IDX(severity);
    nasm_assert(index < WARN_IDX_ALL);

    return index;
}

static bool skip_this_pass(errflags severity)
{
    errflags type = severity & ERR_MASK;

    /*
     * See if it's a pass-specific error or warning which should be skipped.
     * We can never skip fatal errors as by definition they cannot be
     * resumed from.
     */
    if (type >= ERR_FATAL)
        return false;

    /*
     * ERR_LISTMSG messages are always skipped; the list file
     * receives them anyway as this function is not consulted
     * for sending to the list file.
     */
    if (type == ERR_LISTMSG)
        return true;
    return false;
}

/**
 * check for suppressed message (usually warnings or notes)
 *
 * @param severity the severity of the warning or error
 * @return true if we should abort error/warning printing
 */
static bool is_suppressed(errflags severity)
{
    /* Fatal errors must never be suppressed */
    if ((severity & ERR_MASK) >= ERR_FATAL)
        return false;

    if (!(warning_state[warn_index(severity)] & WARN_ST_ENABLED))
        return true;

    return false;
}

/**
 * Return the true error type (the ERR_MASK part) of the given
 * severity, accounting for warnings that may need to be promoted to
 * error.
 *
 * @param severity the severity of the warning or error
 * @return true if we should error out
 */
static errflags true_error_type(errflags severity)
{
    const uint8_t warn_is_err = WARN_ST_ENABLED|WARN_ST_ERROR;
    int type;

    type = severity & ERR_MASK;

    /* Promote warning to error? */
    if (type == ERR_WARNING) {
        uint8_t state = warning_state[warn_index(severity)];
        if ((state & warn_is_err) == warn_is_err)
            type = ERR_NONFATAL;
    }

    return type;
}

/*
 * The various error type prefixes
 */
static const char * const error_pfx_table[ERR_MASK+1] = {
    ";;; ", "debug: ", "info: ", "warning: ",
        "error: ", "fatal: ", "critical: ", "panic: "
};
static const char no_file_name[] = "nasm"; /* What to print if no file name */

/*
 * For fatal/critical/panic errors, kill this process.
 */
static fatal_func die_hard(errflags true_type)
{
    fflush(NULL);

    if (true_type == ERR_PANIC && abort_on_panic)
        abort();

    /* Terminate immediately */
    exit(true_type - ERR_FATAL + 1);
}

/*
 * Returns the struct src_location appropriate for use, after some
 * potential filename mangling.
 */
static struct src_location error_where(errflags severity)
{
    struct src_location where;

    if (severity & ERR_NOFILE) {
        where.filename = NULL;
        where.lineno = 0;
    } else {
        where = src_where_error();

        if (!where.filename) {
            where.filename = NULL;
            where.lineno = 0;
        }
    }

    return where;
}

/*
 * error reporting for critical and panic errors: minimize
 * the amount of system dependencies for getting a message out,
 * and in particular try to avoid memory allocations.
 */
fatal_func nasm_verror_critical(errflags severity, const char *fmt, va_list args)
{
    struct src_location where;
    errflags true_type = severity & ERR_MASK;
    static bool been_here = false;

    if (unlikely(been_here))
        abort();                /* Recursive error... just die */

    been_here = true;

    where = error_where(severity);
    if (!where.filename)
        where.filename = no_file_name;

    fputs(error_pfx_table[severity], error_file);
    fputs(where.filename, error_file);
    if (where.lineno) {
        fprintf(error_file, "%s%"PRId32"%s",
                errfmt->beforeline, where.lineno, errfmt->afterline);
    }
    fputs(errfmt->beforemsg, error_file);
    vfprintf(error_file, fmt, args);
    fputc('\n', error_file);

    die_hard(true_type);
}

/**
 * Stack of tentative error hold lists.
 */
struct nasm_errtext {
    struct nasm_errtext *next;
    char *msg;                  /* Owned by this structure */
    struct src_location where;  /* Owned by the srcfile system */
    errflags severity;
    errflags true_type;
};
struct nasm_errhold {
    struct nasm_errhold *up;
    struct nasm_errtext *head, **tail;
};

static void nasm_free_error(struct nasm_errtext *et)
{
    nasm_free(et->msg);
    nasm_free(et);
}

static void nasm_issue_error(struct nasm_errtext *et);

struct nasm_errhold *nasm_error_hold_push(void)
{
    struct nasm_errhold *eh;

    nasm_new(eh);
    eh->up = errhold_stack;
    eh->tail = &eh->head;
    errhold_stack = eh;

    return eh;
}

void nasm_error_hold_pop(struct nasm_errhold *eh, bool issue)
{
    struct nasm_errtext *et, *etmp;

    /* Allow calling with a null argument saying no hold in the first place */
    if (!eh)
        return;

    /* This *must* be the current top of the errhold stack */
    nasm_assert(eh == errhold_stack);

    if (eh->head) {
        if (issue) {
            if (eh->up) {
                /* Commit the current hold list to the previous level */
                *eh->up->tail = eh->head;
                eh->up->tail = eh->tail;
            } else {
                /* Issue errors */
                list_for_each_safe(et, etmp, eh->head)
                    nasm_issue_error(et);
            }
        } else {
            /* Free the list, drop errors */
            list_for_each_safe(et, etmp, eh->head)
                nasm_free_error(et);
        }
    }

    errhold_stack = eh->up;
    nasm_free(eh);
}

/**
 * common error reporting
 * This is the common back end of the error reporting schemes currently
 * implemented.  It prints the nature of the warning and then the
 * specific error message to error_file and may or may not return.  It
 * doesn't return if the error severity is a "panic" or "debug" type.
 *
 * @param severity the severity of the warning or error
 * @param fmt the printf style format string
 */
void nasm_verror(errflags severity, const char *fmt, va_list args)
{
    struct nasm_errtext *et;
    errflags true_type = true_error_type(severity);

    if (true_type >= ERR_CRITICAL)
        nasm_verror_critical(severity, fmt, args);

    if (is_suppressed(severity))
        return;

    nasm_new(et);
    et->severity = severity;
    et->true_type = true_type;
    et->msg = nasm_vasprintf(fmt, args);
    et->where = error_where(severity);

    if (errhold_stack && true_type <= ERR_NONFATAL) {
        /* It is a tentative error */
        *errhold_stack->tail = et;
        errhold_stack->tail = &et->next;
    } else {
        nasm_issue_error(et);
    }
}

/*
 * Actually print, list and take action on an error
 */
static void nasm_issue_error(struct nasm_errtext *et)
{
    const char *pfx;
    char warnsuf[64];           /* Warning suffix */
    char linestr[64];           /* Formatted line number if applicable */
    const errflags severity  = et->severity;
    const errflags true_type = et->true_type;
    const struct src_location where = et->where;

    if (severity & ERR_NO_SEVERITY)
        pfx = "";
    else
        pfx = error_pfx_table[true_type];

    *warnsuf = 0;
    if ((severity & (ERR_MASK|ERR_HERE|ERR_PP_LISTMACRO)) == ERR_WARNING) {
        /*
         * It's a warning without ERR_HERE defined, and we are not already
         * unwinding the macros that led us here.
         */
        snprintf(warnsuf, sizeof warnsuf, " [-w+%s%s]",
                 (true_type >= ERR_NONFATAL) ? "error=" : "",
                 warning_name[warn_index(severity)]);
    }

    *linestr = 0;
    if (where.lineno) {
        snprintf(linestr, sizeof linestr, "%s%"PRId32"%s",
                 errfmt->beforeline, where.lineno, errfmt->afterline);
    }

    if (!skip_this_pass(severity)) {
        const char *file = where.filename ? where.filename : no_file_name;
        const char *here = "";

        if (severity & ERR_HERE) {
            here = where.filename ? " here" : " in an unknown location";
        }

        fprintf(error_file, "%s%s%s%s%s%s%s\n",
                file, linestr, errfmt->beforemsg,
                pfx, et->msg, here, warnsuf);
    }

    /* Are we recursing from error_list_macros? */
    if (severity & ERR_PP_LISTMACRO)
        goto done;

    if (skip_this_pass(severity))
        goto done;

    if (true_type >= ERR_FATAL)
        die_hard(true_type);

done:
    nasm_free_error(et);
}
