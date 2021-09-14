cmake_minimum_required(VERSION 3.8)
project(MaciaLanguage C CXX)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configure primary executable target
add_executable(macia 
    generator/generator.cpp
    interpreter/interpreter.cpp
    lexer/scanner.cpp
    parser/parser.cpp
    shared/codeobject.cpp
    shared/datapool.cpp
    shared/element.cpp
    shared/stringbuffer.cpp
    macia.cpp
)

# Add a new install target
install(TARGETS macia
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY examples DESTINATION share/macia)
