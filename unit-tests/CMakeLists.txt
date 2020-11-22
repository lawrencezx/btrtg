SET(CMAKE_C_FLAGS "-g -O0 -Wall -DHAVE_CONFIG_H")

file(GLOB_RECURSE UNIT_TESTS_SRCS *.c)

add_executable(ut ${UNIT_TESTS_SRCS})

target_link_libraries(ut nasmlib)
