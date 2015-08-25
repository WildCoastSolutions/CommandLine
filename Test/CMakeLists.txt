
include_directories (../../UnitTesting)
include_directories (../)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DWILD_UNITTESTING_SHOW_FAILURE_DETAILS" )
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-write-strings" )

add_executable (CommandLineTest CommandLine.Test.cpp)

add_custom_command(
	TARGET CommandLineTest POST_BUILD
   	COMMAND CommandLineTest
 )
