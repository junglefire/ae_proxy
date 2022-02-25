PROJECT(smart_proxy)

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

IF(WIN32)
	ADD_DEFINITIONS(-DWIN32_MESSAGE)
	ADD_DEFINITIONS(-DWIN32_MSVC)
	ADD_COMPILE_OPTIONS(/W4)
	ADD_COMPILE_OPTIONS(/wd4100)
	ADD_COMPILE_OPTIONS(/wd4244)
ELSE()
	ADD_DEFINITIONS(-DDETAIL_MESSAGE)
	# ADD_DEFINITIONS(-DSIMPLE_MESSAGE)
	ADD_DEFINITIONS(-w)
ENDIF()

IF(WIN32)
ELSE()
	SET(CMAKE_C_FLAGS_DEBUG "-O0 -Wall -g -ggdb")
	SET(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")
	SET(CMAKE_CXX_STANDARD 14)
	SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb")
	SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")
ENDIF()

SET(CMAKE_BUILD_TYPE Release)
SET(EXECUTABLE_OUTPUT_PATH bin)
SET(LIBRARY_OUTPUT_PATH lib)

INCLUDE_DIRECTORIES(base/)
INCLUDE_DIRECTORIES(ae/)
INCLUDE_DIRECTORIES(pkg/include)

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/pkg/lib)



###############################################################################
##############################  module  #######################################
###############################################################################

AUX_SOURCE_DIRECTORY(ae AE)
ADD_LIBRARY(ae ${AE})



###############################################################################
###########################   application  ####################################
###############################################################################

# SET(SMARTPROXY app/main.cpp app/udp_proxy.cpp app/udp_agent.cpp app/tcp_proxy.cpp)
# ADD_EXECUTABLE(smart_proxy ${SMARTPROXY})
# TARGET_LINK_LIBRARIES(smart_proxy ae uv msgpackc)



###############################################################################
##############################   test  ########################################
###############################################################################

# SET(TCPCLIENT tests/test_tcp_client.cpp)
# ADD_EXECUTABLE(test_tcp_client ${TCPCLIENT})
# TARGET_LINK_LIBRARIES(test_tcp_client ae uv)

# SET(TCPSERVER tests/test_tcp_server.cpp)
# ADD_EXECUTABLE(test_tcp_server ${TCPSERVER})
# TARGET_LINK_LIBRARIES(test_tcp_server ae uv)

# SET(UDPSERVER tests/test_udp_server.cpp)
# ADD_EXECUTABLE(test_udp_server ${UDPSERVER})
# TARGET_LINK_LIBRARIES(test_udp_server ae uv)

SET(UDPCLIENT tests/test_udp_client.cpp)
ADD_EXECUTABLE(test_udp_client ${UDPCLIENT})
TARGET_LINK_LIBRARIES(test_udp_client ae uv_a msgpackc Ws2_32 Iphlpapi Userenv Psapi)

# SET(MSGPACK tests/test_msgpack.cpp)
# ADD_EXECUTABLE(test_msgpack ${MSGPACK})
# TARGET_LINK_LIBRARIES(test_msgpack msgpackc)







