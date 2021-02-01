/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 1996-2017 The NASM Authors - All Rights Reserved
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
 * assemble.h - header file for stuff private to the assembler
 */

#ifndef NASM_ASSEMBLE_H
#define NASM_ASSEMBLE_H

#include "nasm.h"
#include "iflag.h"

extern iflag_t cpu;
extern bool in_absolute;        /* Are we in an absolute segment? */
extern struct location absolute;

enum match_result {
    /*
     * Matching errors.  These should be sorted so that more specific
     * errors come later in the sequence.
     */
    MERR_INVALOP,
    MERR_OPSIZEMISSING,
    MERR_OPSIZEMISMATCH,
    MERR_BRNOTHERE,
    MERR_BRNUMMISMATCH,
    MERR_MASKNOTHERE,
    MERR_DECONOTHERE,
    MERR_BADCPU,
    MERR_BADMODE,
    MERR_BADHLE,
    MERR_ENCMISMATCH,
    MERR_BADBND,
    MERR_BADREPNE,
    MERR_REGSETSIZE,
    MERR_REGSET,
    /*
     * Matching success; the conditional ones first
     */
    MOK_JUMP,		/* Matching OK but needs jmp_match() */
    MOK_GOOD		/* Matching unconditionally OK */
};

void gencode(struct out_data *data, insn *ins);
enum match_result find_match(const struct itemplate **tempp,
                                    insn *instruction,
                                    int32_t segment, int64_t offset, int bits);

int64_t insn_size(int32_t segment, int64_t offset, int bits, insn *instruction);
int64_t assemble(int32_t segment, int64_t offset, int bits, insn *instruction);

#endif
