###########
# Project #
###########
cmake_minimum_required(VERSION 3.16)
project(ksmaze CXX)

###########
# Headers #
###########
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

##########################
# Compiler configuration #
##########################

add_compile_options(-O2 -Werror -Wall -Wextra -Wformat=2 -Wno-unused-parameter
-Wformat-overflow=2 -Wformat-truncation=2 -Wformat-security -Wnull-dereference
-Wstack-protector -Wtrampolines -Walloca -Wvla -Warray-bounds=2
-Wimplicit-fallthrough=3 -Wshift-overflow=2 -Wcast-qual -Wstringop-overflow=4
-Wconversion -Warith-conversion -Wlogical-op -Wduplicated-cond
-Wduplicated-branches -Wformat-signedness -Wshadow -Wstrict-overflow=4 -Wundef
-Wswitch-default -Wswitch-enum -Wstack-usage=1000000 -Wcast-align=strict
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
-fstack-protector-strong -fstack-clash-protection -fPIE -fsanitize=bounds
-fsanitize-undefined-trap-on-error -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
-Wl,-z,separate-code) 
# src: https://github.com/airbus-seclab/c-compiler-security (Not C but well...)

########################
# CMake Configurations #
########################

# Generate compile commands (for nvim/emacs LSP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Use ccache if found by default
option(CCACHE_ENABLE "Enable ccache compilation cache" ON)
find_program(CCACHE_BIN ccache)
if(CCACHE_BIN AND ${CCACHE_ENABLE})
	message(STATUS "Enabled ccache (${CCACHE_BIN}) for the project")
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_BIN}")
	set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_BIN}")
endif()

###############
# Executables #
###############
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# ksm
add_executable(ksmaze "${SRC}/main.cpp")
target_link_libraries(ksmaze ${CURSES_LIBRARIES})

add_custom_command(TARGET ksmaze
	COMMENT "Symlinking compile_commands.json for LSP convenience"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
	POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E create_symlink 
	  "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
	  "${PROJECT_SOURCE_DIR}/compile_commands.json")
