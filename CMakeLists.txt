project(xkcptun)
cmake_minimum_required(VERSION 2.6)

set(PLATFORM X86)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

if(WIN32) # Win32
  set(PLATFORM_FOLDER win32)
elseif(APPLE)# osx or ios
  set(PLATFORM_FOLDER mac)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
else() # Assume Linux
  set(PLATFORM_FOLDER linux)
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM X64)
    MESSAGE(STATUS "Detected 64-bit platform")
	set(ARCH_DIR "64bit")
else()
    MESSAGE(STATUS "Detected 32-bit platform")
	set(ARCH_DIR "32bit")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message("-- Using Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_MACOSX_RPATH 1)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message("-- Using GCC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -pthread -fPIC") #-lrt
  add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
  add_definitions(-fno-delete-null-pointer-checks)
  #add_definitions(-ggdb3)
  #add_definitions(-Wall -Wfatal-errors -Wextra)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  message("-- Using Intel C++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  message("-- Using Visual Studio C++")
endif()

#if(CMAKE_COMPILER_IS_GNUCXX)
#endif()

if (WIN32)
  set(CMAKE_CPP_FLAGS "${CMAKE_C_FLAGS} /wd4819")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

  if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS
					-D_SCL_SECURE_NO_WARNINGS
					-wd4251 -wd4244 -wd4334
					-wd4005 -wd4820 -wd4710
					-wd4514 -wd4056 -wd4996 -wd4099
					-wd4819)
  endif()

  add_definitions(-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
  if(${PLATFORM} STREQUAL "X64")
    add_definitions("-D_WIN64")
    #Enable extended object support for debug compiles on X64 (not required on X86)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
  endif()
elseif(APPLE)
  MESSAGE("-- Using APPLE")
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
  add_definitions("-DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl/1.0.2l/")
elseif(UNIX)
  message("-- Using UNIX")
endif()

if (NOT WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX)
    add_subdirectory(jemalloc)
    set(xd_libs ${xd_libs} jemalloc)
  else()
    set(xd_libs ${xd_libs} iconv)
  endif()
endif()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

####################################################################
option(WITH_OPENSSL_SOURCES         "use openssl sources"       ON)
option(WITH_EVENT_SOURCES         "use libevents sources"       ON)

if(WITH_OPENSSL_SOURCES)
  add_subdirectory(openssl)
  set(OPENSSL_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/openssl/include")
  set(OPENSSL_LIBRARIES     "ssl;crypto")
else()
  find_package(OpenSSL REQUIRED)
endif()

message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL lib: ${OPENSSL_LIBRARIES}")

if(WITH_EVENT_SOURCES)
  set(EVENT_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/libevent/include" "${CMAKE_BINARY_DIR}/libevent/include")
  set(EVENT_LIBRARY event_static event_openssl_static)
  #set(EVENT_PTHREADS_LIBRARY event_pthreads_static)
  add_subdirectory(libevent)
else()
  find_path(EVENT_INCLUDE_DIR event2/event.h
    PATHS /usr/include /usr/local/include
  )

  find_library(EVENT_PTHREADS_LIBRARY
    NAMES event_pthreads
    PATHS /usr/lib /usr/local/lib
  )
  find_library(EVENT_LIBRARY
    NAMES event
    PATHS /usr/lib /usr/local/lib
  )
  find_library(EVENT_EVENT_OPENSSL_LIBRARY
    NAMES event_openssl
    PATHS /usr/lib /usr/local/lib
  )


  include(FindPackageHandleStandardArgs)

  find_package_handle_standard_args(EVENT
    DEFAULT_MSG
    EVENT_INCLUDE_DIR
    EVENT_LIBRARY
  )
endif()

message("event library:  ${EVENT_LIBRARY} ${EVENT_PTHREADS_LIBRARY} ${EVENT_EVENT_OPENSSL_LIBRARY}")

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${EVENT_INCLUDE_DIR}
)
####################################################################

add_subdirectory(src)

