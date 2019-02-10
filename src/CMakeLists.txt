cmake_minimum_required(VERSION 2.6)

project(xkcptun C)

#######################################################################

option(WITH_EVENT_SOURCES         "use libevents sources"       ON)
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

#########################################################
set(src_xkcp_spy
	xkcp_spy.c)

set(src_xkcp_server
	xkcp_server.c
	tcp_client.c
	debug.c
	ikcp.c
	jwHash.c
	commandline.c
	xkcp_util.c
	xkcp_config.c
    json.c
	xkcp_mon.c)
	
set(src_xkcp_client
	tcp_proxy.c
	xkcp_client.c
	xkcp_config.c
	xkcp_util.c
	commandline.c
	debug.c
	ikcp.c
	jwHash.c
	json.c
	xkcp_mon.c)

set(libs
    m
	event)

#ADD_DEFINITIONS(-Wall -O2 --std=gnu99 -Wmissing-declarations)


add_executable(xkcp_spy ${src_xkcp_spy})
target_link_libraries(xkcp_spy ${link_flag} ${libs})

add_executable(xkcp_client ${src_xkcp_client})
target_link_libraries(xkcp_client ${link_flag} ${libs})

add_executable(xkcp_server ${src_xkcp_server})
target_link_libraries(xkcp_server ${link_flag} ${libs})

install(TARGETS xkcp_client xkcp_server
        RUNTIME DESTINATION bin
)
