file(GLOB_RECURSE TEST_SRCS *.c)
add_library(testlib STATIC ${TEST_SRCS})
add_dependencies(testlib perlreq)
