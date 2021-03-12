SET(CMAKE_C_FLAGS "-g -O0 -Wall -DHAVE_CONFIG_H ${EXTRA_CFLGAS}")

file(GLOB INTG_TESTS_SRCS *.c)

add_executable(btrtg ${INTG_TESTS_SRCS})
install(TARGETS btrtg RUNTIME DESTINATION /usr/local/bin)

target_link_libraries(btrtg nasmlib ${PKGS_DEP_LDFLAGS})
