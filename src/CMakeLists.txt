cmake_minimum_required(VERSION 2.6)

project(xkcptun C)

set(src_win32 netstd.h getopt.c getopt1.c)

set(src_xkcp_spy
	${src_win32}
	xkcp_spy.c)

set(src_xkcp_server
	${src_win32}
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
	${src_win32}
	getopt.c
	getopt1.c
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

set(libs ${libs} ${EVENT_LIBRARY} ${EVENT_PTHREADS_LIBRARY} ${EVENT_EVENT_OPENSSL_LIBRARY})

#ADD_DEFINITIONS(-Wall -O2 --std=gnu99 -Wmissing-declarations)


add_executable(xkcp_spy ${src_xkcp_spy})
target_link_libraries(xkcp_spy ${link_flag} ${libs})

add_executable(xkcp_client ${src_xkcp_client})
target_link_libraries(xkcp_client ${link_flag} ${libs})

add_executable(xkcp_server ${src_xkcp_server})
target_link_libraries(xkcp_server ${link_flag} ${libs})

#install(TARGETS xkcp_client xkcp_server
#        RUNTIME DESTINATION bin
#)
