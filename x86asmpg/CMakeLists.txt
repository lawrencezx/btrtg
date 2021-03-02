SET(CMAKE_C_FLAGS "-g -O0 -Wall -DHAVE_CONFIG_H ${EXTRA_CFLGAS}")

file(GLOB INTG_TESTS_SRCS *.c)

add_executable(x86asmpg ${INTG_TESTS_SRCS})
install(TARGETS x86asmpg)

target_link_libraries(x86asmpg nasmlib ${PKGS_DEP_LDFLAGS})
