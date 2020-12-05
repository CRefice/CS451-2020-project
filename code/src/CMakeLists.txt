# DO NAME THE SYMBOLIC VARIABLE `SOURCES`

include_directories(include)
set(SOURCES
    src/best-effort-broadcast.cpp
    src/config-parser.cpp
    src/fair-loss-link.cpp
    src/fifo-broadcast.cpp
    src/localized-causal-broadcast.cpp
    src/logger.cpp
    src/main.cpp
    src/perfect-link.cpp
    src/rolling-set.cpp
    src/udp.cpp
    src/uniform-reliable-broadcast.cpp
)

# DO NOT EDIT THE FOLLOWING LINE
find_package(Threads)
add_executable(da_proc ${SOURCES})
target_link_libraries(da_proc ${CMAKE_THREAD_LIBS_INIT})
