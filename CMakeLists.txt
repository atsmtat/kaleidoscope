cmake_minimum_required(VERSION 3.5)

# set the project name
project(kaleidoscope)

SET (CMAKE_CXX_FLAGS_INIT "`llvm-config --cxxflags --ldflags --system-libs --libs core native`")
SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-g")

# add the executable
add_executable(klc src/lexer.cpp src/parser.cpp src/codegen.cpp src/klc.cpp)

install(TARGETS klc DESTINATION bin)
