cmake_minimum_required(VERSION 3.18)
set (CMAKE_C_STANDARD 11)

project(cedh_matching C)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg].*")
    if(supported)
        message(STATUS "IPO/LTO enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO/LTO not supported: <${error}>")
    endif()
else()
    message(STATUS "IPO/LTO disabled in non-release build")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# find_package(OpenSSL 3 REQUIRED)

set(INCLUDES
    lib/rosalia/includes
    src
)

set(SOURCES
    src/rosa_impl/argparse.c
    src/rosa_impl/json.c
    src/rosa_impl/noise.c
    src/rosa_impl/timestamp.c
    src/rosa_impl/vector.c

    src/objects.c
    src/search.c
    src/tests.c

    src/main.c
)

add_executable(cedh_matching "${SOURCES}")

target_compile_options(cedh_matching PRIVATE
    # "-Wall"
    # "-Wextra"
    # "-Werror" # warnings as errors
    "-Wfatal-errors" # stop after first error
)

target_include_directories(cedh_matching PRIVATE ${INCLUDES})

target_link_libraries(cedh_matching Threads::Threads)

set_target_properties(cedh_matching PROPERTIES EXPORT_COMPILE_COMMANDS true)
