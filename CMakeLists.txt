cmake_minimum_required(VERSION 3.10)

project(x86-insn-generator)

SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_C_FLAGS "-c -g -O2 -std=c17 -ggdb3 -O0 -fno-omit-frame-pointer -fwrapv -U__STRICT_ANSI__ -fno-common -Werror=attributes -ffunction-sections -fdata-sections -fvisibility=hidden -Wall -W -pedantic -Wc90-c99-compat -Wno-long-long -Wno-stringop-truncation -Wno-shift-negative-value -Werror=implicit -Werror=missing-braces -Werror=return-type -Werror=trigraphs -Werror=pointer-arith -Werror=missing-prototypes -Werror=missing-declarations -Werror=comment -Werror=vla -DHAVE_CONFIG_H ")
SET(RUNPERL perl -I${srcdir}/perllib -I${srcdir})

# @headers
include_directories(${srcdir}
                    ${srcdir}/include
                    ${srcdir}/x86
                    ${srcdir}/asm
                    ${srcdir}/output
                    ${srcdir}/config)

# @OUTPUT: generate dependencies
SET(GEN_TARS
  ${srcdir}/version.h
  ${srcdir}/version.mac
  ${srcdir}/version.sed
  ${srcdir}/version.mak

  ${srcdir}/x86/iflag.c
  ${srcdir}/x86/iflaggen.h
  ${srcdir}/x86/insnsb.c
  ${srcdir}/x86/insnsa.c
  ${srcdir}/x86/insnsd.c
  ${srcdir}/x86/insnsi.h
  ${srcdir}/x86/insnsn.c
  ${srcdir}/x86/regs.c
  ${srcdir}/x86/regflags.c
  ${srcdir}/x86/regdis.c
  ${srcdir}/x86/regdis.h
  ${srcdir}/x86/regvals.c
  ${srcdir}/x86/regs.h

  ${srcdir}/include/warnings.h
  ${srcdir}/asm/warnings.c
  ${srcdir}/asm/tokhash.c
  ${srcdir}/asm/tokens.h
  ${srcdir}/asm/pptok.h
  ${srcdir}/asm/pptok.c
  ${srcdir}/asm/pptok.ph
  ${srcdir}/asm/directiv.h
  ${srcdir}/asm/directbl.c

  ${srcdir}/macros/macros.c)

add_custom_command(OUTPUT ${GEN_TARS}
  COMMAND ${RUNPERL} ${srcdir}/version.pl h < ${srcdir}/version > ${srcdir}/version.h
  COMMAND ${RUNPERL} ${srcdir}/version.pl mac < ${srcdir}/version > ${srcdir}/version.mac
  COMMAND ${RUNPERL} ${srcdir}/version.pl sed < ${srcdir}/version > ${srcdir}/version.sed
  COMMAND ${RUNPERL} ${srcdir}/version.pl make < ${srcdir}/version > ${srcdir}/version.mak

  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -fc ${srcdir}/x86/insns.dat ${srcdir}/x86/iflag.c
  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -fh ${srcdir}/x86/insns.dat ${srcdir}/x86/iflaggen.h
  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -b ${srcdir}/x86/insns.dat ${srcdir}/x86/insnsb.c
  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -a ${srcdir}/x86/insns.dat ${srcdir}/x86/insnsa.c
  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -d ${srcdir}/x86/insns.dat ${srcdir}/x86/insnsd.c
  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -i ${srcdir}/x86/insns.dat ${srcdir}/x86/insnsi.h
  COMMAND ${RUNPERL} ${srcdir}/x86/insns.pl -n ${srcdir}/x86/insns.dat ${srcdir}/x86/insnsn.c
 
  COMMAND ${RUNPERL} ${srcdir}/x86/regs.pl c ${srcdir}/x86/regs.dat > ${srcdir}/x86/regs.c
  COMMAND ${RUNPERL} ${srcdir}/x86/regs.pl fc ${srcdir}/x86/regs.dat > ${srcdir}/x86/regflags.c
  COMMAND ${RUNPERL} ${srcdir}/x86/regs.pl dc ${srcdir}/x86/regs.dat > ${srcdir}/x86/regdis.c
  COMMAND ${RUNPERL} ${srcdir}/x86/regs.pl dh ${srcdir}/x86/regs.dat > ${srcdir}/x86/regdis.h
  COMMAND ${RUNPERL} ${srcdir}/x86/regs.pl vc ${srcdir}/x86/regs.dat > ${srcdir}/x86/regvals.c
  COMMAND ${RUNPERL} ${srcdir}/x86/regs.pl h ${srcdir}/x86/regs.dat > ${srcdir}/x86/regs.h

  COMMAND ${RUNPERL} ${srcdir}/asm/warnings.pl c ${srcdir}/asm/warnings.c ${srcdir}
  COMMAND ${RUNPERL} ${srcdir}/asm/warnings.pl h ${srcdir}/include/warnings.h ${srcdir}
  COMMAND ${RUNPERL} ${srcdir}/asm/tokhash.pl c ${srcdir}/x86/insns.dat ${srcdir}/x86/regs.dat ${srcdir}/asm/tokens.dat > ${srcdir}/asm/tokhash.c
  COMMAND ${RUNPERL} ${srcdir}/asm/tokhash.pl h ${srcdir}/x86/insns.dat ${srcdir}/x86/regs.dat ${srcdir}/asm/tokens.dat > ${srcdir}/asm/tokens.h
  COMMAND ${RUNPERL} ${srcdir}/asm/pptok.pl h ${srcdir}/asm/pptok.dat ${srcdir}/asm/pptok.h
  COMMAND ${RUNPERL} ${srcdir}/asm/pptok.pl c ${srcdir}/asm/pptok.dat ${srcdir}/asm/pptok.c
  COMMAND ${RUNPERL} ${srcdir}/asm/pptok.pl ph ${srcdir}/asm/pptok.dat ${srcdir}/asm/pptok.ph
  COMMAND ${RUNPERL} ${srcdir}/nasmlib/perfhash.pl h ${srcdir}/asm/directiv.dat ${srcdir}/asm/directiv.h
  COMMAND ${RUNPERL} ${srcdir}/nasmlib/perfhash.pl c ${srcdir}/asm/directiv.dat ${srcdir}/asm/directbl.c

  COMMAND ${RUNPERL} ${srcdir}/macros/macros.pl ${srcdir}/version.mac ${srcdir}/macros/*.mac ${srcdir}/output/*.mac ${srcdir})


# @target: perlreq, nasmlib dependencies
add_custom_target(perlreq DEPENDS ${GEN_TARS})

# @library: nasm library
file(GLOB LIB_SRCS stdlib/*.c
                   nasmlib/*.c
                   common/*.c
                   x86/*.c
                   asm/*.c
                   macros/*.c
                   output/*.c)
add_library(nasmlib STATIC ${LIB_SRCS})
add_dependencies(nasmlib perlreq)

# @executable: nasm
SET(CMAKE_C_FLAGS "-g -O2 -std=c17 -ggdb3 -O0 -fno-omit-frame-pointer -fwrapv -U__STRICT_ANSI__ -fno-common -Werror=attributes -ffunction-sections -fdata-sections -fvisibility=hidden -Wall -W -pedantic -Wc90-c99-compat -Wno-long-long -Wno-stringop-truncation -Wno-shift-negative-value -Werror=implicit -Werror=missing-braces -Werror=return-type -Werror=trigraphs -Werror=pointer-arith -Werror=missing-prototypes -Werror=missing-declarations -Werror=comment -Werror=vla -DHAVE_CONFIG_H ")
add_executable(nasm ${srcdir}/asm/nasm.c)
target_link_libraries(nasm nasmlib)
