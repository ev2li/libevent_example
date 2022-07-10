cmake_minimum_required(VERSION 2.8)
project(ev2li)
include (cmake/utils.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations -DENABLE_PRECOMPILED_HEADERS=OFF")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(LIB_SRC
       ev2li/test.cc
)

set(LIBS
        event
)

add_library(ev2li SHARED ${LIB_SRC})

force_redefine_file_macro_for_sources(ev2li) #__FILE__

ev2li_add_executable(test_first_eventlib "tests/first_libevent.cc" ev2li "${LIBS}")
ev2li_add_executable(test_server "tests/test_server.cc" ev2li "${LIBS}")
ev2li_add_executable(test_config "tests/test_config.cc" ev2li "${LIBS}")
ev2li_add_executable(test_signal "tests/test_signal.cc" ev2li "${LIBS}")
ev2li_add_executable(test_timer "tests/test_timer.cc" ev2li "${LIBS}")
ev2li_add_executable(test_file "tests/test_file.cc" ev2li "${LIBS}")
ev2li_add_executable(test_event_server "tests/test_event_server.cc" ev2li "${LIBS}")
ev2li_add_executable(test_signal_loop "tests/test_signal_loop.cc" ev2li "${LIBS}")
ev2li_add_executable(test_buffer "tests/test_buffer.cc" ev2li "${LIBS}")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)