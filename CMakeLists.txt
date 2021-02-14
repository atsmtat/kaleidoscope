cmake_minimum_required(VERSION 3.5)

# set the project name
project(kaleidoscope)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# prepare llvm specific flags
execute_process(COMMAND llvm-config --cxxflags COMMAND tr -s "\n" " " OUTPUT_VARIABLE llvm-cxxflags )
execute_process(COMMAND llvm-config --ldflags COMMAND tr -s "\n" " " OUTPUT_VARIABLE llvm-ldflags)
execute_process(COMMAND llvm-config --system-libs COMMAND tr -s "\n" " " OUTPUT_VARIABLE llvm-system-libs)
execute_process(COMMAND llvm-config --libs core native COMMAND tr -s "\n" " " OUTPUT_VARIABLE llvm-libs)
string(CONCAT llvm-flags ${llvm-cxxflags} ${llvm-ldflags} ${llvm-system-libs} ${llvm-libs})

# set cxx flags
set(CMAKE_CXX_FLAGS "${llvm-flags} -Wno-unused-command-line-argument")

# add a lib
add_library(irgen src/lexer.cpp src/parser.cpp src/codegen.cpp)

# add the executable
add_executable(klc src/klc.cpp)
target_link_libraries(klc PUBLIC irgen)

# installation
install(TARGETS klc DESTINATION bin)
