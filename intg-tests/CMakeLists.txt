SET(CMAKE_C_FLAGS "-g -O0 -Wall -DHAVE_CONFIG_H ${EXTRA_CFLGAS}")

file(GLOB_RECURSE INTG_TESTS_SRCS *.c)

add_executable(intg ${INTG_TESTS_SRCS})

target_link_libraries(intg nasmlib ${PKGS_DEP_LDFLAGS})
