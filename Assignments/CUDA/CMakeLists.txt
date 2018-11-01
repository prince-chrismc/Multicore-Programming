cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Cuda-Assignment)

# set up versioning.
set(BUILD_MAJOR "0")
set(BUILD_MINOR "0")
set(BUILD_VERSION "0")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_VERSION})

# You can tweak some common (for all subprojects) stuff here. For example:
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

# OS and compiler checks.
if(UNIX)
    # linux / normal unix
    add_definitions(-D_LINUX)
    set(CMAKE_CXX_STANDARD 17)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
      message("Building 'Debug' configuration.")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D_DEBUG")
    elseif(CMAKE_BUILD_TYPE MATCHES Release)
      message("Building 'Release' configuration.")
    else()
      set(CMAKE_BUILD_TYPE Release)
      message("Defaulting to 'Release' configuration.")
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_definitions(-D_CLANG)
    endif()
    if(CYGWIN)
        # Special Cygwin stuff here
    elseif(APPLE)
        # Special Apple stuff here
        remove_definitions(-D_LINUX)
        add_definitions(-D_DARWIN)
    endif()
elseif(WIN32)
    add_definitions(-D_WIN32)
    if(MINGW)
        # Special MINGW stuff here
    elseif(MSVC)
        # Special MSVC stuff here
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    else()
        # No idea what it is, but there's a good chance it's too weird.
        MESSAGE( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

if(NOT DEFINED CMAKE_CUDA_STANDARD)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif()

find_package(CUDA REQUIRED)

# When done tweaking common stuff, configure the components (subprojects).

# NOTE: The order matters! The most independent ones should go first.

FILE(GLOB CUDA_SOURCE_CODE "src/*")

if( CMAKE_CL_64 )
    FILE(GLOB CUDA_LIBS "${CUDA_TOOLKIT_ROOT_DIR}/lib/x64/*") # I only need this to work on windows as of now
elseif()
    FILE(GLOB CUDA_LIBS "${CUDA_TOOLKIT_ROOT_DIR}/lib/Win32/*")
endif()

if(UNIX)
elseif(WIN32)
    ADD_EXECUTABLE(Galaxy-Collider Main.cpp ${CUDA_SOURCE_CODE})
    TARGET_LINK_LIBRARIES(Galaxy-Collider ${CUDA_LIBS})
    target_include_directories(Galaxy-Collider PRIVATE src ${CUDA_TOOLKIT_ROOT_DIR}/include)
endif()
