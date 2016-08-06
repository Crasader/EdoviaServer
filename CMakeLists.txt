cmake_minimum_required(VERSION 3.5)
project(EdoviaServer)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

IF (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    # use -Wall for gcc
    IF(NOT CMAKE_CXX_FLAGS MATCHES "-Wall$")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    ENDIF(NOT CMAKE_CXX_FLAGS MATCHES "-Wall$")

    IF(NOT CMAKE_C_FLAGS MATCHES "-Wall$")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    ENDIF(NOT CMAKE_C_FLAGS MATCHES "-Wall$")
ENDIF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -std=c++11")

set(SOURCE_FILES src/main.cpp src/Network/Network.cpp src/Network/Network.h src/Network/Connection.cpp src/Network/Connection.h src/Log/Logger.cpp src/Log/Logger.h src/Network/ByteBuffer.h src/Network/Packet.h src/World/Zone.cpp src/World/Zone.h src/utility/utility.h src/Server/Server.cpp src/Server/Server.h src/World/ZonePool.cpp src/World/ZonePool.h src/Network/OpcodeHandler.cpp src/Network/OpcodeHandler.h src/Network/PlayerSession.cpp src/Network/PlayerSession.h src/World/ZoneManager.cpp src/World/ZoneManager.h thirdparty/concurrentqueue/concurrentqueue.h)
add_executable(EdoviaServer ${SOURCE_FILES})

find_package(LibUV REQUIRED)


include_directories(SYSTEM thirdparty src ${LIBUV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBUV_LIBRARIES})
