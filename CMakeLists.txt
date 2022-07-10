cmake_minimum_required(VERSION 3.5)

if(${CMAKE_VERSION} VERSION_LESS 3.5)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(PlusWM VERSION 0.1
    DESCRIPTION "my questionable attempt at writing a dynamic window manager in the style of suckless' dwm, but in c++, cuz i'm modern af"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE "Debug")

find_program(DISTCC_EXECUTABLE distcc)
mark_as_advanced(DISTCC_EXECUTABLE)
if(DISTCC_EXECUTABLE)
  foreach(LANG C CXX)
	if(NOT DEFINED CMAKE_${LANG}_COMPILER_LAUNCHER AND NOT CMAKE_${LANG}_COMPILER MATCHES ".*/distcc")
	  message(STATUS "Enabling 'distcc' for ${LANG}")
	  set(CMAKE_${LANG}_COMPILER_LAUNCHER distcc)
	endif()
  endforeach()
endif()

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_ASM_COMPILER gcc)
set(CMAKE_LINKER ld)
set(CMAKE_STRIP strip)

# compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)

add_custom_target(run
	COMMAND ${CMAKE_SOURCE_DIR}/meta/run.sh
	USES_TERMINAL
	)

add_custom_target(format
	COMMAND ${CMAKE_SOURCE_DIR}/meta/format.sh
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)

INSTALL(FILES build/pluswm DESTINATION /usr/local/bin/)


if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    endif ()
endif ()

add_compile_definitions(VERSION=${PROJECT_VERSION})

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-Wextra)
add_compile_options(-Wlogical-op)
add_compile_options(-Wmisleading-indentation)
add_compile_options(-Wimplicit-fallthrough)
add_compile_options(-Wmissing-declarations)
add_compile_options(-Wunused)
add_compile_options(-Wundef)

add_compile_options(-fno-exceptions)
add_compile_options(-ggdb)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(lib)

add_executable(pluswm src/main.cpp)

target_link_libraries(pluswm WM Util Keybind Client Button glog)
