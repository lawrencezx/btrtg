cmake_minimum_required(VERSION 3.10)

project(x86-insn-generator)

INCLUDE(FindPkgConfig)
pkg_check_modules(PKGS_DEP REQUIRED glib-2.0)

FOREACH(flag ${PKGS_DEP_CFLAGS})
	SET(EXTRA_CFLGAS "${EXTRA_CFLGAS} ${flag}")
ENDFOREACH(flag)

SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_C_FLAGS "-c -g -O2 -ggdb3 -O0 -fno-omit-frame-pointer -fwrapv -U__STRICT_ANSI__ -fno-common -Werror=attributes -ffunction-sections -fdata-sections -fvisibility=hidden -Wall -W -pedantic -Wno-long-long -Wno-stringop-truncation -Wno-shift-negative-value -Werror=implicit -Werror=missing-braces -Werror=return-type -Werror=trigraphs -Werror=pointer-arith -Werror=missing-prototypes -Werror=missing-declarations -Werror=comment -Werror=vla -DHAVE_CONFIG_H ${EXTRA_CFLGAS}")
SET(RUNPERL perl -I${srcdir}/perllib -I${srcdir})

# @headers
include_directories(${srcdir}
                    ${srcdir}/include
                    ${srcdir}/x86
                    ${srcdir}/stat
                    ${srcdir}/xmlparser
                    ${srcdir}/engine
                    ${srcdir}/config
                    ${srcdir}/disasm
                    /usr/include/libxml2)

# @OUTPUT: generate dependencies
SET(GEN_TARS
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
  ${srcdir}/engine/warnings.c
  ${srcdir}/engine/tokhash.c
  ${srcdir}/engine/tokens.h)

add_custom_command(OUTPUT ${GEN_TARS}
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

  COMMAND ${RUNPERL} ${srcdir}/engine/warnings.pl c ${srcdir}/engine/warnings.c ${srcdir}
  COMMAND ${RUNPERL} ${srcdir}/engine/warnings.pl h ${srcdir}/include/warnings.h ${srcdir}
  COMMAND ${RUNPERL} ${srcdir}/engine/tokhash.pl c ${srcdir}/x86/insns.dat ${srcdir}/x86/regs.dat ${srcdir}/engine/tokens.dat > ${srcdir}/engine/tokhash.c
  COMMAND ${RUNPERL} ${srcdir}/engine/tokhash.pl h ${srcdir}/x86/insns.dat ${srcdir}/x86/regs.dat ${srcdir}/engine/tokens.dat > ${srcdir}/engine/tokens.h)


# @target: perlreq, nasmlib dependencies
add_custom_target(perlreq DEPENDS ${GEN_TARS})

# @library: nasm library
file(GLOB LIB_SRCS stdlib/*.c
                   nasmlib/*.c
                   common/*.c
                   x86/*.c
                   stat/*.c
                   xmlparser/*.c
                   engine/*.c
                   disasm/*.c
                   output/*.c)
add_library(nasmlib STATIC ${LIB_SRCS})
target_link_libraries(nasmlib xml2)
#target_link_libraries(nasmlib libxml2.so)
add_dependencies(nasmlib perlreq)

add_subdirectory(intg-tests)

#configure_file(./intg-tests/user-lib/ld.script ld.script COPYONLY)
#configure_file(./intg-tests/user-lib/user-compile.sh user-compile.sh COPYONLY)
#configure_file(./intg-tests/user-lib/verification.h verification.h COPYONLY)
#configure_file(./intg-tests/user-lib/outputlib.c outputlib.c COPYONLY)
#configure_file(./intg-tests/user-lib/checklib.c checklib.c COPYONLY)
#configure_file(./intg-tests/user-lib/checkfunctions.h checkfunctions.h COPYONLY)

file(COPY ./intg-tests/user-lib DESTINATION ./)
file(COPY ./intg-tests/system-lib DESTINATION ./)
